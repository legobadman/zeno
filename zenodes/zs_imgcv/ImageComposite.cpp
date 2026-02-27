#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <inodedata.h>
#include <inodeimpl.h>
#include <zcommon.h>

#include "ImageObject.h"


namespace zeno {

namespace {
static std::string get_input2_string(INodeData* ptrNodeData, const char* name) {
    char buf[256] = {};
    ptrNodeData->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static zs_imgcv::ImageObject* require_image_input(INodeData* ptrNodeData, const char* name) {
    IObject2* obj = ptrNodeData->get_input_object(name);
    auto* image = dynamic_cast<zs_imgcv::ImageObject*>(obj);
    if (image == nullptr) {
        throw std::runtime_error(std::string("input is not ImageObject: ") + name);
    }
    return image;
}

static glm::vec3 read_rgb(const zs_imgcv::ImageObject& image, int idx) {
    const auto& data = image.raw();
    const int channels = image.channels();
    if (channels <= 0) {
        return glm::vec3(0.0f);
    }
    const std::size_t base = static_cast<std::size_t>(idx) * static_cast<std::size_t>(channels);
    const float r = data[base + 0];
    const float g = channels > 1 ? data[base + 1] : r;
    const float b = channels > 2 ? data[base + 2] : r;
    return glm::vec3(r, g, b);
}

static float read_alpha(const zs_imgcv::ImageObject& image, int idx) {
    const int channels = image.channels();
    if (channels < 4) {
        return 1.0f;
    }
    const auto& data = image.raw();
    const std::size_t base = static_cast<std::size_t>(idx) * static_cast<std::size_t>(channels);
    return data[base + 3];
}

static void write_rgb(zs_imgcv::ImageObject& image, int idx, const glm::vec3& rgb) {
    auto& data = image.raw();
    const int channels = image.channels();
    if (channels <= 0) {
        return;
    }
    const std::size_t base = static_cast<std::size_t>(idx) * static_cast<std::size_t>(channels);
    data[base + 0] = rgb[0];
    if (channels > 1) data[base + 1] = rgb[1];
    if (channels > 2) data[base + 2] = rgb[2];
}

static void write_alpha(zs_imgcv::ImageObject& image, int idx, float alpha) {
    if (image.channels() < 4) {
        return;
    }
    auto& data = image.raw();
    const std::size_t base = static_cast<std::size_t>(idx) * static_cast<std::size_t>(image.channels());
    data[base + 3] = alpha;
}

template <class T>
static T BlendMode(const float &alpha1, const float &alpha2, const T& rgb1, const T& rgb2, const T& background, const glm::vec3 opacity, std::string compmode)//rgb1 and background is premultiplied?
{
        if(compmode == std::string("Copy")) {//copy and over is different!
                T value = rgb1 * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Over")) {
                T value = (rgb1 + background * (1 - alpha1)) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Under")) {
                T value = (background + rgb1 * (1 - alpha2)) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Atop")) {
                T value = (rgb1 * alpha2 + background * (1 - alpha1)) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("In")) {
                T value = rgb1 * alpha2 * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Out")) {
                T value = (rgb1 * (1 - alpha2)) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Xor")) {
                T value = (rgb1 * (1 - alpha2) + background * (1 - alpha1)) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Add")) {
                T value = (rgb1 + background) * opacity[0] + rgb2 * (1 - opacity[0]);//clamp?
                return value;
        }
        else if(compmode == std::string("Subtract")) {
                T value = (background - rgb1) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Multiply")) {
                T value = rgb1 * background * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Max(Lighten)")) {
                T value = glm::max(rgb1, background) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Min(Darken)")) {
                T value = glm::min(rgb1, background) * opacity[0] + rgb2 * (1 - opacity[0]);
                return value;
        }
        else if(compmode == std::string("Screen")) {//A+B-AB if A and B between 0-1, else A if A>B else B
                    T value = (T(1) - (T(1) - background) * (T(1) - rgb1)) * opacity[0] + rgb2 * (1 - opacity[0]);//only care 0-1!
                    return value;
        }
        else if(compmode == std::string("Difference")) {
                    T value = glm::abs(rgb1 - background) * opacity[0] + rgb2 * (1 - opacity[0]);
                    return value;
        }
        else if(compmode == std::string("Average")) {
                    T value = (rgb1 + background) * 0.5f * opacity[0] + rgb2 * (1 - opacity[0]);
                    return value;
        }
        return T(0);
}

static glm::vec3 BlendModeV(const float &alpha1, const float &alpha2, const glm::vec3& rgb1, const glm::vec3& rgb2, const glm::vec3& background, const glm::vec3 opacity, std::string compmode)
{
        if(compmode == std::string("Overlay")) {
                    glm::vec3 value;
                    for (int k = 0; k < 3; k++) {
                        if (rgb2[k] < 0.5) {
                            value[k] = 2 * rgb1[k] * background[k];
                        } else {
                            value[k] = 1 - 2 * (1 - rgb1[k]) * (1 - background[k]);//screen
                        }
                    }
                    value = value * opacity[0] + rgb2 * (1 - opacity[0]);
                    return value;
        }
        else if(compmode == std::string("SoftLight")) {
                    glm::vec3 value;
                    for (int k = 0; k < 3; k++) {
                        if (rgb1[k] < 0.5) {
                            value[k] = 2 * rgb1[k] * background[k] + background[k] * background[k] * (1 - 2 * rgb1[k]);
                        } else {
                            value[k] = 2 * background[k] * (1 - rgb1[k]) + sqrt(background[k]) * (2 * rgb1[k] - 1);
                        }
                    }
                    /*for (int k = 0; k < 3; k++) {     Nuke method
                        if (rgb1[k] * rgb2[k] < 1) {
                            value[k] = rgb2[k] * (2 * rgb1[k] + (rgb2[k] * (1-rgb1[k] * rgb2[k])));
                        } else {
                            value[k] = 2 * rgb2[k] * rgb1[k];
                        }
                    }*/
                    value = value * opacity[0] + rgb2 * (1 - opacity[0]);
                    return value;
        }
        else if(compmode == std::string("Divide")) {
                    glm::vec3 value;
                    for (int k = 0; k < 3; k++) {
                        if (rgb1[k] == 0) {
                            value[k] = 1;
                        } else {
                            value[k] = background[k] / rgb1[k];
                        }
                    }
                    value = value * opacity[0] + rgb2 * (1 - opacity[0]);
                    return value;
        }
        return glm::vec3(0.0f);
}

struct Blend : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* blend = require_image_input(ptrNodeData, "Foreground");
        auto* base = require_image_input(ptrNodeData, "Background");
        const float maskopacity = ptrNodeData->get_input2_float("Mask Opacity");
        const std::string compmode = get_input2_string(ptrNodeData, "Blending Mode");
        const std::string alphamode = get_input2_string(ptrNodeData, "Alpha Mode");
        const float opacity1 = ptrNodeData->get_input2_float("Foreground Opacity");
        const float opacity2 = ptrNodeData->get_input2_float("Background Opacity");

        if (blend->width() != base->width() || blend->height() != base->height()) {
            throw std::runtime_error("Foreground/Background size mismatch");
        }

        const int w = base->width();
        const int h = base->height();
        const int imagesize = w * h;
        zs_imgcv::ImageObject* mask = nullptr;
        if (ptrNodeData->has_link_input("Mask")) {
            mask = require_image_input(ptrNodeData, "Mask");
            if (mask->width() != w || mask->height() != h) {
                throw std::runtime_error("Mask size mismatch");
            }
        }

        const bool alphaoutput = blend->channels() >= 4 || base->channels() >= 4;
        auto output = std::make_unique<zs_imgcv::ImageObject>(w, h, alphaoutput ? 4 : 3, 0.0f);

#pragma omp parallel for
        for (int i = 0; i < imagesize; i++) {
            const glm::vec3 foreground = read_rgb(*blend, i) * opacity1;
            const glm::vec3 rgb2 = read_rgb(*base, i);
            const glm::vec3 background = rgb2 * opacity2;
            const float maskv = mask ? std::clamp(read_rgb(*mask, i)[0], 0.0f, 1.0f) : 1.0f;
            const glm::vec3 opacity = glm::vec3(maskv * maskopacity);
            const float alpha1 = std::clamp(read_alpha(*blend, i) * opacity1, 0.0f, 1.0f);
            const float alpha2 = std::clamp(read_alpha(*base, i) * opacity2, 0.0f, 1.0f);

            glm::vec3 c(0.0f);
            if (compmode == "Overlay" || compmode == "SoftLight" || compmode == "Divide") {
                c = BlendModeV(alpha1, alpha2, foreground, rgb2, background, opacity, compmode);
            } else {
                c = BlendMode<glm::vec3>(alpha1, alpha2, foreground, rgb2, background, opacity, compmode);
            }
            write_rgb(*output, i, c);
            if (alphaoutput) {
                const float alpha = BlendMode<float>((alpha1), alpha2, (alpha1), alpha2, (alpha2), opacity, alphamode);
                write_alpha(*output, i, alpha);
            }
        }

        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Blend,
    Z_INPUTS(
        {"Foreground", zs_imgcv::gParamType_ImageObject},
        {"Background", zs_imgcv::gParamType_ImageObject},
        {"Mask", zs_imgcv::gParamType_ImageObject},
        {"Blending Mode", _gParamType_String, ZString("Over"), Combobox, Z_STRING_ARRAY("Over", "Copy", "Under", "Atop", "In", "Out", "Screen", "Add", "Subtract", "Multiply", "Max(Lighten)", "Min(Darken)", "Average", "Difference", "Overlay", "SoftLight", "Divide", "Xor")},
        {"Alpha Mode", _gParamType_String, ZString("Over"), Combobox, Z_STRING_ARRAY("Over", "Under", "Atop", "In", "Out", "Screen", "Add", "Subtract", "Multiply", "Max(Lighten)", "Min(Darken)", "Average", "Difference", "Xor")},
        {"Mask Opacity", _gParamType_Float, ZFloat(1.0f)},
        {"Foreground Opacity", _gParamType_Float, ZFloat(1.0f)},
        {"Background Opacity", _gParamType_Float, ZFloat(1.0f)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "comp",
    "",
    "",
    ""
);

// 自定义卷积核
std::vector<std::vector<float>> createKernel(float blurValue,
                                             float l_blurValue, float r_blurValue,
                                             float t_blurValue, float b_blurValue,
                                             float lt_blurValue, float rt_blurValue,
                                             float lb_blurValue, float rb_blurValue) {
    std::vector<std::vector<float>> kernel;
    kernel = {{lt_blurValue, t_blurValue, rt_blurValue},
              {l_blurValue, blurValue, r_blurValue},
              {lb_blurValue, b_blurValue, rb_blurValue}};
    return kernel;
}

struct CompBlur : INode2 {//TODO::delete
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* image = require_image_input(ptrNodeData, "image");
        const int s = ptrNodeData->get_input2_int("strength");
        const auto ktop = ptrNodeData->get_input2_vec3f("kerneltop");
        const auto kmid = ptrNodeData->get_input2_vec3f("kernelmid");
        const auto kbot = ptrNodeData->get_input2_vec3f("kernelbot");

        const int w = image->width();
        const int h = image->height();
        const int channels = image->channels();
        if (w <= 0 || h <= 0 || channels <= 0) {
            throw std::runtime_error("invalid image shape");
        }

        auto output = std::make_unique<zs_imgcv::ImageObject>(w, h, channels, 0.0f);
        std::vector<float> src = image->raw();
        std::vector<float> dst(src.size(), 0.0f);

        const std::vector<std::vector<float>> k = createKernel(
            kmid.y, kmid.x, kmid.z, ktop.y, kbot.y, ktop.x, ktop.z, kbot.x, kbot.z);

        for (int iter = 0; iter < s; iter++) {
#pragma omp parallel for
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    const bool border = (x == 0 || x == w - 1 || y == 0 || y == h - 1);
                    for (int c = 0; c < std::min(channels, 3); c++) {
                        float sum = 0.0f;
                        if (border) {
                            sum = src[(y * w + x) * channels + c];
                        } else {
                            for (int i = 0; i < 3; i++) {
                                for (int j = 0; j < 3; j++) {
                                    const int kernelX = x + j - 1;
                                    const int kernelY = y + i - 1;
                                    sum += src[(kernelY * w + kernelX) * channels + c] * k[i][j];
                                }
                            }
                        }
                        dst[(y * w + x) * channels + c] = sum;
                    }
                    for (int c = 3; c < channels; c++) {
                        dst[(y * w + x) * channels + c] = src[(y * w + x) * channels + c];
                    }
                }
            }
            src.swap(dst);
        }

        output->raw() = std::move(src);
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CompBlur,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"strength", _gParamType_Int, ZInt(5)},
        {"kerneltop", _gParamType_Vec3f, ZVec3f(0.075f, 0.124f, 0.075f)},
        {"kernelmid", _gParamType_Vec3f, ZVec3f(0.124f, 0.204f, 0.124f)},
        {"kernelbot", _gParamType_Vec3f, ZVec3f(0.075f, 0.124f, 0.075f)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "deprecated",
    "",
    "",
    ""
);

struct ImageExtractChannel : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* image = require_image_input(ptrNodeData, "image");
        const std::string channel = get_input2_string(ptrNodeData, "channel");
        const int w = image->width();
        const int h = image->height();
        const int in_channels = image->channels();

