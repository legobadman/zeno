#include <Windows.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <inodedata.h>
#include <inodeimpl.h>
#include <zcommon.h>

#include "ImageObject.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <tinygltf/stb_image.h>
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

namespace zeno::zs_imgcv {

namespace {

static bool ends_with_ci(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) {
        return false;
    }
    const std::size_t off = s.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        char a = s[off + i];
        char b = suffix[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
        if (a != b) {
            return false;
        }
    }
    return true;
}

static std::string get_input2_string(INodeData* ptrNodeData, const char* name) {
    char buf[1024] = {};
    ptrNodeData->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static std::unique_ptr<ImageObject> read_image_stb_f32(const std::string& path) {
    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf(path.c_str(), &w, &h, &n, 0);
    if (!data) {
        throw std::runtime_error("cannot open image file: " + path);
    }

    auto img = std::make_unique<ImageObject>(w, h, (n == 4) ? 4 : 3, 0.0f);
    auto& dst = img->raw();
    const std::size_t pixels = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    if (n == 1) {
        for (std::size_t i = 0; i < pixels; ++i) {
            dst[i * 3 + 0] = data[i];
            dst[i * 3 + 1] = data[i];
            dst[i * 3 + 2] = data[i];
        }
    } else if (n == 2) {
        for (std::size_t i = 0; i < pixels; ++i) {
            dst[i * 3 + 0] = data[i * 2 + 0];
            dst[i * 3 + 1] = data[i * 2 + 1];
            dst[i * 3 + 2] = 0.0f;
        }
    } else if (n == 3) {
        std::memcpy(dst.data(), data, pixels * 3 * sizeof(float));
    } else if (n == 4) {
        std::memcpy(dst.data(), data, pixels * 4 * sizeof(float));
    } else {
        stbi_image_free(data);
        throw std::runtime_error("unsupported channel count");
    }
    stbi_image_free(data);
    return img;
}

static std::unique_ptr<ImageObject> read_exr_file(const std::string& path) {
    int nx = 0, ny = 0;
    float* rgba = nullptr;
    const char* err = nullptr;
    const int ret = LoadEXR(&rgba, &nx, &ny, path.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        std::string msg = "load exr failed";
        if (err != nullptr) {
            msg += ": ";
            msg += err;
            FreeEXRErrorMessage(err);
        }
        throw std::runtime_error(msg);
    }
    nx = std::max(nx, 1);
    ny = std::max(ny, 1);
    auto img = std::make_unique<ImageObject>(nx, ny, 4, 0.0f);
    auto& dst = img->raw();
    const std::size_t pixels = static_cast<std::size_t>(nx) * static_cast<std::size_t>(ny);
    for (std::size_t i = 0; i < pixels; ++i) {
        dst[i * 4 + 0] = rgba[i * 4 + 0];
        dst[i * 4 + 1] = rgba[i * 4 + 1];
        dst[i * 4 + 2] = rgba[i * 4 + 2];
        dst[i * 4 + 3] = rgba[i * 4 + 3];
    }
    free(rgba);
    return img;
}

static std::unique_ptr<ImageObject> read_pfm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open pfm file: " + path);
    }
    std::string format;
    int nx = 0;
    int ny = 0;
    float scale = 0.0f;
    file >> format >> nx >> ny >> scale;
    file.ignore(1);
    if (format != "PF" || nx <= 0 || ny <= 0) {
        throw std::runtime_error("invalid pfm file: " + path);
    }
    auto img = std::make_unique<ImageObject>(nx, ny, 3, 0.0f);
    auto& dst = img->raw();
    file.read(reinterpret_cast<char*>(dst.data()), static_cast<std::streamsize>(dst.size() * sizeof(float)));
    if (!file) {
        throw std::runtime_error("pfm read failed: " + path);
    }
    return img;
}

} // namespace

struct ReadImageFile : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        const std::string path = get_input2_string(ptrNodeData, "path");
        const bool linearize = ptrNodeData->get_input2_bool("Linearize Non-linear Images");
        std::unique_ptr<ImageObject> image;

        if (ends_with_ci(path, ".exr")) {
            image = read_exr_file(path);
        } else if (ends_with_ci(path, ".pfm")) {
            image = read_pfm_file(path);
        } else {
            image = read_image_stb_f32(path);
            if (!linearize) {
                auto& data = image->raw();
                const int channels = image->channels();
                for (std::size_t i = 0; i < image->pixel_count(); ++i) {
                    const std::size_t base = i * static_cast<std::size_t>(channels);
                    for (int c = 0; c < std::min(channels, 3); ++c) {
                        data[base + static_cast<std::size_t>(c)] =
                            std::pow(data[base + static_cast<std::size_t>(c)], 1.0f / 2.2f);
                    }
                }
            }
        }

        ptrNodeData->set_output_object("image", image.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ReadImageFile,
    Z_INPUTS(
        {"path", _gParamType_String, ZString(""), ReadPathEdit},
        {"Linearize Non-linear Images", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS(
        {"image", gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

} // namespace zeno::zs_imgcv
