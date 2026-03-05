#include "api_zs_fbx.h"

#include <Windows.h>
#include <cstring>

namespace zeno::zs_fbx {

namespace {

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

} // namespace zeno::zs_fbx

