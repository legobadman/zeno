
#pragma once

#include <zeno/reflect/type>
#include <zeno/reflect/polyfill.hpp>
#include <zeno/reflect/reflection_traits.hpp>
#include <zeno/reflect/registry.hpp>
#include <type_traits>

/* include headers from user define */
#include <zeno/types/ObjectDef.h>

////////////////////////////////////////////////
/// Begin generated RTTI for types

///////////////////////////
/// Begin RTTI of "bool"
#ifndef _REFLECT_RTTI_GUARD_bool_15698046148323980066
#define _REFLECT_RTTI_GUARD_bool_15698046148323980066 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<bool>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<bool>::type>::type,bool>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool",
                15698046148323980066ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool",
                15698046148323980066ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<bool>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _SboolRegistrator {
        _SboolRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(15698046148323980066, zeno::reflect::type_info<bool>());
        }
    };
    static _SboolRegistrator global_SboolRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Bool = 15698046148323980066ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_bool_15698046148323980066
/// End RTTI of "bool"
///////////////////////////

///////////////////////////
/// Begin RTTI of "bool &"
#ifndef _REFLECT_RTTI_GUARD_bool_14233656939025313984
#define _REFLECT_RTTI_GUARD_bool_14233656939025313984 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<bool &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<bool &>::type>::type,bool &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool &",
                14233656939025313984ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool &",
                14233656939025313984ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<bool &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_bool_14233656939025313984
/// End RTTI of "bool &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "bool &&"
#ifndef _REFLECT_RTTI_GUARD_bool_502081220818802386
#define _REFLECT_RTTI_GUARD_bool_502081220818802386 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<bool &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<bool &&>::type>::type,bool &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool &&",
                502081220818802386ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool &&",
                502081220818802386ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<bool &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_bool_502081220818802386
/// End RTTI of "bool &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const bool &"
#ifndef _REFLECT_RTTI_GUARD_const_bool_13650828574261202711
#define _REFLECT_RTTI_GUARD_const_bool_13650828574261202711 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const bool &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const bool &>::type>::type,const bool &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const bool &",
                13650828574261202711ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const bool &",
                13650828574261202711ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const bool &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_bool_13650828574261202711
/// End RTTI of "const bool &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const bool *"
#ifndef _REFLECT_RTTI_GUARD_const_bool_Mul_13650815380121664179
#define _REFLECT_RTTI_GUARD_const_bool_Mul_13650815380121664179 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const bool *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const bool *>::type>::type,const bool *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const bool *",
                13650815380121664179ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const bool *",
                13650815380121664179ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const bool *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_bool_Mul_13650815380121664179
/// End RTTI of "const bool *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "bool *"
#ifndef _REFLECT_RTTI_GUARD_bool_Mul_14233670133164852516
#define _REFLECT_RTTI_GUARD_bool_Mul_14233670133164852516 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<bool *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<bool *>::type>::type,bool *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool *",
                14233670133164852516ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "bool *",
                14233670133164852516ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<bool *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_bool_Mul_14233670133164852516
/// End RTTI of "bool *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "int"
#ifndef _REFLECT_RTTI_GUARD_int_3143511548502526014
#define _REFLECT_RTTI_GUARD_int_3143511548502526014 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<int>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<int>::type>::type,int>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int",
                3143511548502526014ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int",
                3143511548502526014ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<int>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _SintRegistrator {
        _SintRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(3143511548502526014, zeno::reflect::type_info<int>());
        }
    };
    static _SintRegistrator global_SintRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Int = 3143511548502526014ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_int_3143511548502526014
/// End RTTI of "int"
///////////////////////////

///////////////////////////
/// Begin RTTI of "int &"
#ifndef _REFLECT_RTTI_GUARD_int_18022097905200034772
#define _REFLECT_RTTI_GUARD_int_18022097905200034772 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<int &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<int &>::type>::type,int &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int &",
                18022097905200034772ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int &",
                18022097905200034772ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<int &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_int_18022097905200034772
/// End RTTI of "int &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "int &&"
#ifndef _REFLECT_RTTI_GUARD_int_13758166382949368886
#define _REFLECT_RTTI_GUARD_int_13758166382949368886 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<int &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<int &&>::type>::type,int &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int &&",
                13758166382949368886ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int &&",
                13758166382949368886ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<int &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_int_13758166382949368886
/// End RTTI of "int &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const int &"
#ifndef _REFLECT_RTTI_GUARD_const_int_6998541147902673403
#define _REFLECT_RTTI_GUARD_const_int_6998541147902673403 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const int &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const int &>::type>::type,const int &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const int &",
                6998541147902673403ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const int &",
                6998541147902673403ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const int &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_int_6998541147902673403
/// End RTTI of "const int &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const int *"
#ifndef _REFLECT_RTTI_GUARD_const_int_Mul_6998536749856160559
#define _REFLECT_RTTI_GUARD_const_int_Mul_6998536749856160559 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const int *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const int *>::type>::type,const int *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const int *",
                6998536749856160559ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const int *",
                6998536749856160559ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const int *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_int_Mul_6998536749856160559
/// End RTTI of "const int *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "int *"
#ifndef _REFLECT_RTTI_GUARD_int_Mul_18022084711060496240
#define _REFLECT_RTTI_GUARD_int_Mul_18022084711060496240 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<int *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<int *>::type>::type,int *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int *",
                18022084711060496240ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "int *",
                18022084711060496240ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<int *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_int_Mul_18022084711060496240
/// End RTTI of "int *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "float"
#ifndef _REFLECT_RTTI_GUARD_float_11532138274943533413
#define _REFLECT_RTTI_GUARD_float_11532138274943533413 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<float>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<float>::type>::type,float>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float",
                11532138274943533413ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float",
                11532138274943533413ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<float>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _SfloatRegistrator {
        _SfloatRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(11532138274943533413, zeno::reflect::type_info<float>());
        }
    };
    static _SfloatRegistrator global_SfloatRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Float = 11532138274943533413ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_float_11532138274943533413
/// End RTTI of "float"
///////////////////////////

///////////////////////////
/// Begin RTTI of "float &"
#ifndef _REFLECT_RTTI_GUARD_float_10144569969248838267
#define _REFLECT_RTTI_GUARD_float_10144569969248838267 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<float &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<float &>::type>::type,float &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float &",
                10144569969248838267ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float &",
                10144569969248838267ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<float &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_float_10144569969248838267
/// End RTTI of "float &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "float &&"
#ifndef _REFLECT_RTTI_GUARD_float_7296309548120171527
#define _REFLECT_RTTI_GUARD_float_7296309548120171527 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<float &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<float &&>::type>::type,float &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float &&",
                7296309548120171527ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float &&",
                7296309548120171527ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<float &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_float_7296309548120171527
/// End RTTI of "float &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const float &"
#ifndef _REFLECT_RTTI_GUARD_const_float_9674859982983335220
#define _REFLECT_RTTI_GUARD_const_float_9674859982983335220 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const float &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const float &>::type>::type,const float &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const float &",
                9674859982983335220ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const float &",
                9674859982983335220ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const float &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_float_9674859982983335220
/// End RTTI of "const float &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const float *"
#ifndef _REFLECT_RTTI_GUARD_const_float_Mul_9674846788843796688
#define _REFLECT_RTTI_GUARD_const_float_Mul_9674846788843796688 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const float *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const float *>::type>::type,const float *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const float *",
                9674846788843796688ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const float *",
                9674846788843796688ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const float *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_float_Mul_9674846788843796688
/// End RTTI of "const float *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "float *"
#ifndef _REFLECT_RTTI_GUARD_float_Mul_10144565571202325423
#define _REFLECT_RTTI_GUARD_float_Mul_10144565571202325423 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<float *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<float *>::type>::type,float *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float *",
                10144565571202325423ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "float *",
                10144565571202325423ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<float *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_float_Mul_10144565571202325423
/// End RTTI of "float *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "double"
#ifndef _REFLECT_RTTI_GUARD_double_11567507311810436776
#define _REFLECT_RTTI_GUARD_double_11567507311810436776 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<double>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<double>::type>::type,double>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double",
                11567507311810436776ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double",
                11567507311810436776ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<double>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _SdoubleRegistrator {
        _SdoubleRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(11567507311810436776, zeno::reflect::type_info<double>());
        }
    };
    static _SdoubleRegistrator global_SdoubleRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Double = 11567507311810436776ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_double_11567507311810436776
/// End RTTI of "double"
///////////////////////////

///////////////////////////
/// Begin RTTI of "double &"
#ifndef _REFLECT_RTTI_GUARD_double_16622477481007348826
#define _REFLECT_RTTI_GUARD_double_16622477481007348826 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<double &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<double &>::type>::type,double &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double &",
                16622477481007348826ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double &",
                16622477481007348826ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<double &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_double_16622477481007348826
/// End RTTI of "double &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "double &&"
#ifndef _REFLECT_RTTI_GUARD_double_6534538014209640116
#define _REFLECT_RTTI_GUARD_double_6534538014209640116 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<double &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<double &&>::type>::type,double &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double &&",
                6534538014209640116ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double &&",
                6534538014209640116ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<double &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_double_6534538014209640116
/// End RTTI of "double &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const double &"
#ifndef _REFLECT_RTTI_GUARD_const_double_16209805948554021567
#define _REFLECT_RTTI_GUARD_const_double_16209805948554021567 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const double &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const double &>::type>::type,const double &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const double &",
                16209805948554021567ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const double &",
                16209805948554021567ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const double &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_double_16209805948554021567
/// End RTTI of "const double &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const double *"
#ifndef _REFLECT_RTTI_GUARD_const_double_Mul_16209810346600534411
#define _REFLECT_RTTI_GUARD_const_double_Mul_16209810346600534411 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const double *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const double *>::type>::type,const double *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const double *",
                16209810346600534411ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const double *",
                16209810346600534411ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const double *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_double_Mul_16209810346600534411
/// End RTTI of "const double *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "double *"
#ifndef _REFLECT_RTTI_GUARD_double_Mul_16622464286867810294
#define _REFLECT_RTTI_GUARD_double_Mul_16622464286867810294 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<double *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<double *>::type>::type,double *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double *",
                16622464286867810294ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "double *",
                16622464286867810294ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<double *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_double_Mul_16622464286867810294
/// End RTTI of "double *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::basic_string<char>"
#ifndef _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_1350625706064273086
#define _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_1350625706064273086 1
// !!! importance: This is a template specialization "class std::basic_string<char>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::basic_string<char>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::basic_string<char>>::type>::type,class std::basic_string<char>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char>",
                1350625706064273086ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char>",
                1350625706064273086ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::basic_string<char>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_basicstringLABcharRABRegistrator {
        _Sclass_std_basicstringLABcharRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(1350625706064273086, zeno::reflect::type_info<class std::basic_string<char>>());
        }
    };
    static _Sclass_std_basicstringLABcharRABRegistrator global_Sclass_std_basicstringLABcharRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_String = 1350625706064273086ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_1350625706064273086
/// End RTTI of "class std::basic_string<char>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::basic_string<char> &"
#ifndef _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6673018989892701780
#define _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6673018989892701780 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::basic_string<char> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::basic_string<char> &>::type>::type,class std::basic_string<char> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> &",
                6673018989892701780ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> &",
                6673018989892701780ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::basic_string<char> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6673018989892701780
/// End RTTI of "class std::basic_string<char> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::basic_string<char> &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6448363039830379446
#define _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6448363039830379446 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::basic_string<char> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::basic_string<char> &&>::type>::type,class std::basic_string<char> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> &&",
                6448363039830379446ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> &&",
                6448363039830379446ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::basic_string<char> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_6448363039830379446
/// End RTTI of "class std::basic_string<char> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::basic_string<char> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_10943554084914833555
#define _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_10943554084914833555 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::basic_string<char> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::basic_string<char> &>::type>::type,const class std::basic_string<char> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::basic_string<char> &",
                10943554084914833555ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::basic_string<char> &",
                10943554084914833555ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::basic_string<char> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_10943554084914833555
/// End RTTI of "const class std::basic_string<char> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::basic_string<char> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_Mul_10943567279054372087
#define _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_Mul_10943567279054372087 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::basic_string<char> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::basic_string<char> *>::type>::type,const class std::basic_string<char> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::basic_string<char> *",
                10943567279054372087ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::basic_string<char> *",
                10943567279054372087ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::basic_string<char> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_basicstringLABcharRAB_Mul_10943567279054372087
/// End RTTI of "const class std::basic_string<char> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::basic_string<char> *"
#ifndef _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_Mul_6673005795753163248
#define _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_Mul_6673005795753163248 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::basic_string<char> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::basic_string<char> *>::type>::type,class std::basic_string<char> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> *",
                6673005795753163248ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::basic_string<char> *",
                6673005795753163248ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::basic_string<char> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_basicstringLABcharRAB_Mul_6673005795753163248
/// End RTTI of "class std::basic_string<char> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::String"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_String_14378431277616792538
#define _REFLECT_RTTI_GUARD_class_zeno_String_14378431277616792538 1
namespace zeno {
class String;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::String>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::String>::type>::type,class zeno::String>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String",
                14378431277616792538ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String",
                14378431277616792538ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::String>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_StringRegistrator {
        _Sclass_zeno_StringRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(14378431277616792538, zeno::reflect::type_info<class zeno::String>());
        }
    };
    static _Sclass_zeno_StringRegistrator global_Sclass_zeno_StringRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiString = 14378431277616792538ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_String_14378431277616792538
