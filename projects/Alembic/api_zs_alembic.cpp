#include "api_zs_alembic.h"

#include <Windows.h>
#include <cstring>
#include <vector>
#include <algorithm>

namespace zeno::zs_alembic {

namespace {

static void extract_geometry(IGeometryObject* geom,
    std::vector<Vec3f>& points,
    std::vector<std::vector<int>>& faces)
{
    const int np = geom->npoints();
    const int nf = geom->nfaces();
    points.resize(np);
    if (np > 0)
        geom->points_pos(points.data(), points.size());
    faces.resize(nf);
    for (int i = 0; i < nf; i++) {
        int cnt = geom->face_vertex_count(i);
        faces[i].resize(cnt);
        if (cnt > 0) {
            std::vector<int> pts(cnt);
            geom->face_points(i, pts.data(), cnt);
            faces[i].assign(pts.begin(), pts.end());
        }
    }
}

static void copy_userdata(IGeometryObject* dst, IGeometryObject* src) {
    IUserData2* sud = src->userData();
    IUserData2* dud = dst->userData();
    if (!sud || !dud) return;
    dud->copy(sud);
}

static void copy_face_attrs(IGeometryObject* dst, IGeometryObject* src,
    const std::vector<int>& face_mapping)
{
    const int nattr = src->nattributes(ATTR_FACE);
    for (int a = 0; a < nattr; a++) {
        char name[256] = {};
        if (src->get_attr_name(ATTR_FACE, a, name, sizeof(name)) == 0) continue;
        if (src->has_attr(ATTR_FACE, name, ATTR_INT)) {
            std::vector<int> old_attr(src->nfaces());
            src->get_int_attr(ATTR_FACE, name, old_attr.data(), old_attr.size());
            std::vector<int> new_attr(face_mapping.size());
            for (size_t i = 0; i < face_mapping.size(); i++)
                new_attr[i] = old_attr[face_mapping[i]];
            dst->create_attr_by_int(ATTR_FACE, name, new_attr.data(), new_attr.size());
        } else if (src->has_attr(ATTR_FACE, name, ATTR_FLOAT)) {
            std::vector<float> old_attr(src->nfaces());
            src->get_float_attr(ATTR_FACE, name, old_attr.data(), old_attr.size());
            std::vector<float> new_attr(face_mapping.size());
            for (size_t i = 0; i < face_mapping.size(); i++)
                new_attr[i] = old_attr[face_mapping[i]];
            dst->create_attr_by_float(ATTR_FACE, name, new_attr.data(), new_attr.size());
        } else if (src->has_attr(ATTR_FACE, name, ATTR_VEC3)) {
            std::vector<Vec3f> old_attr(src->nfaces());
            src->get_vec3f_attr(ATTR_FACE, name, old_attr.data(), old_attr.size());
            std::vector<Vec3f> new_attr(face_mapping.size());
            for (size_t i = 0; i < face_mapping.size(); i++)
                new_attr[i] = old_attr[face_mapping[i]];
            dst->create_attr_by_vec3(ATTR_FACE, name, new_attr.data(), new_attr.size());
        }
    }
}

static void copy_point_attrs(IGeometryObject* dst, IGeometryObject* src) {
    const int nattr = src->nattributes(ATTR_POINT);
    for (int a = 0; a < nattr; a++) {
        char name[256] = {};
        if (src->get_attr_name(ATTR_POINT, a, name, sizeof(name)) == 0) continue;
        if (strcmp(name, "pos") == 0) continue;
        if (src->has_attr(ATTR_POINT, name, ATTR_INT)) {
            std::vector<int> attr(src->npoints());
            src->get_int_attr(ATTR_POINT, name, attr.data(), attr.size());
            dst->create_attr_by_int(ATTR_POINT, name, attr.data(), attr.size());
        } else if (src->has_attr(ATTR_POINT, name, ATTR_FLOAT)) {
            std::vector<float> attr(src->npoints());
            src->get_float_attr(ATTR_POINT, name, attr.data(), attr.size());
            dst->create_attr_by_float(ATTR_POINT, name, attr.data(), attr.size());
        } else if (src->has_attr(ATTR_POINT, name, ATTR_VEC3)) {
            std::vector<Vec3f> attr(src->npoints());
            src->get_vec3f_attr(ATTR_POINT, name, attr.data(), attr.size());
            dst->create_attr_by_vec3(ATTR_POINT, name, attr.data(), attr.size());
        }
    }
}

struct ScopedArrays {
    Vec3f* points = nullptr;
    ZIntArray* faces = nullptr;
    size_t faceCount = 0;

