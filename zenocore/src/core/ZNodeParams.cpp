#include <zeno/core/ZNodeParams.h>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <zeno/utils/helper.h>
#include <zeno/core/ZNode.h>
#include <zeno/core/ZNodeStatus.h>
#include <zeno/core/ZNodeExecutor.h>
#include <zeno/core/Graph.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/extra/GraphException.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/extra/GlobalState.h>
#include "zeno_types/reflect/reflection.generated.hpp"


using namespace zeno::reflect;
using namespace zeno::types;

namespace zeno {

    ZNodeParams::ZNodeParams(ZNode* pNode, const CustomUI& cui)
        : m_pNode(pNode)
    {
        for (const ParamObject& paramObj : cui.inputObjs) {
            auto iter = m_inputObjs.find(paramObj.name);
            if (iter == m_inputObjs.end()) {
                add_input_obj_param(paramObj);
                continue;
            }
            auto& sparam = iter->second;
            sparam.socketType = paramObj.socketType;
        }

        for (auto tab : cui.inputPrims) {
            for (auto group : tab.groups) {
                for (auto param : group.params) {
                    auto iter = m_inputPrims.find(param.name);
                    if (iter == m_inputPrims.end()) {
                        add_input_prim_param(param);
                        continue;
                    }
                    auto& sparam = iter->second;

                    convertToEditVar(param.defl, param.type);
                    sparam.defl = param.defl;
                    convertToEditVar(sparam.defl, param.type);

                    if (!sparam.defl.has_value()) {
                        sparam.defl = initAnyDeflValue(param.type);
                    }

                    sparam.bSocketVisible = param.bSocketVisible;
                }
            }
        }

        for (const ParamPrimitive& param : cui.outputPrims) {
            auto iter = m_outputPrims.find(param.name);
            if (iter == m_outputPrims.end()) {
                add_output_prim_param(param);
                continue;
            }
            auto& sparam = iter->second;
            sparam.bSocketVisible = param.bSocketVisible;
        }

        for (const ParamObject& paramObj : cui.outputObjs) {
            add_output_obj_param(paramObj);
        }
    }

    std::map<std::string, ObjectParam>& ZNodeParams::get_input_object_params2() { return m_inputObjs; }
    std::map<std::string, ObjectParam>& ZNodeParams::get_output_object_params2() { return m_outputObjs; }
    std::map<std::string, PrimitiveParam>& ZNodeParams::get_input_prim_params2() { return m_inputPrims; }
    std::map<std::string, PrimitiveParam>& ZNodeParams::get_output_prim_params2() { return m_outputPrims; }

    // ------------------------- basic param queries -------------------------

    CommonParam ZNodeParams::get_input_param(std::string const& name, bool* bExist)
    {
        auto primparam = get_input_prim_param(name, bExist);
        if (bExist && *bExist)
            return primparam;
        auto objparam = get_input_obj_param(name, bExist);
        if (bExist && *bExist)
            return objparam;
        if (bExist)
            *bExist = false;
        return CommonParam();
    }

    CommonParam ZNodeParams::get_output_param(std::string const& name, bool* bExist)
    {
        auto primparam = get_output_prim_param(name, bExist);
        if (bExist && *bExist)
            return primparam;
        auto objparam = get_output_obj_param(name, bExist);
        if (bExist && *bExist)
            return objparam;
        if (bExist)
            *bExist = false;
        return CommonParam();
    }

    ObjectParams ZNodeParams::get_input_object_params() const
    {
        ObjectParams params;
        for (auto& [name, spObjParam] : m_inputObjs)
        {
            ParamObject obj;
            for (auto linkInfo : spObjParam.links) {
                obj.links.push_back(getEdgeInfo(linkInfo));
            }
            obj.name = name;
            obj.type = spObjParam.type;
            obj.bInput = true;
            obj.socketType = spObjParam.socketType;
            obj.wildCardGroup = spObjParam.wildCardGroup;
            //obj.prop = ?
            params.push_back(obj);
        }
        return params;
    }

    ObjectParams ZNodeParams::get_output_object_params() const
    {
        ObjectParams params;
        for (auto& [name, spObjParam] : m_outputObjs)
        {
            ParamObject obj;
            for (auto linkInfo : spObjParam.links) {
                obj.links.push_back(getEdgeInfo(linkInfo));
            }
            obj.name = name;
            obj.type = spObjParam.type;
            obj.bInput = false;
            obj.socketType = spObjParam.socketType;
            obj.wildCardGroup = spObjParam.wildCardGroup;
            //obj.prop = ?
            params.push_back(obj);
        }
        return params;
    }

    PrimitiveParams ZNodeParams::get_input_primitive_params() const
    {
        //TODO: deprecated node.
        PrimitiveParams params;
        for (auto& [name, spParamObj] : m_inputPrims) {
            ParamPrimitive param;
            param.bInput = true;
            param.name = name;
            param.type = spParamObj.type;
            param.control = spParamObj.control;
            param.ctrlProps = spParamObj.ctrlProps;
            param.defl = spParamObj.defl;
            param.bSocketVisible = spParamObj.bSocketVisible;
            for (auto spLink : spParamObj.links) {
                param.links.push_back(getEdgeInfo(spLink));
            }
            param.socketType = spParamObj.socketType;
            param.wildCardGroup = spParamObj.wildCardGroup;
            params.push_back(param);
        }
        return params;
    }

    PrimitiveParams ZNodeParams::get_output_primitive_params() const
    {
        PrimitiveParams params;
        for (auto& [name, spParamObj] : m_outputPrims) {
            ParamPrimitive param;
            param.bInput = false;
            param.name = name;
            param.type = spParamObj.type;
            param.control = NullControl;
            param.defl = spParamObj.defl;
            for (auto spLink : spParamObj.links) {
                param.links.push_back(getEdgeInfo(spLink));
            }
            param.socketType = spParamObj.socketType;
            param.wildCardGroup = spParamObj.wildCardGroup;
            params.push_back(param);
        }
        return params;
    }

    ParamPrimitive ZNodeParams::get_input_prim_param(std::string const& name, bool* pExist) const
    {
        ParamPrimitive param;
        auto iter = m_inputPrims.find(name);
        if (iter != m_inputPrims.end()) {
            auto& paramPrim = iter->second;
            param = paramPrim.exportParam();
            if (pExist)
                *pExist = true;
        }
        else {
            if (pExist)
                *pExist = false;
        }
        return param;
    }

    ParamObject ZNodeParams::get_input_obj_param(std::string const& name, bool* pExist) const
    {
        ParamObject param;
        auto iter = m_inputObjs.find(name);
        if (iter != m_inputObjs.end()) {
            auto& paramObj = iter->second;
            param = paramObj.exportParam();
            if (pExist)
                *pExist = true;
        }
        else {
            if (pExist)
                *pExist = false;
        }
        return param;
    }

    ParamPrimitive ZNodeParams::get_output_prim_param(std::string const& name, bool* pExist) const
    {
        ParamPrimitive param;
        auto iter = m_outputPrims.find(name);
        if (iter != m_outputPrims.end()) {
            auto& paramPrim = iter->second;
            param = paramPrim.exportParam();
            if (pExist)
                *pExist = true;
        }
        else {
            if (pExist)
                *pExist = false;
        }
        return param;
    }

    ParamObject ZNodeParams::get_output_obj_param(std::string const& name, bool* pExist) const
    {
        ParamObject param;
        auto iter = m_outputObjs.find(name);
        if (iter != m_outputObjs.end()) {
            auto& paramObj = iter->second;
            param = paramObj.exportParam();
            if (pExist)
                *pExist = true;
        }
        else {
            if (pExist)
                *pExist = false;
        }
        return param;
    }

    zeno::reflect::Any ZNodeParams::get_defl_value(std::string const& name)
    {
        //向量情况也挺麻烦的，因为可能存在公式
        ParamPrimitive param;
        auto iter = m_inputPrims.find(name);
        if (iter != m_inputPrims.end()) {
            zeno::reflect::Any defl = iter->second.defl;
            //不支持取公式，因为公式要引发计算，很麻烦
            convertToOriginalVar(defl, iter->second.type);
            return defl;
        }
        else {
            return zeno::reflect::Any();
        }
    }

    Graph* ZNodeParams::getGraph() const {
        return m_pNode->getNodeStatus()->getGraph();
    }

    zeno::reflect::Any ZNodeParams::get_param_result(std::string const& name)
    {
        const PrimitiveParam& param = safe_at(m_inputPrims, name, "prim param");
        return param.result;
    }

