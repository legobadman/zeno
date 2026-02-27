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
#include <string>
#include <limits>
#include <glm/glm.hpp>

namespace zeno {

    static void bboxFromPositionsMatchSize(IGeometryObject* geom, glm::vec3& outMin, glm::vec3& outMax) {
        int n = geom->npoints();
        if (n <= 0 || !geom->has_point_attr("pos")) {
            outMin = outMax = glm::vec3(0.0f);
            return;
        }

        std::vector<Vec3f> posAbi((size_t)n);
        size_t got = geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), n);
        if (got != (size_t)n) {
            outMin = outMax = glm::vec3(0.0f);
            return;
        }

        outMin = glm::vec3(std::numeric_limits<float>::max());
        outMax = glm::vec3(std::numeric_limits<float>::lowest());
        for (int i = 0; i < n; ++i) {
            glm::vec3 p(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            outMin = glm::min(outMin, p);
            outMax = glm::max(outMax, p);
        }
    }

    struct MatchSize : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(INodeData* ptrNodeData) override {
            auto input_geom = ptrNodeData->get_input_Geometry("Input Object");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            if (nPts == 0 || nFaces == 0) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            Vec3f targetPositionAbi = ptrNodeData->get_input2_vec3f("Target Position");
            Vec3f targetSizeAbi = ptrNodeData->get_input2_vec3f("Target Size");
            glm::vec3 targetPosition(targetPositionAbi.x, targetPositionAbi.y, targetPositionAbi.z);
            glm::vec3 targetSize(targetSizeAbi.x, targetSizeAbi.y, targetSizeAbi.z);
            bool doTranslate = ptrNodeData->get_input2_bool("Translate");
            bool doScaleToFit = ptrNodeData->get_input2_bool("Scale To Fit");
            std::string translateXTo = get_string_param(ptrNodeData, "Translate X To");
            std::string translateYTo = get_string_param(ptrNodeData, "Translate Y To");
            std::string translateZTo = get_string_param(ptrNodeData, "Translate Z To");

            if (!doTranslate && !doScaleToFit) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            glm::vec3 srcMin, srcMax;
            bboxFromPositionsMatchSize(input_geom, srcMin, srcMax);
            glm::vec3 srcSize = srcMax - srcMin;

            auto match_geom = ptrNodeData->get_input_Geometry("Match Object");
            glm::vec3 boxMin, boxMax;
            if (match_geom && match_geom->npoints() > 0 && match_geom->has_point_attr("pos")) {
                bboxFromPositionsMatchSize(match_geom, boxMin, boxMax);
            }
            else {
                boxMin = targetPosition - 0.5f * targetSize;
                boxMax = targetPosition + 0.5f * targetSize;
            }
            glm::vec3 boxSize = boxMax - boxMin;

            if (translateXTo == "Min") {
                boxMin.x += boxSize.x * 0.5f;
                boxMax.x += boxSize.x * 0.5f;
            }
            else if (translateXTo == "Max") {
                boxMin.x -= boxSize.x * 0.5f;
                boxMax.x -= boxSize.x * 0.5f;
            }

            if (translateYTo == "Min") {
                boxMin.y += boxSize.y * 0.5f;
                boxMax.y += boxSize.y * 0.5f;
            }
            else if (translateYTo == "Max") {
                boxMin.y -= boxSize.y * 0.5f;
                boxMax.y -= boxSize.y * 0.5f;
            }

            if (translateZTo == "Min") {
                boxMin.z += boxSize.z * 0.5f;
                boxMax.z += boxSize.z * 0.5f;
            }
            else if (translateZTo == "Max") {
                boxMin.z -= boxSize.z * 0.5f;
                boxMax.z -= boxSize.z * 0.5f;
            }

            glm::mat4 M(1.0f);
            if (doScaleToFit) {
                M = glm::scale(M, boxSize);
            }
            if (doTranslate) {
                glm::vec3 boxCenter = 0.5f * (boxMin + boxMax);
                glm::vec3 srcCenter = 0.5f * (srcMin + srcMax);
                glm::vec3 centerOffset = boxCenter - srcCenter;
                M = glm::translate(M, centerOffset);
            }

            std::vector<Vec3f> posAbi((size_t)nPts);
            size_t nr = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (nr != (size_t)nPts) {
                ptrNodeData->report_error("failed to read input positions");
                return ZErr_ParamError;
            }
            std::vector<glm::vec3> pos((size_t)nPts);
            for (int i = 0; i < nPts; ++i) {
                pos[(size_t)i] = glm::vec3(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            }

            bool hasNrm = input_geom->has_point_attr("nrm");
            std::vector<glm::vec3> nrm;
            if (hasNrm) {
                std::vector<Vec3f> nrmAbi((size_t)nPts);
                input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
                nrm.resize((size_t)nPts);
                for (int i = 0; i < nPts; ++i) {
                    nrm[(size_t)i] = glm::vec3(nrmAbi[(size_t)i].x, nrmAbi[(size_t)i].y, nrmAbi[(size_t)i].z);
                }
            }

            std::vector<glm::vec3> outPos((size_t)nPts);
            std::vector<glm::vec3> outNrm;
            if (hasNrm) outNrm.resize((size_t)nPts);
            applyTransform(M, pos.data(), (size_t)nPts, outPos.data(), hasNrm ? nrm.data() : nullptr, hasNrm ? outNrm.data() : nullptr);

            std::vector<std::vector<int>> faces;
            faces.reserve((size_t)nFaces);
            for (int iFace = 0; iFace < nFaces; ++iFace) {
                int nv = input_geom->face_vertex_count(iFace);
                if (nv < 3) continue;
                std::vector<int> f((size_t)nv);
                size_t got = input_geom->face_points(iFace, f.data(), nv);
                if (got == (size_t)nv) faces.push_back(std::move(f));
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, outPos, faces);
            if (!output) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }

            if (hasNrm) {
                std::vector<Vec3f> nrmAbi((size_t)nPts);
                for (int i = 0; i < nPts; ++i) {
                    nrmAbi[(size_t)i] = Vec3f{ outNrm[(size_t)i].x, outNrm[(size_t)i].y, outNrm[(size_t)i].z };
                }
                output->create_attr_by_vec3(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
            }

            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(MatchSize,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Match Object", _gParamType_Geometry },
            { "Target Position", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Target Size", _gParamType_Vec3f, ZVec3f(1,1,1) },
            { "Translate", _gParamType_Bool, ZInt(1), Checkbox },
            { "Translate X To", _gParamType_String, ZString("Center"), Combobox, Z_STRING_ARRAY("Min", "Center", "Max") },
            { "Translate Y To", _gParamType_String, ZString("Center"), Combobox, Z_STRING_ARRAY("Min", "Center", "Max") },
            { "Translate Z To", _gParamType_String, ZString("Center"), Combobox, Z_STRING_ARRAY("Min", "Center", "Max") },
            { "Scale To Fit", _gParamType_Bool, ZInt(1), Checkbox }
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

