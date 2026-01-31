#include <zeno/core/NodeRegister.h>
#include <zeno/utils/logger.h>
#include <zeno/core/INodeClass.h>
#include <zeno/utils/helper.h>
#include <zeno/core/reflectdef.h>
#include <zeno/core/typeinfo.h>

using namespace zeno::reflect;
using namespace zeno::types;


namespace zeno {

    static CustomUI descToCustomui(const Descriptor& desc) {
        //兼容以前写的各种ZENDEFINE
        CustomUI ui;

        ui.nickname = desc.displayName;
        ui.uistyle.iconResPath = desc.iconResPath;
        ui.doc = desc.doc;
        if (!desc.categories.empty())
            ui.category = desc.categories[0];   //很多cate都只有一个

        ParamGroup default_group;
        for (const SocketDescriptor& param_desc : desc.inputs) {
            if (param_desc._desc.type == Desc_Prim) {
                //有可能没赋初值，要检查一下
                if (!param_desc._desc.primparam.defl.has_value()) {
                    ParamPrimitive newparam = param_desc._desc.primparam;
                    newparam.defl = initAnyDeflValue(newparam.type);
                    default_group.params.push_back(newparam);
                }
                else {
                    //字符串的defl都是以const char*作为内部储存的类型，这里转一下string，以免上层逻辑处理疏漏
                    ParamPrimitive newparam = param_desc._desc.primparam;
                    if (newparam.defl.type() == zeno::reflect::type_info<const char*>()) {
                        std::string str = zeno::reflect::any_cast<const char*>(newparam.defl);
                        newparam.defl = str;
                    }
                    convertToEditVar(newparam.defl, newparam.type);
                    default_group.params.push_back(newparam);
                }
            }
            else if (param_desc._desc.type == Desc_Obj) {
                ui.inputObjs.push_back(param_desc._desc.objparam);
            }
            else {
                ParamType type = param_desc.type;
                if (isPrimitiveType(type)) {
                    //如果是数值类型，就添加到组里
                    ParamPrimitive param;
                    param.name = param_desc.name;
                    param.type = type;
                    param.defl = zeno::str2any(param_desc.defl, param.type);
                    convertToEditVar(param.defl, param.type);
                    if (param_desc.socketType != zeno::NoSocket)
                        param.socketType = param_desc.socketType;
                    if (param_desc.control != NullControl)
                        param.control = param_desc.control;
                    if (!param_desc.comboxitems.empty()) {
                        //compatible with old case of combobox items.
                        param.type = zeno::types::gParamType_String;
                        param.control = Combobox;
                        std::vector<std::string> items = split_str(param_desc.comboxitems, ' ', false);
                        if (!items.empty()) {
                            items.erase(items.begin());
                            param.ctrlProps = items;
                        }
                    }
                    if (param.type != Param_Null && param.control == NullControl)
                        param.control = getDefaultControl(param.type);
                    param.tooltip = param_desc.doc;
                    param.sockProp = Socket_Normal;
                    param.wildCardGroup = param_desc.wildCard;
                    param.bSocketVisible = false;
                    default_group.params.push_back(param);
                }
                else
                {
                    //其他一律认为是对象（Zeno目前的类型管理非常混乱，有些类型值是空字符串，但绝大多数是对象类型
                    ParamObject param;
                    param.name = param_desc.name;
                    param.type = type;
                    param.socketType = Socket_Clone;        //在此版本里，不再区分owing, readonly clone，全都是clone.
                    //if (param_desc.socketType != zeno::NoSocket)
                    //    param.socketType = param_desc.socketType;
                    param.bInput = true;
                    param.wildCardGroup = param_desc.wildCard;

                    //dict和list允许多连接口，且不限定对象类型（但只能是对象，暂不接收primitive，否则就违反了对象和primitive分开连的设计了）
                    if (type == gParamType_List) {
                        param.sockProp = Socket_MultiInput;
                    }
                    else {
                        param.sockProp = Socket_Normal;
                    }

                    ui.inputObjs.push_back(param);
                }
            }
        }
        for (const ParamDescriptor& param_desc : desc.params) {
            ParamPrimitive param;
            param.name = param_desc.name;
            param.type = param_desc.type;
            param.defl = zeno::str2any(param_desc.defl, param.type);
            convertToEditVar(param.defl, param.type);
            param.socketType = NoSocket;
            //其他控件估计是根据类型推断的。
            if (!param_desc.comboxitems.empty()) {
                //compatible with old case of combobox items.
                param.type = zeno::types::gParamType_String;
                param.control = Combobox;
                std::vector<std::string> items = split_str(param_desc.comboxitems, ' ', false);
                if (!items.empty()) {
                    items.erase(items.begin());
                    param.ctrlProps = items;
                }
            }
            if (param.type != Param_Null) {
                if (param_desc.control == NullControl) {
                    param.control = getDefaultControl(param.type);
                }
                else {
                    param.control = param_desc.control;
                }
            }
            param.tooltip = param_desc.doc;
            param.bSocketVisible = false;
            default_group.params.push_back(param);
        }
        for (const SocketDescriptor& param_desc : desc.outputs) {
            if (param_desc._desc.type == Desc_Prim) {
                ui.outputPrims.push_back(param_desc._desc.primparam);
            }
            else if (param_desc._desc.type == Desc_Obj) {
                ui.outputObjs.push_back(param_desc._desc.objparam);
            }
            else {
                ParamType type = param_desc.type;
                if (isPrimitiveType(type)) {
                    //如果是数值类型，就添加到组里
                    ParamPrimitive param;
                    param.name = param_desc.name;
                    param.type = type;
                    param.defl = zeno::str2any(param_desc.defl, param.type);
                    //输出的数据端口没必要将vec转为vecedit
                    if (param_desc.socketType != zeno::NoSocket)
                        param.socketType = param_desc.socketType;
                    param.control = NullControl;
                    param.tooltip = param_desc.doc;
                    param.sockProp = Socket_Normal;
                    param.wildCardGroup = param_desc.wildCard;
                    param.bSocketVisible = false;
                    ui.outputPrims.push_back(param);
                }
                else
                {
                    //其他一律认为是对象（Zeno目前的类型管理非常混乱，有些类型值是空字符串，但绝大多数是对象类型
                    ParamObject param;
                    param.name = param_desc.name;
                    param.type = type;
                    if (param_desc.socketType != zeno::NoSocket)
                        param.socketType = param_desc.socketType;
                    if (!param.bWildcard)  //输出可能是wildCard
                        param.socketType = Socket_Output;
                    param.bInput = false;
                    param.sockProp = Socket_Normal;
                    param.wildCardGroup = param_desc.wildCard;
                    ui.outputObjs.push_back(param);
                }
            }
        }
        ParamTab tab;
        tab.groups.emplace_back(std::move(default_group));
        ui.inputPrims.emplace_back(std::move(tab));
        return ui;
    }


