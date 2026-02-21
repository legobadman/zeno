#include <zvec.h>
#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include "geom_transform.h"
#include <Windows.h>
#include <vector>
#include <glm/glm.hpp>
#include <cmath>

namespace zeno {

    struct CopyAndTransform : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            int totalNumber = ptrNodeData->get_input2_int("Total Number");
            zeno::Vec3f transAbi = ptrNodeData->get_input2_vec3f("Translate");
            zeno::Vec3f rotAbi = ptrNodeData->get_input2_vec3f("Rotate");
            zeno::Vec3f scaleAbi = ptrNodeData->get_input2_vec3f("Scale");
            float uniformScale = ptrNodeData->get_input2_float("Uniform Scale");

            glm::vec3 translate(transAbi.x, transAbi.y, transAbi.z);
            glm::vec3 rotate(rotAbi.x, rotAbi.y, rotAbi.z);
            glm::vec3 scaleBase(scaleAbi.x, scaleAbi.y, scaleAbi.z);

            if (!input_geom) {
                ptrNodeData->report_error("empty input geometry");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }
            if (totalNumber <= 0) {
                ptrNodeData->report_error("Total Number must be positive");
                return ZErr_ParamError;
            }

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            if (nPts == 0 || nFaces == 0) {
                ptrNodeData->report_error("input has no points or faces");
                return ZErr_ParamError;
            }

            std::vector<zeno::Vec3f> posAbi(nPts);
            size_t nr = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (nr != (size_t)nPts) {
                ptrNodeData->report_error("failed to get positions");
                return ZErr_ParamError;
            }
            std::vector<glm::vec3> inputPos(nPts);
            for (int i = 0; i < nPts; i++) {
                inputPos[i] = glm::vec3(posAbi[i].x, posAbi[i].y, posAbi[i].z);
            }

            bool hasNrm = input_geom->has_point_attr("nrm");
            std::vector<glm::vec3> inputNrm;
            if (hasNrm) {
                std::vector<zeno::Vec3f> nrmAbi(nPts);
                input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
                inputNrm.resize(nPts);
                for (int i = 0; i < nPts; i++) {
                    inputNrm[i] = glm::vec3(nrmAbi[i].x, nrmAbi[i].y, nrmAbi[i].z);
                }
            }

            std::vector<std::vector<int>> inputFaces(nFaces);
            for (int i = 0; i < nFaces; i++) {
                int nv = input_geom->face_vertex_count(i);
                std::vector<int> pts(nv);
                size_t got = input_geom->face_points(i, pts.data(), nv);
                if (got == (size_t)nv)
                    inputFaces[i] = pts;
            }

            size_t totalPoints = (size_t)totalNumber * nPts;
            size_t totalFaces = (size_t)totalNumber * nFaces;
            std::vector<glm::vec3> mergedPos(totalPoints);
            std::vector<glm::vec3> mergedNrm;
            if (hasNrm) mergedNrm.resize(totalPoints);
            std::vector<std::vector<int>> mergedFaces;
            mergedFaces.reserve(totalFaces);

            for (int copyIdx = 0; copyIdx < totalNumber; copyIdx++) {
                glm::vec3 t = translate * (float)copyIdx;
                glm::vec3 r = rotate * (float)copyIdx;
                glm::vec3 s(
                    std::pow(scaleBase.x * uniformScale, (float)copyIdx),
                    std::pow(scaleBase.y * uniformScale, (float)copyIdx),
                    std::pow(scaleBase.z * uniformScale, (float)copyIdx));

                glm::mat4 M = buildTransformMatrix(t, r, s);
                size_t ptOffset = (size_t)copyIdx * nPts;
                applyTransform(M,
                    inputPos.data(), nPts,
                    mergedPos.data() + ptOffset,
                    hasNrm ? inputNrm.data() : nullptr,
                    hasNrm ? mergedNrm.data() + ptOffset : nullptr);

                size_t faceOffset = (size_t)copyIdx * nFaces;
                for (int j = 0; j < nFaces; j++) {
                    std::vector<int> pts = inputFaces[j];
                    for (size_t k = 0; k < pts.size(); k++)
                        pts[k] += (int)ptOffset;
                    mergedFaces.push_back(std::move(pts));
                }
            }

            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh2, false, mergedPos, mergedFaces);
            if (!spOutput) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }

            if (hasNrm) {
                std::vector<zeno::Vec3f> nrmAbi(totalPoints);
                for (size_t i = 0; i < totalPoints; i++) {
                    nrmAbi[i] = zeno::Vec3f{ mergedNrm[i].x, mergedNrm[i].y, mergedNrm[i].z };
                }
                spOutput->create_attr_by_vec3(ATTR_POINT, "nrm", nrmAbi.data(), totalPoints);
            }

            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

// CopyAndTransform
// ------------------------------------------------------------
// 功能说明：
// 以类似于 Houdini "Copy and Transform"（copyxform）节点的方式，
// 对输入几何体进行多次复制，并根据用户设置对每个副本应用
// 平移、旋转和缩放变换，最终输出所有变换后的几何体集合。
//
// 详细行为：
// 1. 基本复制逻辑
//    - "Input" 几何体为要被复制的原始几何体。
//    - "Total Number" 用于指定要生成副本的总数量。
//      节点按从 0 到 (Total Number - 1) 的索引生成对应数量的副本。
//    - 所有副本基于原始输入几何体进行变换累积。
//
// 2. Translate（平移）
//    - 指定基础平移向量 (Vec3f)。
//    - 对于每一个副本，按顺序应用该平移向量。
//    - 可实现沿某方向重复移动（类似级联复制）。
//
// 3. Rotate（旋转）
//    - 指定基础旋转向量 (Vec3f)。
//    - 节点会将该旋转作为每次复制的旋转增量。
//    - 每个副本相对于前一个副本按该旋转进行叠加。
//
// 4. Scale（缩放）
//    - 指定每次复制的非 uniform 缩放向量 (Vec3f)。
//    - 用于控制每次复制时在 xyz 各方向的缩放变换。
//    - 该缩放值为乘法增量，会叠加应用到所有副本上。
//
// 5. Uniform Scale（统一缩放）
//    - 提供一个统一的缩放比例（float）。
//    - 该值将与 "Scale" 向量共同作用于缩放变换，可看作全局统一缩放因子。
//    - 若未使用该字段，则仅按 Vec3f 的缩放控制比例。
//
// 输出：
//    - "Output" 包含 Total Number 份基于输入几何体复制并依次应用
//      平移、旋转、缩放后的所有几何体。
//
// 节点行为示例：
//    - Total Number = 5, Translate = (1,0,0):
//      将在 X 方向上产生 5 个沿 X 轴均匀间隔排列的几何体副本。
//    - Rotate = (0,45,0):
//      每个副本比前一个绕 Y 轴旋转 45 度。
//    - Scale = (1.1,1.1,1.1), Uniform Scale = 0.9:
//      每次从基础几何体开始，先按 1.1 在 xyz 缩放，再整体按 0.9 缩放。
//
    ZENDEFNODE_ABI(CopyAndTransform,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Total Number", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(0, 20, 1) },
            { "Translate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Scale", _gParamType_Vec3f, ZVec3f(1,1,1) },
            { "Uniform Scale", _gParamType_Float, ZFloat(1.0f) }
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
