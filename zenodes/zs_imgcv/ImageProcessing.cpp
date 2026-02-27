#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <glm/glm.hpp>
#include <Windows.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <zcommon.h>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "ImageObject.h"
#include <tuple>

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

static void RGBtoHSV(float r, float g, float b, float &h, float &s, float &v) {
    float rd = r;
    float gd = g;
    float bd = b;
    float cmax = fmax(rd, fmax(gd, bd));
    float cmin = fmin(rd, fmin(gd, bd));
    float delta = cmax - cmin;

    if (delta != 0) {
        if (cmax == rd) {
            h = fmod((gd - bd) / delta, 6.0);
        } else if (cmax == gd) {
            h = (bd - rd) / delta + 2.0;
        } else if (cmax == bd) {
            h = (rd - gd) / delta + 4.0;
        }
        h *= 60.0;
        if (h < 0) {
            h += 360.0;
        }
    }
    s = (cmax != 0) ? delta / cmax : 0.0;
    v = cmax;
}

static glm::vec3 RGBtoXYZ(glm::vec3 rgb) {//INPUT RANGE 0-1
    float r = rgb[0];
    float g = rgb[1];
    float b = rgb[2];
    if (r > 0.04045) {
        r = pow((r + 0.055) / 1.055, 2.4);
    } else {
        r = r / 12.92;
    }
    if (g > 0.04045) {
        g = pow((g + 0.055) / 1.055, 2.4);
    } else {
        g = g / 12.92;
    }
    if (b > 0.04045) {
        b = pow((b + 0.055) / 1.055, 2.4);
    } else {
        b = b / 12.92;
    }
    r *= 100;
    g *= 100;
    b *= 100;
    return glm::vec3(0.412453f * r + 0.357580f * g + 0.180423f * b, 0.212671f * r + 0.715160f * g + 0.072169f * b, 0.019334f * r + 0.119193f * g + 0.950227f * b);
}

static glm::vec3 XYZtoRGB(glm::vec3 xyz) {//OUTPUT RANGE 0-1
    float x = xyz[0];
    float y = xyz[1];
    float z = xyz[2];
    x /= 100;
    y /= 100;
    z /= 100;
    float r = x * 3.2406 + y * -1.5372 + z * -0.4986;
    float g = x * -0.9689 + y * 1.8758 + z * 0.0415;
    float b = x * 0.0557 + y * -0.2040 + z * 1.0570;
    if (r > 0.0031308) {
        r = 1.055 * pow(r, 1 / 2.4) - 0.055;
    } else {
        r = 12.92 * r;
    }
    if (g > 0.0031308) {
        g = 1.055 * pow(g, 1 / 2.4) - 0.055;
    } else {
        g = 12.92 * g;
    }
    if (b > 0.0031308) {
        b = 1.055 * pow(b, 1 / 2.4) - 0.055;
    } else {
        b = 12.92 * b;
    }
    return glm::vec3(r, g, b);
}

static glm::vec3 XYZtoLab(glm::vec3 xyz) {
    float x = xyz[0];
    float y = xyz[1];
    float z = xyz[2];
    x /= 95.047;
    y /= 100;
    z /= 108.883;
    if (x > 0.008856) {
        x = pow(x, 1.0 / 3.0);
    } else {
        x = (7.787 * x) + (16.0 / 116.0);
    }
    if (y > 0.008856) {
        y = pow(y, 1.0 / 3.0);
    } else {
        y = (7.787 * y) + (16.0 / 116.0);
    }
    if (z > 0.008856) {
        z = pow(z, 1.0 / 3.0);
    } else {
        z = (7.787 * z) + (16.0 / 116.0);
    }
    return glm::vec3((116 * y) - 16, 500 * (x - y), 200 * (y - z));
}

static glm::vec3 LabtoXYZ(glm::vec3 lab) {
    float l = lab[0];
    float a = lab[1];
    float b = lab[2];
    float y = (l + 16) / 116;
    float x = a / 500 + y;
    float z = y - b / 200;
    if (pow(y, 3) > 0.008856) {
        y = pow(y, 3);
    } else {
        y = (y - 16.0 / 116.0) / 7.787;
    }
    if (pow(x, 3) > 0.008856) {
        x = pow(x, 3);
    } else {
        x = (x - 16.0 / 116.0) / 7.787;
    }
    if (pow(z, 3) > 0.008856) {
        z = pow(z, 3);
    } else {
        z = (z - 16.0 / 116.0) / 7.787;
    }
    return glm::vec3(x * 95.047f, y * 100.0f, z * 108.883f);
}

static glm::vec3 RGBtoLab(glm::vec3 rgb) {//input range 0-1
    return XYZtoLab(RGBtoXYZ(rgb));
}

static glm::vec3 LabtoRGB(glm::vec3 lab) {//output range 0-1
    return XYZtoRGB(LabtoXYZ(lab));
}

static void HSVtoRGB(float h, float s, float v, float &r, float &g, float &b)
{
    int i;
    float f, p, q, t;
    if( s == 0 ) {
        // achromatic (grey)
        r = g = b = v;
        return;
    }
    h /= 60;            // sector 0 to 5
    i = floor( h );
    f = h - i;          // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:        // case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
}


/*struct ImageResize: INode {//TODO::FIX BUG
    void apply() override {
        std::shared_ptr<PrimitiveObject> image = get_input<PrimitiveObject>("image");
        int width = get_input2_int("width");
        int height = get_input2_int("height");
        auto &ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        auto image2 = std::make_shared<PrimitiveObject>();
        image2->verts.resize(width * height);
        image2->userData().set2("isImage", 1);
        image2->userData().set2("w", width);
        image2->userData().set2("h", height);
        if(image->has_attr("alpha")){
            image2->verts.add_attr<float>("alpha");
        }

        float scaleX = static_cast<float>(w) / width;
        float scaleY = static_cast<float>(h) / height;

        for (auto a = 0; a < image->verts.size(); a++){
            int x = a / w;
            int y = a % w;
            int srcX = static_cast<int>(x * scaleX);
            int srcY = static_cast<int>(y * scaleY);
            image2->verts[y * width + x] = image->verts[srcY * w + srcX];
            image2->verts.attr<float>("alpha")[y * width + x] = image->verts.attr<float>("alpha")[srcY * w + srcX];
        }set_output("image", std::move(image2)));
    }
};
ZENDEFNODE(ImageResize, {
    {
        {"image"},
        {gParamType_Int, "width", "1024"},
        {gParamType_Int, "height", "1024"},
    },
    {
        {"image"},
    },
    {},
    {"image"},
});*/

