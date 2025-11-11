#include <zeno/zeno.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <Windows.h>
#include <zeno/io/iohelper.h>
#include <zeno/io/zenreader.h>
#include <zeno/core/NodeImpl.h>
#include <zenovis/Session.h>
#include <zenovis/RenderEngine.h>
#include <zenovis/Scene.h>
#include <zeno/types/UserData.h>
#include <Python.h>
#include <pybind11/pybind11.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace py = pybind11;

PyMODINIT_FUNC PyInit_zen(void);

namespace fs = std::filesystem;

struct ArgDef {
    std::string name;
    std::string type;
    std::string desc;
    std::string defaultVal;
    bool required;
};

static void init_plugins(char* argv0) {
#ifndef _WIN32
    return; //TODO
#endif
    fs::path exePath = fs::path(std::filesystem::canonical(fs::path(argv0))).parent_path();

    std::vector<std::string> plugins = {
        "zs_base.dll",
        "zs_alembic.dll",
        "zs_fbx.dll",
        "zs_imgcv.dll",
        "zs_rigid.dll",
        "zs_subdiv.dll",
        "zs_vdb.dll"
    };

    for (auto plugin : plugins) {
        fs::path targetFile = exePath / plugin;
        if (fs::exists(targetFile)) {
            HMODULE hDll = LoadLibrary(targetFile.string().c_str());
            if (hDll) {
                std::cout << targetFile << " has been loaded" << std::endl;
            }
            else {
                std::cerr << targetFile << " is NULL" << std::endl;
            }
        }
        else {
            std::cerr << targetFile << " does not exist" << std::endl;
        }
    }

}

inline std::wstring s2ws(const std::string& str)
{
#if defined(_WIN32)
    // Windows 平台：使用 MultiByteToWideChar
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    wstr.pop_back(); // 去掉多余的 '\0'
    return wstr;
#else
    // Linux / macOS 平台：使用 std::wstring_convert
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
#endif
}


// 初始化参数表
static const std::vector<ArgDef> gArgDefs = {
    {"file",       "string", "Full path of project (.zen)",          "",     true},
    {"frame",      "int2",   "Frame range of the scene",             "",     false},
    {"size",       "int2",   "Render image size",                    "1024 768", false},
    {"sample",     "int",    "Sample count",                         "1",    false},
    {"path",       "string", "Record output directory",              "",     true},
    {"bitrate",    "int",    "Bitrate",                              "2000", false},
    {"fps",        "int",    "Frames per second",                    "24",   false},
    {"optix",      "bool",   "Use OptiX ray tracing",                "true", false},
    {"video",      "bool",   "Export video",                         "true", false},
    {"aov",        "bool",   "Enable AOV",                           "false",false},
    {"exr",        "bool",   "Export EXR",                           "false",false},
    {"needDenoise","bool",   "Apply denoiser",                       "false",false},
    {"videoname",  "string", "Export video's name",                  "output.mp4", false},
    {"zdalist",    "string", "List of ZDA asset paths",              "",     false}
};

struct CmdArgs {
    std::string file;
    std::vector<int> frame;
    std::vector<int> size = { 1024, 768 };
    int sample = 1;
    std::string path;
    int bitrate = 2000;
    int fps = 24;
    bool optix = true;
    bool video = true;
    bool aov = false;
    bool exr = false;
    bool needDenoise = false;
    bool useFrameInProj = true;
    std::string videoname = "output.mp4";
    std::string zdalist;
};

static bool parseBool(const std::string& s) {
    std::string v = s;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    return (v == "1" || v == "true" || v == "yes" || v == "on");
}

static int runFFmpeg(const std::string& exe, const std::string& args) {
#ifdef _WIN32
    std::ostringstream cmd;
    char drive = std::toupper(exe[0]);
    cmd << drive << ": && \"" << exe << "\" " << args;
    return std::system(cmd.str().c_str());
#else
    std::ostringstream cmd;
    cmd << "\"" << exe << "\" " << args;
    return std::system(cmd.str().c_str());
#endif
}

static void printHelp() {
    std::cout << "ZenoBatch Command Line Tool\n\n";
    std::cout << "Usage:\n  zenobatch [options]\n\n";

    std::cout << "Options:\n";
    for (auto& def : gArgDefs) {
        std::cout << "  --" << def.name;
        if (def.type == "int2") std::cout << " <a> <b>";
        else if (def.type != "bool") std::cout << " <value>";
        std::cout << "\n    Type: " << def.type
            << ", Default: " << (def.defaultVal.empty() ? "(none)" : def.defaultVal)
            << ", " << (def.required ? "Required" : "Optional")
            << "\n    " << def.desc << "\n\n";
    }
    std::cout << "  --help\n    Show this help message\n";
}

