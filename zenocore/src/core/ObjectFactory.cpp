#include <zeno/types/PrimitiveObject.h>


namespace zeno {

ZENO_API std::unique_ptr<PrimitiveObject> createPrimitive() {
    return std::make_unique<PrimitiveObject>();
}

ZENO_API std::unique_ptr<PrimitiveObject> clonePrimitive(PrimitiveObject* pObj) {
    return std::make_unique<PrimitiveObject>(*pObj);
}

}