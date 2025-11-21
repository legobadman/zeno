#pragma once

#include <string>
#include <thread>
#include <functional>
#include <condition_variable>

namespace zeno
{
    class PyExecuteProxy
    {
        enum State {
            STOP,
            PENDING,
            SUCCEED,
            FAILED,
        };

        enum ExecType {
            RunScript,
            RunInteractive,
            RunCompletion
        };

        struct ExecuteResult {
            std::string info;
            State state = STOP;

            void clear() {
                state = STOP;
                info.clear();
            }
        };

    public:
        PyExecuteProxy();
        void initPyzenFunc(std::function<void()> pyzenFunc);
        bool runPython(const std::string& code);
        bool runPythonInteractive(const std::string& line, bool& needMore, std::string& output);
        void waitLoop();
        bool completePython(const std::string& text, std::vector<std::string>& out);

    private:
        void initialize();

        std::thread m_thd;
        std::condition_variable m_cv;
        std::mutex m_mtx;
        std::string m_code;
        ExecuteResult m_context;
        std::function<void()> m_pyzenFunc;
        bool m_bInitEnv;

        ExecType m_execType = ExecType::RunScript;
        std::string m_lineInput;
        bool m_needMoreInput = false;  // 是否还需下一行（对应 py_console.push 返回值）

        // ---- 自动补全功能 ----
        std::string m_completionPrefix;
        std::vector<std::string> m_completionList;
    };

}
