//
// Created by zh on 2025/4/14.
//
#include <zeno/extra/GlobalComm.h>
#include <zeno/types/DummyObject.h>
#include <zeno/core/Graph.h>
#include "zeno/types/PrimitiveObject.h"
#include <zeno/types/IGeometryObject.h>
#include "zeno/types/ListObject_impl.h"
#include "zeno/types/UserData.h"
#include "zeno/utils/fileio.h"
#include "zeno/utils/log.h"
#include "zeno/utils/string.h"
#include "zeno/utils/scope_exit.h"
#include <deque>
#include <string>
#include <tinygltf/json.hpp>
#include <zeno/zeno.h>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <zeno/utils/eulerangle.h>


using Json = nlohmann::json;
namespace zeno {

template <typename Key, typename Value>
class IndexMap {
private:
    std::vector<Key> insertion_order;
    std::unordered_map<Key, std::pair<Value, size_t>> data_map;

public:
    void insert(const Key& key, const Value& value) {
        auto it = data_map.find(key);
        if (it == data_map.end()) {
            data_map[key] = {value, insertion_order.size()};
            insertion_order.push_back(key);
        }
    }

    Value& at(const Key& key) {
        return data_map.at(key).first;
    }

    Value& at_index(size_t index) {
        if (index >= insertion_order.size()) {
            throw std::out_of_range("Index out of range");
        }
        return data_map.at(insertion_order[index]).first;
    }

    size_t get_index(const Key& key) const {
        return data_map.at(key).second;
    }

    size_t size() const {
        return insertion_order.size();
    }

    size_t count(const Key& key) const {
        return data_map.count(key);
    }

    auto begin() { return insertion_order.begin(); }
    auto end() { return insertion_order.end(); }
};
struct JsonObject : IObjectClone<JsonObject> {
    Json json;
};
static std::string get_parent_path(const std::string& path) {
    if (path.empty() || path == "/") {
        return path;  // 根目录的父目录是它自己
    }

    // 移除末尾的斜杠（如果有）
    std::string normalized = path;
    if (normalized.back() == '/') {
        normalized.pop_back();
    }

    // 查找最后一个斜杠
    auto last_slash = normalized.find_last_of('/');

    if (last_slash == std::string::npos) {
        return "/";  // 没有斜杠，返回根目录
    }

    if (last_slash == 0) {
        return "/";  // 已经是根目录的直接子目录
    }

    return normalized.substr(0, last_slash);
}

struct CppTimer {
    void tick() {
        struct timespec t;
        std::timespec_get(&t, TIME_UTC);
        last = t.tv_sec * 1e3 + t.tv_nsec * 1e-6;
    }
    void tock() {
        struct timespec t;
        std::timespec_get(&t, TIME_UTC);
        cur = t.tv_sec * 1e3 + t.tv_nsec * 1e-6;
    }
    float elapsed() const noexcept {return cur-last;}
    void tock(std::string_view tag) {
        tock();
        printf("%s: %f ms\n", tag.data(), elapsed());
    }

  private:
    double last, cur;
};

struct SceneTreeNode {
    std::vector<std::string> meshes;
    std::string matrix;
    std::vector<std::string> children;
    int visibility = 1;
};

struct SceneObject : IObject {
    IndexMap<std::string, SceneTreeNode> scene_tree;
    std::unordered_map<std::string, glm::mat4> node_to_matrix;
    std::unordered_map<std::string, std::shared_ptr<PrimitiveObject>> prim_list;
    std::unordered_map<std::string, std::shared_ptr<GeometryObject_Adapter>> geom_list;
    std::vector<std::string> geom_path;
    std::string root_name;

    zeno::SharedPtr<IObject> clone() const override {
        auto newSceneObj = std::make_shared<SceneObject>();
        newSceneObj->scene_tree = scene_tree;
        newSceneObj->node_to_matrix = node_to_matrix;
        newSceneObj->root_name = root_name;
        newSceneObj->geom_path = geom_path;
        for (auto& [key, geom] : geom_list) {
            auto new_geom = std::static_pointer_cast<GeometryObject_Adapter>(geom->clone());
            newSceneObj->geom_list.emplace(key, std::move(new_geom));
        }
        return newSceneObj;
    }

    std::string get_new_root_name(const std::string &root_name, const std::string &new_root_name, const std::string &path) {
        return new_root_name + path.substr(root_name.size());
    }

    std::shared_ptr<SceneObject> root_rename(std::string new_root_name, std::optional<glm::mat4> root_xform) {
        auto new_scene_obj = std::make_shared<SceneObject>();

        for (auto const &path: scene_tree) {
            auto new_key = get_new_root_name(root_name, new_root_name, path);
//            zeno::log_info("path_rename {} -> {}", path, new_key);
            auto &stn = scene_tree.at(path);
            SceneTreeNode nstn;
            nstn.visibility = stn.visibility;
            if (stn.matrix.size()) {
                nstn.matrix = get_new_root_name(root_name, new_root_name, stn.matrix);
            }
            for (auto & mesh: stn.meshes) {
                nstn.meshes.push_back(get_new_root_name(root_name, new_root_name, mesh));
            }
            for (auto & child: stn.children) {
                nstn.children.push_back(get_new_root_name(root_name, new_root_name, child));
            }
            new_scene_obj->scene_tree.insert(new_key, nstn);
        }

        for (auto &[k, v]: node_to_matrix) {
            auto new_key = get_new_root_name(root_name, new_root_name, k);
            new_scene_obj->node_to_matrix[new_key] = v;
        }
        for (auto &[k, p]: prim_list) {
            auto new_key = get_new_root_name(root_name, new_root_name, k);
            auto new_prim = std::static_pointer_cast<PrimitiveObject>(p->clone());
            new_prim->userData()->set_string("ObjectName", stdString2zs(new_key));
            new_scene_obj->prim_list[new_key] = new_prim;
        }
        for (const auto& k : geom_path) {
            auto new_key = get_new_root_name(root_name, new_root_name, k);
            new_scene_obj->geom_path.push_back(new_key);
        }
        for (auto &[k, p]: geom_list) {
            auto new_key = get_new_root_name(root_name, new_root_name, k);
            auto new_geom = std::static_pointer_cast<GeometryObject_Adapter>(p->clone());
            new_geom->userData()->set_string("ObjectName", stdString2zs(new_key));
            new_scene_obj->geom_list[new_key] = new_geom;
            if (!p->key().empty())
                new_geom->update_key(p->key());
        }
        new_scene_obj->root_name = new_root_name;
        std::string xform_name = new_root_name + "_m";
        if (root_xform.has_value()) {
            new_scene_obj->node_to_matrix[xform_name] = root_xform.value();
        }
        else {
            if (new_scene_obj->node_to_matrix.count(xform_name) == 0) {
                new_scene_obj->node_to_matrix[xform_name] = glm::mat4(1);
            }
        }
        return new_scene_obj;
    }