/// End RTTI of "class zeno::String"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::String &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_String_10870142455111122232
#define _REFLECT_RTTI_GUARD_class_zeno_String_10870142455111122232 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::String &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::String &>::type>::type,class zeno::String &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String &",
                10870142455111122232ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String &",
                10870142455111122232ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::String &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_String_10870142455111122232
/// End RTTI of "class zeno::String &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::String &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_String_2289592383082502650
#define _REFLECT_RTTI_GUARD_class_zeno_String_2289592383082502650 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::String &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::String &&>::type>::type,class zeno::String &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String &&",
                2289592383082502650ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String &&",
                2289592383082502650ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::String &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_String_2289592383082502650
/// End RTTI of "class zeno::String &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::String &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_String_17167223094526725969
#define _REFLECT_RTTI_GUARD_const_class_zeno_String_17167223094526725969 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::String &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::String &>::type>::type,const class zeno::String &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::String &",
                17167223094526725969ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::String &",
                17167223094526725969ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::String &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_String_17167223094526725969
/// End RTTI of "const class zeno::String &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::String *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_String_Mul_17167218696480213125
#define _REFLECT_RTTI_GUARD_const_class_zeno_String_Mul_17167218696480213125 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::String *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::String *>::type>::type,const class zeno::String *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::String *",
                17167218696480213125ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::String *",
                17167218696480213125ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::String *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_String_Mul_17167218696480213125
/// End RTTI of "const class zeno::String *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::String *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_String_Mul_10870138057064609388
#define _REFLECT_RTTI_GUARD_class_zeno_String_Mul_10870138057064609388 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::String *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::String *>::type>::type,class zeno::String *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String *",
                10870138057064609388ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::String *",
                10870138057064609388ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::String *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_String_Mul_10870138057064609388
/// End RTTI of "class zeno::String *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, int>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16965886646643039397
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16965886646643039397 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<2, int>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, int>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int>>::type>::type,struct zeno::_impl_vec::vec<2, int>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int>",
                16965886646643039397ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int>",
                16965886646643039397ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB2_intRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB2_intRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16965886646643039397, zeno::reflect::type_info<struct zeno::_impl_vec::vec<2, int>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB2_intRABRegistrator global_Sstruct_zeno_implvec_vecLAB2_intRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec2i = 16965886646643039397ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16965886646643039397
/// End RTTI of "struct zeno::_impl_vec::vec<2, int>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, int> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16586788172887211963
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16586788172887211963 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> &>::type>::type,struct zeno::_impl_vec::vec<2, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> &",
                16586788172887211963ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> &",
                16586788172887211963ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_16586788172887211963
/// End RTTI of "struct zeno::_impl_vec::vec<2, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, int> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_13510553429106923463
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_13510553429106923463 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, int> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> &&>::type>::type,struct zeno::_impl_vec::vec<2, int> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> &&",
                13510553429106923463ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> &&",
                13510553429106923463ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_13510553429106923463
/// End RTTI of "struct zeno::_impl_vec::vec<2, int> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, int> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_12048283957452113580
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_12048283957452113580 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, int> &>::type>::type,const struct zeno::_impl_vec::vec<2, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, int> &",
                12048283957452113580ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, int> &",
                12048283957452113580ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_12048283957452113580
/// End RTTI of "const struct zeno::_impl_vec::vec<2, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, int> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_Mul_12048288355498626424
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_Mul_12048288355498626424 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, int> *>::type>::type,const struct zeno::_impl_vec::vec<2, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, int> *",
                12048288355498626424ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, int> *",
                12048288355498626424ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_intRAB_Mul_12048288355498626424
/// End RTTI of "const struct zeno::_impl_vec::vec<2, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, int> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_Mul_16586783774840699119
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_Mul_16586783774840699119 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> *>::type>::type,struct zeno::_impl_vec::vec<2, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> *",
                16586783774840699119ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, int> *",
                16586783774840699119ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_intRAB_Mul_16586783774840699119
/// End RTTI of "struct zeno::_impl_vec::vec<2, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2i"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_12756616248127599565
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_12756616248127599565 1
namespace zeno {
struct Vec2i;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2i>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2i>::type>::type,struct zeno::Vec2i>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i",
                12756616248127599565ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i",
                12756616248127599565ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2i>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec2iRegistrator {
        _Sstruct_zeno_Vec2iRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12756616248127599565, zeno::reflect::type_info<struct zeno::Vec2i>());
        }
    };
    static _Sstruct_zeno_Vec2iRegistrator global_Sstruct_zeno_Vec2iRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec2i = 12756616248127599565ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_12756616248127599565
/// End RTTI of "struct zeno::Vec2i"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2i &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_4321193235392509795
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_4321193235392509795 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2i &>::type>::type,struct zeno::Vec2i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i &",
                4321193235392509795ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i &",
                4321193235392509795ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_4321193235392509795
/// End RTTI of "struct zeno::Vec2i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2i &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_5372478271181679167
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_5372478271181679167 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2i &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2i &&>::type>::type,struct zeno::Vec2i &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i &&",
                5372478271181679167ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i &&",
                5372478271181679167ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2i &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_5372478271181679167
/// End RTTI of "struct zeno::Vec2i &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec2i &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_2343239544131597726
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_2343239544131597726 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec2i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec2i &>::type>::type,const struct zeno::Vec2i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2i &",
                2343239544131597726ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2i &",
                2343239544131597726ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec2i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_2343239544131597726
/// End RTTI of "const struct zeno::Vec2i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec2i *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_Mul_2343235146085084882
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_Mul_2343235146085084882 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec2i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec2i *>::type>::type,const struct zeno::Vec2i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2i *",
                2343235146085084882ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2i *",
                2343235146085084882ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec2i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2i_Mul_2343235146085084882
/// End RTTI of "const struct zeno::Vec2i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2i *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_Mul_4321206429532048327
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_Mul_4321206429532048327 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2i *>::type>::type,struct zeno::Vec2i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i *",
                4321206429532048327ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2i *",
                4321206429532048327ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2i_Mul_4321206429532048327
/// End RTTI of "struct zeno::Vec2i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_6471145251105555636
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_6471145251105555636 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<2, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float>>::type>::type,struct zeno::_impl_vec::vec<2, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float>",
                6471145251105555636ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float>",
                6471145251105555636ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB2_floatRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB2_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(6471145251105555636, zeno::reflect::type_info<struct zeno::_impl_vec::vec<2, float>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB2_floatRABRegistrator global_Sstruct_zeno_implvec_vecLAB2_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec2f = 6471145251105555636ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_6471145251105555636
/// End RTTI of "struct zeno::_impl_vec::vec<2, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_12564677542502202862
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_12564677542502202862 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> &>::type>::type,struct zeno::_impl_vec::vec<2, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> &",
                12564677542502202862ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> &",
                12564677542502202862ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_12564677542502202862
/// End RTTI of "struct zeno::_impl_vec::vec<2, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_8568394785495202520
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_8568394785495202520 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> &&>::type>::type,struct zeno::_impl_vec::vec<2, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> &&",
                8568394785495202520ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> &&",
                8568394785495202520ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_8568394785495202520
/// End RTTI of "struct zeno::_impl_vec::vec<2, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_14578011249785260901
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_14578011249785260901 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, float> &>::type>::type,const struct zeno::_impl_vec::vec<2, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, float> &",
                14578011249785260901ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, float> &",
                14578011249785260901ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_14578011249785260901
/// End RTTI of "const struct zeno::_impl_vec::vec<2, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_Mul_14578015647831773745
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_Mul_14578015647831773745 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, float> *>::type>::type,const struct zeno::_impl_vec::vec<2, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, float> *",
                14578015647831773745ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, float> *",
                14578015647831773745ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_floatRAB_Mul_14578015647831773745
/// End RTTI of "const struct zeno::_impl_vec::vec<2, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_Mul_12564673144455690018
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_Mul_12564673144455690018 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> *>::type>::type,struct zeno::_impl_vec::vec<2, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> *",
                12564673144455690018ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, float> *",
                12564673144455690018ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_floatRAB_Mul_12564673144455690018
/// End RTTI of "struct zeno::_impl_vec::vec<2, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2f"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_12756599755453176400
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_12756599755453176400 1
namespace zeno {
struct Vec2f;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2f>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2f>::type>::type,struct zeno::Vec2f>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f",
                12756599755453176400ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f",
                12756599755453176400ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2f>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec2fRegistrator {
        _Sstruct_zeno_Vec2fRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12756599755453176400, zeno::reflect::type_info<struct zeno::Vec2f>());
        }
    };
    static _Sstruct_zeno_Vec2fRegistrator global_Sstruct_zeno_Vec2fRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec2f = 12756599755453176400ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_12756599755453176400
/// End RTTI of "struct zeno::Vec2f"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2f &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_13405541921285000322
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_13405541921285000322 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2f &>::type>::type,struct zeno::Vec2f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f &",
                13405541921285000322ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f &",
                13405541921285000322ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_13405541921285000322
/// End RTTI of "struct zeno::Vec2f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2f &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_16996958865650072236
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_16996958865650072236 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2f &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2f &&>::type>::type,struct zeno::Vec2f &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f &&",
                16996958865650072236ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f &&",
                16996958865650072236ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2f &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_16996958865650072236
/// End RTTI of "struct zeno::Vec2f &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec2f &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_15233721452913857191
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_15233721452913857191 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec2f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec2f &>::type>::type,const struct zeno::Vec2f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2f &",
                15233721452913857191ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2f &",
                15233721452913857191ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec2f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_15233721452913857191
/// End RTTI of "const struct zeno::Vec2f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec2f *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_Mul_15233708258774318659
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_Mul_15233708258774318659 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec2f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec2f *>::type>::type,const struct zeno::Vec2f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2f *",
                15233708258774318659ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec2f *",
                15233708258774318659ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec2f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec2f_Mul_15233708258774318659
/// End RTTI of "const struct zeno::Vec2f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec2f *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_Mul_13405546319331513166
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_Mul_13405546319331513166 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec2f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec2f *>::type>::type,struct zeno::Vec2f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f *",
                13405546319331513166ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec2f *",
                13405546319331513166ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec2f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec2f_Mul_13405546319331513166
