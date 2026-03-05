//
// FBX SDK based nodes for projects/FBX (zs_fbx).
// New version using only interface ABI and FBX SDK.
//

#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <Windows.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <zcommon.h>
#include <zenum.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <set>
#include <numeric>
#include <filesystem>
#include "tinygltf/json.hpp"
#include <mutex>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include "format.h"
#include "zs_primitive.h"
#include "stringhelper.h"
#include "api_zs_fbx.h"

#ifdef ZENO_FBXSDK
#include <fbxsdk.h>
#endif

namespace zeno {

    using Json = nlohmann::json;
    namespace fs = std::filesystem;

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[1024] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

// Simple wrapper around FBX SDK manager + scene, shared via shared_ptr so
// multiple node instances can reference the same loaded file.
struct FBXObject : IObject2 {

    struct Inner {
        FbxManager* lSdkManager = nullptr;
        FbxScene*   lScene = nullptr;
        ~Inner() {
            if (lScene) {
                lScene->Destroy();
                lScene = nullptr;
            }
            if (lSdkManager) {
                lSdkManager->Destroy();
                lSdkManager = nullptr;
            }
        }
    };
    std::shared_ptr<Inner> inner;

    std::string m_key;

    FBXObject() = default;

    IObject2* clone() const override {
        auto* o = new FBXObject();
#ifdef ZENO_FBXSDK
        o->inner = inner;
#endif
        o->m_key = m_key;
        return o;
    }

    size_t key(char* buf, size_t buf_size) const override {
        if (!buf || buf_size == 0) return 0;
        const size_t n = std::min(buf_size - 1, m_key.size());
        std::memcpy(buf, m_key.data(), n);
        buf[n] = '\0';
        return n;
    }

    void update_key(const char* key) override {
        m_key = key ? key : "";
    }

    size_t serialize_json(char* buf, size_t buf_size) const override {
        (void)buf; (void)buf_size;
        return 0;
    }

    IUserData2* userData() override {
        return nullptr;
    }

    void Delete() override {
        delete this;
    }

    ZObjectType type() const override {
        return static_cast<ZObjectType>(_gParamType_FBXObject);
    }
};

// ReadFBXFile: minimal wrapper that loads an FBX scene and exposes it as FBXObject.
struct ReadFBXFile : INode2 {
    DEF_OVERRIDE_FOR_INODE

    std::string m_usedPath;
    std::shared_ptr<FBXObject::Inner> m_cachedInner;

    ZErrorCode apply(INodeData* nd) override {
        const std::string path = get_input2_string(nd, "path");
        if (path.empty()) {
            nd->report_error("ReadFBXFile: empty path");
            return ZErr_ParamError;
        }

#ifndef ZENO_FBXSDK
        nd->report_error("ReadFBXFile: ZENO_FBXSDK not enabled at build time");
        return ZErr_ParamError;
#else
        if (path == m_usedPath && m_cachedInner) {
            auto* obj = new FBXObject();
            obj->inner = m_cachedInner;
            obj->update_key(path.c_str());
            nd->set_output_object("fbx_object", obj);
            return ZErr_OK;
        }

        FbxManager* manager = FbxManager::Create();
        if (!manager) {
            nd->report_error("ReadFBXFile: failed to create FbxManager");
            return ZErr_ParamError;
        }

        FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
        manager->SetIOSettings(ios);

        FbxImporter* importer = FbxImporter::Create(manager, "");
        if (!importer->Initialize(path.c_str(), -1, manager->GetIOSettings())) {
            std::string err = "ReadFBXFile: FbxImporter::Initialize failed: ";
            err += importer->GetStatus().GetErrorString();
        importer->Destroy();
            manager->Destroy();
            nd->report_error(err.c_str());
            return ZErr_ParamError;
        }

        FbxScene* scene = FbxScene::Create(manager, "scene");
        if (!scene) {
            importer->Destroy();
            manager->Destroy();
            nd->report_error("ReadFBXFile: failed to create FbxScene");
            return ZErr_ParamError;
        }

        importer->Import(scene);
        FbxRootNodeUtility::RemoveAllFbxRoots(scene);
        importer->Destroy();

        auto inner = std::make_shared<FBXObject::Inner>();
        inner->lSdkManager = manager;
        inner->lScene = scene;
        m_cachedInner = inner;
        m_usedPath = path;

        auto* obj = new FBXObject();
        obj->inner = inner;
        obj->update_key(path.c_str());
        nd->set_output_object("fbx_object", obj);

        return ZErr_OK;
#endif
    }
};

ZENDEFNODE_ABI(ReadFBXFile,
    Z_INPUTS(
        {"path", _gParamType_String, ZString(""), ReadPathEdit}
    ),
    Z_OUTPUTS(
        {"fbx_object", _gParamType_FBXObject}
    ),
    "FBXSDK",
    "",
    "",
    ""
);




std::mutex s_fbx_mutex;


struct ParseFBX : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        auto lFilename = get_input2_string(ptrNodeData, "FBX Path");
        int start_frame = ptrNodeData->get_input2_int("Start Frame");
        int end_frame = std::lround(ptrNodeData->get_input2_int("End Frame"));

