#include <zeno/zeno.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/ZNode.h>
#include <zeno/core/common.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/types/CurveObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/Error.h>
#include <zeno/utils/log.h>
#include <zeno/utils/vec.h>
#include <cmath>
#include <vector>
#include <algorithm>

namespace zeno {

namespace {

int Pos2Idx(int x, int z, int nx) {
    return z * nx + x;
}

float fit(float data, float ss, float se, float ds, float de) {
    float b = std::numeric_limits<float>::epsilon();
    b = std::max(std::abs(se - ss), b);
    b = (se - ss >= 0) ? b : -b;
    float alpha = (data - ss) / b;
    return ds + (de - ds) * alpha;
}

CurvesData get_curve_param(ZNodeParams* params, const char* name) {
    auto any = params->get_param_result(name);
    if (!any)
        return CurvesData{};
    try {
        return zeno::reflect::any_cast<CurvesData>(any);
    } catch (...) {
        return CurvesData{};
    }
}

float eval_curve_safe(const CurvesData& curve, float x) {
    if (curve.empty() || !curve.contains("x"))
        return x;
    return curve.eval(x);
}

} // namespace

struct HF_maskByFeature : INode2 {
    NodeType type() const override { return Node_Normal; }
    void clearCalcResults() override {}
    float time() const override { return 1.0f; }

    ZErrorCode apply(INodeData* pNodeData) override {
        auto* params = static_cast<ZNodeParams*>(pNodeData);
        auto* terrain = dynamic_cast<GeometryObject*>(params->clone_input_Geometry("HeightField"));
        if (!terrain) {
            throw makeError<UnimplError>("HF_maskByFeature: no input HeightField");
        }

        auto ud = terrain->userData();
        if (!ud || !ud->has_int("nx") || !ud->has_int("nz")) {
            zeno::log_error("HF_maskByFeature: no such UserData named 'nx' and 'nz'");
            params->set_output_object("HeightField", terrain);
            return ZErr_OK;
        }
        int nx = ud->get_int("nx");
        int nz = ud->get_int("nz");

        const auto& pos = terrain->points_pos();
        if (pos.size() < 2) {
            params->set_output_object("HeightField", terrain);
            return ZErr_OK;
        }
        zeno::vec3f p0 = pos[0];
        zeno::vec3f p1 = pos[1];
        float cellSize = length(p1 - p0);

        std::string heightLayer = params->get_input2_string("height_layer");
        std::string maskLayer = params->get_input2_string("mask_layer");
        bool invertMask = params->get_input2_bool("invert_mask");

        bool useSlope = params->get_input2_bool("use_slope");
        float minSlope = params->get_input2_float("min_slopeangle");
        float maxSlope = params->get_input2_float("max_slopeangle");
        CurvesData curve_slope = get_curve_param(params, "slope_ramp");

        bool useDir = params->get_input2_bool("use_direction");
        float goalAngle = params->get_input2_float("goal_angle");
        float angleSpread = params->get_input2_float("angle_spread");
        CurvesData curve_dir = get_curve_param(params, "dir_ramp");

        bool useHeight = params->get_input2_bool("use_height");
        float minHeight = params->get_input2_float("min_height");
        float maxHeight = params->get_input2_float("max_height");
        CurvesData curve_height = get_curve_param(params, "height_ramp");

        if (!terrain->has_point_attr(heightLayer) || !terrain->has_point_attr(maskLayer)) {
            throw makeError<UnimplError>("HF_maskByFeature: no such data layer named '" +
                heightLayer + "' or '" + maskLayer + "'");
        }

        auto height = terrain->get_attrs<float>(ATTR_POINT, heightLayer);
        auto mask = height;

        std::vector<zeno::vec3f> _grad(static_cast<size_t>(terrain->npoints()), zeno::vec3f(0, 0, 0));

#pragma omp parallel for
        for (int id_z = 0; id_z < nz; id_z++) {
            for (int id_x = 0; id_x < nx; id_x++) {
                int idx = Pos2Idx(id_x, id_z, nx);
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

                _grad[static_cast<size_t>(idx)][0] = (height[static_cast<size_t>(idx_xr)] - height[static_cast<size_t>(idx_xl)]) / (static_cast<float>(scale_x) * cellSize);
                _grad[static_cast<size_t>(idx)][2] = (height[static_cast<size_t>(idx_zr)] - height[static_cast<size_t>(idx_zl)]) / (static_cast<float>(scale_z) * cellSize);

                zeno::vec3f dx = normalizeSafe(zeno::vec3f(1, 0, _grad[static_cast<size_t>(idx)][0]));
                zeno::vec3f dy = normalizeSafe(zeno::vec3f(0, 1, _grad[static_cast<size_t>(idx)][2]));
                zeno::vec3f n = normalizeSafe(cross(dx, dy));

                mask[static_cast<size_t>(idx)] = 1.0f;

                if (useSlope) {
                    float slope = 180.0f * std::acos(n[2]) / static_cast<float>(M_PI);
                    slope = fit(slope, minSlope, maxSlope, 0, 1);
                    slope = eval_curve_safe(curve_slope, slope);
                    mask[static_cast<size_t>(idx)] *= slope;
                }

                if (useDir) {
                    float direction = 180.0f * std::atan2(n[0], n[1]) / static_cast<float>(M_PI);
                    direction -= goalAngle;
                    direction -= 360.0f * std::floor(direction / 360.0f);
                    direction -= 180.0f;
                    direction = fit(direction, -angleSpread, angleSpread, 0, 1);
                    direction = eval_curve_safe(curve_dir, direction);
                    mask[static_cast<size_t>(idx)] *= direction;
                }

                if (useHeight) {
                    float h = fit(height[static_cast<size_t>(idx)], minHeight, maxHeight, 0, 1);
                    mask[static_cast<size_t>(idx)] *= eval_curve_safe(curve_height, h);
                }

                if (invertMask) {
                    mask[static_cast<size_t>(idx)] = std::min(std::max(mask[static_cast<size_t>(idx)], 0.0f), 1.0f);
                    mask[static_cast<size_t>(idx)] = 1.0f - mask[static_cast<size_t>(idx)];
                }
            }
        }

        terrain->set_point_attr(maskLayer, AttrVar(mask));
        params->set_output_object("HeightField", terrain);
        return ZErr_OK;
    }
};

ZENDEFNODE(HF_maskByFeature,
    {
        {
            {gParamType_Geometry, "HeightField"},
            {gParamType_Bool, "invert_mask", "0"},
            {gParamType_String, "height_layer", "height"},
            {gParamType_String, "mask_layer", "mask"},
            {gParamType_Int, "smooth_radius", "1"},
            {gParamType_Bool, "use_slope", "0"},
            {gParamType_Float, "min_slopeangle", "0"},
            {gParamType_Float, "max_slopeangle", "90"},
            {gParamType_Curve, "slope_ramp"},
            {gParamType_Bool, "use_direction", "0"},
            {gParamType_Float, "goal_angle", "0"},
            {gParamType_Float, "angle_spread", "30"},
            {gParamType_Curve, "dir_ramp"},
            {gParamType_Bool, "use_height", "0"},
            {gParamType_Float, "min_height", "0.5"},
            {gParamType_Float, "max_height", "1"},
            {gParamType_Curve, "height_ramp"},
        },
        {
            {gParamType_Geometry, "HeightField"},
        },
        {},
        {"erode"}
    });

} // namespace zeno
