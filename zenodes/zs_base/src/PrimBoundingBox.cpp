#include "simple_geometry_common.h"

#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static vec3f to_vec3f(const Vec3f& v) {
    return vec3f(v.x, v.y, v.z);
}

static Vec3f to_abi(const vec3f& v) {
    return Vec3f{v[0], v[1], v[2]};
}

static std::pair<vec3f, vec3f> geo_bounding_box(IGeometryObject* geo) {
    const int n = geo->npoints();
    if (n <= 0) return {vec3f(0.0f), vec3f(0.0f)};
    std::vector<Vec3f> pos(static_cast<std::size_t>(n));
    geo->points_pos(pos.data(), pos.size());
    vec3f bmin = to_vec3f(pos[0]);
    vec3f bmax = bmin;
    for (int i = 1; i < n; ++i) {
        const vec3f p = to_vec3f(pos[static_cast<std::size_t>(i)]);
        bmin = zeno::min(bmin, p);
        bmax = zeno::max(bmax, p);
    }
    return {bmin, bmax};
}

} // namespace

struct PrimBoundingBox : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* prim = nd->get_input_Geometry("prim");
        vec3f bmin, bmax;
        std::tie(bmin, bmax) = geo_bounding_box(prim);

        const float extra_bound = nd->get_input2_float("extraBound");
        if (extra_bound != 0.0f) {
            bmin -= extra_bound;
            bmax += extra_bound;
        }

        const vec3f center = (bmin + bmax) * 0.5f;
        const vec3f radius = (bmax - bmin) * 0.5f;
        const vec3f diameter = bmax - bmin;

        nd->set_output_vec3f("bmin", to_abi(bmin));
        nd->set_output_vec3f("bmax", to_abi(bmax));
        nd->set_output_vec3f("center", to_abi(center));
        nd->set_output_vec3f("radius", to_abi(radius));
        nd->set_output_vec3f("diameter", to_abi(diameter));
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimBoundingBox,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"extraBound", _gParamType_Float, ZFloat(0.0f)}
    ),
    Z_OUTPUTS(
        {"bmin", _gParamType_Vec3f},
        {"bmax", _gParamType_Vec3f},
        {"center", _gParamType_Vec3f},
        {"radius", _gParamType_Vec3f},
        {"diameter", _gParamType_Vec3f}
    ),
    "primitive",
    "",
    "",
    ""
);

struct BoundingBoxFitInto : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        const vec3f bmin_src = to_vec3f(nd->get_input2_vec3f("bminSrc"));
        const vec3f bmax_src = to_vec3f(nd->get_input2_vec3f("bmaxSrc"));
        const vec3f bmin_dst = to_vec3f(nd->get_input2_vec3f("bminDst"));
        const vec3f bmax_dst = to_vec3f(nd->get_input2_vec3f("bmaxDst"));

        vec3f translation(0.0f);
        for (int i = 0; i < 3; ++i) {
            const bool lt = bmin_src[i] < bmin_dst[i];
            const bool gt = bmax_src[i] > bmax_dst[i];
            if (lt && !gt) {
                translation[i] = bmin_dst[i] - bmin_src[i];
            } else if (!lt && gt) {
                translation[i] = bmax_dst[i] - bmax_src[i];
            } else if (lt && gt) {
                translation[i] = (bmin_dst[i] - bmin_src[i] + bmax_dst[i] - bmax_src[i]) * 0.5f;
            }
        }

        const vec3f bmin_trans = bmin_src + translation;
        const vec3f bmax_trans = bmax_src + translation;
        nd->set_output_vec3f("translation", to_abi(translation));
        nd->set_output_vec3f("bminTrans", to_abi(bmin_trans));
        nd->set_output_vec3f("bmaxTrans", to_abi(bmax_trans));
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(BoundingBoxFitInto,
    Z_INPUTS(
        {"bminSrc", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"bmaxSrc", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"bminDst", _gParamType_Vec3f, ZVec3f(-1.0f, -1.0f, -1.0f)},
        {"bmaxDst", _gParamType_Vec3f, ZVec3f(1.0f, 1.0f, 1.0f)}
    ),
    Z_OUTPUTS(
        {"translation", _gParamType_Vec3f},
        {"bminTrans", _gParamType_Vec3f},
        {"bmaxTrans", _gParamType_Vec3f}
    ),
    "primitive",
    "",
    "",
    ""
);

