#ifndef __GEOMETRY_UTIL_H__
#define __GEOMETRY_UTIL_H__

#include <vector>
#include <tuple>
#include <array>
#include <zeno/types/ListObject.h>
#include <zeno/types/GeometryObject.h>
#include <iobject2.h>

namespace zeno
{
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

    enum DivideKeep {
        Keep_Both,
        Keep_Below,
        Keep_Above
    };

    ZENO_API std::pair<vec3f, vec3f> GetGeomBoundingBox(GeometryObject* geo);
    ZENO_API bool geomBoundingBox(IGeometryObject* geo, Vec3f& bbmin, Vec3f& bbmax);
    //ZENO_EXPORT IGeometryObject* __cdecl mergeObjects(
    //    IListObject* spList,
    //    const char* tagAttr,
    //    bool tag_on_vert,
    //    bool tag_on_face);
    ZENO_API void geom_set_abcpath(IGeometryObject* prim, const char* path_name);
    ZENO_API void geom_set_faceset(IGeometryObject* prim, const char* faceset_name);
    ZENO_API IGeometryObject* fuseGeometry(IGeometryObject* input, float threshold);
    ZENO_API IGeometryObject* constructGeom(const ZFacesPoints* faces);
    ZENO_API IGeometryObject* scatter(
        IGeometryObject* input,
        const char* sampleRegion,
        const int count,
        int seed);
    ZENO_API IGeometryObject* divideObject(
        IGeometryObject* input_object,
        DivideKeep keep,
        Vec3f center_pos,
        Vec3f direction
    );
    void transformGeom(
        IGeometryObject* geom
        , ZMat4 matrix
        , const char* pivotType
        , Vec3f pivotPos
        , Vec3f localX
        , Vec3f localY
        , Vec3f translate
        , Vec4f _rotation
        , Vec3f scaling);
}


#endif