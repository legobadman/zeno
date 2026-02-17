#include <zeno/zeno.h>
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
#include <zeno/utils/interfaceutil.h>
#include <zeno/types/ListObject_impl.h>
#include <regex>


namespace zeno {

    using namespace zeno::reflect;

    //refer to PrimitiveTransform

    struct Transform : INode2 {

        NodeType type() const { return Node_Normal; }
        void clearCalcResults() {}
        float time() const { return 1.0f; }

        ZErrorCode apply(INodeData* ptrNodeData) override {
            ZNodeParams* nodeParams = static_cast<ZNodeParams*>(ptrNodeData);
            auto input_object = nodeParams->clone_input("Input Geometry");
            zeno::vec3f translate = toVec3f(nodeParams->get_input2_vec3f("Translation"));
            zeno::vec3f eulerXYZ = toVec3f(nodeParams->get_input2_vec3f("eulerXYZ"));
            zeno::vec4f rotation = toVec4f(nodeParams->get_input2_vec4f("quatRotation"));
            zeno::vec3f scaling = toVec3f(nodeParams->get_input2_vec3f("Scaling"));
            zeno::vec3f shear = toVec3f(nodeParams->get_input2_vec3f("Shear"));
            std::string pivotType = nodeParams->get_input2_string("pivot");
            zeno::vec3f pivotPos = toVec3f(nodeParams->get_input2_vec3f("pivotPos"));
            zeno::vec3f localX = toVec3f(nodeParams->get_input2_vec3f("localX"));
            zeno::vec3f localY = toVec3f(nodeParams->get_input2_vec3f("localY"));
            glm::mat4 pre_mat = toGlmMat4(nodeParams->get_input2_mat4("PreMatrix"));
            glm::mat4 pre_apply = toGlmMat4(nodeParams->get_input2_mat4("preTransform"));
            glm::mat4 local = toGlmMat4(nodeParams->get_input2_mat4("Local Matrix"));
            std::string order = nodeParams->get_input2_string("EulerRotationOrder");
            std::string measure = nodeParams->get_input2_string("EulerAngleMeasure");

            if (!input_object) {
                throw makeError<UnimplError>("empty input object.");
            }
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

            std::function<void(IObject2*)> transformListObj = [&](IObject2* obj) {
                if (auto listobj = dynamic_cast<ListObject*>(obj)) {
                    for (int i = 0; i < listobj->size(); ++i) {
                        transformListObj(listobj->get(i));
                    }
                } else if (auto geoObj = dynamic_cast<GeometryObject*>(obj)) {
                    transformGeom(geoObj,
                        toZMat4(matrix),
                        pivotType.c_str(),
                        toAbiVec3f(pivotPos),
                        toAbiVec3f(localX),
                        toAbiVec3f(localY),
                        toAbiVec3f(translate),
                        toAbiVec4f(rotation),
                        toAbiVec3f(scaling));

                    auto transform_ptr = glm::value_ptr(matrix);

                    zeno::vec4f row0, row1, row2, row3;
                    memcpy(row0.data(), transform_ptr, sizeof(float) * 4);
                    memcpy(row1.data(), transform_ptr + 4, sizeof(float) * 4);
                    memcpy(row2.data(), transform_ptr + 8, sizeof(float) * 4);
                    memcpy(row3.data(), transform_ptr + 12, sizeof(float) * 4);

                    geoObj->userData()->set_vec4f("_transform_row0", toAbiVec4f(row0));
                    geoObj->userData()->set_vec4f("_transform_row1", toAbiVec4f(row1));
                    geoObj->userData()->set_vec4f("_transform_row2", toAbiVec4f(row2));
                    geoObj->userData()->set_vec4f("_transform_row3", toAbiVec4f(row3));
                }
            };

            transformListObj(input_object.get());
            nodeParams->set_output("Output", std::move(input_object));
            return ZErr_OK;
        }
    };

    ZENDEFNODE(Transform,
    {
        {
            {gParamType_IObject2, "Input Geometry"},
            ParamPrimitive("Translation", gParamType_Vec3f),
            ParamPrimitive("Scaling", gParamType_Vec3f, zeno::vec3f(1,1,1)),
            ParamPrimitive("eulerXYZ", gParamType_Vec3f),
            ParamPrimitive("Shear", gParamType_Vec3f),
            ParamPrimitive("quatRotation", gParamType_Vec4f, zeno::vec4f(0,0,0,1)),
            ParamPrimitive("PreMatrix", gParamType_Matrix4, glm::mat4(1.f)),
            ParamPrimitive("pivot", gParamType_String, "bboxCenter", Combobox, std::vector<std::string>{"world", "bboxCenter", "custom"}),
            ParamPrimitive("pivotPos", gParamType_Vec3f),
            ParamPrimitive("Local Matrix", gParamType_Matrix4),
            ParamPrimitive("localX", gParamType_Vec3f, zeno::vec3f(1,1,0)),
            ParamPrimitive("localY", gParamType_Vec3f, zeno::vec3f(0,1,0)),
            ParamPrimitive("preTransform", gParamType_Matrix4, glm::mat4(1.f)),
            ParamPrimitive("EulerRotationOrder", gParamType_String, "YXZ", Combobox, std::vector<std::string>{"XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX"}),
            ParamPrimitive("EulerAngleMeasure", gParamType_String, "Degree", Combobox, std::vector<std::string>{"Degree", "Radians"}),
        },
        {
            {gParamType_IObject2, "Output"},
        },
        {},
        {"geom"},
        "Transform a geometry",
        "",
        ":/icons/node/transform.svg",
        "#FFFFFF"
    });

}