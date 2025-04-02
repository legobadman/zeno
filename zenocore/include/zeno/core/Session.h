#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/Descriptor.h>
#include <zeno/core/data.h>
#include <memory>
#include <string>
#include <map>
#include <zeno/core/common.h>
#include <zeno/io/iocommon.h>


namespace zeno {

    namespace reflect {
        class TypeBase;
    }

struct Graph;
struct Session;
struct INode;
struct GlobalVariableManager;
struct IObject;
struct INodeClass;
struct GlobalState;
struct GlobalComm;
struct GlobalError;
struct EventCallbacks;
struct UserData;
struct CalcManager;
struct ObjectManager;
struct AssetsMgr;
class FunctionManager;

struct Session {
    std::map<std::string, std::unique_ptr<INodeClass>> nodeClasses;

    std::unique_ptr<GlobalState> const globalState;
    std::unique_ptr<GlobalComm> const globalComm;
    std::unique_ptr<GlobalError> const globalError;
    std::unique_ptr<EventCallbacks> const eventCallbacks;
    std::unique_ptr<UserData> const m_userData;
    std::unique_ptr<ObjectManager> const objsMan;
    std::shared_ptr<Graph> mainGraph;
    std::shared_ptr<AssetsMgr> assets;
    std::unique_ptr<GlobalVariableManager> globalVariableManager;
    std::unique_ptr<FunctionManager> funcManager;

    ZENO_API Session();
    ZENO_API ~Session();

    Session(Session const &) = delete;
    Session &operator=(Session const &) = delete;
    Session(Session &&) = delete;
    Session &operator=(Session &&) = delete;

    ZENO_API UserData &userData() const;
    ZENO_API std::shared_ptr<Graph> createGraph(const std::string& name);
    ZENO_API std::shared_ptr<INode> getNodeByUuidPath(std::string const& uuid_path);
    ZENO_API void resetMainGraph();
    ZENO_API bool run(const std::string& currgraph = "");
    ZENO_API void interrupt();
    ZENO_API bool is_interrupted() const;
    //ZENO_API 
    ZENO_API void set_auto_run(bool bOn);
    ZENO_API bool is_auto_run() const;
    ZENO_API void set_Rerun();
    ZENO_API std::string dumpDescriptorsJSON() const;
    ZENO_API zeno::NodeCates dumpCoreCates();
    ZENO_API void defNodeClass(std::shared_ptr<INode>(*ctor)(), std::string const &id, Descriptor const &desc = {});
    ZENO_API void defNodeClass2(std::shared_ptr<INode>(*ctor)(), std::string const& nodecls, CustomUI const& customui);
    ZENO_API void defNodeReflectClass(std::function<std::shared_ptr<INode>()> ctor, zeno::reflect::TypeBase* pTypeBase);
    ZENO_API void setApiLevelEnable(bool bEnable);
    ZENO_API void beginApiCall();
    ZENO_API void endApiCall();
    ZENO_API void setDisableRunning(bool bOn);
    ZENO_API void switchToFrame(int frameid);
    ZENO_API void updateFrameRange(int start, int end);
    ZENO_API int registerObjId(const std::string& objprefix);
    ZENO_API void registerRunTrigger(std::function<void()> func);
    ZENO_API void registerNodeCallback(F_NodeStatus func);
    ZENO_API void registerCommitRender(F_CommitRender&& func);
    ZENO_API std::shared_ptr<Graph> getGraphByPath(const std::string& path);

    void commit_to_render(render_update_info info);

    ZENO_API void registerObjUIInfo(size_t hashcode, std::string_view color, std::string_view nametip);
    ZENO_API bool getObjUIInfo(size_t hashcode, std::string_view& color, std::string_view& nametip);
    ZENO_API void initEnv(const zenoio::ZSG_PARSE_RESULT ioresult);
    void reportNodeStatus(const ObjPath& path, bool bDirty, NodeRunStatus status);

private:
    zeno::NodeCates m_cates;
    int m_apiLevel = 0;
    bool m_bApiLevelEnable = true;
    bool m_bAutoRun = false;
    bool m_bInterrupted = false;
    bool m_bDisableRunning = false;
    bool m_bReentrance = false;

    std::function<void()> m_callbackRunTrigger;
    F_NodeStatus m_funcNodeStatus;
    F_CommitRender m_func_commitrender;
};

ZENO_API Session &getSession();

}
