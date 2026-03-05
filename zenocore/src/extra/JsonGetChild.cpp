//
// JsonGetChild - migrated from zeno/src/nodes/JsonProcess.cpp
// Output is now AnyNumeric instead of object.
//

#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/types/JsonObject.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <tinygltf/json.hpp>
#include <string>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

} // namespace

struct JsonGetChild : INode2 {
    DEF_OVERRIDE_FOR_INODE

    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);

        // Get input json object
        IObject2* inObj = nd->get_input_object("json");
        if (!inObj) {
            nd->report_error("JsonGetChild: input 'json' is null");
            return ZErr_ParamError;
        }

        auto* jsonObj = dynamic_cast<JsonObject*>(inObj);
        if (!jsonObj) {
            nd->report_error("JsonGetChild: input is not JsonObject");
            return ZErr_ParamError;
        }

        const std::string name = get_input2_string(nd, "name");
        const std::string type = get_input2_string(nd, "type");

        if (!jsonObj->json.contains(name)) {
            // No such key: leave output at its default value
            return ZErr_OK;
        }

        const nlohmann::json& jval = jsonObj->json[name];
        zeno::reflect::Any outVal;

        try {
            if (type == "int") {
                outVal = zeno::reflect::Any(static_cast<int>(jval));
            } else if (type == "float") {
                outVal = zeno::reflect::Any(static_cast<float>(jval));
            } else if (type == "vec2f") {
                vec2f v;
                v[0] = static_cast<float>(jval.at(0));
                v[1] = static_cast<float>(jval.at(1));
                outVal = zeno::reflect::Any(v);
            } else if (type == "vec3f") {
                vec3f v;
                v[0] = static_cast<float>(jval.at(0));
                v[1] = static_cast<float>(jval.at(1));
                v[2] = static_cast<float>(jval.at(2));
                outVal = zeno::reflect::Any(v);
            } else if (type == "vec4f") {
                vec4f v;
                v[0] = static_cast<float>(jval.at(0));
                v[1] = static_cast<float>(jval.at(1));
                v[2] = static_cast<float>(jval.at(2));
                v[3] = static_cast<float>(jval.at(3));
                outVal = zeno::reflect::Any(v);
            } else {
                nd->report_error("JsonGetChild: unsupported type (only int/float/vec2f/vec3f/vec4f are allowed)");
                return ZErr_ParamError;
            }
        } catch (const std::exception&) {
            nd->report_error("JsonGetChild: failed to convert json value");
            return ZErr_ParamError;
        }

        params->set_primitive_output("out", outVal);
        return ZErr_OK;
    }
};

ZENDEFNODE(JsonGetChild, {
    {
        {gParamType_JsonObject, "json"},
        {gParamType_String, "name"},
        ParamPrimitive("type", gParamType_String, "int", zeno::Combobox,
                       std::vector<std::string>{"int", "float", "vec2f", "vec3f", "vec4f"}),
    },
    {
        {gParamType_AnyNumeric, "out"},
    },
    {},
    {
        "json"
    },
});

} // namespace zeno

