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
        ui.uistyle.background = "#1A5447";
        ui.uistyle.iconResPath = "<svg width=\"30\" height=\"24\" viewBox=\"0 0 30 24\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M24.3807 8.74361L7.07678 9.38639C6.50413 9.38639 6.17022 9.60077 6.01249 10.1514L2.42052 22.4397C2.27201 22.962 1.56007 23.2063 1.0648 23.2063C0.56987 23.2063 0.164835 22.8013 0.164835 22.3064V18.8626V4.75384V4.07977V2.01409C0.164835 1.32158 0.726286 0.760132 1.4188 0.760132H10.3283C10.6608 0.760132 10.9796 0.89218 11.2147 1.1273L13.7997 3.71228C14.0348 3.9474 14.3539 4.07944 14.6862 4.07944H23.1267C23.8192 4.07944 24.3807 4.6409 24.3807 5.33341V5.76182V8.74361Z\" fill=\"#E0AD31\"/><path d=\"M1.0648 23.2063C1.55974 23.2063 1.81626 22.8286 1.96477 22.3063L5.58572 9.67383C5.74345 9.12325 6.24695 8.7439 6.81993 8.7439H29.0076C29.5375 8.7439 29.9181 9.25299 29.7686 9.76142L26.1806 21.954C26.0186 22.5306 25.6633 23.2112 24.9283 23.2063H1.0648Z\" fill=\"#FFC843\"/></svg>";
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