/// End RTTI of "struct zeno::Vec2f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> >"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_14482195190842139743
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_14482195190842139743 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<2, class std::basic_string<char> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, class std::basic_string<char> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> >>::type>::type,struct zeno::_impl_vec::vec<2, class std::basic_string<char> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> >",
                14482195190842139743ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> >",
                14482195190842139743ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RABRegistrator {
        _Sstruct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(14482195190842139743, zeno::reflect::type_info<struct zeno::_impl_vec::vec<2, class std::basic_string<char> >>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RABRegistrator global_Sstruct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec2s = 14482195190842139743ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_14482195190842139743
/// End RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1868953549774563409
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1868953549774563409 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::type>::type,struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &",
                1868953549774563409ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &",
                1868953549774563409ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1868953549774563409
/// End RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1673703922837635637
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1673703922837635637 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&>::type>::type,struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&",
                1673703922837635637ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&",
                1673703922837635637ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_1673703922837635637
/// End RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_15066641495504284748
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_15066641495504284748 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::type>::type,const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &",
                15066641495504284748ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &",
                15066641495504284748ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_15066641495504284748
/// End RTTI of "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_15066645893550797592
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_15066645893550797592 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::type>::type,const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *",
                15066645893550797592ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *",
                15066645893550797592ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_15066645893550797592
/// End RTTI of "const struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_1868949151728050565
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_1868949151728050565 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::type>::type,struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *",
                1868949151728050565ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *",
                1868949151728050565ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB2_class_std_basicstringLABcharRAB_RAB_Mul_1868949151728050565
/// End RTTI of "struct zeno::_impl_vec::vec<2, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, int>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_8367839167412710198
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_8367839167412710198 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<3, int>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, int>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int>>::type>::type,struct zeno::_impl_vec::vec<3, int>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int>",
                8367839167412710198ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int>",
                8367839167412710198ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB3_intRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB3_intRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8367839167412710198, zeno::reflect::type_info<struct zeno::_impl_vec::vec<3, int>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB3_intRABRegistrator global_Sstruct_zeno_implvec_vecLAB3_intRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec3i = 8367839167412710198ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_8367839167412710198
/// End RTTI of "struct zeno::_impl_vec::vec<3, int>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, int> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_17006952341255564
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_17006952341255564 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> &>::type>::type,struct zeno::_impl_vec::vec<3, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> &",
                17006952341255564ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> &",
                17006952341255564ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_17006952341255564
/// End RTTI of "struct zeno::_impl_vec::vec<3, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, int> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_7629302141302353886
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_7629302141302353886 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, int> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> &&>::type>::type,struct zeno::_impl_vec::vec<3, int> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> &&",
                7629302141302353886ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> &&",
                7629302141302353886ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_7629302141302353886
/// End RTTI of "struct zeno::_impl_vec::vec<3, int> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, int> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_4466301078465832923
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_4466301078465832923 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, int> &>::type>::type,const struct zeno::_impl_vec::vec<3, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, int> &",
                4466301078465832923ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, int> &",
                4466301078465832923ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_4466301078465832923
/// End RTTI of "const struct zeno::_impl_vec::vec<3, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, int> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_Mul_4466296680419320079
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_Mul_4466296680419320079 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, int> *>::type>::type,const struct zeno::_impl_vec::vec<3, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, int> *",
                4466296680419320079ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, int> *",
                4466296680419320079ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_intRAB_Mul_4466296680419320079
/// End RTTI of "const struct zeno::_impl_vec::vec<3, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, int> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_Mul_17011350387768408
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_Mul_17011350387768408 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> *>::type>::type,struct zeno::_impl_vec::vec<3, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> *",
                17011350387768408ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, int> *",
                17011350387768408ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_intRAB_Mul_17011350387768408
/// End RTTI of "struct zeno::_impl_vec::vec<3, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3i"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_12757412294546235104
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_12757412294546235104 1
namespace zeno {
struct Vec3i;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3i>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3i>::type>::type,struct zeno::Vec3i>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i",
                12757412294546235104ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i",
                12757412294546235104ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3i>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec3iRegistrator {
        _Sstruct_zeno_Vec3iRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12757412294546235104, zeno::reflect::type_info<struct zeno::Vec3i>());
        }
    };
    static _Sstruct_zeno_Vec3iRegistrator global_Sstruct_zeno_Vec3iRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec3i = 12757412294546235104ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_12757412294546235104
/// End RTTI of "struct zeno::Vec3i"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3i &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_16893363253903781202
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_16893363253903781202 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3i &>::type>::type,struct zeno::Vec3i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i &",
                16893363253903781202ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i &",
                16893363253903781202ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_16893363253903781202
/// End RTTI of "struct zeno::Vec3i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3i &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_17360091157091056668
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_17360091157091056668 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3i &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3i &&>::type>::type,struct zeno::Vec3i &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i &&",
                17360091157091056668ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i &&",
                17360091157091056668ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3i &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_17360091157091056668
/// End RTTI of "struct zeno::Vec3i &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec3i &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_4404355408080616175
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_4404355408080616175 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec3i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec3i &>::type>::type,const struct zeno::Vec3i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3i &",
                4404355408080616175ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3i &",
                4404355408080616175ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec3i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_4404355408080616175
/// End RTTI of "const struct zeno::Vec3i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec3i *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_Mul_4404359806127129019
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_Mul_4404359806127129019 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec3i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec3i *>::type>::type,const struct zeno::Vec3i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3i *",
                4404359806127129019ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3i *",
                4404359806127129019ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec3i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3i_Mul_4404359806127129019
/// End RTTI of "const struct zeno::Vec3i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3i *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_Mul_16893367651950294046
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_Mul_16893367651950294046 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3i *>::type>::type,struct zeno::Vec3i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i *",
                16893367651950294046ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3i *",
                16893367651950294046ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3i_Mul_16893367651950294046
/// End RTTI of "struct zeno::Vec3i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_1291108797552895579
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_1291108797552895579 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<3, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float>>::type>::type,struct zeno::_impl_vec::vec<3, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float>",
                1291108797552895579ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float>",
                1291108797552895579ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB3_floatRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB3_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(1291108797552895579, zeno::reflect::type_info<struct zeno::_impl_vec::vec<3, float>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB3_floatRABRegistrator global_Sstruct_zeno_implvec_vecLAB3_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec3f = 1291108797552895579ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_1291108797552895579
/// End RTTI of "struct zeno::_impl_vec::vec<3, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_5114418059158551365
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_5114418059158551365 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> &>::type>::type,struct zeno::_impl_vec::vec<3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> &",
                5114418059158551365ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> &",
                5114418059158551365ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_5114418059158551365
/// End RTTI of "struct zeno::_impl_vec::vec<3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_10945095583477475641
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_10945095583477475641 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> &&>::type>::type,struct zeno::_impl_vec::vec<3, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> &&",
                10945095583477475641ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> &&",
                10945095583477475641ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_10945095583477475641
/// End RTTI of "struct zeno::_impl_vec::vec<3, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_9096414431717339790
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_9096414431717339790 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, float> &>::type>::type,const struct zeno::_impl_vec::vec<3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, float> &",
                9096414431717339790ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, float> &",
                9096414431717339790ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_9096414431717339790
/// End RTTI of "const struct zeno::_impl_vec::vec<3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_Mul_9096410033670826946
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_Mul_9096410033670826946 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, float> *>::type>::type,const struct zeno::_impl_vec::vec<3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, float> *",
                9096410033670826946ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, float> *",
                9096410033670826946ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_floatRAB_Mul_9096410033670826946
/// End RTTI of "const struct zeno::_impl_vec::vec<3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_Mul_5114422457205064209
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_Mul_5114422457205064209 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> *>::type>::type,struct zeno::_impl_vec::vec<3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> *",
                5114422457205064209ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, float> *",
                5114422457205064209ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_floatRAB_Mul_5114422457205064209
/// End RTTI of "struct zeno::_impl_vec::vec<3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3f"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_12757428787220658269
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_12757428787220658269 1
namespace zeno {
struct Vec3f;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3f>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3f>::type>::type,struct zeno::Vec3f>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f",
                12757428787220658269ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f",
                12757428787220658269ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3f>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec3fRegistrator {
        _Sstruct_zeno_Vec3fRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12757428787220658269, zeno::reflect::type_info<struct zeno::Vec3f>());
        }
    };
    static _Sstruct_zeno_Vec3fRegistrator global_Sstruct_zeno_Vec3fRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec3f = 12757428787220658269ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_12757428787220658269
/// End RTTI of "struct zeno::Vec3f"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3f &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_7870235375457968755
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_7870235375457968755 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3f &>::type>::type,struct zeno::Vec3f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f &",
                7870235375457968755ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f &",
                7870235375457968755ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_7870235375457968755
/// End RTTI of "struct zeno::Vec3f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3f &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_8788699271526995567
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_8788699271526995567 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3f &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3f &&>::type>::type,struct zeno::Vec3f &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f &&",
                8788699271526995567ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f &&",
                8788699271526995567ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3f &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_8788699271526995567
/// End RTTI of "struct zeno::Vec3f &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec3f &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_35264535117933110
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_35264535117933110 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec3f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec3f &>::type>::type,const struct zeno::Vec3f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3f &",
                35264535117933110ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3f &",
                35264535117933110ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec3f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_35264535117933110
/// End RTTI of "const struct zeno::Vec3f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec3f *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_Mul_35277729257471642
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_Mul_35277729257471642 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec3f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec3f *>::type>::type,const struct zeno::Vec3f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3f *",
                35277729257471642ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec3f *",
                35277729257471642ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec3f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec3f_Mul_35277729257471642
/// End RTTI of "const struct zeno::Vec3f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec3f *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_Mul_7870248569597507287
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_Mul_7870248569597507287 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec3f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec3f *>::type>::type,struct zeno::Vec3f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f *",
                7870248569597507287ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec3f *",
                7870248569597507287ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec3f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec3f_Mul_7870248569597507287
/// End RTTI of "struct zeno::Vec3f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> >"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_7612059177550748566
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_7612059177550748566 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<3, class std::basic_string<char> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, class std::basic_string<char> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> >>::type>::type,struct zeno::_impl_vec::vec<3, class std::basic_string<char> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> >",
                7612059177550748566ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> >",
                7612059177550748566ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RABRegistrator {
        _Sstruct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(7612059177550748566, zeno::reflect::type_info<struct zeno::_impl_vec::vec<3, class std::basic_string<char> >>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RABRegistrator global_Sstruct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec3s = 7612059177550748566ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_7612059177550748566
/// End RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_16666429440376245228
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_16666429440376245228 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::type>::type,struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &",
                16666429440376245228ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &",
                16666429440376245228ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_16666429440376245228
/// End RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_13948589195073882174
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_13948589195073882174 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&>::type>::type,struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&",
                13948589195073882174ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&",
                13948589195073882174ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_13948589195073882174
/// End RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_8149892020530621233
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_8149892020530621233 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::type>::type,const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &",
                8149892020530621233ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &",
                8149892020530621233ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_8149892020530621233
/// End RTTI of "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_8149887622484108389
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_8149887622484108389 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::type>::type,const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *",
                8149887622484108389ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *",
                8149887622484108389ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_8149887622484108389
/// End RTTI of "const struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_16666433838422758072
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_16666433838422758072 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::type>::type,struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *",
                16666433838422758072ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *",
                16666433838422758072ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB3_class_std_basicstringLABcharRAB_RAB_Mul_16666433838422758072
/// End RTTI of "struct zeno::_impl_vec::vec<3, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, int>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_724601356907542639
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_724601356907542639 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<4, int>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, int>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int>>::type>::type,struct zeno::_impl_vec::vec<4, int>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int>",
                724601356907542639ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int>",
                724601356907542639ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB4_intRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB4_intRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(724601356907542639, zeno::reflect::type_info<struct zeno::_impl_vec::vec<4, int>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB4_intRABRegistrator global_Sstruct_zeno_implvec_vecLAB4_intRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec4i = 724601356907542639ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_724601356907542639
/// End RTTI of "struct zeno::_impl_vec::vec<4, int>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, int> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_14678328827275646945
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_14678328827275646945 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> &>::type>::type,struct zeno::_impl_vec::vec<4, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> &",
                14678328827275646945ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> &",
                14678328827275646945ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_14678328827275646945
/// End RTTI of "struct zeno::_impl_vec::vec<4, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, int> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_5100356474045228837
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_5100356474045228837 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, int> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> &&>::type>::type,struct zeno::_impl_vec::vec<4, int> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> &&",
                5100356474045228837ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> &&",
                5100356474045228837ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_5100356474045228837
/// End RTTI of "struct zeno::_impl_vec::vec<4, int> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, int> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_17503558991700914754
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_17503558991700914754 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, int> &>::type>::type,const struct zeno::_impl_vec::vec<4, int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, int> &",
                17503558991700914754ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, int> &",
                17503558991700914754ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_17503558991700914754
/// End RTTI of "const struct zeno::_impl_vec::vec<4, int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, int> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_Mul_17503563389747427598
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_Mul_17503563389747427598 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, int> *>::type>::type,const struct zeno::_impl_vec::vec<4, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, int> *",
                17503563389747427598ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, int> *",
                17503563389747427598ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_intRAB_Mul_17503563389747427598
/// End RTTI of "const struct zeno::_impl_vec::vec<4, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, int> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_Mul_14678324429229134101
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_Mul_14678324429229134101 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> *>::type>::type,struct zeno::_impl_vec::vec<4, int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> *",
                14678324429229134101ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, int> *",
                14678324429229134101ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_intRAB_Mul_14678324429229134101
/// End RTTI of "struct zeno::_impl_vec::vec<4, int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4i"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_12758300699941640367
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_12758300699941640367 1
namespace zeno {
struct Vec4i;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4i>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4i>::type>::type,struct zeno::Vec4i>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i",
                12758300699941640367ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i",
                12758300699941640367ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4i>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec4iRegistrator {
        _Sstruct_zeno_Vec4iRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12758300699941640367, zeno::reflect::type_info<struct zeno::Vec4i>());
        }
    };
    static _Sstruct_zeno_Vec4iRegistrator global_Sstruct_zeno_Vec4iRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec4i = 12758300699941640367ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_12758300699941640367
/// End RTTI of "struct zeno::Vec4i"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4i &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_8169497181227057697
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_8169497181227057697 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4i &>::type>::type,struct zeno::Vec4i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i &",
                8169497181227057697ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i &",
                8169497181227057697ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_8169497181227057697
/// End RTTI of "struct zeno::Vec4i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4i &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_4462992548126487013
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_4462992548126487013 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4i &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4i &&>::type>::type,struct zeno::Vec4i &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i &&",
                4462992548126487013ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i &&",
                4462992548126487013ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4i &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_4462992548126487013
/// End RTTI of "struct zeno::Vec4i &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec4i &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_857750731121949572
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_857750731121949572 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec4i &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec4i &>::type>::type,const struct zeno::Vec4i &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4i &",
                857750731121949572ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4i &",
                857750731121949572ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec4i &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_857750731121949572
/// End RTTI of "const struct zeno::Vec4i &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec4i *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_Mul_857737536982411040
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_Mul_857737536982411040 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec4i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec4i *>::type>::type,const struct zeno::Vec4i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4i *",
                857737536982411040ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4i *",
                857737536982411040ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec4i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4i_Mul_857737536982411040
/// End RTTI of "const struct zeno::Vec4i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4i *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_Mul_8169492783180544853
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_Mul_8169492783180544853 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4i *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4i *>::type>::type,struct zeno::Vec4i *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i *",
                8169492783180544853ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4i *",
                8169492783180544853ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4i *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4i_Mul_8169492783180544853
/// End RTTI of "struct zeno::Vec4i *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_5986645728587245802
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_5986645728587245802 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<4, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float>>::type>::type,struct zeno::_impl_vec::vec<4, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float>",
                5986645728587245802ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float>",
                5986645728587245802ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB4_floatRABRegistrator {
        _Sstruct_zeno_implvec_vecLAB4_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(5986645728587245802, zeno::reflect::type_info<struct zeno::_impl_vec::vec<4, float>>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB4_floatRABRegistrator global_Sstruct_zeno_implvec_vecLAB4_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec4f = 5986645728587245802ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_5986645728587245802
/// End RTTI of "struct zeno::_impl_vec::vec<4, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_566628881145308616
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_566628881145308616 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> &>::type>::type,struct zeno::_impl_vec::vec<4, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> &",
                566628881145308616ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> &",
                566628881145308616ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_566628881145308616
/// End RTTI of "struct zeno::_impl_vec::vec<4, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_9316668780275168106
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_9316668780275168106 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> &&>::type>::type,struct zeno::_impl_vec::vec<4, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> &&",
                9316668780275168106ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> &&",
                9316668780275168106ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_9316668780275168106
/// End RTTI of "struct zeno::_impl_vec::vec<4, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_433944387539666367
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_433944387539666367 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, float> &>::type>::type,const struct zeno::_impl_vec::vec<4, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, float> &",
                433944387539666367ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, float> &",
                433944387539666367ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_433944387539666367
/// End RTTI of "const struct zeno::_impl_vec::vec<4, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_Mul_433948785586179211
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_Mul_433948785586179211 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, float> *>::type>::type,const struct zeno::_impl_vec::vec<4, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, float> *",
                433948785586179211ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, float> *",
                433948785586179211ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_floatRAB_Mul_433948785586179211
/// End RTTI of "const struct zeno::_impl_vec::vec<4, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_Mul_566624483098795772
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_Mul_566624483098795772 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> *>::type>::type,struct zeno::_impl_vec::vec<4, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> *",
                566624483098795772ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, float> *",
                566624483098795772ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_floatRAB_Mul_566624483098795772
/// End RTTI of "struct zeno::_impl_vec::vec<4, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4f"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_12758306197499781422
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_12758306197499781422 1
namespace zeno {
struct Vec4f;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4f>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4f>::type>::type,struct zeno::Vec4f>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f",
                12758306197499781422ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f",
                12758306197499781422ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4f>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_Vec4fRegistrator {
        _Sstruct_zeno_Vec4fRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12758306197499781422, zeno::reflect::type_info<struct zeno::Vec4f>());
        }
    };
    static _Sstruct_zeno_Vec4fRegistrator global_Sstruct_zeno_Vec4fRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec4f = 12758306197499781422ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_12758306197499781422
/// End RTTI of "struct zeno::Vec4f"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4f &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_11290398264918044420
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_11290398264918044420 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4f &>::type>::type,struct zeno::Vec4f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f &",
                11290398264918044420ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f &",
                11290398264918044420ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_11290398264918044420
/// End RTTI of "struct zeno::Vec4f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4f &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_6826445747632527046
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_6826445747632527046 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4f &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4f &&>::type>::type,struct zeno::Vec4f &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f &&",
                6826445747632527046ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f &&",
                6826445747632527046ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4f &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_6826445747632527046
/// End RTTI of "struct zeno::Vec4f &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec4f &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_16183593721140514465
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_16183593721140514465 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec4f &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec4f &>::type>::type,const struct zeno::Vec4f &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4f &",
                16183593721140514465ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4f &",
                16183593721140514465ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec4f &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_16183593721140514465
/// End RTTI of "const struct zeno::Vec4f &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::Vec4f *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_Mul_16183589323094001621
#define _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_Mul_16183589323094001621 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::Vec4f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::Vec4f *>::type>::type,const struct zeno::Vec4f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4f *",
                16183589323094001621ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::Vec4f *",
                16183589323094001621ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::Vec4f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_Vec4f_Mul_16183589323094001621
/// End RTTI of "const struct zeno::Vec4f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::Vec4f *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_Mul_11290385070778505888
#define _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_Mul_11290385070778505888 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::Vec4f *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::Vec4f *>::type>::type,struct zeno::Vec4f *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f *",
                11290385070778505888ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::Vec4f *",
                11290385070778505888ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::Vec4f *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_Vec4f_Mul_11290385070778505888
/// End RTTI of "struct zeno::Vec4f *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> >"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8375677720103744597
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8375677720103744597 1
// !!! importance: This is a template specialization "struct zeno::_impl_vec::vec<4, class std::basic_string<char> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, class std::basic_string<char> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> >>::type>::type,struct zeno::_impl_vec::vec<4, class std::basic_string<char> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> >",
                8375677720103744597ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> >",
                8375677720103744597ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RABRegistrator {
        _Sstruct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8375677720103744597, zeno::reflect::type_info<struct zeno::_impl_vec::vec<4, class std::basic_string<char> >>());
        }
    };
    static _Sstruct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RABRegistrator global_Sstruct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec4s = 8375677720103744597ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8375677720103744597
/// End RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_7408569892634104299
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_7408569892634104299 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::type>::type,struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &",
                7408569892634104299ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &",
                7408569892634104299ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_7408569892634104299
/// End RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_15017057577923133271
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_15017057577923133271 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&>::type>::type,struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&",
                15017057577923133271ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&",
                15017057577923133271ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_15017057577923133271
/// End RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8113368083818801046
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8113368083818801046 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::type>::type,const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &",
                8113368083818801046ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &",
                8113368083818801046ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_8113368083818801046
/// End RTTI of "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_8113381277958339578
#define _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_8113381277958339578 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::type>::type,const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *",
                8113381277958339578ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *",
                8113381277958339578ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_8113381277958339578
/// End RTTI of "const struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_7408565494587591455
#define _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_7408565494587591455 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::type>::type,struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *",
                7408565494587591455ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *",
                7408565494587591455ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_implvec_vecLAB4_class_std_basicstringLABcharRAB_RAB_Mul_7408565494587591455
/// End RTTI of "struct zeno::_impl_vec::vec<4, class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<3, 3, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_16310354986700837126
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_16310354986700837126 1
// !!! importance: This is a template specialization "struct glm::mat<3, 3, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<3, 3, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float>>::type>::type,struct glm::mat<3, 3, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float>",
                16310354986700837126ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float>",
                16310354986700837126ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_glm_matLAB3_3_floatRABRegistrator {
        _Sstruct_glm_matLAB3_3_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16310354986700837126, zeno::reflect::type_info<struct glm::mat<3, 3, float>>());
        }
    };
    static _Sstruct_glm_matLAB3_3_floatRABRegistrator global_Sstruct_glm_matLAB3_3_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Matrix3 = 16310354986700837126ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_16310354986700837126
