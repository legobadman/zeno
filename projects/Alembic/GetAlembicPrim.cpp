#include "ABCCommon.h"
#include "ABCTree.h"
#include "api_zs_alembic.h"
#include <iobject2.h>
#include <zenum.h>
#include <zvec.h>
#include <zcommon.h>
#include <inodeimpl.h>
#include <inodedata.h>
#include <queue>
#include <utility>
#include <sstream>
#include <cmath>
#include <optional>


namespace zeno {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[4096] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static std::vector<std::string> split_str(const std::string& s, std::initializer_list<char> delims) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        bool is_delim = false;
        for (char d : delims) if (c == d) { is_delim = true; break; }
        if (is_delim) {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else cur += c;
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static zeno::Vec3f vec3f_normalize(zeno::Vec3f v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len <= 0.f) return v;
    return zeno::Vec3f(v.x / len, v.y / len, v.z / len);
}
static zeno::Vec3f vec3f_cross(zeno::Vec3f a, zeno::Vec3f b) {
    return zeno::Vec3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}
static float radians_to_degrees(float rad) {
    return rad * (180.f / 3.14159265f);
}

int count_alembic_prims(zeno::ABCTree* abctree) {
    int count = 0;
    abctree->visitPrims([&] (auto const &p) {
        count++;
    });
    return count;
}

struct CountAlembicPrims : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("CountAlembicPrims: need ABCTree"); return ZErr_ParamError; }
        nd->set_output_int("count", count_alembic_prims(abctree));
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CountAlembicPrims,
    Z_INPUTS({"abctree", _gParamType_IObject}),
    Z_OUTPUTS({"count", _gParamType_Int}),
    "alembic", "", "", "");

std::unique_ptr<IGeometryObject, ABCTreeGeomDeleter> get_alembic_prim(zeno::ABCTree* abctree, int index) {
    std::unique_ptr<IGeometryObject, ABCTreeGeomDeleter> geom;
    abctree->visitPrims([&](IGeometryObject* p) {
        if (index == 0) {
            IObject2* c = p->clone();
            geom.reset(dynamic_cast<IGeometryObject*>(c));
            return false;
        }
        index--;
        return true;
    });
    if (!geom) {
        throw;// Exception("index out of range in abctree");
    }
    return geom;
}

int get_alembic_prim_index(zeno::ABCTree* abctree, const std::string& name) {
    int index = 0;
    char buf[4096] = {};
    abctree->visitPrims([&](zeno::IGeometryObject* p) {
        IUserData2* ud = p->userData();
        if (ud) { ud->get_string("abcpath_0", "", buf, sizeof(buf)); }
        std::string _abc_path(buf);
        if (_abc_path == name) return false;
        index++;
        return true;
    });
    return index;
}
void dfs_abctree(
    ABCTree* root,
    int parent_index,
    std::vector<ABCTree*>& linear_abctrees,
    std::vector<int>& linear_abctree_parent
) {
    int self_index = linear_abctrees.size();
    linear_abctrees.push_back(root);
    linear_abctree_parent.push_back(parent_index);
    for (auto const &ch: root->children) {
        dfs_abctree(ch.get(), self_index, linear_abctrees, linear_abctree_parent);
    }
}

IListObject* get_xformed_prims_igeom(zeno::ABCTree* abctree) {
    IListObject* list = zeno::zs_alembic::createList();
    if (!list) return nullptr;
    std::vector<ABCTree*> linear_abctrees;
    std::vector<int> linear_abctree_parent;
    dfs_abctree(abctree, -1, linear_abctrees, linear_abctree_parent);
    std::vector<Alembic::Abc::M44d> transforms;
    for (size_t i = 0; i < linear_abctrees.size(); i++) {
        auto const* abc_node = linear_abctrees[i];
        int parent_index = linear_abctree_parent[i];
        if (parent_index >= 0)
            transforms.push_back(abc_node->xform * transforms[parent_index]);
        else
            transforms.push_back(abc_node->xform);
        if (abc_node->prim) {
            IGeometryObject* src = abc_node->prim.get();
            IObject2* c = src->clone();
            IGeometryObject* cloned = dynamic_cast<IGeometryObject*>(c);
            if (!cloned) { if (c) c->Delete(); continue; }
            int np = cloned->npoints();
            std::vector<zeno::Vec3f> pts((size_t)np);
            cloned->points_pos(pts.data(), (size_t)np);
            Alembic::Abc::M44d const& mat = transforms.back();
            for (int j = 0; j < np; j++) {
                Imath::V4d q(pts[(size_t)j][0], pts[(size_t)j][1], pts[(size_t)j][2], 1);
                q = q * mat;
                pts[(size_t)j] = zeno::Vec3f((float)q.x, (float)q.y, (float)q.z);
            }
            int nf = cloned->nfaces();
            std::vector<std::vector<int>> faces((size_t)nf);
            std::vector<int> buf(4);
            for (int f = 0; f < nf; f++) {
                size_t n = cloned->face_points(f, buf.data(), 4);
                faces[(size_t)f].assign(buf.data(), buf.data() + n);
            }
            cloned->Delete();
            IGeometryObject* xformed = zeno::zs_alembic::createGeometryByPointFace(
                zeno::Topo_IndiceMesh, true, pts, faces);
            if (xformed) list->push_back(xformed);
        }
    }
    return list;
}

