#ifndef __GEOMETRY_UTIL_H__
#define __GEOMETRY_UTIL_H__

#include <vector>
#include <tuple>
#include <array>
#include <zeno/types/GeometryObject.h>

namespace zeno
{
    using namespace zeno::reflect;

    struct GeometryObject;

    bool prim_remove_point(GeometryObject* prim, int ptnum);
    ZENO_API std::vector<vec3f> calc_point_normals(GeometryObject* prim, const std::vector<vec3i>& tris, float flip = 1.0f);
    ZENO_API std::vector<vec3f> calc_triangles_tangent(
        GeometryObject* geo,
        bool has_uv,
        const std::vector<vec3i>& tris,
        const std::vector<vec3f>& pos,
        const std::vector<vec3f>& uv,
        const std::vector<vec3f>& uv0,
        const std::vector<vec3f>& uv1,
        const std::vector<vec3f>& uv2
        );
    ZENO_API std::vector<vec3f> compute_vertex_tangent(
        const std::vector<vec3i>& tris,
        const std::vector<vec3f>& tang,
        const std::vector<vec3f>& pos
        );
}


#endif