    ShaderData ZNodeParams::get_input_shader(const std::string& param, zeno::reflect::Any defl)
    {
        auto iter = m_inputPrims.find(param);
        if (iter == m_inputPrims.end()) {
            throw std::runtime_error("not valid param");
        }

        ShaderData shader;

        if (!iter->second.result.has_value()) {
            bool bSucceed = false;
            shader.data = AnyToNumeric(defl, bSucceed);
            if (!bSucceed) {
                throw std::runtime_error("cannot get NumericValue on defl value");
            }
            return shader;
        }

        const zeno::reflect::Any& result = iter->second.result;
        if (result.type().hash_code() == gParamType_Shader) {
            shader = zeno::reflect::any_cast<ShaderData>(result);
        }
        else {
            bool bSucceed = false;
            shader.data = AnyToNumeric(result, bSucceed);
            if (!bSucceed) {
                throw std::runtime_error("cannot get NumericValue");
            }
        }

        ParamPath uuidpath = m_pNode->getNodeStatus()->get_uuid_path() + "/" + param;
        shader.curr_param = uuidpath;
        return shader;
    }

    ParamType ZNodeParams::get_anyparam_type(bool bInput, const std::string& name)
    {
        if (bInput) {
            if (m_inputObjs.find(name) != m_inputObjs.end()) {
                return m_inputObjs[name].type;
            }
            else if (m_inputPrims.find(name) != m_inputPrims.end()) {
                return m_inputPrims[name].type;
            }
        }
        else {
            if (m_outputObjs.find(name) != m_outputObjs.end()) {
                return m_outputObjs[name].type;
            }
            else if (m_outputPrims.find(name) != m_outputPrims.end()) {
                return m_outputPrims[name].type;
            }
        }
        return Param_Null;
    }

    std::string ZNodeParams::get_viewobject_output_param() const
    {
        auto it = m_outputObjs.find("ViewObject");
        if (it != m_outputObjs.end()) return "ViewObject";
        it = m_outputObjs.find("Output");
        if (it != m_outputObjs.end()) return "Output";
        return {};
    }

    bool ZNodeParams::set_output(std::string const& param, zany2&& obj)
    {
        auto it = m_outputObjs.find(param);
        if (it == m_outputObjs.end()) return false;
        it->second.spObject = std::move(obj);
        return true;
    }

    bool ZNodeParams::update_param_impl(const std::string& param, const zeno::reflect::Any& new_value)
    {
        auto it = m_inputPrims.find(param);
        if (it == m_inputPrims.end()) return false;

        it->second.defl = new_value;
        convertToEditVar(it->second.defl, it->second.type);

        constructReference(param);
        checkParamsConstrain();
        return true;
    }

    bool ZNodeParams::set_primitive_output(std::string const& id, const zeno::reflect::Any& val)
    {
        auto it = m_outputPrims.find(id);
        if (it == m_outputPrims.end()) return false;
        it->second.result = val;
        return true;
    }

    bool ZNodeParams::set_primitive_input(std::string const& id, const zeno::reflect::Any& val)
    {
        auto it = m_inputPrims.find(id);
        if (it == m_inputPrims.end()) return false;
        it->second.result = val;
        return true;
    }

    // ------------------------- update APIs -------------------------

    bool ZNodeParams::update_param(const std::string& name, const zeno::reflect::Any& new_value)
    {
        bool ok = update_param_impl(name, new_value);
        return ok;
    }

    bool ZNodeParams::update_param_socket_type(const std::string& param, SocketType type)
    {
        CORE_API_BATCH
        auto name = m_pNode->getNodeStatus()->get_name();
        auto& spParam = safe_at(m_inputObjs, param, "miss input param `" + param + "` on node `" + name + "`");
        if (type != spParam.socketType)
        {
            spParam.socketType = type;
    #if 0
            if (type == Socket_Owning)
            {
                auto spGraph = graph;
                spGraph->removeLinks(m_name, true, param);
            }
    #endif
            m_pNode->getNodeExecutor()->mark_dirty(true);
            CALLBACK_NOTIFY(update_param_socket_type, param, type)
            return true;
        }
        return false;
    }

    bool ZNodeParams::update_param_wildcard(const std::string& name, bool isWildcard)
    {
        return false;
    }

    bool ZNodeParams::update_param_type(const std::string& param, bool bPrim, bool bInput, ParamType type)
    {
        CORE_API_BATCH
        if (bPrim)
        {
            auto& prims = bInput ? m_inputPrims : m_outputPrims;
            auto prim = prims.find(param);
            if (prim != prims.end())
            {
                auto& spParam = prim->second;
                if (type != spParam.type)
                {
                    spParam.type = type;
                    CALLBACK_NOTIFY(update_param_type, param, type, bInput)

                        //默认值也要更新
                        if (bInput) {
                            zeno::reflect::Any defl = initAnyDeflValue(type);
                            convertToEditVar(defl, type);
                            update_param(spParam.name, defl);
                        }
                    return true;
                }
            }
        }
        else
        {
            auto& objects = bInput ? m_inputObjs : m_outputObjs;
            auto object = objects.find(param);
            if (object != objects.end())
            {
                auto& spParam = object->second;
                if (type != spParam.type)
                {
                    spParam.type = type;
                    CALLBACK_NOTIFY(update_param_type, param, type, bInput)
                        return true;
                }
            }
        }
        return false;
    }

    bool ZNodeParams::update_param_control(const std::string& name, ParamControl control)
    {
        auto it = m_inputPrims.find(name);
        if (it == m_inputPrims.end()) return false;
        it->second.control = control;
        return true;
    }

    bool ZNodeParams::update_param_control_prop(const std::string& name, zeno::reflect::Any props)
    {
        auto it = m_inputPrims.find(name);
        if (it == m_inputPrims.end()) return false;
        it->second.ctrlProps = props;
        return true;
    }

    bool ZNodeParams::update_param_socket_visible(const std::string& param, bool bVisible, bool bInput)
    {
        CORE_API_BATCH
        auto name = m_pNode->getNodeStatus()->get_name();
        if (bInput) {
            auto& spParam = safe_at(m_inputPrims, param, "miss input param `" + param + "` on node `" + name + "`");
            if (spParam.bSocketVisible != bVisible)
            {
                spParam.bSocketVisible = bVisible;
                CALLBACK_NOTIFY(update_param_socket_visible, param, bVisible, bInput)
                return true;
            }
        }
        else {
            auto& spParam = safe_at(m_outputPrims, param, "miss output param `" + param + "` on node `" + name + "`");
            if (spParam.bSocketVisible != bVisible)
            {
                spParam.bSocketVisible = bVisible;
                CALLBACK_NOTIFY(update_param_socket_visible, param, bVisible, bInput)
                    return true;
            }
        }
        return false;
    }

    bool ZNodeParams::update_param_visible(const std::string& name, bool bOn, bool bInput)
    {
        if (bInput) {
            if (auto iter = m_inputObjs.find(name); iter != m_inputObjs.end()) {
                auto& paramObj = iter->second;
                if (paramObj.bVisible != bOn) {
                    paramObj.bVisible = bOn;
                    return true;
                }
            }
            else if (auto iter = m_inputPrims.find(name); iter != m_inputPrims.end()){
                auto& paramPrim = iter->second;
                if (paramPrim.bVisible != bOn) {
                    paramPrim.bVisible = bOn;
                    return true;
                }
            }
        }
        else {
            if (auto iter = m_outputObjs.find(name); iter != m_outputObjs.end()) {
                auto& paramObj = iter->second;
                if (paramObj.bVisible != bOn) {
                    paramObj.bVisible = bOn;
                    return true;
                }
            }
            else if (auto iter = m_outputPrims.find(name); iter != m_outputPrims.end()) {
                auto& paramPrim = iter->second;
                if (paramPrim.bVisible != bOn) {
                    paramPrim.bVisible = bOn;
                    return true;
                }
            }
        }
        return false;
    }

