#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#endif
#include <zeno/extra/PyExecuteProxy.h>
#include <zeno/zeno.h>
#include <thread>
#include <Windows.h>


namespace zeno {

    PyExecuteProxy::PyExecuteProxy() : m_bInitEnv(false) {
        //启动线程，并初始化Python环境，并且线程处于监听状态
        m_thd = std::thread(&PyExecuteProxy::waitLoop, this);
    }

    void PyExecuteProxy::initPyzenFunc(std::function<void()> pyzenFunc) {
        m_pyzenFunc = pyzenFunc;
    }

    void PyExecuteProxy::initialize() {
#ifdef ZENO_WITH_PYTHON
        if (!m_pyzenFunc) {
            throw makeError<UnimplError>("the pyzen has not been initialized");
        }
        m_pyzenFunc();
        char filename[256];
        DWORD size = GetModuleFileNameA(NULL, filename, MAX_PATH);
        m_bInitEnv = true;

        wchar_t* program = Py_DecodeLocale(filename, NULL);
        if (program == NULL) {
            fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
            exit(1);
        }

        Py_SetProgramName(program);
        Py_Initialize();
        PyMem_RawFree(program);
#endif
    }

    void PyExecuteProxy::waitLoop() {
        while (true) {
            std::unique_lock lck(m_mtx);
            m_cv.wait(lck, [&]() {return m_context.state == State::PENDING; });

            if (!m_bInitEnv) {
                initialize();
                m_bInitEnv = true;
            }

            bool bFailed = false;

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

            PyObject* pModule = PyImport_AddModule("__main__"); //create main module
            int ret = PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect

            if (PyRun_SimpleString(m_code.c_str()) < 0) {
                bFailed = true;
            }

            PyObject* catcher = PyObject_GetAttrString(pModule, "catchOutErr"); //get our catchOutErr created above
            PyObject* output = PyObject_GetAttrString(catcher, "value"); //get the stdout and stderr from our catchOutErr object
            if (output != Py_None)
            {
                std::string str = _PyUnicode_AsString(output);
                m_context.info = str;
                for (const auto& line : split_str(str, '\n'))
                {
                    if (!line.empty())
                    {
                        zeno::log_info(line);
                    }
                }
            }
#endif
            m_code.clear();
            m_context.state = bFailed ? FAILED : SUCCEED;
            m_cv.notify_one();
        }
    }

    bool PyExecuteProxy::runPython(const std::string& code) {
        {
            std::lock_guard lck(m_mtx);
            m_code = code;
            m_context.state = State::PENDING;
            m_cv.notify_one();
        }
        std::unique_lock lck(m_mtx);
        m_cv.wait(lck, [&]() { return m_context.state != State::PENDING; });
        bool bSucceed = m_context.state == State::SUCCEED;
        m_context.clear();
        return bSucceed;
    }

}