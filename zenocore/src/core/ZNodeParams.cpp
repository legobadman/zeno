#include <zeno/core/ZNodeParams.h>
#include <utility>

namespace zeno {

    ZNodeParams::ZNodeParams(const CustomUI& cui)
    {
    }

    // ------------------------- basic param queries -------------------------

    CommonParam ZNodeParams::get_input_param(std::string const& name, bool* bExist)
    {
        if (bExist) *bExist = false;
        return CommonParam{};
    }

    CommonParam ZNodeParams::get_output_param(std::string const& name, bool* bExist)
    {
        if (bExist) *bExist = false;
        return CommonParam{};
    }

    ObjectParams ZNodeParams::get_input_object_params() const
    {
        return ObjectParams{};
    }

    ObjectParams ZNodeParams::get_output_object_params() const
    {
        return ObjectParams{};
    }

    PrimitiveParams ZNodeParams::get_input_primitive_params() const
    {
        return PrimitiveParams{};
    }

    PrimitiveParams ZNodeParams::get_output_primitive_params() const
    {
        return PrimitiveParams{};
    }

    ParamPrimitive ZNodeParams::get_input_prim_param(std::string const& name, bool* pExist) const
    {
        if (pExist) *pExist = false;
        return ParamPrimitive{};
    }

    ParamObject ZNodeParams::get_input_obj_param(std::string const& name, bool* pExist) const
    {
        if (pExist) *pExist = false;
        return ParamObject{};
    }

    ParamPrimitive ZNodeParams::get_output_prim_param(std::string const& name, bool* pExist) const
    {
        if (pExist) *pExist = false;
        return ParamPrimitive{};
    }

    ParamObject ZNodeParams::get_output_obj_param(std::string const& name, bool* pExist) const
    {
        if (pExist) *pExist = false;
        return ParamObject{};
    }

    zeno::reflect::Any ZNodeParams::get_defl_value(std::string const& name)
    {
        return zeno::reflect::Any{};
    }

    zeno::reflect::Any ZNodeParams::get_param_result(std::string const& name)
    {
        return zeno::reflect::Any{};
    }

    ShaderData ZNodeParams::get_input_shader(const std::string& param, zeno::reflect::Any defl)
    {
        (void)param;
        (void)defl;
        return ShaderData{};
    }

    ParamType ZNodeParams::get_anyparam_type(bool bInput, const std::string& name)
    {
        (void)bInput;
        (void)name;
        return ParamType{};
    }

    std::string ZNodeParams::get_viewobject_output_param() const
    {
        return {};
    }

    bool ZNodeParams::set_output(std::string const& param, zany2&& obj)
    {
        (void)param;
        (void)obj;
        return false;
    }

    bool ZNodeParams::update_param_impl(const std::string& param, const zeno::reflect::Any& new_value)
    {
        (void)param;
        (void)new_value;
        return false;
    }

    bool ZNodeParams::set_primitive_output(std::string const& id, const zeno::reflect::Any& val)
    {
        (void)id;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_primitive_input(std::string const& id, const zeno::reflect::Any& val)
    {
        (void)id;
        (void)val;
        return false;
    }

    // ------------------------- update APIs -------------------------

    bool ZNodeParams::update_param(const std::string& name, const zeno::reflect::Any& new_value)
    {
        (void)name;
        (void)new_value;
        return false;
    }

    bool ZNodeParams::update_param_socket_type(const std::string& name, SocketType type)
    {
        (void)name;
        (void)type;
        return false;
    }

    bool ZNodeParams::update_param_wildcard(const std::string& name, bool isWildcard)
    {
        (void)name;
        (void)isWildcard;
        return false;
    }

    bool ZNodeParams::update_param_type(const std::string& name, bool bPrim, bool bInput, ParamType type)
    {
        (void)name;
        (void)bPrim;
        (void)bInput;
        (void)type;
        return false;
    }

    bool ZNodeParams::update_param_control(const std::string& name, ParamControl control)
    {
        (void)name;
        (void)control;
        return false;
    }

    bool ZNodeParams::update_param_control_prop(const std::string& name, zeno::reflect::Any props)
    {
        (void)name;
        (void)props;
        return false;
    }

    bool ZNodeParams::update_param_socket_visible(const std::string& name, bool bVisible, bool bInput)
    {
        (void)name;
        (void)bVisible;
        (void)bInput;
        return false;
    }

    bool ZNodeParams::update_param_visible(const std::string& name, bool bOn, bool bInput)
    {
        (void)name;
        (void)bOn;
        (void)bInput;
        return false;
    }

    bool ZNodeParams::update_param_enable(const std::string& name, bool bOn, bool bInput)
    {
        (void)name;
        (void)bOn;
        (void)bInput;
        return false;
    }

    void ZNodeParams::update_param_color(const std::string& name, std::string& clr)
    {
        (void)name;
        (void)clr;
    }

    void ZNodeParams::update_layout(params_change_info& changes)
    {
        (void)changes;
    }

    params_change_info ZNodeParams::update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit)
    {
        (void)params;
        (void)bSubnetInit;
        return {};
    }