    Json to_json() {
        Json json;
        json["root_name"] = root_name;
        {
            Json part;
            for (auto &path: scene_tree) {
                auto &stn = scene_tree.at(path);
                Json node;
                node["meshes"] = Json::array();
                for (auto &mesh: stn.meshes) {
                    node["meshes"].push_back(mesh);
                }
                node["children"] = Json::array();
                for (auto &child: stn.children) {
                    node["children"].push_back(child);
                }
                node["matrix"] = stn.matrix;
                node["visibility"] = stn.visibility;
                part[path] = node;
            }
            json["scene_tree"] = part;
        }
        {
            Json mat_json;
            for (auto const &[path, mat]: node_to_matrix) {
                Json matrix = Json::array();
                for (auto i = 0; i < 4; i++) {
                    for (auto j = 0; j < 3; j++) {
                        matrix.push_back(mat[i][j]);
                    }
                }
                mat_json[path] = matrix;
            }
            json["node_to_matrix"] = mat_json;
        }
        return json;
    }
    void from_json(Json &json) {
        root_name = json["root_name"];
        {
            Json &mat_json = json["node_to_matrix"];
            for (auto& [path, mat_json] : mat_json.items()) {
                auto matrix = glm::mat4(1);
                for (auto i = 0; i < 4; i++) {
                    for (auto j = 0; j < 3; j++) {
                        int index = i * 3 + j;
                        matrix[i][j] = float(mat_json[index]);
                    }
                }
                node_to_matrix[path] = matrix;
            }
        }
        {
            Json &part = json["scene_tree"];
            for (auto& [path, jstn] : part.items()) {
                SceneTreeNode stn;
                stn.matrix = jstn["matrix"];
                stn.visibility = jstn["visibility"];
                for (auto &child : jstn["children"]) {
                    stn.children.push_back(child);
                }
                for (auto &mesh : jstn["meshes"]) {
                    stn.meshes.push_back(mesh);
                }
                scene_tree.insert(path, stn);
            }
        }
    }
    std::shared_ptr<zeno::ListObject> to_layer_structure(container_elem_update_info& summary_info, bool use_static = true) {
        auto scene_list = std::make_shared<zeno::ListObject>();
        auto dict = std::make_shared<PrimitiveObject>();
        scene_list->push_back(dict);
        {
            for (auto &[abc_path, p]: geom_list) {
                scene_list->push_back(p);
                //是否需要加到summary_info已经由外面的list参数决定
            }
            int prim_count = geom_list.size();
            dict->userData()->set_int("prim_count", prim_count);

            std::string objkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
            dict->update_key(stdString2zs(objkey));
            summary_info.new_added.insert(objkey);
        }
        {
            int matrix_count = 0;
            for (auto const &path: scene_tree) {
                auto matrix = glm::mat4(1);
                auto &stn = scene_tree.at(path);
                if (stn.visibility) {
                    if (stn.matrix.size() && node_to_matrix.count(stn.matrix)) {
                        matrix = node_to_matrix[stn.matrix];
                    }
                    else {
                        continue;
                    }
                }
                else {
                    matrix = glm::mat4(0);
                }
                auto r0 = matrix[0];
                auto r1 = matrix[1];
                auto r2 = matrix[2];
                auto t  = matrix[3];
                auto prim = std::make_shared<PrimitiveObject>();
                prim->verts.resize(1);
                prim->verts[0] = {t[0], t[1], t[2]};
                prim->verts.add_attr<vec3f>("r0")[0] = {r0[0], r0[1], r0[2]};
                prim->verts.add_attr<vec3f>("r1")[0] = {r1[0], r1[1], r1[2]};
                prim->verts.add_attr<vec3f>("r2")[0] = {r2[0], r2[1], r2[2]};

                prim->userData()->set_string("ResourceType", "Matrixes");
                if (use_static) {
                    prim->userData()->set_string("stamp-change", "UnChanged");
                }
                else {
                    prim->userData()->set_string("stamp-change", "TotalChange");
                }
                std::string object_name = path + "_m";
                if (stn.matrix.size()) {
                    object_name = path + "_m";
                }
                prim->userData()->set_string("ObjectName", stdString2zs(object_name));

                std::string primkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
                prim->update_key(stdString2zs(primkey));
                scene_list->push_back(prim);
                //这些矩阵都是执行到这里新生成的，所以直接加到new_added.
                summary_info.new_added.insert(primkey);
                matrix_count += 1;
            }
            dict->userData()->set_int("matrix_count", matrix_count);
        }
        {
            auto scene_descriptor = std::make_shared<PrimitiveObject>();
            auto ud = scene_descriptor->userData();
            ud->set_string("ResourceType", "SceneDescriptor");
            Json json;
            Json BasicRenderInstances = Json();
            for (const auto &[path, prim]: geom_list) {
                BasicRenderInstances[path]["Geom"] = path;
                BasicRenderInstances[path]["Material"] = "Default";
            }
            json["BasicRenderInstances"] = BasicRenderInstances;

            Json RenderGroups = Json();
            for (auto const &path: scene_tree) {
                auto &stn = scene_tree.at(path);
                Json render_group = Json();
                for (auto &child: stn.children) {
                    render_group[child] = Json::array({path+"_m"});
                }
                for (auto &child: stn.meshes) {
                    render_group[child] = Json::array({path+"_m"});
                }
                RenderGroups[path] = render_group;
            }
            if (use_static) {
                json["StaticRenderGroups"] = RenderGroups;
            }
            else {
                json["DynamicRenderGroups"] = RenderGroups;
            }
            ud->set_string("Scene", stdString2zs(std::string(json.dump())));
            std::string objkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
            summary_info.new_added.insert(objkey);
            scene_descriptor->update_key(stdString2zs(objkey));
            scene_list->push_back(scene_descriptor);
        }
        {
            auto st = std::make_shared<JsonObject>();
            st->json = to_json();
            std::string objkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
            summary_info.new_added.insert(objkey);
            st->update_key(stdString2zs(objkey));
            scene_list->push_back(st);
        }
        return scene_list;
    }

