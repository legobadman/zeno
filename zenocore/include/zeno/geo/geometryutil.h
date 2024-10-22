#ifndef __GEOMETRY_UTIL_H__
#define __GEOMETRY_UTIL_H__

#include <vector>
#include <tuple>
#include <array>
#include <zeno/types/GeometryObject.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace zeno
{
    using namespace zeno::reflect;

    enum Rotate_Orientaion
    {
        Orientaion_YZ,
        Orientaion_XY,
        Orientaion_ZX
    };

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
    ZENO_API glm::mat4 calc_rotate_matrix(
        float xangle,
        float yangle,
        float zangle,
        Rotate_Orientaion orientaion
        );
    ZENO_API std::pair<vec3f, vec3f> geomBoundingBox(GeometryObject* geo);
}


#endif