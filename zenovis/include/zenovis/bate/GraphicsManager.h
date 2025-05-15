#pragma once

#include <map>
#include <vector>
#include <zeno/types/UserData.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/MapStablizer.h>
#include <zeno/utils/PolymorphicMap.h>
#include <zeno/utils/log.h>
#include <zenovis/bate/IGraphic.h>
#include <zenovis/Scene.h>
#include <zeno/core/Graph.h>

namespace zenovis {

struct GraphicsManager {
    Scene *scene;

    zeno::MapStablizer<zeno::PolymorphicMap<std::map<
        std::string, std::unique_ptr<IGraphic>>>> graphics;
    zeno::PolymorphicMap<std::map<std::string, std::unique_ptr<IGraphic>>> realtime_graphics;

    explicit GraphicsManager(Scene *scene) : scene(scene) {
    }

    bool load_realtime_object(const std::string &key, std::shared_ptr<zeno::IObject> const &obj) {
        int interactive;
        if (obj->userData()->has("interactive"))
            interactive = obj->userData()->get_int("interactive");
        else return false;
        if (interactive) {
            zeno::log_debug("load_realtime_object: loading realtime graphics [{}]", key);
            // printf("reload %s\n", key.c_str());
            auto ig = makeGraphic(scene, obj.get());
            zeno::log_debug("load_realtime_object: loaded realtime graphics to {}", ig.get());
            ig->nameid = key;
            ig->objholder = obj;
            realtime_graphics.try_emplace(key, std::move(ig));
            return true;
        }
        return false;
    }

    bool add_object(zeno::zany obj) {
        if (auto spList = std::dynamic_pointer_cast<zeno::ListObject>(obj)) {
            for (auto obj : spList->m_impl->get()) {
                bool ret = add_object(obj);
                assert(ret);
            }
            return true;
        }
        if (auto spDict = std::dynamic_pointer_cast<zeno::DictObject>(obj)) {
            for (auto& [key, spObject] : spDict->get()) {
                bool ret = add_object(spObject);
                assert(ret);
            }
            return true;
        }

        const std::string& key = zsString2Std(obj->key());
        if (!obj || key.empty())
            return false;

        auto& wtf = graphics.m_curr.m_curr;
        auto it = wtf.find(key);
        if (it == wtf.end()) {
            zeno::log_debug("load_object: loading graphics [{}]", key);
            auto ig = makeGraphic(scene, obj.get());
            if (!ig)
                return false;
            zeno::log_debug("load_object: loaded graphics to {}", ig.get());
            ig->nameid = key;
            ig->objholder = obj;
            graphics.m_curr.m_curr.insert(std::make_pair(key, std::move(ig)));
        }
        else {
            auto ig = makeGraphic(scene, obj.get());
            if (!ig)
                return false;
            ig->nameid = key;
            ig->objholder = obj;
            it->second = std::move(ig);
        }
        return true;
    }

    bool remove_object_bykey(const std::string& key) {
        auto& graphics_ = graphics.m_curr.m_curr;
        auto iter = graphics_.find(key);
        if (iter == graphics_.end())
            return false;

        graphics_.erase(key);
        return true;
    }

    bool remove_object(zeno::zany spObj) {
        if (auto spList = std::dynamic_pointer_cast<zeno::ListObject>(spObj)) {
            for (auto obj : spList->m_impl->get()) {
                bool ret = remove_object(obj);
                assert(ret);
            }
            return true;
        }
        if (auto spDict = std::dynamic_pointer_cast<zeno::DictObject>(spObj)) {
            for (auto& [key, spObject] : spDict->get()) {
                bool ret = remove_object(spObject);
                assert(ret);
            }
            return true;
        }
        const std::string& key = zsString2Std(spObj->key());
        auto& graphics_ = graphics.m_curr.m_curr;
        auto iter = graphics_.find(key);
        if (iter == graphics_.end())
            return false;

        graphics_.erase(key);
        return true;
    }