    std::shared_ptr<zeno::ListObject> to_flatten_structure(container_elem_update_info& summary_info, bool use_static) {
//        zeno::log_info("to_flatten_structure root_name: {}", root_name);
        auto scene_list = std::make_shared<zeno::ListObject>();
        auto dict = std::make_shared<PrimitiveObject>();
        scene_list->push_back(dict);
        {
            for (auto &[abc_path, p]: geom_list) {
                scene_list->push_back(p);
            }
            dict->userData()->set_int("prim_count", int(geom_list.size()));

            std::string objkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
            dict->update_key(stdString2zs(objkey));
            summary_info.new_added.insert(objkey);
        }
        {
            std::unordered_map<std::string, std::vector<glm::mat4>> tmp_matrix_xforms;
            std::deque<std::pair<std::string, glm::mat4>> worker;
            worker.emplace_back(root_name, glm::mat4(1));
            while (worker.size()) {
                auto [path, parent_global_matrix] = worker.front();
                if (scene_tree.count(path) == 0) {
                    zeno::log_error("path: {} not found, size: {}", path, path.size());
                }
                auto stn = scene_tree.at(path);
                worker.pop_front();

                auto local_mat = glm::mat4(1);
                if (stn.visibility) {
                    if (stn.matrix.size()) {
                        local_mat = node_to_matrix[stn.matrix];
                    }
                }
                else {
                    local_mat = glm::mat4(0);
                }
                auto global_matrix = parent_global_matrix * local_mat;
                for (auto &mesh: stn.meshes) {
                    tmp_matrix_xforms[mesh].push_back(global_matrix);
                }
                for (auto &child: stn.children) {
                    worker.emplace_back(child, global_matrix);
                    if (child.empty()) {
                        zeno::log_info("path child empty: {}", path);
                    }
                }
            }
            for (auto &[mesh_name, mats]: tmp_matrix_xforms) {
                auto matrix = std::make_shared<PrimitiveObject>();
                matrix->resize(mats.size());
                auto &r0 = matrix->verts.add_attr<vec3f>("r0");
                auto &r1 = matrix->verts.add_attr<vec3f>("r1");
                auto &r2 = matrix->verts.add_attr<vec3f>("r2");
                for (auto i = 0; i < mats.size(); i++) {
                    auto & mat = mats[i];
                    r0[i] = {mat[0][0], mat[0][1], mat[0][2]};
                    r1[i] = {mat[1][0], mat[1][1], mat[1][2]};
                    r2[i] = {mat[2][0], mat[2][1], mat[2][2]};
                    matrix->verts[i] = {mat[3][0], mat[3][1], mat[3][2]};
                }
                matrix->userData()->set_string("ResourceType", "Matrixes");
                if (use_static) {
                    matrix->userData()->set_string("stamp-change", "UnChanged");
                }
                else {
                    matrix->userData()->set_string("stamp-change", "TotalChange");
                }
                std::string object_name = mesh_name + "_m";
                matrix->userData()->set_string("ObjectName", stdString2zs(object_name));

                std::string primkey = summary_info.container_key + "\\matrix" + std::to_string(scene_list->size());
                matrix->update_key(stdString2zs(primkey));
                scene_list->push_back(matrix);
                //这些矩阵都是执行到这里新生成的，所以直接加到new_added.
                summary_info.new_added.insert(primkey);
            }
            dict->userData()->set_int("matrix_count", int(tmp_matrix_xforms.size()));
        }
        {
            auto scene_descriptor = std::make_shared<PrimitiveObject>();
            auto ud = scene_descriptor->userData();
            ud->set_string("ResourceType", "SceneDescriptor");
            Json json;
            json["BasicRenderInstances"] = Json();
            if (!geom_path.empty()) {
                for (const auto& path : geom_path) {
                    json["BasicRenderInstances"][path]["Geom"] = path;
                    json["BasicRenderInstances"][path]["Material"] = "Default";
                    if (use_static) {
                        json["StaticRenderGroups"]["StaticObjects"][path] = Json::array({ path + "_m" });
                    }
                    else {
                        json["DynamicRenderGroups"]["DynamicObjects"][path] = Json::array({ path + "_m" });
                    }
                }
            }
            else {
                for (const auto& [path, prim] : geom_list) {
                    json["BasicRenderInstances"][path]["Geom"] = path;
                    json["BasicRenderInstances"][path]["Material"] = "Default";
                    if (use_static) {
                        json["StaticRenderGroups"]["StaticObjects"][path] = Json::array({ path + "_m" });
                    }
                    else {
                        json["DynamicRenderGroups"]["DynamicObjects"][path] = Json::array({ path + "_m" });
                    }
                }
            }

            ud->set_string("Scene", stdString2zs(std::string(json.dump())));

            std::string objkey = summary_info.container_key + "\\json" + std::to_string(scene_list->size());
            summary_info.new_added.insert(objkey);
            scene_descriptor->update_key(stdString2zs(objkey));
            scene_list->push_back(scene_descriptor);
        }
        {
            auto st = std::make_shared<JsonObject>();
            st->json = to_json();

            std::string objkey = summary_info.container_key + "\\" + std::to_string(scene_list->size());
            summary_info.new_added.insert(objkey);
            st->update_key(stdString2zs(objkey));
            scene_list->push_back(st);
        }
        return scene_list;
    }
};

static std::shared_ptr<SceneObject> get_scene_tree_from_list(std::shared_ptr<ListObject> list_obj) {
    auto scene_tree = std::make_shared<SceneObject>();
    auto json_obj = std::static_pointer_cast<JsonObject>(list_obj->m_impl->m_objects.back());
    scene_tree->from_json(json_obj->json);
    auto prim_list_size = list_obj->m_impl->m_objects.front()->userData()->get_int("prim_count");
    for (auto i = 1; i <= prim_list_size; i++) {
        auto prim = std::static_pointer_cast<GeometryObject_Adapter>(list_obj->m_impl->m_objects[i]);
        auto object_name = zsString2Std(prim->userData()->get_string("ObjectName"));
        scene_tree->geom_list[object_name] = prim;
        scene_tree->geom_path.push_back(object_name);
    }
    return scene_tree;
}
#if 0
std::shared_ptr<zeno::ListObject> scene_tree_to_structure(SceneObject* sceneSource) {
    std::unordered_map<std::string, glm::mat4> global_matrices;
    std::unordered_map<std::string, std::vector<std::shared_ptr<PrimitiveObject>>> temp_matrices;
    global_matrices["/ABC"] = glm::mat4(1);
    for (const auto &path: sceneSource->matrices) {
        if (path == "/ABC") {
            continue;
        }
        auto parent_path = get_parent_path(path);
        if (global_matrices.count(parent_path) == 0) {
            zeno::log_error("in {}, parent_matrix {} is missing", path, parent_path);
        }
        auto parent_matrix = global_matrices.at(parent_path);
        if (sceneSource->matrices.count(path) == 0) {
            zeno::log_error("in {}, local_matrix {} is missing", path, parent_path);
        }
        auto local_matrix = sceneSource->matrices.at(path);
        if (sceneSource->node_visibility.count(path)) {
            if (sceneSource->node_visibility.at(path) == 0) {
                local_matrix = glm::mat4(0);
            }
        }
        auto global_matrix = parent_matrix * local_matrix;
        auto r0 = global_matrix[0];
        auto r1 = global_matrix[1];
        auto r2 = global_matrix[2];
        auto t  = global_matrix[3];
        global_matrices[path] = global_matrix;
        auto prim = std::make_shared<PrimitiveObject>();
        prim->verts.resize(1);
        prim->verts[0] = {t[0], t[1], t[2]};
        prim->verts.add_attr<vec3f>("r0")[0] = {r0[0], r0[1], r0[2]};
        prim->verts.add_attr<vec3f>("r1")[0] = {r1[0], r1[1], r1[2]};
        prim->verts.add_attr<vec3f>("r2")[0] = {r2[0], r2[1], r2[2]};
        auto zeno_scene_name = path;
        if (sceneSource->instance_source_paths.count(zeno_scene_name)) {
            zeno_scene_name = sceneSource->instance_source_paths[zeno_scene_name];
        }
        temp_matrices[zeno_scene_name].push_back(prim);
    }
    auto scene = std::make_shared<zeno::ListObject>();

    auto dict = std::make_shared<PrimitiveObject>();

    Json dict_json;
    int index = 0;
    for (auto &[zeno_scene_name, prims]: temp_matrices) {
        std::vector<PrimitiveObject*> prims_raws;
        for (auto &prim: prims) {
            prims_raws.push_back(prim.get());
        }
        auto prim = primMerge(prims_raws);
        prim->userData()->set_("ResourceType", "Matrixes");
        prim->userData()->set_("stamp-change", "TotalChange");
        prim->userData()->set_("ObjectName", zeno_scene_name+"_m");
        scene->push_back(prim);
        dict_json[zeno_scene_name+"_m"] = index;
        index += 1;
    }
    dict->userData()->set_("json", dict_json.dump());
    auto prim_list_size = sceneSource->prim_list.size();
    dict->userData()->set_("prim_list_size", int(prim_list_size));

    scene->insert(scene->arr.begin(), dict);
    {
        auto scene_descriptor = std::make_shared<PrimitiveObject>();
        auto ud = scene_descriptor->userData();
        ud->set_("ResourceType", std::string("SceneDescriptor"));
        Json json;
        json["BasicRenderInstances"] = Json();
        json["DynamicRenderGroups"]["Objects"] = Json();
        json["StaticRenderGroups"]["Objects"] = Json();
        for (const auto &[path, prim]: sceneSource->prim_list) {
            json["BasicRenderInstances"][path]["Geom"] = path;
            json["StaticRenderGroups"]["Objects"][path] = Json::array({path+"_m"});
        }

        ud->set_("Scene", std::string(json.dump()));
        scene->push_back(scene_descriptor);
    }

    // prim list
    std::vector<std::string> prim_list_name;
    {
        std::vector<std::shared_ptr<PrimitiveObject>> prim_list;
        for (auto &[abc_path, p]: sceneSource->prim_list) {
            p->userData()->set_("ObjectName", abc_path);
            prim_list.push_back(p);
            prim_list_name.push_back(abc_path);
        }
        scene->arr.insert(scene->arr.begin() + 1, prim_list.begin(), prim_list.end());
    }
    {
        Json json;
        for (auto i = 0; i < prim_list_name.size(); i++) {
            auto prim_name = prim_list_name[i];
            json[prim_name] = i;
        }
        scene->arr[0]->userData()->set_("prim_list_name", json.dump());
    }
    // scene tree prim
    {
        auto scene_tree = std::make_shared<JsonObject>();
        scene_tree->json = sceneSource->to_json();
        scene->push_back(scene_tree);
    }
    return scene;
}
#endif
static void get_local_matrix_map(
    Json &json
    , std::string parent_path
    , std::shared_ptr<SceneObject> scene
) {
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
    mat[3] = {float(t[0]),  float(t[1]),  float(t[2]),  1.0f};
    scene->node_to_matrix[node_path+"_m"] = mat;
    stn.matrix = node_path+"_m";
    stn.visibility = json["visibility"] == 0? 0 : 1;
    if (json.contains("mesh")) {
        stn.meshes.push_back(node_path + "/" + std::string(json["mesh"]));
    }

    for (auto i = 0; i < json["children_name"].size(); i++) {
        std::string child_name = json["children_name"][i];
        std::string child_path = node_path + '/' + child_name;
        if (json[child_name].contains("instance_source_path")) {
            stn.children.push_back(json[child_name]["instance_source_path"]);
        }
        else {
            stn.children.push_back(child_path);
        }
    }
    scene->scene_tree.insert(node_path, stn);

    for (auto i = 0; i < json["children_name"].size(); i++) {
        std::string child_name = json["children_name"][i];
        if (!json[child_name].contains("instance_source_path")) {
            get_local_matrix_map(json[child_name], node_path, scene);
        }
    }
}

struct FormSceneTree : zeno::INode {
    void apply() override {
        auto sceneTree = std::make_shared<SceneObject>();
        auto scene_json = ZImpl(get_input2<JsonObject>("scene_info"));
        sceneTree->root_name = "/ABC";
        auto prim_geom_list = ZImpl(get_input2<ListObject>("prim_list"));
        auto list_updateinfo = get_input_container_info("prim_list");

//        zeno::log_info("prim_list: {}", prim_list->arr.size());
        for (auto p: prim_geom_list->get()) {
            auto abc_path = zsString2Std(p->userData()->get_string("abcpath_0"));
            {
                auto session = &zeno::getSession();
                int currframe = session->globalState->getFrameId();
                int beginframe = session->globalComm->beginFrameNumber;
                /* zeno3不需要stamp机制
                std::string mode = ZImpl(get_input2<std::string>("stampMode"));
                if (mode == "UnChanged") {
                    p->userData()->set_string("stamp-change", "UnChanged");
                } else if (mode == "TotalChange") {
                    p->userData()->set_string("stamp-change", "TotalChange");
                } else if (mode == "DataChange") {
                    p->userData()->set_string("stamp-change", "DataChange");
                    std::string changehint = ZImpl(get_input2<std::string>("changeHint"));
                    p->userData()->set_string("stamp-dataChange-hint", stdString2zs(changehint));
                } else if (mode == "ShapeChange") {
                    p->userData()->set_string("stamp-change", "TotalChange");//shapechange暂时全部按Totalchange处理
                }
                */
                if (!p->userData()->has_string("ResourceType")) {
                    auto sResourceType = ZImpl(get_input2<std::string>("ResourceType"));
                    p->userData()->set_string("ResourceType", stdString2zs(sResourceType));
                }
            }
            /*
            if (auto prim = std::dynamic_pointer_cast<PrimitiveObject>(p)) {
                prim->userData()->set_string("ObjectName", stdString2zs(abc_path));
                sceneTree->prim_list[abc_path] = prim;
            }
            else */if (auto geom = std::dynamic_pointer_cast<GeometryObject_Adapter>(p)) {
                geom->userData()->set_string("ObjectName", stdString2zs(abc_path));
                //total_updateinfo.modified.insert(zsString2Std(p->key()));
                sceneTree->geom_list[abc_path] = geom;
                sceneTree->geom_path.push_back(abc_path);
            }
        }
        get_local_matrix_map(scene_json->json, "", sceneTree);
        container_elem_update_info total_updateinfo;
        total_updateinfo.merge(list_updateinfo);
        total_updateinfo.container_key = zsString2Std(this->uuid());
        auto scene_list = sceneTree->to_layer_structure(total_updateinfo);
        set_output_container_info("scene", total_updateinfo);
        set_output("scene", scene_list);
    }
};

ZENDEFNODE( FormSceneTree, {
    {
        {gParamType_JsonObject, "scene_info"},
        {gParamType_List, "prim_list"},
        {"enum Mesh Matrixes SceneDescriptor", "ResourceType", "Mesh"},
        {"enum UnChanged DataChange ShapeChange TotalChange", "stampMode", "UnChanged"},
        {gParamType_String, "changeHint", ""}
    },
    {
        {gParamType_List, "scene"},
    },
    {},
    {
        "Scene",
    },
});


struct FormSceneTree2 : zeno::INode {
    void apply() override {
        auto sceneTree = std::make_shared<SceneObject>();
        auto scene_json = ZImpl(get_input2<JsonObject>("Scene Info"));
        sceneTree->root_name = "/ABC";
        auto prim_geom_list = get_input_ListObject("Geometry List");
        auto list_updateinfo = get_input_container_info("Geometry List");
        std::vector<std::string> abc_paths;
        if (has_link_input("ABC Path List")) {
            abc_paths = zeno::reflect::any_cast<std::vector<std::string>>(ZImpl(get_param_result("ABC Path List")));
        }

        //如果prim_geom_list的上游节点标记为no-cache,那这里就不应该拿到prim_geom_list.
        if (prim_geom_list) {
            for (auto p : prim_geom_list->get()) {
                auto abc_path = zsString2Std(p->userData()->get_string("abcpath_0"));
                if (!p->userData()->has_string("ResourceType")) {
                    auto sResourceType = ZImpl(get_input2<std::string>("ResourceType"));
                    p->userData()->set_string("ResourceType", stdString2zs(sResourceType));
                }
                if (auto geom = std::dynamic_pointer_cast<GeometryObject_Adapter>(p)) {
                    geom->userData()->set_string("ObjectName", stdString2zs(abc_path));
                    if (!list_updateinfo.empty()) {
                        sceneTree->geom_list[abc_path] = geom;
                    }
                }
            }
        }
        sceneTree->geom_path = abc_paths;
        get_local_matrix_map(scene_json->json, "", sceneTree);
        set_output("scene", sceneTree);
    }
};

ZENDEFNODE(FormSceneTree2, {
    {
        {gParamType_JsonObject, "Scene Info"},
        {gParamType_List, "Geometry List"},
        {"enum Mesh Matrixes SceneDescriptor", "ResourceType", "Mesh"},
        {"enum UnChanged DataChange ShapeChange TotalChange", "stampMode", "UnChanged"},
        {gParamType_String, "changeHint", ""},
        {gParamType_StringList, "ABC Path List", ""}
    },
    {
        {gParamType_Scene, "scene"},
    },
    {},
    {
        "Scene"
    }
});



static void scene_add_prefix(
    std::string path
    , glm::mat4 xform
    , std::shared_ptr<SceneObject> sceneObject
) {
    IndexMap<std::string, SceneTreeNode> scene_tree;
    for (const auto &key: sceneObject->scene_tree) {
        auto &value = sceneObject->scene_tree.at(key);
        SceneTreeNode stn;
        stn.visibility = value.visibility;
        if (value.matrix.size()) {
            stn.matrix = path + value.matrix;
        }
        for (auto &child: value.children) {
            stn.children.push_back(path + child);
        }
        for (auto &mesh: value.meshes) {
            stn.meshes.push_back(path + mesh);
        }
        scene_tree.insert(path + key, stn);
    }
    std::unordered_map<std::string, std::shared_ptr<PrimitiveObject>> new_prim_list;
    for (auto& [key, value] : sceneObject->prim_list) {
        auto obj_name = zsString2Std(value->userData()->get_string("ObjectName"));
        obj_name = path + obj_name;
        value->userData()->set_string("ObjectName", stdString2zs(obj_name));
        new_prim_list[path + key] = value;
    }
    std::unordered_map<std::string, glm::mat4> new_node_to_matrix;
    for (const auto& [key, value] : sceneObject->node_to_matrix) {
        new_node_to_matrix[path + key] = value;
    }
    sceneObject->scene_tree = scene_tree;
    sceneObject->prim_list = new_prim_list;
    sceneObject->node_to_matrix = new_node_to_matrix;
    {
        std::string xform_name = path + "_m";
        sceneObject->node_to_matrix[xform_name] = xform;
    }
}
static std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> result;
    if (path.empty()) return result;

