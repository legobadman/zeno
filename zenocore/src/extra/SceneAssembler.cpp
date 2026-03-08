//
// Migrated from zeno/src/nodes/SceneAssembler.cpp
//
#include <zeno/extra/GlobalComm.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/log.h>
#include <zeno/utils/string.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/extra/SceneAssembler.h>
#include <zeno/geo/geometryutil.h>
#include <zeno/utils/eulerangle.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <tinygltf/json.hpp>
#include <zeno/types/JsonObject.h>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <zeno/funcs/PrimitiveTools.h>
#include <zeno/utils/scope_exit.h>
#include <deque>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

using Json = nlohmann::json;

namespace zeno {

namespace {

static std::string get_input2_string_helper(INodeData* nd, const char* name) {
    size_t sz = nd->get_input_string_size(name);
    if (sz == 0) return "";
    constexpr size_t STACK_THRESHOLD = 8192;
    if (sz < STACK_THRESHOLD) {
        char buf[STACK_THRESHOLD];
        nd->get_input2_string(name, buf, sz + 1);
        return std::string(buf);
    }
    std::vector<char> heap_buf(sz + 1);
    nd->get_input2_string(name, heap_buf.data(), heap_buf.size());
    return std::string(heap_buf.data());
}

static void get_local_matrix_map(Json& json, std::string parent_path, SceneObject* scene) {
    SceneTreeNode stn;
    std::string node_name = json["node_name"];
    std::string node_path = parent_path + '/' + node_name;
    Json r0 = json["r0"];
    Json r1 = json["r1"];
    Json r2 = json["r2"];
    Json t = json["t"];
    glm::mat4 mat;
    mat[0] = {float(r0[0]), float(r0[1]), float(r0[2]), 0.0f};
    mat[1] = {float(r1[0]), float(r1[1]), float(r1[2]), 0.0f};
    mat[2] = {float(r2[0]), float(r2[1]), float(r2[2]), 0.0f};
    mat[3] = {float(t[0]), float(t[1]), float(t[2]), 1.0f};
    scene->node_to_matrix[node_path + "_m"].push_back(mat);
    stn.matrix = node_path + "_m";
    stn.visibility = json["visibility"] == 0 ? 0 : 1;
    if (json.contains("mesh")) {
        stn.meshes.push_back(node_path + "/" + std::string(json["mesh"]));
    }
    for (auto i = 0; i < json["children_name"].size(); i++) {
        std::string child_name = json["children_name"][i];
        std::string child_path = node_path + '/' + child_name;
        if (json.contains(child_name)) {
            if (json[child_name].contains("instance_source_path")) {
                stn.children.push_back(json[child_name]["instance_source_path"]);
            } else {
                stn.children.push_back(child_path);
            }
        }
    }
    scene->scene_tree[node_path] = stn;
    for (auto i = 0; i < json["children_name"].size(); i++) {
        std::string child_name = json["children_name"][i];
        if (json.contains(child_name)) {
            if (!json[child_name].contains("instance_source_path")) {
                get_local_matrix_map(json[child_name], node_path, scene);
            }
        }
    }
}

static std::vector<glm::mat4> get_xform_from_prim(PrimitiveObject* prim) {
    std::vector<glm::mat4> mats;
    for (auto i = 0; i < prim->verts.size(); i++) {
        auto pos = prim->verts[i];
        auto r0 = prim->verts.add_attr<vec3f>("r0")[i];
        auto r1 = prim->verts.add_attr<vec3f>("r1")[i];
        auto r2 = prim->verts.add_attr<vec3f>("r2")[i];
        glm::mat4 mat;
        mat[0] = {r0[0], r0[1], r0[2], 0};
        mat[1] = {r1[0], r1[1], r1[2], 0};
        mat[2] = {r2[0], r2[1], r2[2], 0};
        mat[3] = {pos[0], pos[1], pos[2], 1};
        mats.push_back(mat);
    }
    return mats;
}

static void scene_add_prefix_node(std::string prefix_node_name, std::vector<glm::mat4> xform,
                                  SceneObject* sceneObject) {
    std::unordered_map<std::string, SceneTreeNode> scene_tree;
    for (const auto& [key, value] : sceneObject->scene_tree) {
        SceneTreeNode stn;
        stn.visibility = value.visibility;
        if (value.matrix.size()) {
            stn.matrix = prefix_node_name + value.matrix;
        }
        for (auto& child : value.children) {
            stn.children.push_back(prefix_node_name + child);
        }
        for (auto& mesh : value.meshes) {
            stn.meshes.push_back(prefix_node_name + mesh);
        }
        scene_tree[prefix_node_name + key] = stn;
    }
    std::unordered_map<std::string, std::unique_ptr<GeometryObject>> new_prim_list;
    for (auto& [key, value] : sceneObject->geom_list) {
        char objname_buf[512];
        value->userData()->get_string("ObjectName", "", objname_buf, sizeof(objname_buf));
        auto obj_name = std::string(objname_buf);
        obj_name = prefix_node_name + obj_name;
        value->userData()->set_string("ObjectName", stdString2zs(obj_name));
        IObject2* cloned = value->clone();
        new_prim_list[prefix_node_name + key] =
            std::unique_ptr<GeometryObject>(static_cast<GeometryObject*>(cloned));
    }
    std::unordered_map<std::string, std::vector<glm::mat4>> new_node_to_matrix;
    for (const auto& [key, value] : sceneObject->node_to_matrix) {
        new_node_to_matrix[prefix_node_name + key] = value;
    }
    std::unordered_map<std::string, std::vector<int>> new_node_to_id;
    for (const auto& [key, value] : sceneObject->node_to_id) {
        new_node_to_id[prefix_node_name + key] = value;
    }
    sceneObject->scene_tree = scene_tree;
    sceneObject->geom_list = std::move(new_prim_list);
    sceneObject->node_to_matrix = new_node_to_matrix;
    sceneObject->node_to_id = std::move(new_node_to_id);
    {
        std::string xform_name = prefix_node_name + "_m";
        if (xform.empty()) {
            xform.push_back(glm::mat4(1));
        }
        sceneObject->node_to_matrix[xform_name] = xform;
        SceneTreeNode stn;
        stn.matrix = xform_name;
        stn.children.push_back(prefix_node_name + sceneObject->root_name);
        sceneObject->scene_tree[prefix_node_name] = stn;
    }
    sceneObject->root_name = prefix_node_name;
}

}  // namespace

void merge_scene2_into_scene1(SceneObject* main_object, SceneObject* second_object, std::string insert_path) {
    for (const auto& [key, value] : second_object->scene_tree) {
        SceneTreeNode stn;
        stn.visibility = value.visibility;
        if (value.matrix.size()) {
            stn.matrix = insert_path + value.matrix;
        }
        for (auto& child : value.children) {
            stn.children.push_back(insert_path + child);
        }
        for (auto& mesh : value.meshes) {
            stn.meshes.push_back(insert_path + mesh);
        }
        main_object->scene_tree[insert_path + key] = stn;
    }
    for (auto& [key, value] : second_object->geom_list) {
        char objname_buf[512];
        value->userData()->get_string("ObjectName", "", objname_buf, sizeof(objname_buf));
        auto obj_name = std::string(objname_buf);
        obj_name = insert_path + obj_name;
        value->userData()->set_string("ObjectName", stdString2zs(obj_name));
        IObject2* cloned = value->clone();
        main_object->geom_list[insert_path + key] =
            std::unique_ptr<GeometryObject>(static_cast<GeometryObject*>(cloned));
    }
    for (const auto& [key, value] : second_object->node_to_matrix) {
        main_object->node_to_matrix[insert_path + key] = value;
    }
    for (const auto& [key, value] : second_object->node_to_id) {
        main_object->node_to_id[insert_path + key] = value;
    }
    if (!insert_path.empty()) {
        main_object->scene_tree[insert_path].children.push_back(insert_path + second_object->root_name);
    }
}

// SceneObject implementation
ZObjectType SceneObject::type() const { return (ZObjectType)gParamType_Scene; }
IObject2* SceneObject::clone() const { return new SceneObject(*this); }
size_t SceneObject::key(char* buf, size_t buf_size) const {
    size_t len = m_key.size();
    if (buf && buf_size > 0) {
        size_t copy = (len < buf_size - 1) ? len : (buf_size - 1);
        memcpy(buf, m_key.c_str(), copy);
        buf[copy] = '\0';
    }
    return len;
}
void SceneObject::update_key(const char* key) { m_key = key ? key : ""; }
size_t SceneObject::serialize_json(char* buf, size_t buf_size) const { return 0; }
IUserData2* SceneObject::userData() { return &m_userDat; }
void SceneObject::Delete() { delete this; }

void SceneObject::from_json(const std::string& json_str) {
    auto j = Json::parse(json_str);
    if (j.contains("root_name"))
        root_name = j["root_name"];
    if (j.contains("scene_tree")) {
        scene_tree.clear();
        for (auto& [k, v] : j["scene_tree"].items()) {
            SceneTreeNode stn;
            if (v.contains("matrix"))
                stn.matrix = v["matrix"];
            if (v.contains("visibility"))
                stn.visibility = v["visibility"];
            if (v.contains("children"))
                for (auto& c : v["children"])
                    stn.children.push_back(c);
            if (v.contains("meshes"))
                for (auto& m : v["meshes"])
                    stn.meshes.push_back(m);
            scene_tree[k] = stn;
        }
    }
    if (j.contains("node_to_matrix")) {
        node_to_matrix.clear();
        for (auto& [k, v] : j["node_to_matrix"].items()) {
            std::vector<glm::mat4> mats;
            for (auto& arr : v) {
                glm::mat4 m;
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 3; j++)
                        m[i][j] = float(arr[i * 3 + j]);
                m[0][3] = m[1][3] = m[2][3] = 0;
                m[3][3] = 1;
                mats.push_back(m);
            }
            node_to_matrix[k] = mats;
        }
    }
    if (j.contains("node_to_id")) {
        node_to_id.clear();
        for (auto& [k, v] : j["node_to_id"].items()) {
            std::vector<int> ids;
            for (auto& id : v)
                ids.push_back(int(id));
            node_to_id[k] = ids;
        }
    }
}

