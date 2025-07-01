#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/IObject.h>
#include <zeno/core/INode.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <variant>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <zeno/types/DictObject.h>
#include <zeno/types/ListObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/core/data.h>
#include <zeno/utils/uuid.h>
#include <zeno/utils/safe_at.h>
#include <zeno/core/CoreParam.h>
#include <functional>
#include <reflect/registry.hpp>


namespace zeno
{
    struct Graph;
    struct INodeClass;
    struct Scene;
    struct Session;
    struct GlobalState;
    struct TempNodeCaller;
    struct CoreParam;
    struct ObjectParam;
    struct PrimitiveParam;
    struct ObjectLink;
    struct PrimitiveLink;
    struct SubnetNode;
    struct CalcContext;

    class ZENO_API NodeImpl
    {
    public:
        INodeClass* nodeClass = nullptr;

        NodeImpl(INode* pNode);
        virtual ~NodeImpl();

        void doApply(CalcContext* pContext);
        void doApply_Parameter(std::string const& name, CalcContext* pContext); //引入数值输入参数，并不计算整个节点
        void doOnlyApply();
        void mark_dirty(bool bOn, DirtyReason reason = zeno::Dirty_All, bool bWholeSubnet = true, bool bRecursively = true);
        virtual void dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively);
        virtual void clearCalcResults();

        //BEGIN new api
        void init(const NodeData& dat);
        std::string get_nodecls() const;
        std::string get_ident() const;
        std::string get_show_name() const;
        std::string get_show_icon() const;
        virtual CustomUI get_customui() const;     //由节点默认定义导出的customUi
        virtual CustomUI export_customui() const;          //由此刻实际存在的节点输入输出导出的customUi
        ObjPath get_path() const;
        ObjPath get_graph_path() const;
        ObjPath get_uuid_path() const;
        std::string get_uuid() const;
        Graph* getGraph() const;
        INode* coreNode() const;
        void initUuid(Graph* pGraph, const std::string nodecls);

        virtual NodeType nodeType() const;
        virtual bool is_locked() const;
        virtual void set_locked(bool);

        void set_view(bool bOn);
        CALLBACK_REGIST(set_view, void, bool)
        bool is_view() const;

        void set_mute(bool bOn);
        CALLBACK_REGIST(set_mute, void, bool)
        bool is_mute() const;

        bool is_dirty() const { return m_dirty; }
        NodeRunStatus get_run_status() const { return m_status; }

        CommonParam get_input_param(std::string const& name, bool* bExist = nullptr);
        CommonParam get_output_param(std::string const& name, bool* bExist = nullptr);

        ObjectParams get_input_object_params() const;
        ObjectParams get_output_object_params() const;
        PrimitiveParams get_input_primitive_params() const;
        PrimitiveParams get_output_primitive_params() const;
        ParamPrimitive get_input_prim_param(std::string const& name, bool* pExist = nullptr) const;
        ParamObject get_input_obj_param(std::string const& name, bool* pExist = nullptr) const;
        ParamPrimitive get_output_prim_param(std::string const& name, bool* pExist = nullptr) const;
        ParamObject get_output_obj_param(std::string const& name, bool* pExist = nullptr) const;
        zeno::reflect::Any get_defl_value(std::string const& name);
        zeno::reflect::Any get_param_result(std::string const& name);
        zany get_input_obj(std::string const& name) const;
        ShaderData get_input_shader(const std::string& param, zeno::reflect::Any defl = zeno::reflect::Any());
        ParamType get_anyparam_type(bool bInput, const std::string& name);

        /*container_info是记录参数的ListObject的增删改情况，便于作部分加载*/
        container_elem_update_info get_input_container_info(const std::string& param);
        container_elem_update_info get_output_container_info(const std::string& param);
        void set_input_container_info(const std::string& param, const container_elem_update_info& info);
        void set_output_container_info(const std::string& param, const container_elem_update_info& info);
        void clear_container_info();

        std::string get_viewobject_output_param() const;
        virtual NodeData exportInfo() const;
        void set_result(bool bInput, const std::string& name, zany spObj);
        bool set_output(std::string const& param, zany obj);

        bool update_param_impl(const std::string& param, zeno::reflect::Any new_value, zeno::reflect::Any& oldVal);
        bool set_primitive_output(std::string const& id, const zeno::reflect::Any& val);
        bool set_primitive_input(std::string const& id, const zeno::reflect::Any& val);

        template <class T>
        const T get_input_prim(std::string const& name) const {
            auto& prim = safe_at(m_inputPrims, name, "input primtive");
            return zeno::reflect::any_cast<T>(prim.defl);
        }