    std::istringstream iss(path);
    std::string segment;
    std::string currentPath;

    //  跳过第一个空段（因为路径以 '/' 开头）
    std::getline(iss, segment, '/');

    while (std::getline(iss, segment, '/')) {
        if (segment.empty()) continue; //  跳过连续的 '/'
        currentPath += "/" + segment;
        result.push_back(currentPath);
    }

    return result;
}
static void merge_scene(
    std::shared_ptr<SceneObject> main_object
    , std::shared_ptr<SceneObject> second_object
) {
    for (const auto& key : second_object->scene_tree) {
        auto &stn = second_object->scene_tree.at(key);
        main_object->scene_tree.insert(key, stn);
    }
    for (const auto& [key, mat] : second_object->node_to_matrix) {
        main_object->node_to_matrix[key] = mat;
    }
    for (const auto& [key, prim] : second_object->prim_list) {
        main_object->prim_list[key] = prim;
    }
}
glm::mat4 get_xform_from_prim(std::shared_ptr<PrimitiveObject> prim) {
    auto pos = prim->verts[0];
    auto r0 = prim->verts.add_attr<vec3f>("r0")[0];
    auto r1 = prim->verts.add_attr<vec3f>("r1")[0];
    auto r2 = prim->verts.add_attr<vec3f>("r2")[0];
    glm::mat4 mat;
    mat[0] = {r0[0], r0[1], r0[2], 0};
    mat[1] = {r1[0], r1[1], r1[2], 0};
    mat[2] = {r2[0], r2[1], r2[2], 0};
    mat[3] = {pos[0], pos[1], pos[2], 1};
    return mat;
}