        auto output = std::make_unique<zs_imgcv::ImageObject>(w, h, 3, 0.0f);
        const int N = w * h;
        for (int i = 0; i < N; i++) {
            const glm::vec3 rgb = read_rgb(*image, i);
            float v = 0.0f;
            if (channel == "R") v = rgb[0];
            else if (channel == "G") v = rgb[1];
            else if (channel == "B") v = rgb[2];
            else if (channel == "A") {
                if (in_channels < 4) {
                    throw std::runtime_error("image has no alpha channel");
                }
                v = read_alpha(*image, i);
            }
            write_rgb(*output, i, glm::vec3(v));
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};
ZENDEFNODE_ABI(ImageExtractChannel,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"channel", _gParamType_String, ZString("R"), Combobox, Z_STRING_ARRAY("R", "G", "B", "A")}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

/* 导入地形网格的属性，可能会有多个属性。它将地形的属性转换为图
像，每个属性对应一个图层。 */

struct CompImport : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto prim = ptrNodeData->get_input_Geometry("prim");
        auto* ud = prim->userData();
        const int nx = ud->has("nx") ? ud->get_int("nx") : ud->get_int("w");
        const int ny = ud->has("ny") ? ud->get_int("ny") : ud->get_int("h");
        const std::string attrName = get_input2_string(ptrNodeData, "attrName");
        const auto remapRange = ptrNodeData->get_input2_vec2f("RemapRange");
        const bool remap = ptrNodeData->get_input2_bool("Remap");
        const std::string attributesType = get_input2_string(ptrNodeData, "AttributesType");

