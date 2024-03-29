#include <zeno/core/ObjectManager.h>
#include <zeno/core/Graph.h>
#include <zeno/types/ListObject.h>


namespace zeno {

    ObjectManager::ObjectManager()
    {
    }

    ObjectManager::~ObjectManager()
    {
    }

    ZENO_API void ObjectManager::commit()
    {
    }

    ZENO_API void ObjectManager::revert()
    {
    }

    ZENO_API void ObjectManager::collectingObject(const std::string& id, std::shared_ptr<IObject> obj, std::shared_ptr<INode> view_node, bool bView)
    {
        std::lock_guard lck(m_mtx);

        zeno::getSession().globalState->setCalcObjStatus(zeno::Collecting);

        auto it = m_objects.find(id);
        auto path = view_node->get_uuid_path();
        if (it == m_objects.end()) {
            _ObjInfo info;
            info.obj = obj;
            info.attach_nodes.insert(path);
            m_objects.insert(std::make_pair(id, info));
        }
        else {
            it->second.obj = obj;
            it->second.attach_nodes.insert(path);
        }
        if (bView) {
            m_viewObjs.insert(id);
            if (m_lastViewObjs.find(id) != m_lastViewObjs.end()) {
                m_lastViewObjs.erase(id);   //��һ��������view����һ��Ҳ��view
            }
            else {
                //��һ��û��view�������view��Ҫô����������Ҫô�������´�view
                m_newAdded.insert(id);
            }
        }
        else {
        }
    }

    ZENO_API void ObjectManager::removeObject(const std::string& id)
    {
        std::lock_guard lck(m_mtx);
        m_lastUnregisterObjs.push_back(id); //�ȱ�ǣ���һ��run��ʱ����ȥm_objects���Ƴ�
    }

    ZENO_API void ObjectManager::notifyTransfer(std::shared_ptr<IObject> obj)
    {
        //std::lock_guard lck(m_mtx);
        //CALLBACK_NOTIFY(notifyTransfer, obj)
    }

    ZENO_API void ObjectManager::viewObject(std::shared_ptr<IObject> obj, bool bView)
    {
        //std::lock_guard lck(m_mtx);
    }

    ZENO_API int ObjectManager::registerObjId(const std::string& objprefix)
    {
        if (m_objRegister.find(objprefix) == m_objRegister.end()) {
            m_objRegister.insert(std::make_pair(objprefix, 0));
            m_objRegister[objprefix]++;
            return 0;
        }
        else {
            int newObjId = m_objRegister[objprefix]++;
            return newObjId;
        }
    }

    ZENO_API std::set<ObjPath> ObjectManager::getAttachNodes(const std::string& id)
    {
        auto it = m_objects.find(id);
        if (it != m_objects.end())
        {
            return it->second.attach_nodes;
        }
        return std::set<ObjPath>();
    }

    ZENO_API void ObjectManager::beforeRun()
    {
        std::lock_guard lck(m_mtx);     //���ܴ�ʱ��Ⱦ����load_objects
        m_lastViewObjs = m_viewObjs;
        m_viewObjs.clear();
        m_newAdded.clear();
        m_modify.clear();
        m_remove.clear();
    }

    ZENO_API void ObjectManager::afterRun()
    {
        std::lock_guard lck(m_mtx);
        //m_lastViewObjsʣ�����Ķ�����һ��view������һ��û��view�ġ�
        m_remove = m_lastViewObjs;
        m_lastViewObjs.clear();
        m_removing_objs.clear();
    }

    ZENO_API void ObjectManager::clearLastUnregisterObjs()
    {
        for (auto& key : m_lastUnregisterObjs)
            if (m_objects.find(key) != m_objects.end())
                m_objects.erase(key);
        m_lastUnregisterObjs.clear();
    }

    ZENO_API void ObjectManager::clear_last_run()
    {
        std::lock_guard lck(m_mtx);
        m_newAdded.clear();
        m_modify.clear();
        m_remove.clear();
    }

    ZENO_API void ObjectManager::collect_removing_objs(const std::string& objkey)
    {
        m_removing_objs.insert(objkey);
    }

    ZENO_API void ObjectManager::remove_attach_node_by_removing_objs()
    {
        for (auto obj_key : m_removing_objs) {
            auto nodes = getAttachNodes(obj_key);
            for (auto node_path : nodes) {
                auto spNode = zeno::getSession().mainGraph->getNode(node_path);
                if (spNode)
                    spNode->mark_dirty(true);
            }
            removeObject(obj_key);
        }
    }