#if 0
void rotateimage(std::shared_ptr<PrimitiveObject> src, std::shared_ptr<PrimitiveObject> & dst, float angle, bool balpha) {

    double radians = angle * 3.14159 / 180.0;
    int width = src->userData()->get_int("w");
    int height = src->userData()->get_int("h");

    int centerX = width / 2;
    int centerY = height / 2;

    int rotatedWidth = static_cast<int>(std::abs(width * cos(radians)) + std::abs(height * sin(radians)));
    int rotatedHeight = static_cast<int>(std::abs(height * cos(radians)) + std::abs(width * sin(radians)));

    dst->verts.resize(rotatedWidth * rotatedHeight);
    dst->userData().set2("w", rotatedWidth);
    dst->userData().set2("h", rotatedHeight);
    if(src->verts.has_attr("alpha")){
        dst->verts.add_attr<float>("alpha");
        if(balpha){
            for(int i = 0;i<dst->size();i++){
                dst->verts.attr<float>("alpha")[i] = 0;
            }
        }
        else{
            for(int i = 0;i<dst->size();i++){
                dst->verts.attr<float>("alpha")[i] = 1;
            }
        }
        for (int y = 0; y < rotatedHeight; ++y) {
            for (int x = 0; x < rotatedWidth; ++x) {

                int srcX = static_cast<int>((x - rotatedWidth / 2) * cos(-radians) - (y - rotatedHeight / 2) * sin(-radians) + centerX);
                int srcY = static_cast<int>((x - rotatedWidth / 2) * sin(-radians) + (y - rotatedHeight / 2) * cos(-radians) + centerY);

                if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                    dst->verts[y * rotatedWidth + x] = src->verts[srcY * width + srcX] ;
                    dst->verts.attr<float>("alpha")[y * rotatedWidth + x] = src->verts.attr<float>("alpha")[srcY * width + srcX];
                }
            }
        }
    }
    else{
        dst->verts.add_attr<float>("alpha");
        if(balpha){
            for(int i = 0;i<dst->size();i++){
                dst->verts.attr<float>("alpha")[i] = 0;
            }
        }
        else{
            for(int i = 0;i<dst->size();i++){
                dst->verts.attr<float>("alpha")[i] = 1;
            }
        }
        for (int y = 0; y < rotatedHeight; ++y) {
            for (int x = 0; x < rotatedWidth; ++x) {
                int srcX = static_cast<int>((x - rotatedWidth / 2) * cos(-radians) - (y - rotatedHeight / 2) * sin(-radians) + centerX);
                int srcY = static_cast<int>((x - rotatedWidth / 2) * sin(-radians) + (y - rotatedHeight / 2) * cos(-radians) + centerY);

                if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                    dst->verts[y * rotatedWidth + x] = src->verts[srcY * width + srcX] ;
                    dst->verts.attr<float>("alpha")[y * rotatedWidth + x] = 1;
                }
            }
        }
    }
    for (int y = 0; y < rotatedHeight; ++y) {
        for (int x = 0; x < rotatedWidth; ++x) {
            int srcX = static_cast<int>((x - rotatedWidth / 2) * cos(-radians) - (y - rotatedHeight / 2) * sin(-radians) + centerX);
            int srcY = static_cast<int>((x - rotatedWidth / 2) * sin(-radians) + (y - rotatedHeight / 2) * cos(-radians) + centerY);

            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                dst->verts[y * rotatedWidth + x] = src->verts[srcY * width + srcX] ;
            }
        }
    }
}
struct ImageRotate: INode {//TODO::transform and rorate
    void apply() override {
        std::shared_ptr<PrimitiveObject> image = get_input<PrimitiveObject>("image");
        auto balpha = get_input2_bool("alpha");
        float rotate = get_input2<float>("rotate");
        auto &ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        auto image2 = std::make_shared<PrimitiveObject>();
        image2->verts.resize(w * h);
        image2->userData().set2("isImage", 1);
        rotateimage(image,image2,rotate,balpha);
        auto &ud2 = image2->userData();
        w = ud2->get_int("w");
        h = ud2->get_int("h");set_output("image", std::move(image2)));
    }
};
ZENDEFNODE(ImageRotate, {
    {
        {gParamType_Primitive, "image"},
        {gParamType_Float, "rotate", "0.0"},
        {gParamType_Bool, "alpha", "0"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    {"image"},
});
#endif

struct ImageFlip : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        const bool fliphori = ptrNodeData->get_input2_bool("Flip Horizontally");
        const bool flipvert = ptrNodeData->get_input2_bool("Flip Vertically");
        auto* input = require_image_input(ptrNodeData, "image");
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);

        const int w = output->width();
        const int h = output->height();
        const int channels = output->channels();
        auto& data = output->raw();
        if (w <= 0 || h <= 0 || channels <= 0) {
            ptrNodeData->set_output_object("image", output.release());
            return ZErr_OK;
        }

        auto flip_horizontal = [&](std::vector<float>& buf) {
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w / 2; x++) {
                    const int lx = x;
                    const int rx = w - 1 - x;
                    for (int c = 0; c < channels; c++) {
                        std::swap(buf[(y * w + lx) * channels + c], buf[(y * w + rx) * channels + c]);
                    }
                }
            }
        };

        auto flip_vertical = [&](std::vector<float>& buf) {
            for (int y = 0; y < h / 2; y++) {
                const int ty = y;
                const int by = h - 1 - y;
                for (int x = 0; x < w; x++) {
                    for (int c = 0; c < channels; c++) {
                        std::swap(buf[(ty * w + x) * channels + c], buf[(by * w + x) * channels + c]);
                    }
                }
            }
        };

        if (fliphori) {
            flip_horizontal(data);
        }
        if (flipvert) {
            flip_vertical(data);
        }

        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageFlip,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"Flip Horizontally", _gParamType_Bool, ZInt(0)},
        {"Flip Vertically", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

#if 0
struct ImageRGB2HSV : INode {
    virtual void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        float H = 0, S = 0, V = 0;
        for (auto i = 0; i < image->verts.size(); i++){
            float R = image->verts[i][0];
            float G = image->verts[i][1];
            float B = image->verts[i][2];
            zeno::RGBtoHSV(R, G, B, H, S, V);
            image->verts[i][0]= H;
            image->verts[i][1]= S;
            image->verts[i][2]= V;
        }set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageRGB2HSV, {
    {
        {gParamType_Primitive, "image"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    { "image" },
});

struct ImageHSV2RGB : INode {
    virtual void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        float R = 0, G = 0, B = 0;
        for (auto i = 0; i < image->verts.size(); i++){
            float H = image->verts[i][0];
            float S = image->verts[i][1];
            float V = image->verts[i][2];
            zeno::HSVtoRGB(H, S, V, R, G, B);
            image->verts[i][0]= R ;
            image->verts[i][1]= G ;
            image->verts[i][2]= B ;
        }set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageHSV2RGB, {
    {
        {gParamType_Primitive, "image"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    { "image" },
});

struct ImageEditHSV : INode {//TODO::FIX BUG
    virtual void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        float H = 0, S = 0, V = 0;
        float Hi = get_input2<float>("H");
        float Si = get_input2<float>("S");
        float Vi = get_input2<float>("V");
        for (auto i = 0; i < image->verts.size(); i++) {
            float R = image->verts[i][0];
            float G = image->verts[i][1];
            float B = image->verts[i][2];
            zeno::RGBtoHSV(R, G, B, H, S, V);
            //S = S + (S - 0.5)*(Si-1);
            //V = V + (V - 0.5)*(Vi-1);
            //S = S + (Si - 1) * (S < 0.5 ? S : 1.0 - S);
            //V = V + (Vi - 1) * (V < 0.5 ? V : 1.0 - V);
            H = fmod(H + Hi, 360.0);
            S = S * Si;
            V = V * Vi;
            zeno::HSVtoRGB(H, S, V, R, G, B);
            image->verts[i][0] = R;
            image->verts[i][1] = G;
            image->verts[i][2] = B;
        }set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageEditHSV, {
    {
        {gParamType_Primitive, "image"},
        {gParamType_Float, "H", "0"},
        {gParamType_Float, "S", "1"},
        {gParamType_Float, "V", "1"},
    },
    {
        {gParamType_Primitive, "image"}
    },
    {},
    { "image" },
});
#endif


float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2 * sigma * sigma));
}

// MedianBlur
void MedianBlur(zs_imgcv::ImageObject& image, zs_imgcv::ImageObject& imagetmp, int width, int height, int kernel_size) {
    using KernelRGB = std::tuple<float, float, float>;
    std::vector<KernelRGB> kernel_values(static_cast<std::size_t>(kernel_size * kernel_size));
    const int channels = image.channels();
    const auto& src = image.raw();
    auto& dst = imagetmp.raw();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t cur = (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * channels;
            const float current_value0 = src[cur + 0];
            const float current_value1 = channels > 1 ? src[cur + 1] : current_value0;
            const float current_value2 = channels > 2 ? src[cur + 2] : current_value0;

            for (int ky = 0; ky < kernel_size; ++ky) {
                for (int kx = 0; kx < kernel_size; ++kx) {
                    const int px = x - kernel_size / 2 + kx;
                    const int py = y - kernel_size / 2 + ky;
                    if (px < 0 || px >= width || py < 0 || py >= height) {
                        kernel_values[static_cast<std::size_t>(ky * kernel_size + kx)] = {current_value0, current_value1, current_value2};
                    } else {
                        const std::size_t pbase =
                            (static_cast<std::size_t>(py) * width + static_cast<std::size_t>(px)) * channels;
                        kernel_values[static_cast<std::size_t>(ky * kernel_size + kx)] = {
                            src[pbase + 0],
                            channels > 1 ? src[pbase + 1] : src[pbase + 0],
                            channels > 2 ? src[pbase + 2] : src[pbase + 0]
                        };
                    }
                }
            }

            std::sort(kernel_values.begin(), kernel_values.end());
            const std::size_t mid = static_cast<std::size_t>(kernel_size * kernel_size / 2);
            dst[cur + 0] = std::get<0>(kernel_values[mid]);
            if (channels > 1) dst[cur + 1] = std::get<1>(kernel_values[mid]);
            if (channels > 2) dst[cur + 2] = std::get<2>(kernel_values[mid]);
            for (int c = 3; c < channels; ++c) {
                dst[cur + static_cast<std::size_t>(c)] = src[cur + static_cast<std::size_t>(c)];
            }
        }
    }
}

float bilateral(float src, float dst, float sigma_s, float sigma_r) {
    return gaussian(src - dst, sigma_s) * gaussian(abs(src - dst), sigma_r);
}

void bilateralFilter(zs_imgcv::ImageObject& image, zs_imgcv::ImageObject& imagetmp, int width, int height, float sigma_s, float sigma_r) {
    const int k = static_cast<int>(std::ceil(3 * sigma_s));
    const int channels = image.channels();
    const auto& src = image.raw();
    auto& dst = imagetmp.raw();
    dst = src; // keep border unchanged

    for (int i = k; i < height - k; i++) {
        for (int j = k; j < width - k; j++) {
            float sum0 = 0, sum1 = 0, sum2 = 0;
            float wsum0 = 0, wsum1 = 0, wsum2 = 0;
            const std::size_t cur = (static_cast<std::size_t>(i) * width + static_cast<std::size_t>(j)) * channels;
            for (int m = -k; m <= k; m++) {
                for (int n = -k; n <= k; n++) {
                    const std::size_t nb =
                        (static_cast<std::size_t>(i + m) * width + static_cast<std::size_t>(j + n)) * channels;
                    const float w0 = bilateral(src[cur + 0], src[nb + 0], sigma_s, sigma_r);
                    const float w1 = bilateral(channels > 1 ? src[cur + 1] : src[cur + 0],
                                               channels > 1 ? src[nb + 1] : src[nb + 0], sigma_s, sigma_r);
                    const float w2 = bilateral(channels > 2 ? src[cur + 2] : src[cur + 0],
                                               channels > 2 ? src[nb + 2] : src[nb + 0], sigma_s, sigma_r);

                    sum0 += w0 * src[nb + 0];
                    sum1 += w1 * (channels > 1 ? src[nb + 1] : src[nb + 0]);
                    sum2 += w2 * (channels > 2 ? src[nb + 2] : src[nb + 0]);

                    wsum0 += w0;
                    wsum1 += w1;
                    wsum2 += w2;
                }
            }
            dst[cur + 0] = sum0 / std::max(wsum0, 1e-8f);
            if (channels > 1) dst[cur + 1] = sum1 / std::max(wsum1, 1e-8f);
            if (channels > 2) dst[cur + 2] = sum2 / std::max(wsum2, 1e-8f);
            for (int c = 3; c < channels; ++c) {
                dst[cur + static_cast<std::size_t>(c)] = src[cur + static_cast<std::size_t>(c)];
            }
        }
    }
}


///////////////////////////
std::vector<int> boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
	auto wIdeal = sqrt((12 * sigma*sigma / n) + 1);  // Ideal averaging filter width
	int wl = floor(wIdeal);
	if (wl % 2 == 0)
		wl--;
	int wu = wl + 2;

	float mIdeal = (12 * sigma*sigma - n * wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
	int m = round(mIdeal);
	// var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	std::vector<int> sizes(n);
	for (auto i = 0; i < n; i++)
		sizes[i] = i < m ? wl : wu;
	return sizes;
}

void boxBlurH(std::vector<glm::vec3>& scl, std::vector<glm::vec3>& tcl, int w, int h, int r) {
	float iarr = 1.f / (r + r + 1);
    #pragma omp parallel for
	for (int i = 0; i < h; i++) {
		int ti = i * w, li = ti, ri = ti + r;
		auto fv = scl[ti], lv = scl[ti + w - 1];
		auto val = fv * static_cast<float>(r + 1);
		for (int j = 0; j < r; j++) val += scl[ti + j];
		for (int j = 0; j <= r; j++, ri++, ti++) { val += scl[ri] - fv;   tcl[ti] = val*iarr; }
		for (int j = r + 1; j < w - r; j++, ri++, ti++, li++) { val += scl[ri] - scl[li];   tcl[ti] = val*iarr; }
		for (int j = w - r; j < w; j++, ti++, li++) { val += lv - scl[li];   tcl[ti] = val*iarr; }//border?
	}
}
void boxBlurT(std::vector<glm::vec3>& scl, std::vector<glm::vec3>& tcl, int w, int h, int r) {
	float iarr = 1.f / (r + r + 1);// radius range on either side of a pixel + the pixel itself
    #pragma omp parallel for
	for (auto i = 0; i < w; i++) {
		int ti = i, li = ti, ri = ti + r * w;
		auto fv = scl[ti], lv = scl[ti + w * (h - 1)];
		auto val = fv * static_cast<float>(r + 1);
		for (int j = 0; j < r; j++) val += scl[ti + j * w];
		for (int j = 0; j <= r; j++, ri+=w, ti+=w) { val += scl[ri] - fv;  tcl[ti] = val*iarr; }
		for (int j = r + 1; j < h - r; j++, ri+=w, ti+=w, li+=w) { val += scl[ri] - scl[li];  tcl[ti] = val*iarr; }
		for (int j = h - r; j < h; j++, ti+=w, li+=w) { val += lv - scl[li];  tcl[ti] = val*iarr; }
	}
}
void boxBlur(std::vector<glm::vec3>& scl, std::vector<glm::vec3>& tcl, int w, int h, int r) {
    std::swap(scl, tcl);
	boxBlurH(tcl, scl, w, h, r);
	boxBlurT(scl, tcl, w, h, r);
}
void gaussBlur(std::vector<glm::vec3> scl, std::vector<glm::vec3>& tcl, int w, int h, float sigma, int blurNumber) {
	auto bxs = boxesForGauss(sigma, blurNumber);
	boxBlur(scl, tcl, w, h, (bxs[0] - 1) / 2);
	boxBlur(tcl, scl, w, h, (bxs[1] - 1) / 2);
	boxBlur(scl, tcl, w, h, (bxs[2] - 1) / 2);
    /*for (auto i = 0; i < blurNumber; i++) {
        boxBlur(scl, tcl, w, h, (bxs[i] - 1) / 2);
    }*/
}

struct ImageBlur : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* image = require_image_input(ptrNodeData, "image");
        int kernelSize = ptrNodeData->get_input2_int("kernelSize");
        const std::string type = get_input2_string(ptrNodeData, "type");
        const bool fastgaussian = ptrNodeData->get_input2_bool("Fast Blur(Gaussian)");
        const float sigmaX = ptrNodeData->get_input2_float("GaussianSigma");
        const auto bilateral = ptrNodeData->get_input2_vec2f("BilateralSigma");
        const float sigmaColor = bilateral.x;
        const float sigmaSpace = bilateral.y;

        const int w = image->width();
        const int h = image->height();
        const int channels = image->channels();
        auto output = std::make_unique<zs_imgcv::ImageObject>(*image);

        std::vector<glm::vec3> rgb_data(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
        {
            const auto& src = image->raw();
            for (int i = 0; i < w * h; ++i) {
                const std::size_t base = static_cast<std::size_t>(i) * static_cast<std::size_t>(channels);
                const float r = src[base + 0];
                const float g = channels > 1 ? src[base + 1] : r;
                const float b = channels > 2 ? src[base + 2] : r;
                rgb_data[static_cast<std::size_t>(i)] = glm::vec3(r, g, b);
            }
        }

        if (type == "Gaussian" && fastgaussian) {
            std::vector<glm::vec3> rgb_out(rgb_data.size(), glm::vec3(0.0f));
            gaussBlur(rgb_data, rgb_out, w, h, sigmaX, 3);
            auto& dst = output->raw();
            for (int i = 0; i < w * h; ++i) {
                const std::size_t base = static_cast<std::size_t>(i) * static_cast<std::size_t>(channels);
                dst[base + 0] = rgb_out[static_cast<std::size_t>(i)].x;
                if (channels > 1) dst[base + 1] = rgb_out[static_cast<std::size_t>(i)].y;
                if (channels > 2) dst[base + 2] = rgb_out[static_cast<std::size_t>(i)].z;
            }
        } else {
            cv::Mat imagecvin(h, w, CV_32FC3);
            cv::Mat imagecvout(h, w, CV_32FC3);
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    const auto& rgb = rgb_data[static_cast<std::size_t>(i * w + j)];
                    imagecvin.at<cv::Vec3f>(i, j) = {rgb.x, rgb.y, rgb.z};
                }
            }
            if (kernelSize % 2 == 0) {
                kernelSize += 1;
            }
            if (type == "Box") {
                cv::boxFilter(imagecvin, imagecvout, -1, cv::Size(kernelSize, kernelSize));
            } else if (type == "Gaussian") {
                cv::GaussianBlur(imagecvin, imagecvout, cv::Size(kernelSize, kernelSize), sigmaX);
            } else if (type == "Median") {
                cv::medianBlur(imagecvin, imagecvout, kernelSize);
            } else if (type == "Bilateral") {
                cv::bilateralFilter(imagecvin, imagecvout, kernelSize, sigmaColor, sigmaSpace);
            } else if (type == "Stack") {
                // keep legacy behavior: stack blur is not implemented
            } else {
                ptrNodeData->report_error("ImageBlur: Blur type does not exist");
                return ZErr_UnimplError;
            }

            auto& dst = output->raw();
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    const auto rgb = imagecvout.at<cv::Vec3f>(i, j);
                    const std::size_t base =
                        static_cast<std::size_t>(i * w + j) * static_cast<std::size_t>(channels);
                    dst[base + 0] = rgb[0];
                    if (channels > 1) dst[base + 1] = rgb[1];
                    if (channels > 2) dst[base + 2] = rgb[2];
                }
            }
        }

        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageBlur,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"type", _gParamType_String, ZString("Gaussian"), Combobox, Z_STRING_ARRAY("Gaussian", "Box", "Median", "Bilateral", "Stack")},
        {"kernelSize", _gParamType_Int, ZInt(5)},
        {"GaussianSigma", _gParamType_Float, ZFloat(3.0f)},
        {"BilateralSigma", _gParamType_Vec2f, ZVec2f(50.0f, 50.0f)},
        {"Fast Blur(Gaussian)", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageEditContrast : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        const float contrast_ratio = ptrNodeData->get_input2_float("ContrastRatio");
        const float contrast_center = ptrNodeData->get_input2_float("ContrastCenter");
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        auto& data = output->raw();
        const int channels = output->channels();
        for (std::size_t i = 0; i < output->pixel_count(); ++i) {
            const std::size_t base = i * static_cast<std::size_t>(channels);
            for (int c = 0; c < std::min(channels, 3); ++c) {
                data[base + static_cast<std::size_t>(c)] =
                    data[base + static_cast<std::size_t>(c)] +
                    (data[base + static_cast<std::size_t>(c)] - contrast_center) * (contrast_ratio - 1.0f);
            }
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageEditContrast,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"ContrastRatio", _gParamType_Float, ZFloat(1.0f)},
        {"ContrastCenter", _gParamType_Float, ZFloat(0.5f)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);


struct ImageEditInvert : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        auto& data = output->raw();
        const int channels = output->channels();
        for (std::size_t i = 0; i < output->pixel_count(); ++i) {
            const std::size_t base = i * static_cast<std::size_t>(channels);
            for (int c = 0; c < std::min(channels, 3); ++c) {
                data[base + static_cast<std::size_t>(c)] = 1.0f - data[base + static_cast<std::size_t>(c)];
            }
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};
ZENDEFNODE_ABI(ImageEditInvert,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

/* 将高度图像转换为法线贴图 */
struct ImageToNormalMap : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* image = require_image_input(ptrNodeData, "image");
        const float strength = ptrNodeData->get_input2_float("strength");
        const bool invertR = ptrNodeData->get_input2_bool("InvertR");
        const bool invertG = ptrNodeData->get_input2_bool("InvertG");
        const int w = image->width();
        const int h = image->height();
        const int channels = image->channels();
        const auto& src = image->raw();

        auto normalmap = std::make_unique<zs_imgcv::ImageObject>(w, h, 3, 0.0f);
        auto& dst = normalmap->raw();
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (i == 0 || i == h - 1 || j == 0 || j == w - 1) {
                    const std::size_t base = static_cast<std::size_t>(i * w + j) * 3;
                    dst[base + 0] = 0.0f;
                    dst[base + 1] = 0.0f;
                    dst[base + 2] = 1.0f;
                }
            }
        }

