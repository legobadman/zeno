#include <utility>
#include <zeno/zeno.h>
#include <zeno/types/CameraObject.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/utils/log.h>

#include <zeno/zeno.h>
#include <zeno/utils/logger.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/DictObject.h>
#include <zeno/types/CameraObject.h>
#include <zeno/types/UserData.h>
#include <zeno/types/LightObject.h>

#include "assimp/scene.h"

#include "Definition.h"
#include "tinygltf/json.hpp"

#include <memory>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <fstream>

#define SET_CAMERA_DATA                         \
    out_pos = (n->pos);                       \
    out_up = (n->up);                         \
    out_view = (n->view);                     \
    out_fov = (n->fov);                       \
    out_aperture = (n->aperture);             \
    out_focalPlaneDistance = (n->focalPlaneDistance); \

namespace zeno {
namespace {

struct CihouMayaCameraFov : INode {
    virtual void apply() override {
        auto m_fit_gate = array_index_safe({"Horizontal", "Vertical"},
                                           zsString2Std(get_input2_string("fit_gate")),
                                           "fit_gate") + 1;
        auto m_focL = get_input2_float("focL");
        auto m_fw = get_input2_float("fw");
        auto m_fh = get_input2_float("fh");
        auto m_nx = get_input2_float("nx");
        auto m_ny = get_input2_float("ny");
        float c_fov = 0;
        float c_aspect = m_fw/m_fh;
        float u_aspect = m_ny&&m_nx? m_nx/m_ny : c_aspect;
        zeno::log_info("cam nx {} ny {} fw {} fh {} aspect {} {}",
                       m_nx, m_ny, m_fw, m_fh, u_aspect, c_aspect);
        std::cout << "m_fit_gate:" << m_fit_gate << "\n";
        std::cout << "u_aspect:" << u_aspect << "\n";
        std::cout << "c_aspect:" << c_aspect << "\n";
        if(m_fit_gate == 1){
            c_fov = 2.0f * std::atan(m_fh/(u_aspect/c_aspect) / (2.0f * m_focL) ) * (180.0f / M_PI);
        }else if(m_fit_gate == 2){
            c_fov = 2.0f * std::atan(m_fw/c_aspect / (2.0f * m_focL) ) * (180.0f / M_PI);
        }
        set_output_float("fov", c_fov);
    }
};

ZENO_DEFNODE(CihouMayaCameraFov)({
    {
        {"enum Horizontal Vertical", "fit_gate", "Horizontal"},
        {gParamType_Float, "focL", "35"},
        {gParamType_Float, "fw", "36"},
        {gParamType_Float, "fh", "24"},
        {gParamType_Float, "nx", "0"},
        {gParamType_Float, "ny", "0"},
    },
    {
        {gParamType_Float, "fov"},
    },
    {},
    {"FBX"},
});

struct CameraEval: zeno::INode {

    glm::quat to_quat(zeno::vec3f up, zeno::vec3f view){
        auto glm_view = -1.0f * glm::normalize(glm::vec3(view[0], view[1], view[2]));
        auto _up = glm::normalize(glm::vec3(up[0], up[1], up[2]));
        auto _view = glm::normalize(glm_view);
        auto _right = glm::normalize(glm::cross(_up, _view));
        glm::mat3 _rotate;
        _rotate[0][0] = _right[0]; _rotate[0][1] = _up[0]; _rotate[0][2] = _view[0];
        _rotate[1][0] = _right[1]; _rotate[1][1] = _up[1]; _rotate[1][2] = _view[1];
        _rotate[2][0] = _right[2]; _rotate[2][1] = _up[2]; _rotate[2][2] = _view[2];
        glm::quat rotation = glm::quat_cast(_rotate);
        return rotation;
    }

