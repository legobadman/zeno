#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#include <zeno/zeno.h>
#include <zeno/utils/log.h>
#include <zeno/utils/string.h>
#include <Windows.h>


namespace zeno {

struct PythonNode : zeno::INode {

    virtual void apply() override {

        std::string stdOutErr =
            "import sys\n\
\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
    def flush(self):\n\
        pass\n\
catchOutErr = CatchOutErr()\n\
sys.stdout = catchOutErr\n\
sys.stderr = catchOutErr\n\
"; //this is python code to redirect stdouts/stderr

        //Py_Initialize();
        PyObject* pModule = PyImport_AddModule("__main__"); //create main module
        int ret = PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect

        printf("Python version: %s\n", Py_GetVersion());

        auto prim = get_input_prim_param("script");
        std::string script = zeno::reflect::any_cast<std::string>(prim.result);

        if (PyRun_SimpleString(script.c_str()) < 0) {
            zeno::log_warn("Python Script run failed");
        }
        PyObject* catcher = PyObject_GetAttrString(pModule, "catchOutErr"); //get our catchOutErr created above
        PyObject* output = PyObject_GetAttrString(catcher, "value"); //get the stdout and stderr from our catchOutErr object
        if (output != Py_None)
        {
            std::string str = _PyUnicode_AsString(output);
            for (const auto& line : split_str(str, '\n'))
            {
                if (!line.empty())
                {
                    zeno::log_info(line);
                }
            }
        }
    }
};

ZENDEFNODE(PythonNode, {
    {
        {gParamType_String, "script", "", Socket_Primitve, CodeEditor}
    },
    {},
    {},
    {"command"},
});

}

#endif