#include <zeno/zeno.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/utils/logger.h>
#include <cstdio>
#include <thread>


namespace {

struct PrintMessage : zeno::INode {
    virtual void apply() override {
        auto message = ZImpl(get_param<std::string>("message"));
        printf("%s\n", message.c_str());
    }
};

ZENDEFNODE(PrintMessage, {
    {},
    {},
    {{gParamType_String, "message", "hello-stdout"}},
    {"debug"},
});


struct PrintMessageStdErr : zeno::INode {
    virtual void apply() override {
        auto message = ZImpl(get_param<std::string>("message"));
        fprintf(stderr, "%s\n", message.c_str());
    }
};

ZENDEFNODE(PrintMessageStdErr, {
    {},
    {},
    {{gParamType_String, "message", "hello-stderr"}},
    {"debug"},
});


struct InfiniteLoop : zeno::INode {
    virtual void apply() override {
        while (true) {
            int j;
            j = 0;
        }
    }
};

ZENDEFNODE(InfiniteLoop, {
    {{gParamType_Int, "input", "2"}},
    {{gParamType_Int, "output"}},
    {},
    {"debug"}
});


struct TriggerExitProcess : zeno::INode {
    virtual void apply() override {
        int status = ZImpl(get_param<int>("status"));
        exit(status);
    }
};

ZENDEFNODE(TriggerExitProcess, {
    {},
    {},
    {{gParamType_Int, "status", "-1"}},
    {"debug"},
});


struct TriggerSegFault : zeno::INode {
    virtual void apply() override {
        *(volatile float *)nullptr = 0;
    }
};

ZENDEFNODE(TriggerSegFault, {
    {},
    {},
    {},
    {"debug"},
});


struct TriggerDivideZero : zeno::INode {
    virtual void apply() override {
        volatile int x = 0;
        x = x / x;
    }
};

ZENDEFNODE(TriggerDivideZero, {
    {},
    {},
    {},
    {"debug"},
});


struct TriggerAbortSignal : zeno::INode {
    virtual void apply() override {
        abort();
    }
};

ZENDEFNODE(TriggerAbortSignal, {
    {},
    {},
    {},
    {"debug"},
});



struct SpdlogInfoMessage : zeno::INode {
    virtual void apply() override {
        zeno::log_info("{}", ZImpl(get_param<std::string>("message")));
    }
};

ZENDEFNODE(SpdlogInfoMessage, {
    {},
    {},
    {{gParamType_String, "message", "hello from spdlog!"}},
    {"debug"},
});


struct SpdlogErrorMessage : zeno::INode {
    virtual void apply() override {
        zeno::log_error("{}", ZImpl(get_param<std::string>("message")));
    }
};

ZENDEFNODE(SpdlogErrorMessage, {
    {},
    {},
    {{gParamType_String, "message", "error from spdlog!"}},
    {"debug"},
});


struct TriggerException : zeno::INode {
    virtual void apply() override {
        throw zeno::Exception(ZImpl(get_param<std::string>("message")));
    }
};

ZENDEFNODE(TriggerException, {
    {},
    {},
    {{gParamType_String, "message", "exception occurred!"}},
    {"debug"},
});

struct TriggerViewportFault : zeno::INode {
    virtual void apply() override {
        auto prim = std::make_shared<zeno::PrimitiveObject>();
        prim->tris.resize(1);
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(TriggerViewportFault, {
    {},
    {{gParamType_Primitive, "prim"}},
    {},
    {"debug"},
});


struct MockRunning : zeno::INode {
    virtual void apply() override {
        int secs = ZImpl(get_input<zeno::NumericObject>("wait seconds"))->get<int>();
        std::this_thread::sleep_for(std::chrono::seconds(secs));
    }
};

ZENDEFNODE(MockRunning, {
    {{gParamType_List, "SRC"},
     {gParamType_Int, "wait seconds", "1", zeno::NoSocket, zeno::Lineedit}
    },
    {{gParamType_IObject, "DST"}},
    {},
    {"debug"},
});


struct Blackboard : zeno::INode {
    virtual void apply() override {
    }
};

ZENDEFNODE(Blackboard, {
    {},
    {},
    {},
    {"layout"},
});

struct Group : zeno::INode {
    virtual void apply() override {
    }
};

ZENDEFNODE(Group, {
    {{gParamType_String, "title", "title"},{gParamType_String, "items"},{gParamType_Vec3f, "background", "0, 0.39, 0.66"},{gParamType_Vec2f, "size", "500,500"}},
    {},
    {},
    {"layout"},
    });

#if 0
struct CustomNode : zeno::INode {
    virtual void apply() override {

    }
};

ZENDEFINE(CustomNode, {
    {
        {"obj_intput1", Param_Null, zeno::Socket_ReadOnly},
    },
    {
        {
            {
                "Default",
                {
                    {
                        "Group1",
                        {
                            {"param1", Param_Null, zeno::Socket_ReadOnly},
                            {"param2", gParamType_Primitive, zeno::Socket_ReadOnly},
                            {"param3", zeno::types::gParamType_Int,  zeno::NoSocket, 2, zeno::Lineedit, {}}
                        }
                    },
                    {
                        "Group2",
                        {
                            {"param4", zeno::types::gParamType_String, zeno::Socket_ReadOnly, "", zeno::Multiline, {}},
                            {"param5", gParamType_Primitive, zeno::Socket_ReadOnly},
                            {"param6", Param_Null, zeno::NoSocket}
                        }
                    }
                }
            },
            {
                "Default2",
                {
                    {
                        "Group3",
                        {
                            {"param7", Param_Null, zeno::Socket_ReadOnly},
                            {"param8", gParamType_Primitive, zeno::Socket_ReadOnly},
                            {"param9", Param_Null, zeno::NoSocket}
                        }
                    },
                    {
                        "Group4",
                        {
                            {"param10", Param_Null, zeno::Socket_ReadOnly},
                            {"param11", gParamType_Primitive, zeno::Socket_ReadOnly},
                            {"param12", Param_Null, zeno::NoSocket}
                        }
                    }
                }
            },
        }
    },
    {
        {"prim_output1", Param_Null, zeno::Socket_ReadOnly},
    },
    {
        {"obj_output1", Param_Null, zeno::Socket_ReadOnly},
    },
    "debug",
    "CUI",
});
#endif

}
