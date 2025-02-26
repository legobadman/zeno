#include <zeno/include/zenoutil.h>
#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#endif
#include <zeno/utils/log.h>
#include <zeno/utils/string.h>

namespace zeno {
    ZENO_API bool runPython(const std::string& script) {
#ifdef ZENO_WITH_PYTHON
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

        Py_Initialize();
        PyObject* pModule = PyImport_AddModule("__main__"); //create main module
        int ret = PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect

        bool bFailed = false;
        if (PyRun_SimpleString(script.c_str()) < 0) {
            bFailed = true;
        }
        if (bFailed) {
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
        return !bFailed;
#else
        return false;
#endif
    }
}