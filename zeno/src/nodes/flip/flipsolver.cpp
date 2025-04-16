#include <rapidjson/document.h>
#include <zeno/zeno.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/funcs/ObjectCodec.h>
#include <zeno/geo/geometryutil.h>
#include <zeno/utils/log.h>
#include <zeno/utils/uuid.h>
#include "glm/gtc/matrix_transform.hpp"
#if defined(_WIN32)
#include <Windows.h>
#else

#endif

#undef GetObject    //winapi居然把rapidjson的GetObject方法给宏定义了


namespace zeno {
#if 0 //TODO
    using namespace zeno::reflect;

    #define DEBUG_NODE

    struct ZDEFNODE() FlipSolver : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"init_fluid", ParamObject("Initialize Fluid")},
                {"static_collider", ParamObject("Static Collider")},
                {"emission_source", ParamObject("Emission Source")},

                {"accuracy", ParamPrimitive("Accuracy")},
                {"max_substep", ParamPrimitive("Max Substep")},
                {"timestep", ParamPrimitive("Timestep")},
                {"gravity", ParamPrimitive("Gravity")},
                {"emission", ParamPrimitive("Emission Velocity")},
                {"is_emission", ParamPrimitive("Is Emission")},
                {"dynamic_collide_strength", ParamPrimitive("Dynamic Collide Strength")},
                {"density", ParamPrimitive("Density")},
                {"surface_tension", ParamPrimitive("Surface Tension")},
                {"viscosity", ParamPrimitive("Viscosity")},
                {"wall_viscosity", ParamPrimitive("Wall Viscosity")},
                {"wall_viscosityRange", ParamPrimitive("Wall Viscosity Range")},
                {"curve_endframe", ParamPrimitive("Curve Endframe")},
                {"curve_range", ParamPrimitive("Curve Range")},
                {"preview_size", ParamPrimitive("Preview Size")},
                {"preview_minVelocity", ParamPrimitive("Preview Minimum Velocity")},
                {"preview_maxVelocity", ParamPrimitive("Preview Maximum Velocity")},

                {"cache_path", ParamPrimitive("Cache Path")}
            },
            //输出：
            _Group {
                {"", ParamObject("Output")},
            },
            //数值参数布局：
            CustomUIParams {
                ParamTab {
                    "Solver",
                    {
                        ParamGroup {
                            "Group1",
                            {
                                ParamPrimitive("Accuracy"),
                                ParamPrimitive("Max Substep"),
                                ParamPrimitive("Timestep"),
                                ParamPrimitive("Gravity"),
                                ParamPrimitive("Emission Velocity"),
                                ParamPrimitive("Is Emission"),
                                ParamPrimitive("Dynamic Collide Strength"),
                                ParamPrimitive("Density"),
                                ParamPrimitive("Surface Tension"),
                                ParamPrimitive("Viscosity"),
                                ParamPrimitive("Wall Viscosity"),
                                ParamPrimitive("Wall Viscosity Range"),
                                ParamPrimitive("Curve Endframe"),
                                ParamPrimitive("Curve Range"),
                                ParamPrimitive("Preview Size"),
                                ParamPrimitive("Preview Minimum Velocity"),
                                ParamPrimitive("Preview Maximum Velocity")
                            }
                        }
                    }
                },
                ParamTab {
                    "Cache",
                    {
                        ParamGroup {
                            "Group1",
                            {
                                ParamPrimitive("Cache Path")
                            }
                        }
                    }
                },
            }
        };

        FlipSolver() {
#if defined(_WIN32)
            initHandles();
#else
#endif
        }

        void initHandles() {
#if defined(_WIN32)
            ZeroMemory(&m_pi, sizeof(m_pi));
            CloseHandle(m_shm_initFluid);  //初始流体共享内存文件句柄
            CloseHandle(m_shm_staticCollider);
            CloseHandle(m_hPipe_solver_read);
            CloseHandle(m_hPipe_main_write);
            CloseHandle(m_hPipe_main_read);
            CloseHandle(m_hPipe_sovler_write);
            m_shm_initFluid = INVALID_HANDLE_VALUE;
            m_shm_staticCollider = INVALID_HANDLE_VALUE;
            m_hPipe_solver_read = INVALID_HANDLE_VALUE;
            m_hPipe_main_write = INVALID_HANDLE_VALUE;
            m_hPipe_main_read = INVALID_HANDLE_VALUE;
            m_hPipe_sovler_write = INVALID_HANDLE_VALUE;
#else
#endif
        }

        std::shared_ptr<IObject> checkCache(std::string cache_path, int frame) {
            //checkResult的作用是：无论解算器处在什么样的状态，检查当前全局的时间帧，如果cache_path有缓存且合法的cache，
            //就作为当前节点的输出。

            //case1: 有可能这是第一次运行，cache_path已经缓存了zencache，但由于重新打开工程要标脏，所以
            //即便上一次缓存了，参数一致，也得全部删掉（至于交给谁清理后续讨论）

            //case2: 已经开始运行，但只是运行了一部分


            //case3: 已经运行结束了（solver进程已经退出）,此时路径下有cache

            return nullptr;
        }

        void mark_dirty(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively) {
            INode::mark_dirty(bOn, reason, bWholeSubnet, bRecursively);
            if (reason != Dirty_FrameChanged) {
                //清空所有cache.
            }
        }

        std::shared_ptr<zeno::IObject> apply(
            std::shared_ptr<zeno::IObject> init_fluid,
            std::shared_ptr<zeno::IObject> static_collider,
            std::shared_ptr<zeno::IObject> emission_source,
            float accuracy = 0.08f,
            float timestep = 0.04f,
            float max_substep = 1.f,
            zeno::vec3f gravity = zeno::vec3f({ 0.f, -9.8f, 0.f }),
            zeno::vec3f emission = zeno::vec3f({ 0.f, -9.8f, 0.f }),
            bool is_emission = true,
            float dynamic_collide_strength = 1.f,
            float density = 1000,
            float surface_tension = 0,
            float viscosity = 0,
            float wall_viscosity = 0,
            float wall_viscosityRange = 0,
            int curve_endframe = 100,
            float curve_range = 1.1f,
            float preview_size = 0,
            float preview_minVelocity = 0,
            float preview_maxVelocity = 2.f,
            std::string cache_path = ""
        ) {
            if (!init_fluid || !static_collider) {
                throw makeError<UnimplError>("need init fluid and static collider");
            }
            //检测cache路径是否合理，如果不合理，直接退出
            int frame = zeno::getSession().globalState->getFrameId();
            auto spRes = checkCache(cache_path, frame);
            if (!spRes)
            {
                //在检测不到cache，而且solver进程不存在，此时就认为需要启动solver进程去解算，有三种可能的情况：
                //1.从来没有运行解算器
                //2.之前运行了一次，后续标脏了，因为参数类型的标脏，必须要把cache全部删掉（暂时不考虑备份，为了程序方便）
                //3.只是跑了一部分的帧，其他范围的帧要重新跑一次
#if defined(_WIN32)
                //检测m_pi.hProcess是否存在
                DWORD exitCode;
                GetExitCodeProcess(m_pi.hProcess, &exitCode);
                if (exitCode != STILL_ACTIVE) {
                    CloseHandle(m_pi.hProcess);
                    initHandles();
                }

                if (!m_pi.hProcess) {
                    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
                    // 创建管道
                    if (!CreatePipe(&m_hPipe_main_read, &m_hPipe_sovler_write, &sa, 0) ||
                        !CreatePipe(&m_hPipe_solver_read, &m_hPipe_main_write, &sa, 0)) {
                        //throw
                        zeno::log_error("CreatePipe failed");
                        return nullptr;
                    }

                    //配置子进程 I/O 句柄
                    STARTUPINFO si = { sizeof(STARTUPINFO) };
                    /* solver进程有大量的标准输出作为调试日志，故不采用标准输出作为两个进程间的管道
                    si.dwFlags = STARTF_USESTDHANDLES;
                    si.hStdInput = m_hPipeInRead;
                    si.hStdOutput = m_hPipeOutWrite;
                    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
                    */

                    std::vector<char> buf_initfluid, buf_static_collider;
                    bool bConvert = encodeObject(init_fluid.get(), buf_initfluid);
                    if (!bConvert) {
                        throw makeError<UnimplError>("init fluid encoding failed");
                    }
                    bConvert = encodeObject(static_collider.get(), buf_static_collider);
                    if (!bConvert) {
                        throw makeError<UnimplError>("static collider encoding failed");
                    }

                    auto shm_name = generateUUID();
                    auto shm_initname = shm_name + "/" + "init_fluid";
                    const int shm_initfluid_size = buf_initfluid.size();
                    m_shm_initFluid = CreateFileMapping(
                        INVALID_HANDLE_VALUE,
                        NULL,
                        PAGE_READWRITE,
                        0, shm_initfluid_size,
                        shm_initname.c_str()
                    );
                    if (m_shm_initFluid == NULL) {
                        zeno::log_error("CreateFileMapping for init fluid failed, error: {}", GetLastError());
                        return nullptr;
                    }

                    const size_t shm_staticcoll_size = buf_static_collider.size();
                    auto shm_staiccoll_name = shm_name + "/" + "static_coll";
                    m_shm_staticCollider = CreateFileMapping(
                        INVALID_HANDLE_VALUE,
                        NULL,
                        PAGE_READWRITE,
                        0, shm_staticcoll_size,
                        shm_staiccoll_name.c_str()
                    );
                    if (m_shm_staticCollider == NULL) {
                        zeno::log_error("CreateFileMapping for static collider failed, error: {}", GetLastError());
                        return nullptr;
                    }

                    LPVOID pInitBuf = MapViewOfFile(m_shm_initFluid, FILE_MAP_ALL_ACCESS, 0, 0, shm_initfluid_size);
                    if (!pInitBuf) {
                        zeno::log_error("MapViewOfFile failed, error: {}", GetLastError());
                        CloseHandle(m_shm_initFluid);
                    }
                    //写入
                    memcpy(pInitBuf, buf_initfluid.data(), shm_initfluid_size);

                    LPVOID pStaticCollBuf = MapViewOfFile(m_shm_staticCollider, FILE_MAP_ALL_ACCESS, 0, 0, shm_staticcoll_size);
                    if (!pStaticCollBuf) {
                        zeno::log_error("MapViewOfFile failed, error: {}", GetLastError());
                        CloseHandle(m_shm_staticCollider);
                    }
                    //写入
                    memcpy(pStaticCollBuf, buf_static_collider.data(), shm_staticcoll_size);

#ifdef DEBUG_NODE
                    auto cmdargs = zeno::format("C:/zeno-master/Debug/bin/zensolver.exe --pipe-write {} --pipe-read {} --init-fluid \"{}\" --size-init-fluid \"{}\" --static-collider \"{}\" --size-static-collider \"{}\"",
                        (unsigned long long)m_hPipe_sovler_write, (unsigned long long)m_hPipe_solver_read, shm_initname, shm_initfluid_size, shm_staiccoll_name, shm_staticcoll_size);
                    if (!CreateProcess((LPSTR)"C:/zeno-master/Debug/bin/zensolver.exe", (LPSTR)cmdargs.c_str(), NULL, NULL, TRUE, DETACHED_PROCESS, NULL, NULL, &si, &m_pi)) {
                        zeno::log_error("CreateProcess failed");
                        return nullptr;
                    }

                    //从解算进程读取发来的通知
                    std::thread thdReadPipe(&FlipSolver::ReadPipeThread, this, m_hPipe_main_read);
                    thdReadPipe.detach();
#else
                    auto cmdargs = zeno::format("C:/zeno-master/out/build/x64-Release/bin/zensolver.exe --init-fluid \"{}\" --size-init-fluid \"{}\" --static-collider \"{}\" --size-static-collider \"{}\"",
                        m_hPipe_sovler_write, m_hPipe_solver_read, shm_initname, shm_initfluid_size, shm_staiccoll_name, shm_staticcoll_size);
                    // 创建子进程
                    if (!CreateProcess((LPSTR)"C:/zeno-master/out/build/x64-Release/bin/zensolver.exe", (LPSTR)cmdargs.c_str(), NULL, NULL, TRUE, DETACHED_PROCESS, NULL, NULL, &si, &m_pi)) {
                        zeno::log_error("CreateProcess failed");
                        return nullptr;
                    }
#endif
                }
                else {
                    //解算进程已经在运行，但目前当前帧的cache还没结算完，只能返回空（不过返回空，也会使得程序清除脏位）
                    //可以等到子进程回传消息时，再标脏位，从而重新apply
                    //A: 如果用户主动杀进程，而这个句柄还在，会发生什么？

                    return nullptr;
                }
#else
                //TODO: Linux下IPC
#endif

            }
            else {
                //也有一种可能，就是有cache，但解算器只是跑了一部分帧，剩下的需要用户主动触发时间轴更新，然后重新跑
                return spRes;
            }
            return nullptr;
        }

        void ReadPipeThread(HANDLE hReadPipe) {
            DWORD bytesRead;
            char buf[1024];
            while (ReadFile(hReadPipe, &buf, 1024, &bytesRead, NULL) && bytesRead > 0) {
                rapidjson::Document doc;
                doc.Parse(buf, bytesRead);
                if (!doc.IsObject()) {
                    continue;
                }
                if (!doc.HasMember("action")) {
                    continue;
                }
                const rapidjson::Value& val = doc["action"];
                std::string act = val.GetString();
                if (act == "finishFrame") {
                    if (doc.HasMember("key")) {
                        int frameComplete = std::stoi(doc["key"].GetString());
                        zeno::log_info("frame {} from solver completes", frameComplete);
                    }
                }
                else {
                    //failed.
                }
            }
        }

        CustomUI export_customui() const override {
            CustomUI ui = INode::export_customui();
            ui.uistyle.background = "#246283";
            return ui;
        }

private:
#if defined(_WIN32)
        HANDLE m_shm_initFluid = INVALID_HANDLE_VALUE;  //初始流体共享内存文件句柄
        HANDLE m_shm_staticCollider = INVALID_HANDLE_VALUE;
        HANDLE m_hPipe_solver_read = INVALID_HANDLE_VALUE;
        HANDLE m_hPipe_main_write = INVALID_HANDLE_VALUE;
        HANDLE m_hPipe_main_read = INVALID_HANDLE_VALUE;
        HANDLE m_hPipe_sovler_write = INVALID_HANDLE_VALUE;
        PROCESS_INFORMATION m_pi;
#else
        //TODO: linux pipe
#endif

    };
#endif
}