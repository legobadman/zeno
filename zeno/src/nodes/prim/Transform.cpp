#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/UserData.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/geo/geometryutil.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <zeno/utils/eulerangle.h>
#include <zeno/utils/string.h>
#include <regex>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;

    //refer to PrimitiveTransform

    struct ZDEFNODE() Transform : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input Geometry")},
                {"translate", ParamPrimitive("Translation")},
                {"scaling", ParamPrimitive("Scaling")},
                {"rotation", ParamPrimitive("quatRotation")},
                {"pre_mat", ParamPrimitive("Matrix")},
                {"pivotType", ParamPrimitive("pivot", "bboxCenter", Combobox, std::vector<std::string>{"world", "bboxCenter", "custom"})},
                {"pre_apply", ParamPrimitive("preTransform")},
                {"pre_mat", ParamPrimitive("EulerRotationOrder", "YXZ", Combobox, std::vector<std::string>{"XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX"})},
                {"pre_apply", ParamPrimitive("EulerAngleMeasure", "Degree", Combobox, std::vector<std::string>{"Degree", "Radians"})},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        static glm::vec3 mapplypos(glm::mat4 const& matrix, glm::vec3 const& vector) {
            auto vector4 = matrix * glm::vec4(vector, 1.0f);
            return glm::vec3(vector4) / vector4.w;
        }

        static glm::vec3 mapplynrm(glm::mat4 const& matrix, glm::vec3 const& vector) {
            glm::mat3 normMatrix(matrix);
            normMatrix = glm::transpose(glm::inverse(normMatrix));
            auto vector3 = normMatrix * vector;
            return glm::normalize(vector3);
        }

        static void transformObj(
            std::shared_ptr<GeometryObject> geom
            , glm::mat4 matrix
            , std::string pivotType
            , vec3f pivotPos
            , vec3f localX
            , vec3f localY
            , vec3f translate
            , vec4f rotation
            , vec3f scaling)
        {
            zeno::vec3f _pivot = {};
            zeno::vec3f lX = { 1, 0, 0 };
            zeno::vec3f lY = { 0, 1, 0 };
            if (pivotType == "bboxCenter") {
                zeno::vec3f _min;
                zeno::vec3f _max;
                std::tie(_min, _max) = geomBoundingBox(geom.get());
                _pivot = (_min + _max) / 2;
            }
            else if (pivotType == "custom") {
                _pivot = pivotPos;
                lX = localX;
                lY = localY;
            }
            auto lZ = zeno::cross(lX, lY);
            lY = zeno::cross(lZ, lX);

            auto pivot_to_world = glm::mat4(1);
            pivot_to_world[0] = { lX[0], lX[1], lX[2], 0 };
            pivot_to_world[1] = { lY[0], lY[1], lY[2], 0 };
            pivot_to_world[2] = { lZ[0], lZ[1], lZ[2], 0 };
            pivot_to_world[3] = { _pivot[0], _pivot[1], _pivot[2], 1 };
            auto pivot_to_local = glm::inverse(pivot_to_world);
            matrix = pivot_to_world * matrix * pivot_to_local;

            //std::vector<zeno::vec3f> pos = geom->get_attrs<zeno::vec3f>(ATTR_POINT, "pos");
            if (geom->has_attr(ATTR_POINT, "pos"))
            {
                //TODO: 前面可以判断是否符合写时复制，比如transform的tsr是否发生改变
                geom->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", [&](zeno::vec3f old_pos)->zeno::vec3f {
                    auto p = zeno::vec_to_other<glm::vec3>(old_pos);
                    p = mapplypos(matrix, p);
                    auto newpos = zeno::other_to_vec<3>(p);
                    return newpos;
                });
                //prim->verts.add_attr<zeno::vec3f>("_origin_pos") = pos; //视图端transform会用到，这里先不加
            }

            //std::vector<float> xvec = geom->get_attrs<float, 'x'>(ATTR_POINT, "pos");
            //float xpos = geom->get_elem<float, 'x'>(ATTR_POINT, "pos", 0);

            if (geom->has_attr(ATTR_POINT, "nrm"))
            {
                geom->foreach_attr_update<zeno::vec3f>(ATTR_POINT, "pos", [&](zeno::vec3f old_nrm)->zeno::vec3f {
                    auto n = zeno::vec_to_other<glm::vec3>(old_nrm);
                    n = mapplynrm(matrix, n);
                    auto newnrm = zeno::other_to_vec<3>(n);
                    return newnrm;
                });
                //prim->verts.add_attr<zeno::vec3f>("_origin_nrm") = nrm;
            }

            auto& user_data = geom->userData();
            user_data.setLiterial("_translate", translate);
            user_data.setLiterial("_rotate", rotation);
            user_data.setLiterial("_scale", scaling);
            user_data.set2("_pivot", _pivot);
            user_data.set2("_localX", lX);
            user_data.set2("_localY", lY);
            user_data.del("_bboxMin");
            user_data.del("_bboxMax");
        }

        std::shared_ptr<zeno::GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            zeno::vec3f translate = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f eulerXYZ = zeno::vec3f({ 0,0,0 }),
            zeno::vec4f rotation = zeno::vec4f({ 0,0,0,1 }),
            zeno::vec3f scaling = zeno::vec3f({ 1,1,1 }),
            zeno::vec3f shear = zeno::vec3f({ 0,0,0 }),
            std::string pivotType = "bboxCenter",
            zeno::vec3f pivotPos = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f localX = zeno::vec3f({ 1,0,0 }),
            zeno::vec3f localY = zeno::vec3f({ 0,1,0 }),
            glm::mat4 pre_mat = glm::mat4(1.f),
            glm::mat4 pre_apply = glm::mat4(1.f),
            glm::mat4 local = glm::mat4(1.f),
            std::string order = "YXZ",
            std::string measure = "Degree"
        ) {
            glm::mat4 matTrans = glm::translate(glm::vec3(translate[0], translate[1], translate[2]));
            auto orderTyped = magic_enum::enum_cast<EulerAngle::RotationOrder>(order).value_or(EulerAngle::RotationOrder::YXZ);
            auto measureTyped = magic_enum::enum_cast<EulerAngle::Measure>(measure).value_or(EulerAngle::Measure::Radians);
            glm::vec3 eularAngleXYZ = glm::vec3(eulerXYZ[0], eulerXYZ[1], eulerXYZ[2]);
            glm::mat4 matRotate = EulerAngle::rotate(orderTyped, measureTyped, eularAngleXYZ);

            glm::quat myQuat(rotation[3], rotation[0], rotation[1], rotation[2]);
            glm::mat4 matQuat = glm::toMat4(myQuat);
            glm::mat4 matScal = glm::scale(glm::vec3(scaling[0], scaling[1], scaling[2]));
            glm::mat4 matShearX = glm::transpose(glm::mat4(
                1, shear[0], 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1));
            glm::mat4 matShearY = glm::transpose(glm::mat4(
                1, 0, shear[1], 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1));
            glm::mat4 matShearZ = glm::transpose(glm::mat4(
                1, 0, 0, 0,
                0, 1, shear[2], 0,
                0, 0, 1, 0,
                0, 0, 0, 1));

            auto matrix = pre_mat * local * matTrans * matRotate * matQuat * matScal * matShearZ * matShearY * matShearX * glm::inverse(local) * pre_apply;

            transformObj(input_object, matrix, pivotType, pivotPos, localX, localY, translate, rotation, scaling);

            auto transform_ptr = glm::value_ptr(matrix);

            zeno::vec4f row0, row1, row2, row3;
            memcpy(row0.data(), transform_ptr, sizeof(float) * 4);
            memcpy(row1.data(), transform_ptr + 4, sizeof(float) * 4);
            memcpy(row2.data(), transform_ptr + 8, sizeof(float) * 4);
            memcpy(row3.data(), transform_ptr + 12, sizeof(float) * 4);

            input_object->userData().set2("_transform_row0", row0);
            input_object->userData().set2("_transform_row1", row1);
            input_object->userData().set2("_transform_row2", row2);
            input_object->userData().set2("_transform_row3", row3);

            return input_object;
        }
    };

}