std::unique_ptr<IGeometryObject, ABCTreeGeomDeleter> get_xformed_prim_igeom(zeno::ABCTree* abctree, int index) {
    IListObject* list = get_xformed_prims_igeom(abctree);
    if (!list || index < 0 || (size_t)index >= list->size()) {
        if (list) list->Delete();
        return nullptr;
    }
    IObject2* c = list->get(index)->clone();
    list->Delete();
    return std::unique_ptr<IGeometryObject, ABCTreeGeomDeleter>(dynamic_cast<IGeometryObject*>(c));
}

struct GetAlembicPrim : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("GetAlembicPrim: need ABCTree"); return ZErr_ParamError; }
        int index = nd->get_input2_int("index");
        bool use_xform = nd->get_input2_bool("use_xform");
        if (nd->get_input2_bool("use_name"))
            index = get_alembic_prim_index(abctree, get_input2_string(nd, "name"));
        if (use_xform) {
            auto geom = get_xformed_prim_igeom(abctree, index);
            if (!geom) { nd->report_error("GetAlembicPrim: index out of range"); return ZErr_ParamError; }
            if (nd->get_input2_bool("flipFrontBack")) {
                auto* g = zeno::zs_alembic::primFlipFaces(geom.get(), true);
                if (g != geom.get()) geom.reset(g);
            }
            if (nd->get_input2_bool("triangulate")) {
                auto* g = zeno::zs_alembic::primTriangulate(geom.get());
                if (g != geom.get()) geom.reset(g);
            }
            nd->set_output_object("prim", geom.release());
        } else {
            auto geom = get_alembic_prim(abctree, index);
            if (nd->get_input2_bool("flipFrontBack")) {
                auto* g = zeno::zs_alembic::primFlipFaces(geom.get(), true);
                if (g != geom.get()) geom.reset(g);
            }
            if (nd->get_input2_bool("triangulate")) {
                auto* g = zeno::zs_alembic::primTriangulate(geom.get());
                if (g != geom.get()) geom.reset(g);
            }
            nd->set_output_object("prim", geom.release());
        }
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(GetAlembicPrim,
    Z_INPUTS(
        {"abctree", _gParamType_IObject},
        {"index", _gParamType_Int, ZInt(0)},
        {"use_xform", _gParamType_Bool, ZInt(0)},
        {"triangulate", _gParamType_Bool, ZInt(0)},
        {"use_name", _gParamType_Bool, ZInt(0)},
        {"name", _gParamType_String, ZString("")},
        {"flipFrontBack", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS({"prim", _gParamType_Geometry}),
    "alembic", "", "", "");

struct AllAlembicPrim : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("AllAlembicPrim: need ABCTree"); return ZErr_ParamError; }
        std::vector<zeno::IGeometryObject*> geomlst;
        if (nd->get_input2_bool("use_xform")) {
            IListObject* list = get_xformed_prims_igeom(abctree);
            if (list) {
                for (size_t i = 0; i < list->size(); i++)
                    geomlst.push_back(dynamic_cast<zeno::IGeometryObject*>(list->get(i)));
                list->Delete();
            }
        } else {
            abctree->visitPrims([&](zeno::IGeometryObject* p) { geomlst.push_back(p); });
        }
        zeno::IGeometryObject* merged = zeno::zs_alembic::PrimMerge(geomlst);
        if (!merged) { nd->report_error("AllAlembicPrim: merge failed"); return ZErr_ParamError; }
        if (nd->get_input2_bool("flipFrontBack")) zeno::zs_alembic::primFlipFaces(merged, true);
        if (nd->get_input2_int("triangulate") == 1) zeno::zs_alembic::primTriangulate(merged);
        nd->set_output_object("prim", merged);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(AllAlembicPrim,
    Z_INPUTS(
        {"abctree", _gParamType_IObject},
        {"use_xform", _gParamType_Bool, ZInt(0)},
        {"triangulate", _gParamType_Bool, ZInt(0)},
        {"flipFrontBack", _gParamType_Bool, ZInt(1)}
    ),
    Z_OUTPUTS({"prim", _gParamType_Geometry}),
    "alembic", "", "", "");

struct AlembicPrimList : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("AlembicPrimList: need ABCTree"); return ZErr_ParamError; }
        std::vector<zeno::IGeometryObject*> geoms;
        bool use_xform = nd->get_input2_bool("use_xform");
        if (use_xform) {
            IListObject* list = get_xformed_prims_igeom(abctree);
            if (list) {
                for (size_t i = 0; i < list->size(); i++)
                    geoms.push_back(dynamic_cast<zeno::IGeometryObject*>(list->get(i)->clone()));
                list->Delete();
            }
        } else {
            abctree->visitPrims([&](zeno::IGeometryObject* p) { geoms.push_back(p); });
        }
        bool bSplitByFaceset = nd->get_input2_bool("splitByFaceset");
        std::vector<zeno::IGeometryObject*> expanded;
        if (bSplitByFaceset) {
            for (zeno::IGeometryObject* geom : geoms) {
                IListObject* sub = zeno::zs_alembic::abc_split_by_name(geom, false);
                if (sub) {
                    for (size_t i = 0; i < sub->size(); i++)
                        expanded.push_back(dynamic_cast<zeno::IGeometryObject*>(sub->get(i)->clone()));
                    sub->Delete();
                }
            }
            if (use_xform) { for (auto* g : geoms) g->Delete(); }
        } else {
            if (use_xform) {
                expanded = std::move(geoms);
            } else {
                for (zeno::IGeometryObject* geom : geoms)
                    expanded.push_back(dynamic_cast<zeno::IGeometryObject*>(geom->clone()));
            }
        }
        auto pathInclude = split_str(get_input2_string(nd, "pathInclude"), {' ', '\n'});
        auto pathExclude = split_str(get_input2_string(nd, "pathExclude"), {' ', '\n'});
        auto facesetInclude = split_str(get_input2_string(nd, "facesetInclude"), {' ', '\n'});
        auto facesetExclude = split_str(get_input2_string(nd, "facesetExclude"), {' ', '\n'});
        std::vector<zeno::IGeometryObject*> filtered;
        for (zeno::IGeometryObject* geom : expanded) {
            IUserData2* ud = geom->userData();
            if (!ud) { filtered.push_back(geom); continue; }
            char buf[4096] = {};
            ud->get_string("abcpath_0", "", buf, sizeof(buf));
            std::string abc_path(buf);
            bool contain = pathInclude.empty();
            if (!contain)
                for (const auto& p : pathInclude) { if (starts_with(abc_path, p)) { contain = true; break; } }
            if (contain)
                for (const auto& p : pathExclude) { if (starts_with(abc_path, p)) { contain = false; break; } }
            if (contain && ud->has_string("faceset_0")) {
                ud->get_string("faceset_0", "", buf, sizeof(buf));
                std::string faceset(buf);
                contain = facesetInclude.empty();
                if (!contain)
                    for (const auto& p : facesetInclude) { if (starts_with(faceset, p)) { contain = true; break; } }
                if (contain)
                    for (const auto& p : facesetExclude) { if (starts_with(faceset, p)) { contain = false; break; } }
            }
            if (contain) filtered.push_back(geom);
            else geom->Delete();
        }
        IListObject* out_list = zeno::zs_alembic::createList();
        if (!out_list) { for (auto* g : filtered) g->Delete(); return ZErr_ParamError; }
        for (size_t i = 0; i < filtered.size(); i++) {
            zeno::IGeometryObject* geom = filtered[i];
            if (nd->get_input2_bool("flipFrontBack")) {
                auto* g = zeno::zs_alembic::primFlipFaces(geom, true);
                if (g != geom) { geom->Delete(); filtered[i] = geom = g; }
            }
            if (nd->get_input2_bool("triangulate")) {
                auto* g = zeno::zs_alembic::primTriangulate(geom);
                if (g != geom) { geom->Delete(); filtered[i] = geom = g; }
            }
            IUserData2* ud = geom->userData();
            if (ud) {
                char buf[4096] = {};
                ud->get_string("abcpath_0", "", buf, sizeof(buf));
                std::string abcpath_0(buf);
                abcpath_0 += "/mesh";
                ud->set_string("abcpath_0", abcpath_0.c_str());
            }
            out_list->push_back(filtered[i]);
        }
        nd->set_output_object("geoms", out_list);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(AlembicPrimList,
    Z_INPUTS(
        {"abctree", _gParamType_IObject},
        {"use_xform", _gParamType_Bool, ZInt(0)},
        {"triangulate", _gParamType_Bool, ZInt(0)},
        {"splitByFaceset", _gParamType_Bool, ZInt(0)},
        {"killDeadVerts", _gParamType_Bool, ZInt(1)},
        {"flipFrontBack", _gParamType_Bool, ZInt(1)},
        {"pathInclude", _gParamType_String, ZString("")},
        {"pathExclude", _gParamType_String, ZString("")},
        {"facesetInclude", _gParamType_String, ZString("")},
        {"facesetExclude", _gParamType_String, ZString("")}
    ),
    Z_OUTPUTS({"geoms", _gParamType_List}),
    "alembic", "", "", "");