struct PrimCalcCentroid : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* prim = nd->get_input_Geometry("prim");
        const std::string method = get_input2_string(nd, "method");
        const float density = nd->get_input2_float("density");

        double accx = 0.0, accy = 0.0, accz = 0.0, accw = 0.0;

        const int npts = prim->npoints();
        std::vector<Vec3f> pos(static_cast<std::size_t>(npts));
        prim->points_pos(pos.data(), pos.size());

        if (method == "Vertex") {
            for (int i = 0; i < npts; ++i) {
                const auto& p = pos[static_cast<std::size_t>(i)];
                accx += p.x;
                accy += p.y;
                accz += p.z;
                accw += 1.0;
            }
        } else if (method == "BoundBox") {
            vec3f bmin, bmax;
            std::tie(bmin, bmax) = geo_bounding_box(prim);
            const vec3f center = (bmin + bmax) * 0.5f;
            const vec3f diam = bmax - bmin;
            const double volume = static_cast<double>(diam[0]) * diam[1] * diam[2];
            accx = center[0] * volume;
            accy = center[1] * volume;
            accz = center[2] * volume;
            accw = volume;
        } else {
            const bool by_area = (method == "Area");
            const int nfaces = prim->nfaces();
            for (int f = 0; f < nfaces; ++f) {
                const int nv = prim->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> fp(static_cast<std::size_t>(nv));
                prim->face_points(f, fp.data(), fp.size());
                const vec3f a = to_vec3f(pos[static_cast<std::size_t>(fp[0])]);
                for (int k = 1; k + 1 < nv; ++k) {
                    const vec3f b = to_vec3f(pos[static_cast<std::size_t>(fp[k])]);
                    const vec3f c = to_vec3f(pos[static_cast<std::size_t>(fp[k + 1])]);
                    const vec3f tri_center = (a + b + c) * (1.0f / 3.0f);
                    const vec3f cp = cross(b - a, c - a);
                    const float area = length(cp);
                    if (by_area) {
                        const double w = static_cast<double>(area);
                        accx += tri_center[0] * w;
                        accy += tri_center[1] * w;
                        accz += tri_center[2] * w;
                        accw += w;
                    } else {
                        const double w = static_cast<double>(dot(tri_center, cp)) / 6.0;
                        const vec3f vcenter = tri_center * static_cast<float>((3.0 / 4.0) * w);
                        accx += vcenter[0];
                        accy += vcenter[1];
                        accz += vcenter[2];
                        accw += w;
                    }
                }
            }
        }

        vec3f centroid(0.0f);
        if (accw != 0.0) {
            centroid = vec3f(
                static_cast<float>(accx / accw),
                static_cast<float>(accy / accw),
                static_cast<float>(accz / accw)
            );
        }
        const float mass = static_cast<float>(std::abs(accw)) * density;
        nd->set_output_vec3f("centroid", to_abi(centroid));
        nd->set_output_float("mass", mass);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimCalcCentroid,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"method", _gParamType_String, ZString("Volume"), Combobox, Z_STRING_ARRAY("Volume", "Area", "Vertex", "BoundBox")},
        {"density", _gParamType_Float, ZFloat(1.0f)}
    ),
    Z_OUTPUTS(
        {"centroid", _gParamType_Vec3f},
        {"mass", _gParamType_Float}
    ),
    "primitive",
    "",
    "",
    ""
);

} // namespace zeno

