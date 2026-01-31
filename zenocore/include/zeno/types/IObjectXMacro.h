#pragma once

#define ZENO_XMACRO_IObject(PER, ...) \
    PER(PrimitiveObject, __VA_ARGS__) \
    PER(GeometryObject, __VA_ARGS__) \
    PER(CameraObject, __VA_ARGS__) \
    PER(LightObject, __VA_ARGS__) \
    PER(MaterialObject, __VA_ARGS__) \
    PER(ListObject, __VA_ARGS__) \
    PER(DummyObject, __VA_ARGS__)
