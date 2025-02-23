#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#include <zeno/zeno.h>
#include <zeno/utils/log.h>
#include <zeno/utils/string.h>


namespace zeno {

struct PythonNode : zeno::INode {
    virtual void apply() override {

        std::string stdOutErr =
            "import sys\n\
sys.path.append('D:/zeno/Debug/bin')\n\
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

        Py_Initialize();
        PyObject* pModule = PyImport_AddModule("__main__"); //create main module
        int ret = PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect

        auto prim = get_input_prim_param("script");
        std::string script = zeno::reflect::any_cast<std::string>(prim.result);

        PyObject* zeModule = PyImport_ImportModule("ze"); //create main module
        if (!zeModule) {
            PyErr_Print();  // 打印错误信息
        }

        //Py_Initialize();
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
        else {
            zeno::log_warn("The option 'ZENO_WITH_PYTHON' should be ON");
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