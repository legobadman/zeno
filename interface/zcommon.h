#ifndef __ZCOMMON_H__
#define __ZCOMMON_H__

#include <zenum.h>
#include <inodeimpl.h>

#define Z_JOIN(a,b) a##b
#define Z_CAT(a,b) Z_JOIN(a,b)

#define Z_UNIQUE(name) Z_CAT(name,__COUNTER__)

#define Z_JOIN(a,b) a##b
#define Z_CAT(a,b) Z_JOIN(a,b)

#define Z_ARRAY(...) Z_ARRAY_IMPL(__COUNTER__, __VA_ARGS__)
#define Z_ARRAY_IMPL(id, ...) Z_ARRAY_IMPL2(id, __VA_ARGS__)
#define Z_ARRAY_IMPL2(id, ...)                                     \
    ([]() -> ZValue {                                             \
        static const float Z_CAT(_zarr_, id)[] = { __VA_ARGS__ }; \
        ZValue v{};                                               \
        v._type = ZVAL_FLOAT_ARRAY;                                \
        v.farr.data = Z_CAT(_zarr_, id);                          \
        v.farr.size = sizeof(Z_CAT(_zarr_, id)) / sizeof(float); \
        return v;                                                 \
    }())

#define Z_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define Z_FLOAT(name,def) \
    { #name, _gParamType_Float, ZFloat(def), ctrl_Slider, ZNull() }

#define Z_BOOL(name,def) \
    { #name, _gParamType_Bool, ZInt(def), ctrl_Checkbox, ZNull() }

#define Z_INT(name,def) \
    { #name, _gParamType_Int, ZInt(def), ctrl_Lineedit, ZNull() }

#define Z_INPUTS(...) __VA_ARGS__
#define Z_OUTPUTS(...) __VA_ARGS__

struct ZNodeDescriptor {
    const ZParamDescriptor* inputs;
    size_t input_count;

    const ZParamDescriptor* outputs;
    size_t output_count;

    const char* node_name;
    const char* cate;
    const char* doc;
    const char* icon;
};
using fnRegister = bool(__cdecl*)(zeno::INode2* (*ctor)(), void (*dtor)(zeno::INode2*), const char* name, const ZNodeDescriptor&);
using fnUnRegister = bool(__cdecl*)(const char* name);

#define ZENDEFNODE_ABI(Node, Inputs, Outputs, Cate, Doc, Icon)      \
    static const ZParamDescriptor _inputs_of_##Node[] = { Inputs }; \
    static const ZParamDescriptor _outputs_of_##Node[] = { Outputs }; \
    static const ZNodeDescriptor Node##_desc = {                    \
        _inputs_of_##Node,  Z_ARRAY_COUNT(_inputs_of_##Node),       \
        _outputs_of_##Node, Z_ARRAY_COUNT(_outputs_of_##Node),      \
        #Node,  \
        Cate,  \
        Doc,   \
        Icon   \
    };\
    static struct _Def##Node { \
        _Def##Node() { \
            auto h = LoadLibrary("zenocore.dll");\
            if (h == INVALID_HANDLE_VALUE) { throw; }\
            auto procReg = (fnRegister)GetProcAddress(h, "registerNode");\
            if (procReg) { \
                procReg(\
                    [] ()->INode2* { return new Node; }, \
                    [] (INode2* pNode) { delete pNode; }, \
                    #Node, \
                    Node##_desc \
                    ); \
            } \
        } \
\
        ~_Def##Node() {\
            auto h = LoadLibrary("zenocore.dll");\
            if (h == INVALID_HANDLE_VALUE) { throw; }\
            auto procUnReg = (fnUnRegister)GetProcAddress(h, "unRegisterNode");\
            if (procUnReg) { \
                procUnReg(#Node); \
            } \
        }\
    } _def##Node;


#endif