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
        void waitLoop();

    private:
        void initialize();

        std::thread m_thd;
        std::condition_variable m_cv;
        std::mutex m_mtx;
        std::string m_code;
        ExecuteResult m_context;
        std::function<void()> m_pyzenFunc;
        bool m_bInitEnv;
    };

}