    virtual void apply() override {
        int frameid;
        if (has_input("frameid")) {
            frameid = get_input2_int("frameid");
        } else {
            frameid = GetFrameId();
        }
        
        std::vector<zeno::SharedPtr<zeno::CameraObject>> nodelist;
        for (auto spobj : get_input_ListObject("nodelist")->get()) {
            if (auto cam = std::dynamic_pointer_cast<zeno::CameraObject>(spobj)) {
                nodelist.push_back(cam);
            }
        }

        zeno::vec3f out_pos;
        zeno::vec3f out_up;
        zeno::vec3f out_view;
        float out_fov;
        float out_aperture;
        float out_focalPlaneDistance;

        if(nodelist.size() == 1){
            auto n = nodelist[0];
            SET_CAMERA_DATA
        }else{
            int ff = (int)nodelist[0]->userData()->get_float("frame");
            int lf = (int)nodelist[nodelist.size()-1]->userData()->get_float("frame");
            if(frameid <= ff){
                auto n = nodelist[0];
                SET_CAMERA_DATA
            }else if(frameid >= lf) {
                auto n = nodelist[nodelist.size()-1];
                SET_CAMERA_DATA
            }else{
                for(int i=1;i<nodelist.size();i++){
                    auto const & next_node = nodelist[i];
                    auto const & pre_node = nodelist[i-1];
                    int next_frame = (int)next_node->userData()->get_float("frame");
                    int pre_frame = (int)pre_node->userData()->get_float("frame");
                    int total_frame = next_frame - pre_frame;
                    float r = ((float)frameid - pre_frame) / total_frame;

                    if(frameid <= next_frame){
                        auto pos = pre_node->pos + (next_node->pos - pre_node->pos) * r;
                        auto fov = pre_node->fov + (next_node->fov - pre_node->fov) * r;
                        auto aperture = pre_node->aperture + (next_node->aperture - pre_node->aperture) * r;
                        auto focalPlane = pre_node->focalPlaneDistance + (next_node->focalPlaneDistance - pre_node->focalPlaneDistance) * r;

                        auto pre_quat = to_quat(pre_node->up, pre_node->view);
                        auto next_quat = to_quat(next_node->up, next_node->view);
                        auto quat_lerp = glm::slerp(pre_quat, next_quat, r);
                        glm::mat3 matrix_lerp = glm::toMat3(quat_lerp); // Convert quaternion to 3x3 matrix
                        auto right = zeno::vec3f(matrix_lerp[0][0], matrix_lerp[1][0], matrix_lerp[2][0]);
                        auto up = zeno::vec3f(matrix_lerp[0][1], matrix_lerp[1][1], matrix_lerp[2][1]);
                        auto view = zeno::vec3f(matrix_lerp[0][2], matrix_lerp[1][2], matrix_lerp[2][2]);

                        out_pos = (pos);
                        out_up = (up);
                        out_view = (-view);
                        out_fov = (fov);
                        out_aperture = (aperture);
                        out_focalPlaneDistance = (focalPlane);
                        break;
                    }
                }
            }
        }

        auto camera = std::make_shared<zeno::CameraObject>();

        camera->pos = out_pos;
        camera->up = out_up;
        camera->view = out_view;
        camera->fov = out_fov;
        camera->aperture = out_aperture;
        camera->focalPlaneDistance = out_focalPlaneDistance;

        set_output("camera", std::move(camera));
    }
};

ZENO_DEFNODE(CameraEval)({
    {
        {gParamType_Int, "frameid"},
        {gParamType_List, "nodelist"}
    },
    {
        {gParamType_Camera, "camera"},
    },
    {
    },
    {"FBX"},
});

struct ExtractCamera: zeno::INode {

