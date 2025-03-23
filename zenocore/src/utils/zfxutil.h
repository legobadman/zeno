#ifndef __ZFX_UTIL_H__
#define __ZFX_UTIL_H__

#include <zeno/core/data.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/types/AttributeVector.h>

namespace zeno
{
    namespace zfx
    {
        zeno::reflect::Any zfxvarToAny(const zfxvariant& var);
        AttrVar zfxvarToAttrvar(const zfxvariant& var);
        std::vector<zfxvariant> extractAttrValue(zeno::reflect::Any anyval, int size);
        std::vector<zfxvariant> attrvarVecToZfxVec(AttrVarVec anyval, int size);
        AttrVar convertToAttrVar(const std::vector<zfxvariant>& zfxvec);
        void setAttrValue(std::string attrname, const std::string& channel, const ZfxVariable& var, operatorVals opVal, ZfxElemFilter& filter, ZfxContext* pContext);
        ZfxVariable getAttrValue(const std::string& attrname, ZfxContext* pContext, char channel = 0);
    }
}



#endif