int main(int argc, char* argv[]) {
    //--file E:\zeno3\cube.zen --frame 1 20 --path C:\record --optix true
    CmdArgs args;

    if (argc == 1) {
        printHelp();
        return 0;
    }

    std::unordered_map<std::string, ArgDef> defMap;
    for (const auto& d : gArgDefs) defMap["--" + d.name] = d;

    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        std::string value;
        size_t eq = key.find('=');
        if (eq != std::string::npos) {
            value = key.substr(eq + 1);
            key = key.substr(0, eq);
        }

        if (key == "--help" || key == "-h") {
            printHelp();
            return 0;
        }

        auto it = defMap.find(key);
        if (it == defMap.end()) {
            std::cerr << "Unknown argument: " << key << "\n";
            continue;
        }

        const ArgDef& def = it->second;

        if (def.type == "string") {
            if (value.empty() && i + 1 < argc) value = argv[++i];
            if (def.name == "file") args.file = value;
            else if (def.name == "path") args.path = value;
            else if (def.name == "videoname") args.videoname = value;
            else if (def.name == "zdalist") args.zdalist = value;
        }
        else if (def.type == "int") {
            if (value.empty() && i + 1 < argc) value = argv[++i];
            int v = std::stoi(value);
            if (def.name == "sample") args.sample = v;
            else if (def.name == "bitrate") args.bitrate = v;
            else if (def.name == "fps") args.fps = v;
        }
        else if (def.type == "bool") {
            if (value.empty() && i + 1 < argc && argv[i + 1][0] != '-')
                value = argv[++i];
            bool bval = value.empty() ? true : parseBool(value);
            if (def.name == "optix") args.optix = bval;
            else if (def.name == "video") args.video = bval;
            else if (def.name == "aov") args.aov = bval;
            else if (def.name == "exr") args.exr = bval;
            else if (def.name == "needDenoise") args.needDenoise = bval;
        }
        else if (def.type == "int2") {
            if (def.name == "frame") {
                args.frame.clear();
                if (i + 1 < argc) args.frame.push_back(std::stoi(argv[++i]));
                if (i + 1 < argc) args.frame.push_back(std::stoi(argv[++i]));
                args.useFrameInProj = false;
            }
            else if (def.name == "size") {
                args.size.clear();
                if (i + 1 < argc) args.size.push_back(std::stoi(argv[++i]));
                if (i + 1 < argc) args.size.push_back(std::stoi(argv[++i]));
            }
        }
    }

    for (auto& def : gArgDefs) {
        if (def.required) {
            bool ok = true;
            if (def.name == "file" && args.file.empty()) ok = false;
            if (def.name == "path" && args.path.empty()) ok = false;
            if (def.name == "frame" && args.frame.empty()) ok = false;
            if (!ok) {
                std::cerr << "Missing required argument: --" << def.name << "\n";
                return 1;
            }
        }
    }

    std::cout << "===== Parsed Arguments =====\n";
    std::cout << "file: " << args.file << "\n";
    std::cout << "frame: ";
    for (auto f : args.frame) std::cout << f << " ";
    std::cout << "\nsize: " << args.size[0] << "x" << args.size[1] << "\n";
    std::cout << "sample: " << args.sample << "\n";
    std::cout << "path: " << args.path << "\n";
    std::cout << "bitrate: " << args.bitrate << "\n";
    std::cout << "fps: " << args.fps << "\n";
    std::cout << "optix: " << args.optix << "\n";
    std::cout << "video: " << args.video << "\n";
    std::cout << "aov: " << args.aov << "\n";
    std::cout << "exr: " << args.exr << "\n";
    std::cout << "needDenoise: " << args.needDenoise << "\n";
    std::cout << "videoname: " << args.videoname << "\n";
    std::cout << "zdalist: " << args.zdalist << "\n";
    std::cout << "=============================\n";

    auto& sess = zeno::getSession();
    sess.resetMainGraph();

    zeno::getSession().initPyzen([]() {
        if (PyImport_AppendInittab("zen", PyInit_zen) == -1) {
            fprintf(stderr, "Error: could not extend in-built modules table\n");
            exit(1);
        }
        });

    init_plugins(argv[0]);

    sess.createGraph(args.file);

    zenoio::ZSG_PARSE_RESULT ioresult;

    ioresult.path = s2ws(args.file);
    zeno::getSession().globalState->clearState();
    zeno::getSession().init_project_path(ioresult.path);
    zeno::ZSG_VERSION ver = zenoio::getVersion(ioresult.path);

    if (ver == zeno::VER_3) {
        zenoio::ZenReader reader;
        ioresult = reader.openFile(ioresult.path);
        ioresult.num_of_nodes = reader.numOfNodes();
    }
    else {
        ioresult.code = zenoio::PARSE_VERSION_UNKNOWN;
    }

    if (ioresult.code != zenoio::PARSE_NOERROR)
    {
        return -1;
    }

    if (args.useFrameInProj) {
        args.frame.resize(2);
        args.frame[0] = ioresult.timeline.beginFrame;
        args.frame[1] = ioresult.timeline.endFrame;
    }
    else {
        ioresult.timeline.beginFrame = args.frame[0];
        ioresult.timeline.endFrame = args.frame[1];
    }

    sess.initEnv(ioresult);

    auto spGraph = sess.getGraphByPath("main");
    spGraph->register_createNode([&](const std::string& name, zeno::NodeImpl* spNode) {
        int j;
        j = 0;
    });

    zenovis::Session visSess;
    if (args.optix) {
        visSess.set_render_engine("optx");
    }
    else {
        visSess.set_render_engine("bate");

        // 初始化 GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to init GLFW\n";
            return -1;
        }

        // 不显示窗口（真正的离屏渲染）
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // 创建一个隐藏窗口（只为了创建 OpenGL 上下文）
        GLFWwindow* window = glfwCreateWindow(args.size[0], args.size[1], "Offscreen", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        // 初始化 GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD\n";
            return -1;
        }
    }
    visSess.set_window_size(args.size[0], args.size[1]);
    visSess.set_num_samples(args.sample);
    sess.userData().set_bool("output_exr", args.exr);

    zenovis::Scene* pScene = visSess.get_scene();
    zenovis::RenderEngine* pEngine = pScene->renderMan->getEngine();

    fs::path recordDir = args.path;
    if (!fs::exists(recordDir)) {
        std::cerr << "Error: the directory of record " << recordDir << " does not exist\n";
        return 1;
    }
    fs::path imgDir = recordDir / "P";
    if (!fs::exists(imgDir)) {
        fs::create_directory(imgDir);
    }

    for (int frame = args.frame[0]; frame <= args.frame[1]; frame++) {
        zeno::render_reload_info render_infos;
        render_infos.policy = zeno::Reload_Calculation;

        sess.switchToFrame(frame);
        sess.run("", render_infos); //如果局部运行子图，可能要考虑subinput的外部前置计算是否要计算

        if (render_infos.error.failed()) {
            std::cerr << render_infos.error.getErrorMsg();
            return -1;
        }

        sess.globalState->set_working(false);
        auto record_file = zeno::format("{}/P/{:07d}.jpg", args.path, frame);
        visSess.reload(render_infos);
        visSess.do_screenshot(record_file, "jpg", args.optix);
    }

    if (args.video)
    {
        fs::path exePath = fs::absolute(argv[0]);
        fs::path exeDir = exePath.parent_path();
        fs::path ffmpegPath = exeDir / "ffmpeg";

#ifdef _WIN32
        ffmpegPath += ".exe";
#endif

        if (!fs::exists(ffmpegPath)) {
            std::cerr << "Error: ffmpeg not found at " << ffmpegPath << "\n";
            return 1;
        }

        std::string inputPattern = args.path + "/P/%07d.jpg";
        std::string outputFile = args.path + "/" + args.videoname;

        std::ostringstream cmd;
        cmd << " -y"
            << " -start_number " << args.frame[0]
            << " -r " << args.fps
            << " -i \"" << inputPattern << '"'
            << " -b:v " << args.bitrate << "k"
            << " -c:v mpeg4"
            << " \"" << outputFile << '"';

        std::string argscmd = cmd.str();
        int ret = runFFmpeg(ffmpegPath.string(), argscmd);
        if (ret == 0) {
            std::cout << "ffmpeg executed successfully.\n";
        }
        else {
            std::cerr << "ffmpeg execution failed! (exit code = " << ret << ")\n";
            std::cerr << "Please check if ffmpeg exists and the parameters are correct.\n";
        }
    }

    return 0;
}