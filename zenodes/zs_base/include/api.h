#pragma once

#include "iobject2.h"
#include <vector>
#include <vec.h>

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
}