//
// SmartTexture2D - migrated from zeno to zenocore
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/extra/ShaderNode.h>
#include <zeno/types/TextureObject.h>
#include <zeno/utils/Exception.h>
#include <zeno/utils/format.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/utils/vec.h>
#include <tinygltf/stb_image_write.h>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <string>

namespace zeno {

struct SmartTexture2D : ShaderNodeClone<SmartTexture2D> {
    static constexpr char texWrapping[] =
        "REPEAT MIRRORED_REPEAT CLAMP_TO_EDGE CLAMP_TO_BORDER";
    static constexpr char texFiltering[] =
        "NEAREST LINEAR NEAREST_MIPMAP_NEAREST LINEAR_MIPMAP_NEAREST "
        "NEAREST_MIPMAP_LINEAR LINEAR_MIPMAP_LINEAR";

    virtual int determineType(EmissionPass *em, ZNodeParams *params) override {
        (void)em->determineType(params->get_input_shader("uvtiling"));
        (void)em->determineType(params->get_input_shader("normalScale"));
        (void)em->determineType(params->get_input_shader("heightScale"));
        if (params->has_input("coord")) {
            auto coord = em->determineType(params->get_input_shader("coord"));
            if (coord < 2)
                throw zeno::Exception(
                    "ShaderTexture2D expect coord to be at least vec2");
        }

        auto type = params->get_input2_string("type");
        if (type == "float" || type == "R" || type == "G" || type == "B" ||
            type == "A")
            return 1;
        else if (type == "vec2")
            return 2;
        else if (type == "vec3")
            return 3;
        else if (type == "vec4")
            return 4;
        else
            throw zeno::Exception("ShaderTexture2D got bad type: " + type);
    }

