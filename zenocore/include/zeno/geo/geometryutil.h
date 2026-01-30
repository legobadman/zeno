#ifndef __GEOMETRY_UTIL_H__
#define __GEOMETRY_UTIL_H__

#include <vector>
#include <tuple>
#include <array>
#include <zeno/types/ListObject.h>
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

    enum PointSide {
        UnDecided,
        Below,
        Above,
        Both,   //在平面上
    };

    struct DividePoint
    {
        vec3f pos;  //分割点的坐标
        int from;   //分割点所在线段的起点
        int to;     //分割点所在线段的终点，并有from < to，如果分割点恰好是顶点，则from=to
    };

    enum DivideKeep {
        Keep_Both,
        Keep_Below,
        Keep_Above
    };

    struct DivideFace
    {
        std::vector<int> face_indice;
        PointSide side;
    };

    ZENO_API bool geomBoundingBox(IGeometryObject* geo, Vec3f& bbmin, Vec3f& bbmax);
    ZENO_API IGeometryObject* mergeObjects(
        zeno::ListObject* spList,
        std::string const& tagAttr = {},
        bool tag_on_vert = true,
        bool tag_on_face = false);
    ZENO_API void geom_set_abcpath(IGeometryObject* prim, const char* path_name);
    ZENO_API void geom_set_faceset(IGeometryObject* prim, const char* faceset_name);
    ZENO_API IGeometryObject* fuseGeometry(IGeometryObject* input, float threshold);
    ZENO_API IGeometryObject* constructGeom(const std::vector<std::vector<zeno::vec3f>>& faces);
    ZENO_API IGeometryObject* scatter(
        IGeometryObject* input,
        const std::string& sampleRegion,
        const int count,
        int seed);
    ZENO_API IGeometryObject* divideObject(
        IGeometryObject* input_object,
        DivideKeep keep,
        zeno::vec3f center_pos,
        zeno::vec3f direction
    );
    ZENO_API void transformGeom(
        IGeometryObject* geom
        , glm::mat4 matrix
        , std::string pivotType
        , vec3f pivotPos
        , vec3f localX
        , vec3f localY
        , vec3f translate
        , vec4f rotation
        , vec3f scaling);
}


#endif