#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/MatrixObject.h>
#include <zeno/types/UserData.h>
#include <zeno/types/ListObject.h>

#include <zeno/utils/orthonormal.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/utils/log.h>

#include <glm/gtx/quaternion.hpp>
#include <random>
#include <sstream>
#include <ctime>
#include <iostream>

#include "zeno_types/reflect/reflection.generated.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno
{
    using namespace zeno::reflect;

    struct ZDEFNODE() WBTestPrimNode : zeno::INode
    {
        std::shared_ptr<zeno::PrimitiveObject> apply(
                std::shared_ptr<const zeno::PrimitiveObject> input_obj,
                const std::string& name1 = "a16",
                const std::string& name2 = "a24",
                int a = 234,
                float b = 456.234,

                zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
                zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
                zeno::vec2f Size = zeno::vec2f({ 1,1 }),
                int Rows = 2,
                int Columns = 2,

                std::string face_type = "Quadrilaterals",
                std::string Direction = "ZX",
                bool bCalcPointNormals = false
        )
        {
            std::shared_ptr<zeno::PrimitiveObject> result = std::const_pointer_cast<zeno::PrimitiveObject>(input_obj);
            return result;
        }
    };

    struct ZDEFNODE() WBTestGeoNode : zeno::INode
    {
        std::shared_ptr<GeometryObject> apply(
                std::shared_ptr<const GeometryObject> input_geo,
                const std::string& name1 = "a16",
                const std::string& name2 = "a24",
                int a = 234,
                float b = 456.234,

                zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
                zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
                zeno::vec2f Size = zeno::vec2f({ 1,1 }),
                int Rows = 2,
                int Columns = 2,

                std::string face_type = "Quadrilaterals",
                std::string Direction = "ZX",
                bool bCalcPointNormals = false
        )
        {
            std::shared_ptr<GeometryObject> geo = std::const_pointer_cast<GeometryObject>(input_geo);
            return geo;
        }
    };

    struct ZDEFNODE() WBGeoBend : zeno::INode
    {
        ReflectCustomUI m_uilayout = {
            //输入：
            _Group {
                {"input_geo", ParamObject("Input", Socket_Clone)},
                {"limitDeformation", ParamPrimitive("Limit Deformation")},
                {"symmetricDeformation", ParamPrimitive("Symmetric Deformation")},
                {"angle", ParamPrimitive("Bend Angle (degree)")},
                {"upVector", ParamPrimitive("Up Vector")},
                {"capOrigin", ParamPrimitive("Capture Origin")},
                {"dirVector", ParamPrimitive("Capture Direction")},
                {"capLen", ParamPrimitive("Capture Length")},
            },
            //输出：
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
                std::shared_ptr<zeno::GeometryObject> input_geo,
                int limitDeformation = 1,
                int symmetricDeformation = 0,
                float angle = 90,
                zeno::vec3f upVector = zeno::vec3f({ 0,1,0 }),
                zeno::vec3f capOrigin = zeno::vec3f({ 0,0,0 }),
                zeno::vec3f dirVector = zeno::vec3f({ 0,0,1 }),
                float capLen = 1.0
        )
        {
            glm::vec3 up = normalize(glm::vec3(upVector[0], upVector[1], upVector[2]));
            glm::vec3 dir = normalize(glm::vec3(dirVector[0], dirVector[1], dirVector[2]));
            glm::vec3 axis1 = normalize(cross(dir, up));
            glm::vec3 axis2 = cross(axis1, dir);
            glm::vec3 origin = glm::vec3(capOrigin[0], capOrigin[1], capOrigin[2]);
            double rotMatEle[9] = { dir.x, dir.y, dir.z,
                                    axis2.x, axis2.y, axis2.z,
                                    axis1.x, axis1.y, axis1.z };
            glm::mat3 rotMat = glm::make_mat3x3(rotMatEle);
            glm::mat3 inverse = glm::transpose(rotMat);

            input_geo->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", 0, [&](int idx, zeno::vec3f old_pos)->zeno::vec3f {

                glm::vec3 original = glm::vec3(old_pos[0], old_pos[1], old_pos[2]);
                glm::vec3 deformedPos = original;

                original -= origin;
                deformedPos -= origin;

                deformedPos = inverse * deformedPos;
                original = inverse * original;

                double bend_threshold = 0.005;
                if (std::abs(angle) >= bend_threshold)
                {
                    double angleRad = angle * M_PI / 180;
                    double bend = angleRad * (deformedPos.x / capLen);
                    glm::vec3 N = { 0, 1, 0 };
                    glm::vec3 center = (float)(capLen / angleRad) * N;
                    double d = deformedPos.x / capLen;
                    if (symmetricDeformation)
                    {
                        if (limitDeformation && std::abs(deformedPos.x) > capLen)
                        {
                            bend = angleRad;
                            d = 1;
                            if (-deformedPos.x > capLen)
                            {
                                bend *= -1;
                                d *= -1;
                            }
                        }
                    }
                    else
                    {
                        if (deformedPos.x * capLen < 0)
                        {
                            bend = 0;
                            d = 0;
                        }
                        if (limitDeformation && deformedPos.x > capLen)
                        {
                            bend = angleRad;
                            d = 1;
                        }
                    }
                    double cb = std::cos(bend);
                    double sb = std::sin(bend);
                    double bendMatEle[9] = { cb, sb, 0,
                                             -sb, cb, 0,
                                             0, 0, 1 };
                    glm::mat3 bendRotMat = glm::make_mat3x3(bendMatEle);
                    original.x -= d * capLen;
                    original -= center;
                    original = bendRotMat * original;
                    original += center;
                }
                deformedPos = rotMat * original + origin;
                auto newpos = vec3f(deformedPos.x, deformedPos.y, deformedPos.z);
                return newpos;
            });

            return input_geo;
        }
    };


    ///////////////////////////////////////////////////////////////////////////////
    // 2022.07.11 Perlin Noise
    ///////////////////////////////////////////////////////////////////////////////

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Perlin Noise
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    const int noise_permutation[] = {
            151,160,137,91,90,15,
            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
            190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
            88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
            77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
            102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
            135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
            5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
            223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
            49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
            138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
            151,160,137,91,90,15,
            131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
            190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
            88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
            77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
            102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
            135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
            5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
            223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
            49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
            138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    };

    float noise_fade(float t) {
        // Fade function as defined by Ken Perlin.  This eases coordinate values
        // so that they will ease towards integral values.  This ends up smoothing
        // the final output.
        return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
    }

    int noise_inc(int num) {
        return num + 1;
    }

    float noise_grad(int hash, float x, float y, float z) {
        switch (hash & 0xF) {
            case 0x0: return  x + y;
            case 0x1: return -x + y;
            case 0x2: return  x - y;
            case 0x3: return -x - y;
            case 0x4: return  x + z;
            case 0x5: return -x + z;
            case 0x6: return  x - z;
            case 0x7: return -x - z;
            case 0x8: return  y + z;
            case 0x9: return -y + z;
            case 0xA: return  y - z;
            case 0xB: return -y - z;
            case 0xC: return  y + x;
            case 0xD: return -y + z;
            case 0xE: return  y - x;
            case 0xF: return -y - z;
            default: return 0;
        }
    }

    float noise_perlin(float x, float y, float z)
    {
        x = fract(x / 256.f) * 256.f;
        y = fract(y / 256.f) * 256.f;
        z = fract(z / 256.f) * 256.f;

        int xi = (int)x & 255;          // Calculate the "unit cube" that the point asked will be located in
        int yi = (int)y & 255;          // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
        int zi = (int)z & 255;          // plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.

        float xf = x - (int)x;
        float yf = y - (int)y;
        float zf = z - (int)z;

        float u = noise_fade(xf);
        float v = noise_fade(yf);
        float w = noise_fade(zf);

        int aaa = noise_permutation[noise_permutation[noise_permutation[xi] + yi] + zi];
        int aba = noise_permutation[noise_permutation[noise_permutation[xi] + noise_inc(yi)] + zi];
        int aab = noise_permutation[noise_permutation[noise_permutation[xi] + yi] + noise_inc(zi)];
        int abb = noise_permutation[noise_permutation[noise_permutation[xi] + noise_inc(yi)] + noise_inc(zi)];
        int baa = noise_permutation[noise_permutation[noise_permutation[noise_inc(xi)] + yi] + zi];
        int bba = noise_permutation[noise_permutation[noise_permutation[noise_inc(xi)] + noise_inc(yi)] + zi];
        int bab = noise_permutation[noise_permutation[noise_permutation[noise_inc(xi)] + yi] + noise_inc(zi)];
        int bbb = noise_permutation[noise_permutation[noise_permutation[noise_inc(xi)] + noise_inc(yi)] + noise_inc(zi)];

        float x1 = mix(noise_grad(aaa, xf, yf, zf),
                       noise_grad(baa, xf - 1, yf, zf),
                       u);
        float x2 = mix(noise_grad(aba, xf, yf - 1, zf),
                       noise_grad(bba, xf - 1, yf - 1, zf),
                       u);
        float y1 = mix(x1, x2, v);
        x1 = mix(noise_grad(aab, xf, yf, zf - 1),
                 noise_grad(bab, xf - 1, yf, zf - 1),
                 u);
        x2 = mix(noise_grad(abb, xf, yf - 1, zf - 1),
                 noise_grad(bbb, xf - 1, yf - 1, zf - 1),
                 u);
        float y2 = mix(x1, x2, v);

        return mix(y1, y2, w);
    }

    struct ZDEFNODE() erode_noise_perlin_GEO : zeno::INode {

        ReflectCustomUI m_uilayout = {
                //输入：
                _Group{
                        {"input_2d_grid_geo",   ParamObject("Input", Socket_Clone)},
                        {"noiseInputAttrName",  ParamPrimitive("Noise Input (Vec3f)")},
                        {"noiseOutputAttrName", ParamPrimitive("Noise Output Name")},
                        {"noiseOutputAttrType", ParamPrimitive("Noise Attribute Type", "float", zeno::Combobox, std::vector<std::string>{"float", "vec3f"})},
                },
                //输出：
                _Group{
                        {"", ParamObject("Output")},
                }
        };

        std::shared_ptr<zeno::GeometryObject> apply(
                std::shared_ptr<zeno::GeometryObject> input_2d_grid_geo,
                const std::string& noiseInputAttrName = "pos",
                const std::string& noiseOutputAttrName = "noise",
                const std::string& noiseOutputAttrType = "float"
        )
        {
            if (!input_2d_grid_geo->has_attr(ATTR_POINT, noiseInputAttrName)) {
                zeno::log_error("no such data named '{}'.", noiseInputAttrName);
            }else{

            }

            if (!input_2d_grid_geo->has_attr(ATTR_POINT, noiseOutputAttrName)) {
                if (noiseOutputAttrType == "vec3f") input_2d_grid_geo->GeometryObject::create_point_attr(noiseOutputAttrName, zeno::vec3f(0,0,0));
                else if (noiseOutputAttrType == "float") input_2d_grid_geo->GeometryObject::create_point_attr(noiseOutputAttrName, 0.f);
            }

            std::vector<zeno::vec3f> input_data = input_2d_grid_geo->get_attrs<zeno::vec3f>(ATTR_POINT, noiseInputAttrName);

            if (0) {
                if (noiseOutputAttrType == "float") {
                    std::vector<float> res2;
                    for (int i = 0; i < input_2d_grid_geo->npoints(); i++) {
                        //res2.push_back()
                    }
                    input_2d_grid_geo->set_attr(ATTR_POINT, noiseOutputAttrName, res2);
                }
                std::vector<zeno::vec3f> res1;
                input_2d_grid_geo->set_attr(ATTR_POINT, noiseOutputAttrName, res1);

            }else{
                if (noiseOutputAttrType == "float") {
                    input_2d_grid_geo->foreach_attr_update<float>(ATTR_POINT, noiseOutputAttrName, 0, [&](int idx, float)->float {
                        float result = noise_perlin(input_data[idx][0], input_data[idx][1], input_data[idx][2]);
                        return result;
                    });
                }
                else if (noiseOutputAttrType == "vec3f"){
                    input_2d_grid_geo->foreach_attr_update<zeno::vec3f>(ATTR_POINT, noiseOutputAttrName, 0, [&](int idx, zeno::vec3f)->zeno::vec3f {
                        float x = noise_perlin(input_data[idx][0], input_data[idx][1], input_data[idx][2]);
                        float y = noise_perlin(input_data[idx][1], input_data[idx][2], input_data[idx][0]);
                        float z = noise_perlin(input_data[idx][2], input_data[idx][0], input_data[idx][1]);
                        auto result = zeno::vec3f(x, y, z);
                        return result;
                    });
                }
            }

            return input_2d_grid_geo;
        }
    };


}