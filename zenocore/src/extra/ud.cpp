//
// GetUserData3 - migrated to zenocore, behavior aligned with
// the legacy implementation in zeno/src/nodes/PortalNodes.cpp:
// fetch a numeric (or string) user-data entry by key and expose
// it as an AnyNumeric primitive plus a hasValue flag.
//

#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <string>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

} // namespace

struct GetUserData3 : INode2 {
    DEF_OVERRIDE_FOR_INODE

    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);

        auto* obj = nd->get_input_object("object");
        if (!obj) {
            nd->set_output_bool("hasValue", false);
            return ZErr_OK;
        }

        auto* ud = obj->userData();
        if (!ud) {
            nd->set_output_bool("hasValue", false);
            return ZErr_OK;
        }

        const std::string key = get_input2_string(nd, "key");
        bool hasValue = false;
        zeno::reflect::Any outVal;

        if (ud->has_int(key.c_str())) {
            int v = ud->get_int(key.c_str(), 0);
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_float(key.c_str())) {
            float v = ud->get_float(key.c_str(), 0.0f);
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec2i(key.c_str())) {
            Vec2i v = ud->get_vec2i(key.c_str());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec3i(key.c_str())) {
            Vec3i v = ud->get_vec3i(key.c_str());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec4i(key.c_str())) {
            Vec4i v = ud->get_vec4i(key.c_str());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec2f(key.c_str())) {
            Vec2f v = ud->get_vec2f(key.c_str(), Vec2f());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec3f(key.c_str())) {
            Vec3f v = ud->get_vec3f(key.c_str(), Vec3f());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_vec4f(key.c_str())) {
            Vec4f v = ud->get_vec4f(key.c_str());
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has_string(key.c_str())) {
            char sbuf[512] = {};
            ud->get_string(key.c_str(), "", sbuf, sizeof(sbuf));
            outVal = zeno::reflect::Any(std::string(sbuf));
            hasValue = true;
        } else if (ud->has_bool(key.c_str())) {
            bool v = ud->get_bool(key.c_str(), false);
            outVal = zeno::reflect::Any(v);
            hasValue = true;
        } else if (ud->has(key.c_str())) {
            nd->report_error("GetUserData3: unsupported UserData type for given key");
            return ZErr_ParamError;
        }

        if (hasValue) {
            params->set_primitive_output("data", outVal);
        }

        nd->set_output_bool("hasValue", hasValue);
        return ZErr_OK;
    }
};

ZENDEFNODE(GetUserData3, {
    {
        {_gParamType_IObject, "object"},
        {gParamType_String, "key", ""},
    },
    {
        {gParamType_AnyNumeric, "data"},
        {gParamType_Bool, "hasValue"},
    },
    {},
    {"lifecycle"},
});

} // namespace zeno