    ZENO_API void ObjectManager::collect_modify_objs(std::set<std::string>& newobjKeys)
    {
        std::lock_guard lck(m_mtx);
        m_modify.insert(newobjKeys.begin(), newobjKeys.end());
    }

    ZENO_API void ObjectManager::remove_modify_objs(std::set<std::string>& removeobjKeys)
    {
        std::lock_guard lck(m_mtx);
        m_modify.clear();
    }

    ZENO_API void ObjectManager::getModifyObjsInfo(std::map<std::string, std::shared_ptr<zeno::IObject>>& modifyInteractiveObjs)
    {
        std::lock_guard lck(m_mtx);
        for (auto& key : m_modify)
            if (m_objects.find(key) != m_objects.end())
                modifyInteractiveObjs.insert(std::make_pair(key, m_objects[key].obj));
    }

    ZENO_API void ObjectManager::export_loading_objs(RenderObjsInfo& info)
    {
        std::lock_guard lck(m_mtx);
        for (auto objkey : m_newAdded) {
            auto it = m_objects.find(objkey);
            if (it != m_objects.end()) {
                convertToView(it->second.obj, info.newObjs, std::set<std::string>(), false, it->second.obj->key);
            }
        }
        for (auto objkey : m_modify) {
            auto it = m_objects.find(objkey);
            if (it != m_objects.end())
                convertToView(it->second.obj, info.modifyObjs, std::set<std::string>(), false, it->second.obj->key);
        }
        for (auto objkey : m_remove) {
            auto it = m_objects.find(objkey);
            if (it != m_objects.end()) {
                convertToView(it->second.obj, SharedObjects(), info.remObjs, true, it->second.obj->key);
            }
        }
    }

    ZENO_API void ObjectManager::export_all_view_objs(RenderObjsInfo& info)
    {
        std::lock_guard lck(m_mtx);
        for (auto& key : m_viewObjs) {
            auto& it = m_objects.find(key);
            if (it != m_objects.end())
                convertToView(it->second.obj, info.allObjects, std::set<std::string>(), false, it->second.obj->key);
        }
    }

    ZENO_API void ObjectManager::export_all_view_objs(std::vector<std::pair<std::string, std::shared_ptr<zeno::IObject>>>& info)
    {
        std::lock_guard lck(m_mtx);
        RenderObjsInfo tmp;
        for (auto& key : m_viewObjs) {
            auto& it = m_objects.find(key);
            if (it != m_objects.end())
                convertToView(it->second.obj, tmp.allObjects, std::set<std::string>(), false, it->second.obj->key);
        }
        for (auto& pair : tmp.allObjects)
            info.emplace_back(std::move(pair));
    }

    ZENO_API std::shared_ptr<zeno::IObject> ObjectManager::getObj(std::string name)
    {
        std::lock_guard lck(m_mtx);
        if (m_objects.find(name) != m_objects.end())
            return m_objects[name].obj;
        else {
            if (name.find("/") != std::string::npos)    //this item come from a listObj
            {
                auto viewnodeid = name.substr(0, name.find_first_of("/"));
                if (m_objects.find(viewnodeid) != m_objects.end()) {
                    auto idxlist = name.substr(name.find_first_of("/") + 1);
                    auto reslist = split_str(idxlist, '/');
                    auto root = m_objects[viewnodeid].obj;
                    for (auto& idx : reslist)
                    {
                        if (idx.empty())
                            continue;
                        int i = std::stoi(idx);
                        if (auto list = std::dynamic_pointer_cast<ListObject>(root)) {
                            if (i >= list->arr.size())
                                return nullptr;
                            root = list->arr[i];
                        }
                        else
                            return root;
                    }
                    return root;
                }
            }
        }
        return nullptr;
    }

    void ObjectManager::convertToView(zany const& objToBeConvert, SharedObjects& objConvertResult, std::set<std::string>& keyConvertResult, bool convertKeyOnly, std::string listitemidx)
    {
        if (std::shared_ptr<ListObject> lst = std::dynamic_pointer_cast<ListObject>(objToBeConvert)) {
            for (size_t i = 0; i < lst->arr.size(); i++) {
                std::string idx = "";
                idx = listitemidx + '/' + std::to_string(i);
                convertToView(lst->arr[i], objConvertResult, keyConvertResult, convertKeyOnly, idx);
            }
            return;
        }
        if (!objToBeConvert)
            return;
        else {
            objToBeConvert->key = listitemidx;
            if (convertKeyOnly)
                keyConvertResult.insert(objToBeConvert->key);
            else
                objConvertResult.insert(std::make_pair(objToBeConvert->key, objToBeConvert));
        }
    }

    void ObjectManager::clear()
    {
        //todo
    }

}