#pragma omp parallel for
        for (int i = 1; i < h - 1; i++) {
            for (int j = 1; j < w - 1; j++) {
                auto sample = [&](int y, int x) -> float {
                    return src[(static_cast<std::size_t>(y * w + x) * static_cast<std::size_t>(channels)) + 0];
                };
                float gx = -sample(i - 1, j - 1) + sample(i - 1, j + 1)
                         - 2.0f * sample(i, j - 1) + 2.0f * sample(i, j + 1)
                         - sample(i + 1, j - 1) + sample(i + 1, j + 1);
                float gy = sample(i - 1, j - 1) + 2.0f * sample(i - 1, j) + sample(i - 1, j + 1)
                         - sample(i + 1, j - 1) - 2.0f * sample(i + 1, j) - sample(i + 1, j + 1);
                gx *= strength;
                gy *= strength;
                glm::vec3 rgb(gx, gy, 1.0f);
                const float len = glm::length(rgb);
                if (len > 1e-8f) {
                    rgb /= len;
                } else {
                    rgb = glm::vec3(0.0f, 0.0f, 1.0f);
                }
                rgb = 0.5f * (rgb + 1.0f);
                if (invertG) {
                    rgb.y = 1.0f - rgb.y;
                } else if (invertR) {
                    rgb.x = 1.0f - rgb.x;
                }
                const std::size_t base = static_cast<std::size_t>(i * w + j) * 3;
                dst[base + 0] = rgb.x;
                dst[base + 1] = rgb.y;
                dst[base + 2] = rgb.z;
            }
        }
        ptrNodeData->set_output_object("image", normalmap.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageToNormalMap,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"strength", _gParamType_Float, ZFloat(10.0f)},
        {"InvertR", _gParamType_Bool, ZInt(0)},
        {"InvertG", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageGray : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        const std::string mode = get_input2_string(ptrNodeData, "mode");
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        auto& data = output->raw();
        const int channels = output->channels();
        for (std::size_t i = 0; i < output->pixel_count(); ++i) {
            const std::size_t base = i * static_cast<std::size_t>(channels);
            const float r = data[base + 0];
            const float g = channels > 1 ? data[base + 1] : r;
            const float b = channels > 2 ? data[base + 2] : r;
            float gray = r;
            if (mode == "Average") gray = (r + g + b) / 3.0f;
            else if (mode == "Luminance") gray = 0.3f * r + 0.59f * g + 0.11f * b;
            else if (mode == "Red") gray = r;
            else if (mode == "Green") gray = g;
            else if (mode == "Blue") gray = b;
            else if (mode == "MaxComponent") gray = std::max(r, std::max(g, b));
            else if (mode == "MinComponent") gray = std::min(r, std::min(g, b));
            data[base + 0] = gray;
            if (channels > 1) data[base + 1] = gray;
            if (channels > 2) data[base + 2] = gray;
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};
ZENDEFNODE_ABI(ImageGray,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"mode", _gParamType_String, ZString("Average"), Combobox, Z_STRING_ARRAY("Average", "Luminance", "Red", "Green", "Blue", "MaxComponent", "MinComponent")}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageTile : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        const std::string tilemode = get_input2_string(ptrNodeData, "tilemode");
        const int rows = ptrNodeData->get_input2_int("rows");
        const int cols = ptrNodeData->get_input2_int("cols");
        const int w = input->width();
        const int h = input->height();
        const int ch = input->channels();
        const int w1 = w * cols;
        const int h1 = h * rows;
        auto output = std::make_unique<zs_imgcv::ImageObject>(w1, h1, ch, 0.0f);
        const auto& src = input->raw();
        auto& dst = output->raw();

        for (int ty = 0; ty < rows; ++ty) {
            for (int tx = 0; tx < cols; ++tx) {
                for (int y = 0; y < h; ++y) {
                    for (int x = 0; x < w; ++x) {
                        int sx = x;
                        int sy = y;
                        if (tilemode == "mirror") {
                            if (tx % 2 == 1) sx = w - 1 - x;
                            if (ty % 2 == 1) sy = h - 1 - y;
                        }
                        const int dx = tx * w + x;
                        const int dy = ty * h + y;
                        const std::size_t sbase = (static_cast<std::size_t>(sy) * w + static_cast<std::size_t>(sx)) * ch;
                        const std::size_t dbase = (static_cast<std::size_t>(dy) * w1 + static_cast<std::size_t>(dx)) * ch;
                        for (int c = 0; c < ch; ++c) {
                            dst[dbase + static_cast<std::size_t>(c)] = src[sbase + static_cast<std::size_t>(c)];
                        }
                    }
                }
            }
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageTile,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"tilemode", _gParamType_String, ZString("normal"), Combobox, Z_STRING_ARRAY("normal", "mirror")},
        {"rows", _gParamType_Int, ZInt(2)},
        {"cols", _gParamType_Int, ZInt(2)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageDilate : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        const int strength = ptrNodeData->get_input2_int("strength");
        const int kwidth = ptrNodeData->get_input2_int("kernel_width");
        const int kheight = ptrNodeData->get_input2_int("kernel_height");
        const int w = input->width();
        const int h = input->height();
        const int ch = input->channels();
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        const auto& src = input->raw();
        auto& dst = output->raw();

        cv::Mat imagecvin(h, w, CV_32FC3);
        cv::Mat imagecvout(h, w, CV_32FC3);
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                const std::size_t base = (static_cast<std::size_t>(i) * w + static_cast<std::size_t>(j)) * ch;
                imagecvin.at<cv::Vec3f>(i, j) = {src[base + 0], src[base + 1], src[base + 2]};
            }
        }
        cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(kheight, kwidth));
        cv::dilate(imagecvin, imagecvout, kernel, cv::Point(-1, -1), strength);

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                const std::size_t base = (static_cast<std::size_t>(i) * w + static_cast<std::size_t>(j)) * ch;
                const cv::Vec3f rgb = imagecvout.at<cv::Vec3f>(i, j);
                dst[base + 0] = rgb[0];
                dst[base + 1] = rgb[1];
                dst[base + 2] = rgb[2];
            }
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageDilate,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"strength", _gParamType_Int, ZInt(1)},
        {"kernel_width", _gParamType_Int, ZInt(3)},
        {"kernel_height", _gParamType_Int, ZInt(3)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageErode : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        const int strength = ptrNodeData->get_input2_int("strength");
        const int kwidth = ptrNodeData->get_input2_int("kernel_width");
        const int kheight = ptrNodeData->get_input2_int("kernel_height");
        const int w = input->width();
        const int h = input->height();
        const int ch = input->channels();
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        const auto& src = input->raw();
        auto& dst = output->raw();

        cv::Mat imagecvin(h, w, CV_32FC3);
        cv::Mat imagecvout(h, w, CV_32FC3);
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                const std::size_t base = (static_cast<std::size_t>(i) * w + static_cast<std::size_t>(j)) * ch;
                imagecvin.at<cv::Vec3f>(i, j) = {src[base + 0], src[base + 1], src[base + 2]};
            }
        }
        cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(kheight, kwidth));
        cv::erode(imagecvin, imagecvout, kernel, cv::Point(-1, -1), strength);

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                const std::size_t base = (static_cast<std::size_t>(i) * w + static_cast<std::size_t>(j)) * ch;
                const cv::Vec3f rgb = imagecvout.at<cv::Vec3f>(i, j);
                dst[base + 0] = rgb[0];
                dst[base + 1] = rgb[1];
                dst[base + 2] = rgb[2];
            }
        }
        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageErode,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"strength", _gParamType_Int, ZInt(1)},
        {"kernel_width", _gParamType_Int, ZInt(3)},
        {"kernel_height", _gParamType_Int, ZInt(3)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

struct ImageColor : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        const auto color = ptrNodeData->get_input2_vec4f("Color");
        const auto size = ptrNodeData->get_input2_vec2f("Size");
        const bool balpha = ptrNodeData->get_input2_bool("alpha");
        const int w = size.x;
        const int h = size.y;
        const int channels = balpha ? 4 : 3;
        auto image = std::make_unique<zs_imgcv::ImageObject>(w, h, channels, 0.0f);
        auto& data = image->raw();
        for (int i = 0; i < w * h; ++i) {
            const std::size_t base = static_cast<std::size_t>(i) * channels;
            data[base + 0] = color.x;
            data[base + 1] = color.y;
            data[base + 2] = color.z;
            if (channels > 3) {
                data[base + 3] = color.w;
            }
        }
        ptrNodeData->set_output_object("image", image.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageColor,
    Z_INPUTS(
        {"Color", _gParamType_Vec4f, ZVec4f(1.0f, 1.0f, 1.0f, 1.0f)},
        {"Size", _gParamType_Vec2f, ZVec2f(1024.0f, 1024.0f)},
        {"alpha", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "deprecated",
    "",
    "",
    ""
);

/*
struct ImageColor2 : INode {
    virtual void apply() override {
        auto image = create_GeometryObject(true, );
        auto color = get_input2<glm::vec3>("Color");
        auto alpha = get_input2<float>("Alpha");
        auto size = get_input2<zeno::vec2i>("Size");
        auto balpha = get_input2_bool("alpha");
        auto vertsize = size[0] * size[1];
        image->verts.resize(vertsize);
        image->userData().set2("isImage", 1);
        image->userData().set2("w", size[0]);
        image->userData().set2("h", size[1]);
        auto imageverts = image->points_pos();
        if(balpha){
            auto &alphaAttr = image->verts.add_attr<float>("alpha");
            for (int i = 0; i < vertsize ; i++) {
                image->verts[i] = {color[0], color[1], color[2]};
                alphaAttr[i] = alpha;
            }
        }
        else{
            for (int i = 0; i < vertsize ; i++) {
                image->verts[i] = {color[0], color[1], color[2]};
            }
        }
        image->set_point_attr("pos", imageverts);set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageColor2, {
    {
        {gParamType_Vec3f, "Color", "1,1,1"},
        {gParamType_Float, "Alpha", "1"},
        {"vec2i", "Size", "1024,1024"},
        {gParamType_Bool, "alpha", "1"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    { "image" },
});

struct ImageClamp: INode {//Add Unpremultiplied Space Option?
    void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        auto background = get_input2_string("ClampedValue");
        auto &ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        auto up = get_input2<float>("Max");
        auto low = get_input2<float>("Min");
        auto imageverts = image->points_pos();

        if(background == "LimitValue"){
        for (auto i = 0; i < image->verts.size(); i++) {
            image->verts[i] = zeno::clamp(image->verts[i], low, up);
            }
        }
        else if(background == "Black"){
            for (auto i = 0; i < image->verts.size(); i++) {
                glm::vec3 &v = image->verts[i];
                for(int j = 0; j < 3; j++){
                    if((v[j]<low) || (v[j]>up)){
                        v[j] = 0;
                    }
                }
            }
        }
        else if(background == "White"){
            for (auto i = 0; i < image->verts.size(); i++) {
                glm::vec3 &v = image->verts[i];
                for(int j = 0; j < 3; j++){
                    if((v[j]<low) || (v[j]>up)){
                        v[j] = 1;
                    }
                }
            }
        }

        image->set_point_attr("pos", imageverts);set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageClamp, {
    {
        {gParamType_Primitive, "image"},
        {gParamType_Float, "Max", "1"},
        {gParamType_Float, "Min", "0"},
        {"enum LimitValue Black White", "ClampedValue", "LimitValue"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    {"image"},
});

struct ImageMatting: INode {
    virtual void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        auto &ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        auto imagemode = get_input2_string("imagemode");
        auto maskmode = get_input2_string("maskmode");

        if(imagemode == "origin"){
            if (!image->has_attr("alpha")) {
                image->verts.add_attr<float>("alpha");
                for (int i = 0; i < image->verts.size(); i++) {
                    image->verts.attr<float>("alpha")[i] = 1;
                }
            }
        }
        else if(imagemode == "deleteblack"){
            if (!image->has_attr("alpha")) {
                image->verts.add_attr<float>("alpha");
                for (int i = 0; i < image->verts.size(); i++) {
                    image->verts.attr<float>("alpha")[i] = 1;
                }
            }
            for (int i = 0; i < image->verts.size(); i++) {
                if(image->verts[i][0] <= 0.01 && image->verts[i][1] <= 0.01 &&
                   image->verts[i][2] <= 0.01 && image->verts.attr<float>("alpha")[i] != 0){
                    image->verts.attr<float>("alpha")[i] = 0;
                }
            }
        }
        else if(imagemode == "deletewhite"){
            if (!image->has_attr("alpha")) {
                image->verts.add_attr<float>("alpha");
                for (int i = 0; i < image->verts.size(); i++) {
                    image->verts.attr<float>("alpha")[i] = 1;
                }
            }
            for (int i = 0; i < image->verts.size(); i++) {
                if(image->verts[i][0] >= 0.99 && image->verts[i][1] >= 0.99 &&
                   image->verts[i][2] >= 0.99 && image->verts.attr<float>("alpha")[i] != 0){
                    image->verts.attr<float>("alpha")[i] = 0;
                }
            }
        }
        if (has_input("mask")) {
            auto gimage = get_input2<PrimitiveObject>("mask");
            auto &gud = gimage->userData();
            int wg = gud->get_int("w");
            int hg = gud->get_int("h");
            if (wg == w && hg == h) {
                if (maskmode == "black") {
                    if (gimage->verts.has_attr("alpha")) {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (gimage->verts.attr<float>("alpha")[i * w + j] != 0 &&
                                    image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    if (gimage->verts[i][0] <= 0.01 && gimage->verts[i][1] <= 0.01 &&
                                        gimage->verts[i][2] <= 0.01) {
                                        image->verts.attr<float>("alpha")[i * w + j] = 1;
                                    } else {
                                        image->verts.attr<float>("alpha")[i * w + j] = 0;
                                    }
                                } else {
                                    image->verts.attr<float>("alpha")[i * w + j] = 0;
                                }
                            }
                        }
                    } else {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    if (gimage->verts[i * w + j][0] == 0 && gimage->verts[i * w + j][1] == 0 &&
                                        gimage->verts[i * w + j][2] == 0) {
                                        image->verts.attr<float>("alpha")[i * w + j] = 1;
                                    } else {
                                        image->verts.attr<float>("alpha")[i * w + j] = 0;
                                    }
                                }
                            }
                        }
                    }
                } else if (maskmode == "white") {
                    if (gimage->verts.has_attr("alpha")) {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (gimage->verts.attr<float>("alpha")[i * w + j] != 0 &&
                                    image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    if (gimage->verts[i][0] >= 0.99 && gimage->verts[i][1] >= 0.99 &&
                                        gimage->verts[i][2] >= 0.99) {
                                        image->verts.attr<float>("alpha")[i * w + j] = 1;
                                    } else {
                                        image->verts.attr<float>("alpha")[i * w + j] = 0;
                                    }
                                } else {
                                    image->verts.attr<float>("alpha")[i * w + j] = 0;
                                }
                            }
                        }
                    } else {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    if (gimage->verts[i * w + j][0] == 0 && gimage->verts[i * w + j][1] == 0 &&
                                        gimage->verts[i * w + j][2] == 0) {
                                        image->verts.attr<float>("alpha")[i * w + j] = 1;
                                    } else {
                                        image->verts.attr<float>("alpha")[i * w + j] = 0;
                                    }
                                }
                            }
                        }
                    }
                } else if (maskmode == "gray_black") {
                    if (gimage->verts.has_attr("alpha")) {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (gimage->verts.attr<float>("alpha")[i * w + j] != 0 &&
                                    image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    image->verts.attr<float>("alpha")[i * w + j] = 1 - gimage->verts[i * w + j][0];
                                } else {
                                    image->verts.attr<float>("alpha")[i * w + j] = 0;
                                }
                            }
                        }
                    } else {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    image->verts.attr<float>("alpha")[i * w + j] = 1 - gimage->verts[i * w + j][0];
                                }
                            }
                        }
                    }
                } else if (maskmode == "gray_white") {
                    if (gimage->verts.has_attr("alpha")) {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (gimage->verts.attr<float>("alpha")[i * w + j] != 0 &&
                                    image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    image->verts.attr<float>("alpha")[i * w + j] = gimage->verts[i * w + j][0];
                                } else {
                                    image->verts.attr<float>("alpha")[i * w + j] = 0;
                                }
                            }
                        }
                    } else {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    image->verts.attr<float>("alpha")[i * w + j] = gimage->verts[i * w + j][0];
                                }
                            }
                        }
                    }
                }
                else if (maskmode == "alpha") {
                    if (gimage->verts.has_attr("alpha")) {
                        image->verts.attr<float>("alpha") = gimage->verts.attr<float>("alpha");
                    } else {
#pragma omp parallel for
                        for (int i = 0; i < h; i++) {
                            for (int j = 0; j < w; j++) {
                                if (image->verts.attr<float>("alpha")[i * w + j] != 0) {
                                    image->verts.attr<float>("alpha")[i * w + j] = 1;
                                }
                            }
                        }
                    }
                }
            }
            else if (wg < w && hg < h) {
            }
        }set_output("image", std::move(image)));
    }
};

ZENDEFNODE(ImageMatting, {
    {
        {gParamType_Primitive, "image"},
        {"mask"},
        {"enum origin deleteblack deletewhite", "imagemode", "origin"},
        {"enum gray_black gray_white black white alpha", "maskmode", "gray_black"},
    },
    {
        {gParamType_Primitive, "image"}
    },
    {},
    { "image" },
});*/

struct ImageLevels : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto* input = require_image_input(ptrNodeData, "image");
        auto output = std::make_unique<zs_imgcv::ImageObject>(*input);
        auto inputLevels = ptrNodeData->get_input2_vec2f("Input Levels");
        auto outputLevels = ptrNodeData->get_input2_vec2f("Output Levels");
        const float gamma = ptrNodeData->get_input2_float("gamma");
        const std::string channel = get_input2_string(ptrNodeData, "channel");
        const bool doClamp = ptrNodeData->get_input2_bool("Clamp Output");
        const bool autolevel = ptrNodeData->get_input2_bool("Auto Level");
        const int w = output->width();
        const int h = output->height();
        const int ch = output->channels();
        auto& data = output->raw();

        const float inputMin = inputLevels.x;
        const float inputRange = std::max(1e-6f, inputLevels.y - inputLevels.x);
        const float outputMin = outputLevels.x;
        const float outputRange = outputLevels.y - outputLevels.x;
        const float gammaCorrection = 1.0f / std::max(gamma, 1e-6f);

        auto apply_one = [&](float v, float inMin, float inRange) -> float {
            v = (v < inMin) ? inMin : v;
            v = (v - inMin) / std::max(inRange, 1e-6f);
            v = std::pow(v, gammaCorrection);
            v = v * outputRange + outputMin;
            return doClamp ? std::clamp(v, 0.0f, 1.0f) : v;
        };

        float minC[3] = {0, 0, 0};
        float maxC[3] = {1, 1, 1};
        if (autolevel) {
            std::vector<int> hist[3];
            hist[0].assign(256, 0);
            hist[1].assign(256, 0);
            hist[2].assign(256, 0);
            for (int i = 0; i < w * h; ++i) {
                const std::size_t base = static_cast<std::size_t>(i) * ch;
                hist[0][std::clamp(static_cast<int>(data[base + 0] * 255.99f), 0, 255)]++;
                hist[1][std::clamp(static_cast<int>(data[base + 1] * 255.99f), 0, 255)]++;
                hist[2][std::clamp(static_cast<int>(data[base + 2] * 255.99f), 0, 255)]++;
            }
            const int total = w * h;
            for (int c = 0; c < 3; ++c) {
                int sum = 0;
                int lo = 0, hi = 255;
                for (int i = 0; i < 256; ++i) {
                    sum += hist[c][i];
                    if (sum >= total * 0.001f) { lo = i; break; }
                }
                sum = 0;
                for (int i = 255; i >= 0; --i) {
                    sum += hist[c][i];
                    if (sum >= total * 0.001f) { hi = i; break; }
                }
                minC[c] = lo / 255.0f;
                maxC[c] = std::max((hi / 255.0f), minC[c] + 1e-6f);
            }
        }

#pragma omp parallel for
        for (int i = 0; i < w * h; ++i) {
            const std::size_t base = static_cast<std::size_t>(i) * ch;
            if (autolevel) {
                data[base + 0] = apply_one(data[base + 0], minC[0], maxC[0] - minC[0]);
                data[base + 1] = apply_one(data[base + 1], minC[1], maxC[1] - minC[1]);
                data[base + 2] = apply_one(data[base + 2], minC[2], maxC[2] - minC[2]);
            } else if (channel == "All") {
                data[base + 0] = apply_one(data[base + 0], inputMin, inputRange);
                data[base + 1] = apply_one(data[base + 1], inputMin, inputRange);
                data[base + 2] = apply_one(data[base + 2], inputMin, inputRange);
            } else if (channel == "R") {
                data[base + 0] = apply_one(data[base + 0], inputMin, inputRange);
            } else if (channel == "G") {
                data[base + 1] = apply_one(data[base + 1], inputMin, inputRange);
            } else if (channel == "B") {
                data[base + 2] = apply_one(data[base + 2], inputMin, inputRange);
            } else if (channel == "A" && ch > 3) {
                data[base + 3] = apply_one(data[base + 3], inputMin, inputRange);
            }
        }

        ptrNodeData->set_output_object("image", output.release());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImageLevels,
    Z_INPUTS(
        {"image", zs_imgcv::gParamType_ImageObject},
        {"Input Levels", _gParamType_Vec2f, ZVec2f(0.0f, 1.0f)},
        {"gamma", _gParamType_Float, ZFloat(1.0f)},
        {"Output Levels", _gParamType_Vec2f, ZVec2f(0.0f, 1.0f)},
        {"channel", _gParamType_String, ZString("All"), Combobox, Z_STRING_ARRAY("All", "R", "G", "B", "A")},
        {"Auto Level", _gParamType_Bool, ZInt(0)},
        {"Clamp Output", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS(
        {"image", zs_imgcv::gParamType_ImageObject}
    ),
    "image",
    "",
    "",
    ""
);

#if 0
struct ImageQuantization: INode {
    void apply() override {
        auto image = get_input<PrimitiveObject>("image");
        int clusternumber = get_input2_int("Number of Color");
        bool outputcenter = get_input2_bool("Output Cluster Centers");
        //int simplifyscale = get_input2_int("Image Simplification Scale");
        auto clusterattribute = get_input2_string("Cluster Attribute");
        UserData &ud = image->userData();
        int w = ud->get_int("w");
        int h = ud->get_int("h");
        auto &imagepos = image->verts.attr<glm::vec3>("pos");
        const int HistogramSize = 64 * 64 * 64;// (256/simplifyscale)  256 / 4 = 64
        std::vector<glm::vec3> labs(HistogramSize, glm::vec3(0.0f));
        std::vector<glm::vec3> seeds(clusternumber, glm::vec3(0.0f));
        std::vector<glm::vec3> newseeds(clusternumber, glm::vec3(0.0f));
        std::vector<int> weights(HistogramSize, 0);
        std::vector<int> seedsweight(clusternumber, 0);
        std::vector<int> clusterindices(HistogramSize, 0);
        #pragma omp parallel
        {
            std::vector<glm::vec3> local_labs(HistogramSize, glm::vec3(0.0f));
            std::vector<int> local_weights(HistogramSize, 0);

            #pragma omp for
            for (int i = 0; i < w * h; i++) {
                int index = ((zeno::clamp(int(imagepos[i][0] * 255.99), 0, 255) / 4) * 64
                + (zeno::clamp(int(imagepos[i][1] * 255.99), 0, 255) / 4)) * 64
                + (zeno::clamp(int(imagepos[i][2] * 255.99), 0, 255) / 4);

                local_labs[index] += RGBtoLab(imagepos[i]);
                local_weights[index]++;
            }

            #pragma omp critical
            {
                for (int i = 0; i < HistogramSize; i++) {
                    labs[i] += local_labs[i];
                    weights[i] += local_weights[i];
                }
            }
        }
        int maxindex = 0;
        glm::vec3 seedcolor;
        const int squaredSeparationCoefficient = 1400;// For photos, we can use a higher coefficient, from 900 to 6400
        auto weightclone = weights;
        for (int i = 0; i < clusternumber; i++) {
            maxindex = std::max_element(weightclone.begin(), weightclone.end()) - weightclone.begin();
            if(weightclone[maxindex] == 0){
                break;
            }
            seedcolor = labs[maxindex] / weights[maxindex];
            seeds[i] = seedcolor;
            weightclone[maxindex] = 0;
            for (int i = 0; i < HistogramSize; i++)
            {
                if(weightclone[i] > 0 ) {
                    weightclone[i] *= (1 - exp(-zeno::lengthSquared(seedcolor - labs[i] / weights[i]) / squaredSeparationCoefficient));
                    }
                }
            }
        bool optimumreached = false;
        while(!optimumreached){
            optimumreached = true;
            std::fill(newseeds.begin(), newseeds.end(), glm::vec3(0.0f));
            std::fill(seedsweight.begin(), seedsweight.end(), 0);
            #pragma omp parallel
            {
                std::vector<glm::vec3> local_newseeds(clusternumber, glm::vec3(0.0f));
                std::vector<int> local_seedsweight(clusternumber, 0);

                #pragma omp for
                for (int i = 0; i < HistogramSize; i++) {
                    if(weights[i] == 0){
                        continue;
                    }
                    //get closest seed index
                    int mindist = 100000000;
                    int clusterindex;
                    for(int j = 0; j < clusternumber; j++){
                        auto dist = zeno::lengthSquared(labs[i] / weights[i] - seeds[j]);
                        if(dist < mindist){
                            mindist = dist;
                            clusterindex = j;
                        }
                    }
                    if(clusterindices[i] != clusterindex && optimumreached){
                        optimumreached = false;
                    }
                    clusterindices[i] = clusterindex;
                    // Accumulate colors and weights per cluster.
                    local_newseeds[clusterindex] += labs[i];
                    local_seedsweight[clusterindex] += weights[i];
                }

                #pragma omp critical
                {
                    for (int i = 0; i < clusternumber; i++) {
                        newseeds[i] += local_newseeds[i];
                        seedsweight[i] += local_seedsweight[i];
                    }
                }
            }
            // Average accumulated colors to get new seeds.
            for (int i = 0; i < clusternumber; i++) {// update seeds
                if (seedsweight[i] == 0){
                    seeds[i] = glm::vec3(0.0f);
                }
                else{
                    seeds[i] = newseeds[i] / seedsweight[i];
                }
            }
        }
        auto &clusterattr = image->verts.add_attr<int>(clusterattribute);
        //export pallete
        if (outputcenter) {
            image->verts.resize(clusternumber);
            image->verts.update();
            image->userData().set2("w", clusternumber);
            image->userData().set2("h", 1);
            for (int i = 0; i < clusternumber; i++) {
                image->verts[i] = LabtoRGB(seeds[i]);
                clusterattr[i] = i;
                }
        }
        else{
#pragma omp parallel for
            for (int i = 0; i < w * h; i++) {
                int index = ((zeno::clamp(int(imagepos[i][0] * 255.99), 0, 255) / 4) * 64 + 
                            (zeno::clamp(int(imagepos[i][1] * 255.99), 0, 255) / 4)) * 64 + 
                            (zeno::clamp(int(imagepos[i][2] * 255.99), 0, 255) / 4);
                image->verts[i] = LabtoRGB(seeds[clusterindices[index]]);
                clusterattr[i] = clusterindices[index];
            }
        }set_output("image", std::move(image)));
    }
};
ZENDEFNODE(ImageQuantization, {
    {
        {gParamType_Primitive, "image"},
        {gParamType_Int, "Number of Color", "5"},
        {gParamType_Bool, "Output Cluster Centers", "1"},
        {gParamType_String, "Cluster Attribute", "cluster"},
    },
    {
        {gParamType_Primitive, "image"},
    },
    {},
    {"image"},
});
#endif

}
}