    ZENO_API NodeRegister& getNodeRegister() {
        return NodeRegister::instance();
    }

	NodeRegister& NodeRegister::instance() {
		static NodeRegister reg;
		return reg;
	}

    NodeRegister::NodeRegister() {

    }

	int NodeRegister::registerNodeClass(
		INode2* (*ctor)(),
        void (*dtor)(INode2*),
		std::string const& clsname,
		Descriptor const& desc)
	{
        if (nodeClasses.find(clsname) != nodeClasses.end()) {
            log_warn("node class redefined: `{}`\n", clsname);
            return -1;
        }

        CustomUI ui = descToCustomui(desc);
        auto cls = std::make_unique<ImplNodeClass>(ctor, dtor, ui, clsname);
        if (!clsname.empty() && clsname.front() == '^')
            return -1;

        NodeInfo info;
        info.module_path = m_current_loading_module;
        info.name = clsname;
        info.status = ZModule_Loaded;
        info.cate = cls->m_customui.category;

        m_cates.push_back(std::move(info));
        nodeClasses.emplace(clsname, std::move(cls));
        return 0;
	}

	int NodeRegister::registerNodeClass(
		INode2* (*ctor)(),
        void (*dtor)(INode2*),
		std::string const& nodecls,
		CustomUI const& customui)
	{
        if (nodeClasses.find(nodecls) != nodeClasses.end()) {
            log_error("node class redefined: `{}`\n", nodecls);
            return -1;
        }
        CustomUI ui = customui;
        initControlsByType(ui);
        auto cls = std::make_unique<ImplNodeClass>(ctor, dtor, ui, nodecls);

        NodeInfo info;
        info.module_path = m_current_loading_module;
        info.name = nodecls;
        info.status = ZModule_Loaded;
        info.cate = cls->m_customui.category;

        m_cates.push_back(std::move(info));
        nodeClasses.emplace(nodecls, std::move(cls));
        return 0;
	}

