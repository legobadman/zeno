#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <inodedata.h>
#include <inodeimpl.h>
#include <zcommon.h>

#include "ImageObject.h"

namespace zeno::zs_imgcv {

namespace {

static std::string get_input2_string(INodeData* ptrNodeData, const char* name) {
    char buf[1024] = {};
    ptrNodeData->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static ImageObject* require_image_input(INodeData* ptrNodeData, const char* name) {
    IObject2* obj = ptrNodeData->get_input_object(name);
    auto* image = dynamic_cast<ImageObject*>(obj);
    if (image == nullptr) {
        throw std::runtime_error(std::string("input is not ImageObject: ") + name);
    }
    return image;
}

static glm::vec3 to_glm3(const Vec3f& v) {
    return glm::vec3(v.x, v.y, v.z);
}

static Vec3f to_abi3(const glm::vec3& v) {
    return Vec3f{v.x, v.y, v.z};
}

static glm::vec2 to_glm2(const Vec2f& v) {
    return glm::vec2(v.x, v.y);
}

static glm::vec2 clamp01(glm::vec2 uv) {
    uv.x = std::clamp(uv.x, 0.0f, 1.0f);
    uv.y = std::clamp(uv.y, 0.0f, 1.0f);
    return uv;
}

static glm::vec3 read_rgb(const ImageObject& image, int x, int y) {
    const int w = image.width();
    const int h = image.height();
    const int ch = image.channels();
    const auto& data = image.raw();
    const int cx = std::clamp(x, 0, w - 1);
    const int cy = std::clamp(y, 0, h - 1);
    const std::size_t base =
        (static_cast<std::size_t>(cy) * static_cast<std::size_t>(w) + static_cast<std::size_t>(cx)) *
        static_cast<std::size_t>(ch);
    const float r = data[base + 0];
    const float g = (ch > 1) ? data[base + 1] : r;
    const float b = (ch > 2) ? data[base + 2] : r;
    return glm::vec3(r, g, b);
}

static glm::ivec2 uv_to_tex_repeat(const glm::vec2& texCoord, int w, int h) {
    int x = static_cast<int>(texCoord.x * static_cast<float>(w) - 0.5f) % w;
    int y = static_cast<int>(texCoord.y * static_cast<float>(h) - 0.5f) % h;
    x = (x < 0) ? (w + x) : x;
    y = (y < 0) ? (h + y) : y;
    return glm::ivec2(x, y);
}

static glm::vec3 sample_linear_repeat(const glm::vec2& uv, const ImageObject& image) {
    const int w = image.width();
    const int h = image.height();
    glm::vec2 tc = uv * glm::vec2(static_cast<float>(w), static_cast<float>(h)) - glm::vec2(0.5f);
    glm::vec2 f = glm::fract(tc);
    int x0 = static_cast<int>(std::floor(tc.x));
    int y0 = static_cast<int>(std::floor(tc.y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    x0 = (x0 % w + w) % w;
    x1 = (x1 % w + w) % w;
    y0 = (y0 % h + h) % h;
    y1 = (y1 % h + h) % h;
    glm::vec3 s1 = read_rgb(image, x0, y0);
    glm::vec3 s2 = read_rgb(image, x1, y0);
    glm::vec3 s3 = read_rgb(image, x0, y1);
    glm::vec3 s4 = read_rgb(image, x1, y1);
    return glm::mix(glm::mix(s1, s2, f.x), glm::mix(s3, s4, f.x), f.y);
}

static glm::vec3 sample_linear_clamp(glm::vec2 uv, const ImageObject& image) {
    uv = clamp01(uv);
    return sample_linear_repeat(uv, image);
}

static glm::vec3 sample_nearest_repeat(const glm::vec2& uv, const ImageObject& image) {
    const glm::ivec2 tex = uv_to_tex_repeat(uv, image.width(), image.height());
    return read_rgb(image, tex.x, tex.y);
}

static glm::vec3 sample_nearest_clamp(glm::vec2 uv, const ImageObject& image) {
    uv = clamp01(uv);
    const glm::ivec2 tex = uv_to_tex_repeat(uv, image.width(), image.height());
    return read_rgb(image, tex.x, tex.y);
}

static glm::vec3 transform_uv(glm::mat4 const& matrix, const glm::vec3& uv3) {
    const glm::vec4 v4 = matrix * glm::vec4(uv3, 1.0f);
    return glm::vec3(v4) / std::max(v4.w, 1e-8f);
}

} // namespace

struct PrimSample2D : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* prim = ptrNodeData->clone_input_Geometry("prim");
        auto* image = require_image_input(ptrNodeData, "image");
        const std::string srcChannel = get_input2_string(ptrNodeData, "uvChannel");
        const std::string uvSource = get_input2_string(ptrNodeData, "uvSource");
        const std::string dstChannel = get_input2_string(ptrNodeData, "targetChannel");
        const std::string wrap = get_input2_string(ptrNodeData, "wrap");
        const std::string filter = get_input2_string(ptrNodeData, "filter");
        const Vec3f borderColorAbi = ptrNodeData->get_input2_vec3f("borderColor");
        const glm::vec3 borderColor = to_glm3(borderColorAbi);
        const bool invertU = ptrNodeData->get_input2_bool("invert U");
        const bool invertV = ptrNodeData->get_input2_bool("invert V");
        const float scale = ptrNodeData->get_input2_float("scale");
        const float rotate = ptrNodeData->get_input2_float("rotate");
        const glm::vec2 translate = to_glm2(ptrNodeData->get_input2_vec2f("translate"));

        glm::vec3 pre_scale(scale, scale, 0.0f);
        if (invertU) pre_scale.x *= -1.0f;
        if (invertV) pre_scale.y *= -1.0f;
        const glm::mat4 matScal = glm::scale(pre_scale);
        const glm::mat4 matRot = glm::rotate(static_cast<float>(rotate * 3.14159265358979323846 / 180.0),
                                             glm::vec3(0, 0, 1));
        const glm::mat4 matTrans = glm::translate(glm::vec3(translate.x, translate.y, 0));
        const glm::mat4 matrix =
            glm::translate(glm::vec3(0.5f, 0.5f, 0.0f)) * matTrans * matRot * matScal *
            glm::translate(glm::vec3(-0.5f, -0.5f, 0.0f));

        const int nPts = prim->npoints();
        std::vector<Vec3f> clrs(static_cast<std::size_t>(nPts), Vec3f{0, 0, 0});

        auto sample_color = [&](const glm::vec3& uv3) -> glm::vec3 {
            const glm::vec2 uv(uv3.x, uv3.y);
            const bool outOfRange = (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f);
            if (filter == "nearest") {
                if (wrap == "REPEAT") return sample_nearest_repeat(uv, *image);
                if (wrap == "CLAMP_TO_EDGE") return sample_nearest_clamp(uv, *image);
                if (wrap == "CLAMP_TO_BORDER") return outOfRange ? borderColor : sample_nearest_clamp(uv, *image);
            } else {
                if (wrap == "REPEAT") return sample_linear_repeat(uv, *image);
                if (wrap == "CLAMP_TO_EDGE") return sample_linear_clamp(uv, *image);
                if (wrap == "CLAMP_TO_BORDER") return outOfRange ? borderColor : sample_linear_clamp(uv, *image);
            }
            throw std::runtime_error("unknown wrap/filter");
        };

        if (uvSource == "vertex") {
            std::vector<Vec3f> uv(static_cast<std::size_t>(nPts));
            prim->get_vec3f_attr(ATTR_POINT, srcChannel.c_str(), uv.data(), uv.size());
#pragma omp parallel for
            for (int i = 0; i < nPts; ++i) {
                const glm::vec3 coord = transform_uv(matrix, to_glm3(uv[static_cast<std::size_t>(i)]));
                clrs[static_cast<std::size_t>(i)] = to_abi3(sample_color(coord));
            }
        } else if (uvSource == "tris") {
            const int nFaces = prim->nfaces();
            std::vector<Vec3f> uv0(static_cast<std::size_t>(nFaces));
            std::vector<Vec3f> uv1(static_cast<std::size_t>(nFaces));
            std::vector<Vec3f> uv2(static_cast<std::size_t>(nFaces));
            prim->get_vec3f_attr(ATTR_FACE, "uv0", uv0.data(), uv0.size());
            prim->get_vec3f_attr(ATTR_FACE, "uv1", uv1.data(), uv1.size());
            prim->get_vec3f_attr(ATTR_FACE, "uv2", uv2.data(), uv2.size());
#pragma omp parallel for
            for (int i = 0; i < nFaces; ++i) {
                int pts[8] = {};
                const std::size_t count = prim->face_points(i, pts, 8);
                if (count < 3) {
                    continue;
                }
                const glm::vec3 c0 = transform_uv(matrix, to_glm3(uv0[static_cast<std::size_t>(i)]));
                const glm::vec3 c1 = transform_uv(matrix, to_glm3(uv1[static_cast<std::size_t>(i)]));
                const glm::vec3 c2 = transform_uv(matrix, to_glm3(uv2[static_cast<std::size_t>(i)]));
                clrs[static_cast<std::size_t>(pts[0])] = to_abi3(sample_color(c0));
                clrs[static_cast<std::size_t>(pts[1])] = to_abi3(sample_color(c1));
                clrs[static_cast<std::size_t>(pts[2])] = to_abi3(sample_color(c2));
            }
        } else if (uvSource == "loopsuv") {
            throw std::runtime_error("don't support loopsuv right now");
        } else {
            throw std::runtime_error("unknown uvSource");
        }

        if (prim->has_point_attr(dstChannel.c_str())) {
            prim->delete_attr(ATTR_POINT, dstChannel.c_str());
        }
        prim->create_attr_by_vec3(ATTR_POINT, dstChannel.c_str(), clrs.data(), clrs.size());
        ptrNodeData->set_output_object("outPrim", prim);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(PrimSample2D,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"image", gParamType_ImageObject},
        {"uvChannel", _gParamType_String, ZString("uv")},
        {"uvSource", _gParamType_String, ZString("vertex"), Combobox, Z_STRING_ARRAY("vertex", "tris", "loopsuv")},
        {"targetChannel", _gParamType_String, ZString("clr")},
        {"remapMin", _gParamType_Float, ZFloat(0.0f)},
        {"remapMax", _gParamType_Float, ZFloat(1.0f)},
        {"wrap", _gParamType_String, ZString("REPEAT"), Combobox, Z_STRING_ARRAY("REPEAT", "CLAMP_TO_EDGE", "CLAMP_TO_BORDER")},
        {"filter", _gParamType_String, ZString("nearest"), Combobox, Z_STRING_ARRAY("nearest", "linear")},
        {"borderColor", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"invert U", _gParamType_Bool, ZInt(0)},
        {"invert V", _gParamType_Bool, ZInt(0)},
        {"scale", _gParamType_Float, ZFloat(1.0f)},
        {"rotate", _gParamType_Float, ZFloat(0.0f)},
        {"translate", _gParamType_Vec2f, ZVec2f(0.0f, 0.0f)}
    ),
    Z_OUTPUTS(
        {"outPrim", _gParamType_Geometry}
    ),
    "primitive",
    "",
    "",
    ""
);

} // namespace zeno::zs_imgcv
