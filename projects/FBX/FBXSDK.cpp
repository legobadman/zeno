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
#include <cmath>
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
#include "FBXUserData.h"
#include <iobject2.h>

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

static std::string get_ud_string(IUserData2* pUserData, const std::string& param) {
    if (!pUserData) return "";
    char buf[256];
    pUserData->get_string(param.c_str(), "", buf, sizeof(buf));
    return std::string(buf);
}

static void TraverseNodesToGetJson(FbxNode* pNode, Json& json, FbxTime curTime) {
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
            json["r0"] = { 0.0, 0.0, 0.0 };
            json["r1"] = { 0.0, 0.0, 0.0 };
            json["r2"] = { 0.0, 0.0, 0.0 };
            json["t"] = { 0.0, 0.0, 0.0 };
        }
        else {
            json["r0"] = { r0[0], r0[1], r0[2] };
            json["r1"] = { r1[0], r1[1], r1[2] };
            json["r2"] = { r2[0], r2[1], r2[2] };
            json["t"] = { t[0], t[1], t[2] };
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

template<typename T>
static void getAttrForGeom(T* arr, int nPoints, int nVerts,
    std::vector<Vec3f>* outPoint, std::vector<Vec3f>* outVertex)
{
    if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint) {
        if (!outPoint) return;
        outPoint->resize(nPoints);
        for (int i = 0; i < nPoints; i++) {
            int pIndex = i;
            if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
                pIndex = arr->GetIndexArray().GetAt(i);
            auto v = arr->GetDirectArray().GetAt(pIndex);
            (*outPoint)[i] = Vec3f((float)v[0], (float)v[1], (float)v[2]);
        }
    }
    else if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex) {
        if (!outVertex) return;
        outVertex->resize(nVerts);
        for (size_t i = 0; i < (size_t)nVerts; i++) {
            int pIndex = (int)i;
            if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
                pIndex = arr->GetIndexArray().GetAt((int)i);
            auto v = arr->GetDirectArray().GetAt(pIndex);
            (*outVertex)[i] = Vec3f((float)v[0], (float)v[1], (float)v[2]);
        }
    }
}

// IGeometryObject version of prim_copy_faceset_to_matid.
// Copies:
//   - userData: faceset_* -> Material_*, faceset_count -> matNum
//   - face attr: int faceset[] -> int matid[] (if present)
static void geom_copy_faceset_to_matid(IGeometryObject* geom) {
    if (!geom) {
        return;
    }
    auto* ud = geom->userData();
    if (!ud) {
        return;
    }

    int faceset_count = ud->get_int("faceset_count", 0);
    ud->set_int("matNum", faceset_count);

    for (int i = 0; i < faceset_count; ++i) {
        std::string faceset_key = zeno::format("faceset_{}", i);
        std::string material_key = zeno::format("Material_{}", i);
        std::string value = get_ud_string(ud, faceset_key);
        ud->set_string(material_key.c_str(), value.c_str());
    }

    // Copy per-face faceset -> matid if we have an int face attribute "faceset".
    const int nfaces = geom->nfaces();
    if (nfaces > 0 && geom->has_attr(ATTR_FACE, "faceset", ATTR_INT)) {
        std::vector<int> faceset(nfaces);
        size_t got = geom->get_int_attr(ATTR_FACE, "faceset", faceset.data(), faceset.size());
        if (got == static_cast<size_t>(nfaces)) {
            geom->create_attr_by_int(ATTR_FACE, "matid", faceset.data(), faceset.size());
        }
    }
}

