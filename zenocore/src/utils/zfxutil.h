#ifndef __ZFX_UTIL_H__
#define __ZFX_UTIL_H__

#include <zeno/core/data.h>
#include <zeno/formula/syntax_tree.h>

namespace zeno
{
    namespace zfx
    {
        zeno::reflect::Any zfxvarToAny(const zfxvariant& var);
        std::vector<zfxvariant> extractAttrValue(zeno::reflect::Any anyval, int size);
    }
}



#endif