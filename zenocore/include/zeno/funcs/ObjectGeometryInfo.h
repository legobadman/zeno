#pragma once

#include <zeno/utils/vec.h>
#include <iobject2.h>
#include <zeno/types/PrimitiveObject.h>

namespace zeno {

ZENO_API bool objectGetBoundingBox(IObject2 *ptr, vec3f &bmin, vec3f &bmax);
ZENO_API bool objectGetFocusCenterRadius(IObject2 *ptr, vec3f &center, float &radius);

}
