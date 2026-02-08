#pragma once

#include <zeno/utils/api.h>
#include <iobject2.h>
#include <inodedata.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <variant>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <zeno/types/ListObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/core/data.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/utils/uuid.h>
#include <zeno/utils/safe_at.h>
#include <zeno/core/CoreParam.h>
#include <functional>
#include <reflect/registry.hpp>
#include <zcommon.h>
#include <inodedata.h>
#include <inodeimpl.h>


namespace zeno {

    struct RefLinkInfo
    {
        EdgeInfo reflink;
        bool bOutParamIsOutput;     //reflink的source可能是一个output也可能是一个input，true表示reflink引用了一个output参数
    };

    class ZNode;

    class ZNodeParams : public INodeData
    {
    public:
        ZNodeParams(ZNode* pNode, const CustomUI& cui);
        ZNodeParams() = delete;
        ZNodeParams(const ZNodeParams& rhs) = delete;
        ZNode* getNode() const { return m_pNode; }

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
        ShaderData get_input_shader(const std::string& param, zeno::reflect::Any defl = zeno::reflect::Any());
        ParamType get_anyparam_type(bool bInput, const std::string& name);
        std::string get_viewobject_output_param() const;
        bool set_output(std::string const& param, zany2&& obj);
        bool update_param_impl(const std::string& param, const zeno::reflect::Any& new_value);
        bool set_primitive_output(std::string const& id, const zeno::reflect::Any& val);
        bool set_primitive_input(std::string const& id, const zeno::reflect::Any& val);

        bool update_param(const std::string& name, const zeno::reflect::Any& new_value);
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
        bool removeRefLinkDesParamIndx(bool bInput, bool bPrimitivParam, const std::string& paramName, bool bUiNeedRemoveReflink = false);
        bool removeRefLink(const EdgeInfo& edge, bool outParamIsOutput);

        virtual params_change_info update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit = false);
        //由param这个参数值的变化触发节点params重置
        virtual void trigger_update_params(const std::string& param, bool changed, params_change_info changes);
        bool has_frame_relative_params() const;

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
        std::vector<std::pair<std::string, bool>> getWildCardParams(const std::string& name, bool bPrim);
        void getParamTypeAndSocketType(const std::string& param_name, bool bPrim, bool bInput, ParamType& paramType, SocketType& socketType, bool& bWildcard);
        void constructReference(const std::string& param_name);
        void checkParamsConstrain();

        //referLink相关
        std::vector<RefLinkInfo> getReflinkInfo(bool bOnlySearchByDestNode = true);
        void removeNodeUpdateRefLink(const zeno::EdgeInfo& link, bool bAddRef, bool bOutParamIsOutput);//前端删除节点时undo/redo相关param的reflink

        CALLBACK_REGIST(update_visable_enable, void, zeno::NodeImpl*, std::set<std::string>, std::set<std::string>)
        CALLBACK_REGIST(addRefLink, void, EdgeInfo, bool outParamIsOutput)
        CALLBACK_REGIST(removeRefLink, void, EdgeInfo, bool outParamIsOutput)

        template <class T>
        bool has_input(std::string const& id) const {
            if (!has_input(id)) return false;
            auto obj = clone_input(id);
            return !!dynamic_cast<T*>(obj.get());
        }

        template <class T>
        bool has_input2(std::string const& id) const {
            if (!has_input(id)) return false;
            return objectIsLiterial<T>(clone_input(id).get());
        }

        template <class T>
        std::unique_ptr<T> get_input(std::string const& id) const {
            auto obj = clone_input(id);
            return safe_uniqueptr_cast<T>(std::move(obj));
        }

    public://INodeData
        IObject2* get_input_object(const char* param) override;
        IObject2* clone_input_object(const char* param) override;
        IPrimitiveObject* get_input_PrimitiveObject(const char* param) override;
        IGeometryObject* get_input_Geometry(const char* param) override;
        IGeometryObject* clone_input_Geometry(const char* param) override;
        IListObject* get_input_ListObject(const char* param) override;
        int get_input2_int(const char* param) override;
        float get_input2_float(const char* param) override;
        int get_input2_string(const char* param, char* ret, size_t cap) override;
        bool get_input2_bool(const char* param) override;
        bool has_input(const char* param) override;
        bool has_link_input(const char* param) override;
        Vec2i get_input2_vec2i(const char* param) override;
        Vec2f get_input2_vec2f(const char* param) override;
        Vec3i get_input2_vec3i(const char* param) override;
        Vec3f get_input2_vec3f(const char* param) override;
        Vec4i get_input2_vec4i(const char* param) override;
        Vec4f get_input2_vec4f(const char* param) override;
        bool set_output_object(const char* param, IObject2* detached_obj) override;
        bool set_output_int(const char* param, int val) override;
        bool set_output_float(const char* param, float val) override;
        bool set_output_bool(const char* param, bool val) override;
        bool set_output_string(const char* param, const char* val) override;
        bool set_output_vec2f(const char* param, Vec2f val) override;
        bool set_output_vec2i(const char* param, Vec2i val) override;
        bool set_output_vec3f(const char* param, Vec3f val) override;
        bool set_output_vec3i(const char* param, Vec3i val) override;
        bool set_output_vec4f(const char* param, Vec4f val) override;
        bool set_output_vec4i(const char* param, Vec4i val) override;
        int GetFrameId() const override;
        void report_error(const char* error_info) override;

    private:
        zany2 clone_input(std::string const& id) const;
        Graph* getGraph() const;
        bool addRefLink(const EdgeInfo& edge, bool outParamIsOutput);
        void initReferLinks(PrimitiveParam* target_param);
        std::set<RefSourceInfo> resolveReferSource(const zeno::reflect::Any& param_defl);
        GlobalState* getGlobalState() const;
        bool has_input(std::string const& id) const;
        bool has_link_input(std::string const& id) const;

        std::map<std::string, ObjectParam> m_inputObjs;
        std::map<std::string, PrimitiveParam> m_inputPrims;
        std::map<std::string, PrimitiveParam> m_outputPrims;
        std::map<std::string, ObjectParam> m_outputObjs;
        ZNode* m_pNode{};
    };
}