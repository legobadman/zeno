#include <zeno/zeno.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/data.h>
#include <zeno/types/GeometryObject.h>
#include <vector>

namespace zeno {

static zeno::vec3f interp_heatmap(const std::vector<zeno::vec3f>& colors, float x) {
    if (colors.empty()) return zeno::vec3f(0.0f);
    if (x <= 0.0f) return colors.front();
    if (x >= 1.0f) return colors.back();
    const float xx = zeno::clamp(x, 0.0f, 1.0f) * static_cast<float>(colors.size() - 1);
    const int i = static_cast<int>(zeno::floor(xx));
    const float f = xx - static_cast<float>(i);
    return zeno::mix(colors[i], colors[i + 1], f);
}

struct PrimitiveColorByHeatmap : zeno::INode2 {
    NodeType type() const override { return Node_Normal; }
    void clearCalcResults() override {}
    float time() const override { return 1.0f; }

    ZErrorCode apply(INodeData* pNodeData) override {
        auto* params = static_cast<ZNodeParams*>(pNodeData);
        auto* prim = params->clone_input_Geometry("prim");
        if (!prim) {
            throw makeError<UnimplError>("PrimitiveColorByHeatmap: empty input geometry");
        }

        HeatmapData heatmap = zeno::reflect::any_cast<HeatmapData>(params->get_param_result("heatmap"));
        std::string attrName2 = params->get_input2_string("attrName2");
        std::string attrName = attrName2.empty() ? params->get_input2_string("attrName") : attrName2;
        if (!prim->has_attr(ATTR_POINT, attrName.c_str(), ATTR_FLOAT)) {
            throw makeError<UnimplError>("PrimitiveColorByHeatmap: attr not found");
        }

        const float minv = params->get_input2_float("min");
        const float maxv = params->get_input2_float("max");
        const float denom = (maxv - minv == 0.0f) ? 1.0f : (maxv - minv);
        const std::vector<zeno::vec3f> heatmap_clrs = heatmap.toVecColors(1024);

        const int n = prim->npoints();
        std::vector<float> src(static_cast<std::size_t>(n), 0.0f);
        prim->get_float_attr(ATTR_POINT, attrName.c_str(), src.data(), src.size());

        std::vector<Vec3f> clr(static_cast<std::size_t>(n));
        for (int i = 0; i < n; i++) {
            const float x = (src[static_cast<std::size_t>(i)] - minv) / denom;
            const auto c = interp_heatmap(heatmap_clrs, x);
            clr[static_cast<std::size_t>(i)] = Vec3f{c[0], c[1], c[2]};
        }

        prim->delete_attr(ATTR_POINT, "clr");
        prim->create_attr_by_vec3(ATTR_POINT, "clr", clr.data(), clr.size());
        params->set_output_object("prim", prim);
        return ZErr_OK;
    }
};

ZENDEFNODE(PrimitiveColorByHeatmap,
    { /* inputs: */ {
        {gParamType_Geometry, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_String, "attrName2"},
        {gParamType_Heatmap, "heatmap", "", zeno::Socket_Primitve, zeno::Heatmap},
        {gParamType_Float, "min", "0"},
        {gParamType_Float, "max", "1"},
    },
    /* outputs: */ {
        {gParamType_Geometry, "prim"},
    }, /* params: */ {
        {gParamType_String, "attrName", "rho"},
    }, /* category: */ {
        "visualize",
    }});

} // namespace zeno

