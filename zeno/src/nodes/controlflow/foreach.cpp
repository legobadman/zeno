#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/core/Graph.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/core/reflectdef.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/helper.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno
{
    struct ZDEFNODE() ForEachBegin : INode
    {
        ReflectCustomUI m_uilayout = {
            _Group {
                _CommonParam {"init_object", ParamObject("Initial Object", Socket_Clone)},
            },
            //输出：
            _Group {
                _CommonParam {"", ParamObject("Output Object")},
            }
        };

        std::shared_ptr<INode> get_foreachend() {
            std::shared_ptr<Graph> graph = this->getGraph().lock();
            std::shared_ptr<INode> foreach_end = graph->getNode(m_foreach_end_path);
            if (!foreach_end) {
                throw makeError<KeyError>("foreach_end_path", "the path of foreach_end_path is not exist");
            }
            return foreach_end;
        }

        std::shared_ptr<IObject> apply(std::shared_ptr<IObject> init_object) {
            if (!init_object) {
                throw makeError<UnimplError>(this->get_name() + " get empty input object.");
            }
            _out_iteration = m_current_iteration;
            auto foreach_end = get_foreachend();

            if (m_fetch_mehod == "Initial Object") {
                //看foreachend是迭代object还是container,如果是container，就得取element元素
                std::string itemethod = zeno::reflect::any_cast<std::string>(foreach_end->get_defl_value("Iterate Method"));
                if (itemethod == "By Count") {
                    return init_object;
                }
                else if (itemethod == "By Container") {
                    //TODO: 目前只支持list，后续可支持dict
                    if (auto spList = std::dynamic_pointer_cast<ListObject>(init_object)) {
                        int n = spList->size();
                        if (m_current_iteration >= 0 && m_current_iteration < n) {
                            zany elemObj = spList->get(m_current_iteration);
                            return elemObj;
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
                int startValue = zeno::reflect::any_cast<int>(foreach_end->get_defl_value("Start Value"));
                if (startValue == m_current_iteration) {
                    return init_object;
                }
                else {
                    std::shared_ptr<IObject> outputObj = foreach_end->get_iterate_object();
                    //outputObj of last iteration as a feedback to next procedure.
                    return outputObj ? outputObj : init_object;
                }
            }
            else if (m_fetch_mehod == "Element of Object") {
                //TODO
                return nullptr;
            }
            return nullptr;
        }

        int get_current_iteration() {
            int current_iteration = zeno::reflect::any_cast<int>(get_defl_value("Current Iteration"));
            return current_iteration;
        }

        void update_iteration(int new_iteration) {
            m_current_iteration = new_iteration;
            //不能引发事务重新执行，执行权必须由外部Graph发起
            zeno::reflect::Any oldvalue;
            update_param_impl("Current Iteration", m_current_iteration, oldvalue);
        }

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Fetch Method", Control = zeno::Combobox, ComboBoxItems = ("Initial Object", "From Last Feedback", "Element of Object"))
        std::string m_fetch_mehod = "From Last Feedback";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "ForEachEnd Path")
        std::string m_foreach_end_path;

        //当前迭代值外部不可修改，但可被其他参数引用，因此还是作为正式参数，当然有另一种可能，就是支持引用output参数，但当前并没有这个打算
        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Current Iteration", InnerSocket = 1)
        int m_current_iteration = 0;

        ZPROPERTY(Role = zeno::Role_OutputPrimitive, DisplayName = "Current Iteration")
        int _out_iteration = 0;
    };


    struct ZDEFNODE() ForEachEnd : INode
    {
        ReflectCustomUI m_uilayout = {
            _Group {
                {"iterate_object", ParamObject("Iterate Object", Socket_Clone)},
            },
            //输出：
            _Group {
                {"", ParamObject("Output Object")},
            }
        };

        ForEachEnd() : m_collect_objs(std::make_shared<ListObject>()) {

        }

        std::shared_ptr<ForEachBegin> get_foreach_begin() {
            //这里不能用m_foreach_begin_path，因为可能还没从基类数据同步过来，后者需要apply操作前才会同步
            std::string foreach_begin_path = zeno::reflect::any_cast<std::string>(get_defl_value("ForEachBegin Path"));
            std::shared_ptr<Graph> graph = this->getGraph().lock();
            std::shared_ptr<ForEachBegin> foreach_begin = std::dynamic_pointer_cast<ForEachBegin>(graph->getNode(foreach_begin_path));
            if (!foreach_begin) {
                throw makeError<KeyError>("foreach_begin_path", "the path of foreach_begin_path is not exist");
            }
            return foreach_begin;
        }

        ZENO_API void reset_forloop_settings() override {
            m_collect_objs->clear();
            std::shared_ptr<ForEachBegin> foreach_begin = get_foreach_begin();
            int start_value = zeno::reflect::any_cast<int>(get_defl_value("Start Value"));
            //挺可悲的，明明有一个m_start_value，但因为ui修改的时候没来得及同步过来，而拿不了
            foreach_begin->update_iteration(/*m*/start_value);
            m_start_value = start_value;
            m_stop_condition = zeno::reflect::any_cast<int>(get_defl_value("Stop Condition"));
            m_iterations = zeno::reflect::any_cast<int>(get_defl_value("Iterations"));
            m_increment = zeno::reflect::any_cast<int>(get_defl_value("Increment"));
        }

        ZENO_API bool is_continue_to_run() override {
            std::string iter_method = zeno::reflect::any_cast<std::string>(get_defl_value("Iterate Method"));

            std::shared_ptr<ForEachBegin> foreach_begin = get_foreach_begin();
            int current_iter = foreach_begin->get_current_iteration();

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
                zany initobj = foreach_begin->get_input("Initial Object");
                if (!initobj && foreach_begin->is_dirty()) {
                    //可能上游还没算，先把上游的依赖解了
                    //foreach_begin->preApply(nullptr);
                    foreach_begin->doApply(nullptr);
                    initobj = foreach_begin->get_input("Initial Object");
                }
                if (auto spList = std::dynamic_pointer_cast<ListObject>(initobj)) {
                    int n = spList->size();
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

        ZENO_API void increment() override {
            if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
                std::shared_ptr<ForEachBegin> foreach_begin = get_foreach_begin();
                int current_iter = foreach_begin->get_current_iteration();
                int new_iter = current_iter + m_increment;
                foreach_begin->update_iteration(new_iter);
            }
            else {
                //TODO: By Container
            }
        }

        ZENO_API std::shared_ptr<IObject> get_iterate_object() override {
            return m_iterate_object;
        }

        std::shared_ptr<IObject> apply(std::shared_ptr<IObject> iterate_object) {
            //construct the `result` object
            m_iterate_object = iterate_object;
            if (m_iterate_method == "By Count" || m_iterate_method == "By Container") {
                if (m_collect_method == "Feedback to Begin") {
                    std::shared_ptr<IObject> iobj = m_iterate_object->clone();
                    if (std::shared_ptr< zeno::ListObject> spList = std::dynamic_pointer_cast<zeno::ListObject>(iobj)) {
                        update_list_root_key(spList, get_uuid_path());
                    } else if (std::shared_ptr< zeno::DictObject> spDict = std::dynamic_pointer_cast<zeno::DictObject>(iobj)) {
                        update_dict_root_key(spDict, get_uuid_path());
                    } else {
                        iobj->update_key(get_uuid_path());
                    }
                    return iobj;
                } else if (m_collect_method == "Gather Each Iteration") {
                    std::shared_ptr<ForEachBegin> foreach_begin = get_foreach_begin();
                    int current_iter = foreach_begin->get_current_iteration();

                    std::shared_ptr<IObject> iobj = m_iterate_object->clone();
                    if (auto spList = std::dynamic_pointer_cast<zeno::ListObject>(iobj)) {
                        update_list_root_key(spList, get_uuid_path() + "\\iter" + std::to_string(current_iter));
                        for (int i = 0; i < spList->size(); i++) {
                            std::shared_ptr<IObject> cloneobj = spList->get(i);
                            m_collect_objs->append(cloneobj);
                            m_collect_objs->m_new_added.insert(cloneobj->key());
                        }
                    } else if (auto spDict = std::dynamic_pointer_cast<zeno::DictObject>(iobj)) {
                        update_dict_root_key(spDict, get_uuid_path() + "\\iter" + std::to_string(current_iter));
                        for (auto& [key, obj] : spDict->get()) {
                            m_collect_objs->append(obj);
                            m_collect_objs->m_new_added.insert(obj->key());
                        }
                    } else { 
                        std::string currIterKey = get_uuid_path() + '\\' + iterate_object->key() + ":iter:" + std::to_string(current_iter);
                        iobj->update_key(currIterKey);
                        m_collect_objs->append(iobj);
                        m_collect_objs->m_new_added.insert(currIterKey);
                    }
                    return m_collect_objs;
                }
                else {
                    assert(false);
                    return nullptr;
                }
            }
            else {
                throw makeError<UnimplError>("only support By Count or By Container at ForeachEnd");
            }
            return nullptr;
        }

        void adjustCollectObjInfo() {
            std::function<void(std::shared_ptr<IObject>)> flattenList = [&flattenList, this](std::shared_ptr<IObject> obj) {
                if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(obj)) {
                    for (int i = 0; i < _spList->size(); ++i) {
                        flattenList(_spList->get(i));
                    }
                } else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(obj)) {
                    for (auto& [key, obj] : _spDict->get()) {
                        flattenList(obj);
                    }
                } else {
                    m_collect_objs->m_new_removed.insert(obj->key());
                }
            };
            for (auto it = m_last_collect_objs.begin(); it != m_last_collect_objs.end();) {
                if (m_collect_objs->m_new_added.find((*it)->key()) == m_collect_objs->m_new_added.end()) {
                    flattenList(*it);
                    it = m_last_collect_objs.erase(it);
                } else {
                    m_collect_objs->m_modify.insert((*it)->key());
                    m_collect_objs->m_new_added.erase((*it)->key());
                    ++it;
                }
            }
            m_last_collect_objs.clear();
            m_last_collect_objs = m_collect_objs->get();
        }

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "ForEachBegin Path")
        std::string m_foreach_begin_path;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Iterations", 
                Constrain = "enabled = parameter('Iterate Method').value == 'By Count';")
        int m_iterations = 10;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Increment",
                Constrain = "enabled = parameter('Iterate Method').value == 'By Count';")
        int m_increment = 1;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Start Value",
                Constrain = "visible = parameter('Iterate Method').value == 'By Count';")
        int m_start_value = 0;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Stop Condition")
        int m_stop_condition = 1;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Iterate Method", Control = zeno::Combobox, ComboBoxItems = ("By Count", "By Container", "By Geometry Point", "By Geometry Face"))
        std::string m_iterate_method = "By Count";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Collect Method", Control = zeno::Combobox, ComboBoxItems = ("Feedback to Begin", "Gather Each Iteration"))
        std::string m_collect_method = "Feedback to Begin";

        std::shared_ptr<IObject> m_iterate_object;
        std::shared_ptr<ListObject> m_collect_objs;     //TODO: 如果foreach的对象是Dict，但这里收集的结果将会以list返回出去，以后再支持Dict的收集
        std::vector<std::shared_ptr<IObject>> m_last_collect_objs;
    };
}