        // Initialize the SDK manager. This object handles all our memory management.
        std::vector<std::unique_ptr<PrimitiveObject>> prims;
        std::vector<std::string> scene_info_list;
        {
            std::lock_guard scopeLock(s_fbx_mutex);
        FbxManager* lSdkManager = FbxManager::Create();

        // Create the IO settings object.
            FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
        lSdkManager->SetIOSettings(ios);

        // Create an importer using the SDK manager.
            FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

        // Use the first argument as the filename for the importer.
            if (!lImporter->Initialize(lFilename.c_str(), -1, lSdkManager->GetIOSettings())) {
            printf("Call to FbxImporter::Initialize() failed.\n");
            printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
            exit(-1);
        }
        int major, minor, revision;
        lImporter->GetFileVersion(major, minor, revision);
        auto fbx_object = std::make_unique<FBXObject>();

            fbx_object->inner->lSdkManager = lSdkManager;
        // Create a new scene so that it can be populated by the imported file.
            fbx_object->inner->lScene = FbxScene::Create(lSdkManager, "myScene");

        // Import the contents of the file into the scene.
            lImporter->Import(fbx_object->inner->lScene);
            FbxRootNodeUtility::RemoveAllFbxRoots(fbx_object->inner->lScene);

        // The file is imported; so get rid of the importer.
        lImporter->Destroy();
        fbx_object->userData()->set_vec3i("version", zeno::Vec3i(major, minor, revision));
            fbx_object->userData()->set_string("file_path", lFilename.c_str());

            auto lScene = fbx_object->inner->lScene;
            // Print the nodes of the scene and their attributes recursively.
            // Note that we are not printing the root node because it should
            // not contain any attributes.
            FbxNode* lRootNode = lScene->GetRootNode();
            bool output_tex_even_missing = ptrNodeData->get_input2_bool("OutputTexEvenMissing");

            if (lRootNode) {
                TraverseNodesToGetPrims(lRootNode, prims, output_tex_even_missing, "", false);
            }

            auto vectors_str = get_input2_string(ptrNodeData, "vectors");
            std::vector<std::string> vectors = zeno::split_str(vectors_str, ',');

            for (auto& prim : prims) {
                if (ptrNodeData->get_input2_bool("CopyVectorsFromLoopsToVert")) {
                    for (auto vector : vectors) {
                        vector = zeno::trim_string(vector);
                        if (vector.size() && prim->loops.attr_is<Vec3f>(vector)) {
                            auto& nrm = prim->loops.attr<Vec3f>(vector);
                            auto& vnrm = prim->verts.add_attr<Vec3f>(vector);
                            for (auto i = 0; i < prim->loops.size(); i++) {
                                vnrm[prim->loops[i]] += nrm[i];
                            }
                            for (auto i = 0; i < prim->verts.size(); i++) {
                                vnrm[i] = normalizeSafe(vnrm[i]);
                            }
                        }
                    }
                }
                if (ptrNodeData->get_input2_bool("CopyFacesetToMatid")) {
                    prim_copy_faceset_to_matid(prim.get());
                }
            }

            for (int frameid = start_frame; frameid <= end_frame; frameid++) {
                float fps = ptrNodeData->get_input2_float("fps");
                float t = float(frameid) / fps;
                FbxTime curTime;       // The time for each key in the animation curve(s)
                curTime.SetSecondDouble(t);   // Starting time

                auto lScene = fbx_object->inner->lScene;
                FbxNode* lRootNode = lScene->GetRootNode();
                Json json;
                if (lRootNode != nullptr) {
                    TraverseNodesToGetJson(lRootNode, json, curTime);
                }
                scene_info_list.push_back(json.dump());
            }

            lSdkManager->Destroy();
        }

        std::vector<std::string> abc_paths;
        abc_paths.reserve(prims.size());

        auto geo_list = createList();
        for (auto& prim : prims) {
            auto spGeom = create_GeometryObject(prim.get());

            auto abc_path = zsString2Std(spGeom->userData()->get_string("abcpath_0"));
            abc_paths.push_back(abc_path);
            geo_list->push_back(std::move(spGeom));
        }
        set_output("Scene Json List", std::move(scene_info_list));
        set_output("Geometry List", std::move(geo_list));
    }
};
    
ZENDEFNODE_ABI(ParseFBX,
    Z_INPUTS(
        { "FBX Path", _gParamType_String, ZString(""), zeno::ReadPathEdit },
        { "vectors", _gParamType_String, ZString("nrm,tang") },
        { "CopyVectorsFromLoopsToVert", _gParamType_Bool, ZInt(1) },
        { "CopyFacesetToMatid", _gParamType_Bool, ZInt(1) },
        { "OutputTexEvenMissing", _gParamType_Bool, ZInt(0) },
        { "SkipInvisibleMesh", _gParamType_Bool, ZInt(0) },
        { "Start Frame", _gParamType_Int, ZInt(0) },
        { "End Frame", _gParamType_Int, ZInt(0) },
        { "fps", _gParamType_Float, ZFloat(25.0f) }
    ),
    Z_OUTPUTS(
        { "Geometry List", _gParamType_List },
        { "Scene Json List", _gParamType_StringList }
    ),
    "FBXSDK",
    "",
    "",
    ""
);


/**
* Return a string-based representation based on the attribute type.
*/
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
    switch(type) {
        case FbxNodeAttribute::eUnknown: return "unidentified";
        case FbxNodeAttribute::eNull: return "null";
        case FbxNodeAttribute::eMarker: return "marker";
        case FbxNodeAttribute::eSkeleton: return "skeleton";
        case FbxNodeAttribute::eMesh: return "mesh";
        case FbxNodeAttribute::eNurbs: return "nurbs";
        case FbxNodeAttribute::ePatch: return "patch";
        case FbxNodeAttribute::eCamera: return "camera";
        case FbxNodeAttribute::eCameraStereo: return "stereo";
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
        case FbxNodeAttribute::eLight: return "light";
        case FbxNodeAttribute::eOpticalReference: return "optical reference";
        case FbxNodeAttribute::eOpticalMarker: return "marker";
        case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
        case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
        case FbxNodeAttribute::eBoundary: return "boundary";
        case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
        case FbxNodeAttribute::eShape: return "shape";
        case FbxNodeAttribute::eLODGroup: return "lodgroup";
        case FbxNodeAttribute::eSubDiv: return "subdiv";
        default: return "unknown";
    }
}

/**
* Print an attribute.
*/
void PrintAttribute(FbxNodeAttribute* pAttribute) {
    if(!pAttribute) return;

    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    // Note: to retrieve the character array of a FbxString, use its Buffer() method.
    printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

//void PrintNode(FbxNode* pNode) {
//    const char* nodeName = pNode->GetName();
//    FbxDouble3 translation = pNode->LclTranslation.Get();
//    FbxDouble3 rotation = pNode->LclRotation.Get();
//    FbxDouble3 scaling = pNode->LclScaling.Get();
//
//    // Print the contents of the node.
//    printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
//           nodeName,
//           translation[0], translation[1], translation[2],
//           rotation[0], rotation[1], rotation[2],
//           scaling[0], scaling[1], scaling[2]
//    );
//
//    // Print the node's attributes.
//    for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
//        PrintAttribute(pNode->GetNodeAttributeByIndex(i));
//
//    // Recursively print the children.
//    for(int j = 0; j < pNode->GetChildCount(); j++)
//        PrintNode(pNode->GetChild(j));
//
//    printf("</node>\n");
//}

template<typename T>
void getAttr(T* arr, std::string name, PrimitiveObject* prim) {
    if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint) {
//        zeno::log_info("{}, eByControlPoint", name);
        auto &attr = prim->verts.add_attr<Vec3f>(name);
        for (auto i = 0; i < prim->verts.size(); i++) {
            int pIndex = i;
            if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect) {
                pIndex = arr->GetIndexArray().GetAt(i);
            }
            auto x = arr->GetDirectArray().GetAt(pIndex)[0];
            auto y = arr->GetDirectArray().GetAt(pIndex)[1];
            auto z = arr->GetDirectArray().GetAt(pIndex)[2];
            attr[i] = Vec3f(x, y, z);
        }
    }
    else if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex) {
        auto &attr = prim->loops.add_attr<Vec3f>(name);
        for (auto i = 0; i < prim->loops.size(); i++) {
            int pIndex = i;
            if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect) {
                pIndex = arr->GetIndexArray().GetAt(i);
            }
            auto x = arr->GetDirectArray().GetAt(pIndex)[0];
            auto y = arr->GetDirectArray().GetAt(pIndex)[1];
            auto z = arr->GetDirectArray().GetAt(pIndex)[2];
            attr[i] = Vec3f(x, y, z);
        }
    }
}

