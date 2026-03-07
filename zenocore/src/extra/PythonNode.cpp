#ifdef ZENO_WITH_PYTHON
//
// PythonNode - migrated from zeno/src/nodes/python/pythonnode.cpp
// Runs Python script via Session::asyncRunPython, then passes through object.
//
#include <zeno/core/defNode.h>
#include <zeno/core/Session.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace zeno {

struct PythonNode : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        char scriptBuf[65536] = {};
        nd->get_input2_string("script", scriptBuf, sizeof(scriptBuf));
        std::string script(scriptBuf);

        auto& sess = getSession();
        sess.asyncRunPython(script);
#ifdef _WIN32
        HANDLE hEventPyReady = static_cast<HANDLE>(sess.hEventOfPyFinish());
        WaitForSingleObject(hEventPyReady, INFINITE);
        ResetEvent(hEventPyReady);
#else
        (void)sess;
#endif
        IObject2* inobj = nd->clone_input_object("object");
        nd->set_output_object("object", inobj);
        return ZErr_OK;
    }
};

ZENDEFNODE(PythonNode, {
    {
        {_gParamType_IObject, "object"},
        {gParamType_String, "script", "", zeno::Socket_Primitve, zeno::CodeEditor}
    },
    {
        {_gParamType_IObject, "object"}
    },
    {},
    {"command"},
});

} // namespace zeno

#endif
