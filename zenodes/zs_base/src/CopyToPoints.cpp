#include <zvec.h>
#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "geom_transform.h"
#include <cmath>
#include <limits>

namespace zeno {

    static void bboxFromPositions(IGeometryObject* geom, glm::vec3& outMin, glm::vec3& outMax) {
        int n = geom->npoints();
        if (n == 0) {
            outMin = outMax = glm::vec3(0.f);
            return;
        }
        std::vector<zeno::Vec3f> buf(n);
        geom->get_vec3f_attr(ATTR_POINT, "pos", buf.data(), n);
        outMin = glm::vec3(std::numeric_limits<float>::max());
        outMax = glm::vec3(std::numeric_limits<float>::lowest());
        for (int i = 0; i < n; i++) {
            glm::vec3 p(buf[i].x, buf[i].y, buf[i].z);
            outMin = glm::min(outMin, p);
            outMax = glm::max(outMax, p);
        }
    }

    struct CopyToPoints : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Object To Be Copied");
            auto points_geom = ptrNodeData->get_input_Geometry("Points From");
            std::string alignTo = get_string_param(ptrNodeData, "Align To");
            std::string trans_by = get_string_param(ptrNodeData, "Translate By Attribute");
            std::string scale_by = get_string_param(ptrNodeData, "Scale By Attribute");
            std::string rotate_by = get_string_param(ptrNodeData, "Rotate By Attribute");

            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (!points_geom) {
                ptrNodeData->report_error("empty target object");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("invalid input object (no pos attr)");
                return ZErr_ParamError;
            }
            if (!points_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("invalid target object (no pos attr)");
                return ZErr_ParamError;
            }

            int inputObjPointsCount = input_geom->npoints();
            int inputObjFacesCount = input_geom->nfaces();
            int targetObjPointsCount = points_geom->npoints();
            if (inputObjPointsCount == 0 || inputObjFacesCount == 0) {
                ptrNodeData->report_error("input object has no points or faces");
                return ZErr_ParamError;
            }
            if (targetObjPointsCount == 0) {
                ptrNodeData->report_error("target object has no points");
                return ZErr_ParamError;
            }

            glm::vec3 bbMin, bbMax;
            bboxFromPositions(input_geom, bbMin, bbMax);
            glm::vec3 originCenter;
            if (alignTo == "Align To Point Center") {
                originCenter = (bbMin + bbMax) * 0.5f;
            } else {
                originCenter = bbMin;
            }

            std::vector<zeno::Vec3f> inputPosAbi(inputObjPointsCount);
            size_t nr = input_geom->get_vec3f_attr(ATTR_POINT, "pos", inputPosAbi.data(), inputObjPointsCount);
            if (nr != (size_t)inputObjPointsCount) {
                ptrNodeData->report_error("failed to get input positions");
                return ZErr_ParamError;
            }
            std::vector<glm::vec3> inputPos(inputObjPointsCount);
            for (int i = 0; i < inputObjPointsCount; i++) {
                inputPos[i] = glm::vec3(inputPosAbi[i].x, inputPosAbi[i].y, inputPosAbi[i].z);
            }

            std::vector<std::vector<int>> inputFacesPoints(inputObjFacesCount);
            for (int i = 0; i < inputObjFacesCount; i++) {
                int nv = input_geom->face_vertex_count(i);
                std::vector<int> pts(nv);
                size_t got = input_geom->face_points(i, pts.data(), nv);
                if (got == (size_t)nv) {
                    inputFacesPoints[i] = pts;
                }
            }