    virtual void apply() override {
        auto cam = zeno::safe_dynamic_cast<zeno::CameraObject>(get_input("camobject"));

        auto pos = std::make_shared<zeno::NumericObject>();
        auto up = std::make_shared<zeno::NumericObject>();
        auto view = std::make_shared<zeno::NumericObject>();
        auto fov = std::make_shared<zeno::NumericObject>();
        auto aperture = std::make_shared<zeno::NumericObject>();
        auto focalPlaneDistance = std::make_shared<zeno::NumericObject>();

        pos->set<zeno::vec3f>(cam->pos);
        up->set<zeno::vec3f>(cam->up);
        view->set<zeno::vec3f>(cam->view);
        fov->set<float>(cam->fov);
        aperture->set<float>(cam->aperture);
        focalPlaneDistance->set<float>(cam->focalPlaneDistance);


        set_output("pos", std::move(pos));
        set_output("up", std::move(up));
        set_output("view", std::move(view));
        set_output("fov", std::move(fov));
        set_output("aperture", std::move(aperture));
        set_output("focalPlaneDistance", std::move(focalPlaneDistance));
    }
};
ZENDEFNODE(ExtractCamera,
           {       /* inputs: */
               {
                   {gParamType_Camera, "camobject"}
               },  /* outputs: */
               {
                   {gParamType_Vec3f,"pos"}, 
                   {gParamType_Vec3f,"up"}, 
                   {gParamType_Vec3f,"view"}, 
                   {gParamType_Float,"fov"}, 
                   {gParamType_Float,"aperture"}, 
                   {gParamType_Float,"focalPlaneDistance"}
               },  /* params: */
               {

               },  /* category: */
               {
                   "FBX",
               }
           });

struct DirtyTBN : INode {
    virtual void apply() override {

        auto AxisT = toVec3f(get_input2_vec3f("T"));
        auto AxisB = toVec3f(get_input2_vec3f("B"));
        //auto AxisN = get_input2<zeno::vec3f>("N");

        if (lengthSquared(AxisT) == 0 ) {
            AxisT = {1,0,0};
        }
        AxisT = zeno::normalize(AxisT);
        
        if (lengthSquared(AxisB) == 0 ) {
            AxisB = {0,0,1};
        }
        AxisB = zeno::normalize(AxisB);

        auto tmp = zeno::dot(AxisT, AxisB);
        if (abs(tmp) > 0.0) { // not vertical
            AxisB -= AxisT * tmp;
            AxisB = zeno::normalize(AxisB);
        }
        
        if (has_input("prim")) {
            auto prim = get_input_PrimitiveObject("prim");

            auto pos = toVec3f(prim->userData()->get_vec3f("pos", Vec3f()));
            auto scale = toVec3f(prim->userData()->get_vec3f("scale", Vec3f(1,1,1)));

            auto v0 = pos - AxisT * scale[0] * 0.5f - AxisB * scale[2] * 0.5f;
            auto e1 = AxisT * scale[0];
            auto e2 = AxisB * scale[2];

            prim->verts[0] = v0 + e1 + e2;
            prim->verts[1] = v0 + e1;
            prim->verts[2] = v0 + e2;
            prim->verts[3] = v0;

            set_output("prim", std::move(prim));
        }
    }
};


ZENO_DEFNODE(DirtyTBN)({
    {
        {gParamType_Primitive, "prim"},
        {gParamType_Vec3f, "T", "1, 0, 0"},
        {gParamType_Vec3f, "B", "0, 0, 1"},
    },
    {
        {gParamType_Primitive, "prim"}
    },
    {},
    {"shader"},
});

struct LiveMeshNode : INode {
    typedef std::vector<std::vector<float>> UVS;
    typedef std::vector<std::vector<float>> VERTICES;
    typedef std::vector<int> VERTEX_COUNT;
    typedef std::vector<int> VERTEX_LIST;

    struct PrimIngredient{
        UVS uvs;
        VERTICES vertices;
        VERTEX_COUNT vertexCount;
        VERTEX_LIST vertexList;
    };

    void GeneratePrimitiveObject(PrimIngredient& ingredient, std::shared_ptr<zeno::PrimitiveObject> primObject){
        auto& vert = primObject->verts;
        auto& loops = primObject->loops;
        auto& polys = primObject->polys;

        for(int i=0; i<ingredient.vertices.size(); i++){
            auto& v = ingredient.vertices[i];
            vert.emplace_back(v[0], v[1], v[2]);
        }

        int start = 0;
        for(int i=0; i<ingredient.vertexCount.size(); i++){
            auto count = ingredient.vertexCount[i];
            for(int j=start; j<start+count; j++){
                loops.emplace_back(ingredient.vertexList[j]);
            }
            polys.emplace_back(start, count);

            start += count;
        }

        primObject->uvs.resize(loops.size());
        for (auto i = 0; i < loops.size(); i++) {
            primObject->uvs[i] = vec2f(ingredient.uvs[i][0], ingredient.uvs[i][1]);
        }
        auto& loopuvs = primObject->loops.add_attr<int>("uvs");
        for (auto i = 0; i < loops.size(); i++) {
            loopuvs[i] = i;
        }
    }

