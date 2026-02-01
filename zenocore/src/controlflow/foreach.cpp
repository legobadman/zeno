#include <zeno/extra/foreach.h>
#include <Windows.h>

namespace zeno {

    void ForEachBegin::getIconResource(char* recv, size_t cap) {
        recv[0] = '\0';
        //const char* icon = ":/icons/node/switchif.svg";
        //strcpy(recv, icon);
        //recv[strlen(icon)] = '\0';
    }

    void ForEachBegin::getBackgroundClr(char* recv, size_t cap) {
        recv[0] = '\0';
        //const char* icon = ":/icons/node/switchif.svg";
        //strcpy(recv, icon);
        //recv[strlen(icon)] = '\0';
    }

    NodeImpl* ForEachBegin::get_foreachend(NodeImpl* m_pAdapter) {
        auto graph = m_pAdapter->getGraph();
        std::string m_foreach_end_path = m_pAdapter->get_input2_string("ForEachEnd Path");
        auto foreach_end = graph->getNode(m_foreach_end_path);
        if (!foreach_end) {
            throw makeError<KeyError>("foreach_end_path", "the path of foreach_end_path is not exist");
        }
        return foreach_end;
    }

    void ForEachBegin::apply(INodeData* ptrNodeData) {
        NodeImpl* m_pAdapter = static_cast<NodeImpl*>(ptrNodeData);
        IObject2* init_object = m_pAdapter->get_input_obj("Initial Object");
        std::string m_fetch_mehod = m_pAdapter->get_input2_string("Fetch Method");
        std::string m_foreach_end_path = m_pAdapter->get_input2_string("ForEachEnd Path");
        int m_current_iteration = m_pAdapter->get_input2_int("Current Iteration");

        if (!init_object) {
            throw makeError<UnimplError>(m_pAdapter->get_name() + " get empty input object.");
        }
        auto foreach_end_data = get_foreachend(m_pAdapter);
        auto foreach_end = static_cast<ForEachEnd*>(foreach_end_data->coreNode());

        if (m_fetch_mehod == "Initial Object") {
            //看foreachend是迭代object还是container,如果是container，就得取element元素
            auto itemethod = foreach_end_data->get_input2_string("Iterate Method");
            if (itemethod == "By Count") {
                m_pAdapter->set_output("Output Object", zany2(init_object->clone()));
                return;
            }
            else if (itemethod == "By Container") {
                //TODO: 目前只支持list，后续可支持dict
                if (auto spList = dynamic_cast<ListObject*>(init_object)) {
                    int n = spList->size();
                    if (m_current_iteration >= 0 && m_current_iteration < n) {
                        auto elemObj = spList->get(m_current_iteration);
                        zany2 spClonedObj(elemObj->clone());
                        m_pAdapter->set_output("Output Object", std::move(spClonedObj));
                        return;
                    }
                    else {
                        throw makeError<UnimplError>("current iteration on foreach begin exceeds the range of Listobject");
                    }
                }
                else {
                    throw makeError<UnimplError>("Only support ListObject on Initial Object when select `By Container` mode in foreach_end");
                }
            }
            else {
                throw makeError<UnimplError>("Only support `By Count` and `By Container` mode.");
            }
        }
        else if (m_fetch_mehod == "From Last Feedback") {
            int startValue = foreach_end_data->get_input2_int("Start Value");
            if (startValue == m_current_iteration) {
                m_pAdapter->set_output("Output Object", zany2(init_object->clone()));
                return;
            }
            else {
                auto outputObj = foreach_end->get_iterate_object();
                if (outputObj) {
                    m_pAdapter->set_output("Output Object", zany2(outputObj->clone()));
                }
                else {
                    m_pAdapter->set_output("Output Object", zany2(init_object->clone()));
                }
                //outputObj of last iteration as a feedback to next procedure.
                return;
            }
        }
        else if (m_fetch_mehod == "Element of Object") {
            if (auto spList = dynamic_cast<ListObject*>(init_object)) {
                zany2 elemObj(spList->get(m_current_iteration)->clone());
                m_pAdapter->set_output("Output Object", std::move(elemObj));
            }
            else {
                //TODO
                m_pAdapter->set_output("Output Object", nullptr);
            }
        }
        else {
            m_pAdapter->set_output("Output Object", nullptr);
        }
    }