    void ZNodeParams::trigger_update_params(const std::string& param, bool changed, params_change_info changes)
    {
        (void)param;
        (void)changed;
        (void)changes;
    }

    bool ZNodeParams::has_frame_relative_params() const
    {
        return false;
    }

    // ------------------------- add params -------------------------

    bool ZNodeParams::add_input_prim_param(ParamPrimitive param)
    {
        (void)param;
        return false;
    }

    bool ZNodeParams::add_input_obj_param(ParamObject param)
    {
        (void)param;
        return false;
    }

    bool ZNodeParams::add_output_prim_param(ParamPrimitive param)
    {
        (void)param;
        return false;
    }

    bool ZNodeParams::add_output_obj_param(ParamObject param)
    {
        (void)param;
        return false;
    }

    // ------------------------- links -------------------------

    void ZNodeParams::init_object_link(bool bInput, const std::string& paramname,
        std::shared_ptr<ObjectLink> spLink,
        const std::string& targetParam)
    {
        (void)bInput;
        (void)paramname;
        (void)spLink;
        (void)targetParam;
    }

    void ZNodeParams::init_primitive_link(bool bInput, const std::string& paramname,
        std::shared_ptr<PrimitiveLink> spLink,
        const std::string& targetParam)
    {
        (void)bInput;
        (void)paramname;
        (void)spLink;
        (void)targetParam;
    }

    bool ZNodeParams::isPrimitiveType(bool bInput, const std::string& param_name, bool& bExist)
    {
        (void)bInput;
        (void)param_name;
        bExist = false;
        return false;
    }

    std::vector<EdgeInfo> ZNodeParams::getLinks() const
    {
        return {};
    }

    std::vector<EdgeInfo> ZNodeParams::getLinksByParam(bool bInput, const std::string& param_name) const
    {
        (void)bInput;
        (void)param_name;
        return {};
    }

    bool ZNodeParams::updateLinkKey(bool bInput, const zeno::EdgeInfo& edge,
        const std::string& oldkey, const std::string& newkey)
    {
        (void)bInput;
        (void)edge;
        (void)oldkey;
        (void)newkey;
        return false;
    }

    bool ZNodeParams::moveUpLinkKey(bool bInput, const std::string& param_name, const std::string& key)
    {
        (void)bInput;
        (void)param_name;
        (void)key;
        return false;
    }

    bool ZNodeParams::removeLink(bool bInput, const EdgeInfo& edge)
    {
        (void)bInput;
        (void)edge;
        return false;
    }

    std::vector<std::pair<std::string, bool>> ZNodeParams::getWildCardParams(const std::string& name, bool bPrim)
    {
        (void)name;
        (void)bPrim;
        return {};
    }

    void ZNodeParams::getParamTypeAndSocketType(const std::string& param_name,
        bool bPrim, bool bInput,
        ParamType& paramType,
        SocketType& socketType,
        bool& bWildcard)
    {
        (void)param_name;
        (void)bPrim;
        (void)bInput;
        paramType = ParamType{};
        socketType = SocketType{};
        bWildcard = false;
    }

