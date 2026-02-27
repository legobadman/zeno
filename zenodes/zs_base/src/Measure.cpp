#include <vec.h>
#include <glm/glm.hpp>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <string>

namespace zeno {

    struct Measure : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override {
            auto input_geom = ptrNodeData->clone_input_Geometry("Input Object");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }

            std::string measure = get_string_param(ptrNodeData, "Measure");
            std::string outputAttrName = get_string_param(ptrNodeData, "Output Attribute Name");
            if (outputAttrName.empty()) outputAttrName = "area";

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();

            std::vector<Vec3f> posAbi((size_t)nPts);
            size_t gotPos = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (gotPos != (size_t)nPts) {
                ptrNodeData->report_error("failed to read input positions");
                return ZErr_ParamError;
            }
            std::vector<glm::vec3> pos((size_t)nPts);
            for (int i = 0; i < nPts; ++i) {
                pos[(size_t)i] = glm::vec3(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            }

            std::vector<std::vector<int>> faces;
            faces.reserve((size_t)nFaces);
            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 1) continue;
                std::vector<int> face((size_t)nv);
                size_t gotFace = input_geom->face_points(f, face.data(), nv);
                if (gotFace == (size_t)nv) faces.push_back(std::move(face));
            }

            std::vector<float> measurements(faces.size(), 0.0f);
            for (size_t fi = 0; fi < faces.size(); ++fi) {
                const auto& face = faces[fi];
                int n = (int)face.size();
                if (measure == "Area") {
                    if (n < 3) continue;
                    glm::vec3 p0 = pos[(size_t)face[0]];
                    float area = 0.0f;
                    for (int i = 1; i < n - 1; ++i) {
                        glm::vec3 p1 = pos[(size_t)face[(size_t)i]];
                        glm::vec3 p2 = pos[(size_t)face[(size_t)(i + 1)]];
                        area += 0.5f * glm::length(glm::cross(p1 - p0, p2 - p0));
                    }
                    measurements[fi] = area;
                }
                else if (measure == "Length") {
                    float len = 0.0f;
                    if (n == 2) {
                        len = glm::length(pos[(size_t)face[1]] - pos[(size_t)face[0]]);
                    }
                    else if (n > 2) {
                        for (int i = 0; i < n; ++i) {
                            int j = (i + 1) % n;
                            len += glm::length(pos[(size_t)face[(size_t)j]] - pos[(size_t)face[(size_t)i]]);
                        }
                    }
                    measurements[fi] = len;
                }
            }

            input_geom->create_attr_by_float(ATTR_FACE, outputAttrName.c_str(), measurements.data(), (int)measurements.size());
            ptrNodeData->set_output_object("Output", input_geom);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Measure,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Measure", _gParamType_String, ZString("Area"), Combobox, Z_STRING_ARRAY("Area", "Length") },
            { "Output Attribute Name", _gParamType_String, ZString("area"), Lineedit }
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