struct MergeScene : zeno::INode {
    void apply() override {
        auto main_scene = get_scene_tree_from_list(ZImpl(get_input2<ListObject>("main_scene")));
        auto second_scene = get_scene_tree_from_list(ZImpl(get_input2<ListObject>("second_scene")));
        auto namespace1 = ZImpl(get_input2<std::string>("namespace1"));
        auto namespace2 = ZImpl(get_input2<std::string>("namespace2"));
        auto insert_path = ZImpl(get_input2<std::string>("insert_path"));

        auto append_path1 = (namespace1 == ""? "" : "/") + namespace1;
        if (append_path1.size()) {
            glm::mat4 xform1 = glm::mat4(1);
            if (ZImpl(has_input2<PrimitiveObject>("xform1"))) {
                xform1 = get_xform_from_prim(ZImpl(get_input2<PrimitiveObject>("xform1")));
            }
            scene_add_prefix(append_path1, xform1, main_scene);
        }
        auto append_path2 = append_path1 + (insert_path == ""? "" : "/") + insert_path + (namespace2 == ""? "" : "/") + namespace2;
        glm::mat4 xform2 = glm::mat4(1);
        if (ZImpl(has_input2<PrimitiveObject>("xform2"))) {
            xform2 = get_xform_from_prim(ZImpl(get_input2<PrimitiveObject>("xform2")));
        }
        scene_add_prefix(append_path2, xform2, second_scene);

        merge_scene(main_scene, second_scene);
        if (append_path1.size()) {
            auto abc_stn = SceneTreeNode();
            abc_stn.children.push_back(append_path1 + main_scene->root_name);
            main_scene->scene_tree.insert(append_path1, abc_stn);
            main_scene->root_name = append_path1;
        }
        {
            auto inner_parent = append_path1 + (insert_path == ""? "" : "/") + insert_path;
            main_scene->scene_tree.at(inner_parent).children.push_back( namespace2==""?append_path2+second_scene->root_name:append_path2);
        }
        container_elem_update_info _;
        auto scene = main_scene->to_layer_structure(_);
        set_output("scene", scene);
    }
};