    ~ScopedArrays() {
        delete[] points;
        if (faces) {
            for (size_t i = 0; i < faceCount; ++i) {
                delete[] faces[i].arr;
            }
            delete[] faces;
        }
    }
};

} // namespace

// Geometry / list factories ---------------------------------------------------

IGeometryObject* createGeometry(
    GeomTopoType type,
    bool bTriangle,
    int nPoints,
    int nFaces,
    bool bInitFaces)
{
#ifdef _WIN32
    HMODULE hDll = ::LoadLibraryA("zenocore.dll");
    if (!hDll || hDll == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    using fnCreateGeo = IGeometryObject* (__cdecl*)(GeomTopoType, bool, int, int, bool);
    auto fCall = reinterpret_cast<fnCreateGeo>(GetProcAddress(hDll, "createGeometry"));
    if (fCall) {
        return fCall(type, bTriangle, nPoints, nFaces, bInitFaces);
    }
#endif
    return nullptr;
}

IGeometryObject* createGeometryByPointFace(
    GeomTopoType type,
    bool bTriangle,
    const std::vector<Vec3f>& points,
    const std::vector<std::vector<int>>& faces)
{
    ScopedArrays tmp;

    const size_t pointCount = points.size();
    if (pointCount > 0) {
        tmp.points = new Vec3f[pointCount];
        for (size_t i = 0; i < pointCount; ++i) {
            tmp.points[i] = points[i];
        }
    }

    const size_t faceCount = faces.size();
    tmp.faceCount = faceCount;
    if (faceCount > 0) {
        tmp.faces = new ZIntArray[faceCount];
        for (size_t i = 0; i < faceCount; ++i) {
            const auto& f = faces[i];
            tmp.faces[i].size = static_cast<int>(f.size());
            if (!f.empty()) {
                tmp.faces[i].arr = new int[f.size()];
                std::memcpy(tmp.faces[i].arr, f.data(), f.size() * sizeof(int));
            } else {
                tmp.faces[i].arr = nullptr;
            }
        }
    }

#ifdef _WIN32
    HMODULE hDll = ::LoadLibraryA("zenocore.dll");
    if (!hDll || hDll == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    using fnCreateGeo = IGeometryObject* (__cdecl*)(GeomTopoType, bool, const Vec3f*, size_t, const ZIntArray*, size_t);
    auto fCall = reinterpret_cast<fnCreateGeo>(GetProcAddress(hDll, "createGeometryByPointFace"));
    if (fCall) {
        return fCall(type, bTriangle, tmp.points, pointCount, tmp.faces, faceCount);
    }
#endif
    return nullptr;
}

IListObject* createList()
{
#ifdef _WIN32
    HMODULE hDll = ::LoadLibraryA("zenocore.dll");
    if (!hDll || hDll == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    using fnCreateList = IListObject* (__cdecl*)();
    auto fCall = reinterpret_cast<fnCreateList>(GetProcAddress(hDll, "createList"));
    if (fCall) {
        return fCall();
    }
#endif
    return nullptr;
}

// Primitive-style utilities ---------------------------------------------------

IGeometryObject* primTriangulate(
    IGeometryObject* geom,
    bool with_uv,
    bool has_lines,
    bool with_attr)
{
    if (!geom || geom->nfaces() == 0) return geom;
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;
    extract_geometry(geom, points, faces);

    std::vector<std::vector<int>> new_faces;
    std::vector<int> face_mapping;
    bool need_triangulate = false;
    for (size_t i = 0; i < faces.size(); i++) {
        int len = (int)faces[i].size();
        if (len >= 3) {
            for (int j = 2; j < len; j++) {
                new_faces.push_back({faces[i][0], faces[i][j - 1], faces[i][j]});
                face_mapping.push_back((int)i);
            }
            if (len > 3) need_triangulate = true;
        } else if (len == 2 && has_lines) {
            new_faces.push_back(faces[i]);
            face_mapping.push_back((int)i);
        }
    }
    if (!need_triangulate && new_faces.size() == faces.size()) return geom;

    IGeometryObject* result = createGeometryByPointFace(Topo_IndiceMesh, true, points, new_faces);
    if (!result) return geom;
    copy_userdata(result, geom);
    copy_point_attrs(result, geom);
    if (with_attr) copy_face_attrs(result, geom, face_mapping);
    return result;
}

IGeometryObject* primFlipFaces(
    IGeometryObject* geom,
    bool only_face)
{
    if (!geom || geom->nfaces() == 0) return geom;
    std::vector<Vec3f> points;
    std::vector<std::vector<int>> faces;
    extract_geometry(geom, points, faces);

    bool changed = false;
    for (auto& f : faces) {
        if (f.size() == 2 && !only_face) {
            std::swap(f[0], f[1]);
            changed = true;
        } else if (f.size() == 3) {
            std::swap(f[0], f[2]);
            changed = true;
        } else if (f.size() == 4) {
            std::swap(f[0], f[3]);
            std::swap(f[1], f[2]);
            changed = true;
        } else if (f.size() > 1) {
            for (size_t i = 0; i < f.size() / 2; i++)
                std::swap(f[i], f[f.size() - 1 - i]);
            changed = true;
        }
    }
    if (!changed) return geom;

    IGeometryObject* result = createGeometryByPointFace(Topo_IndiceMesh, false, points, faces);
    if (!result) return geom;
    copy_userdata(result, geom);
    copy_point_attrs(result, geom);
    std::vector<int> identity_map(result->nfaces());
    for (size_t i = 0; i < identity_map.size(); i++) identity_map[i] = (int)i;
    copy_face_attrs(result, geom, identity_map);
    return result;
}

IGeometryObject* PrimMerge(
    const std::vector<IGeometryObject*>& /*geoms*/,
    const char* /*tagAttr*/,
    bool /*tag_on_vert*/,
    bool /*tag_on_face*/)
{
    // TODO: implement IGeometryObject-based merge or bridge to zenocore
    // via a dedicated ABI function when available.
    return nullptr;
}

IListObject* abc_split_by_name(
    IGeometryObject* /*geom*/,
    bool /*add_when_none*/)
{
    // TODO: implement IGeometryObject-based split or bridge to zenocore
    // via a dedicated ABI function when available.
    return nullptr;
}

} // namespace zeno::zs_alembic

