#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/Descriptor.h>
#include <zeno/core/data.h>
#include <inodedata.h>
#include <memory>
#include <string>
#include <atomic>
#include <map>
#include <zeno/core/common.h>
#include <zeno/io/iocommon.h>


namespace zeno {

    namespace reflect {
        class TypeBase;
    }

struct Graph;
struct Session;
struct ZNode;
struct GlobalVariableManager;
struct ObjectRecorder;
struct IObject2;
struct INodeClass;
struct GlobalState;
struct GlobalComm;
struct GlobalError;
struct EventCallbacks;
struct UserData;
struct CalcManager;
struct AssetsMgr;
struct PythonEnvWrapper;
class PyExecuteProxy;
class FunctionManager;

struct Session {
    std::unique_ptr<GlobalState> const globalState;
    std::unique_ptr<GlobalComm> const globalComm;
    std::unique_ptr<GlobalError> const globalError;
    std::unique_ptr<EventCallbacks> const eventCallbacks;
    std::unique_ptr<UserData> const m_userData;
    std::unique_ptr<AssetsMgr> assets;
    std::unique_ptr<FunctionManager> funcManager;
    std::unique_ptr<ObjectRecorder> m_recorder;

    ZENO_API Session();
    ZENO_API ~Session();
    ZENO_API void destroy();

    Session(Session const &) = delete;
    Session &operator=(Session const &) = delete;
    Session(Session &&) = delete;
    Session &operator=(Session &&) = delete;

    ZENO_API UserData &userData() const;
    ZENO_API std::shared_ptr<Graph> createGraph(const std::string& name);
    ZENO_API std::shared_ptr<Graph> mainGraph() const;
    ZENO_API ZNode* getNodeByUuidPath(std::string const& uuid_path);
    ZENO_API ZNode* getNodeByPath(std::string const& uuid_path);
    ZENO_API void resetMainGraph();
    ZENO_API void clearMainGraph();
    ZENO_API void clearAssets();
    ZENO_API bool run(const std::string& currgraph, render_reload_info& infos);
    ZENO_API void interrupt();
    ZENO_API bool is_interrupted() const { return m_bInterrupted; }
    ZENO_API bool is_async_executing() const;
    ZENO_API void set_async_executing(bool bOn);
    ZENO_API unsigned long mainThreadId() const;
    ZENO_API void setMainThreadId(unsigned long threadId);
    ZENO_API void set_solver(const std::string& solver);
    ZENO_API std::string get_solver();
    ZENO_API void terminate_solve();
    ZENO_API void init_project_path(const std::wstring& path);
    ZENO_API std::wstring get_project_path() const;
    //ZENO_API 
    ZENO_API void set_auto_run(bool bOn);
    ZENO_API bool is_auto_run() const;
    ZENO_API bool is_frame_node(const std::string& node_cls);
    ZENO_API int get_frame_id() const;
    ZENO_API void markDirtyAndCleanResult();
    ZENO_API void setApiLevelEnable(bool bEnable);
    ZENO_API void beginApiCall();
    ZENO_API void endApiCall();
    ZENO_API void setDisableRunning(bool bOn);
    ZENO_API void switchToFrame(int frameid);
    ZENO_API void updateFrameRange(int start, int end);
    ZENO_API void registerRunTrigger(std::function<void()> func);
    ZENO_API void registerNodeCallback(F_NodeStatus func);
    ZENO_API void registerIOCallback(F_IOProgress func);
    ZENO_API Graph* getGraphByPath(const std::string& path);
    ZENO_API void initEnv(const zenoio::ZSG_PARSE_RESULT ioresult);
    ZENO_API void initPyzen(std::function<void()> pyzenFunc);
    ZENO_API bool asyncRunPython(const std::string& code);
    ZENO_API bool runPythonInteractive(const std::string& line, bool& needMore, std::string& output);
    ZENO_API bool completePython(const std::string& text, std::vector<std::string>& out);
    ZENO_API void* hEventOfPyFinish();
    void reportNodeStatus(const ObjPath& path, bool bDirty, NodeRunStatus status);
    void reportIOProgress(const std::string& info, int inc);

private:
    int m_apiLevel = 0;
    bool m_bApiLevelEnable = true;
    bool m_bAutoRun = false;
    bool m_bDisableRunning = false;
    bool m_bReentrance = false;
    bool m_bAsyncExecute = false;
    std::string m_current_loading_module;
    std::string m_solver;

    std::wstring m_proj_path;
    unsigned long m_mainThreadId;
    std::atomic<bool> m_bInterrupted;
    std::unique_ptr<PyExecuteProxy> m_pyexecutor;

    std::shared_ptr<Graph> m_spMainGraph;

    std::function<void()> m_callbackRunTrigger;
    F_NodeStatus m_funcNodeStatus;
    F_IOProgress m_funcIOCallback;
};

ZENO_API Session &getSession();

ZENO_API std::unique_ptr<Session> createApplication();

}