/// End RTTI of "struct glm::mat<3, 3, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<3, 3, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_7667824956873288412
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_7667824956873288412 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<3, 3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> &>::type>::type,struct glm::mat<3, 3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> &",
                7667824956873288412ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> &",
                7667824956873288412ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_7667824956873288412
/// End RTTI of "struct glm::mat<3, 3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<3, 3, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_2311796857357764302
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_2311796857357764302 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<3, 3, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> &&>::type>::type,struct glm::mat<3, 3, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> &&",
                2311796857357764302ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> &&",
                2311796857357764302ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_2311796857357764302
/// End RTTI of "struct glm::mat<3, 3, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::mat<3, 3, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_4528981001855611761
#define _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_4528981001855611761 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::mat<3, 3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::mat<3, 3, float> &>::type>::type,const struct glm::mat<3, 3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<3, 3, float> &",
                4528981001855611761ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<3, 3, float> &",
                4528981001855611761ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::mat<3, 3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_4528981001855611761
/// End RTTI of "const struct glm::mat<3, 3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::mat<3, 3, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_Mul_4528976603809098917
#define _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_Mul_4528976603809098917 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::mat<3, 3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::mat<3, 3, float> *>::type>::type,const struct glm::mat<3, 3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<3, 3, float> *",
                4528976603809098917ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<3, 3, float> *",
                4528976603809098917ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::mat<3, 3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_matLAB3_3_floatRAB_Mul_4528976603809098917
/// End RTTI of "const struct glm::mat<3, 3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<3, 3, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_Mul_7667829354919801256
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_Mul_7667829354919801256 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<3, 3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> *>::type>::type,struct glm::mat<3, 3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> *",
                7667829354919801256ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<3, 3, float> *",
                7667829354919801256ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<3, 3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB3_3_floatRAB_Mul_7667829354919801256
/// End RTTI of "struct glm::mat<3, 3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<4, 4, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_16993842308719517276
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_16993842308719517276 1
// !!! importance: This is a template specialization "struct glm::mat<4, 4, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<4, 4, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float>>::type>::type,struct glm::mat<4, 4, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float>",
                16993842308719517276ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float>",
                16993842308719517276ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_glm_matLAB4_4_floatRABRegistrator {
        _Sstruct_glm_matLAB4_4_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16993842308719517276, zeno::reflect::type_info<struct glm::mat<4, 4, float>>());
        }
    };
    static _Sstruct_glm_matLAB4_4_floatRABRegistrator global_Sstruct_glm_matLAB4_4_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Matrix4 = 16993842308719517276ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_16993842308719517276
/// End RTTI of "struct glm::mat<4, 4, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<4, 4, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_3323012302293670934
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_3323012302293670934 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<4, 4, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> &>::type>::type,struct glm::mat<4, 4, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> &",
                3323012302293670934ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> &",
                3323012302293670934ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_3323012302293670934
/// End RTTI of "struct glm::mat<4, 4, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<4, 4, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_15283130311840261520
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_15283130311840261520 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<4, 4, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> &&>::type>::type,struct glm::mat<4, 4, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> &&",
                15283130311840261520ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> &&",
                15283130311840261520ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_15283130311840261520
/// End RTTI of "struct glm::mat<4, 4, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::mat<4, 4, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_5872948055800936291
#define _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_5872948055800936291 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::mat<4, 4, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::mat<4, 4, float> &>::type>::type,const struct glm::mat<4, 4, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<4, 4, float> &",
                5872948055800936291ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<4, 4, float> &",
                5872948055800936291ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::mat<4, 4, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_5872948055800936291
/// End RTTI of "const struct glm::mat<4, 4, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::mat<4, 4, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_Mul_5872961249940474823
#define _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_Mul_5872961249940474823 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::mat<4, 4, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::mat<4, 4, float> *>::type>::type,const struct glm::mat<4, 4, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<4, 4, float> *",
                5872961249940474823ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::mat<4, 4, float> *",
                5872961249940474823ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::mat<4, 4, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_matLAB4_4_floatRAB_Mul_5872961249940474823
/// End RTTI of "const struct glm::mat<4, 4, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::mat<4, 4, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_Mul_3323025496433209466
#define _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_Mul_3323025496433209466 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::mat<4, 4, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> *>::type>::type,struct glm::mat<4, 4, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> *",
                3323025496433209466ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::mat<4, 4, float> *",
                3323025496433209466ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::mat<4, 4, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_matLAB4_4_floatRAB_Mul_3323025496433209466
/// End RTTI of "struct glm::mat<4, 4, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::vec<3, float>"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_2551243981112912689
#define _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_2551243981112912689 1
// !!! importance: This is a template specialization "struct glm::vec<3, float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::vec<3, float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::vec<3, float>>::type>::type,struct glm::vec<3, float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float>",
                2551243981112912689ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float>",
                2551243981112912689ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::vec<3, float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_glm_vecLAB3_floatRABRegistrator {
        _Sstruct_glm_vecLAB3_floatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(2551243981112912689, zeno::reflect::type_info<struct glm::vec<3, float>>());
        }
    };
    static _Sstruct_glm_vecLAB3_floatRABRegistrator global_Sstruct_glm_vecLAB3_floatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_GLMVec3 = 2551243981112912689ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_2551243981112912689
/// End RTTI of "struct glm::vec<3, float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::vec<3, float> &"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_8524234399107204543
#define _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_8524234399107204543 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::vec<3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::vec<3, float> &>::type>::type,struct glm::vec<3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> &",
                8524234399107204543ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> &",
                8524234399107204543ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::vec<3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_8524234399107204543
/// End RTTI of "struct glm::vec<3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::vec<3, float> &&"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_17775990145902658299
#define _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_17775990145902658299 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::vec<3, float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::vec<3, float> &&>::type>::type,struct glm::vec<3, float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> &&",
                17775990145902658299ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> &&",
                17775990145902658299ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::vec<3, float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_17775990145902658299
/// End RTTI of "struct glm::vec<3, float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::vec<3, float> &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_78970757044426552
#define _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_78970757044426552 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::vec<3, float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::vec<3, float> &>::type>::type,const struct glm::vec<3, float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::vec<3, float> &",
                78970757044426552ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::vec<3, float> &",
                78970757044426552ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::vec<3, float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_78970757044426552
/// End RTTI of "const struct glm::vec<3, float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct glm::vec<3, float> *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_Mul_78966358997913708
#define _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_Mul_78966358997913708 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct glm::vec<3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct glm::vec<3, float> *>::type>::type,const struct glm::vec<3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::vec<3, float> *",
                78966358997913708ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct glm::vec<3, float> *",
                78966358997913708ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct glm::vec<3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_glm_vecLAB3_floatRAB_Mul_78966358997913708
/// End RTTI of "const struct glm::vec<3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct glm::vec<3, float> *"
#ifndef _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_Mul_8524238797153717387
#define _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_Mul_8524238797153717387 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct glm::vec<3, float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct glm::vec<3, float> *>::type>::type,struct glm::vec<3, float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> *",
                8524238797153717387ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct glm::vec<3, float> *",
                8524238797153717387ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct glm::vec<3, float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_glm_vecLAB3_floatRAB_Mul_8524238797153717387
/// End RTTI of "struct glm::vec<3, float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::basic_string<char> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8472269399861637118
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8472269399861637118 1
// !!! importance: This is a template specialization "class std::vector<class std::basic_string<char> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::basic_string<char> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> >>::type>::type,class std::vector<class std::basic_string<char> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> >",
                8472269399861637118ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> >",
                8472269399861637118ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABclass_std_basicstringLABcharRAB_RABRegistrator {
        _Sclass_std_vectorLABclass_std_basicstringLABcharRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8472269399861637118, zeno::reflect::type_info<class std::vector<class std::basic_string<char> >>());
        }
    };
    static _Sclass_std_vectorLABclass_std_basicstringLABcharRAB_RABRegistrator global_Sclass_std_vectorLABclass_std_basicstringLABcharRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_StringList = 8472269399861637118ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8472269399861637118
/// End RTTI of "class std::vector<class std::basic_string<char> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_2365152962125189524
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_2365152962125189524 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > &>::type>::type,class std::vector<class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > &",
                2365152962125189524ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > &",
                2365152962125189524ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_2365152962125189524