struct AlembicSceneInfo : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("AlembicSceneInfo: need ABCTree"); return ZErr_ParamError; }
        Json j = abctree->get_scene_info();
        nd->set_output_string("json", j.dump().c_str());
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(AlembicSceneInfo,
    Z_INPUTS({"abctree", _gParamType_IObject}),
    Z_OUTPUTS({"json", _gParamType_String}),
    "alembic", "", "", "");

struct GetAlembicCamera : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("abctree");
        auto* abctree = dynamic_cast<ABCTree*>(obj);
        if (!abctree) { nd->report_error("GetAlembicCamera: need ABCTree"); return ZErr_ParamError; }
        std::queue<std::pair<Alembic::Abc::v12::M44d, ABCTree*>> q;
        q.emplace(Alembic::Abc::v12::M44d(), abctree);
        Alembic::Abc::v12::M44d mat;
        std::optional<CameraInfo> cam_info;
        while (q.size() > 0) {
            auto [m, t] = q.front();
            q.pop();
            if (t->camera_info) {
                mat = m;
                cam_info = *(t->camera_info);
                break;
            }
            for (auto& ch : t->children)
                q.emplace(t->xform * m, ch.get());
        }
        if (!cam_info.has_value()) {
            nd->report_error("GetAlembicCamera: camera not found");
            return ZErr_ParamError;
        }
        auto pos = Imath::V4d(0, 0, 0, 1) * mat;
        auto up = Imath::V4d(0, 1, 0, 0) * mat;
        auto right = Imath::V4d(1, 0, 0, 0) * mat;
        float focal_length = (float)cam_info.value().focal_length;
        nd->set_output_vec3f("pos", zeno::Vec3f((float)pos.x, (float)pos.y, (float)pos.z));
        zeno::Vec3f up_v((float)up.x, (float)up.y, (float)up.z);
        zeno::Vec3f right_v((float)right.x, (float)right.y, (float)right.z);
        zeno::Vec3f _up = vec3f_normalize(up_v);
        zeno::Vec3f _right = vec3f_normalize(right_v);
        zeno::Vec3f view = vec3f_cross(_up, _right);
        nd->set_output_vec3f("up", _up);
        nd->set_output_vec3f("right", _right);
        nd->set_output_vec3f("view", view);
        nd->set_output_float("focal_length", focal_length);
        nd->set_output_float("near", (float)cam_info.value()._near);
        nd->set_output_float("far", (float)cam_info.value()._far);
        nd->set_output_float("horizontalAperture", (float)cam_info->horizontalAperture);
        nd->set_output_float("verticalAperture", (float)cam_info->verticalAperture);
        float m_nx = (float)nd->get_input2_int("nx");
        float m_ny = (float)nd->get_input2_int("ny");
        float m_ha = (float)cam_info->horizontalAperture;
        float m_va = (float)cam_info->verticalAperture;
        float c_aspect = m_ha / m_va;
        float u_aspect = m_nx / m_ny;
        float fov_y = radians_to_degrees(2.0f * std::atan(m_va / (u_aspect / c_aspect) / (2.0f * focal_length)));
        nd->set_output_float("fov_y", fov_y);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(GetAlembicCamera,
    Z_INPUTS(
        {"abctree", _gParamType_IObject},
        {"nx", _gParamType_Int, ZInt(1920)},
        {"ny", _gParamType_Int, ZInt(1080)}
    ),
    Z_OUTPUTS(
        {"pos", _gParamType_Vec3f},
        {"up", _gParamType_Vec3f},
        {"view", _gParamType_Vec3f},
        {"right", _gParamType_Vec3f},
        {"fov_y", _gParamType_Float},
        {"focal_length", _gParamType_Float},
        {"horizontalAperture", _gParamType_Float},
        {"verticalAperture", _gParamType_Float},
        {"near", _gParamType_Float},
        {"far", _gParamType_Float}
    ),
    "alembic", "", "", "");