    bool ZNodeParams::update_param_enable(const std::string& name, bool bOn, bool bInput)
    {
        if (bInput) {
            if (auto iter = m_inputObjs.find(name); iter != m_inputObjs.end()) {
                auto& paramObj = iter->second;
                if (paramObj.bEnable != bOn) {
                    paramObj.bEnable = bOn;
                    return true;
                }
            }
            else if (auto iter = m_inputPrims.find(name); iter != m_inputPrims.end()) {
                auto& paramPrim = iter->second;
                if (paramPrim.bEnable != bOn) {
                    paramPrim.bEnable = bOn;
                    return true;
                }
            }
            else {
                return false;
            }
        }
        else {
            if (auto iter = m_outputObjs.find(name); iter != m_outputObjs.end()) {
                auto& paramObj = iter->second;
                if (paramObj.bEnable != bOn) {
                    paramObj.bEnable = bOn;
                    return true;
                }
            }
            else if (auto iter = m_outputPrims.find(name); iter != m_outputPrims.end()) {
                auto& paramPrim = iter->second;
                if (paramPrim.bEnable != bOn) {
                    paramPrim.bEnable = bOn;
                    return true;
                }
            }
            else {
                return false;
            }
        }
        return false;
    }

    void ZNodeParams::update_param_color(const std::string& name, std::string& clr)
    {
        CORE_API_BATCH
        CALLBACK_NOTIFY(update_param_color, name, clr)
    }

    void ZNodeParams::update_layout(params_change_info& changes)
    {
        CALLBACK_NOTIFY(update_layout, changes);
    }

    bool ZNodeParams::removeRefLink(const EdgeInfo& edge, bool outParamIsOutput)
    {
        CORE_API_BATCH
        CALLBACK_NOTIFY(removeRefLink, edge, outParamIsOutput)
        return true;
    }

    bool ZNodeParams::removeRefLinkDesParamIndx(bool bInput, bool bPrimitivParam, const std::string& paramName, bool bUiNeedRemoveReflink)
    {
        if (bPrimitivParam) {
            auto& self_prim_params = bInput ? m_inputPrims : m_outputPrims;
            auto it = self_prim_params.find(paramName);
            if (it != self_prim_params.end()) {
                for (auto reflinkIt : it->second.reflinks) {
                    for (auto iter = reflinkIt->dest_inparam->reflinks.begin(); iter != reflinkIt->dest_inparam->reflinks.end(); ) {
                        if ((*iter)->source_inparam == reflinkIt->source_inparam && (*iter)->dest_inparam == reflinkIt->dest_inparam) {
                            iter = reflinkIt->dest_inparam->reflinks.erase(iter);
                        }
                        else {
                            iter++;
                        }
                    }
                    if (bUiNeedRemoveReflink) {
                        EdgeInfo refLink{
                            reflinkIt->source_inparam->m_wpNode->get_name(), paramName, "",
                            reflinkIt->dest_inparam->m_wpNode->get_name(),
                            reflinkIt->dest_inparam->name, "", "", false };
                        removeRefLink(refLink, !bInput);
                    }
                }
                return true;
            }
        }
        else {
            auto& self_obj_params = bInput ? m_inputObjs : m_outputObjs;
            auto it = self_obj_params.find(paramName);
            if (it != self_obj_params.end()) {
                for (auto reflinkIt : it->second.reflinks) {
                    for (auto iter = reflinkIt->dest_inparam->reflinks.begin(); iter != reflinkIt->dest_inparam->reflinks.end(); ) {
                        if ((*iter)->source_inparam == reflinkIt->source_inparam && (*iter)->dest_inparam == reflinkIt->dest_inparam) {
                            iter = reflinkIt->dest_inparam->reflinks.erase(iter);
                        }
                        else {
                            iter++;
                        }
                    }
                    if (bUiNeedRemoveReflink) {
                        EdgeInfo refLink{
                            reflinkIt->source_inparam->m_wpNode->get_name(),
                            paramName, 
                            "",
                            reflinkIt->dest_inparam->m_wpNode->get_name(),
                            reflinkIt->dest_inparam->name, "", "", false };
                        removeRefLink(refLink, !bInput);
                    }
                }
                return true;
            }
        }
        return false;
    }