static std::unique_ptr<PrimitiveObject> GetMesh(
        FbxNode* pNode
        , bool output_tex_even_missing
        , std::string fbx_path
        , bool apply_transform
    ) {
    FbxMesh* pMesh = pNode->GetMesh();
    if (!pMesh) return nullptr;
    auto mesh_name = pMesh->GetName();
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") {
        nodeName = "ABC";
    }
    auto prim = std::make_unique<PrimitiveObject>();
    prim->userData()->set_string("RootName", stdString2zs(nodeName));
    prim->userData()->set_string("_abc_name", stdString2zs(nodeName));
    prim->userData()->set_string("fbx_path", stdString2zs(fbx_path));

    FbxAMatrix bindMatrix = pNode->EvaluateGlobalTransform();
    auto s = bindMatrix.GetS();
    auto t = bindMatrix.GetT();
//    zeno::log_info("s {} {} {}", s[0], s[1], s[2]);
//    zeno::log_info("t {} {} {}", t[0], t[1], t[2]);

    FbxAMatrix Geometry;
    {
        FbxVector4 Translation, Rotation, Scaling;
        Translation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
        Rotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
        Scaling = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
        Geometry.SetT(Translation);
        Geometry.SetR(Rotation);
        Geometry.SetS(Scaling);
        FbxAMatrix PivotGeometry;
        FbxVector4 RotationPivot = pNode->GetRotationPivot(FbxNode::eSourcePivot);
        FbxVector4 FullPivot;
        FullPivot[0] = -RotationPivot[0];
        FullPivot[1] = -RotationPivot[1];
        FullPivot[2] = -RotationPivot[2];
        PivotGeometry.SetT(FullPivot);
        Geometry = Geometry * PivotGeometry;
    }

    int numVertices = pMesh->GetControlPointsCount();
    FbxVector4* vertices = pMesh->GetControlPoints();
    prim->verts.resize(numVertices);

    for (int i = 0; i < numVertices; ++i) {
        if (apply_transform) {
            auto pos = Geometry.MultT(FbxVector4(vertices[i][0], vertices[i][1], vertices[i][2], 1.0));
            pos = bindMatrix.MultT(pos);
            prim->verts[i] = Vec3f(pos[0], pos[1], pos[2]);
        }
        else {
            auto pos = Geometry.MultT(FbxVector4(vertices[i][0], vertices[i][1], vertices[i][2], 1.0));
            prim->verts[i] = Vec3f(pos[0], pos[1], pos[2]);
        }
    }

    int numPolygons = pMesh->GetPolygonCount();
    prim->polys.resize(numPolygons);
    std::vector<int> loops;
    loops.reserve(numPolygons * 4);
    int count = 0;
    for (int i = 0; i < numPolygons; ++i) {
        int numVertices = pMesh->GetPolygonSize(i);
        for (int j = 0; j < numVertices; ++j) {
            int vertexIndex = pMesh->GetPolygonVertex(i, j);
            loops.push_back(vertexIndex);
        }
        prim->polys[i] = {count, numVertices};
        count += numVertices;
    }
    loops.shrink_to_fit();
    prim->loops.values = loops;
