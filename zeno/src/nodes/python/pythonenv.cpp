#if 0
#include <Python.h>
#include <zeno/utils/log.h>
#include <zeno/zeno.h>
#include "pythonenv.h"

#ifdef WIN32
#include <Windows.h>
#endif

PyMODINIT_FUNC PyInit_zeno(void);

void initPythonEnv(const char* progName)
{
    wchar_t* program = Py_DecodeLocale(progName, NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    if (PyImport_AppendInittab("zeno", PyInit_zeno) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    Py_SetProgramName(program);

    Py_Initialize();

    PyObject* pmodule = PyImport_ImportModule("zeno");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'zeno'\n");
    }

    std::string tempCode;
    tempCode = "import zeno; gra = zeno.graph('main')";

    if (PyRun_SimpleString(tempCode.c_str()) < 0) {
        zeno::log_warn("Failed to initialize Python module");
        return;
    }

    PyMem_RawFree(program);
}

struct _SGlobal_initPythonEnv
{
    _SGlobal_initPythonEnv() {
#ifdef WIN32
        char filename[MAX_PATH];
        DWORD size = GetModuleFileNameA(NULL, filename, MAX_PATH);
        initPythonEnv(filename);
#else
        initPythonEnv("");
#endif
    }
};

static _SGlobal_initPythonEnv _inst;


#endif