    int ForEachBegin::get_current_iteration(NodeImpl* m_pAdapter) {
        int current_iteration = zeno::reflect::any_cast<int>(m_pAdapter->get_defl_value("Current Iteration"));
        return current_iteration;
    }

    void ForEachBegin::update_iteration(NodeImpl* m_pAdapter, int new_iteration) {
        //m_current_iteration = new_iteration;
        //不能引发事务重新执行，执行权必须由外部Graph发起
        m_pAdapter->update_param_impl("Current Iteration", new_iteration);
        ZImpl(set_primitive_output("Index", new_iteration));
    }

    ZENDEFNODE(ForEachBegin,
    {
        {
            {gParamType_IObject2, "Initial Object"},
            // InnerSocket = 1 当前迭代值外部不可修改，但可被其他参数引用，因此还是作为正式参数，当然有另一种可能，就是支持引用output参数，但当前并没有这个打算
            ParamPrimitive("Current Iteration", gParamType_Int, 0),
            ParamPrimitive("Fetch Method", gParamType_String, "From Last Feedback", zeno::Combobox, std::vector<std::string>{"Initial Object", "From Last Feedback", "Element of Object"}),
            ParamPrimitive("ForEachEnd Path", gParamType_String),
        },
        {
            {gParamType_IObject2, "Output Object"},
            ParamPrimitive("Index", gParamType_Int, 0),
        },
        {},
        {"geom"}
    });



    ForEachEnd::ForEachEnd() {
        m_collect_objs = create_ListObject();
    }

    NodeImpl* ForEachEnd::get_foreach_begin(NodeImpl* m_pAdapter) {
        //这里不能用m_foreach_begin_path，因为可能还没从基类数据同步过来，后者需要apply操作前才会同步
        std::string foreach_begin_path = zeno::any_cast_to_string(m_pAdapter->get_defl_value("ForEachBegin Path"));
        auto graph = m_pAdapter->getGraph();
        auto foreach_begin_node = graph->getNode(foreach_begin_path);
        if (!foreach_begin_node) {
            throw makeError<UnimplError>("unknown foreach begin");
        }
        return foreach_begin_node;
    }

    void ForEachEnd::reset_forloop_settings(NodeImpl* m_pAdapter) {
        if (m_collect_objs)
            m_collect_objs->clear();
        auto foreach_begin_node = get_foreach_begin(m_pAdapter);
        auto foreach_begin = static_cast<ForEachBegin*>(foreach_begin_node->coreNode());
        int start_value = zeno::reflect::any_cast<int>(ZImpl(get_defl_value("Start Value")));
        foreach_begin->update_iteration(foreach_begin_node, start_value);
    }

    bool ForEachEnd::is_continue_to_run(NodeImpl* m_pAdapter, CalcContext* pContext) {
        std::string iter_method = m_pAdapter->get_input2_string("Iterate Method");

        auto foreach_begin_node = get_foreach_begin(m_pAdapter);
        ForEachBegin* foreach_begin = static_cast<ForEachBegin*>(foreach_begin_node->coreNode());
        int current_iter = foreach_begin->get_current_iteration(foreach_begin_node);
        int m_iterations = m_pAdapter->get_input2_int("Iterations");
        int m_increment = m_pAdapter->get_input2_int("Increment");
        int m_start_value = m_pAdapter->get_input2_int("Start Value");
        bool m_need_stop_cond = m_pAdapter->get_input2_bool("Has Stop Condition");
        int m_stop_condition = m_pAdapter->get_input2_int("Stop Condition");

        if (iter_method == "By Count") {
            if (m_iterations <= 0 || m_increment == 0) {//迭代次数非正或increment为0返回
                adjustCollectObjInfo(m_pAdapter);
                return false;
            }
            else {
                if (m_increment > 0) {//正增长
                    if (m_iterations <= current_iter - m_start_value ||
                        (m_need_stop_cond && current_iter >= m_stop_condition)) {
                        adjustCollectObjInfo(m_pAdapter);
                        return false;
                    }
                }
                else {//负增长
                    if (m_iterations <= m_start_value - current_iter ||
                        (m_need_stop_cond && current_iter <= m_stop_condition)) {
                        adjustCollectObjInfo(m_pAdapter);
                        return false;
                    }
                }
            }
            return true;
        }
        else if (iter_method == "By Container") {
            auto foreachbegin_impl = foreach_begin_node;
            IObject2* initobj = foreachbegin_impl->get_input_obj("Initial Object");
            if (!initobj && foreachbegin_impl->is_dirty()) {
                //可能上游还没算，先把上游的依赖解了
                //foreach_begin->preApply(nullptr);
                foreachbegin_impl->execute(pContext);
                initobj = foreachbegin_impl->get_input_obj("Initial Object");
            }
            if (auto spList = dynamic_cast<ListObject*>(initobj)) {
                int n = spList->size();
                if (current_iter >= 0 && current_iter < n) {
                    return true;
                }
                else {
                    adjustCollectObjInfo(m_pAdapter);
                    return false;
                };
            }
            else {
                adjustCollectObjInfo(m_pAdapter);
                return false;
            }
        }
        else {
            adjustCollectObjInfo(m_pAdapter);
            return false;
        }
    }

