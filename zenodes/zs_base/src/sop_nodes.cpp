#include <vec.h>
#include "glm/gtc/matrix_transform.hpp"
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include <Windows.h>
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>


namespace zeno {

#if 0
    struct CopyToPoints : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }

        void getIconResource(char* recv, size_t cap) override {}
        void getBackgroundClr(char* recv, size_t cap) override {}
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(INodeData* d) override {

            auto input_object = d->get_input_Geometry("Object To Be Copied");
            auto target_Obj = d->get_input_Geometry("Points From");

            char buf[256]{};
            d->get_input2_string("Align To", buf, sizeof(buf));
            std::string alignTo = buf;

            if (!input_object || !target_Obj)
            {
                return ZErr_ParamError;
            }

            if (!input_object->has_point_attr("pos") ||
                !target_Obj->has_point_attr("pos"))
            {
                return ZErr_ParamError;
            }

            auto inputPos = input_object->get_vec3f_attr(ATTR_POINT, "pos");

            std::vector<std::tuple<bool, std::vector<int>>> inputFacesPoints(
                input_object->nfaces());

            for (int i = 0; i < input_object->nfaces(); ++i) {
                inputFacesPoints[i] = { input_object->isLineFace(i), input_object->face_points(i) };
            }

            bool hasNrm = input_object->has_point_attr("nrm");
            std::vector<zeno::vec3f> inputNrm;
            if (hasNrm) {
                input_object->get_vec3f_attr();
                inputNrm = input_object->get_vec3f_attr(ATTR_POINT, "nrm");
            }


            zeno::vec3f originCenter, _min, _max;
            std::tie(_min, _max) = geomBoundingBox(input_object);

            originCenter =
                alignTo == "Align To Point Center" ?
                (_min + _max) / 2 : _min;

            size_t tgtPts = target_Obj->npoints();
            size_t inPts = input_object->npoints();
            size_t inFaces = input_object->nfaces();

            size_t newPts = tgtPts * inPts;

            std::vector<zeno::vec3f> newPos(newPts);
            std::vector<zeno::vec3f> newNrm;
            if (hasNrm) newNrm.resize(newPts);

            std::vector<std::vector<int>> faces;

            for (size_t i = 0; i < tgtPts; i++) {

                auto tp = target_Obj->get_vec3f_elem("pos", i);
                auto dx = tp - originCenter;

                glm::mat4 T = glm::translate(glm::vec3(dx[0], dx[1], dx[2]));

                size_t pt_off = i * inPts;

                for (size_t j = 0; j < inPts; j++) {

                    auto& pt = inputPos[j];
                    glm::vec4 gp = T * glm::vec4(pt[0], pt[1], pt[2], 1);

                    newPos[pt_off + j] = zeno::vec3f(gp.x, gp.y, gp.z);
                    if (hasNrm) newNrm[pt_off + j] = inputNrm[j];
                }

                for (size_t j = 0; j < inFaces; j++) {
                    auto fp = std::get<1>(inputFacesPoints[j]);
                    for (auto& k : fp) k += pt_off;
                    faces.push_back(fp);
                }
            }

            auto geo = create_GeometryObject(
                Topo_IndiceMesh,
                input_object->is_base_triangle(),
                newPos,
                faces);

            if (hasNrm) {
                size_t cnt;
                auto abi = convert_points_to_abi(newNrm, cnt);
                geo->create_attr_by_vec3(ATTR_POINT, "nrm", abi.get(), cnt);
            }

            d->set_output_object("Output", geo);
        }
    };


    ZENDEFNODE_ABI(CopyToPoints,
        Z_INPUTS(
            { "Object To Be Copied", _gParamType_Geometry },
            { "Points From", _gParamType_Geometry },
            { "Align To", _gParamType_String,
              ZString("Align To Point Center"),
              Combobox,
              Z_STRING_ARRAY("Align To Point Center", "Align To Min") },
            { "Translate By Attribute", _gParamType_String },
            { "Scale By Attribute", _gParamType_String },
            { "Rotate By Attribute", _gParamType_String }
        ),

        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),

        "geometry",
        ""
    );
#endif

    struct Merge : zeno::INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }

        void getIconResource(char* recv, size_t cap) override {
            recv[0] = 0;
        }

        void getBackgroundClr(char* recv, size_t cap) override {
            recv[0] = 0;
        }

        float time() const override { return 1.f; }

        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto list_object = ptrNodeData->get_input_ListObject("Input Of Objects");
            if (!list_object) {
                ptrNodeData->report_error("empty input list");
                return ZErr_ParamError;
            }

            auto mergedObj = zeno::mergeObjects(list_object);
            ptrNodeData->set_output_object("Output", mergedObj);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Merge,
        Z_INPUTS(
            { "Input Of Objects", _gParamType_List }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "prim",
        ""
    );
}