/// End RTTI of "class std::vector<class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::basic_string<char> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_13068630557970649462
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_13068630557970649462 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::basic_string<char> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > &&>::type>::type,class std::vector<class std::basic_string<char> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > &&",
                13068630557970649462ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > &&",
                13068630557970649462ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_13068630557970649462
/// End RTTI of "class std::vector<class std::basic_string<char> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class std::basic_string<char> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8941804213459840763
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8941804213459840763 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class std::basic_string<char> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class std::basic_string<char> > &>::type>::type,const class std::vector<class std::basic_string<char> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::basic_string<char> > &",
                8941804213459840763ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::basic_string<char> > &",
                8941804213459840763ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class std::basic_string<char> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_8941804213459840763
/// End RTTI of "const class std::vector<class std::basic_string<char> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_8941799815413327919
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_8941799815413327919 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class std::basic_string<char> > *>::type>::type,const class std::vector<class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::basic_string<char> > *",
                8941799815413327919ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::basic_string<char> > *",
                8941799815413327919ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_8941799815413327919
/// End RTTI of "const class std::vector<class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::basic_string<char> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_2365139767985650992
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_2365139767985650992 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::basic_string<char> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > *>::type>::type,class std::vector<class std::basic_string<char> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > *",
                2365139767985650992ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::basic_string<char> > *",
                2365139767985650992ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::basic_string<char> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_basicstringLABcharRAB_RAB_Mul_2365139767985650992
/// End RTTI of "class std::vector<class std::basic_string<char> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<class zeno::String>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_6422359787960396687
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_6422359787960396687 1
// !!! importance: This is a template specialization "class zeno::Vector<class zeno::String>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<class zeno::String>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String>>::type>::type,class zeno::Vector<class zeno::String>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String>",
                6422359787960396687ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String>",
                6422359787960396687ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABclass_zeno_StringRABRegistrator {
        _Sclass_zeno_VectorLABclass_zeno_StringRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(6422359787960396687, zeno::reflect::type_info<class zeno::Vector<class zeno::String>>());
        }
    };
    static _Sclass_zeno_VectorLABclass_zeno_StringRABRegistrator global_Sclass_zeno_VectorLABclass_zeno_StringRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiStringList = 6422359787960396687ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_6422359787960396687
/// End RTTI of "class zeno::Vector<class zeno::String>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<class zeno::String> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_4644249792292658945
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_4644249792292658945 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<class zeno::String> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> &>::type>::type,class zeno::Vector<class zeno::String> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> &",
                4644249792292658945ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> &",
                4644249792292658945ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_4644249792292658945
/// End RTTI of "class zeno::Vector<class zeno::String> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<class zeno::String> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_5599156546111008581
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_5599156546111008581 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<class zeno::String> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> &&>::type>::type,class zeno::Vector<class zeno::String> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> &&",
                5599156546111008581ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> &&",
                5599156546111008581ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_5599156546111008581
/// End RTTI of "class zeno::Vector<class zeno::String> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<class zeno::String> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_12226458265119155872
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_12226458265119155872 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<class zeno::String> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<class zeno::String> &>::type>::type,const class zeno::Vector<class zeno::String> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<class zeno::String> &",
                12226458265119155872ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<class zeno::String> &",
                12226458265119155872ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<class zeno::String> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_12226458265119155872
/// End RTTI of "const class zeno::Vector<class zeno::String> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<class zeno::String> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_Mul_12226471459258694404
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_Mul_12226471459258694404 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<class zeno::String> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<class zeno::String> *>::type>::type,const class zeno::Vector<class zeno::String> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<class zeno::String> *",
                12226471459258694404ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<class zeno::String> *",
                12226471459258694404ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<class zeno::String> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABclass_zeno_StringRAB_Mul_12226471459258694404
/// End RTTI of "const class zeno::Vector<class zeno::String> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<class zeno::String> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_Mul_4644245394246146101
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_Mul_4644245394246146101 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<class zeno::String> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> *>::type>::type,class zeno::Vector<class zeno::String> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> *",
                4644245394246146101ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<class zeno::String> *",
                4644245394246146101ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<class zeno::String> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABclass_zeno_StringRAB_Mul_4644245394246146101
/// End RTTI of "class zeno::Vector<class zeno::String> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<int>"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_11332342544482872366
#define _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_11332342544482872366 1
// !!! importance: This is a template specialization "class std::vector<int>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<int>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<int>>::type>::type,class std::vector<int>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int>",
                11332342544482872366ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int>",
                11332342544482872366ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<int>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABintRABRegistrator {
        _Sclass_std_vectorLABintRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(11332342544482872366, zeno::reflect::type_info<class std::vector<int>>());
        }
    };
    static _Sclass_std_vectorLABintRABRegistrator global_Sclass_std_vectorLABintRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_IntList = 11332342544482872366ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_11332342544482872366
/// End RTTI of "class std::vector<int>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<int> &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_15328464289661596164
#define _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_15328464289661596164 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<int> &>::type>::type,class std::vector<int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> &",
                15328464289661596164ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> &",
                15328464289661596164ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_15328464289661596164
/// End RTTI of "class std::vector<int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<int> &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_118670429401591750
#define _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_118670429401591750 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<int> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<int> &&>::type>::type,class std::vector<int> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> &&",
                118670429401591750ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> &&",
                118670429401591750ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<int> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_118670429401591750
/// End RTTI of "class std::vector<int> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<int> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_4474395814862989361
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_4474395814862989361 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<int> &>::type>::type,const class std::vector<int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<int> &",
                4474395814862989361ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<int> &",
                4474395814862989361ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_4474395814862989361
/// End RTTI of "const class std::vector<int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<int> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_Mul_4474391416816476517
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_Mul_4474391416816476517 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<int> *>::type>::type,const class std::vector<int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<int> *",
                4474391416816476517ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<int> *",
                4474391416816476517ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABintRAB_Mul_4474391416816476517
/// End RTTI of "const class std::vector<int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<int> *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_Mul_15328451095522057632
#define _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_Mul_15328451095522057632 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<int> *>::type>::type,class std::vector<int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> *",
                15328451095522057632ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<int> *",
                15328451095522057632ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABintRAB_Mul_15328451095522057632
/// End RTTI of "class std::vector<int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<int>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_5294446623426350797
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_5294446623426350797 1
// !!! importance: This is a template specialization "class zeno::Vector<int>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<int>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<int>>::type>::type,class zeno::Vector<int>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int>",
                5294446623426350797ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int>",
                5294446623426350797ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<int>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABintRABRegistrator {
        _Sclass_zeno_VectorLABintRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(5294446623426350797, zeno::reflect::type_info<class zeno::Vector<int>>());
        }
    };
    static _Sclass_zeno_VectorLABintRABRegistrator global_Sclass_zeno_VectorLABintRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiIntList = 5294446623426350797ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_5294446623426350797
/// End RTTI of "class zeno::Vector<int>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<int> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_13022999964374026851
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_13022999964374026851 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<int> &>::type>::type,class zeno::Vector<int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> &",
                13022999964374026851ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> &",
                13022999964374026851ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_13022999964374026851
/// End RTTI of "class zeno::Vector<int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<int> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_9945909420696154943
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_9945909420696154943 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<int> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<int> &&>::type>::type,class zeno::Vector<int> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> &&",
                9945909420696154943ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> &&",
                9945909420696154943ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<int> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_9945909420696154943
/// End RTTI of "class zeno::Vector<int> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<int> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_16864831801774663028
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_16864831801774663028 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<int> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<int> &>::type>::type,const class zeno::Vector<int> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<int> &",
                16864831801774663028ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<int> &",
                16864831801774663028ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<int> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_16864831801774663028
/// End RTTI of "const class zeno::Vector<int> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<int> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_Mul_16864818607635124496
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_Mul_16864818607635124496 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<int> *>::type>::type,const class zeno::Vector<int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<int> *",
                16864818607635124496ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<int> *",
                16864818607635124496ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABintRAB_Mul_16864818607635124496
/// End RTTI of "const class zeno::Vector<int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<int> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_Mul_13023013158513565383
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_Mul_13023013158513565383 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<int> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<int> *>::type>::type,class zeno::Vector<int> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> *",
                13023013158513565383ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<int> *",
                13023013158513565383ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<int> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABintRAB_Mul_13023013158513565383
/// End RTTI of "class zeno::Vector<int> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<float>"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_8862306467697783251
#define _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_8862306467697783251 1
// !!! importance: This is a template specialization "class std::vector<float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<float>>::type>::type,class std::vector<float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float>",
                8862306467697783251ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float>",
                8862306467697783251ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABfloatRABRegistrator {
        _Sclass_std_vectorLABfloatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8862306467697783251, zeno::reflect::type_info<class std::vector<float>>());
        }
    };
    static _Sclass_std_vectorLABfloatRABRegistrator global_Sclass_std_vectorLABfloatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_FloatList = 8862306467697783251ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_8862306467697783251
/// End RTTI of "class std::vector<float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<float> &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_7872655411913166013
#define _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_7872655411913166013 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<float> &>::type>::type,class std::vector<float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> &",
                7872655411913166013ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> &",
                7872655411913166013ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_7872655411913166013
/// End RTTI of "class std::vector<float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<float> &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_14555072055348390753
#define _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_14555072055348390753 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<float> &&>::type>::type,class std::vector<float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> &&",
                14555072055348390753ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> &&",
                14555072055348390753ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_14555072055348390753
/// End RTTI of "class std::vector<float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<float> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_5923678845714885592
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_5923678845714885592 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<float> &>::type>::type,const class std::vector<float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<float> &",
                5923678845714885592ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<float> &",
                5923678845714885592ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_5923678845714885592
/// End RTTI of "const class std::vector<float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<float> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_Mul_5923674447668372748
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_Mul_5923674447668372748 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<float> *>::type>::type,const class std::vector<float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<float> *",
                5923674447668372748ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<float> *",
                5923674447668372748ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABfloatRAB_Mul_5923674447668372748
/// End RTTI of "const class std::vector<float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<float> *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_Mul_7872642217773627481
#define _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_Mul_7872642217773627481 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<float> *>::type>::type,class std::vector<float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> *",
                7872642217773627481ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<float> *",
                7872642217773627481ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABfloatRAB_Mul_7872642217773627481
/// End RTTI of "class std::vector<float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<float>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_131869058210785820
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_131869058210785820 1
// !!! importance: This is a template specialization "class zeno::Vector<float>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<float>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<float>>::type>::type,class zeno::Vector<float>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float>",
                131869058210785820ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float>",
                131869058210785820ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<float>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABfloatRABRegistrator {
        _Sclass_zeno_VectorLABfloatRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(131869058210785820, zeno::reflect::type_info<class zeno::Vector<float>>());
        }
    };
    static _Sclass_zeno_VectorLABfloatRABRegistrator global_Sclass_zeno_VectorLABfloatRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiFloatList = 131869058210785820ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_131869058210785820
/// End RTTI of "class zeno::Vector<float>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<float> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_7450363161895796182
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_7450363161895796182 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<float> &>::type>::type,class zeno::Vector<float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> &",
                7450363161895796182ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> &",
                7450363161895796182ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_7450363161895796182
/// End RTTI of "class zeno::Vector<float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<float> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_2675147615069404880
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_2675147615069404880 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<float> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<float> &&>::type>::type,class zeno::Vector<float> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> &&",
                2675147615069404880ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> &&",
                2675147615069404880ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<float> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_2675147615069404880
/// End RTTI of "class zeno::Vector<float> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<float> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_16092859679938708973
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_16092859679938708973 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<float> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<float> &>::type>::type,const class zeno::Vector<float> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<float> &",
                16092859679938708973ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<float> &",
                16092859679938708973ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<float> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_16092859679938708973
/// End RTTI of "const class zeno::Vector<float> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<float> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_Mul_16092846485799170441
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_Mul_16092846485799170441 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<float> *>::type>::type,const class zeno::Vector<float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<float> *",
                16092846485799170441ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<float> *",
                16092846485799170441ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABfloatRAB_Mul_16092846485799170441
/// End RTTI of "const class zeno::Vector<float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<float> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_Mul_7450376356035334714
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_Mul_7450376356035334714 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<float> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<float> *>::type>::type,class zeno::Vector<float> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> *",
                7450376356035334714ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<float> *",
                7450376356035334714ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<float> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABfloatRAB_Mul_7450376356035334714