    virtual void apply() override {
        auto outDict = get_input2_bool("outDict");
        auto prims_list = std::make_shared<zeno::ListObject>();
        auto prims_dict = std::make_shared<zeno::DictObject>();
        auto vertSrc = zsString2Std(get_input2_string("vertSrc"));

        int frameid;
        if (has_input("frameid")) {
            frameid = get_input2_int("frameid");
        } else {
            frameid = GetFrameId();
        }

        if(! vertSrc.empty()){
            using json = nlohmann::json;

            json parseData = json::parse(vertSrc);

            /*
            auto& frameData = parseData[std::to_string(frameid)];
            auto frameDataSize = frameData["DATA"].size();

            std::cout << "src size " << vertSrc.size()
                      << " data size " << frameDataSize
                      << " frame " << frameid
                      << "\n";

            auto& AllMeshData = frameData["DATA"];
            for(auto& mapItem: AllMeshData.items()){
                auto prim = std::make_shared<zeno::PrimitiveObject>();
                std::cout << "iter map key " << mapItem.key() << "\n";
                auto& mapData = mapItem.value();
                int vertices_size = mapData["MESH_POINTS"].size();
                int vertexCount_size = mapData["MESH_VERTEX_COUNTS"].size();
                int vertexList_size = mapData["MESH_VERTEX_LIST"].size();
                PrimIngredient ingredient;
                ingredient.vertices = mapData["MESH_POINTS"].get<VERTICES>();
                ingredient.vertexCount = mapData["MESH_VERTEX_COUNTS"].get<VERTEX_COUNT>();
                ingredient.vertexList = mapData["MESH_VERTEX_LIST"].get<VERTEX_LIST>();
                std::cout << "Vertices Size " << vertices_size << " " << vertexCount_size << " " << vertexList_size << "\n";
                GeneratePrimitiveObject(ingredient, prim);

                prims->arr.emplace_back(prim);
            }
             */

            auto& parsedFrameData = parseData[std::to_string(frameid)];
            if(! parsedFrameData.empty()){
                auto bPathI = parsedFrameData["BPATHI"].get<std::string>();
                auto bPath = parsedFrameData["BPATH"].get<std::string>();
                std::cout<< "bPath info " << bPathI << "\n";
                std::ifstream t(bPathI);
                std::stringstream buffer;
                buffer << t.rdbuf();

                auto& sizesData = parsedFrameData["SIZES"];

                json infoData = json::parse(buffer.str());
                for(auto& mapItem: infoData.items()){
                    auto& key = mapItem.key();
                    auto& value = mapItem.value();

                    auto sizes = sizesData[key].get<std::vector<int>>();

                    auto u = value["UV"].get<std::string>();
                    auto v = value["VERTEX"].get<std::string>();
                    auto i = value["INDICES"].get<std::string>();
                    auto c = value["COUNTS"].get<std::string>();

                    std::cout << "sync info " << key << " sizes " << sizes.size() << "\n";
                    std::cout << " u " << u << "\n";
                    std::cout << " v " << v << "\n";
                    std::cout << " i " << i << "\n";
                    std::cout << " c " << c << "\n";

                    auto pu = bPath+"/"+u;
                    auto pv = bPath+"/"+v;
                    auto pi = bPath+"/"+i;
                    auto pc = bPath+"/"+c;

                    std::cout << " u.p " << pu << " u.s " << sizes[0] << "\n";
                    std::cout << " v.p " << pv << " v.s " << sizes[1] << "\n";
                    std::cout << " i.p " << pi << " i.s " << sizes[2] << "\n";
                    std::cout << " c.p " << pc << " c.s " << sizes[3] << "\n";

                    FILE *fp_u = fopen(pu.c_str(), "rb");
                    FILE *fp_v = fopen(pv.c_str(), "rb");
                    FILE *fp_i = fopen(pi.c_str(), "rb");
                    FILE *fp_c = fopen(pc.c_str(), "rb");

                    float *_u = new float[sizes[0]];
                    float *_v = new float[sizes[1]];
                    int   *_i = new int[sizes[2]];
                    int   *_c = new int[sizes[3]];

                    fread((void*)(_u), sizeof(float), sizes[0], fp_u);
                    fread((void*)(_v), sizeof(float), sizes[1], fp_v);
                    fread((void*)(_i), sizeof(int), sizes[2], fp_i);
                    fread((void*)(_c), sizeof(int), sizes[3], fp_c);

                    UVS _vu{};
                    VERTICES _vv{};
                    VERTEX_LIST _vi{};
                    VERTEX_COUNT _vc{};

                    for(int s = 0; s < sizes[0]; s+=2){
                        _vu.push_back({_u[s], _u[s+1]});
                    }
                    for(int s = 0; s < sizes[1]; s+=3){
                        _vv.push_back({_v[s], _v[s+1], _v[s+2]});
                    }
                    for(int s = 0; s < sizes[2]; ++s){
                        _vi.push_back(_i[s]);
                    }
                    for(int s = 0; s < sizes[3]; ++s){
                        _vc.push_back(_c[s]);
                    }

                    delete [] _u;
                    delete [] _v;
                    delete [] _c;
                    delete [] _i;

                    fclose(fp_u);
                    fclose(fp_v);
                    fclose(fp_i);
                    fclose(fp_c);

                    PrimIngredient ingredient;
                    ingredient.uvs = _vu;
                    ingredient.vertices = _vv;
                    ingredient.vertexCount = _vc;
                    ingredient.vertexList = _vi;
                    auto prim = std::make_shared<zeno::PrimitiveObject>();
                    GeneratePrimitiveObject(ingredient, prim);
                    if(outDict) {
                        prims_dict->lut[key] = prim;
                    }else{
                        prims_list->push_back(prim);
                    }
                }
            }else{
                std::cout << "not parsed frame " << frameid << "\n";
            }
        }
        if(outDict) {
            set_output("prims", std::move(prims_dict));
        }else{
            set_output("prims", std::move(prims_list));
        }
    }
};

ZENO_DEFNODE(LiveMeshNode)({
    {
        {gParamType_Int, "frameid"},
        {gParamType_String, "vertSrc", ""},
        {gParamType_Bool, "outDict", "false"}
    },
    {
        {gParamType_List, "prims", ""}
    },
    {
    },
    {"FBX"},
});


struct LiveCameraNode : INode{
    typedef std::vector<float> CAMERA_TRANS;
    struct CameraIngredient{
        CAMERA_TRANS translation;
    };

