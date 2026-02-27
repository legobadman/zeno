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

static int Pos2Idx(int x, int z, int nx) {
    return z * nx + x;
}

static float fit(float data, float ss, float se, float ds, float de) {
    float b = std::numeric_limits<float>::epsilon();
    b = std::max(std::abs(se - ss), b);
    b = (se - ss >= 0.0f) ? b : -b;
    const float alpha = (data - ss) / b;
    return ds + (de - ds) * alpha;
}

static vec3f from_abi(const Vec3f& v) {
    return vec3f(v.x, v.y, v.z);
}

} // namespace

struct HF_maskByFeature : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* terrain = nd->clone_input_Geometry("HeightField");

        int nx = 0;
        int nz = 0;
        if (auto* ud = terrain->userData()) {
            nx = ud->get_int("nx", 0);
            nz = ud->get_int("nz", 0);
        }
        if (nx <= 0 || nz <= 0) {
            throw std::runtime_error("Node [HF_maskByFeature], missing userdata nx/nz");
        }

        const int npts = terrain->npoints();
        if (npts != nx * nz) {
            throw std::runtime_error("Node [HF_maskByFeature], point count not match nx*nz");
        }

        std::vector<Vec3f> pos_abi(static_cast<std::size_t>(npts));
        terrain->points_pos(pos_abi.data(), pos_abi.size());
        const vec3f p0 = from_abi(pos_abi[0]);
        const vec3f p1 = (npts > 1) ? from_abi(pos_abi[1]) : p0 + vec3f(1.0f, 0.0f, 0.0f);
        const float cell_size = std::max(length(p1 - p0), 1e-6f);

        const std::string height_layer = get_input2_string(nd, "height_layer");
        const std::string mask_layer = get_input2_string(nd, "mask_layer");
        const int smooth_radius = nd->get_input2_int("smooth_radius");
        const bool invert_mask = nd->get_input2_bool("invert_mask");

        const bool use_slope = nd->get_input2_bool("use_slope");
        const float min_slope = nd->get_input2_float("min_slopeangle");
        const float max_slope = nd->get_input2_float("max_slopeangle");
        (void)smooth_radius;

        const bool use_dir = nd->get_input2_bool("use_direction");
        const float goal_angle = nd->get_input2_float("goal_angle");
        const float angle_spread = nd->get_input2_float("angle_spread");

        const bool use_height = nd->get_input2_bool("use_height");
        const float min_height = nd->get_input2_float("min_height");
        const float max_height = nd->get_input2_float("max_height");

        if (!terrain->has_attr(ATTR_POINT, height_layer.c_str(), ATTR_FLOAT) ||
            !terrain->has_attr(ATTR_POINT, mask_layer.c_str(), ATTR_FLOAT)) {
            throw std::runtime_error("Node [HF_maskByFeature], no such data layer named `" +
                height_layer + "` or `" + mask_layer + "`");
        }

        std::vector<float> height(static_cast<std::size_t>(npts), 0.0f);
        std::vector<float> mask(static_cast<std::size_t>(npts), 0.0f);
        terrain->get_float_attr(ATTR_POINT, height_layer.c_str(), height.data(), height.size());
        terrain->get_float_attr(ATTR_POINT, mask_layer.c_str(), mask.data(), mask.size());

        std::vector<vec3f> grad(static_cast<std::size_t>(npts), vec3f(0.0f));

        for (int id_z = 0; id_z < nz; ++id_z) {
            for (int id_x = 0; id_x < nx; ++id_x) {
                const int idx = Pos2Idx(id_x, id_z, nx);
                int idx_xl, idx_xr, idx_zl, idx_zr;
                int scale_x = 0;
                int scale_z = 0;

                if (id_x == 0) {
                    idx_xl = idx;
                    idx_xr = Pos2Idx(id_x + 1, id_z, nx);
                    scale_x = 1;
                } else if (id_x == nx - 1) {
                    idx_xl = Pos2Idx(id_x - 1, id_z, nx);
                    idx_xr = idx;
                    scale_x = 1;
                } else {
                    idx_xl = Pos2Idx(id_x - 1, id_z, nx);
                    idx_xr = Pos2Idx(id_x + 1, id_z, nx);
                    scale_x = 2;
                }

                if (id_z == 0) {
                    idx_zl = idx;
                    idx_zr = Pos2Idx(id_x, id_z + 1, nx);
                    scale_z = 1;
                } else if (id_z == nz - 1) {
                    idx_zl = Pos2Idx(id_x, id_z - 1, nx);
                    idx_zr = idx;
                    scale_z = 1;
                } else {
                    idx_zl = Pos2Idx(id_x, id_z - 1, nx);
                    idx_zr = Pos2Idx(id_x, id_z + 1, nx);
                    scale_z = 2;
                }

                grad[static_cast<std::size_t>(idx)][0] =
                    (height[static_cast<std::size_t>(idx_xr)] - height[static_cast<std::size_t>(idx_xl)]) /
                    (static_cast<float>(scale_x) * cell_size);
                grad[static_cast<std::size_t>(idx)][2] =
                    (height[static_cast<std::size_t>(idx_zr)] - height[static_cast<std::size_t>(idx_zl)]) /
                    (static_cast<float>(scale_z) * cell_size);

                const vec3f dx = normalizeSafe(vec3f(1.0f, 0.0f, grad[static_cast<std::size_t>(idx)][0]));
                const vec3f dy = normalizeSafe(vec3f(0.0f, 1.0f, grad[static_cast<std::size_t>(idx)][2]));
                const vec3f n = normalizeSafe(cross(dx, dy));

                float m = 1.0f;
                if (use_slope) {
                    float slope = 180.0f * std::acos(std::clamp(n[2], -1.0f, 1.0f)) / static_cast<float>(M_PI);
                    slope = fit(slope, min_slope, max_slope, 0.0f, 1.0f);
                    m *= slope;
                }
                if (use_dir) {
                    float direction = 180.0f * std::atan2(n[0], n[1]) / static_cast<float>(M_PI);
                    direction -= goal_angle;
                    direction -= 360.0f * std::floor(direction / 360.0f);
                    direction -= 180.0f;
                    direction = fit(direction, -angle_spread, angle_spread, 0.0f, 1.0f);
                    m *= direction;
                }
                if (use_height) {
                    const float h = fit(height[static_cast<std::size_t>(idx)], min_height, max_height, 0.0f, 1.0f);
                    m *= h;
                }
                m = std::clamp(m, 0.0f, 1.0f);
                if (invert_mask) m = 1.0f - m;
                mask[static_cast<std::size_t>(idx)] = m;
            }
        }

        terrain->delete_attr(ATTR_POINT, mask_layer.c_str());
        terrain->create_attr_by_float(ATTR_POINT, mask_layer.c_str(), mask.data(), mask.size());
        nd->set_output_object("HeightField", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(HF_maskByFeature,
    Z_INPUTS(
        {"HeightField", _gParamType_Geometry},
        {"invert_mask", _gParamType_Bool, ZInt(0)},
        {"height_layer", _gParamType_String, ZString("height")},
        {"mask_layer", _gParamType_String, ZString("mask")},
        {"smooth_radius", _gParamType_Int, ZInt(1)},
        {"use_slope", _gParamType_Bool, ZInt(0)},
        {"min_slopeangle", _gParamType_Float, ZFloat(0.0f)},
        {"max_slopeangle", _gParamType_Float, ZFloat(90.0f)},
        {"slope_ramp", _gParamType_String, ZString("")},
        {"use_direction", _gParamType_Bool, ZInt(0)},
        {"goal_angle", _gParamType_Float, ZFloat(0.0f)},
        {"angle_spread", _gParamType_Float, ZFloat(30.0f)},
        {"dir_ramp", _gParamType_String, ZString("")},
        {"use_height", _gParamType_Bool, ZInt(0)},
        {"min_height", _gParamType_Float, ZFloat(0.5f)},
        {"max_height", _gParamType_Float, ZFloat(1.0f)},
        {"height_ramp", _gParamType_String, ZString("")}
    ),
    Z_OUTPUTS(
        {"HeightField", _gParamType_Geometry}
    ),
    "erode",
    "",
    "",
    ""
);

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

