#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/DummyObject.h>
#include <zeno/extra/SubnetNode.h>
//#include <zeno/types/ConditionObject.h>
//#include <zeno/utils/safe_at.h>
//#include <cassert>


namespace zeno {
namespace {

struct Subnet : INode2 {
    virtual void apply(INodeData* ptrNodeData) override {
        /*zeno::SubnetNode::apply();*/
    }
    NodeType type() const override {
        return Node_SubgraphNode;
    }
    void clearCalcResults() {}
    void getIconResource(char* recv, size_t cap) {}
    void getBackgroundClr(char* recv, size_t cap) {}
    float time() const { return 1.0f; }
};

ZENDEFNODE(Subnet, {
    {},
    {},
    {},
    {"subgraph"},
});

struct SubInput : zeno::INode2 {
    void apply(INodeData* ptrNodeData) override {
        //printf("!!! %s\n", typeid(*ZImpl(get_input("_IN_port"))).name());
        //set_output("port", ZImpl(get_input("_IN_port"))); 
        //set_output("hasValue", ZImpl(get_input("_IN_hasValue")));
    }

    NodeType type() const {
        return Node_SubgraphNode;
    }
    void clearCalcResults() {}
    void getIconResource(char* recv, size_t cap) {
    }
    void getBackgroundClr(char* recv, size_t cap) {
        const char* bg = "#5F5F5F";
        strcpy(recv, bg);
        recv[strlen(bg)] = '\0';
    }
    float time() const { return 1.0f; }
};

ZENDEFNODE(SubInput, {
    {},
    {{gParamType_Bool, "hasValue"}},
    {},
    {"subgraph"},
});

struct SubOutput : zeno::INode2 {
    void apply(INodeData* ptrNodeData) override {
    }

    NodeType type() const {
        return Node_SubgraphNode;
    }

    void clearCalcResults() {}
    void getIconResource(char* recv, size_t cap) {
    }
    void getBackgroundClr(char* recv, size_t cap) {
        const char* bg = "#5F5F5F";
        strcpy(recv, bg);
        recv[strlen(bg)] = '\0';
    }
    float time() const { return 1.0f; }
};

ZENDEFNODE(SubOutput, {
    {},
    {},
    {},
    {"subgraph"},
});

}
}
