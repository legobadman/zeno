#include <zeno/core/NodeImpl.h>
#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/DictObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/helper.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/geo/geometryutil.h>


namespace zeno
{
    struct ForEachEnd;

    struct ForEachBegin : INode
    {
        INode* get_foreachend() {
            auto graph = m_pAdapter->getGraph();
            std::string m_foreach_end_path = ZImpl(get_input2<std::string>("ForEachEnd Path"));
            auto foreach_end = graph->getNode(m_foreach_end_path)->coreNode();
            if (!foreach_end) {
                throw makeError<KeyError>("foreach_end_path", "the path of foreach_end_path is not exist");
            }
            return foreach_end;
        }

        void apply() override {
            zany init_object = ZImpl(get_input("Initial Object"));
            std::string m_fetch_mehod = ZImpl(get_input2<std::string>("Fetch Method"));
            std::string m_foreach_end_path = ZImpl(get_input2<std::string>("ForEachEnd Path"));
            int m_current_iteration = ZImpl(get_input2<int>("Current Iteration"));

            if (!init_object) {
                throw makeError<UnimplError>(m_pAdapter->get_name() + " get empty input object.");
            }
            auto foreach_end = get_foreachend();

            if (m_fetch_mehod == "Initial Object") {
                //看foreachend是迭代object还是container,如果是container，就得取element元素
                auto itemethod = foreach_end->get_input2_string("Iterate Method");
                if (itemethod == "By Count") {
                    ZImpl(set_output("Output Object", init_object));
                    return;
                }
                else if (itemethod == "By Container") {
                    //TODO: 目前只支持list，后续可支持dict
                    if (auto spList = std::dynamic_pointer_cast<ListObject>(init_object)) {
                        int n = spList->m_impl->size();
                        if (m_current_iteration >= 0 && m_current_iteration < n) {
                            zany elemObj = spList->m_impl->get(m_current_iteration);
                            zany spClonedObj = elemObj->clone();
                            ZImpl(set_output("Output Object", spClonedObj));
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
                int startValue = foreach_end->get_input2_int("Start Value");
                if (startValue == m_current_iteration) {
                    ZImpl(set_output("Output Object", init_object));
                    return;
                }
                else {
                    zany outputObj = foreach_end->get_iterate_object();
                    if (outputObj) {
                        ZImpl(set_output("Output Object", outputObj));
                    }
                    else {
                        ZImpl(set_output("Output Object", init_object));
                    }
                    //outputObj of last iteration as a feedback to next procedure.
                    return;
                }
            }
            else if (m_fetch_mehod == "Element of Object") {
                //TODO
                ZImpl(set_output("Output Object", nullptr));
                return;
            }
            ZImpl(set_output("Output Object", nullptr));
        }

        int get_current_iteration() {
            int current_iteration = zeno::reflect::any_cast<int>(m_pAdapter->get_defl_value("Current Iteration"));
            return current_iteration;
        }

        void update_iteration(int new_iteration) {
            //m_current_iteration = new_iteration;
            //不能引发事务重新执行，执行权必须由外部Graph发起
            zeno::reflect::Any oldvalue;
            m_pAdapter->update_param_impl("Current Iteration", new_iteration, oldvalue);
            ZImpl(set_primitive_output("Index", new_iteration));
        }
    };

    ZENDEFNODE(ForEachBegin,
    {
        {
            {gParamType_IObject, "Initial Object"},
            // InnerSocket = 1 当前迭代值外部不可修改，但可被其他参数引用，因此还是作为正式参数，当然有另一种可能，就是支持引用output参数，但当前并没有这个打算
            ParamPrimitive("Current Iteration", gParamType_Int, 0),
            ParamPrimitive("Fetch Method", gParamType_String, "From Last Feedback", zeno::Combobox, std::vector<std::string>{"Initial Object", "From Last Feedback", "Element of Object"}),
            ParamPrimitive("ForEachEnd Path", gParamType_String),
        },
        {
            {gParamType_IObject, "Output Object"},
            ParamPrimitive("Index", gParamType_Int, 0),
        },
        {},
        {"geom"}
    });


    struct ForEachEnd : INode
    {
        ForEachEnd() {
            m_collect_objs = create_ListObject();
        }

        ForEachBegin* get_foreach_begin() {
            //这里不能用m_foreach_begin_path，因为可能还没从基类数据同步过来，后者需要apply操作前才会同步
            std::string foreach_begin_path = zeno::any_cast_to_string(m_pAdapter->get_defl_value("ForEachBegin Path"));
            auto graph = this->m_pAdapter->getGraph();
            auto foreach_begin_node = graph->getNode(foreach_begin_path);
            if (!foreach_begin_node) {
                throw makeError<UnimplError>("unknown foreach begin");
            }
            auto foreach_begin = dynamic_cast<ForEachBegin*>(foreach_begin_node->coreNode());
            if (!foreach_begin) {
                throw makeError<KeyError>("foreach_begin_path", "the path of foreach_begin_path is not exist");
            }
            return foreach_begin;
        }

        void reset_forloop_settings() override {
            m_collect_objs->m_impl->clear();
            auto foreach_begin = get_foreach_begin();
            int start_value = zeno::reflect::any_cast<int>(ZImpl(get_defl_value("Start Value")));
            foreach_begin->update_iteration(start_value);
        }

        bool is_continue_to_run(CalcContext* pContext) override {
            std::string iter_method = ZImpl(get_input2<std::string>("Iterate Method"));

            ForEachBegin* foreach_begin = get_foreach_begin();
            int current_iter = foreach_begin->get_current_iteration();
            int m_iterations = ZImpl(get_input2<int>("Iterations"));
            int m_increment = ZImpl(get_input2<int>("Increment"));
            int m_start_value = ZImpl(get_input2<int>("Start Value"));
            int m_stop_condition = ZImpl(get_input2<int>("Stop Condition"));

            if (iter_method == "By Count") {
                if (m_iterations <= 0 || m_increment == 0) {//迭代次数非正或increment为0返回
                    adjustCollectObjInfo();
                    return false;
                }
                else {
                    if (m_increment > 0) {//正增长
                        if (m_iterations <= current_iter - m_start_value || current_iter >= m_stop_condition) {
                            adjustCollectObjInfo();
                            return false;
                        }
                    } else {//负增长
                        if (m_iterations <= m_start_value - current_iter || current_iter <= m_stop_condition) {
                            adjustCollectObjInfo();
                            return false;
                        }
                    }
                }
                return true;
            }
            else if (iter_method == "By Container") {
                auto foreachbegin_impl = foreach_begin->m_pAdapter;
                zany initobj = foreachbegin_impl->get_input("Initial Object");
                if (!initobj && foreachbegin_impl->is_dirty()) {
                    //可能上游还没算，先把上游的依赖解了
                    //foreach_begin->preApply(nullptr);
                    foreachbegin_impl->doApply(pContext);
                    initobj = foreachbegin_impl->get_input("Initial Object");
                }
                if (auto spList = std::dynamic_pointer_cast<ListObject>(initobj)) {
                    int n = spList->m_impl->size();
                    if (current_iter >= 0 && current_iter < n) {
                        return true;
                    } else {
                        adjustCollectObjInfo();
                        return false;
                    };
                }
                else {
                    adjustCollectObjInfo();
                    return false;
                }
            }
            else {
                adjustCollectObjInfo();
                return false;
            }
        }

        void increment() override {
            std::string m_iterate_method = ZImpl(get_input2<std::string>("Iterate Method"));
            if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
                auto foreach_begin = get_foreach_begin();
                int current_iter = foreach_begin->get_current_iteration();
                int m_increment = ZImpl(get_input2<int>("Increment"));
                int new_iter = current_iter + m_increment;
                foreach_begin->update_iteration(new_iter);
            }
            else {
                //TODO: By Container
            }
        }

        zany get_iterate_object() override {
            return m_iterate_object;
        }

        void apply() override {
            zany iterate_object = ZImpl(get_input("Iterate Object"));
            if (!iterate_object) {
                throw makeError<UnimplError>("No Iterate Object given to `ForEachEnd`");
            }
            std::string m_iterate_method = ZImpl(get_input2<std::string>("Iterate Method"));
            std::string m_collect_method = ZImpl(get_input2<std::string>("Collect Method"));
            std::string m_foreach_begin_path = ZImpl(get_input2<std::string>("ForEachBegin Path"));
            int m_iterations = ZImpl(get_input2<int>("Iterations"));
            int m_increment = ZImpl(get_input2<int>("Increment"));
            int m_start_value = ZImpl(get_input2<int>("Start Value"));
            int m_stop_condition = ZImpl(get_input2<int>("Stop Condition"));

            //construct the `result` object
            m_iterate_object = iterate_object;

            if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
                if (m_collect_method == "Feedback to Begin") {
                    zany iobj = m_iterate_object->clone();
                    if (std::shared_ptr< zeno::ListObject> spList = std::dynamic_pointer_cast<zeno::ListObject>(iobj)) {
                        update_list_root_key(spList.get(), ZImpl(get_uuid_path()));
                    } else if (std::shared_ptr< zeno::DictObject> spDict = std::dynamic_pointer_cast<zeno::DictObject>(iobj)) {
                        update_dict_root_key(spDict.get(), ZImpl(get_uuid_path()));
                    } else {
                        iobj->update_key(stdString2zs(ZImpl(get_uuid_path())));
                    }
                    ZImpl(set_output("Output Object", iobj));
                    return;
                } 
                else if (m_collect_method == "Gather Each Iteration") {
                    auto foreach_begin = get_foreach_begin();
                    int current_iter = foreach_begin->get_current_iteration();

                    zany iobj = m_iterate_object->clone();
                    if (auto spList = std::dynamic_pointer_cast<zeno::ListObject>(iobj)) {
                        update_list_root_key(spList.get(), ZImpl(get_uuid_path()) + "\\iter" + std::to_string(current_iter));
                        for (int i = 0; i < spList->m_impl->size(); i++) {
                            zany cloneobj = spList->m_impl->get(i);
                            m_collect_objs->m_impl->append(cloneobj);
                            m_collect_objs->m_impl->m_new_added.insert(zsString2Std(cloneobj->key()));
                        }
                    } else if (auto spDict = std::dynamic_pointer_cast<zeno::DictObject>(iobj)) {
                        update_dict_root_key(spDict.get(), ZImpl(get_uuid_path()) + "\\iter" + std::to_string(current_iter));
                        for (auto& [key, obj] : spDict->get()) {
                            m_collect_objs->m_impl->append(obj);
                            m_collect_objs->m_impl->m_new_added.insert(zsString2Std(obj->key()));
                        }
                    } else { 
                        std::string currIterKey = ZImpl(get_uuid_path()) + '\\' + zsString2Std(iterate_object->key())
                            + ":iter:" + std::to_string(current_iter);
                        iobj->update_key(stdString2zs(currIterKey));
                        m_collect_objs->m_impl->append(iobj);
                        m_collect_objs->m_impl->m_new_added.insert(currIterKey);
                    }

                    //先兼容Houdini，直接以merge的方式返回，如果日后有需求，再提供选项返回list
                    if (m_collect_objs->size() != 0) {
                        if (auto geo = std::dynamic_pointer_cast<zeno::GeometryObject_Adapter>(m_collect_objs->m_impl->get(0))) {
                            auto mergedObj = zeno::mergeObjects(m_collect_objs);
                            ZImpl(set_output("Output Object", mergedObj));
                            return;
                        }
                    }
                    ZImpl(set_output("Output Object", m_collect_objs));
                    return;
                }
                else {
                    assert(false);
                    ZImpl(set_output("Output Object", nullptr));
                }
            }
            else {
                throw makeError<UnimplError>("only support By Count or By Container at ForeachEnd");
            }
            ZImpl(set_output("Output Object", nullptr));
        }

        void adjustCollectObjInfo() {
            std::function<void(zany)> flattenList = [&flattenList, this](zany obj) {
                if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(obj)) {
                    for (int i = 0; i < _spList->m_impl-> size(); ++i) {
                        flattenList(_spList->m_impl->get(i));
                    }
                } else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(obj)) {
                    for (auto& [key, obj] : _spDict->get()) {
                        flattenList(obj);
                    }
                } else {
                    m_collect_objs->m_impl->m_new_removed.insert(zsString2Std(obj->key()));
                }
            };
            for (auto it = m_last_collect_objs.begin(); it != m_last_collect_objs.end();) {
                zeno::String _key = (*it)->key();
                std::string _skey = zsString2Std(_key);
                if (m_collect_objs->m_impl->m_new_added.find(_skey) == m_collect_objs->m_impl->m_new_added.end()) {
                    flattenList(*it);
                    it = m_last_collect_objs.erase(it);
                } else {
                    m_collect_objs->m_impl->m_modify.insert(_skey);
                    m_collect_objs->m_impl->m_new_added.erase(_skey);
                    ++it;
                }
            }
            m_last_collect_objs.clear();
            m_last_collect_objs = m_collect_objs->m_impl->get();
        }