    params_change_info ZNodeParams::update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit)
    {
        //TODO: 这里只有primitive参数类型的情况，还需要整合obj参数的情况。
        std::set<std::string> inputs_old, outputs_old, obj_inputs_old, obj_outputs_old;
        for (const auto& [param_name, _] : m_inputPrims) {
            inputs_old.insert(param_name);
        }
        for (const auto& [param_name, _] : m_outputPrims) {
            outputs_old.insert(param_name);
        }
        for (const auto& [param_name, _] : m_inputObjs) {
            obj_inputs_old.insert(param_name);
        }
        for (const auto& [param_name, _] : m_outputObjs) {
            obj_outputs_old.insert(param_name);
        }

        params_change_info changes;

        for (auto _pair : params) {
            if (const auto& pParam = std::get_if<ParamObject>(&_pair.param))
            {
                const ParamObject& param = *pParam;
                const std::string oldname = _pair.oldName;
                const std::string newname = param.name;

                auto& self_obj_params = param.bInput ? m_inputObjs : m_outputObjs;
                auto& new_params = param.bInput ? changes.new_inputs : changes.new_outputs;
                auto& remove_params = param.bInput ? changes.remove_inputs : changes.remove_outputs;
                auto& rename_params = param.bInput ? changes.rename_inputs : changes.rename_outputs;

                if (oldname.empty()) {
                    //new added name.
                    if (self_obj_params.find(newname) != self_obj_params.end()) {
                        //调整refelink
                        removeRefLinkDesParamIndx(param.bInput, false, newname);
                        self_obj_params[newname].reflinks.clear();
                        // the new name happen to have the same name with the old name, but they are not the same param.
                        self_obj_params.erase(newname);
                        if (param.bInput)
                            obj_inputs_old.erase(newname);
                        else
                            obj_outputs_old.erase(newname);

                        remove_params.insert(newname);
                    }

                    ObjectParam sparam;
                    sparam.name = newname;
                    sparam.type = param.type;
                    sparam.bWildcard = param.bWildcard;
                    if (!param.bInput) {
                        sparam.socketType = Socket_Output;
                    }
                    else {
                        sparam.socketType = param.socketType;
                    }
                    sparam.m_wpNode = this->m_pNode;
                    self_obj_params[newname] = std::move(sparam);

                    new_params.insert(newname);
                }
                else if (self_obj_params.find(oldname) != self_obj_params.end()) {
                    if (oldname != newname) {
                        //exist name changed.
                        self_obj_params[newname] = std::move(self_obj_params[oldname]);
                        self_obj_params.erase(oldname);
                        //调整refelink
                        removeRefLinkDesParamIndx(param.bInput, false, newname);
                        self_obj_params[newname].reflinks.clear();

                        rename_params.insert({ oldname, newname });
                    }
                    else {
                        //name stays.
                    }

                    if (param.bInput)
                        obj_inputs_old.erase(oldname);
                    else
                        obj_outputs_old.erase(oldname);

                    auto& spParam = self_obj_params[newname];
                    if (param.type != spParam.type) {
                        update_param_type(spParam.name, false, param.bInput, param.type);
                    }
                    spParam.name = newname;
                    spParam.bWildcard = param.bWildcard;
                    if (param.bInput)
                    {
                        update_param_socket_type(spParam.name, param.socketType);
                    }
                }
                else {
                    auto path = m_pNode->getNodeStatus()->get_path();
                    throw makeNodeError<KeyError>(path, oldname, "the name does not exist on the node");
                }
            }
            else if (const auto& pParam = std::get_if<ParamPrimitive>(&_pair.param))
            {
                const ParamPrimitive& param = *pParam;
                const std::string oldname = _pair.oldName;
                const std::string newname = param.name;

                auto& self_prim_params = param.bInput ? m_inputPrims : m_outputPrims;
                auto& new_params = param.bInput ? changes.new_inputs : changes.new_outputs;
                auto& remove_params = param.bInput ? changes.remove_inputs : changes.remove_outputs;
                auto& rename_params = param.bInput ? changes.rename_inputs : changes.rename_outputs;

                if (oldname.empty()) {
                    //new added name.
                    if (self_prim_params.find(newname) != self_prim_params.end()) {
                        //调整refelink
                        removeRefLinkDesParamIndx(param.bInput, true, newname);
                        self_prim_params[newname].reflinks.clear();
                        // the new name happen to have the same name with the old name, but they are not the same param.
                        self_prim_params.erase(newname);
                        if (param.bInput)
                            inputs_old.erase(newname);
                        else
                            outputs_old.erase(newname);

                        remove_params.insert(newname);
                    }

                    PrimitiveParam sparam;
                    sparam.defl = param.defl;
                    if (param.type == sparam.type) {
                        sparam.defl = initAnyDeflValue(sparam.type);
                    }
                    convertToEditVar(sparam.defl, param.type);
                    sparam.name = newname;
                    sparam.type = param.type;
                    sparam.control = param.control;
                    sparam.ctrlProps = param.ctrlProps;
                    sparam.socketType = Socket_Primitve;
                    sparam.bWildcard = param.bWildcard;
                    sparam.m_wpNode = m_pNode;
                    sparam.bSocketVisible = param.bSocketVisible;
                    sparam.bInput = param.bInput;
                    self_prim_params[newname] = std::move(sparam);

                    new_params.insert(newname);
                }
                else if (self_prim_params.find(oldname) != self_prim_params.end()) {
                    if (oldname != newname) {
                        //exist name changed.
                        self_prim_params[newname] = std::move(self_prim_params[oldname]);
                        self_prim_params.erase(oldname);
                        //调整refelink
                        removeRefLinkDesParamIndx(param.bInput, true, newname);
                        self_prim_params[newname].reflinks.clear();

                        rename_params.insert({ oldname, newname });
                    }
                    else {
                        //name stays.
                    }

                    if (param.bInput)
                        inputs_old.erase(oldname);
                    else
                        outputs_old.erase(oldname);

                    auto& spParam = self_prim_params[newname];
                    spParam.defl = param.defl;
                    bool bChangeType = false;
                    if (param.type != spParam.type) {
                        bChangeType = true;
                        update_param_type(spParam.name, true, param.bInput, param.type);
                        spParam.defl = initAnyDeflValue(spParam.type);
                    }
                    convertToEditVar(spParam.defl, spParam.type);

                    spParam.name = newname;
                    spParam.socketType = Socket_Primitve;
                    spParam.bWildcard = param.bWildcard;
                    if (param.bInput)
                    {
                        update_param_control_prop(spParam.name, param.ctrlProps);
                        update_param_control(spParam.name, param.control);
                    }
                }
                else {
                    auto path = m_pNode->getNodeStatus()->get_path();
                    throw makeNodeError<KeyError>(path, oldname, "the name does not exist on the node");
                }
            }
        }

        Graph* spGraph = getGraph();
        const auto& name = m_pNode->getNodeStatus()->get_name();

        //the left names are the names of params which will be removed.
        for (auto rem_name : inputs_old) {
            if (spGraph)
                spGraph->removeLinks(name, true, rem_name);

            //调整refelink
            removeRefLinkDesParamIndx(true, true, rem_name);
            m_inputPrims[rem_name].reflinks.clear();

            m_inputPrims.erase(rem_name);
            changes.remove_inputs.insert(rem_name);
        }

        for (auto rem_name : outputs_old) {
            if (spGraph)
                spGraph->removeLinks(name, false, rem_name);

            //调整refelink
            removeRefLinkDesParamIndx(false, true, rem_name);
            m_outputPrims[rem_name].reflinks.clear();

            m_outputPrims.erase(rem_name);
            changes.remove_outputs.insert(rem_name);
        }

        for (auto rem_name : obj_inputs_old) {
            if (spGraph)
                spGraph->removeLinks(name, true, rem_name);

            //调整refelink
            removeRefLinkDesParamIndx(true, false, rem_name);
            m_inputObjs[rem_name].reflinks.clear();

            m_inputObjs.erase(rem_name);
            changes.remove_inputs.insert(rem_name);
        }

        for (auto rem_name : obj_outputs_old) {
            if (spGraph)
                spGraph->removeLinks(name, false, rem_name);

            //调整refelink
            removeRefLinkDesParamIndx(false, false, rem_name);
            m_outputObjs[rem_name].reflinks.clear();

            m_outputObjs.erase(rem_name);
            changes.remove_outputs.insert(rem_name);
        }
        changes.inputs.clear();
        changes.outputs.clear();
        for (const auto& [param, _] : params) {
            if (auto paramPrim = std::get_if<ParamPrimitive>(&param))
            {
                if (paramPrim->bInput)
                    changes.inputs.push_back(paramPrim->name);
                else
                    changes.outputs.push_back(paramPrim->name);
            }
            else if (auto paramPrim = std::get_if<ParamObject>(&param))
            {
                if (paramPrim->bInput)
                    changes.inputs.push_back(paramPrim->name);
                else
                    changes.outputs.push_back(paramPrim->name);
            }
        }
        return changes;
    }

    void ZNodeParams::trigger_update_params(const std::string& param, bool changed, params_change_info changes)
    {
        if (changed)
            update_layout(changes);
    }

    bool ZNodeParams::has_frame_relative_params() const
    {
        for (auto& [name, param] : m_inputPrims) {
            assert(param.defl.has_value());
            const std::string& uuid = m_pNode->getNodeStatus()->get_uuid();
            if (gParamType_String == param.type) {
                if (param.defl.type().hash_code() == gParamType_PrimVariant) {//type是string，实际defl可能是primvar
                    const zeno::PrimVar& editVar = zeno::reflect::any_cast<zeno::PrimVar>(param.defl);
                    return std::visit([](auto&& arg)-> bool {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            std::string defl = zeno::reflect::any_cast<std::string>(arg);
                            if (defl.find("$F") != std::string::npos)
                                return true;
                        }
                        return false;
                        }, editVar);
                }
                else if (param.defl.type().hash_code() == gParamType_String) {
                    std::string defl = zeno::any_cast_to_string(param.defl);
                    if (defl.find("$F") != std::string::npos) {
                        return true;
                    }
                }
            }
            else if (gParamType_Int == param.type || gParamType_Float == param.type || gParamType_PrimVariant == param.type) {
                assert(gParamType_PrimVariant == param.defl.type().hash_code());
                const zeno::PrimVar& editVar = zeno::reflect::any_cast<zeno::PrimVar>(param.defl);
                bool bFind = std::visit([=](auto&& arg)->bool {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        if (arg.find("$F") != std::string::npos) {
                            return true;
                        }
                    }
                    return false;
                    }, editVar);
                if (bFind)
                    return true;
            }
            else if (gParamType_Vec2f == param.type ||
                gParamType_Vec2i == param.type ||
                gParamType_Vec3f == param.type ||
                gParamType_Vec3i == param.type ||
                gParamType_Vec4f == param.type ||
                gParamType_Vec4i == param.type)
            {
                assert(gParamType_VecEdit == param.defl.type().hash_code());
                const zeno::vecvar& editVar = zeno::reflect::any_cast<zeno::vecvar>(param.defl);
                for (auto primvar : editVar) {
                    bool bFind = std::visit([=](auto&& arg)->bool {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            if (arg.find("$F") != std::string::npos) {
                                return true;
                            }
                        }
                        return false;
                        }, primvar);
                    if (bFind)
                        return true;
                }
            }
        }
        return false;
    }

    // ------------------------- add params -------------------------

    bool ZNodeParams::add_input_prim_param(ParamPrimitive param)
    {
        if (m_inputPrims.find(param.name) != m_inputPrims.end()) {
            return false;
        }
        PrimitiveParam sparam;
        sparam.bInput = true;
        sparam.control = param.control;
        sparam.defl = param.defl;
        convertToEditVar(sparam.defl, param.type);
        sparam.m_wpNode = m_pNode;
        sparam.name = param.name;
        sparam.socketType = param.socketType;
        sparam.type = param.type;
        sparam.ctrlProps = param.ctrlProps;
        sparam.bSocketVisible = param.bSocketVisible;
        sparam.wildCardGroup = param.wildCardGroup;
        sparam.sockprop = param.sockProp;
        //sparam.bInnerParam = param.bInnerParam;
        sparam.constrain = param.constrain;
        m_inputPrims.insert(std::make_pair(param.name, std::move(sparam)));
        return true;
    }

    bool ZNodeParams::add_input_obj_param(ParamObject param)
    {
        if (m_inputObjs.find(param.name) != m_inputObjs.end()) {
            return false;
        }
        ObjectParam sparam;
        sparam.bInput = true;
        sparam.name = param.name;
        sparam.type = param.type;
        sparam.socketType = Socket_Clone;// param.socketType;
        sparam.m_wpNode = m_pNode;
        sparam.wildCardGroup = param.wildCardGroup;
        sparam.constrain = param.constrain;
        m_inputObjs.insert(std::make_pair(param.name, std::move(sparam)));
        return true;
    }

    bool ZNodeParams::add_output_prim_param(ParamPrimitive param)
    {
        if (m_outputPrims.find(param.name) != m_outputPrims.end()) {
            return false;
        }
        PrimitiveParam sparam;
        sparam.bInput = false;
        sparam.control = param.control;
        sparam.defl = param.defl;
        sparam.m_wpNode = m_pNode;
        sparam.name = param.name;
        sparam.socketType = Socket_Clone;// param.socketType;
        sparam.type = param.type;
        sparam.ctrlProps = param.ctrlProps;
        sparam.wildCardGroup = param.wildCardGroup;
        sparam.bSocketVisible = param.bSocketVisible;
        sparam.constrain = param.constrain;
        m_outputPrims.insert(std::make_pair(param.name, std::move(sparam)));
        return true;
    }

    bool ZNodeParams::add_output_obj_param(ParamObject param)
    {
        if (m_outputObjs.find(param.name) != m_outputObjs.end()) {
            return false;
        }
        ObjectParam sparam;
        sparam.bInput = false;
        sparam.name = param.name;
        sparam.type = param.type;
        sparam.socketType = param.socketType;
        sparam.constrain = param.constrain;
        sparam.m_wpNode = m_pNode;
        sparam.wildCardGroup = param.wildCardGroup;
        m_outputObjs.insert(std::make_pair(param.name, std::move(sparam)));
        return true;
    }

    // ------------------------- links / reflink scaffolding -------------------------

    void ZNodeParams::init_object_link(bool bInput, const std::string& paramname, std::shared_ptr<ObjectLink> spLink, const std::string& targetParam)
    {
        auto iter = bInput ? m_inputObjs.find(paramname) : m_outputObjs.find(paramname);
        if (bInput)
            spLink->toparam = &iter->second;
        else
            spLink->fromparam = &iter->second;
        spLink->targetParam = targetParam;
        iter->second.links.emplace_back(spLink);
    }

    void ZNodeParams::init_primitive_link(bool bInput, const std::string& paramname, std::shared_ptr<PrimitiveLink> spLink, const std::string& targetParam)
    {
        auto iter = bInput ? m_inputPrims.find(paramname) : m_outputPrims.find(paramname);
        if (bInput)
            spLink->toparam = &iter->second;
        else
            spLink->fromparam = &iter->second;
        spLink->targetParam = targetParam;
        iter->second.links.emplace_back(spLink);
    }

    bool ZNodeParams::isPrimitiveType(bool bInput, const std::string& param_name, bool& bExist)
    {
        if (bInput) {
            if (m_inputObjs.find(param_name) != m_inputObjs.end()) {
                bExist = true;
                return false;
            }
            else if (m_inputPrims.find(param_name) != m_inputPrims.end()) {
                bExist = true;
                return true;
            }
            bExist = false;
            return false;
        }
        else {
            if (m_outputObjs.find(param_name) != m_outputObjs.end()) {
                bExist = true;
                return false;
            }
            else if (m_outputPrims.find(param_name) != m_outputPrims.end()) {
                bExist = true;
                return true;
            }
            bExist = false;
            return false;
        }
    }

    std::vector<EdgeInfo> ZNodeParams::getLinks() const
    {
        std::vector<EdgeInfo> remLinks;
        for (const auto& [_, spParam] : m_inputObjs) {
            for (std::shared_ptr<ObjectLink> spLink : spParam.links) {
                remLinks.push_back(getEdgeInfo(spLink));
            }
        }
        for (const auto& [_, spParam] : m_inputPrims) {
            for (std::shared_ptr<PrimitiveLink> spLink : spParam.links) {
                remLinks.push_back(getEdgeInfo(spLink));
            }
        }
        for (const auto& [_, spParam] : m_outputObjs) {
            for (std::shared_ptr<ObjectLink> spLink : spParam.links) {
                remLinks.push_back(getEdgeInfo(spLink));
            }
        }
        for (const auto& [_, spParam] : m_outputPrims) {
            for (std::shared_ptr<PrimitiveLink> spLink : spParam.links) {
                remLinks.push_back(getEdgeInfo(spLink));
            }
        }
        return remLinks;
    }

    std::vector<EdgeInfo> ZNodeParams::getLinksByParam(bool bInput, const std::string& param_name) const
    {
        std::vector<EdgeInfo> links;

        auto& objects = bInput ? m_inputObjs : m_outputObjs;
        auto& primtives = bInput ? m_inputPrims : m_outputPrims;

        auto iter = objects.find(param_name);
        if (iter != objects.end()) {
            for (auto spLink : iter->second.links) {
                links.push_back(getEdgeInfo(spLink));
            }
        }
        else {
            auto iter2 = primtives.find(param_name);
            if (iter2 != primtives.end()) {
                for (auto spLink : iter2->second.links) {
                    links.push_back(getEdgeInfo(spLink));
                }
            }
        }
        return links;
    }

    bool ZNodeParams::updateLinkKey(bool bInput, const zeno::EdgeInfo& edge, const std::string& oldkey, const std::string& newkey)
    {
        auto& objects = bInput ? m_inputObjs : m_outputObjs;
        auto iter = objects.find(edge.inParam);
        if (iter != objects.end()) {
            for (auto spLink : iter->second.links) {
                if (auto fromParam = spLink->fromparam) {
                    if (auto outnode = fromParam->m_wpNode) {
                        if (outnode->get_name() == edge.outNode && spLink->tokey == oldkey) {   //需outnode和tokey均相同
                            spLink->tokey = newkey;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool ZNodeParams::moveUpLinkKey(bool bInput, const std::string& param_name, const std::string& key)
    {
        auto& objects = bInput ? m_inputObjs : m_outputObjs;
        auto iter = objects.find(param_name);
        if (iter != objects.end()) {
            for (auto it = iter->second.links.begin(); it != iter->second.links.end(); it++) {
                if ((*it)->tokey == key) {
                    auto it_ = std::prev(it);
                    std::swap(*it, *it_);
                    return true;
                }
            }
        }
        return false;
    }

    bool ZNodeParams::removeLink(bool bInput, const EdgeInfo& edge)
    {
        //现有规则不允许输入对象和输入数值参数同名，所以不需要bObjLink，而且bObjLink好像只是用于给ui区分而已
        if (bInput) {
            auto iter = m_inputObjs.find(edge.inParam);
            if (iter != m_inputObjs.end()) {
                for (auto spLink : iter->second.links) {
                    if (auto outNode = spLink->fromparam->m_wpNode) {
                        if (outNode->get_name() == edge.outNode && spLink->fromparam->name == edge.outParam && spLink->fromkey == edge.outKey) {
                            iter->second.links.remove(spLink);
                            return true;
                        }
                    }
                }
            }
            auto iter2 = m_inputPrims.find(edge.inParam);
            if (iter2 != m_inputPrims.end()) {
                for (auto spLink : iter2->second.links) {
                    if (auto outNode = spLink->fromparam->m_wpNode) {
                        if (outNode->get_name() == edge.outNode && spLink->fromparam->name == edge.outParam) {
                            iter2->second.links.remove(spLink);
                            return true;
                        }
                    }
                }
            }
        }
        else {
            auto iter = m_outputObjs.find(edge.outParam);
            if (iter != m_outputObjs.end())
            {
                for (auto spLink : iter->second.links) {
                    if (auto inNode = spLink->toparam->m_wpNode) {
                        if (inNode->get_name() == edge.inNode && spLink->toparam->name == edge.inParam/* && spLink->tokey == edge.inKey*/) {
                            iter->second.links.remove(spLink);
                            return true;
                        }
                    }
                }
            }

            auto iter2 = m_outputPrims.find(edge.outParam);
            if (iter2 != m_outputPrims.end())
            {
                for (auto spLink : iter2->second.links) {
                    if (auto inNode = spLink->toparam->m_wpNode) {
                        if (inNode->get_name() == edge.inNode && spLink->toparam->name == edge.inParam) {
                            iter2->second.links.remove(spLink);
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    std::vector<std::pair<std::string, bool>> ZNodeParams::getWildCardParams(const std::string& param_name, bool bPrim)
    {
        std::vector<std::pair<std::string, bool>> params;
        if (bPrim)
        {
            std::string wildCardGroup;
            if (m_inputPrims.find(param_name) != m_inputPrims.end())
            {
                wildCardGroup = m_inputPrims.find(param_name)->second.wildCardGroup;
            }
            else if (m_outputPrims.find(param_name) != m_outputPrims.end())
            {
                wildCardGroup = m_outputPrims.find(param_name)->second.wildCardGroup;
            }
            for (const auto& [name, spParam] : m_inputPrims)
            {
                if (spParam.wildCardGroup == wildCardGroup)
                {
                    if (!wildCardGroup.empty() || param_name == name) {
                        params.push_back({ name, true });
                    }
                }
            }
            for (const auto& [name, spParam] : m_outputPrims)
            {
                if (spParam.wildCardGroup == wildCardGroup)
                {
                    if (!wildCardGroup.empty() || param_name == name) {
                        params.push_back({ name, false });
                    }
                }
            }
        }
        else
        {
            std::string wildCardGroup;
            if (m_inputObjs.find(param_name) != m_inputObjs.end())
            {
                wildCardGroup = m_inputObjs.find(param_name)->second.wildCardGroup;
            }
            else if (m_outputObjs.find(param_name) != m_outputObjs.end())
            {
                wildCardGroup = m_outputObjs.find(param_name)->second.wildCardGroup;
            }
            for (const auto& [name, spParam] : m_inputObjs)
            {
                if (spParam.wildCardGroup == wildCardGroup)
                {
                    if (!wildCardGroup.empty() || param_name == name) {
                        params.push_back({ name, true });
                    }
                }
            }
            for (const auto& [name, spParam] : m_outputObjs)
            {
                if (spParam.wildCardGroup == wildCardGroup)
                {
                    if (!wildCardGroup.empty() || param_name == name) {
                        params.push_back({ name, false });
                    }
                }
            }
        }
        return params;
    }

    void ZNodeParams::getParamTypeAndSocketType(const std::string& param_name, bool bPrim, bool bInput, ParamType& paramType, SocketType& socketType, bool& bWildcard)
    {
        if (bPrim) {
            auto iter = bInput ? m_inputPrims.find(param_name) : m_outputPrims.find(param_name);
            if (bInput ? (iter != m_inputPrims.end()) : (iter != m_outputPrims.end())) {
                paramType = iter->second.type;
                socketType = iter->second.socketType;
                bWildcard = iter->second.bWildcard;
                return;
            }
        }
        else {
            auto iter = bInput ? m_inputObjs.find(param_name) : m_outputObjs.find(param_name);
            if (bInput ? (iter != m_inputObjs.end()) : (iter != m_outputObjs.end())) {
                paramType = iter->second.type;
                socketType = iter->second.socketType;
                bWildcard = iter->second.bWildcard;
                return;
            }
        }
        paramType = Param_Null;
        socketType = Socket_Primitve;
    }

    std::set<RefSourceInfo> ZNodeParams::resolveReferSource(const zeno::reflect::Any& param_defl) {
    std::set<RefSourceInfo> refSources;
        std::vector<std::string> refSegments;

        ParamType deflType = param_defl.type().hash_code();
        if (deflType == zeno::types::gParamType_String) {
            const std::string& param_text = zeno::any_cast_to_string(param_defl);
            if (!param_text.empty()) {//不限制为ref/refout，可能是npoints("./Cube1.Output")之类的
                refSegments.push_back(param_text);
            }
        }
        else if (deflType == zeno::types::gParamType_PrimVariant) {
            zeno::PrimVar var = zeno::reflect::any_cast<zeno::PrimVar>(param_defl);
            if (!std::holds_alternative<std::string>(var)) {
                return refSources;
            }
            std::string param_text = std::get<std::string>(var);
            if (!param_text.empty()) {
                refSegments.push_back(param_text);
            }
        }
        else if (deflType == zeno::types::gParamType_VecEdit) {
            const zeno::vecvar& vec = zeno::reflect::any_cast<zeno::vecvar>(param_defl);
            for (const zeno::PrimVar& elem : vec) {
                if (!std::holds_alternative<std::string>(elem)) {
                    continue;
                }
                std::string param_text = std::get<std::string>(elem);
                if (!param_text.empty()) {
                    refSegments.push_back(param_text);
                }
            }
        }

        if (refSegments.empty())
            return refSources;

        auto namePath = m_pNode->get_path();

        //需要用zfxparser直接parse出所有引用信息
        GlobalError err;
        zeno::GraphException::catched([&] {
            FunctionManager funcMgr;
            ZfxContext ctx;
            ctx.spNode = getNode();
            for (auto param_text : refSegments)
            {
                std::string code = param_text;
                if (param_text.find('\n') == std::string::npos && param_text.back() != ';') {
                    code = param_text + ';';    //方便通过zfx语句编译
                }

                ZfxContext ctx;
                ctx.spNode = getNode();
                ctx.spObject = nullptr;
                ctx.code = code;
                ZfxExecute zfx(code, &ctx);
                int ret = zfx.parse();
                if (ret == 0)
                {
                    ctx.code = code;
                    std::shared_ptr<ZfxASTNode> astRoot = zfx.getASTResult();
                    std::set<RefSourceInfo> paths =
                        funcMgr.getReferSources(astRoot, &ctx);
                    if (!paths.empty()) {
                        refSources.insert(paths.begin(), paths.end());
                    }
                }
            }
        }, err);
        return refSources;
    }

    bool ZNodeParams::addRefLink(const EdgeInfo& edge, bool outParamIsOutput)
    {
        CORE_API_BATCH
        //如果边已存在，删除这条边再添加引用边
        if (outParamIsOutput) {
            if (auto outnode = getGraph()->getNode(edge.outNode)) {
                std::vector<EdgeInfo> links = outnode->getNodeParams()-> getLinksByParam(false, edge.outParam);
                for (auto link : links) {
                    if (link == edge) {
                        getGraph()->removeLink(edge);
                    }
                }
            }
        }

        CALLBACK_NOTIFY(addRefLink, edge, outParamIsOutput)
            return true;
    }

    void ZNodeParams::initReferLinks(PrimitiveParam* target_param) {
        if (getGraph()->isAssetRoot()) {
            //资产图不会执行，无须构造引用关系
            return;
        }
        std::set<RefSourceInfo> refSources = resolveReferSource(target_param->defl);
        auto newAdded = refSources;

        for (auto iter = target_param->reflinks.begin(); iter != target_param->reflinks.end(); )
        {
            bool bExist = false;
            std::shared_ptr<ReferLink> spRefLink = (*iter);
            CoreParam* remote_source = spRefLink->source_inparam;
            assert(remote_source);
            if (remote_source == target_param) {
                iter++;
                continue;
            }

            //查看当前link在新的集合里是否还存在。
            for (const auto& refSrcInfo : refSources)
            {
                auto spSrcNode = remote_source->m_wpNode;
                if (spSrcNode->getNodeStatus()->get_uuid_path() == refSrcInfo.uuidPath &&
                    remote_source->name == refSrcInfo.paramName &&
                    remote_source->bInput == (refSrcInfo.funcName == "ref")) {
                    //已经有了
                    bExist = true;
                    newAdded.erase(refSrcInfo);
                    break;
                }
            }

            if (bExist) {
                iter++;
            }
            else {
                iter = target_param->reflinks.erase(iter);
                auto& other_links = remote_source->reflinks;
                other_links.erase(std::remove(other_links.begin(), other_links.end(), spRefLink));

                EdgeInfo refLink{ remote_source->m_wpNode->get_name(), remote_source->name, "", target_param->m_wpNode->get_name(), target_param->name, "", "", false };
                removeRefLink(refLink, !remote_source->bInput);
            }
        }

        for (const auto& refSrcInfo : newAdded)
        {
            //目前引用功能只支持本图引用，不能跨图引用
            std::string sourcenode_uuid;
            if (refSrcInfo.uuidPath.find('/') != std::string::npos) {
                sourcenode_uuid = refSrcInfo.uuidPath.substr(refSrcInfo.uuidPath.find_last_of('/') + 1);
            }
            else {
                sourcenode_uuid = refSrcInfo.uuidPath;
            }

            auto srcNode = getGraph()->getNodeByUuidPath(sourcenode_uuid);
            if (!srcNode) {
                zeno::log_warn("invalid ref");
                continue;
            }

            if (refSrcInfo.funcName != "refout") {
                bool isSubInput = sourcenode_uuid.find("SubInput") != std::string::npos;
                auto& inoutPrims = isSubInput ? 
                    srcNode->getNodeParams()->m_outputPrims :
                    srcNode->getNodeParams()->m_inputPrims;
                auto iterSrcParam = inoutPrims.find(refSrcInfo.paramName);
                if (iterSrcParam != inoutPrims.end()) {
                    PrimitiveParam& srcparam = iterSrcParam->second;
                    if (&srcparam != target_param)  //排除直接引用自己的情况
                    {
                        //构造reflink
                        std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                        reflink->source_inparam = &srcparam;
                        reflink->dest_inparam = target_param;
                        target_param->reflinks.push_back(reflink);
                        srcparam.reflinks.push_back(reflink);

                        addRefLink(EdgeInfo({ srcNode->get_name(), refSrcInfo.paramName, "", target_param->m_wpNode->get_name(), target_param->name, "", "", false }), isSubInput);
                    }
                }
            }
            if (refSrcInfo.funcName != "ref") {
                bool isSubOutput = sourcenode_uuid.find("SubOutput") != std::string::npos;
                auto& inoutPrims = isSubOutput ? srcNode->getNodeParams()->m_inputPrims : srcNode->getNodeParams()->m_outputPrims;
                auto& inoutObjs = isSubOutput ? srcNode->getNodeParams()->m_inputObjs : srcNode->getNodeParams()->m_outputObjs;

                auto iterSrcObj = inoutObjs.find(refSrcInfo.paramName);
                if (iterSrcObj != inoutObjs.end()) {
                    ObjectParam& srcObj = iterSrcObj->second;
                    //构造reflink
                    std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                    reflink->source_inparam = &srcObj;
                    reflink->dest_inparam = target_param;
                    target_param->reflinks.push_back(reflink);
                    srcObj.reflinks.push_back(reflink);

                    addRefLink(EdgeInfo({ srcNode->get_name(), refSrcInfo.paramName, "", target_param->m_wpNode->get_name(), target_param->name, "", "", false }), !isSubOutput);
                }
                else {
                    auto iterOutPrim = inoutPrims.find(refSrcInfo.paramName);
                    if (iterOutPrim != inoutPrims.end()) {
                        PrimitiveParam& srcparam = iterOutPrim->second;
                        //构造reflink
                        std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                        reflink->source_inparam = &srcparam;
                        reflink->dest_inparam = target_param;
                        target_param->reflinks.push_back(reflink);
                        srcparam.reflinks.push_back(reflink);

                        addRefLink(EdgeInfo({ srcNode->get_name(), refSrcInfo.paramName, "", target_param->m_wpNode->get_name(), target_param->name, "", "", false }), !isSubOutput);
                    }
                }
            }
        }
    }

    void ZNodeParams::constructReference(const std::string& param_name)
    {
        auto iter = m_inputPrims.find(param_name);
        if (iter == m_inputPrims.end())
            return;

        const Any& param_defl = iter->second.defl;
        initReferLinks(&iter->second);
    }

    void ZNodeParams::checkParamsConstrain()
    {
        //ZfxContext
        auto& funcMgr = zeno::getSession().funcManager;
        ZfxContext ctx;
        ctx.spNode = m_pNode;
        //对于所有带有约束的输入参数，调整其可见和可用情况

        std::set<std::string> adjInputs, adjOutputs;

        bool bParamPropChanged = false;
        for (const auto& [name, param] : m_inputObjs) {
            if (!param.constrain.empty()) {
                ctx.code = param.constrain;
                ctx.param_constrain.constrain_param = name;
                ctx.param_constrain.bInput = true;
                ZfxExecute zfx(ctx.code, &ctx);
                zfx.execute();
                if (ctx.param_constrain.update_nodeparam_prop) {
                    bParamPropChanged = true;
                    adjInputs.insert(name);
                }
            }
        }
        for (const auto& [name, param] : m_inputPrims) {
            if (!param.constrain.empty()) {
                ctx.code = param.constrain;
                ctx.param_constrain.constrain_param = name;
                ctx.param_constrain.bInput = true;
                ZfxExecute zfx(ctx.code, &ctx);
                zfx.execute();
                if (ctx.param_constrain.update_nodeparam_prop) {
                    bParamPropChanged = true;
                    adjInputs.insert(name);
                }
            }
        }
        for (const auto& [name, param] : m_outputPrims) {
            if (!param.constrain.empty()) {
                ctx.code = param.constrain;
                ctx.param_constrain.constrain_param = name;
                ctx.param_constrain.bInput = false;
                ZfxExecute zfx(ctx.code, &ctx);
                zfx.execute();
                if (ctx.param_constrain.update_nodeparam_prop) {
                    bParamPropChanged = true;
                    adjOutputs.insert(name);
                }
            }
        }
        for (const auto& [name, param] : m_outputObjs) {
            if (!param.constrain.empty()) {
                ctx.code = param.constrain;
                ctx.param_constrain.constrain_param = name;
                ctx.param_constrain.bInput = false;
                ZfxExecute zfx(ctx.code, &ctx);
                zfx.execute();
                if (ctx.param_constrain.update_nodeparam_prop) {
                    bParamPropChanged = true;
                    adjOutputs.insert(name);
                }
            }
        }

        if (bParamPropChanged) {
            //通知上层UI去统一更新
            // TODO
            //CALLBACK_NOTIFY(update_visable_enable, this, adjInputs, adjOutputs)
        }
    }

    std::vector<RefLinkInfo> ZNodeParams::getReflinkInfo(bool bOnlySearchByDestNode)
    {
        std::vector<RefLinkInfo> refLinksInfo;
        for (auto& [name, param] : m_inputPrims) {
            for (auto link : param.reflinks) {
                if (link->source_inparam && link->dest_inparam) {
                    if (bOnlySearchByDestNode) {
                        if (link->source_inparam == &param) {
                            continue;
                        }
                    }
                    if (link->source_inparam->m_wpNode && link->dest_inparam->m_wpNode) {
                        EdgeInfo refLink{
                            link->source_inparam->m_wpNode->get_name(),
                            link->source_inparam->name, "",
                            link->dest_inparam->m_wpNode->get_name(),
                            link->dest_inparam->name, "", "", false };
                        refLinksInfo.push_back({ refLink, !link->source_inparam->bInput });
                    }
                }
            }
        }
        if (!bOnlySearchByDestNode) {
            for (auto& [name, param] : m_outputPrims) {
                for (auto link : param.reflinks) {
                    if (link->source_inparam && link->dest_inparam) {
                        if (link->source_inparam->m_wpNode && link->dest_inparam->m_wpNode) {
                            EdgeInfo refLink{
                                link->source_inparam->m_wpNode->get_name(),
                                link->source_inparam->name, "",
                                link->dest_inparam->m_wpNode->get_name(),
                                link->dest_inparam->name, "", "", false };
                            refLinksInfo.push_back({ refLink, !link->source_inparam->bInput });
                        }
                    }
                }
            }
            for (auto& [name, param] : m_outputObjs) {
                for (auto link : param.reflinks) {
                    if (link->source_inparam && link->dest_inparam) {
                        if (link->source_inparam->m_wpNode && link->dest_inparam->m_wpNode) {
                            EdgeInfo refLink{
                                link->source_inparam->m_wpNode->get_name(),
                                link->source_inparam->name, "",
                                link->dest_inparam->m_wpNode->get_name(),
                                link->dest_inparam->name, "", "", false };
                            refLinksInfo.push_back({ refLink, !link->source_inparam->bInput });
                        }
                    }
                }
            }
        }
        return refLinksInfo;
    }

    void ZNodeParams::removeNodeUpdateRefLink(const zeno::EdgeInfo& link, bool bAddRef, bool bOutParamIsOutput)
    {
        auto outnode = getGraph()->getNode(link.outNode);
        auto innode = getGraph()->getNode(link.inNode);
        if (outnode && innode) {
            CoreParam* outCoreParam = nullptr;
            auto& outPrims = bOutParamIsOutput ? outnode->getNodeParams()->m_outputPrims : outnode->getNodeParams()->m_inputPrims;
            auto outPrimParamIt = outPrims.find(link.outParam);
            if (outPrimParamIt == outPrims.end()) {
                auto outObjParamIt = outnode->getNodeParams()->m_outputObjs.find(link.outParam);
                if (outObjParamIt == outnode->getNodeParams()->m_outputObjs.end()) {
                    return;
                }
                outCoreParam = &outObjParamIt->second;
            }
            else {
                outCoreParam = &outPrimParamIt->second;
            }
            auto inParamIt = innode->getNodeParams()->m_inputPrims.find(link.inParam);
            if (inParamIt == innode->getNodeParams()->m_inputPrims.end()) {
                return;
            }
            if (bAddRef) {
                for (auto iter = outCoreParam->reflinks.begin(); iter != outCoreParam->reflinks.end(); iter++) {
                    auto spReflink = *iter;
                    if (spReflink->source_inparam->name == link.outParam &&
                        spReflink->dest_inparam->name == link.inParam &&
                        spReflink->dest_inparam->m_wpNode->get_name() == link.inNode)
                        return;
                }
                std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                reflink->source_inparam = outCoreParam;
                reflink->dest_inparam = &inParamIt->second;
                outCoreParam->reflinks.push_back(reflink);
                inParamIt->second.reflinks.push_back(reflink);
                addRefLink(link, bOutParamIsOutput);
            }
            else {
                for (auto iter = outCoreParam->reflinks.begin(); iter != outCoreParam->reflinks.end(); ) {
                    auto spReflink = *iter;
                    if (spReflink->source_inparam->name == link.outParam &&
                        spReflink->dest_inparam->name == link.inParam &&
                        spReflink->dest_inparam->m_wpNode->get_name() == link.inNode) {
                        inParamIt->second.reflinks.erase(std::remove(inParamIt->second.reflinks.begin(), inParamIt->second.reflinks.end(), spReflink));
                        outCoreParam->reflinks.erase(iter);
                        removeRefLink(link, bOutParamIsOutput);
                        break;
                    }
                    iter++;
                }
            }
        }
    }

    zany2 ZNodeParams::clone_input(std::string const& id) const {
        auto iter = m_inputPrims.find(id);
        if (iter != m_inputPrims.end()) {
            throw makeNodeError<UnimplError>(m_pNode->get_path(), id + "is not a input object");
        }
        else {
            auto iter2 = m_inputObjs.find(id);
            if (iter2 != m_inputObjs.end()) {
                if (!iter2->second.spObject)
                    return nullptr;
                return zany2(iter2->second.spObject->clone());
            }
            throw makeNodeError<KeyError>(m_pNode->get_path(), id, "get_input");
        }
        return nullptr;
    }

    bool ZNodeParams::has_link_input(std::string const& id) const {
        //这个对应的是老版本的has_input
        auto iter = m_inputObjs.find(id);
        if (iter != m_inputObjs.end()) {
            return !iter->second.links.empty();
        }
        else {
            auto iter = m_inputPrims.find(id);
            if (iter != m_inputPrims.end()) {
                return !iter->second.links.empty();
            }
            return false;
        }
    }

    bool ZNodeParams::has_input(std::string const& id) const {
        //这个has_input在旧的语义里，代表的是input obj，如果有一些边没有连上，或者有些参数没有设置默认值，就不会加到这个`input`里
        // 设计上过于随意，参考老版本的NewFBXSceneInfo.frameid

        // 由于新版本已经和旧版本不一致，如果要最大限度兼容，只能考虑：
        //1. 有边连着都算
        //2. 只要参数有数值结果，都算（如果是老版本的不带默认参数的，请用has_link_input）

        auto iter = m_inputObjs.find(id);
        if (iter != m_inputObjs.end()) {
            return !iter->second.links.empty();
        }
        else {
            auto iter = m_inputPrims.find(id);
            if (iter != m_inputPrims.end()) {
                const auto& _prim = iter->second;
                if (_prim.result.has_value())
                    return true;
                //看有没有边连着
                return !iter->second.links.empty();
            }
            return false;
        }
    }

    GlobalState* ZNodeParams::getGlobalState() const {
        return getSession().globalState.get();
    }

    // ------------------------- INodeData minimal plumbing -------------------------

    IObject2* ZNodeParams::get_input_object(const char* param)
    {
        auto iter2 = m_inputObjs.find(param);
        if (iter2 != m_inputObjs.end()) {
            return iter2->second.spObject.get();
        }
        return nullptr;
    }

    IObject2* ZNodeParams::clone_input_object(const char* param)
    {
        return clone_input(std::string(param)).release();
    }

    IPrimitiveObject* ZNodeParams::get_input_PrimitiveObject(const char* param)
    {
        auto obj = get_input_object(param);
        if (ZObj_Geometry == obj->type()) {
            return static_cast<IPrimitiveObject*>(obj);
        }
        else {
            return nullptr;
        }
    }

    IGeometryObject* ZNodeParams::get_input_Geometry(const char* param)
    {
        auto obj = get_input_object(param);
        if (ZObj_Geometry == obj->type()) {
            return static_cast<IGeometryObject*>(obj);
        }
        else {
            return nullptr;
        }
    }

    IGeometryObject* ZNodeParams::clone_input_Geometry(const char* param)
    {
        auto obj = get_input_object(param);
        if (ZObj_Geometry == obj->type()) {
            return static_cast<IGeometryObject*>(obj->clone());
        }
        else {
            return nullptr;
        }
    }

    IListObject* ZNodeParams::get_input_ListObject(const char* param)
    {
        auto obj = get_input_object(param);
        if (ZObj_List == obj->type()) {
            return static_cast<IListObject*>(obj);
        }
        else {
            return nullptr;
        }
    }

    int ZNodeParams::get_input2_int(const char* param)
    {
        const auto& anyVal = get_param_result(std::string(param));
        int res = 0;
        if (anyVal.type().hash_code() == zeno::types::gParamType_Int) {
            res = zeno::reflect::any_cast<int>(anyVal);
        }
        else if (anyVal.type().hash_code() == zeno::types::gParamType_Float) {
            res = zeno::reflect::any_cast<float>(anyVal);
        }
        else if (anyVal.type().hash_code() == zeno::types::gParamType_Bool) {
            res = zeno::reflect::any_cast<bool>(anyVal);
        }
        return res;
    }

    float ZNodeParams::get_input2_float(const char* param)
    {
        const auto& anyVal = get_param_result(std::string(param));
        float res = 0;
        if (anyVal.type().hash_code() == zeno::types::gParamType_Int) {
            res = zeno::reflect::any_cast<int>(anyVal);
        }
        else if (anyVal.type().hash_code() == zeno::types::gParamType_Float) {
            res = zeno::reflect::any_cast<float>(anyVal);
        }
        else if (anyVal.type().hash_code() == zeno::types::gParamType_Bool) {
            res = zeno::reflect::any_cast<bool>(anyVal);
        }
        return res;
    }

    int ZNodeParams::get_input2_string(const char* param, char* ret, size_t cap)
    {
        const auto& str = any_cast<std::string>(get_param_result(std::string(param)));
        return stdStr2charArr(str, ret, cap);
    }

    bool ZNodeParams::get_input2_bool(const char* param)
    {
        return any_cast<bool>(get_param_result(std::string(param)));
    }

    bool ZNodeParams::has_input(const char* param)
    {
        return has_input(std::string(param));
    }

    bool ZNodeParams::has_link_input(const char* param)
    {
        return has_link_input(std::string(param));
    }

    Vec2i ZNodeParams::get_input2_vec2i(const char* param) {
        return toAbiVec2i(any_cast<vec2i>(get_param_result(zsString2Std(param))));
    }

    Vec2f ZNodeParams::get_input2_vec2f(const char* param) {
        return toAbiVec2f(any_cast<vec2f>(get_param_result(zsString2Std(param))));
    }

    Vec3i ZNodeParams::get_input2_vec3i(const char* param) {
        return toAbiVec3i(any_cast<vec3i>(get_param_result(zsString2Std(param))));
    }

    Vec3f ZNodeParams::get_input2_vec3f(const char* param) {
        return toAbiVec3f(any_cast<vec3f>(get_param_result(zsString2Std(param))));
    }

    Vec4i ZNodeParams::get_input2_vec4i(const char* param) {
        return toAbiVec4i(any_cast<vec4i>(get_param_result(zsString2Std(param))));
    }

    Vec4f ZNodeParams::get_input2_vec4f(const char* param) {
        return toAbiVec4f(any_cast<vec4f>(get_param_result(zsString2Std(param))));
    }

    bool ZNodeParams::set_output_object(const char* param, IObject2* detached_obj)
    {
        return set_output(std::string(param), zany2(detached_obj));
    }

    bool ZNodeParams::set_output_int(const char* param, int val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_float(const char* param, float val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_bool(const char* param, bool val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_string(const char* param, const char* val)
    {
        if (!param) return false;
        std::string s = val ? std::string(val) : std::string();
        return set_primitive_output(param, zeno::reflect::Any(s));
    }

    bool ZNodeParams::set_output_vec2f(const char* param, Vec2f val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_vec2i(const char* param, Vec2i val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_vec3f(const char* param, Vec3f val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_vec3i(const char* param, Vec3i val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_vec4f(const char* param, Vec4f val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    bool ZNodeParams::set_output_vec4i(const char* param, Vec4i val)
    {
        if (!param) return false;
        return set_primitive_output(param, zeno::reflect::Any(val));
    }

    int ZNodeParams::GetFrameId() const
    {
        return getGlobalState()->getFrameId();
    }

} // namespace zeno