struct ImportAlembicPrim : INode2 {
    Alembic::Abc::v12::IArchive archive;
    std::string usedPath;

    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        int frameid = nd->has_input("frameid") ? nd->get_input2_int("frameid") : nd->GetFrameId();
        auto abctree = std::make_unique<ABCTree>();
        std::string path = get_input2_string(nd, "path");
        bool read_done = archive.valid() && (path == usedPath);
        if (!read_done) {
            archive = readABC(path);
            usedPath = path;
        }
        double start, _end;
        GetArchiveStartAndEndTime(archive, start, _end);
        TimeAndSamplesMap timeMap;
        Alembic::Util::uint32_t numSamplings = archive.getNumTimeSamplings();
        for (Alembic::Util::uint32_t s = 0; s < numSamplings; ++s)
            timeMap.add(archive.getTimeSampling(s), archive.getMaxNumSamplesForTimeSamplingIndex(s));
        auto obj = archive.getTop();
        bool read_face_set = nd->get_input2_bool("read_face_set");
        bool outOfRangeAsEmpty = nd->get_input2_bool("outOfRangeAsEmpty");
        traverseABC(obj, *abctree, frameid, read_done, read_face_set, "", timeMap, ObjectVisibility::kVisibilityDeferred, false, outOfRangeAsEmpty, 0);