        zany m_iterate_object;
        zeno::SharedPtr<ListObject> m_collect_objs;     //TODO: 如果foreach的对象是Dict，但这里收集的结果将会以list返回出去，以后再支持Dict的收集
        std::vector<zany> m_last_collect_objs;
    };

    ZENDEFNODE(ForEachEnd, {
        {
            ParamObject("Iterate Object", gParamType_IObject),
            ParamPrimitive("Iterate Method", gParamType_String, "By Count", zeno::Combobox, std::vector<std::string>{"By Count", "By Container", "By Geometry Point", "By Geometry Face"}),
            ParamPrimitive("Collect Method", gParamType_String, "Feedback to Begin", zeno::Combobox, std::vector<std::string>{"Feedback to Begin", "Gather Each Iteration"}),
            ParamPrimitive("ForEachBegin Path", gParamType_String),
            ParamPrimitive("Iterations", gParamType_Int, 10, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Increment", gParamType_Int, 1, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Start Value", gParamType_Int, 0, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';"),
            ParamPrimitive("Stop Condition", gParamType_Int, 1, zeno::Lineedit, Any(), "enabled = parameter('Iterate Method').value == 'By Count';")
        },
        {
            {gParamType_IObject, "Output Object"}
        },
        {},
        {"geom"}
    });
}