/// End RTTI of "class zeno::Vector<float> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_352580941733667956
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_352580941733667956 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<2, float> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, float> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, float> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> >",
                352580941733667956ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> >",
                352580941733667956ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(352580941733667956, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<2, float> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec2fList = 352580941733667956ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_352580941733667956
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_12582234645242549166
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_12582234645242549166 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > &",
                12582234645242549166ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > &",
                12582234645242549166ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_12582234645242549166
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_34926696774479896
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_34926696774479896 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, float> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, float> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > &&",
                34926696774479896ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > &&",
                34926696774479896ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_34926696774479896
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, float> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_17610063105952861277
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_17610063105952861277 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<2, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, float> > &",
                17610063105952861277ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, float> > &",
                17610063105952861277ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_17610063105952861277
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, float> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_17610049911813322745
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_17610049911813322745 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<2, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, float> > *",
                17610049911813322745ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, float> > *",
                17610049911813322745ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_17610049911813322745
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_12582230247196036322
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_12582230247196036322 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > *",
                12582230247196036322ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, float> > *",
                12582230247196036322ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_floatRAB_RAB_Mul_12582230247196036322
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2f>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_3723469020882266465
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_3723469020882266465 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec2f>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2f>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f>>::type>::type,class zeno::Vector<struct zeno::Vec2f>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f>",
                3723469020882266465ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f>",
                3723469020882266465ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec2fRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec2fRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(3723469020882266465, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec2f>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec2fRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec2fRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec2fList = 3723469020882266465ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_3723469020882266465
/// End RTTI of "class zeno::Vector<struct zeno::Vec2f>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2f> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_6571087022135364463
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_6571087022135364463 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> &>::type>::type,class zeno::Vector<struct zeno::Vec2f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> &",
                6571087022135364463ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> &",
                6571087022135364463ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_6571087022135364463
/// End RTTI of "class zeno::Vector<struct zeno::Vec2f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2f> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_9466921241909882123
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_9466921241909882123 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2f> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> &&>::type>::type,class zeno::Vector<struct zeno::Vec2f> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> &&",
                9466921241909882123ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> &&",
                9466921241909882123ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_9466921241909882123
/// End RTTI of "class zeno::Vector<struct zeno::Vec2f> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec2f> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_503853730876782106
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_503853730876782106 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec2f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2f> &>::type>::type,const class zeno::Vector<struct zeno::Vec2f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2f> &",
                503853730876782106ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2f> &",
                503853730876782106ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_503853730876782106
/// End RTTI of "const class zeno::Vector<struct zeno::Vec2f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec2f> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_503840536737243574
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_503840536737243574 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec2f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2f> *>::type>::type,const class zeno::Vector<struct zeno::Vec2f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2f> *",
                503840536737243574ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2f> *",
                503840536737243574ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_503840536737243574
/// End RTTI of "const class zeno::Vector<struct zeno::Vec2f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2f> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_6571091420181877307
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_6571091420181877307 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> *>::type>::type,class zeno::Vector<struct zeno::Vec2f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> *",
                6571091420181877307ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2f> *",
                6571091420181877307ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2fRAB_Mul_6571091420181877307
/// End RTTI of "class zeno::Vector<struct zeno::Vec2f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_16802479141220513467
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_16802479141220513467 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<3, float> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, float> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> >",
                16802479141220513467ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> >",
                16802479141220513467ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16802479141220513467, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec3fList = 16802479141220513467ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_16802479141220513467
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_15005525077449753509
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_15005525077449753509 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > &",
                15005525077449753509ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > &",
                15005525077449753509ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_15005525077449753509
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_8479965329822136217
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_8479965329822136217 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, float> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > &&",
                8479965329822136217ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > &&",
                8479965329822136217ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_8479965329822136217
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, float> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_11343981422349378854
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_11343981422349378854 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<3, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, float> > &",
                11343981422349378854ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, float> > &",
                11343981422349378854ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_11343981422349378854
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, float> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_11343994616488917386
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_11343994616488917386 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<3, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, float> > *",
                11343994616488917386ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, float> > *",
                11343994616488917386ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_11343994616488917386
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_15005529475496266353
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_15005529475496266353 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > *",
                15005529475496266353ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, float> > *",
                15005529475496266353ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_floatRAB_RAB_Mul_15005529475496266353
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3f>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_2965409629491681554
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_2965409629491681554 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec3f>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3f>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f>>::type>::type,class zeno::Vector<struct zeno::Vec3f>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f>",
                2965409629491681554ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f>",
                2965409629491681554ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec3fRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec3fRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(2965409629491681554, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec3f>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec3fRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec3fRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec3fList = 2965409629491681554ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_2965409629491681554
/// End RTTI of "class zeno::Vector<struct zeno::Vec3f>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3f> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_11684933499308269168
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_11684933499308269168 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> &>::type>::type,class zeno::Vector<struct zeno::Vec3f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> &",
                11684933499308269168ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> &",
                11684933499308269168ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_11684933499308269168
/// End RTTI of "class zeno::Vector<struct zeno::Vec3f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3f> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_16206871830986083362
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_16206871830986083362 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3f> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> &&>::type>::type,class zeno::Vector<struct zeno::Vec3f> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> &&",
                16206871830986083362ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> &&",
                16206871830986083362ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_16206871830986083362
/// End RTTI of "class zeno::Vector<struct zeno::Vec3f> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec3f> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_12744568190939222329
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_12744568190939222329 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec3f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3f> &>::type>::type,const class zeno::Vector<struct zeno::Vec3f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3f> &",
                12744568190939222329ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3f> &",
                12744568190939222329ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_12744568190939222329
/// End RTTI of "const class zeno::Vector<struct zeno::Vec3f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec3f> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_12744581385078760861
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_12744581385078760861 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec3f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3f> *>::type>::type,const class zeno::Vector<struct zeno::Vec3f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3f> *",
                12744581385078760861ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3f> *",
                12744581385078760861ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_12744581385078760861
/// End RTTI of "const class zeno::Vector<struct zeno::Vec3f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3f> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_11684946693447807700
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_11684946693447807700 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> *>::type>::type,class zeno::Vector<struct zeno::Vec3f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> *",
                11684946693447807700ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3f> *",
                11684946693447807700ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3fRAB_Mul_11684946693447807700
/// End RTTI of "class zeno::Vector<struct zeno::Vec3f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_14658396477978350594
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_14658396477978350594 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<4, float> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, float> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, float> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> >",
                14658396477978350594ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> >",
                14658396477978350594ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(14658396477978350594, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<4, float> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec4fList = 14658396477978350594ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_14658396477978350594
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_6500045928564657056
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_6500045928564657056 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > &",
                6500045928564657056ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > &",
                6500045928564657056ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_6500045928564657056
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_1856729881621664946
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_1856729881621664946 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, float> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, float> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > &&",
                1856729881621664946ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > &&",
                1856729881621664946ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_1856729881621664946
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, float> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_11079425167383808335
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_11079425167383808335 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<4, float> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, float> > &",
                11079425167383808335ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, float> > &",
                11079425167383808335ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, float> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_11079425167383808335
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, float> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, float> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_11079429565430321179
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_11079429565430321179 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<4, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, float> > *",
                11079429565430321179ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, float> > *",
                11079429565430321179ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_11079429565430321179
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_6500059122704195588
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_6500059122704195588 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, float> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > *",
                6500059122704195588ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, float> > *",
                6500059122704195588ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, float> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_floatRAB_RAB_Mul_6500059122704195588
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, float> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4f>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5021581036111415351
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5021581036111415351 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec4f>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4f>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f>>::type>::type,class zeno::Vector<struct zeno::Vec4f>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f>",
                5021581036111415351ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f>",
                5021581036111415351ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec4fRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec4fRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(5021581036111415351, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec4f>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec4fRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec4fRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec4fList = 5021581036111415351ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5021581036111415351
/// End RTTI of "class zeno::Vector<struct zeno::Vec4f>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4f> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_3120837157199553449
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_3120837157199553449 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> &>::type>::type,class zeno::Vector<struct zeno::Vec4f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> &",
                3120837157199553449ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> &",
                3120837157199553449ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_3120837157199553449
/// End RTTI of "class zeno::Vector<struct zeno::Vec4f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4f> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5773708084625304573
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5773708084625304573 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4f> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> &&>::type>::type,class zeno::Vector<struct zeno::Vec4f> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> &&",
                5773708084625304573ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> &&",
                5773708084625304573ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_5773708084625304573
/// End RTTI of "class zeno::Vector<struct zeno::Vec4f> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec4f> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_2355556142573455488
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_2355556142573455488 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec4f> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4f> &>::type>::type,const class zeno::Vector<struct zeno::Vec4f> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4f> &",
                2355556142573455488ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4f> &",
                2355556142573455488ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4f> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_2355556142573455488
/// End RTTI of "const class zeno::Vector<struct zeno::Vec4f> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec4f> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_2355569336712994020
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_2355569336712994020 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec4f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4f> *>::type>::type,const class zeno::Vector<struct zeno::Vec4f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4f> *",
                2355569336712994020ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4f> *",
                2355569336712994020ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_2355569336712994020
/// End RTTI of "const class zeno::Vector<struct zeno::Vec4f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4f> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_3120850351339091981
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_3120850351339091981 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4f> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> *>::type>::type,class zeno::Vector<struct zeno::Vec4f> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> *",
                3120850351339091981ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4f> *",
                3120850351339091981ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4f> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4fRAB_Mul_3120850351339091981
/// End RTTI of "class zeno::Vector<struct zeno::Vec4f> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8437938278359228333
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8437938278359228333 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<2, int> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, int> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, int> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> >",
                8437938278359228333ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> >",
                8437938278359228333ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8437938278359228333, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<2, int> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec2iList = 8437938278359228333ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8437938278359228333
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_973569058991833283
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_973569058991833283 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > &",
                973569058991833283ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > &",
                973569058991833283ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_973569058991833283
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8811338761334981919
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8811338761334981919 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, int> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, int> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > &&",
                8811338761334981919ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > &&",
                8811338761334981919ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_8811338761334981919
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, int> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_7086832747635887036
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_7086832747635887036 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<2, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, int> > &",
                7086832747635887036ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, int> > &",
                7086832747635887036ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_7086832747635887036
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, int> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_7086837145682399880
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_7086837145682399880 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<2, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, int> > *",
                7086837145682399880ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<2, int> > *",
                7086837145682399880ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_7086837145682399880
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<2, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_973582253131371815
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_973582253131371815 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<2, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > *",
                973582253131371815ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<2, int> > *",
                973582253131371815ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<2, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB2_intRAB_RAB_Mul_973582253131371815
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<2, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2i>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_3724451984277697874
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_3724451984277697874 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec2i>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2i>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i>>::type>::type,class zeno::Vector<struct zeno::Vec2i>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i>",
                3724451984277697874ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i>",
                3724451984277697874ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec2iRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec2iRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(3724451984277697874, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec2i>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec2iRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec2iRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec2iList = 3724451984277697874ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_3724451984277697874
/// End RTTI of "class zeno::Vector<struct zeno::Vec2i>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2i> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_14693236628915037360
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_14693236628915037360 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> &>::type>::type,class zeno::Vector<struct zeno::Vec2i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> &",
                14693236628915037360ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> &",
                14693236628915037360ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_14693236628915037360
/// End RTTI of "class zeno::Vector<struct zeno::Vec2i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2i> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_13390295336755735266
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_13390295336755735266 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2i> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> &&>::type>::type,class zeno::Vector<struct zeno::Vec2i> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> &&",
                13390295336755735266ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> &&",
                13390295336755735266ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_13390295336755735266
/// End RTTI of "class zeno::Vector<struct zeno::Vec2i> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec2i> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_12109845860866775669
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_12109845860866775669 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec2i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2i> &>::type>::type,const class zeno::Vector<struct zeno::Vec2i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2i> &",
                12109845860866775669ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2i> &",
                12109845860866775669ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_12109845860866775669
/// End RTTI of "const class zeno::Vector<struct zeno::Vec2i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec2i> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_12109850258913288513
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_12109850258913288513 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec2i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2i> *>::type>::type,const class zeno::Vector<struct zeno::Vec2i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2i> *",
                12109850258913288513ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec2i> *",
                12109850258913288513ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec2i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_12109850258913288513
/// End RTTI of "const class zeno::Vector<struct zeno::Vec2i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec2i> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_14693249823054575892
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_14693249823054575892 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec2i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> *>::type>::type,class zeno::Vector<struct zeno::Vec2i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> *",
                14693249823054575892ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec2i> *",
                14693249823054575892ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec2i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec2iRAB_Mul_14693249823054575892
/// End RTTI of "class zeno::Vector<struct zeno::Vec2i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_12969379980554923694
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_12969379980554923694 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<3, int> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, int> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, int> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> >",
                12969379980554923694ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> >",
                12969379980554923694ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(12969379980554923694, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<3, int> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec3iList = 12969379980554923694ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_12969379980554923694
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_8428484872552274052
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_8428484872552274052 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > &",
                8428484872552274052ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > &",
                8428484872552274052ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_8428484872552274052
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_18211938932819991366
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_18211938932819991366 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, int> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, int> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > &&",
                18211938932819991366ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > &&",
                18211938932819991366ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_18211938932819991366
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, int> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_3851573488467759227
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_3851573488467759227 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<3, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, int> > &",
                3851573488467759227ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, int> > &",
                3851573488467759227ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_3851573488467759227
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, int> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_3851569090421246383
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_3851569090421246383 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<3, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, int> > *",
                3851569090421246383ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<3, int> > *",
                3851569090421246383ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_3851569090421246383
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<3, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_8428471678412735520
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_8428471678412735520 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<3, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > *",
                8428471678412735520ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<3, int> > *",
                8428471678412735520ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<3, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB3_intRAB_RAB_Mul_8428471678412735520
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<3, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3i>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_2964426666096250145
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_2964426666096250145 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec3i>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3i>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i>>::type>::type,class zeno::Vector<struct zeno::Vec3i>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i>",
                2964426666096250145ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i>",
                2964426666096250145ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec3iRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec3iRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(2964426666096250145, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec3i>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec3iRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec3iRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec3iList = 2964426666096250145ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_2964426666096250145
/// End RTTI of "class zeno::Vector<struct zeno::Vec3i>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3i> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3562783892528596271
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3562783892528596271 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> &>::type>::type,class zeno::Vector<struct zeno::Vec3i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> &",
                3562783892528596271ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> &",
                3562783892528596271ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3562783892528596271
/// End RTTI of "class zeno::Vector<struct zeno::Vec3i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3i> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_12283497736140230219
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_12283497736140230219 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3i> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> &&>::type>::type,class zeno::Vector<struct zeno::Vec3i> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> &&",
                12283497736140230219ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> &&",
                12283497736140230219ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_12283497736140230219
/// End RTTI of "class zeno::Vector<struct zeno::Vec3i> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec3i> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3517726556915475590
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3517726556915475590 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec3i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3i> &>::type>::type,const class zeno::Vector<struct zeno::Vec3i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3i> &",
                3517726556915475590ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3i> &",
                3517726556915475590ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_3517726556915475590
/// End RTTI of "const class zeno::Vector<struct zeno::Vec3i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec3i> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3517739751055014122
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3517739751055014122 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec3i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3i> *>::type>::type,const class zeno::Vector<struct zeno::Vec3i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3i> *",
                3517739751055014122ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec3i> *",
                3517739751055014122ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec3i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3517739751055014122
/// End RTTI of "const class zeno::Vector<struct zeno::Vec3i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec3i> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3562788290575109115
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3562788290575109115 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec3i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> *>::type>::type,class zeno::Vector<struct zeno::Vec3i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> *",
                3562788290575109115ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec3i> *",
                3562788290575109115ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec3i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec3iRAB_Mul_3562788290575109115
/// End RTTI of "class zeno::Vector<struct zeno::Vec3i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_8958441701489594431
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_8958441701489594431 1
// !!! importance: This is a template specialization "class std::vector<struct zeno::_impl_vec::vec<4, int> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, int> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> >>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, int> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> >",
                8958441701489594431ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> >",
                8958441701489594431ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RABRegistrator {
        _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8958441701489594431, zeno::reflect::type_info<class std::vector<struct zeno::_impl_vec::vec<4, int> >>());
        }
    };
    static _Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RABRegistrator global_Sclass_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Vec4iList = 8958441701489594431ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_8958441701489594431
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_14754627456232265777
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_14754627456232265777 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > &",
                14754627456232265777ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > &",
                14754627456232265777ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_14754627456232265777
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_16312781849536622357
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_16312781849536622357 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, int> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > &&>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, int> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > &&",
                16312781849536622357ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > &&",
                16312781849536622357ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_16312781849536622357
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, int> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_2854055192306113690
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_2854055192306113690 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<4, int> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, int> > &",
                2854055192306113690ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, int> > &",
                2854055192306113690ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, int> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_2854055192306113690
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, int> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, int> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_2854041998166575158
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_2854041998166575158 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<struct zeno::_impl_vec::vec<4, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::type>::type,const class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, int> > *",
                2854041998166575158ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<struct zeno::_impl_vec::vec<4, int> > *",
                2854041998166575158ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_2854041998166575158
/// End RTTI of "const class std::vector<struct zeno::_impl_vec::vec<4, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_14754623058185752933
#define _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_14754623058185752933 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<struct zeno::_impl_vec::vec<4, int> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::type>::type,class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > *",
                14754623058185752933ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<struct zeno::_impl_vec::vec<4, int> > *",
                14754623058185752933ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<struct zeno::_impl_vec::vec<4, int> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABstruct_zeno_implvec_vecLAB4_intRAB_RAB_Mul_14754623058185752933
/// End RTTI of "class std::vector<struct zeno::_impl_vec::vec<4, int> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4i>"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_5033963736065867708
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_5033963736065867708 1
// !!! importance: This is a template specialization "class zeno::Vector<struct zeno::Vec4i>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4i>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i>>::type>::type,class zeno::Vector<struct zeno::Vec4i>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i>",
                5033963736065867708ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i>",
                5033963736065867708ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_zeno_VectorLABstruct_zeno_Vec4iRABRegistrator {
        _Sclass_zeno_VectorLABstruct_zeno_Vec4iRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(5033963736065867708, zeno::reflect::type_info<class zeno::Vector<struct zeno::Vec4i>>());
        }
    };
    static _Sclass_zeno_VectorLABstruct_zeno_Vec4iRABRegistrator global_Sclass_zeno_VectorLABstruct_zeno_Vec4iRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AbiVec4iList = 5033963736065867708ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_5033963736065867708
/// End RTTI of "class zeno::Vector<struct zeno::Vec4i>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4i> &"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12340739596885358326
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12340739596885358326 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> &>::type>::type,class zeno::Vector<struct zeno::Vec4i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> &",
                12340739596885358326ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> &",
                12340739596885358326ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12340739596885358326
/// End RTTI of "class zeno::Vector<struct zeno::Vec4i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4i> &&"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_17726608495519294320
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_17726608495519294320 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4i> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> &&>::type>::type,class zeno::Vector<struct zeno::Vec4i> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> &&",
                17726608495519294320ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> &&",
                17726608495519294320ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_17726608495519294320
/// End RTTI of "class zeno::Vector<struct zeno::Vec4i> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec4i> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12741371416950012287
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12741371416950012287 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec4i> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4i> &>::type>::type,const class zeno::Vector<struct zeno::Vec4i> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4i> &",
                12741371416950012287ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4i> &",
                12741371416950012287ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4i> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_12741371416950012287
/// End RTTI of "const class zeno::Vector<struct zeno::Vec4i> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class zeno::Vector<struct zeno::Vec4i> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12741375814996525131
#define _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12741375814996525131 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class zeno::Vector<struct zeno::Vec4i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4i> *>::type>::type,const class zeno::Vector<struct zeno::Vec4i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4i> *",
                12741375814996525131ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class zeno::Vector<struct zeno::Vec4i> *",
                12741375814996525131ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class zeno::Vector<struct zeno::Vec4i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12741375814996525131
/// End RTTI of "const class zeno::Vector<struct zeno::Vec4i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class zeno::Vector<struct zeno::Vec4i> *"
#ifndef _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12340752791024896858
#define _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12340752791024896858 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class zeno::Vector<struct zeno::Vec4i> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> *>::type>::type,class zeno::Vector<struct zeno::Vec4i> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> *",
                12340752791024896858ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class zeno::Vector<struct zeno::Vec4i> *",
                12340752791024896858ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class zeno::Vector<struct zeno::Vec4i> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_zeno_VectorLABstruct_zeno_Vec4iRAB_Mul_12340752791024896858
/// End RTTI of "class zeno::Vector<struct zeno::Vec4i> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class zeno::reflect::Any>"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_4563279078811816794
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_4563279078811816794 1
// !!! importance: This is a template specialization "class std::vector<class zeno::reflect::Any>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class zeno::reflect::Any>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any>>::type>::type,class std::vector<class zeno::reflect::Any>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any>",
                4563279078811816794ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any>",
                4563279078811816794ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABclass_zeno_reflect_AnyRABRegistrator {
        _Sclass_std_vectorLABclass_zeno_reflect_AnyRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(4563279078811816794, zeno::reflect::type_info<class std::vector<class zeno::reflect::Any>>());
        }
    };
    static _Sclass_std_vectorLABclass_zeno_reflect_AnyRABRegistrator global_Sclass_std_vectorLABclass_zeno_reflect_AnyRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_AnyList = 4563279078811816794ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_4563279078811816794
/// End RTTI of "class std::vector<class zeno::reflect::Any>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class zeno::reflect::Any> &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_5251976634634287288
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_5251976634634287288 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class zeno::reflect::Any> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> &>::type>::type,class std::vector<class zeno::reflect::Any> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> &",
                5251976634634287288ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> &",
                5251976634634287288ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_5251976634634287288
/// End RTTI of "class std::vector<class zeno::reflect::Any> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class zeno::reflect::Any> &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_16180654479438591098
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_16180654479438591098 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class zeno::reflect::Any> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> &&>::type>::type,class std::vector<class zeno::reflect::Any> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> &&",
                16180654479438591098ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> &&",
                16180654479438591098ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_16180654479438591098
/// End RTTI of "class std::vector<class zeno::reflect::Any> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class zeno::reflect::Any> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_17300295942803379059
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_17300295942803379059 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class zeno::reflect::Any> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class zeno::reflect::Any> &>::type>::type,const class std::vector<class zeno::reflect::Any> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class zeno::reflect::Any> &",
                17300295942803379059ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class zeno::reflect::Any> &",
                17300295942803379059ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class zeno::reflect::Any> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_17300295942803379059
/// End RTTI of "const class std::vector<class zeno::reflect::Any> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class zeno::reflect::Any> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_17300309136942917591
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_17300309136942917591 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class zeno::reflect::Any> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class zeno::reflect::Any> *>::type>::type,const class std::vector<class zeno::reflect::Any> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class zeno::reflect::Any> *",
                17300309136942917591ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class zeno::reflect::Any> *",
                17300309136942917591ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class zeno::reflect::Any> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_17300309136942917591
/// End RTTI of "const class std::vector<class zeno::reflect::Any> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class zeno::reflect::Any> *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_5251972236587774444
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_5251972236587774444 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class zeno::reflect::Any> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> *>::type>::type,class std::vector<class zeno::reflect::Any> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> *",
                5251972236587774444ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class zeno::reflect::Any> *",
                5251972236587774444ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class zeno::reflect::Any> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_zeno_reflect_AnyRAB_Mul_5251972236587774444
/// End RTTI of "class std::vector<class zeno::reflect::Any> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::CurvesData"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_16327668114186180410
#define _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_16327668114186180410 1
namespace zeno {
struct CurvesData;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::CurvesData>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::CurvesData>::type>::type,struct zeno::CurvesData>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData",
                16327668114186180410ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData",
                16327668114186180410ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::CurvesData>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_CurvesDataRegistrator {
        _Sstruct_zeno_CurvesDataRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16327668114186180410, zeno::reflect::type_info<struct zeno::CurvesData>());
        }
    };
    static _Sstruct_zeno_CurvesDataRegistrator global_Sstruct_zeno_CurvesDataRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Curve = 16327668114186180410ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_16327668114186180410
/// End RTTI of "struct zeno::CurvesData"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::CurvesData &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_96402494007902360
#define _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_96402494007902360 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::CurvesData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::CurvesData &>::type>::type,struct zeno::CurvesData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData &",
                96402494007902360ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData &",
                96402494007902360ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::CurvesData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_96402494007902360
/// End RTTI of "struct zeno::CurvesData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::CurvesData &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_1402897154312356570
#define _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_1402897154312356570 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::CurvesData &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::CurvesData &&>::type>::type,struct zeno::CurvesData &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData &&",
                1402897154312356570ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData &&",
                1402897154312356570ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::CurvesData &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_1402897154312356570
/// End RTTI of "struct zeno::CurvesData &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::CurvesData &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_9526337862275095147
#define _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_9526337862275095147 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::CurvesData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::CurvesData &>::type>::type,const struct zeno::CurvesData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::CurvesData &",
                9526337862275095147ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::CurvesData &",
                9526337862275095147ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::CurvesData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_9526337862275095147
/// End RTTI of "const struct zeno::CurvesData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::CurvesData *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_Mul_9526333464228582303
#define _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_Mul_9526333464228582303 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::CurvesData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::CurvesData *>::type>::type,const struct zeno::CurvesData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::CurvesData *",
                9526333464228582303ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::CurvesData *",
                9526333464228582303ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::CurvesData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_CurvesData_Mul_9526333464228582303
/// End RTTI of "const struct zeno::CurvesData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::CurvesData *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_Mul_96398095961389516
#define _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_Mul_96398095961389516 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::CurvesData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::CurvesData *>::type>::type,struct zeno::CurvesData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData *",
                96398095961389516ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::CurvesData *",
                96398095961389516ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::CurvesData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_CurvesData_Mul_96398095961389516
/// End RTTI of "struct zeno::CurvesData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::BCurveObject"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_6841588794272413944
#define _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_6841588794272413944 1
namespace zeno {
struct BCurveObject;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::BCurveObject>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::BCurveObject>::type>::type,struct zeno::BCurveObject>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject",
                6841588794272413944ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject",
                6841588794272413944ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::BCurveObject>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_BCurveObjectRegistrator {
        _Sstruct_zeno_BCurveObjectRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(6841588794272413944, zeno::reflect::type_info<struct zeno::BCurveObject>());
        }
    };
    static _Sstruct_zeno_BCurveObjectRegistrator global_Sstruct_zeno_BCurveObjectRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_BCurve = 6841588794272413944ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_6841588794272413944
/// End RTTI of "struct zeno::BCurveObject"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::BCurveObject &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11657110111117189930
#define _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11657110111117189930 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::BCurveObject &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::BCurveObject &>::type>::type,struct zeno::BCurveObject &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject &",
                11657110111117189930ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject &",
                11657110111117189930ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::BCurveObject &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11657110111117189930
/// End RTTI of "struct zeno::BCurveObject &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::BCurveObject &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11905258533495410020
#define _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11905258533495410020 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::BCurveObject &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::BCurveObject &&>::type>::type,struct zeno::BCurveObject &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject &&",
                11905258533495410020ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject &&",
                11905258533495410020ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::BCurveObject &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_11905258533495410020
/// End RTTI of "struct zeno::BCurveObject &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::BCurveObject &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_8523929680390604637
#define _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_8523929680390604637 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::BCurveObject &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::BCurveObject &>::type>::type,const struct zeno::BCurveObject &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::BCurveObject &",
                8523929680390604637ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::BCurveObject &",
                8523929680390604637ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::BCurveObject &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_8523929680390604637
/// End RTTI of "const struct zeno::BCurveObject &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::BCurveObject *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_Mul_8523916486251066105
#define _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_Mul_8523916486251066105 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::BCurveObject *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::BCurveObject *>::type>::type,const struct zeno::BCurveObject *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::BCurveObject *",
                8523916486251066105ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::BCurveObject *",
                8523916486251066105ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::BCurveObject *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_BCurveObject_Mul_8523916486251066105
/// End RTTI of "const struct zeno::BCurveObject *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::BCurveObject *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_Mul_11657096916977651398
#define _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_Mul_11657096916977651398 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::BCurveObject *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::BCurveObject *>::type>::type,struct zeno::BCurveObject *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject *",
                11657096916977651398ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::BCurveObject *",
                11657096916977651398ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::BCurveObject *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_BCurveObject_Mul_11657096916977651398
/// End RTTI of "struct zeno::BCurveObject *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::HeatmapData"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_16907690619462570378
#define _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_16907690619462570378 1
namespace zeno {
struct HeatmapData;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::HeatmapData>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::HeatmapData>::type>::type,struct zeno::HeatmapData>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData",
                16907690619462570378ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData",
                16907690619462570378ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::HeatmapData>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_HeatmapDataRegistrator {
        _Sstruct_zeno_HeatmapDataRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(16907690619462570378, zeno::reflect::type_info<struct zeno::HeatmapData>());
        }
    };
    static _Sstruct_zeno_HeatmapDataRegistrator global_Sstruct_zeno_HeatmapDataRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Heatmap = 16907690619462570378ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_16907690619462570378
/// End RTTI of "struct zeno::HeatmapData"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::HeatmapData &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_1989512508592356968
#define _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_1989512508592356968 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::HeatmapData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::HeatmapData &>::type>::type,struct zeno::HeatmapData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData &",
                1989512508592356968ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData &",
                1989512508592356968ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::HeatmapData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_1989512508592356968
/// End RTTI of "struct zeno::HeatmapData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::HeatmapData &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_14004932897472323210
#define _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_14004932897472323210 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::HeatmapData &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::HeatmapData &&>::type>::type,struct zeno::HeatmapData &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData &&",
                14004932897472323210ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData &&",
                14004932897472323210ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::HeatmapData &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_14004932897472323210
/// End RTTI of "struct zeno::HeatmapData &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::HeatmapData &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_8379271405830377253
#define _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_8379271405830377253 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::HeatmapData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::HeatmapData &>::type>::type,const struct zeno::HeatmapData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::HeatmapData &",
                8379271405830377253ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::HeatmapData &",
                8379271405830377253ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::HeatmapData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_8379271405830377253
/// End RTTI of "const struct zeno::HeatmapData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::HeatmapData *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_Mul_8379275803876890097
#define _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_Mul_8379275803876890097 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::HeatmapData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::HeatmapData *>::type>::type,const struct zeno::HeatmapData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::HeatmapData *",
                8379275803876890097ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::HeatmapData *",
                8379275803876890097ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::HeatmapData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_HeatmapData_Mul_8379275803876890097
/// End RTTI of "const struct zeno::HeatmapData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::HeatmapData *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_Mul_1989508110545844124
#define _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_Mul_1989508110545844124 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::HeatmapData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::HeatmapData *>::type>::type,struct zeno::HeatmapData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData *",
                1989508110545844124ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::HeatmapData *",
                1989508110545844124ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::HeatmapData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_HeatmapData_Mul_1989508110545844124
/// End RTTI of "struct zeno::HeatmapData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::ShaderData"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_5880748608921663691
#define _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_5880748608921663691 1
namespace zeno {
struct ShaderData;
}

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::ShaderData>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::ShaderData>::type>::type,struct zeno::ShaderData>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData",
                5880748608921663691ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData",
                5880748608921663691ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::ShaderData>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sstruct_zeno_ShaderDataRegistrator {
        _Sstruct_zeno_ShaderDataRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(5880748608921663691, zeno::reflect::type_info<struct zeno::ShaderData>());
        }
    };
    static _Sstruct_zeno_ShaderDataRegistrator global_Sstruct_zeno_ShaderDataRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_Shader = 5880748608921663691ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_5880748608921663691
/// End RTTI of "struct zeno::ShaderData"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::ShaderData &"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_4125865228026216245
#define _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_4125865228026216245 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::ShaderData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::ShaderData &>::type>::type,struct zeno::ShaderData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData &",
                4125865228026216245ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData &",
                4125865228026216245ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::ShaderData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_4125865228026216245
/// End RTTI of "struct zeno::ShaderData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::ShaderData &&"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_17198355082522979657
#define _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_17198355082522979657 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::ShaderData &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::ShaderData &&>::type>::type,struct zeno::ShaderData &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData &&",
                17198355082522979657ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData &&",
                17198355082522979657ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::ShaderData &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_17198355082522979657
/// End RTTI of "struct zeno::ShaderData &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::ShaderData &"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_6722822090631911282
#define _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_6722822090631911282 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::ShaderData &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::ShaderData &>::type>::type,const struct zeno::ShaderData &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::ShaderData &",
                6722822090631911282ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::ShaderData &",
                6722822090631911282ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::ShaderData &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_6722822090631911282
/// End RTTI of "const struct zeno::ShaderData &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const struct zeno::ShaderData *"
#ifndef _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_Mul_6722826488678424126
#define _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_Mul_6722826488678424126 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const struct zeno::ShaderData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const struct zeno::ShaderData *>::type>::type,const struct zeno::ShaderData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::ShaderData *",
                6722826488678424126ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const struct zeno::ShaderData *",
                6722826488678424126ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const struct zeno::ShaderData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_struct_zeno_ShaderData_Mul_6722826488678424126
/// End RTTI of "const struct zeno::ShaderData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "struct zeno::ShaderData *"
#ifndef _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_Mul_4125869626072729089
#define _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_Mul_4125869626072729089 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<struct zeno::ShaderData *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<struct zeno::ShaderData *>::type>::type,struct zeno::ShaderData *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData *",
                4125869626072729089ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "struct zeno::ShaderData *",
                4125869626072729089ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<struct zeno::ShaderData *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_struct_zeno_ShaderData_Mul_4125869626072729089
/// End RTTI of "struct zeno::ShaderData *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "enum zeno::ParamControl"
#ifndef _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_18413074326739205324
#define _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_18413074326739205324 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<enum zeno::ParamControl>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<enum zeno::ParamControl>::type>::type,enum zeno::ParamControl>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl",
                18413074326739205324ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl",
                18413074326739205324ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<enum zeno::ParamControl>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Senum_zeno_ParamControlRegistrator {
        _Senum_zeno_ParamControlRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(18413074326739205324, zeno::reflect::type_info<enum zeno::ParamControl>());
        }
    };
    static _Senum_zeno_ParamControlRegistrator global_Senum_zeno_ParamControlRegistrator{};
    
}





}
#endif // _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_18413074326739205324
/// End RTTI of "enum zeno::ParamControl"
///////////////////////////