    virtual void apply() override {
        auto camera = std::make_shared<zeno::CameraObject>();
        auto camSrc = zsString2Std(get_input2_string("camSrc"));

        if(! camSrc.empty()){
            std::cout << "src came " << camSrc.size() << "\n";
            using json = nlohmann::json;
            json parseData = json::parse(camSrc);
            int translation_size = parseData["translation"].size();
            CameraIngredient ingredient;
            ingredient.translation = parseData["translation"].get<CAMERA_TRANS>();
            std::cout << " translation_size " << translation_size << "\n";

            float transX = ingredient.translation[0];
            float transY = ingredient.translation[1];
            float transZ = ingredient.translation[2];
            float rotateX = ingredient.translation[3];
            float rotateY = ingredient.translation[4];
            float rotateZ = ingredient.translation[5];
            //float scaleX = ingredient.translation[6];
            //float scaleY = ingredient.translation[7];
            //float scaleZ = ingredient.translation[8];

            glm::mat4 transMatrixR = glm::translate(glm::vec3(transX, transY, -transZ));
            glm::mat4 transMatrixL = glm::translate(glm::vec3(transX, transY, transZ));
            float ax = rotateX * (M_PI / 180.0);
            float ay = rotateY * (M_PI / 180.0);
            float az = rotateZ * (M_PI / 180.0);
            glm::mat3 mx = glm::mat3(1,0,0,  0,cos(ax),-sin(ax),  0,sin(ax),cos(ax));
            glm::mat3 my = glm::mat3(cos(ay),0,sin(ay),  0,1,0,  -sin(ay),0,cos(ay));
            glm::mat3 mz = glm::mat3(cos(az),-sin(az),0,  sin(az),cos(az),0,  0,0,1);
            auto rotateMatrix3 = mx*my*mz;
            auto rotateMatrix4 = glm::mat4((rotateMatrix3));

            //auto matrix = transMatrixL * rotateMatrix4 * transMatrixR;
            auto matrix = rotateMatrix4;
            glm::vec3 trans, scale, skew; glm::quat rot; glm::vec4 perp;
            glm::decompose(matrix, trans, rot, scale, skew, perp);
            glm::mat3 rotMatrix = glm::mat3_cast(rot);

            camera->pos = zeno::vec3f(transX, transY, transZ);
            camera->view = zeno::vec3f(rotMatrix[2][0], rotMatrix[2][1], rotMatrix[2][2]);
            camera->up = zeno::vec3f(rotMatrix[1][0], rotMatrix[1][1], rotMatrix[1][2]);
            std::cout << "RotateMatrix\n\t" << rotMatrix[0][0] << " " << rotMatrix[0][1] << " " << rotMatrix[0][2]
                      << "\n\t" << rotMatrix[1][0] << " " << rotMatrix[1][1] << " " << rotMatrix[1][2]
                      << "\n\t" << rotMatrix[2][0] << " " << rotMatrix[2][1] << " " << rotMatrix[2][2] << "\n";
            std::cout << "pos " <<  trans[0] << " " << trans[1] << " " << trans[2] << "\n";
            std::cout << "view " <<  camera->view[0] << " " << camera->view[1] << " " << camera->view[2] << "\n";
            std::cout << "up " <<  camera->up[0] << " " << camera->up[1] << " " << camera->up[2] << "\n";
        }
        set_output("camera", std::move(camera));
    }
};

//ZENO_DEFNODE(LiveCameraNode)({
//    {
//        {gParamType_String, "camSrc", ""},
//    },
//    {
//        "camera"
//    },
//    {
//    },
//    {"FBX"},
//});

}
}
