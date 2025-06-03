#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#include <zeno/zeno.h>
#include <zeno/utils/log.h>
#include <zeno/utils/string.h>
#include <Windows.h>
#include <zeno/include/zenoutil.h>


namespace zeno {

struct PythonNode : zeno::INode {

    virtual void apply() override {
        auto prim = ZImpl(get_input_prim_param("script"));
        std::string script = zeno::reflect::any_cast<std::string>(prim.result);
        runPython(script);
        set_output("object", get_input("object"));
    }
};

ZENDEFNODE(PythonNode, {
    {
        {gParamType_IObject, "object"},
        {gParamType_String, "script", "", Socket_Primitve, CodeEditor}
    },
    {
        {gParamType_IObject, "object"}
    },
    {},
    {"command"},
});

}

#endif