///////////////////////////
/// Begin RTTI of "enum zeno::ParamControl &"
#ifndef _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_12283126699397920966
#define _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_12283126699397920966 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<enum zeno::ParamControl &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<enum zeno::ParamControl &>::type>::type,enum zeno::ParamControl &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl &",
                12283126699397920966ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl &",
                12283126699397920966ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<enum zeno::ParamControl &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}





}
#endif // _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_12283126699397920966
/// End RTTI of "enum zeno::ParamControl &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "enum zeno::ParamControl &&"
#ifndef _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_7059083039699641504
#define _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_7059083039699641504 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<enum zeno::ParamControl &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<enum zeno::ParamControl &&>::type>::type,enum zeno::ParamControl &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl &&",
                7059083039699641504ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl &&",
                7059083039699641504ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<enum zeno::ParamControl &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}





}
#endif // _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_7059083039699641504
/// End RTTI of "enum zeno::ParamControl &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const enum zeno::ParamControl &"
#ifndef _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_13214106014975902125
#define _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_13214106014975902125 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const enum zeno::ParamControl &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const enum zeno::ParamControl &>::type>::type,const enum zeno::ParamControl &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const enum zeno::ParamControl &",
                13214106014975902125ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const enum zeno::ParamControl &",
                13214106014975902125ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const enum zeno::ParamControl &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}





}
#endif // _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_13214106014975902125
/// End RTTI of "const enum zeno::ParamControl &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const enum zeno::ParamControl *"
#ifndef _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_Mul_13214092820836363593
#define _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_Mul_13214092820836363593 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const enum zeno::ParamControl *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const enum zeno::ParamControl *>::type>::type,const enum zeno::ParamControl *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const enum zeno::ParamControl *",
                13214092820836363593ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const enum zeno::ParamControl *",
                13214092820836363593ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const enum zeno::ParamControl *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}





}
#endif // _REFLECT_RTTI_GUARD_const_enum_zeno_ParamControl_Mul_13214092820836363593
/// End RTTI of "const enum zeno::ParamControl *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "enum zeno::ParamControl *"
#ifndef _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_Mul_12283139893537459498
#define _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_Mul_12283139893537459498 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<enum zeno::ParamControl *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<enum zeno::ParamControl *>::type>::type,enum zeno::ParamControl *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl *",
                12283139893537459498ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "enum zeno::ParamControl *",
                12283139893537459498ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<enum zeno::ParamControl *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}





}
#endif // _REFLECT_RTTI_GUARD_enum_zeno_ParamControl_Mul_12283139893537459498
/// End RTTI of "enum zeno::ParamControl *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_8007795410826436593
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_8007795410826436593 1
// !!! importance: This is a template specialization "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >>::type>::type,class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >",
                8007795410826436593ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >",
                8007795410826436593ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RABRegistrator {
        _Sclass_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(8007795410826436593, zeno::reflect::type_info<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >>());
        }
    };
    static _Sclass_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RABRegistrator global_Sclass_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_VecEdit = 8007795410826436593ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_8007795410826436593
