#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/DummyObject.h>
//#include <zeno/extra/SubnetNode.h>
//#include <zeno/types/ConditionObject.h>
//#include <zeno/utils/safe_at.h>
//#include <cassert>


namespace zeno {
namespace {

struct Subnet : INode2 {
    virtual ZErrorCode apply(INodeData* ptrNodeData) override {
        /*zeno::SubnetNode::apply();*/
        return ZErr_OK;
    }
    NodeType type() const override {
        return Node_SubgraphNode;
    }
    void clearCalcResults() {}
    float time() const { return 1.0f; }
};

ZENDEFNODE(Subnet, {
    {},
    {},
    {},
    {"subgraph"},
    "",
    "",
    ":/icons/node/subnet.svg",
    "#1D5F51"
});

struct SubInput : zeno::INode2 {
    ZErrorCode apply(INodeData* ptrNodeData) override {
        //printf("!!! %s\n", typeid(*ZImpl(get_input("_IN_port"))).name());
        //set_output("port", ZImpl(get_input("_IN_port"))); 
        //set_output("hasValue", ZImpl(get_input("_IN_hasValue")));
        return ZErr_OK;
    }

    NodeType type() const {
        return zeno::SubInput;
    }
    void clearCalcResults() {}
    float time() const { return 1.0f; }
};

ZENDEFNODE(SubInput, {
    {},
    {{gParamType_Bool, "hasValue"}},
    {},
    {"subgraph"},
    "",
    "",
    "",
    "#5F5F5F"
});

struct SubOutput : zeno::INode2 {
    ZErrorCode apply(INodeData* ptrNodeData) override {
        return ZErr_OK;
    }

    NodeType type() const {
        return zeno::SubOutput;
    }

    void clearCalcResults() {}
    float time() const { return 1.0f; }
};

ZENDEFNODE(SubOutput, {
    {},
    {},
    {},
    {"subgraph"},
    "",
    "",
    "",
    "#5F5F5F"
});

}
}
