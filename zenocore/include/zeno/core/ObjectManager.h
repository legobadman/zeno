#pragma once

#include <zeno/core/IObject.h>
#include <zeno/core/NodeImpl.h>
#include <zeno/utils/PolymorphicMap.h>
#include <zeno/utils/api.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <set>
#include <functional>


namespace zeno {

    using SharedObjects = std::map<std::string, std::shared_ptr<zeno::IObject>>;

    struct RenderObjsInfo {
        SharedObjects newObjs;
        SharedObjects modifyObjs;
        SharedObjects remObjs;
        SharedObjects lightObjs;    //TODO:
        SharedObjects allObjects;

        bool empty() const {
            return newObjs.empty() && modifyObjs.empty() && remObjs.empty() && lightObjs.empty();
        }
    };

    struct ObjectNodeInfo {
        std::optional<zany> rootObj;        //list/dict case.
        zany transformingObj;
        NodeImpl* spViewNode;
    };


class ObjectManager
{
    struct _ObjInfo {
        std::shared_ptr<IObject> obj;
        std::set<ObjPath> attach_nodes;
        //ObjPath view_node;
    };

    using ViewObjects = std::map<std::string, _ObjInfo>;
    

    enum CacheType {
        MemoryCache,
        DiskCache
    };

    struct FrameData {
        std::optional<ViewObjects> view_objs;
        FILE* m_file = nullptr;
        CacheType cache;
    };

public:
     ObjectManager();
    ~ObjectManager();

    ZENO_API void beforeRun();
    ZENO_API void afterRun();
    ZENO_API void clearLastUnregisterObjs();
    ZENO_API void clear();

    ZENO_API void collect_render_update(zeno::render_update_info info);

    ZENO_API void collectingObject(zany, NodeImpl* view_node, bool bView);
    CALLBACK_REGIST(collectingObject, void, zany, bool)

    ZENO_API void removeObject(const std::string& id);
    CALLBACK_REGIST(removeObject, void, std::string)
    ZENO_API void revertRemoveObject(const std::string& id);

    ZENO_API void notifyTransfer(zany obj);
    CALLBACK_REGIST(notifyTransfer, void, zany)

    ZENO_API void viewObject(zany obj, bool bView);
    CALLBACK_REGIST(viewObject, void, zany, bool)

    ZENO_API int registerObjId(const std::string& objprefix);

    ZENO_API std::set<ObjPath> getAttachNodes(const std::string& id);

    ZENO_API void commit();
    ZENO_API void revert();

    ZENO_API void export_loading_objs(RenderObjsInfo& info);
    ZENO_API void export_render_infos(std::vector<zeno::render_update_info>& infos);
    ZENO_API void export_light_objs(RenderObjsInfo& info);
    ZENO_API void export_all_view_objs(RenderObjsInfo& info);
    ZENO_API void export_all_view_objs(std::map<std::string, zany>& info);
    ZENO_API zany getObj(const std::string& name);
    ZENO_API ObjectNodeInfo getObjectAndViewNode(const std::string& name);
    ZENO_API void clear_last_run();
    ZENO_API void collect_removing_objs(const std::string& objkey);
    ZENO_API void remove_attach_node_by_removing_objs();
    ZENO_API void remove_rendering_obj(zany spObj);

    //viewport interactive obj
    ZENO_API void collect_modify_objs(const std::string& newobjKey, bool isView);
    ZENO_API void collect_modify_objs(const std::set<std::string>& newobjKeys, bool isView);
    ZENO_API void remove_modify_objs(const std::set<std::string>& removeobjKeys);
    ZENO_API void getModifyObjsInfo(std::set<std::string>& modifyInteractiveObjs);  //interactive objs
    ZENO_API void syncObjNodeInfo(zany spObj, NodeImpl* spNode);

    std::set<char*> m_rec_geoms;

private:
    void clear_batch_updates();

    /*DEPRECATED BEGIN*/
    std::map<std::string, int> m_objRegister;

    ViewObjects m_objects;  //记录所有当前计算的对象，当切换帧的时候，可能使得部分依赖帧的对象重算。
    std::map<int, FrameData> m_frameData;   //记录流体相关的帧缓存

    std::set<std::string> m_viewObjs;
    std::set<std::string> m_removing_objs;  //这里是删除节点时记录的要删除的obj，要考虑rollback的情况

    std::set<std::string> m_newAdded;       //渲染端需要新增的obj
    std::set<std::string> m_remove;         //渲染端需要移除的obj
    std::set<std::string> m_modify;         //渲染端更改的obj

    std::set<std::string> m_lastUnregisterObjs;
    /*DEPRECATED END*/

    std::set<std::string> m_lastViewObjs;
    std::vector<zeno::render_update_info> m_render_updates;

    mutable std::mutex m_mtx;
};

}