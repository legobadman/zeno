#include <zeno/zeno.h>
#include <zeno/utils/logger.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/types/DummyObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/safe_at.h>
#include <zeno/core/Graph.h>

namespace zeno {

#if 0
struct PortalIn : zeno::INode {
    virtual void complete() override {
        auto name = ZImpl(get_param<std::string>("name"));
        std::shared_ptr<Graph> spGraph = getThisGraph();
        assert(spGraph);
        spGraph->portalIns[name] = this->get_name();
    }

    virtual void apply() override {
        auto name = ZImpl(get_param<std::string>("name"));
        auto obj = ZImpl(get_input("port"));
        std::shared_ptr<Graph> spGraph = getThisGraph();
        assert(spGraph);
        spGraph->portals[name] = std::move(obj);
    }
};

ZENDEFNODE(PortalIn, {
    {{gParamType_IObject, "port", "", zeno::Socket_ReadOnly}},
    {},
    {{gParamType_String, "name", "RenameMe!"}},
    {"layout"},
});

struct PortalOut : zeno::INode {
    virtual void apply() override {
        auto name = ZImpl(get_param<std::string>("name"));
        std::shared_ptr<Graph> spGraph = getThisGraph();
        assert(spGraph);
        auto depnode = zeno::safe_at(spGraph->portalIns, name, "PortalIn");
        spGraph->applyNode(depnode);
        auto obj = zeno::safe_at(spGraph->portals, name, "portal object");
        ZImpl(set_output("port", std::move(obj)));
    }
};

ZENDEFNODE(PortalOut, {
    {},
    {{gParamType_IObject, "port"}},
    {{gParamType_String, "name", "RenameMe!"}},
    {"layout"},
});
#endif


struct Route : zeno::INode {
    virtual void apply() override {
        if (ZImpl(has_input("input"))) {
            auto obj = ZImpl(get_input("input"));
            ZImpl(set_output("output", std::move(obj)));
        } else {
            ZImpl(set_output("output", std::make_shared<zeno::DummyObject>()));
        }
    }
    CustomUI export_customui() const override {
        CustomUI ui = INode::export_customui();
        ui.uistyle.background = "#99B2C4";
        return ui;
    }
};

ZENDEFNODE(Route, {
    {
        {gParamType_IObject, "input", "", zeno::Socket_ReadOnly},
        {gParamType_Float, "float_val", "0"},
        {gParamType_Int, "int_val", "0"},
        {gParamType_String, "string_val", "0"},
        {gParamType_Vec3f, "vec3f_val", "0,0,0"}
    },
    {{gParamType_IObject, "output"}},
    {},
    {"layout"},
});


struct Stamp : zeno::INode {
    virtual void apply() override {
        if (ZImpl(has_input("input"))) {
            auto obj = ZImpl(get_input("input"));
            ZImpl(set_output("output", std::move(obj)));
        }
        else {
            ZImpl(set_output("output", std::make_shared<zeno::DummyObject>()));
        }
    }
};

ZENDEFNODE(Stamp, {
    {{gParamType_IObject, "input"}},
    {{gParamType_IObject, "output"}},
    {{"enum UnChanged DataChange ShapeChange TotalChange", "mode", "UnChanged"},
     {gParamType_String, "name", ""}},
    {"lifecycle"}
});


struct Clone : zeno::INode {
    virtual void apply() override {
        auto obj = ZImpl(get_input("object"));
        auto newobj = obj->clone();
        if (!newobj) {
            log_error("requested object doesn't support clone");
            return;
        }
        ZImpl(set_output("newObject", std::move(newobj)));
        ZImpl(set_output("origin", obj));
    }
};

ZENDEFNODE(Clone, {
    {{gParamType_IObject, "object", "", zeno::Socket_ReadOnly}},
    {
        {"object", "newObject"},
        {"object", "origin"},
    },
    {},
    {"lifecycle"},
});


struct Assign : zeno::INode {
    virtual void apply() override {
        auto src = ZImpl(get_input("src"));
        auto dst = ZImpl(get_input("dst"));
        *dst = *src;
        ZImpl(set_output("dst", std::move(dst)));
    }
};

ZENDEFNODE(Assign, {
    {{"object", "dst"}, {"object", "src"} },
    {{"object", "dst"}},
    {},
    {"lifecycle"},
});

struct SetUserData : zeno::INode {
    virtual void apply() override {
        auto object = ZImpl(get_input("object"));
        auto key = ZImpl(get_param<std::string>("key"));
        UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
        pUsrData->set(key, ZImpl(get_input("data")));
        ZImpl(set_output("object", std::move(object)));
    }
};

ZENDEFNODE(SetUserData, {
    {
        {gParamType_IObject, "object", "", zeno::Socket_ReadOnly},
        {gParamType_IObject, "data", "", zeno::Socket_ReadOnly},
    },
    {{gParamType_IObject, "object"}},
    {{gParamType_String, "key", ""}},
    {"deprecated"},
});

struct SetUserData2 : zeno::INode {
    virtual void apply() override {
        auto object = ZImpl(get_input("object"));
        auto key = ZImpl(get_input2<std::string>("key"));
        UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
        pUsrData->set(key, ZImpl(get_input("data")));
        ZImpl(set_output("object", std::move(object)));
    }
};

ZENDEFNODE(SetUserData2, {
    {{gParamType_IObject, "object"}, {gParamType_String, "key", ""}, {gParamType_String,"data",""}},
    {{gParamType_IObject, "object"}},
    {},
    {"lifecycle"},
});

struct GetUserData : zeno::INode {
    virtual void apply() override {
        auto object = ZImpl(get_input("object"));
        auto key = ZImpl(get_param<std::string>("key"));
        UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
        auto hasValue = pUsrData->has(key);
        auto data = hasValue ? pUsrData->get(key) : std::make_shared<DummyObject>();
        ZImpl(set_output2("hasValue", hasValue));
        ZImpl(set_output("data", std::move(data)));
    }
};

ZENDEFNODE(GetUserData, {
    {{gParamType_IObject, "object", "", zeno::Socket_ReadOnly}},
    {{gParamType_IObject, "data"}, {gParamType_Bool, "hasValue"}},
    {{gParamType_String, "key", ""}},
    {"deprecated"},
});

struct GetUserData2 : zeno::INode {
  virtual void apply() override {
    auto object = ZImpl(get_input("object"));
    auto key = ZImpl(get_input2<std::string>("key"));
    UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
    auto hasValue = pUsrData->has(key);
    auto data = hasValue ? pUsrData->get(key) : std::make_shared<DummyObject>();
    ZImpl(set_output2("hasValue", hasValue));
    ZImpl(set_output("data", std::move(data)));
  }
};

ZENDEFNODE(GetUserData2, {
                            {{gParamType_IObject, "object", "", zeno::Socket_ReadOnly},
                             {gParamType_String, "key", ""}},
                            {{gParamType_IObject, "data"}, {gParamType_Bool, "hasValue"}},
                            {},
                            {"lifecycle"},
                        });


struct DelUserData : zeno::INode {
    virtual void apply() override {
        auto object = ZImpl(get_input("object"));
        auto key = ZImpl(get_param<std::string>("key"));
        UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
        pUsrData->del(key);
    }
};

ZENDEFNODE(DelUserData, {
    {{gParamType_IObject, "object", "", zeno::Socket_ReadOnly}},
    {},
    {{gParamType_String, "key", ""}},
    {"deprecated"},
});

struct DelUserData2 : zeno::INode {
    virtual void apply() override {
        auto object = ZImpl(get_input("object"));
        auto key = ZImpl(get_input2<std::string>("key"));
        UserData* pUsrData = dynamic_cast<UserData*>(object->userData());
        pUsrData->del(key);
        ZImpl(set_output("object", std::move(object)));
    }
};

ZENDEFNODE(DelUserData2, {
    {{gParamType_String, "key", ""}, {gParamType_IObject, "object", "", zeno::Socket_ReadOnly}},
    {{gParamType_IObject, "object"}},
    {},
    {"lifecycle"},
});

#if 0
struct CopyAllUserData : zeno::INode {
    virtual void apply() override {
        auto src = ZImpl(get_input("src"));
        auto dst = ZImpl(get_input("dst"));
        dst->userData() = src->userData();
        ZImpl(set_output("dst", std::move(dst)));
    }
};

ZENDEFNODE(CopyAllUserData, {
    {
        {gParamType_IObject,  "dst", "", zeno::Socket_ReadOnly},
        {gParamType_IObject,  "src", "", zeno::Socket_ReadOnly},
    },
    {{gParamType_IObject, "dst"}},
    {},
    {"lifecycle"},
});
#endif

}