        bool update_param(const std::string& name, zeno::reflect::Any new_value);
        CALLBACK_REGIST(update_param, void, const std::string&, zeno::reflect::Any, zeno::reflect::Any)

        bool update_param_socket_type(const std::string& name, SocketType type);
        CALLBACK_REGIST(update_param_socket_type, void, const std::string&, SocketType)

        bool update_param_wildcard(const std::string& name, bool isWildcard);
        CALLBACK_REGIST(update_param_wildcard, void, const std::string&, bool)

        bool update_param_type(const std::string& name, bool bPrim, bool bInput, ParamType type);
        CALLBACK_REGIST(update_param_type, void, const std::string&, ParamType, bool)

        bool update_param_control(const std::string& name, ParamControl control);
        CALLBACK_REGIST(update_param_control, void, const std::string&, ParamControl)

        bool update_param_control_prop(const std::string& name, zeno::reflect::Any props);
        CALLBACK_REGIST(update_param_control_prop, void, const std::string&, zeno::reflect::Any)

        bool update_param_socket_visible(const std::string& name, bool bVisible, bool bInput = true);
        CALLBACK_REGIST(update_param_socket_visible, void, const std::string&, bool, bool)

        bool update_param_visible(const std::string& name, bool bOn, bool bInput = true);
        bool update_param_enable(const std::string& name, bool bOn, bool bInput = true);

        void update_param_color(const std::string& name, std::string& clr);
        CALLBACK_REGIST(update_param_color, void, const std::string&, std::string&)

        void update_layout(params_change_info& changes);
        CALLBACK_REGIST(update_layout, void, params_change_info& changes)

        virtual bool is_loaded() const;
        void update_load_info(bool bDisable);
        CALLBACK_REGIST(update_load_info, void, bool)