std::string SceneObject::to_json() const {
    Json j;
    j["root_name"] = root_name;
    j["type"] = scene_type;
    for (auto& [k, v] : scene_tree) {
        j["scene_tree"][k]["matrix"] = v.matrix;
        j["scene_tree"][k]["visibility"] = v.visibility;
        j["scene_tree"][k]["children"] = v.children;
        j["scene_tree"][k]["meshes"] = v.meshes;
    }
    for (auto& [k, v] : node_to_matrix) {
        for (auto& m : v) {
            Json arr = Json::array();
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 3; j++)
                    arr.push_back(m[i][j]);
            j["node_to_matrix"][k].push_back(arr);
        }
    }
    for (auto& [k, v] : node_to_id) {
        j["node_to_id"][k] = v;
    }
    return j.dump();
}

std::unique_ptr<ListObject> SceneObject::to_structure() const {
    auto scene = std::make_unique<ListObject>();
    for (auto& [path, geom] : geom_list) {
        auto prim = geom->toPrimitive();
        prim->userData()->set_string("ObjectName", stdString2zs(path));
        scene->push_back(prim.release());
    }
    return scene;
}

void SceneObject::flatten() {
    // Flatten scene tree - merge matrices along hierarchy
    (void)this;
}

std::unique_ptr<SceneObject> SceneObject::root_rename(const std::string& new_root_name,
                                                       const std::vector<glm::mat4>& root_xform) {
    auto new_scene = std::make_unique<SceneObject>(*this);
    new_scene->root_name = new_root_name;
    if (!root_xform.empty()) {
        new_scene->node_to_matrix[new_root_name + "_m"] = root_xform;
    }
    return new_scene;
}