        bool use_xform = nd->get_input2_bool("use_xform");
        int index = nd->get_input2_int("index");
        int abc_count = count_alembic_prims(abctree.get());

        if (index != -1 && !use_xform) {
            auto geom = get_alembic_prim(abctree.get(), index);
            { auto* g = zeno::zs_alembic::primFlipFaces(geom.get(), true); if (g != geom.get()) geom.reset(g); }
            if (nd->get_input2_bool("triangulate")) { auto* g = zeno::zs_alembic::primTriangulate(geom.get()); if (g != geom.get()) geom.reset(g); }
            if (geom->userData()) geom->userData()->set_int("_abc_prim_count", abc_count);
            nd->set_output_object("prim", geom.release());
            return ZErr_OK;
        }
        if (index != -1 && use_xform) {
            auto geom = get_xformed_prim_igeom(abctree.get(), index);
            if (!geom) { nd->report_error("ImportAlembicPrim: index out of range"); return ZErr_ParamError; }
            { auto* g = zeno::zs_alembic::primFlipFaces(geom.get(), true); if (g != geom.get()) geom.reset(g); }
            if (nd->get_input2_bool("triangulate")) { auto* g = zeno::zs_alembic::primTriangulate(geom.get()); if (g != geom.get()) geom.reset(g); }
            if (geom->userData()) geom->userData()->set_int("_abc_prim_count", abc_count);
            nd->set_output_object("prim", geom.release());
            return ZErr_OK;
        }
        std::vector<zeno::IGeometryObject*> geomlst;
        if (use_xform) {
            IListObject* list = get_xformed_prims_igeom(abctree.get());
            if (list) {
                for (size_t i = 0; i < list->size(); i++)
                    geomlst.push_back(dynamic_cast<zeno::IGeometryObject*>(list->get(i)->clone()));
                list->Delete();
            }
        } else {
            abctree->visitPrims([&](zeno::IGeometryObject* p) { geomlst.push_back(p); });
        }
        zeno::IGeometryObject* merged = zeno::zs_alembic::PrimMerge(geomlst);
        if (!merged) { nd->report_error("ImportAlembicPrim: merge failed"); return ZErr_ParamError; }
        { auto* g = zeno::zs_alembic::primFlipFaces(merged, true); if (g != merged) { merged->Delete(); merged = g; } }
        if (nd->get_input2_bool("triangulate")) { auto* g = zeno::zs_alembic::primTriangulate(merged); if (g != merged) { merged->Delete(); merged = g; } }
        if (merged->userData()) merged->userData()->set_int("_abc_prim_count", abc_count);
        nd->set_output_object("prim", merged);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(ImportAlembicPrim,
    Z_INPUTS(
        {"path", _gParamType_String, ZString("")},
        {"frameid", _gParamType_Int, ZInt(-1)},
        {"index", _gParamType_Int, ZInt(-1)},
        {"use_xform", _gParamType_Bool, ZInt(0)},
        {"triangulate", _gParamType_Bool, ZInt(0)},
        {"read_face_set", _gParamType_Bool, ZInt(0)},
        {"outOfRangeAsEmpty", _gParamType_Bool, ZInt(0)}
    ),
    Z_OUTPUTS({"prim", _gParamType_Geometry}),
    "alembic", "", "", "");

} // namespace zeno

