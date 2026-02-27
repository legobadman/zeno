#include "simple_geometry_common.h"

#include <cmath>
#include <sstream>
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

static std::vector<std::string> parse_channels(IGeometryObject* grid, const std::string& channel_list) {
    std::vector<std::string> channels;
    std::istringstream iss(channel_list);
    std::string token;
    while (iss >> token) {
        if (token == "*") {
            channels.clear();
            const int nattr = grid->nattributes(ATTR_POINT);
            char buf[256] = {};
            for (int i = 0; i < nattr; ++i) {
                size_t len = grid->get_attr_name(ATTR_POINT, i, buf, sizeof(buf));
                if (len > 0) channels.emplace_back(buf);
            }
            break;
        }
        channels.emplace_back(token);
    }
    return channels;
}

template <class T>
static T lerp_value(const T& a, const T& b, float t) {
    return a * (1.0f - t) + b * t;
}

template <class T>
static void sample2d_attr(
    const std::vector<glm::vec3>& coord,
    const std::vector<T>& field,
    std::vector<T>& prim_attr,
    int nx,
    int ny,
    float h,
    glm::vec3 bmin,
    bool periodic
) {
    if (nx < 2 || ny < 2 || h == 0.0f) return;
    if (field.size() < static_cast<std::size_t>(nx * ny)) return;
    if (prim_attr.size() != coord.size()) prim_attr.resize(coord.size());

    std::vector<T> temp(coord.size());
    for (std::size_t tidx = 0; tidx < coord.size(); ++tidx) {
        const glm::vec3 uv = coord[tidx];
        glm::vec3 uv2;
        if (periodic) {
            const float lx = static_cast<float>(nx - 1) * h;
            const float ly = static_cast<float>(ny - 1) * h;
            const int gid_x = static_cast<int>(std::floor((uv[0] - bmin[0]) / lx));
            const int gid_y = static_cast<int>(std::floor((uv[2] - bmin[2]) / ly));
            uv2 = (uv - (bmin + glm::vec3(gid_x * lx, 0.0f, gid_y * ly))) / h;
            uv2 = glm::clamp(
                uv2,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(static_cast<float>(nx) - 1.01f, 0.0f, static_cast<float>(ny) - 1.01f)
            );
        } else {
            uv2 = (uv - bmin) / h;
            uv2 = glm::clamp(
                uv2,
                glm::vec3(0.01f, 0.0f, 0.01f),
                glm::vec3(static_cast<float>(nx) - 1.01f, 0.0f, static_cast<float>(ny) - 1.01f)
            );
        }

        const int i = static_cast<int>(uv2[0]);
        const int j = static_cast<int>(uv2[2]);
        const float cx = uv2[0] - static_cast<float>(i);
        const float cy = uv2[2] - static_cast<float>(j);
        const std::size_t idx00 = static_cast<std::size_t>(j * nx + i);
        const std::size_t idx01 = static_cast<std::size_t>(j * nx + i + 1);
        const std::size_t idx10 = static_cast<std::size_t>((j + 1) * nx + i);
        const std::size_t idx11 = static_cast<std::size_t>((j + 1) * nx + i + 1);
        temp[tidx] = lerp_value<T>(
            lerp_value<T>(field[idx00], field[idx01], cx),
            lerp_value<T>(field[idx10], field[idx11], cx),
            cy
        );
    }

    for (std::size_t tidx = 0; tidx < coord.size(); ++tidx) {
        prim_attr[tidx] = temp[tidx];
    }
}

} // namespace