    virtual void emitCode(EmissionPass *em, ZNodeParams *params) override {
        auto texId = em->tex2Ds.size();
        auto tex = std::make_unique<zeno::Texture2DObject>();
        auto texture_path = params->get_input2_string("path");
        if (params->has_input("heatmap")) {
            if (texture_path.empty()) {
                std::srand(static_cast<unsigned>(std::time(nullptr)));
                texture_path = std::filesystem::temp_directory_path().string() +
                              '/' + "heatmap-" +
                              std::to_string(std::rand()) + ".png";
            }

            HeatmapData heatmap = zeno::reflect::any_cast<HeatmapData>(params->get_param_result("heatmap"));
            if (!heatmap.colors.empty()) {
                std::vector<uint8_t> col;
                int width = static_cast<int>(heatmap.colors.size());
                int height = width;
                col.reserve(width * height * 3);
                for (int i = 0; i < height; i++) {
                    (void)i;
                    for (auto &color : heatmap.colors) {
                        col.push_back(
                            zeno::clamp(static_cast<int>(color[0] * 255.99f),
                                        0, 255));
                        col.push_back(
                            zeno::clamp(static_cast<int>(color[1] * 255.99f),
                                        0, 255));
                        col.push_back(
                            zeno::clamp(static_cast<int>(color[2] * 255.99f),
                                        0, 255));
                    }
                }
                stbi_flip_vertically_on_write(0);
                stbi_write_png(texture_path.c_str(), width, height, 3,
                               col.data(), 0);
            }
        }
        if (!std::filesystem::exists(std::filesystem::u8path(texture_path))) {
            auto type = params->get_input2_string("type");
            zeno::vec4f number = zeno::vec4f(0, 0, 0, 0);
            if (params->has_input_float("value")) {
                number[0] = params->get_input2_float("value");
            }
            if (params->has_input_vec2f("value")) {
                auto in = zeno::toVec2f(params->get_input2_vec2f("value"));
                number[0] = in[0];
                number[1] = in[1];
            }
            if (params->has_input_vec3f("value")) {
                auto in = zeno::toVec3f(params->get_input2_vec3f("value"));
                number[0] = in[0];
                number[1] = in[1];
                number[2] = in[2];
            }
            if (params->has_input_vec4f("value")) {
                auto in = zeno::toVec4f(params->get_input2_vec4f("value"));
                number[0] = in[0];
                number[1] = in[1];
                number[2] = in[2];
                number[3] = in[3];
            }

            if (type == "float" || type == "R")
                em->emitCode(zeno::format("{}", number[0]));
            else if (type == "G")
                em->emitCode(zeno::format("{}", number[1]));
            else if (type == "B")
                em->emitCode(zeno::format("{}", number[2]));
            else if (type == "A")
                em->emitCode(zeno::format("{}", number[3]));
            else if (type == "vec2")
                em->emitCode(
                    zeno::format("vec2({},{})", number[0], number[1]));
            else if (type == "vec3")
                em->emitCode(zeno::format("vec3({},{},{})", number[0],
                              number[1], number[2]));
            else if (type == "vec4")
                em->emitCode(zeno::format("vec4({},{},{},{})", number[0],
                              number[1], number[2], number[3]));
            else
                throw zeno::Exception("ShaderTexture2D got bad type: " + type);

            return;
        }

        tex->path = texture_path;
        tex->blockCompression = params->get_input2_bool("blockCompression");

#define SET_TEX_WRAP(TEX, WRAP)                                                \
    if (WRAP == "REPEAT")                                                     \
        TEX->WRAP = Texture2DObject::TexWrapEnum::REPEAT;                     \
    else if (WRAP == "MIRRORED_REPEAT")                                       \
        TEX->WRAP = Texture2DObject::TexWrapEnum::MIRRORED_REPEAT;            \
    else if (WRAP == "CLAMP_TO_EDGE")                                         \
        TEX->WRAP = Texture2DObject::TexWrapEnum::CLAMP_TO_EDGE;              \
    else if (WRAP == "CLAMP_TO_BORDER")                                       \
        TEX->WRAP = Texture2DObject::TexWrapEnum::CLAMP_TO_BORDER;            \
    else                                                                      \
        throw zeno::Exception(std::string(#WRAP) + WRAP);

        auto wrapS = params->get_input2_string("wrapS");
        SET_TEX_WRAP(tex, wrapS)
        auto wrapT = params->get_input2_string("wrapT");
        SET_TEX_WRAP(tex, wrapT)

#undef SET_TEX_WRAP

#define SET_TEX_FILTER(TEX, FILTER)                                           \
    if (FILTER == "NEAREST")                                                  \
        TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST;                \
    else if (FILTER == "LINEAR")                                              \
        TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR;                 \
    else if (FILTER == "NEAREST_MIPMAP_NEAREST")                              \
        TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST_MIPMAP_NEAREST; \
    else if (FILTER == "LINEAR_MIPMAP_NEAREST")                               \
        TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR_MIPMAP_NEAREST;  \
    else if (FILTER == "NEAREST_MIPMAP_LINEAR")                               \
        TEX->FILTER = Texture2DObject::TexFilterEnum::NEAREST_MIPMAP_LINEAR;  \
    else if (FILTER == "LINEAR_MIPMAP_LINEAR")                                \
        TEX->FILTER = Texture2DObject::TexFilterEnum::LINEAR_MIPMAP_LINEAR;   \
    else                                                                      \
        throw zeno::Exception(std::string(#FILTER) + FILTER);

        auto minFilter = params->get_input2_string("minFilter");
        SET_TEX_FILTER(tex, minFilter)
        auto magFilter = params->get_input2_string("magFilter");
        SET_TEX_FILTER(tex, magFilter)

#undef SET_TEX_FILTER

        em->tex2Ds.push_back(std::move(tex));
        auto type = params->get_input2_string("type");
        std::string suffix;
        if (type == "R") {
            suffix = ".x";
            type = "";
        } else if (type == "G") {
            suffix = ".y";
            type = "";
        } else if (type == "B") {
            suffix = ".z";
            type = "";
        } else if (type == "A") {
            suffix = ".w";
            type = "";
        }
        auto uvtiling =
            em->determineExpr(params->get_input_shader("uvtiling"));
        auto nscale =
            em->determineExpr(params->get_input_shader("normalScale"));
        auto hscale =
            em->determineExpr(params->get_input_shader("heightScale"));
        std::string coord = "att_uv";
        if (params->has_input("coord")) {
            coord = em->determineExpr(params->get_input_shader("coord"));
        }
        auto postprocess = params->get_input2_string("post_process");

        if (type == "float" && postprocess == "1-x") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format(
                    "1.0f - texture2D<float,float>(zenotex[{}], saturate( "
                    "vec2({}) * {}) )",
                    texId, coord, uvtiling));
            } else {
                em->emitCode(zeno::format(
                    "1.0f - texture2D<float,float>(zenotex[{}], vec2({}) * {} )",
                    texId, coord, uvtiling));
            }
            return;
        }
        if (postprocess == "raw") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format(
                    "{}(texture2D(zenotex[{}], saturate( vec2({}) * {}) )){}",
                    type, texId, coord, uvtiling, suffix));
            } else {
                em->emitCode(zeno::format(
                    "{}(texture2D(zenotex[{}], vec2({}) * {})){}", type,
                    texId, coord, uvtiling, suffix));
            }
        } else if (postprocess == "srgb") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format(
                    "pow({}(texture2D(zenotex[{}], saturate(vec2({}) * "
                    "{}))),2.2f){}",
                    type, texId, coord, uvtiling, suffix));
            } else {
                em->emitCode(zeno::format(
                    "pow({}(texture2D(zenotex[{}], vec2({}) * {})),2.2f){}",
                    type, texId, coord, uvtiling, suffix));
            }
        } else if (postprocess == "normal_map") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format(
                    "normalize({}(texture2D(zenotex[{}], saturate(vec2({}) * "
                    "{}))) * vec3({},{},1.0) - vec3(0.5*{},0.5*{},0.0)){}",
                    type, texId, coord, uvtiling, nscale, nscale, nscale,
                    nscale, suffix));
            } else {
                em->emitCode(zeno::format(
                    "normalize({}(texture2D(zenotex[{}], vec2({}) * {})) * "
                    "vec3({},{},1.0) - vec3(0.5*{},0.5*{},0.0)){}",
                    type, texId, coord, uvtiling, nscale, nscale, nscale,
                    nscale, suffix));
            }
        } else if (postprocess == "1-x") {
            if (wrapS == "CLAMP_TO_EDGE") {
                em->emitCode(zeno::format(
                    "{}(1.0) - {}(texture2D(zenotex[{}], saturate(vec2({}) * "
                    "{}))){}",
                    type, type, texId, coord, uvtiling, suffix));
            } else {
                em->emitCode(zeno::format(
                    "{}(1.0) - {}(texture2D(zenotex[{}], vec2({}) * {})){}",
                    type, type, texId, coord, uvtiling, suffix));
            }
        } else if (postprocess == "displacement") {
            em->emitCode(zeno::format(
                "{}(parallaxCall(*(TriangleInput*)&attrs, zenotex[{}], "
                "vec2({}), {}, {})){}",
                type, texId, coord, uvtiling, hscale, suffix));
        }
    }
};

ZENDEFNODE(SmartTexture2D, {
    {
        {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::ReadPathEdit},
        {gParamType_Heatmap, "heatmap"},
        {(std::string) "enum " + SmartTexture2D::texWrapping, "wrapS", "REPEAT"},
        {(std::string) "enum " + SmartTexture2D::texWrapping, "wrapT", "REPEAT"},
        {(std::string) "enum " + SmartTexture2D::texFiltering, "minFilter",
         "LINEAR"},
        {(std::string) "enum " + SmartTexture2D::texFiltering, "magFilter",
         "LINEAR"},
        {gParamType_Vec2f, "coord"},
        {gParamType_Vec2f, "uvtiling", "1,1"},
        {gParamType_Vec4f, "value", "0,0,0,0"},
        {gParamType_Float, "normalScale", "1.0"},
        {gParamType_Vec4f, "heightScale", "1.0,1.0,0.0,1.0"},
        {"enum float vec2 vec3 vec4 R G B A", "type", "vec3"},
        {"enum raw srgb normal_map 1-x displacement", "post_process", "raw"},
        {gParamType_Bool, "blockCompression", "false"}
    },
    {
        {gParamType_Shader, "out"},
    },
    {},
    {
        "shader",
    },
});

} // namespace zeno
