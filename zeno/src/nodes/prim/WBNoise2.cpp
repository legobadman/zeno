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

    struct ZDEFNODE() WBTestGeoNode : zeno::INode
    {
        std::shared_ptr<GeometryObject> apply(
                std::shared_ptr<const GeometryObject> input_obj,
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
//            std::shared_ptr<zeno::PrimitiveObject> result = std::const_pointer_cast<zeno::PrimitiveObject>(input_obj);
//            auto geo = std::make_shared<zeno::GeometryObject>(0,1,0);
            std::shared_ptr<GeometryObject> geo = std::const_pointer_cast<GeometryObject>(input_obj);
            return geo;
        }
    };

    struct ZDEFNODE() WBPrimBend_G : zeno::INode
    {
        ReflectCustomUI m_uilayout = {
            //输入：
            _Group {
                {"input_obj", ParamObject("Input Geometry", Socket_Clone)},
                {"limitDeformation",     ParamPrimitive("Limit Deformation")},
                {"symmetricDeformation",     ParamPrimitive("Symmetric Deformation")},
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
                std::shared_ptr<zeno::GeometryObject> input_obj,
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

            input_obj->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", 0, [&](int idx, zeno::vec3f old_pos)->zeno::vec3f {

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

            return input_obj;
        }
    };

    struct ZDEFNODE() WBTestPrimNode : zeno::INode
    {
//        ReflectCustomUI m_uilayout = {
//                //输入：
//                _Group {
//                        {"input_obj", ParamObject("Input Object", Socket_Owning) },
//                        {"name1",     ParamPrimitive("Name 1")},
//                        {"name2",     ParamPrimitive("Name 2")},
//                        {"a", ParamPrimitive("A")},
//                        {"b", ParamPrimitive("B")},
//                },
//                //输出：
//                _Group {
//                        {"", ParamObject("Output Object", Socket_Owning)},
//                },
//                //数值参数布局：
//                CustomUIParams {
//                        ParamTab {
//                                "CustomTab1",
//                                {
//                                        ParamGroup {
//                                                "Group1",
//                                                {
//                                                        ParamPrimitive("Name 1"),
//                                                        ParamPrimitive("Name 2"),
//                                                }
//                                        },
//                                        ParamGroup {
//                                                "Group2",
//                                                {
//                                                        ParamPrimitive("A"),
//                                                        ParamPrimitive("B"),
//                                                }
//                                        },
//                                }
//                        },
//                        ParamTab {
//                                "Tab2",
//                        },
//                }
//        };

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
}