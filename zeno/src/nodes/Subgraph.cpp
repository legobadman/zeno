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
    CustomUI export_customui() const override {
        CustomUI ui = zeno::SubnetNode::export_customui();
        ui.uistyle.background = "#1D5F51";
        ui.uistyle.iconResPath = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">    <path d=\"M4 7H12L15 10H28V25H4V7Z\" stroke=\"#CCCCCC\" stroke-width=\"2\" stroke-linejoin=\"round\"/>    <line x1=\"4\" y1=\"14\" x2=\"28\" y2=\"14\" stroke=\"#CCCCCC\" stroke-width=\"2\"/></svg>";
        return ui;
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
        ui.uistyle.background = "#5F5F5F";
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

    ZENO_API virtual zany get_default_output_object() override {
        zany obj = get_input("port");
        return obj;
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
        ui.uistyle.background = "#5F5F5F";
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