    void ForEachEnd::increment(NodeImpl* m_pAdapter) {
        std::string m_iterate_method = m_pAdapter->get_input2_string("Iterate Method");
        if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
            NodeImpl* foreach_begin_node = get_foreach_begin(m_pAdapter);
            auto foreach_begin = static_cast<ForEachBegin*>(foreach_begin_node->coreNode());
            int current_iter = foreach_begin->get_current_iteration(foreach_begin_node);
            int m_increment = m_pAdapter->get_input2_int("Increment");
            int new_iter = current_iter + m_increment;
            foreach_begin->update_iteration(foreach_begin_node, new_iter);
        }
        else {
            //TODO: By Container
        }
    }

    IObject2* ForEachEnd::get_iterate_object() {
        return m_iterate_object.get();
    }

    void ForEachEnd::apply(INodeData* ptrNodeData) {}

    void ForEachEnd::apply_foreach(INodeData* ptrNodeData, CalcContext* pContext) {
        NodeImpl* thisNodeData = static_cast<NodeImpl*>(ptrNodeData);
        zany2 iterate_object = zany2(ptrNodeData->clone_input_object("Iterate Object"));
        if (!iterate_object) {
            throw makeError<UnimplError>("No Iterate Object given to `ForEachEnd`");
        }
        m_iterate_object = std::move(iterate_object);

        std::string m_iterate_method = thisNodeData->get_input2_string("Iterate Method");
        std::string m_collect_method = thisNodeData->get_input2_string("Collect Method");
        std::string m_foreach_begin_path = thisNodeData->get_input2_string("ForEachBegin Path");
        int m_iterations = thisNodeData->get_input2_int("Iterations");
        int m_increment = thisNodeData->get_input2_int("Increment");
        int m_start_value = thisNodeData->get_input2_int("Start Value");
        int m_stop_condition = thisNodeData->get_input2_int("Stop Condition");
        bool output_list = thisNodeData->get_input2_bool("Output List");

        //construct the `result` object
        const std::string& uuidpath = thisNodeData->get_uuid_path();

        if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
            if (m_collect_method == "Feedback to Begin") {
                zany2 iobj(m_iterate_object->clone());
                
                if (auto spList = dynamic_cast<zeno::ListObject*>(iobj.get())) {
                    update_list_root_key(spList, uuidpath);
                }
                else {
                    iobj->update_key(uuidpath.c_str());
                }
                thisNodeData->set_output("Output Object", std::move(iobj));
                return;
            }
            else if (m_collect_method == "Gather Each Iteration") {
                auto foreach_begin_node = get_foreach_begin(thisNodeData);
                auto foreach_begin = static_cast<ForEachBegin*>(foreach_begin_node->coreNode());
                int current_iter = foreach_begin->get_current_iteration(foreach_begin_node);

                zany2 iobj(m_iterate_object->clone());
                if (auto spList = dynamic_cast<zeno::ListObject*>(iobj.get())) {
                    update_list_root_key(spList, uuidpath + "\\iter" + std::to_string(current_iter));
                    for (int i = 0; i < spList->size(); i++) {
                        auto cloneobj = spList->get(i);
                        m_collect_objs->push_back(cloneobj->clone());
                        m_collect_objs->m_new_added.insert(get_object_key(cloneobj));
                    }
                }
                else {
                    std::string currIterKey = uuidpath + '\\' + get_object_key(m_iterate_object)
                        + ":iter:" + std::to_string(current_iter);
                    iobj->update_key(stdString2zs(currIterKey));
                    m_collect_objs->push_back(iobj->clone());
                    m_collect_objs->m_new_added.insert(currIterKey);
                }

                //应该要在最后一步合并吧
                if (output_list) {
                    //set_output("Output Object", m_collect_objs->clone());
                }
                else {
                    if (m_collect_objs->size() != 0) {
                        auto firstObj = m_collect_objs->get(0);
                        if (ZObj_Geometry == firstObj->type()) {
                            auto mergedObj = zeno::mergeObjects(m_collect_objs.get());
                            thisNodeData->set_output_object("Output Object", mergedObj);
                            return;
                        }
                    }
                    //如果收集的对象里没有几何对象，那就直接输出list
                    thisNodeData->set_output_object("Output Object", m_collect_objs->clone());
                }
            }
            else {
                assert(false);
                thisNodeData->set_output_object("Output Object", nullptr);
            }
        }
        else {
            throw makeError<UnimplError>("only support By Count or By Container at ForeachEnd");
        }
    }

    void ForEachEnd::clearCalcResults() {
        //m_last_collect_objs.clear();
        m_collect_objs->clear();
        m_iterate_object.reset();
    }

    void ForEachEnd::adjustCollectObjInfo(NodeImpl* ptrNodeData) {
        std::function<void(IObject2*)> flattenList = [&flattenList, this](IObject2* obj) {
            if (auto _spList = dynamic_cast<zeno::ListObject*>(obj)) {
                for (int i = 0; i < _spList->size(); ++i) {
                    flattenList(_spList->get(i));
                }
            }
            else {
                m_collect_objs->m_new_removed.insert(get_object_key(obj));
            }
        };
#if 0
        for (auto it = m_last_collect_objs.begin(); it != m_last_collect_objs.end();) {
            zeno::String _key = (*it)->key();
            std::string _skey = zsString2Std(_key);
            if (m_collect_objs->m_new_added.find(_skey) == m_collect_objs->m_new_added.end()) {
                flattenList(*it);
                it = m_last_collect_objs.erase(it);
            }
            else {
                m_collect_objs->m_modify.insert(_skey);
                m_collect_objs->m_new_added.erase(_skey);
                ++it;
            }
        }
        m_last_collect_objs.clear();
        m_last_collect_objs = m_collect_objs->get();
#endif

        bool output_list = ptrNodeData->get_input2_bool("Output List");
        if (output_list) {
            ptrNodeData->set_output_object("Output Object", m_collect_objs->clone());
        }

        if (ptrNodeData->get_input2_bool("Clear Cache In ForEach Begin")) {
            NodeImpl* foreach_begin_node = get_foreach_begin(ptrNodeData);
            ForEachBegin* foreach_begin = static_cast<ForEachBegin*>(foreach_begin_node->coreNode());
            //foreach_begin->m_pAdapter->mark_takeover();
        }
    }


    ZENDEFNODE(ForEachEnd, {
        {
            ParamObject("Iterate Object", gParamType_IObject2),
            ParamPrimitive("Iterate Method", gParamType_String, "By Count", zeno::Combobox, std::vector<std::string>{"By Count", "By Container", "By Geometry Point", "By Geometry Face"}),
            ParamPrimitive("Collect Method", gParamType_String, "Feedback to Begin", zeno::Combobox, std::vector<std::string>{"Feedback to Begin", "Gather Each Iteration"}),
            ParamPrimitive("ForEachBegin Path", gParamType_String),
            ParamPrimitive("Iterations", gParamType_Int, 10, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Increment", gParamType_Int, 1, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Start Value", gParamType_Int, 0, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Stop Condition", gParamType_Int, 0, zeno::Slider, std::vector<int>{0, 10, 1}, "enabled = parameter('Iterate Method').value == 'By Count' && parameter('Has Stop Condition').value == 1;"),
            ParamPrimitive("Has Stop Condition", gParamType_Bool, false),
            ParamPrimitive("Output List", gParamType_Bool, false, zeno::Checkbox, Any()),
            ParamPrimitive("Clear Cache In ForEach Begin", gParamType_Bool, false, zeno::Checkbox, Any())
        },
        {
            {gParamType_IObject2, "Output Object"}
        },
        {},
        {"geom"}
    });
}