        virtual params_change_info update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit = false);

        //由param这个参数值的变化触发节点params重置
        virtual void trigger_update_params(const std::string& param, bool changed, params_change_info changes);

        void set_name(const std::string& name);
        std::string get_name() const;

        void set_pos(std::pair<float, float> pos);
        CALLBACK_REGIST(set_pos, void, std::pair<float, float>)
        std::pair<float, float> get_pos() const;

        bool in_asset_file() const;
        void func1() {}
        void initTypeBase(zeno::reflect::TypeBase* pTypeBase);
        void func2() {}
        bool isInDopnetwork();
        bool has_frame_relative_params() const;

        //foreach特供
        virtual bool is_continue_to_run();
        virtual void increment();
        virtual void reset_forloop_settings();
        virtual std::shared_ptr<IObject> get_iterate_object();

        void onInterrupted();
        void mark_previous_ref_dirty();
        void update_out_objs_key();

        //END new api
        bool add_input_prim_param(ParamPrimitive param);
        bool add_input_obj_param(ParamObject param);
        bool add_output_prim_param(ParamPrimitive param);
        bool add_output_obj_param(ParamObject param);
        void init_object_link(bool bInput, const std::string& paramname, std::shared_ptr<ObjectLink> spLink, const std::string& targetParam);
        void init_primitive_link(bool bInput, const std::string& paramname, std::shared_ptr<PrimitiveLink> spLink, const std::string& targetParam);
        bool isPrimitiveType(bool bInput, const std::string& param_name, bool& bExist);
        std::vector<EdgeInfo> getLinks() const;
        std::vector<EdgeInfo> getLinksByParam(bool bInput, const std::string& param_name) const;
        bool updateLinkKey(bool bInput, const zeno::EdgeInfo& edge, const std::string& oldkey, const std::string& newkey);
        bool moveUpLinkKey(bool bInput, const std::string& param_name, const std::string& key);
        bool removeLink(bool bInput, const EdgeInfo& edge);
        void mark_dirty_objs();
        std::vector<std::pair<std::string, bool>> getWildCardParams(const std::string& name, bool bPrim);
        void getParamTypeAndSocketType(const std::string& param_name, bool bPrim, bool bInput, ParamType& paramType, SocketType& socketType, bool& bWildcard);
        void constructReference(const std::string& param_name);
        void onNodeNameUpdated(const std::string& oldname, const std::string& newname);
        void on_node_about_to_remove();
        void on_link_added_removed(bool bInput, const std::string& paramname, bool bAdded); //参数名包括对象输入和数值输入，不可重名
        void checkParamsConstrain();

        CALLBACK_REGIST(update_visable_enable, void, zeno::NodeImpl*, std::set<std::string>, std::set<std::string>)

    public:
        //为名为ds的输入参数，求得这个参数在依赖边的求值下的值，或者没有依赖边下的默认值。
        bool requireInput(std::string const& ds, CalcContext* pContext);

        Graph* getThisGraph() const;
        Session* getThisSession() const;
        GlobalState* getGlobalState() const;

        bool has_input(std::string const& id) const;
        zany get_input(std::string const& id) const;
        zany get_output_obj(std::string const& sock_name);
        std::vector<zany> get_output_objs();
        virtual zany get_default_output_object();
        virtual container_elem_update_info get_default_output_container_info();

        template <class T>
        std::shared_ptr<T> get_input(std::string const& id) const {
            auto obj = get_input(id);
            return safe_dynamic_cast<T>(std::move(obj), "input socket `" + id + "` of node `" + m_name + "`");
        }

        template <class T>
        bool has_input(std::string const& id) const {
            if (!has_input(id)) return false;
            auto obj = get_input(id);
            return !!dynamic_cast<T*>(obj.get());
        }

        template <class T>
        bool has_input2(std::string const& id) const {
            if (!has_input(id)) return false;
            return objectIsLiterial<T>(get_input(id));
        }

        template <class T>
        auto get_input2(std::string const& id) const {
            return objectToLiterial<T>(get_input(id), "input socket `" + id + "` of node `" + m_name + "`");
        }

        template <class T>
        void set_output2(std::string const& id, T&& value) {
            set_output(id, objectFromLiterial(std::forward<T>(value)));
        }

        template <class T>
        [[deprecated("use get_input2<T>(id)")]]
        T get_param(std::string const& id) const {
            return get_input2<T>(id);
        }

        template <class T = IObject>
        std::shared_ptr<T> get_input(std::string const& id, std::shared_ptr<T> const& defl) const {
            return has_input(id) ? get_input<T>(id) : defl;
        }

        template <class T>
        T get_input2(std::string const& id, T const& defl) const {
            return has_input(id) ? get_input2<T>(id) : defl;
        }

        TempNodeCaller temp_node(std::string const& id);

    protected:
        virtual void complete();
        virtual void apply();
        void reflectNode_apply();
        virtual void initParams(const NodeData& dat);

        std::map<std::string, ObjectParam> m_inputObjs;
        std::map<std::string, PrimitiveParam> m_inputPrims;
        std::map<std::string, PrimitiveParam> m_outputPrims;
        std::map<std::string, ObjectParam> m_outputObjs;
        
    private:
        zeno::reflect::Any processPrimitive(PrimitiveParam* in_param);
        std::shared_ptr<DictObject> processDict(ObjectParam* in_param, CalcContext* pContext);
        std::shared_ptr<ListObject> processList(ObjectParam* in_param, CalcContext* pContext, bool& bDirty);
        bool receiveOutputObj(ObjectParam* in_param, NodeImpl* outNode, ObjectParam* out_param);
        void reportStatus(bool bDirty, NodeRunStatus status);
        float resolve(const std::string& formulaOrKFrame, const ParamType type);
        std::string resolve_string(const std::string& fmla, const std::string& defl);
        zfxvariant execute_fmla(const std::string& expression);
        template<class T, class E> T resolveVec(const zeno::reflect::Any& defl, const ParamType type);
        std::set<std::pair<std::string, std::string>> resolveReferSource(const zeno::reflect::Any& param_defl);
        void initReferLinks(PrimitiveParam* target_param);

        //preApply是先解决所有输入参数（上游）的求值问题
        void preApply(CalcContext* pContext);
        void preApply_Primitives(CalcContext* pContext);
        void commit_to_render(UpdateReason reason);
        void bypass();
        CustomUI _deflCustomUI() const;

        //for timeshift node
        void preApplyTimeshift(CalcContext* pContext);
        //foreach特供
        void foreachend_apply(CalcContext* pContext);
        void switchif_apply(CalcContext* pContext);
        void switchbetween_apply(CalcContext* pContext);
        void init_output_container_updateinfo();

        std::string m_name;
        std::string m_nodecls;
        std::string m_uuid;
        std::pair<float, float> m_pos;

        std::string m_uuidPath;
        NodeRunStatus m_status = Node_DirtyReadyToRun;
        Graph* m_pGraph;
        std::unique_ptr<INode> m_pNode;
        bool m_bView = false;
        bool m_mute = false;
        bool m_dirty = true;

        zeno::reflect::TypeBase* m_pTypebase = nullptr;

        friend class SubnetNode;
    };

}