SceneObject::SceneObject() : m_userDat() {}
SceneObject::SceneObject(const SceneObject& rhs)
    : root_name(rhs.root_name),
      bNeedUpdateDescriptor(rhs.bNeedUpdateDescriptor),
      bResetOptixScene(rhs.bResetOptixScene),
      scene_type(rhs.scene_type),
      scene_tree(rhs.scene_tree),
      node_to_matrix(rhs.node_to_matrix),
      node_to_id(rhs.node_to_id),
      m_key(rhs.m_key),
      m_userDat(rhs.m_userDat) {
    for (auto& [k, v] : rhs.geom_list) {
        IObject2* cloned = v->clone();
        geom_list[k] = std::unique_ptr<GeometryObject>(static_cast<GeometryObject*>(cloned));
    }
}

#define DEF_OVERRIDE_FOR_INODE \
    NodeType type() const override { return Node_Normal; } \
    void clearCalcResults() override {} \
    float time() const override { return 1.0f; }

struct FormSceneTree : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        auto sceneTree = std::make_unique<SceneObject>();

        // Scene Info is now a JSON string; parse it into Json.
        std::string scene_info_str = get_input2_string_helper(nd, "Scene Info");
        if (scene_info_str.empty()) {
            nd->report_error("FormSceneTree: Scene Info is empty");
            return ZErr_ParamError;
        }

        //std::ofstream f("C:\\Users\\Admin\\Desktop\\jsondebug\\zeno3.json");
        //f << scene_info_str;
        //f.close();

        Json scene_json;
        try {
            scene_json = Json::parse(scene_info_str);
        } catch (...) {
            nd->report_error("FormSceneTree: Scene Info must be valid JSON string");
            return ZErr_ParamError;
        }
        sceneTree->root_name = "/ABC";
        auto* prim_geom_list = nd->get_input_ListObject("Geometry List");
        auto* listObj = dynamic_cast<ListObject*>(prim_geom_list);
        sceneTree->bNeedUpdateDescriptor =
            listObj && (!listObj->m_new_added.empty() || !listObj->m_modify.empty());

        if (sceneTree->bNeedUpdateDescriptor && listObj) {
            for (auto p : listObj->get()) {
                auto* geom = dynamic_cast<GeometryObject*>(p);
                if (geom) {
                    Vec3f bbmin, bbmax;
                    if (geomBoundingBox(geom, bbmin, bbmax)) {
                        geom->userData()->set_vec3f("_bboxMin", bbmin);
                        geom->userData()->set_vec3f("_bboxMax", bbmax);
                    }
                    char buf[512];
                    geom->userData()->get_string("abcpath_0", "", buf, sizeof(buf));
                    auto abc_path = std::string(buf);
                    geom->userData()->set_string("ResourceType", "Mesh");
                    geom->userData()->set_string("ObjectName", stdString2zs(abc_path));
                    IObject2* cloned = geom->clone();
                    sceneTree->geom_list[abc_path] =
                        std::unique_ptr<GeometryObject>(static_cast<GeometryObject*>(cloned));
                }
            }
        }
        get_local_matrix_map(scene_json, "", sceneTree.get());
        if (nd->get_input2_bool("flattened")) {
            sceneTree->flatten();
        }
        nd->set_output_object("scene", sceneTree.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(FormSceneTree, {
    {{gParamType_String, "Scene Info"}, {gParamType_List, "Geometry List"}, {gParamType_Bool, "flattened", "1"}},
    {{gParamType_Scene, "scene"}},
    {},
    {"Scene"}
});

struct ConvertXformToMatrix : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("xform");
        if (!raw) {
            nd->report_error("ConvertXformToMatrix: no xform input");
            return ZErr_UnimplError;
        }
        auto xform = std::unique_ptr<PrimitiveObject>(static_cast<PrimitiveObject*>(raw));
        AttrVector<vec3f> verts(xform->verts.size() * 4);
        auto& t_attr = xform->verts.values;
        auto& r0_attr = xform->verts.attr<vec3f>("r0");
        auto& r1_attr = xform->verts.attr<vec3f>("r1");
        auto& r2_attr = xform->verts.attr<vec3f>("r2");
        for (auto i = 0; i < xform->verts.size(); i++) {
            auto r0 = r0_attr[i], r1 = r1_attr[i], r2 = r2_attr[i], t = t_attr[i];
            verts[0 + i * 4][0] = r0[0];
            verts[0 + i * 4][1] = r1[0];
            verts[0 + i * 4][2] = r2[0];
            verts[1 + i * 4][0] = t[0];
            verts[1 + i * 4][1] = r0[1];
            verts[1 + i * 4][2] = r1[1];
            verts[2 + i * 4][0] = r2[1];
            verts[2 + i * 4][1] = t[1];
            verts[2 + i * 4][2] = r0[2];
            verts[3 + i * 4][0] = r1[2];
            verts[3 + i * 4][1] = r2[2];
            verts[3 + i * 4][2] = t[2];
        }
        xform->verts = verts;
        nd->set_output_object("matrix", xform.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(ConvertXformToMatrix,
           {{{gParamType_Primitive, "xform"}}, {{gParamType_Primitive, "matrix"}}, {}, {"Scene"}});

struct MergeScene : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        IObject2* raw = nd->clone_input_object("Main Scene");
        auto main_scene = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        auto namespace1 = get_input2_string_helper(nd, "namespace1");
        if (namespace1.size()) {
            if (!zeno::starts_with(namespace1, "/")) {
                namespace1 = "/" + namespace1;
            }
            std::vector<glm::mat4> xform1;
            if (nd->has_link_input("xform1")) {
                auto any = params->get_param_result("xform1");
                if (any)
                    xform1 = zeno::reflect::any_cast<std::vector<glm::mat4>>(any);
            }
            scene_add_prefix_node(namespace1, xform1, main_scene.get());
        }
        if (nd->has_input("Second Scene")) {
            auto* second_obj = nd->get_input_object("Second Scene");
            auto* second_scene = dynamic_cast<SceneObject*>(second_obj);
            if (second_scene) {
                auto namespace2 = get_input2_string_helper(nd, "namespace2");
                if (namespace2.size()) {
                    if (!zeno::starts_with(namespace2, "/"))
                        namespace2 = "/" + namespace2;
                    std::vector<glm::mat4> xform2;
                    if (nd->has_link_input("xform2")) {
                        auto any = params->get_param_result("xform2");
                        if (any)
                            xform2 = zeno::reflect::any_cast<std::vector<glm::mat4>>(any);
                    }
                    scene_add_prefix_node(namespace2, xform2, second_scene);
                }
                auto insert_path = get_input2_string_helper(nd, "insert_path");
                if (insert_path.size() && !zeno::starts_with(insert_path, "/"))
                    insert_path = "/" + insert_path;
                merge_scene2_into_scene1(main_scene.get(), second_scene, namespace1 + insert_path);
            }
        }
        nd->set_output_object("scene", main_scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(MergeScene, {
    {{gParamType_Scene, "Main Scene"},
     {gParamType_Scene, "Second Scene"},
     {gParamType_String, "insert_path", ""},
     {gParamType_String, "namespace1", ""},
     {gParamType_ListOfMat4, "xform1"},
     {gParamType_String, "namespace2", "namespace2"},
     {gParamType_ListOfMat4, "xform2"}},
    {{gParamType_Scene, "scene"}},
    {},
    {"Scene"}
});

struct SceneRootRename : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        auto* scene_obj = nd->get_input_object("scene");
        auto* scene_tree = dynamic_cast<SceneObject*>(scene_obj);
        if (!scene_tree) {
            nd->report_error("SceneRootRename: scene must be SceneObject");
            return ZErr_UnimplError;
        }
        auto new_root_name = get_input2_string_helper(nd, "new_root_name");
        if (zeno::ends_with(new_root_name, "/"))
            new_root_name.pop_back();
        if (new_root_name.empty())
            new_root_name = scene_tree->root_name;
        if (!zeno::starts_with(new_root_name, "/"))
            new_root_name = "/" + new_root_name;
        std::vector<glm::mat4> root_xform;
        if (nd->has_link_input("xform")) {
            auto any = params->get_param_result("xform");
            if (any)
                root_xform = zeno::reflect::any_cast<std::vector<glm::mat4>>(any);
        }
        auto new_scene_tree = scene_tree->root_rename(new_root_name, root_xform);
        nd->set_output_object("scene", new_scene_tree.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(SceneRootRename,
           {{{gParamType_Scene, "scene"}, {gParamType_String, "new_root_name", "new_scene"}, {gParamType_ListOfMat4, "xform"}},
            {{gParamType_Scene, "scene"}},
            {},
            {"Scene"}});

struct MergeMultiScenes : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto main_scene = std::make_unique<SceneObject>();
        main_scene->root_name = get_input2_string_helper(nd, "root_name");
        if (!main_scene->root_name.empty()) {
            if (!zeno::starts_with(main_scene->root_name, "/"))
                main_scene->root_name = "/" + main_scene->root_name;
            SceneTreeNode root_node;
            root_node.matrix = main_scene->root_name + "_m";
            main_scene->node_to_matrix[root_node.matrix] = {glm::mat4(1)};
            main_scene->scene_tree[main_scene->root_name] = root_node;
        }
        std::unordered_map<std::string, int> sub_root_names;
        if (nd->has_input("Scene List")) {
            auto* scene_list = nd->get_input_ListObject("Scene List");
            auto* listObj = dynamic_cast<ListObject*>(scene_list);
            if (listObj) {
                main_scene->bNeedUpdateDescriptor = false;
                for (size_t i = 0; i < listObj->m_objects.size(); i++) {
                    auto* second_scene = dynamic_cast<SceneObject*>(listObj->m_objects[i].get());
                    if (!second_scene)
                        continue;
                    char keybuf[256];
                    second_scene->key(keybuf, sizeof(keybuf));
                    std::string scene_obj_key(keybuf);
                    if (listObj->m_modify.find(scene_obj_key) != listObj->m_modify.end() ||
                        listObj->m_new_added.find(scene_obj_key) != listObj->m_new_added.end()) {
                        continue;
                    }
                    sub_root_names[second_scene->root_name] += 1;
                    if (sub_root_names[second_scene->root_name] > 1) {
                        zeno::log_warn("MergeMultiScenes: root_name {} is duplicate!", second_scene->root_name);
                    }
                    merge_scene2_into_scene1(main_scene.get(), second_scene, main_scene->root_name);
                    main_scene->bNeedUpdateDescriptor |= second_scene->bNeedUpdateDescriptor;
                }
            }
        }
        nd->set_output_object("scene", main_scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(MergeMultiScenes,
           {{{gParamType_List, "Scene List"}, {gParamType_String, "root_name", "dummyRoot"}},
            {{gParamType_Scene, "scene"}},
            {},
            {"Scene"}});

struct FlattenSceneTree : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("scene");
        auto scene = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        scene->flatten();
        nd->set_output_object("scene", scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(FlattenSceneTree, {{{gParamType_Scene, "scene"}}, {{gParamType_Scene, "scene"}}, {}, {"Scene"}});

struct MakeXform : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto translate = toVec3f(nd->get_input2_vec3f("translate"));
        auto eulerXYZ = toVec3f(nd->get_input2_vec3f("eulerXYZ"));
        auto scale = toVec3f(nd->get_input2_vec3f("scale"));
        glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale[0], scale[1], scale[2]));
        auto* params = static_cast<ZNodeParams*>(nd);
        auto order = params->get_input2_string("EulerRotationOrder");
        auto orderTyped = magic_enum::enum_cast<EulerAngle::RotationOrder>(order).value_or(EulerAngle::RotationOrder::YXZ);
        auto measure = params->get_input2_string("EulerAngleMeasure");
        auto measureTyped = magic_enum::enum_cast<EulerAngle::Measure>(measure).value_or(EulerAngle::Measure::Radians);
        glm::vec3 eularAngleXYZ = glm::vec3(eulerXYZ[0], eulerXYZ[1], eulerXYZ[2]);
        glm::mat4 matRotate = EulerAngle::rotate(orderTyped, measureTyped, eularAngleXYZ);
        auto mat = matRotate * matScale;
        auto xform = std::make_unique<PrimitiveObject>();
        xform->resize(1);
        xform->verts[0] = translate;
        xform->verts.add_attr<vec3f>("r0")[0] = {mat[0][0], mat[0][1], mat[0][2]};
        xform->verts.add_attr<vec3f>("r1")[0] = {mat[1][0], mat[1][1], mat[1][2]};
        xform->verts.add_attr<vec3f>("r2")[0] = {mat[2][0], mat[2][1], mat[2][2]};
        nd->set_output_object("xform", xform.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(MakeXform,
           {{{gParamType_Vec3f, "translate", "0, 0, 0"},
             {gParamType_Vec3f, "eulerXYZ", "0, 0, 0"},
             {gParamType_Vec3f, "scale", "1, 1, 1"}},
            {{gParamType_Primitive, "xform"}},
            {{"enum " + EulerAngle::RotationOrderListString(), "EulerRotationOrder", "ZYX"},
             {"enum " + EulerAngle::MeasureListString(), "EulerAngleMeasure", "Degree"}},
            {"Scene"}});

struct MarkSceneState : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("scene");
        auto scene = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        auto json = Json::parse(scene->to_json());
        json["type"] = get_input2_string_helper(nd, "type");
        json["matrixMode"] = get_input2_string_helper(nd, "matrixMode");
        scene->from_json(json.dump());
        nd->set_output_object("scene", scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(MarkSceneState,
           {{{gParamType_Scene, "scene"}, {"enum static dynamic", "type", "static"}, {"enum UnChanged TotalChange", "matrixMode", "TotalChange"}},
            {{gParamType_Scene, "scene"}},
            {},
            {"Scene"}});

struct GetNodeIds : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        auto* geom = nd->get_input_Geometry("Input");
        auto* geomObj = dynamic_cast<GeometryObject*>(geom);
        std::vector<int> ids;
        if (geomObj && geomObj->has_point_attr("id")) {
            ids = geomObj->get_attrs<int>(ATTR_POINT, "id");
        }
        params->set_primitive_output("node_ids", zeno::reflect::Any(ids));
        return ZErr_OK;
    }
};

ZENDEFNODE(GetNodeIds, {{{gParamType_Geometry, "Input"}}, {{gParamType_IntList, "node_ids"}}, {}, {"Scene"}});

struct SetNodeXform : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        IObject2* raw = nd->clone_input_object("scene");
        auto scene = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        auto node = get_input2_string_helper(nd, "node");
        if (!zeno::starts_with(node, "/"))
            node = "/" + node;
        auto st = Json::parse(scene->to_json());
        auto& node_to_matrix = st["node_to_matrix"];
        if (nd->has_input("xforms")) {
            auto any = params->get_param_result("xforms");
            if (any) {
                auto xforms = zeno::reflect::any_cast<std::vector<glm::mat4>>(any);
                Json mats = Json::array();
                for (const auto& xform : xforms) {
                    Json matrix = Json::array();
                    for (int j = 0; j < 3; j++)
                        matrix.push_back(xform[0][j]);
                    for (int j = 0; j < 3; j++)
                        matrix.push_back(xform[1][j]);
                    for (int j = 0; j < 3; j++)
                        matrix.push_back(xform[2][j]);
                    for (int j = 0; j < 3; j++)
                        matrix.push_back(xform[3][j]);
                    mats.push_back(matrix);
                }
                node_to_matrix[node + "_m"] = mats;
                if (nd->has_link_input("node_ids")) {
                    auto ids_any = params->get_param_result("node_ids");
                    if (ids_any) {
                        auto ids = zeno::reflect::any_cast<std::vector<int>>(ids_any);
                        if (!ids.empty()) {
                            Json ids_json = Json::array();
                            for (auto id : ids)
                                ids_json.push_back(id);
                            st["node_to_id"][node + "_m"] = ids_json;
                        }
                    }
                }
            }
        } else {
            auto index = nd->get_input2_int("index");
            auto r0 = toVec3f(nd->get_input2_vec3f("r0"));
            auto r1 = toVec3f(nd->get_input2_vec3f("r1"));
            auto r2 = toVec3f(nd->get_input2_vec3f("r2"));
            auto t = toVec3f(nd->get_input2_vec3f("t"));
            if (node_to_matrix.contains(node + "_m")) {
                Json matrix = Json::array();
                for (int j = 0; j < 3; j++)
                    matrix.push_back(r0[j]);
                for (int j = 0; j < 3; j++)
                    matrix.push_back(r1[j]);
                for (int j = 0; j < 3; j++)
                    matrix.push_back(r2[j]);
                for (int j = 0; j < 3; j++)
                    matrix.push_back(t[j]);
                node_to_matrix[node + "_m"][index] = matrix;
            }
        }
        scene->from_json(st.dump());
        nd->set_output_object("scene", scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(SetNodeXform, {
    {{gParamType_Scene, "scene"},
     {gParamType_String, "node", ""},
     {gParamType_Int, "index", "0"},
     {gParamType_IntList, "node_ids"},
     {gParamType_Vec3f, "r0", "1, 0, 0"},
     {gParamType_Vec3f, "r1", "0, 1, 0"},
     {gParamType_Vec3f, "r2", "0, 0, 1"},
     {gParamType_Vec3f, "t", "0, 0, 0"},
     {gParamType_ListOfMat4, "xforms"}},
    {{gParamType_Scene, "scene"}},
    {},
    {"Scene"}
});

struct SetNodeId : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("scene");
        auto scene = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        auto node = get_input2_string_helper(nd, "node");
        if (!zeno::starts_with(node, "/"))
            node = "/" + node;
        auto st = Json::parse(scene->to_json());
        auto& node_to_id = st["node_to_id"];
        auto index = nd->get_input2_int("index");
        auto id = nd->get_input2_int("id");
        if (node_to_id.contains(node + "_m")) {
            node_to_id[node + "_m"][index] = id;
        }
        scene->from_json(st.dump());
        nd->set_output_object("scene", scene.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(SetNodeId,
           {{{gParamType_Scene, "scene"}, {gParamType_String, "node", ""}, {gParamType_Int, "index", "0"}, {gParamType_Int, "id", "0"}},
            {{gParamType_Scene, "scene"}},
            {},
            {"Scene"}});

struct SetSceneXform : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        IObject2* raw = nd->clone_input_object("scene");
        auto scene_tree = safe_uniqueptr_cast<SceneObject>(std::unique_ptr<IObject2>(raw));
        auto any = params->get_param_result("xformsList");
        std::vector<std::string> xformsList;
        try {
            if (any)
                xformsList = zeno::reflect::any_cast<std::vector<std::string>>(any);
        } catch (...) {}
        for (const auto& xforms_str : xformsList) {
            if (xforms_str.size()) {
                auto xforms = Json::parse(xforms_str);
                for (const auto& [node_name, mat] : xforms.items()) {
                    auto& stn = scene_tree->scene_tree.at(node_name);
                    if (stn.matrix.empty())
                        stn.matrix = node_name + "_m";
                    auto m = glm::mat4(1);
                    for (int i = 0; i < 4; i++)
                        for (int j = 0; j < 3; j++)
                            m[i][j] = float(mat[i * 3 + j]);
                    scene_tree->node_to_matrix[stn.matrix] = {m};
                }
            }
        }
        nd->set_output_object("scene", scene_tree.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(SetSceneXform,
           {{{gParamType_Scene, "scene"}, {gParamType_StringList, "xformsList"}}, {{gParamType_Scene, "scene"}}, {}, {"Scene"}});

struct SetResourceType : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        IObject2* raw = nd->clone_input_object("Input");
        if (!raw) {
            nd->report_error("SetResourceType: no Input");
            return ZErr_UnimplError;
        }
        auto obj = std::unique_ptr<IObject2>(raw);
        auto ud = obj->userData();
        if (!ud->has_string("ResourceType")) {
            ud->set_string("ResourceType", params->get_input2_string("ResourceType").c_str());
        }
        if (!ud->has_string("ObjectName")) {
            ud->set_string("ObjectName", params->get_input2_string("ObjectName").c_str());
        }
        nd->set_output_object("Output", obj.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(SetResourceType,
           {{{_gParamType_IObject, "Input"}, {"enum Mesh Matrixes SceneDescriptor", "ResourceType", "Mesh"}, {gParamType_String, "ObjectName", ""}, {gParamType_String, "changeHint", ""}},
            {{_gParamType_IObject, "Output"}},
            {},
            {"Scene"}});

struct MakeSceneNode : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        auto scene_tree = std::make_unique<SceneObject>();
        scene_tree->root_name = params->get_input2_string("root_name");
        scene_tree->bResetOptixScene = nd->get_input2_bool("Reset Optix Scene");
        if (!zeno::starts_with(scene_tree->root_name, "/"))
            scene_tree->root_name = "/" + scene_tree->root_name;

        auto* geomRaw = nd->clone_input_Geometry("prim");
        auto* geom = dynamic_cast<GeometryObject*>(geomRaw);
        if (!geom) {
            nd->report_error("MakeSceneNode: prim must be Geometry");
            return ZErr_UnimplError;
        }
        auto geom_ptr = std::unique_ptr<GeometryObject>(geom);
        auto bbox = zeno::geomBoundingBox2(geom);
        if (bbox.has_value()) {
            vec3f bmin, bmax;
            std::tie(bmin, bmax) = bbox.value();
            geom->userData()->set_vec3f("_bboxMin", toAbiVec3f(bmin));
            geom->userData()->set_vec3f("_bboxMax", toAbiVec3f(bmax));
        }
        SceneTreeNode root_node;
        root_node.matrix = scene_tree->root_name + "_m";
        scene_tree->node_to_matrix[root_node.matrix] = {glm::mat4(1)};
        char objname_buf[512];
        geom->userData()->get_string("ObjectName", "", objname_buf, sizeof(objname_buf));
        auto obj_name = std::string(objname_buf);
        scene_tree->geom_list[obj_name] = std::move(geom_ptr);
        {
            std::string node_name = scene_tree->root_name + '/' + obj_name + "_node";
            SceneTreeNode prim_node;
            prim_node.meshes.push_back(obj_name);
            root_node.children.push_back(node_name);
            prim_node.matrix = node_name + "_m";
            scene_tree->scene_tree[node_name] = prim_node;
            scene_tree->node_to_matrix[prim_node.matrix] = {glm::mat4(1)};
        }
        scene_tree->scene_tree[scene_tree->root_name] = root_node;
        if (nd->has_link_input("xforms")) {
            auto any = params->get_param_result("xforms");
            if (any) {
                auto xforms = zeno::reflect::any_cast<std::vector<glm::mat4>>(any);
                scene_tree->node_to_matrix[scene_tree->root_name + "_m"] = xforms;
            }
        }
        nd->set_output_object("scene", scene_tree.release());
        return ZErr_OK;
    }
};

ZENDEFNODE(MakeSceneNode,
           {{{gParamType_Geometry, "prim"}, {gParamType_String, "root_name", "/ABC"}, {gParamType_ListOfMat4, "xforms"}, {gParamType_Bool, "Reset Optix Scene", "0"}},
            {{gParamType_Scene, "scene"}},
            {},
            {"Scene"}});

struct TestSceneNodeCopy : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("scene");
        nd->set_output_object("scene", raw);
        return ZErr_OK;
    }
};

ZENDEFNODE(TestSceneNodeCopy, {{{gParamType_Scene, "scene"}}, {{gParamType_Scene, "scene"}}, {}, {"Scene"}});

struct ObjectToXforms : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        std::vector<glm::mat4> xforms;
        IObject2* raw = nd->clone_input_object("IObject");
        if (!raw) {
            nd->report_error("ObjectToXforms: no IObject");
            return ZErr_UnimplError;
        }
        std::unique_ptr<IObject2> obj(raw);
        if (auto* geoObj = dynamic_cast<GeometryObject*>(obj.get())) {
            auto prim = geoObj->toPrimitive();
            xforms = get_xform_from_prim(prim.get());
        } else if (auto* primObj = dynamic_cast<PrimitiveObject*>(obj.get())) {
            xforms = get_xform_from_prim(primObj);
        }
        params->set_primitive_output("xforms", zeno::reflect::Any(xforms));
        return ZErr_OK;
    }
};

ZENDEFNODE(ObjectToXforms, {{{_gParamType_IObject, "IObject"}}, {{gParamType_ListOfMat4, "xforms"}}, {}, {"Scene"}});

}  // namespace zeno
