#pragma once

#include "iobject2.h"
#include <vector>
#include <vec.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace zeno {

	IGeometryObject* createGeometry(GeomTopoType type, bool bTriangle, int nPoints, int nFaces, bool bInitFaces);
    IGeometryObject* create_GeometryObject(
        GeomTopoType type,
        bool bTriangle,
        const std::vector<vec3f>& points,
        const std::vector<std::vector<int>>& faces);
    IGeometryObject* mergeObjects(
        IListObject* spList,
        const char* tagAttr = 0,
        bool tag_on_vert = true,
        bool tag_on_face = false);
    void transformGeom(
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