ZENDEFNODE( MergeScene, {
    {
        {gParamType_List, "main_scene"},
        {gParamType_List, "second_scene"},
        {gParamType_String , "namespace1", ""},
        {gParamType_String, "namespace2", "namespace2"},
        {gParamType_String, "insert_path", ""},
        {gParamType_Primitive, "xform1"},
        {gParamType_Primitive, "xform2"},
    },
    {
        {gParamType_List, "scene"},
    },
    {},
    {
        "Scene",
    },
});


struct FlattenSceneTree : zeno::INode {
    void apply() override {
        auto scene_tree = get_scene_tree_from_list(get_input_ListObject("scene"));
        auto use_static = get_input2_bool("use_static");

        container_elem_update_info _;
        auto scene = scene_tree->to_flatten_structure(_, use_static);
        set_output("scene", scene);
    }
};

ZENDEFNODE( FlattenSceneTree, {
    {
        {gParamType_List, "scene"},
        {gParamType_Bool, "use_static", "1"},
    },
    {
        {gParamType_List, "scene"}
    },
    {},
    {
        "Scene",
    },
});


static void scene_add_prefix2(
    std::string path
    , glm::mat4 xform
    , std::shared_ptr<SceneObject> sceneObject
) {
    IndexMap<std::string, SceneTreeNode> scene_tree;
    for (const auto &key: sceneObject->scene_tree) {
        auto &value = sceneObject->scene_tree.at(key);
        SceneTreeNode stn;
        stn.visibility = value.visibility;
        if (value.matrix.size()) {
            stn.matrix = "/ABC" + path + value.matrix.substr(4);
        }
        for (auto &child: value.children) {
            stn.children.push_back("/ABC" + path + child.substr(4));
        }
        for (auto &mesh: value.meshes) {
            stn.meshes.push_back("/ABC" + path + mesh.substr(4));
        }
        scene_tree.insert("/ABC" + path + key.substr(4), stn);
    }
    std::unordered_map<std::string, std::shared_ptr<PrimitiveObject>> new_prim_list;
    for (auto& [key, value] : sceneObject->prim_list) {
        auto obj_name = zsString2Std(value->userData()->get_string("ObjectName"));
        obj_name = "/ABC" + path + obj_name.substr(4);
        value->userData()->set_string("ObjectName", stdString2zs(obj_name));
        new_prim_list["/ABC" + path + key.substr(4)] = value;
    }
    std::unordered_map<std::string, glm::mat4> new_node_to_matrix;
    for (const auto& [key, value] : sceneObject->node_to_matrix) {
        new_node_to_matrix["/ABC" + path + key.substr(4)] = value;
    }
    sceneObject->scene_tree = scene_tree;
    sceneObject->prim_list = new_prim_list;
    sceneObject->node_to_matrix = new_node_to_matrix;
    {
        std::string xform_name = "/ABC" + path + "_m";
        sceneObject->node_to_matrix[xform_name] = xform;
    }
}