            bool hasNrm = input_geom->has_point_attr("nrm");
            std::vector<glm::vec3> inputNrm;
            if (hasNrm) {
                std::vector<zeno::Vec3f> nrmAbi(inputObjPointsCount);
                input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), inputObjPointsCount);
                inputNrm.resize(inputObjPointsCount);
                for (int i = 0; i < inputObjPointsCount; i++) {
                    inputNrm[i] = glm::vec3(nrmAbi[i].x, nrmAbi[i].y, nrmAbi[i].z);
                }
            }

            std::vector<float> transFloatBuf;
            std::vector<zeno::Vec3f> transVec3Buf;
            bool hasTrans = false;
            bool transIsFloat = false;
            if (!trans_by.empty() && points_geom->has_attr(ATTR_POINT, trans_by.c_str(), ATTR_FLOAT)) {
                transFloatBuf.resize(targetObjPointsCount);
                points_geom->get_float_attr(ATTR_POINT, trans_by.c_str(), transFloatBuf.data(), targetObjPointsCount);
                hasTrans = true;
                transIsFloat = true;
            } else if (!trans_by.empty() && points_geom->has_attr(ATTR_POINT, trans_by.c_str(), ATTR_VEC3)) {
                transVec3Buf.resize(targetObjPointsCount);
                points_geom->get_vec3f_attr(ATTR_POINT, trans_by.c_str(), transVec3Buf.data(), targetObjPointsCount);
                hasTrans = true;
            }

            std::vector<float> scaleFloatBuf;
            std::vector<zeno::Vec3f> scaleVec3Buf;
            bool hasScale = false;
            bool scaleIsFloat = false;
            if (!scale_by.empty() && points_geom->has_attr(ATTR_POINT, scale_by.c_str(), ATTR_FLOAT)) {
                scaleFloatBuf.resize(targetObjPointsCount);
                points_geom->get_float_attr(ATTR_POINT, scale_by.c_str(), scaleFloatBuf.data(), targetObjPointsCount);
                hasScale = true;
                scaleIsFloat = true;
            } else if (!scale_by.empty() && points_geom->has_attr(ATTR_POINT, scale_by.c_str(), ATTR_VEC3)) {
                scaleVec3Buf.resize(targetObjPointsCount);
                points_geom->get_vec3f_attr(ATTR_POINT, scale_by.c_str(), scaleVec3Buf.data(), targetObjPointsCount);
                hasScale = true;
            }

            std::vector<float> rotateFloatBuf;
            std::vector<zeno::Vec3f> rotateVec3Buf;
            bool hasRotate = false;
            bool rotateIsFloat = false;
            if (!rotate_by.empty() && points_geom->has_attr(ATTR_POINT, rotate_by.c_str(), ATTR_FLOAT)) {
                rotateFloatBuf.resize(targetObjPointsCount);
                points_geom->get_float_attr(ATTR_POINT, rotate_by.c_str(), rotateFloatBuf.data(), targetObjPointsCount);
                hasRotate = true;
                rotateIsFloat = true;
            } else if (!rotate_by.empty() && points_geom->has_attr(ATTR_POINT, rotate_by.c_str(), ATTR_VEC3)) {
                rotateVec3Buf.resize(targetObjPointsCount);
                points_geom->get_vec3f_attr(ATTR_POINT, rotate_by.c_str(), rotateVec3Buf.data(), targetObjPointsCount);
                hasRotate = true;
            }

            size_t newObjPointsCount = (size_t)targetObjPointsCount * inputObjPointsCount;
            std::vector<glm::vec3> newObjPos(newObjPointsCount);
            std::vector<glm::vec3> newObjNrm;
            if (hasNrm) newObjNrm.resize(newObjPointsCount);
            std::vector<std::vector<int>> faces;

            std::vector<zeno::Vec3f> targetPosAbi(targetObjPointsCount);
            points_geom->get_vec3f_attr(ATTR_POINT, "pos", targetPosAbi.data(), targetObjPointsCount);

            for (int i = 0; i < targetObjPointsCount; i++) {
                glm::vec3 targetPos(targetPosAbi[i].x, targetPosAbi[i].y, targetPosAbi[i].z);
                glm::vec3 dx = targetPos - originCenter;
                glm::mat4 translate = glm::translate(glm::mat4(1.0f), dx);

                glm::vec3 trans(0.f);
                if (hasTrans) {
                    if (transIsFloat) {
                        float v = transFloatBuf[i];
                        trans = glm::vec3(v, v, v);
                    } else {
                        trans = glm::vec3(transVec3Buf[i].x, transVec3Buf[i].y, transVec3Buf[i].z);
                    }
                }
                glm::vec3 scaling(1.f);
                if (hasScale) {
                    if (scaleIsFloat) {
                        scaling = glm::vec3(scaleFloatBuf[i], scaleFloatBuf[i], scaleFloatBuf[i]);
                    } else {
                        scaling = glm::vec3(scaleVec3Buf[i].x, scaleVec3Buf[i].y, scaleVec3Buf[i].z);
                    }
                }
                glm::vec3 eulerXYZ(0.f);
                if (hasRotate) {
                    if (rotateIsFloat) {
                        eulerXYZ = glm::vec3(rotateFloatBuf[i], rotateFloatBuf[i], rotateFloatBuf[i]);
                    } else {
                        eulerXYZ = glm::vec3(rotateVec3Buf[i].x, rotateVec3Buf[i].y, rotateVec3Buf[i].z);
                    }
                }
                glm::mat4 matByAttrs = buildTransformMatrix(trans, eulerXYZ, scaling);
                glm::mat4 fullMat = translate * matByAttrs;
                size_t pt_offset = (size_t)i * inputObjPointsCount;
                applyTransform(fullMat,
                    inputPos.data(), inputObjPointsCount,
                    newObjPos.data() + pt_offset,
                    hasNrm ? inputNrm.data() : nullptr,
                    hasNrm ? newObjNrm.data() + pt_offset : nullptr);

                for (int j = 0; j < inputObjFacesCount; j++) {
                    std::vector<int> facePoints = inputFacesPoints[j];
                    for (size_t k = 0; k < facePoints.size(); k++) {
                        facePoints[k] += (int)pt_offset;
                    }
                    faces.push_back(std::move(facePoints));
                }
            }

            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh2, false, newObjPos, faces);
            if (!spOutput) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }

            if (hasNrm) {
                std::vector<zeno::Vec3f> nrmAbi(newObjPointsCount);
                for (size_t i = 0; i < newObjPointsCount; i++) {
                    nrmAbi[i] = zeno::Vec3f{ newObjNrm[i].x, newObjNrm[i].y, newObjNrm[i].z };
                }
                spOutput->create_attr_by_vec3(ATTR_POINT, "nrm", nrmAbi.data(), newObjPointsCount);
            }

            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    // CopyToPoints
