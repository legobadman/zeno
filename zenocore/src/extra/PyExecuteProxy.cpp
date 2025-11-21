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

        int ret = PyRun_SimpleString(
            "import code\n"
            "import sys\n"
            "py_console = code.InteractiveConsole()\n"
            "def push_line(line):\n"
            "    return py_console.push(line)\n"
        );

        PyRun_SimpleString("import zen\n");

        ret = PyRun_SimpleString(
            "import rlcompleter\n"
            "import __main__\n"
            "zen_completer = rlcompleter.Completer(__main__.__dict__)\n"
            "\n"
            "def _zen_get_completions(prefix):\n"
            "    import rlcompleter\n"
            "    import __main__\n"
            "\n"
            "    # --- 如果末尾是点，比如 'zen.' 或 'obj.' ---\n"
            "    if prefix.endswith('.'):\n"
            "        obj = prefix[:-1]\n"
            "        try:\n"
            "            val = eval(obj, __main__.__dict__)\n"
            "            # 返回所有可见属性\n"
            "            return [obj + '.' + k for k in dir(val) if not k.startswith('_')]\n"
            "        except Exception:\n"
            "            return []\n"
            "\n"
            "    # --- 标准 rlcompleter 处理 ---\n"
            "    results = []\n"
            "    i = 0\n"
            "    while True:\n"
            "        r = zen_completer.complete(prefix, i)\n"
            "        if r is None:\n"
            "            break\n"
            "        results.append(r)\n"
            "        i += 1\n"
            "    return results\n"
        );
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
            int ret_ = PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect

            if (m_execType == ExecType::RunScript) {
                if (PyRun_SimpleString(m_code.c_str()) < 0) {
                    bFailed = true;
                }
            }
            else if (m_execType == ExecType::RunInteractive) {
                // 调用 push_line(line)
                PyObject* main = PyImport_AddModule("__main__");
                PyObject* pushFunc = PyObject_GetAttrString(main, "push_line");
                PyObject* ret = PyObject_CallFunction(pushFunc, "s", m_lineInput.c_str());
                if (!ret) {
                    bFailed = true;
                }
                else {
                    // push_line 返回 True = 代码未结束；False = 可执行下一条
                    m_needMoreInput = PyObject_IsTrue(ret);
                    Py_DECREF(ret);
                }
            }
            else if (m_execType == ExecType::RunCompletion) {

                PyObject* main = PyImport_AddModule("__main__");
                PyObject* func = PyObject_GetAttrString(main, "_zen_get_completions");

                PyObject* arg = PyUnicode_FromString(m_completionPrefix.c_str());
                PyObject* listObj = PyObject_CallFunctionObjArgs(func, arg, NULL);

                m_completionList.clear();
                if (PyList_Check(listObj)) {
                    Py_ssize_t n = PyList_Size(listObj);
                    for (Py_ssize_t i = 0; i < n; i++) {
                        PyObject* item = PyList_GetItem(listObj, i);
                        if (PyUnicode_Check(item)) {
                            m_completionList.push_back(PyUnicode_AsUTF8(item));
                        }
                    }
                }

                Py_XDECREF(listObj);
                Py_XDECREF(arg);

                m_context.state = SUCCEED;
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
            m_execType = ExecType::RunScript;
            m_cv.notify_one();
        }
        std::unique_lock lck(m_mtx);
        m_cv.wait(lck, [&]() { return m_context.state != State::PENDING; });
        bool bSucceed = m_context.state == State::SUCCEED;
        m_context.clear();
        return bSucceed;
    }

    bool PyExecuteProxy::completePython(const std::string& prefix, std::vector<std::string>& out) {
        std::unique_lock<std::mutex> lck(m_mtx);

        // Wake the python thread to run completion request
        m_completionPrefix = prefix;
        m_execType = ExecType::RunCompletion;
        m_context.state = State::PENDING;
        m_cv.notify_one();

        m_cv.wait(lck, [&] { return m_context.state != State::PENDING; });

        out = m_completionList;
        return true;
    }

    bool PyExecuteProxy::runPythonInteractive(const std::string& line, bool& needMore, std::string& output) {
        {
            std::lock_guard lck(m_mtx);
            m_execType = ExecType::RunInteractive;
            m_lineInput = line;
            m_context.state = State::PENDING;
            m_cv.notify_one();
        }

        std::unique_lock lck(m_mtx);
        m_cv.wait(lck, [&]() { return m_context.state != State::PENDING; });

        needMore = m_needMoreInput;
        output = m_context.info;
        m_context.clear();
        return m_context.state == State::SUCCEED;
    }

}