struct SceneRootRename : zeno::INode {
    void apply() override {
        auto scene_tree = get_scene_tree_from_list(get_input_ListObject("scene"));
        auto input_updateinfo = get_input_container_info("scene");
//        zeno::log_info("SceneRootRename input root_name {}", scene_tree->root_name);
        auto new_root_name = zsString2Std(get_input2_string("new_root_name"));
        if (zeno::ends_with(new_root_name, "/")) {
            new_root_name.pop_back();
        }
        if (new_root_name.empty()) {
            new_root_name = scene_tree->root_name;
        }
        if (zeno::starts_with(new_root_name, "/") == false) {
            new_root_name = "/" + new_root_name;
        }
        std::optional<glm::mat4> root_xform = std::nullopt;
        if (ZImpl(has_input2<PrimitiveObject>("xform"))) {
            root_xform = get_xform_from_prim(get_input_PrimitiveObject("xform"));
        }
        auto new_scene_tree = scene_tree->root_rename(new_root_name, root_xform);
//        zeno::log_info("SceneRootRename output root_name {}", new_scene_tree->root_name);
        container_elem_update_info total_updateinfo;
        total_updateinfo.merge(input_updateinfo);
        auto scene = new_scene_tree->to_layer_structure(total_updateinfo);
        set_output_container_info("scene", total_updateinfo);
        set_output("scene", scene);
    }
};

ZENDEFNODE( SceneRootRename, {
    {
        {gParamType_List, "scene"},
        {gParamType_String, "new_root_name", "new_scene"},
        {gParamType_Primitive, "xform"},
    },
    {
        {gParamType_List, "scene"},
    },
    {},
    {
        "Scene",
    },
});

struct SceneRootRename2 : zeno::INode {
    void apply() override {
        auto scene_tree = std::dynamic_pointer_cast<SceneObject>(get_input("scene"));
        auto new_root_name = zsString2Std(get_input2_string("new_root_name"));
        if (zeno::ends_with(new_root_name, "/")) {
            new_root_name.pop_back();
        }
        if (new_root_name.empty()) {
            new_root_name = scene_tree->root_name;
        }
        if (zeno::starts_with(new_root_name, "/") == false) {
            new_root_name = "/" + new_root_name;
        }
        std::optional<glm::mat4> root_xform = std::nullopt;
        if (ZImpl(has_input2<PrimitiveObject>("xform"))) {
            root_xform = get_xform_from_prim(get_input_PrimitiveObject("xform"));
        }
        auto new_scene_tree = scene_tree->root_rename(new_root_name, root_xform);
        set_output("scene", new_scene_tree);
    }
};

ZENDEFNODE( SceneRootRename2, {
    {
        {gParamType_Scene, "scene"},
        {gParamType_String, "new_root_name", "new_scene"},
        {gParamType_Primitive, "xform"},
    },
    {
        {gParamType_Scene, "scene"}
    },
    {},
    {
        "Scene"
    }
});



struct RenderScene : zeno::INode {
    std::shared_ptr<ListObject> m_static_scene = nullptr;
    void apply() override {
        auto scene = std::make_shared<ListObject>();
        Json scene_descriptor_json;
        container_elem_update_info output_updateinfo;
        output_updateinfo.container_key = zsString2Std(this->uuid());

        if (has_input("static_scene")) {
            if (!m_static_scene) {
                auto static_scene_tree = get_scene_tree_from_list(get_input_ListObject("static_scene"));
                auto static_updateinfo = get_input_container_info("static_scene");
                output_updateinfo.merge(static_updateinfo);

                auto new_static_scene_tree = static_scene_tree->root_rename("SRG", std::nullopt);
                auto static_scene = get_input2_bool("flatten_static_scene")? new_static_scene_tree->to_flatten_structure(output_updateinfo, true) : new_static_scene_tree->to_layer_structure(output_updateinfo, true);
                for (auto i = 1; i < static_scene->m_impl->m_objects.size() - 2; i++) {
                    scene->push_back(static_scene->m_impl->m_objects[i]);
                }
                m_static_scene = static_scene;
            }
            auto scene_str = m_static_scene->m_impl->m_objects[m_static_scene->m_impl->m_objects.size() - 2]->userData()->get_string("Scene");
            auto static_scene_descriptor = Json::parse(zsString2Std(scene_str));
            scene_descriptor_json["StaticRenderGroups"] = static_scene_descriptor["StaticRenderGroups"];
            scene_descriptor_json["BasicRenderInstances"].update(static_scene_descriptor["BasicRenderInstances"]);
        }
        if (has_input("dynamic_scene")) {
            auto dynamic_scene_tree = get_scene_tree_from_list(get_input_ListObject("dynamic_scene"));
            auto dynamic_updateinfo = get_input_container_info("dynamic_scene");
            output_updateinfo.merge(dynamic_updateinfo);

            auto new_dynamic_scene_tree = dynamic_scene_tree->root_rename("DRG", std::nullopt);
            auto dynamic_scene = get_input2_bool("flatten_dynamic_scene") ? new_dynamic_scene_tree->to_flatten_structure(output_updateinfo, false) : new_dynamic_scene_tree->to_layer_structure(output_updateinfo, false);
            for (auto i = 1; i < dynamic_scene->m_impl->m_objects.size() - 2; i++) {
                scene->push_back(dynamic_scene->m_impl->m_objects[i]);
            }
            auto scene_str = dynamic_scene->m_impl->m_objects[dynamic_scene->m_impl->m_objects.size() - 2]->userData()->get_string("Scene");
            auto dynamic_scene_descriptor = Json::parse(zsString2Std(scene_str));
            scene_descriptor_json["DynamicRenderGroups"] = dynamic_scene_descriptor["DynamicRenderGroups"];
            scene_descriptor_json["BasicRenderInstances"].update(dynamic_scene_descriptor["BasicRenderInstances"]);
        }
        {
            auto scene_descriptor = std::make_shared<PrimitiveObject>();
            auto ud = scene_descriptor->userData();
            ud->set_string("ResourceType", "SceneDescriptor");
            ud->set_string("Scene", stdString2zs(std::string(scene_descriptor_json.dump())));

            std::string objkey = output_updateinfo.container_key + "\\SceneDescriptor";
            scene_descriptor->update_key(stdString2zs(objkey));
            output_updateinfo.new_added.insert(objkey);

            scene->push_back(scene_descriptor);
        }

        set_output_container_info("scene", output_updateinfo);
        set_output("scene", scene);
    }
};

