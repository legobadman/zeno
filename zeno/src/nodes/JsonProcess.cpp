//
// Created by zh on 2023/11/14.
//

//����Python����ʵ����Ҫ�����ֶ�����json��
#ifdef ENABLE_LEGACY_ZENO_NODE

#include "zeno/types/DictObject.h"
#include "zeno/types/ListObject.h"
#include "zeno/utils/fileio.h"
#include "zeno/utils/log.h"
#include "zeno/utils/string.h"
#include <sstream>
#include <string>
#include <tinygltf/json.hpp>
#include <zeno/zeno.h>

using Json = nlohmann::ordered_json;

namespace zeno {
struct JsonObject : IObjectClone<JsonObject> {
    Json json;
};
struct ReadJson : zeno::INode {
    virtual void apply() override {
        auto json = std::make_shared<JsonObject>();
        auto path = ZImpl(get_input2<std::string>("path");
        std::string native_path = std::filesystem::u8path(path).string();
        auto content = zeno::file_get_content(native_path);
        json->json = Json::parse(content);
        ZImpl(set_output("json", json);
    }
};
ZENDEFNODE(ReadJson, {
    {
        {gParamType_String, "path", "", zeno::Socket_Primitve, zeno::ReadPathEdit},
    },
    {
        "json",
    },
    {},
    {
        "json"
    },
});

struct WriteJson : zeno::INode {
    virtual void apply() override {
        auto _json = ZImpl(get_input2<JsonObject>("json");
        auto path = ZImpl(get_input2<std::string>("path");
        path = create_directories_when_write_file(path);
        file_put_content(path, _json->json.dump());
    }
};
ZENDEFNODE(WriteJson, {
    {
        "json",
        {"writepath", "path"},
    },
    {
    },
    {},
    {
        "json"
    },
});
static Json iobject_to_json(std::shared_ptr<IObject> iObject) {
    Json json;
    if (objectIsRawLiterial<int>(iObject)) {
        json = objectToLiterial<int>(iObject);
    }
    else if (objectIsRawLiterial<vec2i>(iObject)) {
        auto value = objectToLiterial<vec2i>(iObject);
        json = { value[0], value[1]};
    }
    else if (objectIsRawLiterial<vec3i>(iObject)) {
        auto value = objectToLiterial<vec3i>(iObject);
        json = { value[0], value[1], value[2]};
    }
    else if (objectIsRawLiterial<vec4i>(iObject)) {
        auto value = objectToLiterial<vec4i>(iObject);
        json = { value[0], value[1], value[2], value[3]};
    }
    else if (objectIsRawLiterial<float>(iObject)) {
        json = objectToLiterial<float>(iObject);
    }
    else if (objectIsRawLiterial<vec2f>(iObject)) {
        auto value = objectToLiterial<vec2f>(iObject);
        json = { value[0], value[1]};
    }
    else if (objectIsRawLiterial<vec3f>(iObject)) {
        auto value = objectToLiterial<vec3f>(iObject);
        json = { value[0], value[1], value[2]};
    }
    else if (objectIsRawLiterial<vec4f>(iObject)) {
        auto value = objectToLiterial<vec4f>(iObject);
        json = { value[0], value[1], value[2], value[3]};
    }
    else if (objectIsRawLiterial<std::string>(iObject)) {
        json = objectToLiterial<std::string>(iObject);
    }
    else if (auto list = std::dynamic_pointer_cast<ListObject>(iObject)) {
        for (auto iObj: list->m_impl->get()) {
            json.push_back(iobject_to_json(iObj));
        }
    }
    else if (auto dict = std::dynamic_pointer_cast<DictObject>(iObject)) {
        for (auto [key, iObj]: dict->lut) {
            json[key] = iobject_to_json(iObj);
        }
    }
    else if (auto sub_json = std::dynamic_pointer_cast<JsonObject>(iObject)) {
        json = sub_json->json;
    }
    return std::move(json);
}
struct FormJson : zeno::INode {
  virtual void apply() override {
      auto _json = std::make_shared<JsonObject>();
      auto iObject = ZImpl(get_input("iObject");
      _json->json = iobject_to_json(iObject);
      ZImpl(set_output2("json", _json);
  }
};
ZENDEFNODE(FormJson, {
     {
         {"object", "iObject"},
     },
     {
         "json",
     },
     {},
     {
         "json"
     },
 });

struct JsonToString : zeno::INode {
  virtual void apply() override {
    auto json = ZImpl(get_input<JsonObject>("json");
    ZImpl(set_output2("out", json->json.dump());
  }
};
ZENDEFNODE(JsonToString, {
     {
         "json",
     },
     {
         {gParamType_String,"out"}
     },
     {},
     {
         "json"
     },
 });
struct JsonSetDataSimple : zeno::INode {
    virtual void apply() override {
        auto in_json = std::make_shared<JsonObject>();
        if (ZImpl(has_input<JsonObject>("json")) {
            in_json = ZImpl(get_input<JsonObject>("json");
        }

        auto path = ZImpl(get_input2<std::string>("path");
        auto names = split_str(path, '/');
        Json *tmp_json = &in_json->json;
        for (auto & name : names) {
            if (tmp_json->is_array()) {
                tmp_json = &tmp_json->operator[](std::stoi(name));
            }
            else {
                tmp_json = &tmp_json->operator[](name);
            }
        }
        auto value = ZImpl(get_input("value");
        *tmp_json = iobject_to_json(value);

        ZImpl(set_output2("json", in_json);
    }
};

ZENDEFNODE(JsonSetDataSimple, {
    {
        {"json"},
        {gParamType_String, "path"},
        "value",
    },
    {
        {"json"},
    },
    {},
    {
        "json"
    },
});


struct JsonSetData : zeno::INode {
    virtual void apply() override {
        auto in_json = std::make_shared<JsonObject>();
        if (ZImpl(has_input<JsonObject>("json")) {
            in_json = ZImpl(get_input<JsonObject>("json");
        }
        auto multi_path = ZImpl(get_input2<std::string>("paths");
        std::istringstream iss(multi_path);
        std::vector<std::string> paths;
        std::string line;
        while (std::getline(iss, line)) {
            line = zeno::trim_string(line);
            if (line.size()) {
                paths.push_back(line);
            }
        }
        auto dict = ZImpl(get_input<DictObject>("dict");
        for (auto &path: paths) {
            auto strings = zeno::split_str(path, ':');
            auto names = split_str(strings[1], '/');

            Json *tmp_json = &in_json->json;
            for (auto & name : names) {
                if (tmp_json->is_array()) {
                    tmp_json = &tmp_json->operator[](std::stoi(name));
                }
                else {
                    tmp_json = &tmp_json->operator[](name);
                }
            }
            std::string new_name = zeno::trim_string(strings[0]);
            *tmp_json = iobject_to_json(dict->lut[new_name]);
        }

        ZImpl(set_output2("json", in_json);
    }
};

ZENDEFNODE(JsonSetData, {
    {
        {"json"},
        {gParamType_String, "paths", "input_name:json_path"},
        {"dict", "dict"},
    },
    {
        {"json"},
    },
    {},
    {
        "json"
    },
});

struct ReadJsonFromString : zeno::INode {
    virtual void apply() override {
        auto json = std::make_shared<JsonObject>();
        auto content = ZImpl(get_input2<std::string>("content");
        json->json = Json::parse(content);
        ZImpl(set_output("json", json);
    }
};
ZENDEFNODE(ReadJsonFromString, {
    {
        {gParamType_String, "content"},
    },
    {
        "json",
    },
    {},
    {
        "json"
    },
});
struct JsonGetArraySize : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        ZImpl(set_output("size", std::make_shared<NumericObject>((int)json->json.size()));
    }
};
ZENDEFNODE(JsonGetArraySize, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
    },
    {
        {gParamType_Int, "size"},
    },
    {},
    {
        "json"
    },
});
struct JsonGetArrayItem : zeno::INode {
    virtual void apply() override {
        auto out_json = std::make_shared<JsonObject>();
        auto json = ZImpl(get_input<JsonObject>("json");
        auto index = ZImpl(get_input2<int>("index");
        out_json->json = json->json[index];
        ZImpl(set_output("json", out_json);
    }
};
ZENDEFNODE(JsonGetArrayItem, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
        {gParamType_Int, "index"}
    },
    {
        "json",
    },
    {},
    {
        "json"
    },
});

struct JsonGetChild : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        auto name = ZImpl(get_input2<std::string>("name");
        auto type = ZImpl(get_input2<std::string>("type");
        if (type == "json") {
            auto out_json = std::make_shared<JsonObject>();
            out_json->json = json->json[name];
            ZImpl(set_output("out", out_json);
        }
        else if (type == "int") {
            ZImpl(set_output2("out", int(json->json[name]));
        }
        else if (type == "float") {
            ZImpl(set_output2("out", float(json->json[name]));
        }
        else if (type == "string") {
            ZImpl(set_output2("out", std::string(json->json[name]));
        }
        else if (type == "vec2f") {
            float x = float(json->json[name][0]);
            float y = float(json->json[name][1]);
            ZImpl(set_output2("out", vec2f(x, y));
        }
        else if (type == "vec3f") {
            float x = float(json->json[name][0]);
            float y = float(json->json[name][1]);
            float z = float(json->json[name][2]);
            ZImpl(set_output2("out", vec3f(x, y, z));
        }
        else if (type == "vec4f") {
            float x = float(json->json[name][0]);
            float y = float(json->json[name][1]);
            float z = float(json->json[name][2]);
            float w = float(json->json[name][3]);
            ZImpl(set_output2("out", vec4f(x, y, z, w));
        }
    }
};
ZENDEFNODE(JsonGetChild, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
        {gParamType_String, "name"},
        {"enum json int float string vec2f vec3f vec4f", "type"},
    },
    {
        {gParamType_Vec4f,"out"},
    },
    {},
    {
        "deprecated"
    },
});
struct JsonGetInt : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        ZImpl(set_output2("value", int(json->json));
    }
};
ZENDEFNODE(JsonGetInt, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
    },
    {
        "value",
    },
    {},
    {
        "deprecated"
    },
});

struct JsonGetFloat : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        ZImpl(set_output2("value", float(json->json));
    }
};
ZENDEFNODE(JsonGetFloat, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
    },
    {
        "value",
    },
    {},
    {
        "deprecated"
    },
});

struct JsonGetString : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        ZImpl(set_output2("string", std::string(json->json));
    }
};
ZENDEFNODE(JsonGetString, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
    },
    {
        "string",
    },
    {},
    {
        "deprecated"
    },
});

struct JsonGetKeys : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json"));
        auto list = create_ListObject();
        for (auto& [key, _] : json->json.items()) {
            list->arr.emplace_back(std::make_shared<zeno::StringObject>(key));
        }
        ZImpl(set_output2("keys", list);
    }
};
ZENDEFNODE(JsonGetKeys, {
    {
        {"json"},
    },
    {
        "keys",
    },
    {},
    {
        "json"
    },
});
struct JsonGetTypeName : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        ZImpl(set_output2("string", std::string(json->json.type_name()));
    }
};
ZENDEFNODE(JsonGetTypeName, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
    },
    {
        "string",
    },
    {},
    {
        "deprecated"
    },
});

struct JsonData : zeno::INode {
    virtual void apply() override {
        auto json = ZImpl(get_input<JsonObject>("json");
        auto path = ZImpl(get_input2<std::string>("path");
        auto strings = zeno::split_str(path, ':');
        auto type = strings[1];
        path = strings[0];
        auto names = split_str(path, '/');

        for (auto & name : names) {
            json->json = json->json[name];
        }


        if (type == "json") {
            auto out_json = std::make_shared<JsonObject>();
            out_json->json = json->json;
            ZImpl(set_output("out", out_json);
        }
        else if (type == "int") {
            ZImpl(set_output2("out", int(json->json));
        }
        else if (type == "float") {
            ZImpl(set_output2("out", float(json->json));
        }
        else if (type == "string") {
            ZImpl(set_output2("out", std::string(json->json));
        }
        else if (type == "vec2f") {
            float x = float(json->json["x"]);
            float y = float(json->json["y"]);
            ZImpl(set_output2("out", vec2f(x, y));
        }
        else if (type == "vec3f") {
            float x = float(json->json["x"]);
            float y = float(json->json["y"]);
            float z = float(json->json["z"]);
            ZImpl(set_output2("out", vec3f(x, y, z));
        }
        else if (type == "vec4f") {
            float x = float(json->json["x"]);
            float y = float(json->json["y"]);
            float z = float(json->json["z"]);
            float w = float(json->json["w"]);
            ZImpl(set_output2("out", vec4f(x, y, z, w));
        }
        else if (type == "vec2i") {
            auto x = int(json->json["x"]);
            auto y = int(json->json["y"]);
            ZImpl(set_output2("out", vec2i(x, y));
        }
        else if (type == "vec3i") {
            auto x = int(json->json["x"]);
            auto y = int(json->json["y"]);
            auto z = int(json->json["z"]);
            ZImpl(set_output2("out", vec3i(x, y, z));
        }
        else if (type == "vec4i") {
            auto x = int(json->json["x"]);
            auto y = int(json->json["y"]);
            auto z = int(json->json["z"]);
            auto w = int(json->json["w"]);
            ZImpl(set_output2("out", vec4i(x, y, z, w));
        }
    }
};
ZENDEFNODE(JsonData, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
        {gParamType_String, "path"},
    },
    {
        "out",
    },
    {},
    {
        "deprecated"
    },
});

struct JsonGetData : zeno::INode {
    virtual void apply() override {
        auto in_json = ZImpl(get_input<JsonObject>("json");
        auto multi_path = ZImpl(get_input2<std::string>("paths");
        std::istringstream iss(multi_path);
        std::vector<std::string> paths;
        std::string line;
        while (std::getline(iss, line)) {
            line = zeno::trim_string(line);
            if (line.size()) {
                paths.push_back(line);
            }
        }

        auto dict = std::make_shared<zeno::DictObject>();
        for (auto &path: paths) {
            auto json = std::make_shared<JsonObject>();
            json->json = in_json->json;
            auto strings = zeno::split_str(path, ':');
            auto type = strings[1];
            path = strings[0];
            std::string new_name = path;
            if (strings.size() == 3) {
                new_name = zeno::trim_string(strings[2]);
            }

            auto names = split_str(path, '/');

            for (auto & name : names) {
                if (json->json.is_array()) {
                    json->json = json->json[std::stoi(name)];
                }
                else {
                    json->json = json->json[name];
                }
            }

            if (type == "json") {
                auto out_json = std::make_shared<JsonObject>();
                out_json->json = json->json;
                dict->lut[new_name] = out_json;
            }
            else if (type == "int") {
                dict->lut[new_name] = std::make_shared<NumericObject>(int(json->json));
            }
            else if (type == "float") {
                dict->lut[new_name] = std::make_shared<NumericObject>(float(json->json));
            }
            else if (type == "string") {
                dict->lut[new_name] = std::make_shared<StringObject>(std::string(json->json));
            }
            else if (type == "vec2f") {
                if (json->json.is_array()) {
                    float x = float(json->json[0]);
                    float y = float(json->json[1]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec2f(x, y));
                }
                else {
                    float x = float(json->json["x"]);
                    float y = float(json->json["y"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec2f(x, y));
                }
            }
            else if (type == "vec3f") {
                if (json->json.is_array()) {
                    float x = float(json->json[0]);
                    float y = float(json->json[1]);
                    float z = float(json->json[2]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec3f(x, y, z));
                }
                else {
                    float x = float(json->json["x"]);
                    float y = float(json->json["y"]);
                    float z = float(json->json["z"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec3f(x, y, z));
                }
            }
            else if (type == "vec4f") {
                if (json->json.is_array()) {
                    float x = float(json->json[0]);
                    float y = float(json->json[1]);
                    float z = float(json->json[2]);
                    float w = float(json->json[3]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec4f(x, y, z, w));
                }
                else {
                    float x = float(json->json["x"]);
                    float y = float(json->json["y"]);
                    float z = float(json->json["z"]);
                    float w = float(json->json["w"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec4f(x, y, z, w));
                }
            }
            else if (type == "vec2i") {
                if (json->json.is_array()) {
                    auto x = int(json->json[0]);
                    auto y = int(json->json[1]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec2i(x, y));
                }
                else {
                    auto x = int(json->json["x"]);
                    auto y = int(json->json["y"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec2i(x, y));
                }
            }
            else if (type == "vec3i") {
                if (json->json.is_array()) {
                    auto x = int(json->json[0]);
                    auto y = int(json->json[1]);
                    auto z = int(json->json[2]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec3i(x, y, z));
                }
                else {
                    auto x = int(json->json["x"]);
                    auto y = int(json->json["y"]);
                    auto z = int(json->json["z"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec3i(x, y, z));
                }
            }
            else if (type == "vec4i") {
                if (json->json.is_array()) {
                    auto x = int(json->json[0]);
                    auto y = int(json->json[1]);
                    auto z = int(json->json[2]);
                    auto w = int(json->json[3]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec4i(x, y, z, w));
                }
                else {
                    auto x = int(json->json["x"]);
                    auto y = int(json->json["y"]);
                    auto z = int(json->json["z"]);
                    auto w = int(json->json["w"]);
                    dict->lut[new_name] = std::make_shared<NumericObject>(vec4i(x, y, z, w));
                }
            }
        }
        ZImpl(set_output("outs", dict);
    }
};
ZENDEFNODE(JsonGetData, {
    {
        {gParamType_, "json", "", zeno::Socket_ReadOnly},
        {gParamType_String, "paths", "json_path:vec3f:output_name"},
    },
    {
        {gParamType_Dict,"outs"}
    },
    {},
    {
        "json"
    },
});

}

#if 0
struct CreateRenderInstance : zeno::INode {
    virtual void apply() override {
        auto instID = ZImpl(get_input2<std::string>("instID");
        auto Geom = ZImpl(get_input2<std::string>("Geom");
        auto Matrix = ZImpl(get_input2<std::string>("Matrix");
        auto Material = ZImpl(get_input2<std::string>("Material");
        if (instID.empty()) {
            auto info = zeno::format("instID {} can not be empty!", instID);
            throw zeno::makeError(info);
        }

        auto out_json = std::make_shared<JsonObject>();
        out_json->json["BasicRenderInstances"][instID] = {
            {"Geom", Geom},
            {"Matrix", Matrix},
            {"Material", Material},
        };
        out_json->json["Root"] = instID;
        ZImpl(set_output("json", out_json);
    }
};

ZENDEFNODE( CreateRenderInstance, {
    {
        {"string", "instID", ""},
        {"string", "Geom", ""},
        {"string", "Matrix", "Identity"},
        {"string", "Material", "Default"},
    },
    {
        {"json"},
    },
    {},
    {
        "shader",
    },
});

struct RenderGroup : zeno::INode {
    virtual void apply() override {
        auto RenderGroupID = ZImpl(get_input2<std::string>("RenderGroupID");
        auto is_static = ZImpl(get_input2<bool>("static");
        auto Matrix_string = ZImpl(get_input2<std::string>("Matrixes");
        std::vector<std::string> Matrixes = zeno::split_str(Matrix_string, {' ', '\n'});
        auto items = ZImpl(get_input<ListObject>("items")->get<JsonObject>();

        std::set<std::string> rinst;
        std::map<std::string, int> id_checker;

        Json node = {};
        node["Objects"] = Json::array();
        for (const auto& item: items) {
            node["Objects"].push_back(item->json["Root"]);
        }
        node["Matrixes"] = Json::array();
        for (auto &matrix: Matrixes) {
            node["Matrixes"].push_back(matrix);
        }

        auto out_json = std::make_shared<JsonObject>();
        out_json->json["Root"] = RenderGroupID;

        for (const auto& item: items) {
            for (auto& [key, value] : item->json["BasicRenderInstances"].items()) {
                out_json->json["BasicRenderInstances"][key] = value;
                rinst.insert(key);
            }
            for (auto& [key, value] : item->json["DynamicRenderGroups"].items()) {
                out_json->json["DynamicRenderGroups"][key] = value;
                id_checker[key] += 1;
            }
            for (auto& [key, value] : item->json["StaticRenderGroups"].items()) {
                if (is_static) {
                    out_json->json["StaticRenderGroups"][key] = value;
                }
                else {
                    out_json->json["DynamicRenderGroups"][key] = value;
                }
                id_checker[key] += 1;
            }
        }

        if (is_static && !out_json->json.contains("DynamicRenderGroups")) {
            out_json->json["StaticRenderGroups"][RenderGroupID] = node;
        }
        else {
            out_json->json["DynamicRenderGroups"][RenderGroupID] = node;
        }
        id_checker[RenderGroupID] += 1;

        for (auto const &[GroupID, count]: id_checker) {
            if (count > 1) {
                auto info = zeno::format("Group ID {} is not unique!", GroupID);
                zeno::log_error(info);
                throw zeno::makeError(info);
            }
            if (rinst.count(GroupID)) {
                auto info = zeno::format("Group ID {} is not same with RenderInstance ID!", GroupID);
                zeno::log_error(info);
                throw zeno::makeError(info);
            }
        }

        ZImpl(set_output("json", out_json);
    }
};

ZENDEFNODE( RenderGroup, {
    {
        {"string", "RenderGroupID"},
        {"list", "items"},
        {"bool", "static", "1"},
        {"string", "Matrixes", "Identity"},
    },
    {
        {"json"},
    },
    {},
    {
        "shader",
    },
});
#endif

}
#endif