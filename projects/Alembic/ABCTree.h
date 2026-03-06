#pragma once

#include <tinygltf/json.hpp>
#include <iobject2.h>
#include <memory>
#include <cstring>
#include <Alembic/AbcGeom/Foundation.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Abc/ErrorHandler.h>

using Alembic::AbcGeom::ObjectVisibility;
using Json = nlohmann::json;

namespace zeno {

// Deleter for IGeometryObject so ownership stays in the creating module (ABI-safe).
struct ABCTreeGeomDeleter {
    void operator()(IGeometryObject* p) const {
        if (p) p->Delete();
    }
};
using ABCTreeUniqueGeom = std::unique_ptr<IGeometryObject, ABCTreeGeomDeleter>;

struct CameraInfo {
    double _far;
    double _near;
    double focal_length;
    double horizontalAperture;
    double verticalAperture;
};

struct ABCTree : IObject2 {
    std::string name;
    ABCTreeUniqueGeom prim;
    Alembic::Abc::M44d xform = Alembic::Abc::M44d();
    std::unique_ptr<CameraInfo> camera_info;
    std::vector<std::unique_ptr<ABCTree>> children;
    ObjectVisibility visible = ObjectVisibility::kVisibilityDeferred;
    std::string instanceSourcePath;

    std::string m_key;

    // IObject2 implementation -------------------------------------------------
    IObject2* clone() const override {
        auto* tree = new ABCTree();
        tree->name = name;
        if (prim) {
            IObject2* c = prim->clone();
            tree->prim.reset(dynamic_cast<IGeometryObject*>(c));
        }
        tree->xform = xform;
        if (camera_info) {
            tree->camera_info = std::make_unique<CameraInfo>(*camera_info);
        }
        tree->children.clear();
        tree->children.reserve(children.size());
        for (auto const& ch : children) {
            if (ch) {
                auto* ch_clone = dynamic_cast<ABCTree*>(ch->clone());
                tree->children.emplace_back(ch_clone);
            }
        }
        tree->visible = visible;
        tree->instanceSourcePath = instanceSourcePath;
        tree->m_key = m_key;
        return tree;
    }

    size_t key(char* buf, size_t buf_size) const override {
        const char* s = m_key.c_str();
        size_t len = m_key.size();   // not including '\0'
        if (buf && buf_size > 0) {
            size_t copy = (len < buf_size - 1) ? len : (buf_size - 1);
            std::memcpy(buf, s, copy);
            buf[copy] = '\0';
        }
        return len;
    }

    void update_key(const char* keyStr) override {
        m_key = keyStr ? keyStr : "";
    }

    size_t serialize_json(char* buf, size_t buf_size) const override {
        (void)buf;
        (void)buf_size;
        return 0;
    }

    IUserData2* userData() override {
        return nullptr;
    }

    void Delete() override {
        delete this;
    }

    ZObjectType type() const override {
        return ZObj_Dummy;
    }

    Json get_scene_info(
        ObjectVisibility parent_visible = ObjectVisibility::kVisibilityVisible
    ) {
        Json json;
        ObjectVisibility cur_visible = visible == ObjectVisibility::kVisibilityDeferred? parent_visible: visible;
        json["visibility"] = int(cur_visible);
        json["node_name"] = name;
        if (instanceSourcePath.size()) {
            json["instance_source_path"] = "/ABC" + instanceSourcePath;
        }
        if (prim) {
            json["mesh"] = "mesh";
        }
        auto r0 = Imath::V4d(1, 0, 0, 0) * xform;
        auto r1 = Imath::V4d(0, 1, 0, 0) * xform;
        auto r2 = Imath::V4d(0, 0, 1, 0) * xform;
        auto t  = Imath::V4d(0, 0, 0, 1) * xform;
        json["r0"] = {r0[0], r0[1], r0[2]};
        json["r1"] = {r1[0], r1[1], r1[2]};
        json["r2"] = {r2[0], r2[1], r2[2]};
        json["t"]  = {t[0], t[1], t[2]};
        json["children_name"] = Json::array();
        if (instanceSourcePath.empty()) {
            for (const auto &child: children) {
                auto cjson = child->get_scene_info(cur_visible);
                auto name = cjson["node_name"];
                json["children_name"].push_back(name);
                json[name] = cjson;
            }
        }
        return json;
    }

    template <class Func>
    bool visitPrims(Func const& func) const {
        if (prim) {
            if constexpr (std::is_void_v<std::invoke_result_t<Func, IGeometryObject*>>) {
                func(prim.get());
            } else {
                if (!func(prim.get()))
                    return false;
            }
        }
        for (auto const& ch : children) {
            if (!ch->visitPrims(func))
                return false;
        }
        return true;
    }
};

}