ZENDEFNODE( RenderScene, {
    {
        {gParamType_List, "static_scene"},
        {gParamType_Bool, "flatten_static_scene", "1"},
        {gParamType_List, "dynamic_scene"},
        {gParamType_Bool, "flatten_dynamic_scene", "1"},
    },
    {
        {gParamType_List, "scene"},
    },
    {},
    {
        "Scene",
    },
});


struct RenderScene2 : zeno::INode {
    Json m_static_scene_descriptor;

    void apply() override {
        auto scene = std::make_shared<ListObject>();
        Json scene_descriptor_json;
        container_elem_update_info output_updateinfo;
        output_updateinfo.container_key = zsString2Std(this->uuid());

        if (has_input("Static Scene")) {
            auto static_scene_tree = std::dynamic_pointer_cast<SceneObject>(get_input("Static Scene"));
            if (static_scene_tree) {
                //auto static_updateinfo = get_input_container_info("static_scene");
                //output_updateinfo.merge(static_updateinfo);

                auto new_static_scene_tree = static_scene_tree->root_rename("SRG", std::nullopt);
                auto static_scene = get_input2_bool("flatten_static_scene") ? new_static_scene_tree->to_flatten_structure(output_updateinfo, true) : new_static_scene_tree->to_layer_structure(output_updateinfo, true);
                for (auto i = 1; i < static_scene->m_impl->m_objects.size() - 2; i++) {
                    scene->push_back(static_scene->m_impl->m_objects[i]);
                }

                auto scene_str = static_scene->m_impl->m_objects[static_scene->m_impl->m_objects.size() - 2]->userData()->get_string("Scene");
                m_static_scene_descriptor = Json::parse(zsString2Std(scene_str));
            }
            scene_descriptor_json["StaticRenderGroups"] = m_static_scene_descriptor["StaticRenderGroups"];
            scene_descriptor_json["BasicRenderInstances"].update(m_static_scene_descriptor["BasicRenderInstances"]);
        }
        if (has_input("Dynamic Scene")) {
            auto dynamic_scene_tree = std::dynamic_pointer_cast<SceneObject>(get_input("Dynamic Scene"));
            //auto dynamic_updateinfo = get_input_container_info("dynamic_scene");
            //output_updateinfo.merge(dynamic_updateinfo);

            auto new_dynamic_scene_tree = dynamic_scene_tree->root_rename("DRG", std::nullopt);
            auto dynamic_scene = get_input2_bool("flatten_dynamic_scene") ? new_dynamic_scene_tree->to_flatten_structure(output_updateinfo, false) : new_dynamic_scene_tree->to_layer_structure(output_updateinfo, false);
            for (auto i = 1; i < dynamic_scene->m_impl->m_objects.size() - 2; i++) {
                scene->push_back(dynamic_scene->m_impl->m_objects[i]);
            }
            auto scene_str = dynamic_scene->m_impl->m_objects[dynamic_scene->m_impl->m_objects.size() - 2]->userData()->get_string("Scene");
            auto dynamic_scene_descriptor = Json::parse(zsString2Std(scene_str));
            scene_descriptor_json["DynamicRenderGroups"] = dynamic_scene_descriptor["DynamicRenderGroups"];
            scene_descriptor_json["BasicRenderInstances"].update(dynamic_scene_descriptor["BasicRenderInstances"]);
        }
        {
            auto scene_descriptor = std::make_shared<PrimitiveObject>();
            auto ud = scene_descriptor->userData();
            ud->set_string("ResourceType", "SceneDescriptor");
            ud->set_string("Scene", stdString2zs(std::string(scene_descriptor_json.dump())));

            std::string objkey = output_updateinfo.container_key + "\\SceneDescriptor";
            scene_descriptor->update_key(stdString2zs(objkey));
            //output_updateinfo.new_added.insert(objkey);

            scene->push_back(scene_descriptor);
        }

        set_output("scene", scene);
    }
};

ZENDEFNODE( RenderScene2, {
    {
        {gParamType_Scene, "Static Scene"},
        {gParamType_Bool, "flatten_static_scene", "1"},
        {gParamType_Scene, "Dynamic Scene"},
        {gParamType_Bool, "flatten_dynamic_scene", "1"},
    },
    {
        {gParamType_List, "scene"}
    },
    {},
    {
        "Scene",
    },
});


struct MakeXform : zeno::INode {
    void apply() override {
        auto translate = ZImpl(get_input2<vec3f>("translate"));
        auto eulerXYZ = ZImpl(get_input2<vec3f>("eulerXYZ"));
        auto scale = ZImpl(get_input2<vec3f>("scale"));
        glm::mat4 matScale  = glm::scale( glm::vec3(scale[0], scale[1], scale[2] ));


        auto order = ZImpl(get_input2<std::string>("EulerRotationOrder"));
        auto orderTyped = magic_enum::enum_cast<EulerAngle::RotationOrder>(order).value_or(EulerAngle::RotationOrder::YXZ);

        auto measure = ZImpl(get_input2<std::string>("EulerAngleMeasure"));
        auto measureTyped = magic_enum::enum_cast<EulerAngle::Measure>(measure).value_or(EulerAngle::Measure::Radians);

        glm::vec3 eularAngleXYZ = glm::vec3(eulerXYZ[0], eulerXYZ[1], eulerXYZ[2]);
        glm::mat4 matRotate = EulerAngle::rotate(orderTyped, measureTyped, eularAngleXYZ);

        auto mat = matRotate * matScale;

        auto xform = std::make_shared<PrimitiveObject>();
        xform->resize(1);
        xform->verts[0] = translate;
        xform->verts.add_attr<vec3f>("r0")[0] = {mat[0][0], mat[0][1], mat[0][2]};
        xform->verts.add_attr<vec3f>("r1")[0] = {mat[1][0], mat[1][1], mat[1][2]};
        xform->verts.add_attr<vec3f>("r2")[0] = {mat[2][0], mat[2][1], mat[2][2]};
        set_output("xform", xform);
    }
};

ZENDEFNODE( MakeXform, {
    {
        {gParamType_Vec3f , "translate", "0, 0, 0"},
        {gParamType_Vec3f, "eulerXYZ", "0, 0, 0"},
        {gParamType_Vec3f, "scale", "1, 1, 1"},
    },
    {
        {gParamType_Primitive, "xform"},
    },
    {
        {"enum " + EulerAngle::RotationOrderListString(), "EulerRotationOrder", "ZYX"},
        {"enum " + EulerAngle::MeasureListString(), "EulerAngleMeasure", "Degree"}
    },
    {
        "Scene",
    },
});


}