    void ZNodeParams::constructReference(const std::string& param_name)
    {
        (void)param_name;
    }

    void ZNodeParams::checkParamsConstrain()
    {
    }

    // ------------------------- reflink -------------------------

    std::vector<RefLinkInfo> ZNodeParams::getReflinkInfo(bool bOnlySearchByDestNode)
    {
        (void)bOnlySearchByDestNode;
        return {};
    }

    void ZNodeParams::removeNodeUpdateRefLink(const zeno::EdgeInfo& link, bool bAddRef, bool bOutParamIsOutput)
    {
        (void)link;
        (void)bAddRef;
        (void)bOutParamIsOutput;
    }

    // ------------------------- INodeData -------------------------

    IObject2* ZNodeParams::get_input_object(const char* param)
    {
        (void)param;
        return nullptr;
    }

    IObject2* ZNodeParams::clone_input_object(const char* param)
    {
        (void)param;
        return nullptr;
    }

    IPrimitiveObject* ZNodeParams::get_input_PrimitiveObject(const char* param)
    {
        (void)param;
        return nullptr;
    }

    IGeometryObject* ZNodeParams::get_input_Geometry(const char* param)
    {
        (void)param;
        return nullptr;
    }

    IGeometryObject* ZNodeParams::clone_input_Geometry(const char* param)
    {
        (void)param;
        return nullptr;
    }

    IListObject* ZNodeParams::get_input_ListObject(const char* param)
    {
        (void)param;
        return nullptr;
    }

    int ZNodeParams::get_input2_int(const char* param)
    {
        (void)param;
        return 0;
    }

    float ZNodeParams::get_input2_float(const char* param)
    {
        (void)param;
        return 0.f;
    }

    int ZNodeParams::get_input2_string(const char* param, char* ret, size_t cap)
    {
        (void)param;
        (void)ret;
        (void)cap;
        return 0;
    }

    bool ZNodeParams::get_input2_bool(const char* param)
    {
        (void)param;
        return false;
    }

    bool ZNodeParams::has_input(const char* param)
    {
        (void)param;
        return false;
    }

    bool ZNodeParams::has_link_input(const char* param)
    {
        (void)param;
        return false;
    }

    Vec2i ZNodeParams::get_input2_vec2i(const char* param)
    {
        (void)param;
        return {};
    }

    Vec2f ZNodeParams::get_input2_vec2f(const char* param)
    {
        (void)param;
        return {};
    }

    Vec3i ZNodeParams::get_input2_vec3i(const char* param)
    {
        (void)param;
        return {};
    }

    Vec3f ZNodeParams::get_input2_vec3f(const char* param)
    {
        (void)param;
        return {};
    }

    Vec4i ZNodeParams::get_input2_vec4i(const char* param)
    {
        (void)param;
        return {};
    }

    Vec4f ZNodeParams::get_input2_vec4f(const char* param)
    {
        (void)param;
        return {};
    }

    bool ZNodeParams::set_output_object(const char* param, IObject2* detached_obj)
    {
        (void)param;
        (void)detached_obj;
        return false;
    }

    bool ZNodeParams::set_output_int(const char* param, int val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_float(const char* param, float val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_bool(const char* param, bool val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_string(const char* param, const char* val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec2f(const char* param, Vec2f val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec2i(const char* param, Vec2i val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec3f(const char* param, Vec3f val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec3i(const char* param, Vec3i val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec4f(const char* param, Vec4f val)
    {
        (void)param;
        (void)val;
        return false;
    }

    bool ZNodeParams::set_output_vec4i(const char* param, Vec4i val)
    {
        (void)param;
        (void)val;
        return false;
    }

    int ZNodeParams::GetFrameId() const
    {
        return 0;
    }

    void ZNodeParams::report_error(const char* error_info) {

    }

} // namespace zeno
