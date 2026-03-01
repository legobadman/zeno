#include "simple_geometry_common.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static float fit(float data, float ss, float se, float ds, float de) {
    float b = std::numeric_limits<float>::epsilon();
    b = std::max(std::abs(se - ss), b);
    b = (se - ss >= 0.0f) ? b : -b;
    const float alpha = (data - ss) / b;
    return ds + (de - ds) * alpha;
}

} // namespace

struct HF_remap : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("prim");
        const std::string remap_layer = get_input2_string(nd, "remap layer");
        if (!terrain->has_attr(ATTR_POINT, remap_layer.c_str(), ATTR_FLOAT)) {
            throw std::runtime_error("Node [HF_remap], no such data layer named `" + remap_layer + "`");
        }

        const bool auto_compute = nd->get_input2_bool("Auto Compute input range");
        float in_min = nd->get_input2_float("input min");
        float in_max = nd->get_input2_float("input max");
        const float out_min = nd->get_input2_float("output min");
        const float out_max = nd->get_input2_float("output max");
        const bool clamp_min = nd->get_input2_bool("clamp min");
        const bool clamp_max = nd->get_input2_bool("clamp max");

        const int npts = terrain->npoints();
        std::vector<float> var(static_cast<std::size_t>(npts), 0.0f);
        terrain->get_float_attr(ATTR_POINT, remap_layer.c_str(), var.data(), var.size());
        if (var.empty()) {
            nd->set_output_object("prim", terrain);
            return ZErr_OK;
        }

        if (auto_compute) {
            in_min = var[0];
            in_max = var[0];
            for (std::size_t i = 1; i < var.size(); ++i) {
                in_min = std::min(in_min, var[i]);
                in_max = std::max(in_max, var[i]);
            }
        }

        // NOTE: ABI path currently has no CurvesData input decoding, so ramp is identity.
        for (std::size_t i = 0; i < var.size(); ++i) {
            const float old_val = var[i];
            float new_val = old_val;
            if (old_val < in_min) {
                if (clamp_min) new_val = out_min;
                else new_val = old_val - in_min + out_min;
            } else if (old_val > in_max) {
                if (clamp_max) new_val = out_max;
                else new_val = old_val - in_max + out_max;
            } else {
                new_val = fit(new_val, in_min, in_max, 0.0f, 1.0f);
                new_val = fit(new_val, 0.0f, 1.0f, out_min, out_max);
            }
            var[i] = new_val;
        }

        terrain->delete_attr(ATTR_POINT, remap_layer.c_str());
        terrain->create_attr_by_float(ATTR_POINT, remap_layer.c_str(), var.data(), var.size());
        nd->set_output_object("prim", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(HF_remap,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"remap layer", _gParamType_String, ZString("height")},
        {"Auto Compute input range", _gParamType_Bool, ZInt(0)},
        {"input min", _gParamType_Float, ZFloat(0.0f)},
        {"input max", _gParamType_Float, ZFloat(1.0f)},
        {"output min", _gParamType_Float, ZFloat(0.0f)},
        {"output max", _gParamType_Float, ZFloat(1.0f)},
        {"remap ramp", _gParamType_String, ZString("")},
        {"clamp min", _gParamType_Bool, ZInt(0)},
        {"clamp max", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS(
        {"prim", _gParamType_Geometry}
    ),
    "deprecated",
    "",
    "",
    ""
);

} // namespace zeno

