#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/common.h>
#include <iobject2.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/UserData.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace zeno {

struct SceneTreeNode {
    std::string matrix;
    int visibility = 1;
    std::vector<std::string> children;
    std::vector<std::string> meshes;
};

struct ZENO_API SceneObject : IObject2 {
    std::string root_name = "/ABC";
    bool bNeedUpdateDescriptor = false;
    bool bResetOptixScene = false;
    std::string scene_type = "static";

    std::unordered_map<std::string, SceneTreeNode> scene_tree;
    std::unordered_map<std::string, std::vector<glm::mat4>> node_to_matrix;
    std::unordered_map<std::string, std::vector<int>> node_to_id;
    std::unordered_map<std::string, std::unique_ptr<GeometryObject>> geom_list;

    SceneObject();
    SceneObject(const SceneObject& rhs);
    SceneObject& operator=(const SceneObject&) = delete;

    ZObjectType type() const override;
    IObject2* clone() const override;
    size_t key(char* buf, size_t buf_size) const override;
    void update_key(const char* key) override;
    size_t serialize_json(char* buf, size_t buf_size) const override;
    IUserData2* userData() override;
    void Delete() override;

    void from_json(const std::string& json_str);
    std::string to_json() const;
    std::unique_ptr<ListObject> to_structure() const;
    void flatten();
    std::unique_ptr<SceneObject> root_rename(const std::string& new_root_name, const std::vector<glm::mat4>& root_xform);

private:
    std::string m_key;
    UserData m_userDat;
};

ZENO_API void merge_scene2_into_scene1(SceneObject* main_object, SceneObject* second_object, std::string insert_path);

}  // namespace zeno