struct Grid2DSample_M : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        const int nx = nd->get_input2_int("nx");
        const int ny = nd->get_input2_int("ny");
        const float h = nd->get_input2_float("h");
        const Vec3f bmin_abi = nd->get_input2_vec3f("bmin");
        const glm::vec3 bmin(bmin_abi.x, bmin_abi.y, bmin_abi.z);
        const std::string channel = get_input2_string(nd, "channel");
        const std::string sample_by = get_input2_string(nd, "sampleBy");
        const std::string attr_t = get_input2_string(nd, "attrT");

        auto* grid = nd->clone_input_Geometry("grid");
        auto* grid2 = nd->get_input_Geometry("grid2");

        if (sample_by != "pos" && !grid->has_attr(ATTR_POINT, sample_by.c_str(), ATTR_VEC3)) {
            nd->set_output_object("prim", grid);
            return ZErr_OK;
        }

        const int npts = grid->npoints();
        std::vector<Vec3f> coords_abi(static_cast<std::size_t>(npts));
        if (sample_by == "pos") {
            grid->get_vec3f_attr(ATTR_POINT, "pos", coords_abi.data(), coords_abi.size());
        } else {
            grid->get_vec3f_attr(ATTR_POINT, sample_by.c_str(), coords_abi.data(), coords_abi.size());
        }
        std::vector<glm::vec3> coords(coords_abi.size());
        for (std::size_t i = 0; i < coords_abi.size(); ++i) {
            coords[i] = glm::vec3(coords_abi[i].x, coords_abi[i].y, coords_abi[i].z);
        }

        if (attr_t == "float") {
            if (!grid->has_attr(ATTR_POINT, channel.c_str(), ATTR_FLOAT) ||
                !grid2->has_attr(ATTR_POINT, channel.c_str(), ATTR_FLOAT)) {
                nd->set_output_object("prim", grid);
                return ZErr_OK;
            }
            std::vector<float> dst(static_cast<std::size_t>(npts), 0.0f);
            grid->get_float_attr(ATTR_POINT, channel.c_str(), dst.data(), dst.size());

            std::vector<float> src(static_cast<std::size_t>(grid2->npoints()), 0.0f);
            grid2->get_float_attr(ATTR_POINT, channel.c_str(), src.data(), src.size());
            sample2d_attr<float>(coords, src, dst, nx, ny, h, bmin, false);

            grid->delete_attr(ATTR_POINT, channel.c_str());
            grid->create_attr_by_float(ATTR_POINT, channel.c_str(), dst.data(), dst.size());
        } else if (attr_t == "vec3") {
            if (!grid->has_attr(ATTR_POINT, channel.c_str(), ATTR_VEC3) ||
                !grid2->has_attr(ATTR_POINT, channel.c_str(), ATTR_VEC3)) {
                nd->set_output_object("prim", grid);
                return ZErr_OK;
            }
            std::vector<Vec3f> dst_abi(static_cast<std::size_t>(npts));
            grid->get_vec3f_attr(ATTR_POINT, channel.c_str(), dst_abi.data(), dst_abi.size());
            std::vector<glm::vec3> dst(dst_abi.size());
            for (std::size_t i = 0; i < dst_abi.size(); ++i) {
                dst[i] = glm::vec3(dst_abi[i].x, dst_abi[i].y, dst_abi[i].z);
            }

            std::vector<Vec3f> src_abi(static_cast<std::size_t>(grid2->npoints()));
            grid2->get_vec3f_attr(ATTR_POINT, channel.c_str(), src_abi.data(), src_abi.size());
            std::vector<glm::vec3> src(src_abi.size());
            for (std::size_t i = 0; i < src_abi.size(); ++i) {
                src[i] = glm::vec3(src_abi[i].x, src_abi[i].y, src_abi[i].z);
            }
            sample2d_attr<glm::vec3>(coords, src, dst, nx, ny, h, bmin, false);

            for (std::size_t i = 0; i < dst_abi.size(); ++i) {
                dst_abi[i] = Vec3f{dst[i].x, dst[i].y, dst[i].z};
            }
            grid->delete_attr(ATTR_POINT, channel.c_str());
            grid->create_attr_by_vec3(ATTR_POINT, channel.c_str(), dst_abi.data(), dst_abi.size());
        }

        nd->set_output_object("prim", grid);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Grid2DSample_M,
    Z_INPUTS(
        {"grid", _gParamType_Geometry},
        {"grid2", _gParamType_Geometry},
        {"nx", _gParamType_Int, ZInt(1)},
        {"ny", _gParamType_Int, ZInt(1)},
        {"h", _gParamType_Float, ZFloat(1.0f)},
        {"bmin", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"channel", _gParamType_String, ZString("pos")},
        {"sampleBy", _gParamType_String, ZString("pos")},
        {"attrT", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("vec3", "float")}
    ),
    Z_OUTPUTS(
        {"prim", _gParamType_Geometry}
    ),
    "deprecated",
    "",
    "",
    ""
);