        auto image = std::make_unique<zs_imgcv::ImageObject>(nx, ny, 3, 0.0f);

        if (attributesType == "float") {
            if (!prim->has_point_attr(attrName.c_str())) {
                throw std::runtime_error("No such attribute in prim");
            }
            std::vector<float> attr(static_cast<std::size_t>(nx) * static_cast<std::size_t>(ny), 0.0f);
            prim->get_float_attr(ATTR_POINT, attrName.c_str(), attr.data(), attr.size());
            const auto [minit, maxit] = std::minmax_element(attr.begin(), attr.end());
            const float minresult = *minit;
            const float maxresult = *maxit;

            for (auto i = 0; i < nx * ny; i++) {
                float v = attr[i];
                if (remap && maxresult > minresult) {
                    v = (v - minresult) / (maxresult - minresult);
                    v = v * (remapRange.y - remapRange.x) + remapRange.x;
                }
                write_rgb(*image, i, glm::vec3(v));
            }
        }
        else if (attributesType == "vec3f") {
            if (!prim->has_point_attr(attrName.c_str())) {
                throw std::runtime_error("No such attribute in prim");
            }
            std::vector<Vec3f> attr(static_cast<std::size_t>(nx) * static_cast<std::size_t>(ny));
            prim->get_vec3f_attr(ATTR_POINT, attrName.c_str(), attr.data(), attr.size());
            for (auto i = 0; i < nx * ny; i++) {
                write_rgb(*image, i, glm::vec3(attr[i].x, attr[i].y, attr[i].z));
            }
        }

        ptrNodeData->set_output_object("image", image.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CompImport,
    Z_INPUTS(
        {"prim", _gParamType_Geometry},
        {"attrName", _gParamType_String, ZString("")},
        {"Remap", _gParamType_Bool, ZInt(0)},
        {"RemapRange", _gParamType_Vec2f, ZVec2f(0.0f, 1.0f)},
        {"AttributesType", _gParamType_String, ZString("float"), Combobox, Z_STRING_ARRAY("float", "vec3f")}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "comp",
    "",
    "",
    ""
);
//TODO::Channel shuffle、RGBA Shuffle

}
}