static std::unique_ptr<IGeometryObject> GetMeshGeometry(
    FbxNode* pNode,
    bool output_tex_even_missing,
    std::string fbx_path,
    bool apply_transform
) {
    FbxMesh* pMesh = pNode->GetMesh();
    if (!pMesh) return nullptr;
    const char* mesh_name = pMesh->GetName();
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") nodeName = "ABC";

    FbxAMatrix bindMatrix = pNode->EvaluateGlobalTransform();
    FbxAMatrix Geometry;
    {
        FbxVector4 Translation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
        FbxVector4 Rotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
        FbxVector4 Scaling = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
        Geometry.SetT(Translation); Geometry.SetR(Rotation); Geometry.SetS(Scaling);
        FbxAMatrix PivotGeometry;
        FbxVector4 RotationPivot = pNode->GetRotationPivot(FbxNode::eSourcePivot);
        FbxVector4 FullPivot(-RotationPivot[0], -RotationPivot[1], -RotationPivot[2]);
        PivotGeometry.SetT(FullPivot);
        Geometry = Geometry * PivotGeometry;
    }

    int numVertices = pMesh->GetControlPointsCount();
    FbxVector4* vertices = pMesh->GetControlPoints();
    std::vector<Vec3f> points;
    points.reserve(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        FbxVector4 pos(vertices[i][0], vertices[i][1], vertices[i][2], 1.0);
        pos = Geometry.MultT(pos);
        if (apply_transform) pos = bindMatrix.MultT(pos);
        points.push_back(Vec3f((float)pos[0], (float)pos[1], (float)pos[2]));
    }

    int numPolygons = pMesh->GetPolygonCount();
    std::vector<std::vector<int>> faces;
    std::vector<int> loops;
    faces.reserve(numPolygons);
    loops.reserve(numPolygons * 4);
    for (int i = 0; i < numPolygons; ++i) {
        int nv = pMesh->GetPolygonSize(i);
        std::vector<int> face;
        face.reserve(nv);
        for (int j = 0; j < nv; ++j) {
            int vi = pMesh->GetPolygonVertex(i, j);
            face.push_back(vi);
            loops.push_back(vi);
        }
        faces.push_back(std::move(face));
    }
    int nVerts = (int)loops.size();

    IGeometryObject* geom = zeno::createGeometryByPointFace(
        zeno::Topo_IndiceMesh, false, points, faces);
    if (!geom) return nullptr;

    std::unique_ptr<IGeometryObject> ugeom(geom);
    IUserData2* ud = geom->userData();
    ud->set_string("RootName", nodeName.c_str());
    ud->set_string("_abc_name", nodeName.c_str());
    ud->set_string("fbx_path", fbx_path.c_str());

    // Skin: userData + point float attrs only (no create_attr_by_int in ABI for boneName_i)
    if (pMesh->GetDeformerCount(FbxDeformer::eSkin)) {
        FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
        std::vector<std::string> bone_names;
        std::vector<std::vector<std::pair<int, float>>> bone_weight(numVertices);
        for (int j = 0; j < pSkin->GetClusterCount(); ++j) {
            FbxCluster* pCluster = pSkin->GetCluster(j);
            FbxNode* pBoneNode = pCluster->GetLink();
            if (!pBoneNode) continue;
            int numIndices = pCluster->GetControlPointIndicesCount();
            int* indices = pCluster->GetControlPointIndices();
            double* weights = pCluster->GetControlPointWeights();
            bone_names.push_back(pBoneNode->GetName());
            for (int k = 0; k < numIndices; ++k)
                bone_weight[indices[k]].emplace_back(j, (float)weights[k]);
        }
        int maxnum_boneWeight = 0;
        for (int i = 0; i < numVertices; i++) {
            int s = (int)bone_weight[i].size();
            if (s > maxnum_boneWeight) maxnum_boneWeight = s;
        }
        ud->set_int("maxnum_boneWeight", maxnum_boneWeight);
        ud->set_int("boneName_count", (int)bone_names.size());
        for (size_t i = 0; i < bone_names.size(); i++)
            ud->set_string(zeno::format("boneName_{}", (int)i).c_str(), bone_names[i].c_str());
        /* IGeometryObject has no create_attr_by_int: point attrs boneName_0, boneName_1, ... are not set */
        for (int slot = 0; slot < maxnum_boneWeight; slot++) {
            std::vector<float> w(numVertices, -1.0f);
            for (int i = 0; i < numVertices; i++) {
                if (slot < (int)bone_weight[i].size())
                    w[i] = bone_weight[i][slot].second;
            }
            std::string name = zeno::format("boneWeight_{}", slot);
            geom->create_attr_by_float(zeno::ATTR_POINT, name.c_str(), w.data(), w.size());
        }
    }

    // UV
    if (pMesh->GetElementUVCount() > 0) {
        FbxLayerElementUV* arr = pMesh->GetElementUV(0);
        if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByControlPoint) {
            std::vector<Vec3f> uv_pt(numVertices);
            for (int i = 0; i < numVertices; i++) {
                int pIndex = i;
                if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
                    pIndex = arr->GetIndexArray().GetAt(i);
                auto v = arr->GetDirectArray().GetAt(pIndex);
                uv_pt[i] = Vec3f((float)v[0], (float)v[1], 0.f);
            }
            geom->create_attr_by_vec3(zeno::ATTR_POINT, "uv", uv_pt.data(), uv_pt.size());
        }
        else if (arr->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygonVertex) {
            std::vector<Vec3f> uv_vert(nVerts);
            int uvCount = arr->GetDirectArray().GetCount();
            for (int i = 0; i < nVerts; i++) {
                int idx = i;
                if (arr->GetReferenceMode() == FbxLayerElement::EReferenceMode::eIndexToDirect)
                    idx = arr->GetIndexArray().GetAt(i);
                if (idx < uvCount) {
                    auto v = arr->GetDirectArray().GetAt(idx);
                    uv_vert[i] = Vec3f((float)v[0], (float)v[1], 0.f);
                }
            }
            geom->create_attr_by_vec3(zeno::ATTR_VERTEX, "uv", uv_vert.data(), uv_vert.size());
        }
    }

    std::vector<Vec3f> ptAttr, vertAttr;
    if (pMesh->GetElementVertexColorCount() > 0)
        getAttrForGeom(pMesh->GetElementVertexColor(0), numVertices, nVerts, &ptAttr, &vertAttr);
    if (!ptAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_POINT, "clr", ptAttr.data(), ptAttr.size());
    if (!vertAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_VERTEX, "clr", vertAttr.data(), vertAttr.size());

    ptAttr.clear(); vertAttr.clear();
    if (pMesh->GetElementNormalCount() > 0)
        getAttrForGeom(pMesh->GetElementNormal(0), numVertices, nVerts, &ptAttr, &vertAttr);
    if (!ptAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_POINT, "nrm", ptAttr.data(), ptAttr.size());
    if (!vertAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_VERTEX, "nrm", vertAttr.data(), vertAttr.size());

    ptAttr.clear(); vertAttr.clear();
    if (pMesh->GetElementTangentCount() > 0)
        getAttrForGeom(pMesh->GetElementTangent(0), numVertices, nVerts, &ptAttr, &vertAttr);
    if (!ptAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_POINT, "tang", ptAttr.data(), ptAttr.size());
    if (!vertAttr.empty())
        geom->create_attr_by_vec3(zeno::ATTR_VERTEX, "tang", vertAttr.data(), vertAttr.size());

    /* IGeometryObject has no create_attr_by_int: face attr "faceset" is not set */
    int mat_count = 0;
    if (pMesh->GetElementMaterialCount() > 0) {
        mat_count = pNode->GetMaterialCount();
        for (int i = 0; i < mat_count; i++) {
            FbxSurfaceMaterial* material = pNode->GetMaterial(i);
            ud->set_string(zeno::format("faceset_{}", i).c_str(), material->GetName());
        }
    }
    ud->set_int("faceset_count", mat_count);

    std::string abcpath = fbx_path + '/' + mesh_name;
    ud->set_int("abcpath_count", 1);
    ud->set_string("abcpath_0", abcpath.c_str());
    /* Per-face int attr "abcpath" not set: IGeometryObject ABI has no create_attr_by_int for face. */

    if (mat_count > 0) {
        for (int i = 0; i < mat_count; i++) {
            FbxSurfaceMaterial* material = pNode->GetMaterial(i);
            std::string mat_name = material->GetName();
            ud->set_string(zeno::format("faceset_{}", i).c_str(), mat_name.c_str());
            Json json;
            {
                FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sEmissive);
                if (output_tex_even_missing) json["emissive_tex"] = "";
                if (property.IsValid()) {
                    FbxDouble3 value = property.Get<FbxDouble3>();
                    json["emissive_value"] = { value[0], value[1], value[2] };
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["emissive_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sAmbient);
                if (output_tex_even_missing) json["ambient_tex"] = "";
                if (property.IsValid()) {
                    FbxDouble3 value = property.Get<FbxDouble3>();
                    json["ambient_value"] = { value[0], value[1], value[2] };
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["ambient_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                if (output_tex_even_missing) json["diffuse_tex"] = "";
                if (property.IsValid()) {
                    FbxDouble3 value = property.Get<FbxDouble3>();
                    json["diffuse_value"] = { value[0], value[1], value[2] };
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["diffuse_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sSpecular);
                if (output_tex_even_missing) json["specular_tex"] = "";
                if (property.IsValid()) {
                    FbxDouble3 value = property.Get<FbxDouble3>();
                    json["specular_value"] = { value[0], value[1], value[2] };
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["specular_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sShininess);
                if (output_tex_even_missing) json["shininess_tex"] = "";
                if (property.IsValid()) {
                    double value = property.Get<double>();
                    json["shininess_value"] = value;
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["shininess_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sBump);
                if (output_tex_even_missing) json["bump_tex"] = "";
                if (property.IsValid()) {
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["bump_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
                if (output_tex_even_missing) json["normal_map_tex"] = "";
                if (property.IsValid()) {
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["normal_map_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sTransparentColor);
                if (output_tex_even_missing) json["transparent_color_tex"] = "";
                if (property.IsValid()) {
                    FbxDouble3 value = property.Get<FbxDouble3>();
                    json["transparent_color_value"] = { value[0], value[1], value[2] };
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["transparent_color_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
                if (output_tex_even_missing) json["opacity_tex"] = "";
                if (property.IsValid()) {
                    double value = property.Get<double>();
                    json["opacity_value"] = value;
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["opacity_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sReflection);
                if (output_tex_even_missing) json["reflection_tex"] = "";
                if (property.IsValid()) {
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["reflection_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sDisplacementColor);
                if (output_tex_even_missing) json["displacement_color_tex"] = "";
                if (property.IsValid()) {
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["displacement_color_tex"] = texture->GetFileName();
                    }
                }
                property = material->FindProperty(FbxSurfaceMaterial::sVectorDisplacementColor);
                if (output_tex_even_missing) json["vector_displacement_color_tex"] = "";
                if (property.IsValid()) {
                    for (int ti = 0; ti < property.GetSrcObjectCount<FbxTexture>(); ++ti) {
                        FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject<FbxTexture>(ti));
                        if (texture) json["vector_displacement_color_tex"] = texture->GetFileName();
                    }
                }
            }
            ud->set_string(mat_name.c_str(), json.dump().c_str());
        }
    }
    return ugeom;
}

static void TraverseNodesToGetGeoms(
    FbxNode* pNode,
    std::vector<std::unique_ptr<IGeometryObject>>& geoms,
    bool output_tex_even_missing,
    std::string fbx_path,
    bool apply_transform
) {
    if (!pNode) return;
    std::string nodeName = pNode->GetName();
    if (nodeName == "RootNode") nodeName = "ABC";
    fbx_path = fbx_path + '/' + nodeName;

    if (FbxMesh* mesh = pNode->GetMesh()) {
        auto sub = GetMeshGeometry(pNode, output_tex_even_missing, fbx_path, apply_transform);
        if (sub) {
            geoms.push_back(std::move(sub));
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++) {
        TraverseNodesToGetGeoms(pNode->GetChild(i), geoms, output_tex_even_missing, fbx_path, apply_transform);
    }
}

// Simple wrapper around FBX SDK manager + scene, shared via shared_ptr so
// multiple node instances can reference the same loaded file.
struct FBXObject : IObject2 {

    struct Inner {
        FbxManager* lSdkManager = nullptr;
        FbxScene*   lScene = nullptr;
        //~Inner() {
        //    if (lScene) {
        //        lScene->Destroy();
        //        lScene = nullptr;
        //    }
        //    if (lSdkManager) {
        //        lSdkManager->Destroy();
        //        lSdkManager = nullptr;
        //    }
        //}
    };
    Inner inner;
    FBXUserData m_userData;
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
        return &m_userData;
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
    FBXObject::Inner m_cachedInner;

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
        if (path == m_usedPath) {
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

        FBXObject::Inner inner;
        inner.lSdkManager = manager;
        inner.lScene = scene;
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
        std::vector<std::unique_ptr<IGeometryObject>> prims;
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
                std::string errStr(lImporter->GetStatus().GetErrorString());
                ptrNodeData->report_error("Call to FbxImporter::Initialize() failed.\n");
                //printf("Call to FbxImporter::Initialize() failed.\n");
                //printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
                //exit(-1);
                return ZErr_ParamError;
            }
            int major, minor, revision;
            lImporter->GetFileVersion(major, minor, revision);
            auto fbx_object = std::make_unique<FBXObject>();

            fbx_object->inner.lSdkManager = lSdkManager;
            // Create a new scene so that it can be populated by the imported file.
            fbx_object->inner.lScene = FbxScene::Create(lSdkManager, "myScene");

            // Import the contents of the file into the scene.
            lImporter->Import(fbx_object->inner.lScene);
            FbxRootNodeUtility::RemoveAllFbxRoots(fbx_object->inner.lScene);

            // The file is imported; so get rid of the importer.
            lImporter->Destroy();
            fbx_object->userData()->set_vec3i("version", zeno::Vec3i(major, minor, revision));
            fbx_object->userData()->set_string("file_path", lFilename.c_str());

            auto lScene = fbx_object->inner.lScene;
            // Print the nodes of the scene and their attributes recursively.
            // Note that we are not printing the root node because it should
            // not contain any attributes.
            FbxNode* lRootNode = lScene->GetRootNode();
            bool output_tex_even_missing = ptrNodeData->get_input2_bool("OutputTexEvenMissing");

            if (lRootNode) {
                TraverseNodesToGetGeoms(lRootNode, prims, output_tex_even_missing, "", false);
            }

            auto vectors_str = get_input2_string(ptrNodeData, "vectors");
            std::vector<std::string> vectors = zeno::split_str(vectors_str, ',');

            for (auto& prim : prims) {
                if (ptrNodeData->get_input2_bool("CopyVectorsFromLoopsToVert")) {
                    IGeometryObject* geom = prim.get();
                    if (geom) {
                        const int npoints = geom->npoints();
                        const int nverts = geom->nvertices();
                        if (npoints > 0 && nverts > 0) {
                            for (auto vector : vectors) {
                                vector = zeno::trim_string(vector);
                                if (vector.empty())
                                    continue;
                                if (!geom->has_attr(ATTR_VERTEX, vector.c_str(), ATTR_VEC3))
                                    continue;

                                std::vector<Vec3f> loopAttr(nverts);
                                size_t got = geom->get_vec3f_attr(ATTR_VERTEX, vector.c_str(), loopAttr.data(), loopAttr.size());
                                if (got != static_cast<size_t>(nverts))
                                    continue;

                                std::vector<Vec3f> pointAttr(npoints, Vec3f(0.0f, 0.0f, 0.0f));
                                for (int v = 0; v < nverts; ++v) {
                                    int p = geom->vertex_point(v);
                                    if (p < 0 || p >= npoints)
                                        continue;
                                    pointAttr[p].x += loopAttr[v].x;
                                    pointAttr[p].y += loopAttr[v].y;
                                    pointAttr[p].z += loopAttr[v].z;
                                }

                                for (int p = 0; p < npoints; ++p) {
                                    auto& val = pointAttr[p];
                                    float len = std::sqrt(val.x * val.x + val.y * val.y + val.z * val.z);
                                    if (len > 1e-8f) {
                                        float inv = 1.0f / len;
                                        val.x *= inv;
                                        val.y *= inv;
                                        val.z *= inv;
                                    }
                                }

                                geom->create_attr_by_vec3(ATTR_POINT, vector.c_str(), pointAttr.data(), pointAttr.size());
                            }
                        }
                    }
                }
                if (ptrNodeData->get_input2_bool("CopyFacesetToMatid")) {
                    geom_copy_faceset_to_matid(prim.get());
                }
            }

            for (int frameid = start_frame; frameid <= end_frame; frameid++) {
                float fps = ptrNodeData->get_input2_float("fps");
                float t = float(frameid) / fps;
                FbxTime curTime;       // The time for each key in the animation curve(s)
                curTime.SetSecondDouble(t);   // Starting time

                auto lScene = fbx_object->inner.lScene;
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
        for (auto& spGeom : prims) {
            auto abc_path = get_ud_string(spGeom->userData(), "abcpath_0");
            abc_paths.push_back(abc_path);
            geo_list->push_back(spGeom.release());
        }

        // Convert std::vector<std::string> to const char** for ABI.
        std::vector<const char*> scene_cstrs;
        scene_cstrs.reserve(scene_info_list.size());
        for (auto& s : scene_info_list) {
            scene_cstrs.push_back(s.c_str());
        }
        ptrNodeData->set_output_string_list(
            "Scene Json List",
            scene_cstrs.empty() ? nullptr : scene_cstrs.data(),
            scene_cstrs.size());
        ptrNodeData->set_output_object("Geometry List", geo_list);
        return ZErr_OK;
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

        const int start_frame = nd->get_input2_int("Start Frame");
        const int idx = frameid - start_frame;
        const int count = static_cast<int>(nd->get_input_string_list_count("Json List"));
        if (idx < 0 || idx >= count) {
            nd->report_error("NewFBXSceneInfo: frame index out of range");
            return ZErr_ParamError;
        }

        // Fetch JSON string from string list.
        char buf[327680] = {};
        size_t written = nd->get_input_string_list("Json List", static_cast<size_t>(idx), buf, sizeof(buf));
        if (written == 0) {
            nd->report_error("NewFBXSceneInfo: selected JSON string is empty");
            return ZErr_ParamError;
        }
        if (written >= sizeof(buf)) {
            nd->report_error("NewFBXSceneInfo: JSON string too long for buffer");
            return ZErr_ParamError;
        }

        std::string json_str(buf, written);

        // Optionally validate/normalize via nlohmann::json.
        try {
            Json j = Json::parse(json_str);
            json_str = j.dump();
        } catch (...) {
            nd->report_error("NewFBXSceneInfo: invalid JSON string in Json List");
            return ZErr_ParamError;
        }

        nd->set_output_string("json", json_str.c_str());
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

}