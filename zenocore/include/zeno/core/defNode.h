#pragma once

#include <zeno/core/Session.h>
#include <zeno/core/NodeRegister.h>

namespace zeno {

#define ZENO_CUSTOMUI_NODE(Class, ...) \
    static zeno::CustomUI ui_##Class(__VA_ARGS__); \
    static struct _Def##Class { \
        _Def##Class() { \
            ::zeno::getNodeRegister().registerNodeClass(\
                []() -> zeno::INode2* { return new Class; }, \
                [] (INode2* pNode) { delete pNode; }, \
                #Class, ui_##Class); \
        } \
\
        ~_Def##Class() {\
            ::zeno::getNodeRegister().unregisterNodeClass(#Class);\
        }\
    } _def##Class


#define ZENO_DEFNODE(Class) \
    static struct _Def##Class { \
        _Def##Class(::zeno::Descriptor const &desc) { \
            ::zeno::getNodeRegister().registerNodeClass( \
                [] () -> zeno::INode2* { return new Class; }, \
                [] (INode2* pNode) { delete pNode; }, \
                 #Class, desc); \
        } \
\
        ~_Def##Class() {\
            ::zeno::getNodeRegister().unregisterNodeClass(#Class);\
        }\
    } _def##Class

// deprecated:
template <class T>
[[deprecated("use ZENO_DEFNODE(T)(...)")]]
inline int defNodeClass(std::string const &id, zeno::Descriptor const &desc = {}) {
    getNodeRegister().registerNodeClass([] () -> zeno::INode* { return new T; }, id, desc);
    return 1;
}

#define ZENDEFNODE(Class, ...) ZENO_DEFNODE(Class)(__VA_ARGS__);

// deprecated:
#define ZENO_DEFOVERLOADNODE(Class, PostFix, ...) \
    static int _deprecatedDefOverload##Class##And##PostFix = [] (::zeno::Descriptor const &) { return 1; }
//#define ZENO_DEFOVERLOADNODE(Class, PostFix, ...) \
    //static int _def##Class##PostFix = ::zeno::_defOverloadNodeClassHelper(std::make_unique<Class##PostFix>, #Class, {__VA_ARGS__})

}