    int NodeRegister::registerNodeClass2(INode2* (*ctor)(), void (*dtor)(INode2*), std::string const& nodecls, const ZNodeDescriptor& desc) {
        if (!nodecls.empty() && nodecls.front() == '^')
            return -1;
        
        Descriptor stddesc;
        for (auto i = 0; i < desc.input_count; i++) {
            const ZParamDescriptor& param_desc = *(desc.inputs + i);
            const std::string& name = std::string(param_desc.name);
            ParamType type = param_desc.type;
            //TODO: constrain
            if (type == _gParamType_IObject ||
                type == _gParamType_Geometry ||
                type == _gParamType_List ||
                type == _gParamType_Dict ||
                type == _gParamType_VDBGrid)
            {
                stddesc.inputs.emplace_back(std::move(ParamObject(name, type)));
            }
            else {
                const auto& deflVal = zvalue2any(param_desc.defl);
                ParamControl ctrl = zctrl2ctrl(param_desc.control);
                const auto& ctrlProp = zvalue2any(param_desc.ctrl_props);
                stddesc.inputs.emplace_back(std::move(ParamPrimitive(name, type, deflVal, ctrl, ctrlProp, "", false)));
            }
        }

        for (auto i = 0; i < desc.output_count; i++) {
            const ZParamDescriptor& param_desc = *(desc.outputs + i);
            const std::string& name = std::string(param_desc.name);
            ParamType type = param_desc.type;
            if (type == _gParamType_IObject ||
                type == _gParamType_Geometry ||
                type == _gParamType_List ||
                type == _gParamType_Dict ||
                type == _gParamType_VDBGrid)
            {
                stddesc.outputs.emplace_back(std::move(ParamObject(name, type)));
            }
            else {
                const auto& deflVal = zvalue2any(param_desc.defl);
                ParamControl ctrl = zctrl2ctrl(param_desc.control);
                const auto& ctrlProp = zvalue2any(param_desc.ctrl_props);
                stddesc.outputs.emplace_back(std::move(ParamPrimitive(name, type, deflVal, ctrl, ctrlProp, "", false)));
            }
        }

        stddesc.displayName = std::string(desc.node_name);
        if (desc.doc)
            stddesc.doc = std::string(desc.doc);
        if (desc.icon)
            stddesc.iconResPath = std::string(desc.icon);
        if (desc.cate)
            stddesc.categories.push_back(std::string(desc.cate));

        CustomUI ui = descToCustomui(stddesc);
        auto cls = std::make_unique<ImplNodeClass>(ctor, dtor, ui, nodecls);

        NodeInfo info;
        info.module_path = m_current_loading_module;
        info.name = nodecls;
        info.status = ZModule_Loaded;
        info.cate = cls->m_customui.category;

        m_cates.push_back(std::move(info));
        nodeClasses.emplace(nodecls, std::move(cls));
        return 0;
    }

    ZENO_API void NodeRegister::registerObjUIInfo(size_t hashcode, std::string_view color, std::string_view nametip) {
        m_objsUIInfo.insert(std::make_pair(hashcode, _ObjUIInfo{ nametip, color }));
    }

    int NodeRegister::unregisterNodeClass(std::string const& nodecls) {
        auto iter = nodeClasses.find(nodecls);
        if (iter == nodeClasses.end()) {
            log_error("node class redefined: `{}`\n", nodecls);
            return -1;
        }
        nodeClasses.erase(iter);
        return 0;
    }

    void NodeRegister::clear() {
        nodeClasses.clear();
    }

    std::vector<NodeInfo> NodeRegister::dumpCoreCates() const {
        return m_cates;
    }

    zeno::CustomUI NodeRegister::getOfficalUIDesc(const std::string& clsname, bool& bExist) {
        if (nodeClasses.find(clsname) == nodeClasses.end()) {
            bExist = false;
            return zeno::CustomUI();
        }
        bExist = true;
        return nodeClasses[clsname]->m_customui;
    }

    INodeClass* NodeRegister::getNodeClassPtr(const std::string& name) {
        auto iter = nodeClasses.find(name);
        if (iter == nodeClasses.end()) {
            return nullptr;
        }
        return iter->second.get();
    }

    void NodeRegister::beginLoadModule(const std::string& module_name) {
        m_current_loading_module = module_name;
    }

    void NodeRegister::uninstallModule(const std::string& module_path) {
        for (NodeInfo& info : m_cates) {
            if (info.module_path == module_path) {
                info.status = ZModule_UnLoaded;
                const std::string uninstall_nodecls = info.name;
                nodeClasses.erase(uninstall_nodecls);
                //所有节点要disable掉，只留一个空壳
                //if (m_spMainGraph)
                //    m_spMainGraph->update_load_info(uninstall_nodecls, true);
            }
        }
    }

    void NodeRegister::endLoadModule() {
        for (NodeInfo& info : m_cates) {
            if (info.module_path == m_current_loading_module) {
                info.status = ZModule_Loaded;
                //if (m_spMainGraph)
                //    m_spMainGraph->update_load_info(info.name, false);
            }
        }
        m_current_loading_module = "";
    }

    ZENO_API bool NodeRegister::getObjUIInfo(size_t hashcode, std::string_view& color, std::string_view& nametip) {
        auto iter = m_objsUIInfo.find(hashcode);
        if (iter == m_objsUIInfo.end()) {
            color = "#000000";
            nametip = "unknown type";
            return false;
        }
        color = iter->second.color;
        nametip = iter->second.name;
        return true;
    }

}

