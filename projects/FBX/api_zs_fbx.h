#pragma once

#include <vector>

#include <iobject2.h>
#include <zenum.h>

namespace zeno {

// Wrapper around zenocore.dll exported geometry factory.
IGeometryObject* createGeometry(
    GeomTopoType type,
    bool bTriangle,
    int nPoints,
    int nFaces,
    bool bInitFaces);

// Convenience helper: create geometry from point list and face index lists.
IGeometryObject* createGeometryByPointFace(
    GeomTopoType type,
    bool bTriangle,
    const std::vector<Vec3f>& points,
    const std::vector<std::vector<int>>& faces);

// Wrapper for zenocore.dll ListObject factory (createList).
IListObject* createList();

} // namespace zeno