// ------------------------------------------------------------
// 功能说明：
// 将 "Object To Be Copied" 输入的几何体，复制到 "Points From"
// 输入几何体所包含的每一个点的位置上，并根据点属性控制
// 每个副本的变换方式。
//
// 详细行为：
// 1. 基础拷贝逻辑
//    - 遍历 "Points From" 中的所有点。
//    - 对于每一个点，生成一份 "Object To Be Copied" 的副本。
//    - 将副本移动到该点的位置。
//
// 2. Align To（对齐方式）
//    - 控制在移动到目标点位置时，几何体的对齐基准。
//    - "Align To Point Center"：以几何体中心对齐到点位置。
//    - "Align To Min"：以几何体包围盒最小点对齐到点位置。
//    - 不同模式会影响初始平移偏移量的计算方式。
//
// 3. Translate By Attribute（按属性平移）
//    - 若指定属性名，则从 "Points From" 的点属性中读取
//      同名属性值。
//    - 该属性值用于对对应副本进行额外平移。
//    - 通常期望为 vector 类型属性。
//
// 4. Scale By Attribute（按属性缩放）
//    - 若指定属性名，则从点属性中读取对应值。
//    - 该属性值用于控制每个副本的缩放。
//    - 可支持 uniform 或非 uniform 缩放（取决于属性类型）。
//
// 5. Rotate By Attribute（按属性旋转）
//    - 若指定属性名，则从点属性中读取对应值。
//    - 该属性值用于控制每个副本的旋转。
//    - 通常期望为欧拉角或四元数类型属性。
//
// 输出：
//    - 输出包含所有复制并变换后的几何体。
//
    ZENDEFNODE_ABI(CopyToPoints,
        Z_INPUTS(
            { "Object To Be Copied", _gParamType_Geometry },
            { "Points From", _gParamType_Geometry },
            { "Align To", _gParamType_String, ZString("Align To Point Center"), Combobox, Z_STRING_ARRAY("Align To Point Center", "Align To Min") },
            { "Translate By Attribute", _gParamType_String },
            { "Scale By Attribute", _gParamType_String },
            { "Rotate By Attribute", _gParamType_String }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "prim",
        "",
        "",
        ""
    );
}
