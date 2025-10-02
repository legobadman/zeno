#include <zeno/zeno.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/ConditionObject.h>
#include <zeno/extra/evaluate_condition.h>
#include <zeno/core/Graph.h>


namespace zeno {

#ifdef ENABLE_LEGACY_ZENO_NODE

struct CachedByKey : zeno::INode {
    std::map<std::string, std::shared_ptr<IObject>> cache;

    virtual void preApply() override {
        requireInput("key");
        auto key = ZImpl(get_input<zeno::StringObject>("key")->get();
        if (auto it = cache.find(key); it != cache.end()) {
            auto value = it->second;
            ZImpl(set_output("output", std::move(value));
        } else {
            requireInput("input");
            auto value = ZImpl(clone_input("input");
            cache[key] = value;
            ZImpl(set_output("output", std::move(value));
        }
    }

    virtual void apply() override {}
};

ZENDEFNODE(CachedByKey, {
    {"input", "key"},
    {"output"},
    {},
    {"control"},
});


struct CachedIf : zeno::INode {
    bool m_done = false;

    virtual void preApply() override {
        if (ZImpl(has_input("keepCache")) {
            requireInput("keepCache");
            bool keep = evaluate_condition(ZImpl(clone_input("keepCache").get());
            if (!keep) {
                m_done = false;
            }
        }
        if (!m_done) {
            INode::preApply();
            m_done = true;
        }
    }

    virtual void apply() override {
        auto ptr = ZImpl(clone_input("input");
        ZImpl(set_output("output", std::move(ptr));
    }
};

ZENDEFNODE(CachedIf, {
    {"input", "keepCache"},
    {"output"},
    {},
    {"control"},
});


struct CachedOnce : zeno::INode {
    bool m_done = false;

    virtual void preApply() override {
        if (!m_done) {
            INode::preApply();
            m_done = true;
        }
    }

    virtual void apply() override {
        auto ptr = ZImpl(clone_input("input");
        ZImpl(set_output("output", std::move(ptr));
    }
};

ZENDEFNODE(CachedOnce, {
    {"input"},
    {"output"},
    {},
    {"control"},
});

struct CacheLastFrameBegin : zeno::INode {
    std::shared_ptr<IObject> m_lastFrameCache = nullptr;

    virtual void apply() override { 
        if (m_lastFrameCache == nullptr) {
            m_lastFrameCache = (*ZImpl(clone_input("input")).clone();            
        }         
        ZImpl(set_output("lastFrame", std::move(m_lastFrameCache));
        ZImpl(set_output("linkFrom", std::make_shared<zeno::IObject>());
    }
};


ZENO_DEFNODE(CacheLastFrameBegin)(
    { /* inputs: */ {
        "input",
    }, /* outputs: */ {
        "linkFrom",
        "lastFrame",
    }, /* params: */ {
    }, /* category: */ {
        "deprecated",
    } }
);


struct CacheLastFrameEnd : zeno::INode {
    CacheLastFrameBegin* m_CacheLastFrameBegin;

    virtual void apply() override {
        {
            zeno::ParamObject paramobj = get_input_obj_param("linkTo");
            if (paramobj.links.empty())
                throw makeError("CacheLastFrameEnd Node: 'linkTo' socket must be connected to CacheLastFrameBegin Node 'linkFrom' socket!\n");

            auto link = paramobj.links[0];

            std::shared_ptr<Graph> spGraph = getThisGraph();
            assert(spGraph);
            m_CacheLastFrameBegin = dynamic_cast<CacheLastFrameBegin*>(spGraph->m_nodes.at(link.outNode).get());
            if (!m_CacheLastFrameBegin) {
                printf("CacheLastFrameEnd Node: 'linkTo' socket must be connected to CacheLastFrameBegin Node 'linkFrom' socket!\n");
                abort();
            }
            auto updatedCache = (*ZImpl(clone_input("updateCache")).clone();
            m_CacheLastFrameBegin->m_lastFrameCache = updatedCache;
            ZImpl(set_output("output", std::move(updatedCache));
            return;
        }
        throw zeno::Exception("CacheLastFrameEnd Node: 'linkTo' socket must be connected to CacheLastFrameBegin Node 'linkFrom' socket!\n");
    }
};


ZENO_DEFNODE(CacheLastFrameEnd)(
    { /* inputs: */ {
        "linkTo",
        "updateCache",
    }, /* outputs: */ {
        "output",
    }, /* params: */ {
    }, /* category: */ {
        "deprecated",
    } }
);


/*struct MakeMutable : zeno::INode {
    virtual void apply() override {
        auto obj = ZImpl(get_input2("anyobj");
        auto ptr = std::make_shared<MutableObject>();
        ptr->set(std::move(obj));
        ZImpl(set_output("mutable", std::move(ptr));
    }
};

ZENDEFNODE(MakeMutable, {
    {"anyobj"},
    {"mutable"},
    {},
    {"control"},
});


struct UpdateMutable : zeno::INode {
    virtual void apply() override {
        auto obj = ZImpl(get_input2("anyobj");
        auto ptr = ZImpl(get_input<MutableObject>("mutable");
        ptr->set(std::move(obj));
        ZImpl(set_output("mutable", std::move(ptr));
    }
};

ZENDEFNODE(UpdateMutable, {
    {"mutable", "anyobj"},
    {"mutable"},
    {},
    {"control"},
});


struct ReadMutable : zeno::INode {
    virtual void apply() override {
        auto ptr = ZImpl(get_input<MutableObject>("mutable");
        auto obj = ptr->value;
        ZImpl(set_output2("anyobj", std::move(obj));
    }
};

ZENDEFNODE(ReadMutable, {
    {"mutable"},
    {"anyobj"},
    {},
    {"control"},
});*/

#endif
}