//    zeno::log_info("pMesh->GetDeformerCount(FbxDeformer::eSkin) {}", pMesh->GetDeformerCount(FbxDeformer::eSkin));
    auto ud = prim->userData();
    if (pMesh->GetDeformerCount(FbxDeformer::eSkin)) {

        FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
        std::vector<std::string> bone_names;
        // Iterate over each cluster (bone)
        std::vector<std::vector<std::pair<int, float>>> bone_weight(numVertices);
        for (int j = 0; j < pSkin->GetClusterCount(); ++j) {
            FbxCluster* pCluster = pSkin->GetCluster(j);

            // Get the link node (bone)
            FbxNode* pBoneNode = pCluster->GetLink();
            if (!pBoneNode) continue;

            // Get the bone weights
            int numIndices = pCluster->GetControlPointIndicesCount();
            int* indices = pCluster->GetControlPointIndices();
            double* weights = pCluster->GetControlPointWeights();

            bone_names.emplace_back(pBoneNode->GetName());
            for (int k = 0; k < numIndices; ++k) {
                bone_weight[indices[k]].emplace_back(j, weights[k]);
                    }
                }
        int maxnum_boneWeight = 0;
        for (auto i = 0; i < prim->verts.size(); i++) {
            maxnum_boneWeight = zeno::max(maxnum_boneWeight, bone_weight[i].size());
            }
        for (auto i = 0; i < maxnum_boneWeight; i++) {
            auto &bi = prim->verts.add_attr<int>(zeno::format("boneName_{}", i));
            std::fill(bi.begin(), bi.end(), -1);
            auto &bw = prim->verts.add_attr<float>(zeno::format("boneWeight_{}", i));
            std::fill(bw.begin(), bw.end(), -1.0f);
        }
        for (auto i = 0; i < prim->verts.size(); i++) {
            for (auto j = 0; j < bone_weight[i].size(); j++) {
                prim->verts.attr<int>(format("boneName_{}", j))[i] = bone_weight[i][j].first;
                prim->verts.attr<float>(format("boneWeight_{}", j))[i] = bone_weight[i][j].second;
            }
        }
        ud->set_int("maxnum_boneWeight", int(maxnum_boneWeight));
        ud->set_int("boneName_count", int(bone_names.size()));
        for (auto i = 0; i < bone_names.size(); i++) {
            ud->set_string(stdString2zs(zeno::format("boneName_{}", i)), stdString2zs(bone_names[i]));
        }
    }
    if (pMesh->GetElementUVCount() > 0) {
        auto* arr = pMesh->GetElementUV(0);
        std::string name = "uv";
        if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint) {
            zeno::log_info("{}, eByControlPoint", name);
            auto &attr = prim->verts.add_attr<Vec3f>(name);
            for (auto i = 0; i < prim->verts.size(); i++) {
                int pIndex = i;
                if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect) {
                    pIndex = arr->GetIndexArray().GetAt(i);
                }
                auto x = arr->GetDirectArray().GetAt(pIndex)[0];
                auto y = arr->GetDirectArray().GetAt(pIndex)[1];
                attr[i] = Vec3f(x, y, 0);
            }
        }
        else if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex) {
            if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect) {
                auto &uvs = prim->loops.add_attr<int>("uvs");
                std::iota(uvs.begin(), uvs.end(), 0);
                prim->uvs.resize(prim->loops.size());
            }
            else if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect) {
                auto &uvs = prim->loops.add_attr<int>("uvs");
                for (auto i = 0; i < prim->loops.size(); i++) {
                    uvs[i] = arr->GetIndexArray().GetAt(i);
                }
                int count = arr->GetDirectArray().GetCount();
                prim->uvs.resize(count);
            }
            for (auto i = 0; i < prim->uvs.size(); i++) {
                auto x = arr->GetDirectArray().GetAt(i)[0];
                auto y = arr->GetDirectArray().GetAt(i)[1];
                prim->uvs[i] = Vec2f(x, y);
            }
        }
    }
    if (pMesh->GetElementVertexColorCount()>0)
    {
        getAttr(pMesh->GetElementVertexColor(0),"clr",prim.get());
    }
    if (pMesh->GetElementNormalCount() > 0) {
        getAttr(pMesh->GetElementNormal(0), "nrm", prim.get());
    }
    if (pMesh->GetElementTangentCount() > 0) {
        getAttr(pMesh->GetElementTangent(0), "tang", prim.get());
    }
    auto &faceset = prim->polys.add_attr<int>("faceset");
    std::fill(faceset.begin(), faceset.end(), -1);
    int mat_count = 0;
    if (pMesh->GetElementMaterialCount() > 0) {
        for (auto i = 0; i < numPolygons; ++i) {
            faceset[i] = pMesh->GetElementMaterial()->GetIndexArray().GetAt(i);
        }
        mat_count = pNode->GetMaterialCount();
        for (auto i = 0; i < mat_count; i++) {
            FbxSurfaceMaterial* material = pNode->GetMaterial(i);
            ud->set_string(stdString2zs(format("faceset_{}", i)), material->GetName());
        }
    }
    ud->set_int("faceset_count", mat_count);
    prim_set_abcpath(prim.get(), stdString2zs(fbx_path + '/' + mesh_name));
    if (mat_count > 0) {
        for (auto i = 0; i < mat_count; i++) {
            FbxSurfaceMaterial* material = pNode->GetMaterial(i);
            std::string mat_name = material->GetName();
            ud->set_string(stdString2zs(format("faceset_{}", i)), stdString2zs(mat_name));
            Json json;
            
            {
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sEmissive);
                    if (output_tex_even_missing) {
                        json["emissive_tex"] = "";
                    }
                    if (property.IsValid()) {
                        FbxDouble3 value = property.Get<FbxDouble3>();
                        json["emissive_value"] = {value[0], value[1], value[2]};
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["emissive_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sAmbient);
                    if (output_tex_even_missing) {
                        json["ambient_tex"] = "";
                    }
                    if (property.IsValid()) {
                        FbxDouble3 value = property.Get<FbxDouble3>();
                        json["ambient_value"] = {value[0], value[1], value[2]};
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["ambient_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                    if (output_tex_even_missing) {
                        json["diffuse_tex"] = "";
                    }
                    if (property.IsValid()) {
                        FbxDouble3 value = property.Get<FbxDouble3>();
                        json["diffuse_value"] = {value[0], value[1], value[2]};
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["diffuse_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sSpecular);
                    if (output_tex_even_missing) {
                        json["specular_tex"] = "";
                    }
                    if (property.IsValid()) {
                        FbxDouble3 value = property.Get<FbxDouble3>();
                        json["specular_value"] = {value[0], value[1], value[2]};
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["specular_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sShininess);
                    if (output_tex_even_missing) {
                        json["shininess_tex"] = "";
                    }
                    if (property.IsValid()) {
                        double value = property.Get<double>();
                        json["shininess_value"] = value;
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["shininess_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sBump);
                    if (output_tex_even_missing) {
                        json["bump_tex"] = "";
                    }
                    if (property.IsValid()) {
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["bump_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
                    if (output_tex_even_missing) {
                        json["normal_map_tex"] = "";
                    }
                    if (property.IsValid()) {
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["normal_map_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sTransparentColor);
                    if (output_tex_even_missing) {
                        json["transparent_color_tex"] = "";
                    }
                    if (property.IsValid()) {
                        FbxDouble3 value = property.Get<FbxDouble3>();
                        json["transparent_color_value"] = {value[0], value[1], value[2]};
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["transparent_color_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
                    if (output_tex_even_missing) {
                        json["opacity_tex"] = "";
                    }
                    if (property.IsValid()) {
                        double value = property.Get<double>();
                        json["opacity_value"] = value;
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["opacity_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sReflection);
                    if (output_tex_even_missing) {
                        json["reflection_tex"] = "";
                    }
                    if (property.IsValid()) {
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["reflection_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDisplacementColor);
                    if (output_tex_even_missing) {
                        json["displacement_color_tex"] = "";
                    }
                    if (property.IsValid()) {
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["displacement_color_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
                {
                    FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sVectorDisplacementColor);
                    if (output_tex_even_missing) {
                        json["vector_displacement_color_tex"] = "";
                    }
                    if (property.IsValid()) {
                        int textureCount = property.GetSrcObjectCount<FbxTexture>();
                        for (int i = 0; i < textureCount; ++i) {
                            FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(i));
                            if (texture) {
                                json["vector_displacement_color_tex"] = texture->GetFileName();
                            }
                        }
                    }
                }
            }
            ud->set_string(stdString2zs(mat_name), stdString2zs(json.dump()));
        }
    }
    return prim;
}

static std::unique_ptr<PrimitiveObject> GetSkeleton(FbxNode* pNode) {
    FbxMesh* pMesh = pNode->GetMesh();
    if (!pMesh) return nullptr;
    std::vector<std::string> bone_names;
    std::vector<Vec3f> poss;
    std::vector<Vec3f> transform_r0;
    std::vector<Vec3f> transform_r1;
    std::vector<Vec3f> transform_r2;
    std::map<std::string, std::string> parent_mapping;
    if (pMesh->GetDeformerCount(FbxDeformer::eSkin)) {
        FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
        // Iterate over each cluster (bone)
        for (int j = 0; j < pSkin->GetClusterCount(); ++j) {
            FbxCluster* pCluster = pSkin->GetCluster(j);

            FbxNode* pBoneNode = pCluster->GetLink();
            if (!pBoneNode) continue;
            FbxAMatrix transformLinkMatrix;
            pCluster->GetTransformLinkMatrix(transformLinkMatrix);

            // The transformation of the mesh at binding time
            FbxAMatrix transformMatrix;
            pCluster->GetTransformMatrix(transformMatrix);

            // Inverse bind matrix.
            FbxAMatrix bindMatrix_ = transformMatrix.Inverse() * transformLinkMatrix;
            auto bindMatrix = bit_cast<FbxMatrix>(bindMatrix_);
            auto t = bindMatrix.GetRow(3);
            poss.emplace_back(t[0], t[1], t[2]);

            auto r0 = bindMatrix.GetRow(0);
            auto r1 = bindMatrix.GetRow(1);
            auto r2 = bindMatrix.GetRow(2);
            transform_r0.emplace_back(r0[0], r0[1], r0[2]);
            transform_r1.emplace_back(r1[0], r1[1], r1[2]);
            transform_r2.emplace_back(r2[0], r2[1], r2[2]);
            std::string boneName = pBoneNode->GetName();
            bone_names.emplace_back(boneName);
            auto pParentNode = pBoneNode->GetParent();
            if (pParentNode) {
                std::string parentName = pParentNode->GetName();
                parent_mapping[boneName] = parentName;
            }
        }
    }
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") {
        nodeName = "ABC";
    }
    auto prim = std::make_unique<PrimitiveObject>();
    prim->userData()->set_string("RootName", stdString2zs(nodeName));
    prim->verts.resize(bone_names.size());
    prim->verts.values = poss;
    prim->verts.add_attr<Vec3f>("transform_r0") = transform_r0;
    prim->verts.add_attr<Vec3f>("transform_r1") = transform_r1;
    prim->verts.add_attr<Vec3f>("transform_r2") = transform_r2;
    std::vector<int> bone_connects;
    for (auto bone_name: bone_names) {
        if (parent_mapping.count(bone_name)) {
            auto parent_name = parent_mapping[bone_name];
            if (std::count(bone_names.begin(), bone_names.end(), parent_name)) {
                auto self_index = std::find(bone_names.begin(), bone_names.end(), bone_name) - bone_names.begin();
                auto parent_index = std::find(bone_names.begin(), bone_names.end(), parent_name) - bone_names.begin();
                bone_connects.push_back(parent_index);
                bone_connects.push_back(self_index);
            }
        }
    }
    prim->loops.values = bone_connects;
    prim->polys.resize(bone_connects.size() / 2);
    for (auto j = 0; j < bone_connects.size() / 2; j++) {
        prim->polys[j] = {j * 2, 2};
    }
    auto &boneNames = prim->verts.add_attr<int>("boneName");
    std::iota(boneNames.begin(), boneNames.end(), 0);
    prim->userData()->set_int("boneName_count", int(bone_names.size()));
    for (auto i = 0; i < bone_names.size(); i++) {
        prim->userData()->set_string(stdString2zs(zeno::format("boneName_{}", i)), 
            stdString2zs(bone_names[i]));
    }
    return prim;
}

static void TraverseNodesToGetNames(FbxNode* pNode, std::vector<std::string> &names) {
    if (!pNode) return;

    FbxMesh* mesh = pNode->GetMesh();
    if (mesh) {
        auto name = pNode->GetName();
        names.emplace_back(name);
    }

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        TraverseNodesToGetNames(pNode->GetChild(i), names);
    }
}
static void TraverseNodesToGetJson(FbxNode* pNode, Json &json, FbxTime curTime) {
    if (!pNode) return;
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") {
        nodeName = "ABC";
    }
    auto pMesh = pNode->GetMesh();
    if (pMesh) {
        auto mesh_name = pMesh->GetName();
        json["mesh"] = mesh_name;
    }
    json["visibility"] = int(pNode->GetVisibility());
    json["node_name"] = nodeName;
    {
        FbxAMatrix bindMatrix = pNode->EvaluateLocalTransform(curTime);
        auto r0 = bindMatrix.GetRow(0);
        auto r1 = bindMatrix.GetRow(1);
        auto r2 = bindMatrix.GetRow(2);
        auto t = bindMatrix.GetRow(3);
        if (
            std::isnan(r0[0]) || std::isnan(r0[1]) || std::isnan(r0[2])
            || std::isnan(r1[0]) || std::isnan(r1[1]) || std::isnan(r1[2])
            || std::isnan(r2[0]) || std::isnan(r2[1]) || std::isnan(r2[2])
            || std::isnan(t[0]) || std::isnan(t[1]) || std::isnan(t[2])
        ) {
            json["r0"] = {0.0, 0.0, 0.0};
            json["r1"] = {0.0, 0.0, 0.0};
            json["r2"] = {0.0, 0.0, 0.0};
            json["t"]  = {0.0, 0.0, 0.0};
        } else {
            json["r0"] = {r0[0], r0[1], r0[2]};
            json["r1"] = {r1[0], r1[1], r1[2]};
            json["r2"] = {r2[0], r2[1], r2[2]};
            json["t"]  = {t[0], t[1], t[2]};
        }
    }
    json["children_name"] = Json::array();
    for (int i = 0; i < pNode->GetChildCount(); i++) {
        Json child;
        TraverseNodesToGetJson(pNode->GetChild(i), child, curTime);
        std::string childName = child["node_name"];
        json[childName] = child;
        json["children_name"].push_back(childName);
    }
}

static void TraverseNodesToGetPrim(
    FbxNode* pNode
    , std::string target_name
    , std::unique_ptr<PrimitiveObject>& prim
    , bool output_tex_even_missing
    , std::string fbx_path
    , bool apply_transform
) {
    if (!pNode) return;
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") {
        nodeName = "ABC";
    }
    fbx_path = fbx_path + '/' + nodeName;

    FbxMesh* mesh = pNode->GetMesh();
    if (mesh) {
        auto name = pNode->GetName();
        if (target_name == name) {
            auto sub_prim = GetMesh(pNode, output_tex_even_missing, fbx_path, apply_transform);
            if (sub_prim) {
                prim = std::move(sub_prim);
        }
            return;
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        TraverseNodesToGetPrim(pNode->GetChild(i), target_name, prim, output_tex_even_missing, fbx_path, apply_transform);
    }
}
static void TraverseNodesToGetPrims(
    FbxNode* pNode, std::vector<std::unique_ptr<PrimitiveObject>>& prims
    , bool output_tex_even_missing
    , std::string fbx_path
    , bool apply_transform
) {
    if (!pNode) return;
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") {
        nodeName = "ABC";
    }
    fbx_path = fbx_path + '/' + nodeName;

    FbxMesh* mesh = pNode->GetMesh();
    if (mesh) {
        auto sub_prim = GetMesh(pNode, output_tex_even_missing, fbx_path, apply_transform);
        if (sub_prim) {
            prims.push_back(std::move(sub_prim));
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        TraverseNodesToGetPrims(pNode->GetChild(i), prims, output_tex_even_missing, fbx_path, apply_transform);
    }
}



static int GetSkeletonFromBindPose(FbxManager* lSdkManager, FbxScene* lScene, PrimitiveObject* prim) {
        auto pose_count = lScene->GetPoseCount();
        bool found_bind_pose = false;
        for (auto i = 0; i < pose_count; i++) {
            auto pose = lScene->GetPose(i);
            if (pose == nullptr || !pose->IsBindPose()) {
                continue;
            }
            found_bind_pose = true;
        }
        if (found_bind_pose == false) {
            lSdkManager->CreateMissingBindPoses(lScene);
        }
        pose_count = lScene->GetPoseCount();

        std::vector<std::string> bone_names;
        std::map<std::string, std::string> parent_mapping;
        std::vector<Vec3f> poss;
        std::vector<Vec3f> transform_r0;
        std::vector<Vec3f> transform_r1;
        std::vector<Vec3f> transform_r2;
        for (auto i = 0; i < pose_count; i++) {
            auto pose = lScene->GetPose(i);
            if (pose == nullptr || !pose->IsBindPose()) {
                continue;
            }
            for (int j = 1; j < pose->GetCount(); ++j) {
                std::string bone_name = pose->GetNode(j)->GetName();
                if (std::count(bone_names.begin(), bone_names.end(), bone_name)) {
                    continue;
                }

                FbxMatrix transformMatrix = pose->GetMatrix(j);
                auto t = transformMatrix.GetRow(3);
                poss.emplace_back(t[0], t[1], t[2]);

                auto r0 = transformMatrix.GetRow(0);
                auto r1 = transformMatrix.GetRow(1);
                auto r2 = transformMatrix.GetRow(2);
                transform_r0.emplace_back(r0[0], r0[1], r0[2]);
                transform_r1.emplace_back(r1[0], r1[1], r1[2]);
                transform_r2.emplace_back(r2[0], r2[1], r2[2]);

                bone_names.emplace_back(pose->GetNode(j)->GetName());
            }
            for (int j = 1; j < pose->GetCount(); ++j) {
                auto self_name = pose->GetNode(j)->GetName();
                auto parent = pose->GetNode(j)->GetParent();
                if (parent) {
                    auto parent_name = parent->GetName();
                    parent_mapping[self_name] = parent_name;
                }
            }
        }
    {
        prim->verts.resize(bone_names.size());
        prim->verts.values = poss;
        prim->verts.add_attr<Vec3f>("transform_r0") = transform_r0;
        prim->verts.add_attr<Vec3f>("transform_r1") = transform_r1;
        prim->verts.add_attr<Vec3f>("transform_r2") = transform_r2;
        auto &boneNames = prim->verts.add_attr<int>("boneName");
        std::iota(boneNames.begin(), boneNames.end(), 0);

        std::vector<int> bone_connects;
        for (auto bone_name: bone_names) {
            if (parent_mapping.count(bone_name)) {
                auto parent_name = parent_mapping[bone_name];
                if (std::count(bone_names.begin(), bone_names.end(), parent_name)) {
                    auto self_index = std::find(bone_names.begin(), bone_names.end(), bone_name) - bone_names.begin();
                    auto parent_index = std::find(bone_names.begin(), bone_names.end(), parent_name) - bone_names.begin();
                    bone_connects.push_back(parent_index);
                    bone_connects.push_back(self_index);
                }
            }
        }
        prim->loops.values = bone_connects;
        prim->polys.resize(bone_connects.size() / 2);
        for (auto j = 0; j < bone_connects.size() / 2; j++) {
            prim->polys[j] = {j * 2, 2};
        }

        prim->userData()->set_int("boneName_count", int(bone_names.size()));
        for (auto i = 0; i < bone_names.size(); i++) {
            prim->userData()->set_string(stdString2zs(zeno::format("boneName_{}", i))
                , stdString2zs(bone_names[i]));
        }
        return pose_count;
    }
    return pose_count;
}

static void TraverseNodesToGetSkeleton(FbxNode* pNode, std::vector<std::string> &bone_names, std::vector<FbxMatrix> &transforms, std::map<std::string, std::string> &parent_mapping) {
    if (!pNode) return;

    FbxMesh* pMesh = pNode->GetMesh();
    if (pMesh && pMesh->GetDeformerCount(FbxDeformer::eSkin)) {
        FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
        // Iterate over each cluster (bone)
        for (int j = 0; j < pSkin->GetClusterCount(); ++j) {
            FbxCluster* pCluster = pSkin->GetCluster(j);

            FbxNode* pBoneNode = pCluster->GetLink();
            if (!pBoneNode) continue;
            std::string boneName = pBoneNode->GetName();
            if (std::count(bone_names.begin(), bone_names.end(), boneName)) {
                continue;
            }
            bone_names.emplace_back(boneName);
            FbxAMatrix transformLinkMatrix;
            pCluster->GetTransformLinkMatrix(transformLinkMatrix);

            // The transformation of the mesh at binding time
            FbxAMatrix transformMatrix;
            pCluster->GetTransformMatrix(transformMatrix);

            // Inverse bind matrix.
            FbxAMatrix bindMatrix_ = transformMatrix.Inverse() * transformLinkMatrix;
            auto bindMatrix = bit_cast<FbxMatrix>(bindMatrix_);
            transforms.emplace_back(bindMatrix);

            auto pParentNode = pBoneNode->GetParent();
            if (pParentNode) {
                std::string parentName = pParentNode->GetName();
                parent_mapping[boneName] = parentName;
            }
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        TraverseNodesToGetSkeleton(pNode->GetChild(i), bone_names, transforms, parent_mapping);
    }
}
std::unique_ptr<PrimitiveObject> GetSkeletonFromMesh(FbxScene* lScene) {
    auto prim = std::make_unique<PrimitiveObject>();

    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        std::vector<std::string> bone_names;
        std::vector<FbxMatrix> transforms;
        std::map<std::string, std::string> parent_mapping;
        TraverseNodesToGetSkeleton(lRootNode, bone_names, transforms, parent_mapping);
        std::vector<Vec3f> poss;
        std::vector<Vec3f> transform_r0;
        std::vector<Vec3f> transform_r1;
        std::vector<Vec3f> transform_r2;
        for (auto i = 0; i < bone_names.size(); i++) {
            auto bone_name = bone_names[i];
            auto bindMatrix = transforms[i];
            auto t = bindMatrix.GetRow(3);
            poss.emplace_back(t[0], t[1], t[2]);

            auto r0 = bindMatrix.GetRow(0);
            auto r1 = bindMatrix.GetRow(1);
            auto r2 = bindMatrix.GetRow(2);
            transform_r0.emplace_back(r0[0], r0[1], r0[2]);
            transform_r1.emplace_back(r1[0], r1[1], r1[2]);
            transform_r2.emplace_back(r2[0], r2[1], r2[2]);
        }
        prim->verts.resize(bone_names.size());
        prim->verts.values = poss;
        prim->verts.add_attr<Vec3f>("transform_r0") = transform_r0;
        prim->verts.add_attr<Vec3f>("transform_r1") = transform_r1;
        prim->verts.add_attr<Vec3f>("transform_r2") = transform_r2;
        std::vector<int> bone_connects;
        for (auto bone_name: bone_names) {
            if (parent_mapping.count(bone_name)) {
                auto parent_name = parent_mapping[bone_name];
                if (std::count(bone_names.begin(), bone_names.end(), parent_name)) {
                    auto self_index = std::find(bone_names.begin(), bone_names.end(), bone_name) - bone_names.begin();
                    auto parent_index = std::find(bone_names.begin(), bone_names.end(), parent_name) - bone_names.begin();
                    bone_connects.push_back(parent_index);
                    bone_connects.push_back(self_index);
                }
            }
        }
        prim->loops.values = bone_connects;
        prim->polys.resize(bone_connects.size() / 2);
        for (auto j = 0; j < bone_connects.size() / 2; j++) {
            prim->polys[j] = {j * 2, 2};
        }
        auto &boneNames = prim->verts.add_attr<int>("boneName");
        std::iota(boneNames.begin(), boneNames.end(), 0);
        prim->userData()->set_int("boneName_count", int(bone_names.size()));
        for (auto i = 0; i < bone_names.size(); i++) {
            prim->userData()->set_string(
                stdString2zs(zeno::format("boneName_{}", i)), stdString2zs(bone_names[i]));
        }
    }
    return prim;
}


struct NewFBXSceneInfo : INode2 {
    DEF_OVERRIDE_FOR_INODE

    ZErrorCode apply(INodeData* nd) override {
        int frameid = 0;
        if (nd->has_link_input("frameid")) {
            frameid = nd->get_input2_int("frameid");
        } else {
            frameid = nd->GetFrameId();
        }

        const float fps = nd->get_input2_float("fps");
        if (fps <= 0.0f) {
            nd->report_error("NewFBXSceneInfo: fps must be > 0");
            return ZErr_ParamError;
        }

#ifdef ZENO_FBXSDK
        FbxTime curTime;
        curTime.SetSecondDouble(static_cast<double>(frameid) / static_cast<double>(fps));
#endif

        IListObject* json_list = nd->get_input_ListObject("Json List");
        if (!json_list) {
            nd->report_error("NewFBXSceneInfo: Json List is null");
            return ZErr_ParamError;
        }

        const int start_frame = nd->get_input2_int("Start Frame");
        const int idx = frameid - start_frame;
        const int count = static_cast<int>(json_list->size());
        if (idx < 0 || idx >= count) {
            nd->report_error("NewFBXSceneInfo: frame index out of range");
            return ZErr_ParamError;
        }

        IObject2* json_obj = json_list->get(static_cast<size_t>(idx));
        if (!json_obj) {
            nd->report_error("NewFBXSceneInfo: selected element is null");
            return ZErr_ParamError;
        }

        nd->set_output_object("json", json_obj->clone());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(NewFBXSceneInfo,
    Z_INPUTS(
        { "Json List", _gParamType_StringList },
        { "Start Frame", _gParamType_Int },
        { "frameid", _gParamType_Int },
        { "fps", _gParamType_Float, ZFloat(25.0f) }
    ),
    Z_OUTPUTS(
        { "json", _gParamType_String }
    ),
    "FBXSDK",
    "",
    "",
    ""
);


static glm::mat4 get_xfrom_from_json(Json json, const std::string &fbx_path) {
    auto names = split_str(fbx_path, '/');
    if (!names.empty()) {
        if (names.begin()->empty()) {
            names.erase(names.begin());
        }
    }
    glm::mat4 total = glm::mat4(1);
    for (auto &name: names) {
        json = json[name];

        Json r0 = json["r0"];
        Json r1 = json["r1"];
        Json r2 = json["r2"];
        Json  t = json["t"];
        glm::mat4 local = glm::mat4(1);
        local[0] = {float(r0[0]), float(r0[1]), float(r0[2]), 0};
        local[1] = {float(r1[0]), float(r1[1]), float(r1[2]), 0};
        local[2] = {float(r2[0]), float(r2[1]), float(r2[2]), 0};
        local[3] = {float( t[0]), float( t[1]), float( t[2]), 1};
        total = total * local;
    }
    return total;
}
static int get_visibility_from_json(Json json, const std::string &fbx_path) {
    auto names = split_str(fbx_path, '/');
    if (!names.empty()) {
        if (names.begin()->empty()) {
            names.erase(names.begin());
        }
    }
    int visibility = 1;
    glm::mat4 total = glm::mat4(1);
    for (auto &name: names) {
        json = json[name];
        visibility = json["visibility"];
    }
    return visibility;
}


struct NewFBXGeometryList : INode2 {
    DEF_OVERRIDE_FOR_INODE

    Vec3f transform_pos(glm::mat4& transform, Vec3f pos) {
        auto p = transform * glm::vec4(pos[0], pos[1], pos[2], 1);
        return { p.x, p.y, p.z };
    }
    Vec3f transform_nrm(glm::mat4& transform, Vec3f pos) {
        auto p = glm::transpose(glm::inverse(transform)) * glm::vec4(pos[0], pos[1], pos[2], 0);
        return { p.x, p.y, p.z };
    }

    ZErrorCode apply(INodeData* nd) override {
#ifndef ZENO_FBXSDK
        nd->report_error("NewFBXGeometryList: ZENO_FBXSDK not enabled at build time");
        return ZErr_ParamError;
#else
        IObject2* obj = nd->get_input_object("fbx_object");
        auto* fbx_object = dynamic_cast<FBXObject*>(obj);
        if (!fbx_object || !fbx_object->inner || !fbx_object->inner->lScene) {
            nd->report_error("NewFBXGeometryList: invalid fbx_object input");
            return ZErr_ParamError;
        }
        auto* lScene = fbx_object->inner->lScene;

        // Print the nodes of the scene and their attributes recursively.
        // Note that we are not printing the root node because it should
        // not contain any attributes.
        FbxNode* lRootNode = lScene->GetRootNode();
        bool output_tex_even_missing = nd->get_input2_bool("OutputTexEvenMissing");
        std::vector<std::unique_ptr<PrimitiveObject>> prims;
        if (lRootNode) {
            TraverseNodesToGetPrims(lRootNode, prims, output_tex_even_missing, "", false);
        }

        auto vectors_str = zsString2Std(get_input2_string(nd, "vectors"));
        std::vector<std::string> vectors = zeno::split_str(vectors_str, ',');
        if (nd->has_input("scene_info")) {
            auto* scene_obj = nd->get_input_object("scene_info");
            auto* json = dynamic_cast<JsonObject*>(scene_obj);
            std::vector<std::unique_ptr<PrimitiveObject>> new_prims;
            for (auto& prim : prims) {
                auto ud = prim->userData();
                auto fbx_path = zsString2Std(ud->get_string("fbx_path"));
                if (nd->get_input2_bool("SkipInvisibleMesh")) {
                    if (get_visibility_from_json(json->json, fbx_path)) {
                        new_prims.push_back(std::move(prim));
                    }
                }
                else {
                    new_prims.push_back(std::move(prim));
                }
            }
            prims = std::move(new_prims);
            for (auto& prim : prims) {
                auto ud = prim->userData();
                auto fbx_path = zsString2Std(ud->get_string("fbx_path"));
                glm::mat4 xform = get_xfrom_from_json(json->json, fbx_path);
                for (auto& v : prim->verts) {
                    v = transform_pos(xform, v);
                }
                for (auto& vector : vectors) {
                    if (prim->verts.attr_is<Vec3f>(vector)) {
                        auto& attr = prim->verts.attr<Vec3f>(vector);
                        for (auto& v : attr) {
                            v = transform_nrm(xform, v);
                        }
                    }
                    else if (prim->loops.attr_is<Vec3f>(vector)) {
                        auto& attr = prim->loops.attr<Vec3f>(vector);
                        for (auto& v : attr) {
                            v = transform_nrm(xform, v);
                        }
                    }
                }
            }
        }

        for (auto& prim : prims) {
            if (nd->get_input2_bool("CopyVectorsFromLoopsToVert")) {
                for (auto vector : vectors) {
                    vector = zeno::trim_string(vector);
                    if (vector.size() && prim->loops.attr_is<Vec3f>(vector)) {
                        auto& nrm = prim->loops.attr<Vec3f>(vector);
                        auto& vnrm = prim->verts.add_attr<Vec3f>(vector);
                        for (auto i = 0; i < prim->loops.size(); i++) {
                            vnrm[prim->loops[i]] += nrm[i];
                        }
                        for (auto i = 0; i < prim->verts.size(); i++) {
                            vnrm[i] = normalizeSafe(vnrm[i]);
                        }
                    }
                }
            }
            if (nd->get_input2_bool("CopyFacesetToMatid")) {
                prim_copy_faceset_to_matid(prim.get());
            }
        }
        auto geo_list = std::make_unique<zeno::ListObject>();
        for (auto& prim : prims) {
            auto spGeom = create_GeometryObject(prim.get());
            geo_list->push_back(std::move(spGeom));
        }
        set_output("Geometry List", std::move(geo_list));
        return ZErr_OK;
#endif
    }
};

ZENDEFNODE_ABI(NewFBXGeometryList,
    Z_INPUTS(
        { "fbx_object", _gParamType_FBXObject },
        { "scene_info", _gParamType_JsonObject },
        { "vectors", _gParamType_String, ZString("nrm,tang") },
        { "CopyVectorsFromLoopsToVert", _gParamType_Bool, ZInt(1) },
        { "CopyFacesetToMatid", _gParamType_Bool, ZInt(1) },
        { "OutputTexEvenMissing", _gParamType_Bool, ZInt(0) },
        { "SkipInvisibleMesh", _gParamType_Bool, ZInt(0) }
    ),
    Z_OUTPUTS(
        { "Geometry List", _gParamType_List }
    ),
    "FBXSDK",
    "",
    "",
    ""
);


static std::vector<glm::mat4> getBoneMatrix(PrimitiveObject *prim) {
        std::vector<glm::mat4> matrixs;
        auto &verts = prim->verts;
        auto &transform_r0 = prim->verts.add_attr<Vec3f>("transform_r0");
        auto &transform_r1 = prim->verts.add_attr<Vec3f>("transform_r1");
        auto &transform_r2 = prim->verts.add_attr<Vec3f>("transform_r2");
        for (auto i = 0; i < prim->verts.size(); i++) {
            glm::mat4 matrix;
            matrix[0] = {transform_r0[i][0], transform_r0[i][1], transform_r0[i][2], 0};
            matrix[1] = {transform_r1[i][0], transform_r1[i][1], transform_r1[i][2], 0};
            matrix[2] = {transform_r2[i][0], transform_r2[i][1], transform_r2[i][2], 0};
            matrix[3] = {verts[i][0], verts[i][1], verts[i][2], 1};
            matrixs.push_back(matrix);
        }
        return matrixs;
}
static std::vector<glm::mat4> getInvertedBoneMatrix(PrimitiveObject *prim) {
        std::vector<glm::mat4> inv_matrixs;
        auto matrixs = getBoneMatrix(prim);
        for (auto i = 0; i < matrixs.size(); i++) {
            auto m = matrixs[i];
            auto inv_m = glm::inverse(m);
            inv_matrixs.push_back(inv_m);
        }
        return inv_matrixs;
}
static Vec3f transform_pos(glm::mat4 &transform, Vec3f pos) {
        auto p = transform * glm::vec4(pos[0], pos[1], pos[2], 1);
        return {p.x, p.y, p.z};
}
static Vec3f transform_nrm(glm::mat4 &transform, Vec3f pos) {
        auto p = glm::transpose(glm::inverse(transform)) * glm::vec4(pos[0], pos[1], pos[2], 0);
        return {p.x, p.y, p.z};
}

static std::map<std::string, int> getBoneNameMapping(PrimitiveObject *prim) {
    auto boneName_count = prim->userData()->get_int("boneName_count");
    std::map<std::string, int> boneNames;
    for (auto i = 0; i < boneName_count; i++) {
        auto boneName = zsString2Std(prim->userData()->get_string(stdString2zs(format("boneName_{}", i))));
        boneNames[boneName] = i;
    }
    return boneNames;
}

static std::vector<std::string> getBoneNames(PrimitiveObject *prim) {
    auto boneName_count = prim->userData()->get_int("boneName_count");
    std::vector<std::string> boneNames;
    boneNames.reserve(boneName_count);
    for (auto i = 0; i < boneName_count; i++) {
        auto boneName = zsString2Std(prim->userData()->get_string(stdString2zs(format("boneName_{}", i))));
        boneNames.emplace_back(boneName);
    }
    return boneNames;
}

static std::vector<int> TopologicalSorting(std::map<int, int> bone_connects, zeno::PrimitiveObject* skeleton) {
    std::vector<int> ordering;
    std::set<int> ordering_set;
    while (bone_connects.size()) {
        std::set<int> need_to_remove;
        for (auto [s, p]: bone_connects) {
            if (bone_connects.count(p) == 0) {
                if (ordering_set.count(p) == 0) {
                    ordering.emplace_back(p);
                    ordering_set.insert(p);
                }
                need_to_remove.insert(s);
            }
        }
        for (auto index: need_to_remove) {
            bone_connects.erase(index);
        }
    }
    for (auto i = 0; i < skeleton->verts.size(); i++) {
        if (ordering_set.count(i) == 0) {
            ordering.push_back(i);
        }
    }
    if (false) { // debug
        for (auto i = 0; i < ordering.size(); i++) {
            auto bi = ordering[i];
            auto bone_name = zsString2Std(skeleton->userData()->get_string(stdString2zs(format("boneName_{}", bi))));
            zeno::log_info("{}: {}: {}", i, bi, bone_name);
        }
    }
    return ordering;
}

}