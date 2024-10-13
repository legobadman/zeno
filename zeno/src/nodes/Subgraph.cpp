#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/DummyObject.h>
#include <zeno/extra/SubnetNode.h>
//#include <zeno/types/ConditionObject.h>
//#include <zeno/utils/safe_at.h>
//#include <cassert>


namespace zeno {
namespace {

struct Subnet : zeno::SubnetNode {
    virtual void apply() override {
        zeno::SubnetNode::apply();
    }
};

ZENDEFNODE(Subnet, {
    {},
    {},
    {},
    {"subgraph"},
});

struct SubInput : zeno::INode {
    virtual void complete() override {
        auto name = get_param<std::string>("name");
        //graph->subInputNodes[name] = myname;
    }

    virtual void apply() override {
        //ֱ����SubnetNode������output��������ʵ������apply�ˡ�
        //printf("!!! %s\n", typeid(*get_input("_IN_port")).name());
        //set_output("port", get_input("_IN_port")); 
        //set_output("hasValue", get_input("_IN_hasValue"));
    }

    CustomUI export_customui() const override {
        CustomUI ui;
        ParamGroup group;
        ParamTab tab;
        tab.groups.emplace_back(std::move(group));
        ui.inputPrims.emplace_back(std::move(tab));
        for (auto param : get_output_primitive_params()) {
            ui.outputPrims.emplace_back(param);
        }
        for (auto param : get_output_object_params()) {
            ui.outputObjs.emplace_back(param);
        }
        return ui;
    }
};

ZENDEFNODE(SubInput, {
    {},
    {{gParamType_Bool, "hasValue"}},
    {},
    {"subgraph"},
});

struct SubOutput : zeno::INode {
    virtual void complete() override {
    }

    virtual void apply() override {
    }

    CustomUI export_customui() const override {
        CustomUI ui;
        ParamGroup group;
        for (auto param : get_input_primitive_params()) {
            group.params.emplace_back(param);
        }
        ParamTab tab;
        tab.groups.emplace_back(std::move(group));
        ui.inputPrims.emplace_back(std::move(tab));
        for (auto param : get_input_object_params()) {
            ui.inputObjs.emplace_back(param);
        }
        return ui;
    }
};

ZENDEFNODE(SubOutput, {
    {},
    {},
    {},
    {"subgraph"},
});

}
}
