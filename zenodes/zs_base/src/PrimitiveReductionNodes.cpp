#include "simple_geometry_common.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

} // namespace

struct PrimReduction : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* prim = nd->get_input_Geometry("prim");
        const std::string attr = get_input2_string(nd, "attrName");
        const std::string op = get_input2_string(nd, "op");
        const int n = prim->npoints();

        if (prim->has_attr(ATTR_POINT, attr.c_str(), ATTR_VEC3)) {
            std::vector<Vec3f> data(static_cast<std::size_t>(n));
            prim->get_vec3f_attr(ATTR_POINT, attr.c_str(), data.data(), data.size());
            vec3f acc = vec3f(0.0f);
            if (op == "max" || op == "min" || op == "absmax") {
                acc = vec3f(data[0].x, data[0].y, data[0].z);
            }
            for (int i = 0; i < n; ++i) {
                vec3f v(data[i].x, data[i].y, data[i].z);
                if (op == "avg") acc += v;
                else if (op == "max") acc = zeno::max(acc, v);
                else if (op == "min") acc = zeno::min(acc, v);
                else if (op == "absmax") acc = zeno::max(acc, zeno::abs(v));
            }
            if (op == "avg" && n > 0) acc /= static_cast<float>(n);
            nd->set_output_vec3f("result", Vec3f{acc[0], acc[1], acc[2]});
            return ZErr_OK;
        }

        std::vector<float> data(static_cast<std::size_t>(n), 0.0f);
        prim->get_float_attr(ATTR_POINT, attr.c_str(), data.data(), data.size());
        float acc = (op == "max" || op == "min" || op == "absmax") ? data[0] : 0.0f;
        for (int i = 0; i < n; ++i) {
            const float v = data[static_cast<std::size_t>(i)];
            if (op == "avg") acc += v;
            else if (op == "max") acc = std::max(acc, v);
            else if (op == "min") acc = std::min(acc, v);
            else if (op == "absmax") acc = std::max(acc, std::abs(v));
        }
        if (op == "avg" && n > 0) acc /= static_cast<float>(n);
        nd->set_output_float("result", acc);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimReduction,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"attrName", _gParamType_String, ZString("pos")},
        {"op", _gParamType_String, ZString("avg"), Combobox, Z_STRING_ARRAY("avg", "max", "min", "absmax")}
    ),
    Z_OUTPUTS(
        {"result", _gParamType_Vec3f}
    ),
    "primitive",
    "",
    "",
    ""
);

struct PrimitiveBoundingBox : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* prim = nd->get_input_Geometry("prim");
        const int n = prim->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(n));
        prim->get_vec3f_attr(ATTR_POINT, "pos", pos.data(), pos.size());

        vec3f bmin = n > 0 ? vec3f(pos[0].x, pos[0].y, pos[0].z) : vec3f(0.0f);
        vec3f bmax = bmin;
        for (int i = 1; i < n; ++i) {
            vec3f p(pos[static_cast<std::size_t>(i)].x, pos[static_cast<std::size_t>(i)].y, pos[static_cast<std::size_t>(i)].z);
            bmin = zeno::min(bmin, p);
            bmax = zeno::max(bmax, p);
        }
        const float exwidth = nd->has_input("exWidth") ? nd->get_input2_float("exWidth") : 0.0f;
        if (exwidth != 0.0f) {
            bmin -= exwidth;
            bmax += exwidth;
        }
        nd->set_output_vec3f("bmin", Vec3f{bmin[0], bmin[1], bmin[2]});
        nd->set_output_vec3f("bmax", Vec3f{bmax[0], bmax[1], bmax[2]});
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimitiveBoundingBox,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"exWidth", _gParamType_Float, ZFloat(0.0f)}
    ),
    Z_OUTPUTS(
        {"bmin", _gParamType_Vec3f},
        {"bmax", _gParamType_Vec3f}
    ),
    "primitive",
    "",
    "",
    ""
);

} // namespace zeno

