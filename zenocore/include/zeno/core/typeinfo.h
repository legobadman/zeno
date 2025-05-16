#pragma once

#if !defined(ZENO_REFLECT_PROCESSING)

#include "common.h"
#include <zeno/reflect/registry.hpp>
#include <zeno/reflection/zenoreflecttypes.cpp.generated.hpp>

using namespace zeno::types;

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
#define STR_CONCATE(s1, objectname, s2) s1##objectname##s2

#define registerObjectTypeHash(ObjectName, Name) \
        constexpr uint64_t gParamType_##Name = zeno::reflect::hash_64_typename(STRINGIZE(shared_ptr<ObjectName>));

#define registerObjectTypeUIInfo(ObjectName, Name, Color) \
        struct _Sclass_registor_##ObjectName {\
            _Sclass_registor_##ObjectName() {\
                zeno::getSession().registerObjUIInfo(gParamType_##Name, Color, #Name); \
            }\
        }; \
        static _Sclass_registor_##ObjectName static_inst_##ObjectName{};


registerObjectTypeHash(IObject, IObject)
registerObjectTypeHash(DictObject, Dict)
registerObjectTypeHash(ListObject, List)
registerObjectTypeHash(PrimitiveObject, Primitive)
registerObjectTypeHash(CameraObject, Camera)
registerObjectTypeHash(LightObject, Light)
registerObjectTypeHash(MeshObject, Mesh)
registerObjectTypeHash(ParticlesObject, Particles)
registerObjectTypeHash(MaterialObject, Material)
registerObjectTypeHash(GeometryObject, Geometry)
registerObjectTypeHash(InstancingObject, Instance)

#define gParamType_VDBGrid          234
#define gParamType_FOR              235
#define gParamType_AnyNumeric       236
#define gParamType_NumericObj       237
#define gParamType_JsonObject       238
#define gParamType_FBXObject        239
#define gParamType_Scene            240
#define gParamType_Heatmap          241
#define gParamType_ABCTree          242

#endif