    bool process_listobj(std::shared_ptr<zeno::ListObject> spList) {
        for (auto spObject : spList->m_impl->get()) {
            assert(spObject);
            std::string const& key = zsString2Std(spObject->key());
            if (spList->m_impl->m_new_added.find(key) != spList->m_impl->m_new_added.end() ||
                spList->m_impl->m_modify.find(key) != spList->m_impl->m_modify.end()) {
                bool ret = false;
                if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(spObject)) {
                    ret = process_listobj(_spList);
                }
                else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(spObject)) {
                    ret = process_dictobj(_spDict);
                }
                else {
                    ret = add_object(spObject);
                }
                assert(ret);
            }
        }
        for (auto& key : spList->m_impl->m_new_removed) {
            auto& graphics_ = graphics.m_curr.m_curr;
            auto iter = graphics_.find(key);
            if (iter == graphics_.end())
                continue;
            graphics_.erase(key);
        }
        return true;
    }

    bool process_dictobj(std::shared_ptr<zeno::DictObject> spDict) {
        for (auto& [key, spObject] : spDict->get()) {
            assert(spObject);
            std::string const& skey = zsString2Std(spObject->key());
            if (spDict->m_new_added.find(skey) != spDict->m_new_added.end() ||
                spDict->m_modify.find(skey) != spDict->m_modify.end()) {
                bool ret = false;
                if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(spObject)) {
                    ret = process_listobj(_spList);
                }
                else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(spObject)) {
                    ret = process_dictobj(_spDict);
                }
                else {
                    ret = add_object(spObject);
                }
                assert(ret);
            }
        }
        for (auto& key : spDict->m_new_removed) {
            auto& graphics_ = graphics.m_curr.m_curr;
            auto iter = graphics_.find(key);
            if (iter == graphics_.end())
                continue;
            graphics_.erase(key);
        }
        return true;
    }

    void reload(const zeno::render_reload_info& info) {
        auto& sess = zeno::getSession();
        if (zeno::Reload_SwitchGraph == info.policy) {
            //由于对象和节点是一一对应，故切换图层次结构必然导致所有对象被重绘
            graphics.clear();
            std::shared_ptr<zeno::Graph> spGraph = sess.getGraphByPath(info.current_ui_graph);
            assert(spGraph);
            if (spGraph->isAssets()) {
                //资产图不能view，因为没有实例化，不属于运行图的范畴
                return;
            }
            const auto& viewnodes = spGraph->get_viewnodes();
            //其实是否可以在外面提前准备好对象列表？
            for (auto viewnode : viewnodes) {
                auto spNode = spGraph->getNode(viewnode);
                zeno::zany spObject = spNode->get_default_output_object();
                if (spObject) {
                    add_object(spObject);
                }
                else {

                }
            }
        }
        else if (zeno::Reload_ToggleView == info.policy) {
            assert(info.objs.size() == 1);
            //TODO: 谁说view只能一个的？？
            const auto& update = info.objs[0];
            if (update.reason == zeno::Update_Remove) {
                for (const std::string& remkey : update.remove_objs) {
                    remove_object_bykey(remkey);
                }
            }
            else {
                auto spNode = sess.getNodeByUuidPath(update.uuidpath_node_objkey);
                assert(spNode);
                zeno::zany spObject = spNode->get_default_output_object();
                if (spObject) {
                    if (update.reason == zeno::Update_View) {
                        add_object(spObject);
                    }
                }
                else {
                    //可能还没计算
                }
            }
        }
        else if (zeno::Reload_Calculation == info.policy) {
            for (const zeno::render_update_info& update : info.objs) {
                auto spNode = sess.getNodeByUuidPath(update.uuidpath_node_objkey);
                assert(spNode);
                zeno::zany spObject = spNode->get_default_output_object();
                if (spObject) {
                    //可能是对象没有通过子图的Suboutput连出来
                    if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(spObject)) {
                        {//可能有和listobj同名但不是list类型的对象存在，需先清除
                            auto& graphics_ = graphics.m_curr.m_curr;
                            std::string listkey = zsString2Std(_spList->key());
                            const auto& it = listkey.find('\\');
                            const std::string& key = it == std::string::npos ? listkey : listkey.substr(0, it);
                            for (auto it = graphics_.begin(); it != graphics_.end(); ) {
                                if (it->first == key)
                                    it = graphics_.erase(it);
                                else
                                    ++it;
                            }
                        }

                        process_listobj(_spList);
                    }
                    else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(spObject)) {
                        {//可能有和dictobj同名但不是dict类型的对象存在，需先清除
                            auto& graphics_ = graphics.m_curr.m_curr;
                            std::string dictkey = zsString2Std(_spDict->key());
                            const auto& it = dictkey.find('\\');
                            const std::string& key = it == std::string::npos ? dictkey : dictkey.substr(0, it);
                            for (auto it = graphics_.begin(); it != graphics_.end(); ) {
                                if (it->first == key)
                                    it = graphics_.erase(it);
                                else
                                    ++it;
                            }
                        }

                        process_dictobj(_spDict);
                    }
                    else {
                        {//可能有和obj同名但是list类型或dict类型的对象存在，需先清除
                            auto& graphics_ = graphics.m_curr.m_curr;
                            std::string objkey = zsString2Std(spObject->key());
                            for (auto it = graphics_.begin(); it != graphics_.end(); ) {
                                if (it->first.find(objkey + '\\') != std::string::npos) {
                                    it = graphics_.erase(it);
                                }
                                else
                                    ++it;
                            }
                        }

                        add_object(spObject);
                    }
                }
            }
        }
    }

    void load_objects3(const std::vector<zeno::render_update_info>& infos) {
        auto& sess = zeno::getSession();
        for (const zeno::render_update_info& info : infos) {
            auto spNode = sess.getNodeByUuidPath(info.uuidpath_node_objkey);
            assert(spNode);
            zeno::zany spObject = spNode->get_default_output_object();
            assert(spObject);

            std::string const& objkey = zsString2Std(spObject->key());
            if (info.reason == zeno::Update_Reconstruct) {
                add_object(spObject);
            }
            else if (info.reason == zeno::Update_View) {
                //要观察当前绘制端是否已经缓存了objkey
                auto& wtf = graphics.m_curr.m_curr;
                auto it = wtf.find(objkey);
                if (it == wtf.end()) {
                    add_object(spObject);
                }
                else {
                    //只是切换view而已，而这个要绘制的object已经在这里绘制端缓存了，不需要重新load
                    int j;
                    j = 0;
                }
            }
            else if (info.reason == zeno::Update_Remove) {
                //包括节点删除以及view移除的情况，在绘制端看来都是移除
                remove_object(spObject);
            }
        }
    }

    void load_objects2(const zeno::RenderObjsInfo& objs) {
        for (auto [key, spObj] : objs.remObjs) {    //if obj both in remObjs and in newObjs, need remove first?
            remove_object(spObj);
        }
        for (auto [key, spObj] : objs.newObjs) {
            add_object(spObj);
        }
        for (auto [key, spObj] : objs.modifyObjs) {
            bool isListDict = false;
            if (auto spList = std::dynamic_pointer_cast<zeno::ListObject>(spObj)) {
                isListDict = true;
            } else if (auto spDict = std::dynamic_pointer_cast<zeno::DictObject>(spObj)) {
                isListDict = true;
            }
            if (isListDict) {
                auto& wtf = graphics.m_curr.m_curr;
                for (auto it = wtf.begin(); it != wtf.end(); ) {
                    if (it->first.find(key) != std::string::npos)
                        it = wtf.erase(it);
                    else
                        ++it;
                }
            }
            add_object(spObj);
        }
    }

    //deprecated
    bool load_objects(std::vector<std::pair<std::string, std::shared_ptr<zeno::IObject>>> const &objs) {
        auto ins = graphics.insertPass();
        realtime_graphics.clear();
        for (auto const &[key, obj] : objs) {
            if (load_realtime_object(key, obj)) continue;
            if (ins.may_emplace(key)) {
                zeno::log_debug("load_object: loading graphics [{}]", key);
                auto ig = makeGraphic(scene, obj.get());
                zeno::log_debug("load_object: loaded graphics to {}", ig.get());
                ig->nameid = key;
                ig->objholder = obj;
                ins.try_emplace(key, std::move(ig));
            }
        }
        return ins.has_changed();
    }

    void draw() {
        for (auto const &[key, gra] : graphics.pairs<IGraphicDraw>()) {
            // if (realtime_graphics.find(key) == realtime_graphics.end())
            gra->draw();
        }
        //for (auto const &[key, gra] : realtime_graphics.pairs<IGraphicDraw>()) {
        //    gra->draw();
        //}
        // printf("graphics count: %d\n", graphics.size());
        // printf("realtime graphics count: %d\n", realtime_graphics.size());
    }
};

} // namespace zenovis
