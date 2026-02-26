#include "simple_geometry_common.h"

namespace zeno {

struct Grid : INode2 {

    enum PlaneDirection {
        Dir_XY,
        Dir_YZ,
        Dir_ZX
    };

    DEF_OVERRIDE_FOR_INODE

    ZErrorCode apply(INodeData* ptrNodeData) override {
        zeno::vec3f Center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
        zeno::vec3f Rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));
        zeno::vec2f Size = toVec2f(ptrNodeData->get_input2_vec2f("Size"));
        int Rows = ptrNodeData->get_input2_int("Rows");
        int Columns = ptrNodeData->get_input2_int("Columns");
        std::string face_type = get_string_param(ptrNodeData, "Face Type");
        std::string Direction = get_string_param(ptrNodeData, "Direction");
        bool bCalcPointNormals = ptrNodeData->get_input2_bool("Point Normal");

        if (Rows < 2 || Columns < 2) {
            return ZErr_ParamError;
        }

        float size1 = Size[0], size2 = Size[1];
        float step1 = size1 / (Rows - 1), step2 = size2 / (Columns - 1);
        float bottom1 = -size1 / 2, up1 = bottom1 + size1;
        float bottom2 = -size2 / 2, up2 = bottom2 + size2;
        bool bQuad = face_type == "Quadrilaterals";

        int nPoints = Rows * Columns;
        int nFaces = (Rows - 1) * (Columns - 1);
        if (!bQuad) {
            nFaces *= 2;
        }

        PlaneDirection dir;
        Rotate_Orientaion ori;
        if (Direction == "ZX") {
            dir = Dir_ZX;
            ori = Orientaion_ZX;
        }
        else if (Direction == "YZ") {
            dir = Dir_YZ;
            ori = Orientaion_YZ;
        }
        else if (Direction == "XY") {
            dir = Dir_XY;
            ori = Orientaion_XY;
        }
        else {
            return ZErr_ParamError;
        }

        std::vector<vec3f> points, normals;
        std::vector<std::vector<int>> faces;
        points.resize(nPoints);
        faces.reserve(nFaces);
        if (bCalcPointNormals)
            normals.resize(nPoints);

        for (size_t i = 0; i < Rows; i++) {
            for (size_t j = 0; j < Columns; j++) {
                vec3f pt, nrm;
                if (dir == Dir_ZX) {
                    pt = vec3f(bottom2 + step2 * j, 0, bottom1 + step1 * i);
                    nrm = vec3f(0, 1, 0);
                }
                else if (dir == Dir_YZ) {
                    pt = vec3f(0, bottom1 + step1 * i, bottom2 + step2 * j);
                    nrm = vec3f(1, 0, 0);
                }
                else {
                    pt = vec3f(bottom1 + step1 * i, bottom2 + step2 * j, 0);
                    nrm = vec3f(0, 0, -1);
                }

                int idx = (int)(i * Columns + j);
                points[idx] = pt;
                if (bCalcPointNormals)
                    normals[idx] = nrm;

                if (j > 0 && i > 0) {
                    int ij = idx;
                    int ij_1 = idx - 1;
                    int i_1j = (int)((i - 1) * Columns + j);
                    int i_1j_1 = i_1j - 1;

                    if (bQuad) {
                        std::vector<int> _face = { ij, i_1j, i_1j_1, ij_1 };
                        faces.push_back(_face);
                    }
                    else {
                        std::vector<int> _face = { ij, i_1j, i_1j_1 };
                        faces.push_back(_face);
                        _face = { ij, i_1j_1, ij_1 };
                        faces.push_back(_face);
                    }
                }
            }
        }

        glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
        glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
        glm::mat4 transform = translate * rotation;
        for (size_t i = 0; i < points.size(); i++)
        {
            auto pt = points[i];
            glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
            points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
            if (bCalcPointNormals) {
                auto nrm = normals[i];
                glm::vec4 gnrm = rotation * glm::vec4(nrm[0], nrm[1], nrm[2], 0);
                normals[i] = zeno::vec3f(gnrm.x, gnrm.y, gnrm.z);
            }
        }

        auto geo = create_GeometryObject(Topo_HalfEdge, !bQuad, to_glm_points(points), faces);
        if (bCalcPointNormals) {
            size_t outCount = 0;
            auto nrmData = convert_points_to_abi(normals, outCount);
            geo->create_attr_by_vec3(ATTR_POINT, "nrm", nrmData.get(), outCount);
        }
        ptrNodeData->set_output_object("Output", std::move(geo));
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Grid,
    Z_INPUTS(
        { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Size", _gParamType_Vec2f, ZVec2f(1,1) },
        { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Rows", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(1, 100, 1) },
        { "Columns", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(1, 100, 1) },
        { "Direction", _gParamType_String, ZString("ZX"), Combobox, Z_STRING_ARRAY("XY", "YZ", "ZX") },
        { "Uniform Scale", _gParamType_Float, ZFloat(1.0f), Lineedit },
        { "Face Type", _gParamType_String, ZString("Quadrilaterals"), Combobox, Z_STRING_ARRAY("Triangles", "Quadrilaterals") },
        { "Point Normal", _gParamType_Bool, ZInt(0), Checkbox }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "geometry",
    "create a cube",
    "",
    ""
);

} // namespace zeno