/// End RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> >"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_9673595235362224255
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_9673595235362224255 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::type>::type,class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &",
                9673595235362224255ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &",
                9673595235362224255ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_9673595235362224255
/// End RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_14638102800581967675
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_14638102800581967675 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&>::type>::type,class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&",
                14638102800581967675ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&",
                14638102800581967675ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_14638102800581967675
/// End RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_5666251999829904224
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_5666251999829904224 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::type>::type,const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &",
                5666251999829904224ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &",
                5666251999829904224ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_5666251999829904224
/// End RTTI of "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_5666265193969442756
#define _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_5666265193969442756 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::type>::type,const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *",
                5666265193969442756ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *",
                5666265193969442756ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_5666265193969442756
/// End RTTI of "const class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *"
#ifndef _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_9673599633408737099
#define _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_9673599633408737099 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::type>::type,class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *",
                9673599633408737099ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *",
                9673599633408737099ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_vectorLABclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_RAB_Mul_9673599633408737099
/// End RTTI of "class std::vector<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> > *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>"
#ifndef _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_9597672160759649577
#define _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_9597672160759649577 1
// !!! importance: This is a template specialization "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>", doesn't generate forward declaration.

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>>::type>::type,class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>",
                9597672160759649577ULL,
                static_cast<size_t>(
                    TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>",
                9597672160759649577ULL,
                static_cast<size_t>(
                    TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
    struct _Sclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRABRegistrator {
        _Sclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRABRegistrator() {
            zeno::reflect::ReflectionRegistry::get().getRttiMap()->add(9597672160759649577, zeno::reflect::type_info<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>>());
        }
    };
    static _Sclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRABRegistrator global_Sclass_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRABRegistrator{};
    
}


namespace types
{
    constexpr size_t gParamType_PrimVariant = 9597672160759649577ULL;
}


}
#endif // _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_9597672160759649577
/// End RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData>"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &"
#ifndef _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4088887018943714103
#define _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4088887018943714103 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::type>::type,class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &",
                4088887018943714103ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &",
                4088887018943714103ULL,
                static_cast<size_t>(
                    TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4088887018943714103
/// End RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&"
#ifndef _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4937513916155684323
#define _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4937513916155684323 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&>::type>::type,class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&",
                4937513916155684323ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&",
                4937513916155684323ULL,
                static_cast<size_t>(
                    TF_IsRValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_4937513916155684323
/// End RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &&"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_8950534024750819624
#define _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_8950534024750819624 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::type>::type,const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &",
                8950534024750819624ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &",
                8950534024750819624ULL,
                static_cast<size_t>(
                    TF_IsConst | TF_IsLValueRef | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_8950534024750819624
/// End RTTI of "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> &"
///////////////////////////

///////////////////////////
/// Begin RTTI of "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *"
#ifndef _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_8950529626704306780
#define _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_8950529626704306780 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::type>::type,const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *",
                8950529626704306780ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *",
                8950529626704306780ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_const_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_8950529626704306780
/// End RTTI of "const class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *"
///////////////////////////

///////////////////////////
/// Begin RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *"
#ifndef _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_4088873824804175571
#define _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_4088873824804175571 1

namespace zeno
{
namespace reflect
{
    template <>
    inline REFLECT_STATIC_CONSTEXPR const RTTITypeInfo& type_info<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>() {
        if REFLECT_FORCE_CONSTEPXR (std::is_same<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::type>::type,class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::value) {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *",
                4088873824804175571ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                0
            };
            return s;
        } else {
            static REFLECT_STATIC_CONSTEXPR RTTITypeInfo s = {
                "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *",
                4088873824804175571ULL,
                static_cast<size_t>(
                    TF_IsPointer | TF_None ),
                type_info<typename std::decay<std::remove_pointer<class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *>::type>::type>().hash_code()
            };
            return s;
        }
    }

    
}



}
#endif // _REFLECT_RTTI_GUARD_class_std_variantLABint_float_class_std_basicstringLABcharRAB_struct_zeno_CurveDataRAB_Mul_4088873824804175571
/// End RTTI of "class std::variant<int, float, class std::basic_string<char>, struct zeno::CurveData> *"
///////////////////////////

/// End generated RTTI for types
////////////////////////////////////////////////


////////////////////////////////////////////////
/// Begin generated reflected types

/// End generated reflected types
////////////////////////////////////////////////