struct Grid2DSample_M2 : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        const int nx = nd->get_input2_int("nx");
        const int ny = nd->get_input2_int("ny");
        const Vec3f bmin_abi = nd->get_input2_vec3f("bmin");
        const glm::vec3 bmin(bmin_abi.x, bmin_abi.y, bmin_abi.z);
        const float h = nd->get_input2_float("h");
        const bool is_periodic = (get_input2_string(nd, "sampleType") == "Periodic");
        const std::string sample_by = get_input2_string(nd, "sampleBy");
        const std::string channel_list = get_input2_string(nd, "channel");

        auto* prim = nd->clone_input_Geometry("prim");
        auto* grid = nd->get_input_Geometry("sampleGrid");

        const int npts = prim->npoints();
        std::vector<Vec3f> coords_abi(static_cast<std::size_t>(npts));
        if (sample_by == "pos") {
            prim->get_vec3f_attr(ATTR_POINT, "pos", coords_abi.data(), coords_abi.size());
        } else {
            if (!prim->has_attr(ATTR_POINT, sample_by.c_str(), ATTR_VEC3)) {
                throw std::runtime_error("[sampleBy] has to be a vec3f attribute!");
            }
            prim->get_vec3f_attr(ATTR_POINT, sample_by.c_str(), coords_abi.data(), coords_abi.size());
        }
        std::vector<glm::vec3> coords(static_cast<std::size_t>(npts));
        for (std::size_t i = 0; i < coords_abi.size(); ++i) {
            coords[i] = glm::vec3(coords_abi[i].x, coords_abi[i].y, coords_abi[i].z);
        }

        const std::vector<std::string> channels = parse_channels(grid, channel_list);
        for (const auto& ch : channels) {
            if (ch == "pos") continue;
            if (grid->has_attr(ATTR_POINT, ch.c_str(), ATTR_VEC3)) {
                std::vector<Vec3f> grid_attr_abi(static_cast<std::size_t>(grid->npoints()));
                grid->get_vec3f_attr(ATTR_POINT, ch.c_str(), grid_attr_abi.data(), grid_attr_abi.size());
                std::vector<glm::vec3> grid_attr(grid_attr_abi.size());
                for (std::size_t i = 0; i < grid_attr_abi.size(); ++i) {
                    grid_attr[i] = glm::vec3(grid_attr_abi[i].x, grid_attr_abi[i].y, grid_attr_abi[i].z);
                }

                std::vector<Vec3f> prim_attr_abi(static_cast<std::size_t>(npts));
                if (prim->has_attr(ATTR_POINT, ch.c_str(), ATTR_VEC3)) {
                    prim->get_vec3f_attr(ATTR_POINT, ch.c_str(), prim_attr_abi.data(), prim_attr_abi.size());
                } else {
                    for (auto& v : prim_attr_abi) v = Vec3f{0.0f, 0.0f, 0.0f};
                }
                std::vector<glm::vec3> prim_attr(prim_attr_abi.size());
                for (std::size_t i = 0; i < prim_attr_abi.size(); ++i) {
                    prim_attr[i] = glm::vec3(prim_attr_abi[i].x, prim_attr_abi[i].y, prim_attr_abi[i].z);
                }
                sample2d_attr<glm::vec3>(coords, grid_attr, prim_attr, nx, ny, h, bmin, is_periodic);
                for (std::size_t i = 0; i < prim_attr_abi.size(); ++i) {
                    prim_attr_abi[i] = Vec3f{prim_attr[i].x, prim_attr[i].y, prim_attr[i].z};
                }
                prim->delete_attr(ATTR_POINT, ch.c_str());
                prim->create_attr_by_vec3(ATTR_POINT, ch.c_str(), prim_attr_abi.data(), prim_attr_abi.size());
            } else if (grid->has_attr(ATTR_POINT, ch.c_str(), ATTR_FLOAT)) {
                std::vector<float> grid_attr(static_cast<std::size_t>(grid->npoints()), 0.0f);
                grid->get_float_attr(ATTR_POINT, ch.c_str(), grid_attr.data(), grid_attr.size());

                std::vector<float> prim_attr(static_cast<std::size_t>(npts), 0.0f);
                if (prim->has_attr(ATTR_POINT, ch.c_str(), ATTR_FLOAT)) {
                    prim->get_float_attr(ATTR_POINT, ch.c_str(), prim_attr.data(), prim_attr.size());
                }
                sample2d_attr<float>(coords, grid_attr, prim_attr, nx, ny, h, bmin, is_periodic);
                prim->delete_attr(ATTR_POINT, ch.c_str());
                prim->create_attr_by_float(ATTR_POINT, ch.c_str(), prim_attr.data(), prim_attr.size());
            } else {
                throw std::runtime_error("only support vec3f or float in Grid2DSample_M2");
            }
        }

        nd->set_output_object("prim", prim);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Grid2DSample_M2,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"sampleGrid", _gParamType_Geometry},
        {"nx", _gParamType_Int, ZInt(1)},
        {"ny", _gParamType_Int, ZInt(1)},
        {"h", _gParamType_Float, ZFloat(1.0f)},
        {"bmin", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"channel", _gParamType_String, ZString("*")},
        {"sampleBy", _gParamType_String, ZString("pos")},
        {"sampleType", _gParamType_String, ZString("Clamp"), Combobox, Z_STRING_ARRAY("Clamp", "Periodic")}
    ),
    Z_OUTPUTS(
        {"prim", _gParamType_Geometry}
    ),
    "zenofx",
    "",
    "",
    ""
);

} // namespace zeno

