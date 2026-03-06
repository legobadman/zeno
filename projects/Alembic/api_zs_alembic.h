#pragma once

#include <vector>

#include <iobject2.h>
#include <zenum.h>

namespace zeno::zs_alembic {

// Geometry / list factories ---------------------------------------------------

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

// Primitive-style utilities (ABI-facing IGeometryObject versions) ------------

// Triangulate indexed mesh geometry. Returns new geometry if modified, else input.
// Caller should use returned pointer and release old if different.
IGeometryObject* primTriangulate(
    IGeometryObject* geom,
    bool with_uv = true,
    bool has_lines = true,
    bool with_attr = true);

// Flip face winding / normals. Returns new geometry if modified, else input.
IGeometryObject* primFlipFaces(
    IGeometryObject* geom,
    bool only_face = false);

// Merge multiple geometries into one, with optional tagging.
IGeometryObject* PrimMerge(
    const std::vector<IGeometryObject*>& geoms,
    const char* tagAttr = "",
    bool tag_on_vert = true,
    bool tag_on_face = false);

// Split a geometry into a list of geometries by name (e.g. faceset),
// returns an IListObject* containing IGeometryObject* elements.
IListObject* abc_split_by_name(
    IGeometryObject* geom,
    bool add_when_none = false);

} // namespace zeno::zs_alembic

