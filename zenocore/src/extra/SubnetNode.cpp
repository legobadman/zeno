#include <zeno/core/NodeImpl.h>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <zeno/core/INodeClass.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/types/DummyObject.h>
#include <zeno/utils/log.h>
#include <zeno/core/CoreParam.h>
#include <zeno/core/Assets.h>
#include <zeno/utils/helper.h>
#include "zeno_types/reflect/reflection.generated.hpp"
#include <zeno/zeno.h>
#include <zeno/funcs/ObjectCodec.h>
#include <zeno/utils/Timer.h>
#include <zeno/types/MaterialObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/interfaceutil.h>


namespace zeno {

SubnetNode::SubnetNode(INode* pNode)
    : NodeImpl(pNode)
    , m_subgraph(std::make_shared<Graph>(""))
{
    //auto cl = safe_at(getSession().nodeClasses, "Subnet", "node class name").get();
    //m_customUi = cl->m_customui;
    //添加一些default的输入输出
    zeno::ParamTab tab;
    zeno::ParamGroup default_group;

    zeno::ParamUpdateInfo info;

    zeno::ParamPrimitive param;
    param.bInput = true;
    param.name = "data_input";
    param.defl = zeno::reflect::make_any<zeno::PrimVar>(zeno::PrimVar(0));;
    param.type = zeno::types::gParamType_Int;
    param.socketType = zeno::Socket_Primitve;
    param.control = zeno::Lineedit;
    param.bSocketVisible = false;
    info.param = param;
    default_group.params.push_back(param);

    zeno::ParamPrimitive outputparam;
    outputparam.bInput = false;
    outputparam.name = "data_output";
    outputparam.defl = 2;
    outputparam.type = gParamType_Int;
    outputparam.socketType = zeno::Socket_Primitve;
    outputparam.bSocketVisible = false;
    info.param = outputparam;

    zeno::ParamObject objInput;
    objInput.bInput = true;
    objInput.name = "Input";
    objInput.type = gParamType_Geometry;

    zeno::ParamObject objOutput;
    objOutput.bInput = false;
    objOutput.name = "Output";
    objOutput.type = gParamType_Geometry;
    objOutput.socketType = zeno::Socket_Output;

    tab.groups.emplace_back(std::move(default_group));
    m_customUi.inputPrims.emplace_back(std::move(tab));
    m_customUi.inputObjs.push_back(objInput);
    m_customUi.outputPrims.push_back(outputparam);
    m_customUi.outputObjs.push_back(objOutput);

    m_customUi.uistyle.background = "#1D5F51";
    m_customUi.uistyle.iconResPath = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">    <path d=\"M4 7H12L15 10H28V25H4V7Z\" stroke=\"#CCCCCC\" stroke-width=\"2\" stroke-linejoin=\"round\"/>    <line x1=\"4\" y1=\"14\" x2=\"28\" y2=\"14\" stroke=\"#CCCCCC\" stroke-width=\"2\"/></svg>";
}

SubnetNode::~SubnetNode() = default;

void SubnetNode::initParams(const NodeData& dat)
{
    NodeImpl::initParams(dat);
    if (dat.subgraph)
        m_subgraph->init(*dat.subgraph);
}

Graph* SubnetNode::get_subgraph() const
{
    return m_subgraph.get();
}

void SubnetNode::init_graph(std::shared_ptr<Graph> subg) {
    m_subgraph = subg;
}

bool SubnetNode::isAssetsNode() const {
    return m_subgraph->isAssets();
}

params_change_info SubnetNode::update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit)
{
    params_change_info changes = NodeImpl::update_editparams(params);
    //update subnetnode.
    if (!m_subgraph->isAssets()) {
        for (auto name : changes.new_inputs) {
            NodeImpl* newNode = m_subgraph->createNode("SubInput", name);

            if (bSubnetInit) {
                if (name == "data_input") {
                    newNode->set_pos({700, 0});
                }
                else if (name == "Input") {
                    newNode->set_pos({0, 0});
                }
            }

            //这里SubInput的类型其实是和Subnet节点创建预设的参数对应，参考AddNodeCommand

            bool exist;     //subnet通过自定义参数面板创建SubInput节点时，根据实际情况添加primitive/obj类型的port端口
            bool isprim = isPrimitiveType(true, name, exist);
            if (isprim) {
                zeno::ParamPrimitive primitive;
                primitive.bInput = false;
                primitive.name = "port";
                primitive.socketType = Socket_Output;
                newNode->add_output_prim_param(primitive);
            }
            else if (!isprim && exist) {
                zeno::ParamObject paramObj;
                paramObj.bInput = false;
                paramObj.name = "port";
                paramObj.type = gParamType_Geometry;
                paramObj.socketType = Socket_Output;
                newNode->add_output_obj_param(paramObj);
            }

            for (const auto& [param, _] : params) {     //创建Subinput时,更新Subinput的port接口类型
                if (auto paramPrim = std::get_if<ParamPrimitive>(&param)) {
                    if (name == paramPrim->name) {
                        newNode->update_param_type("port", true, false, paramPrim->type);
                        break;
                    }
                }
            }
            params_change_info changes;
            changes.new_outputs.insert("port");
            changes.outputs.push_back("port");
            changes.outputs.push_back("hasValue");
            newNode->update_layout(changes);
        }
        for (const auto& [old_name, new_name] : changes.rename_inputs) {
            m_subgraph->updateNodeName(old_name, new_name);
        }
        for (auto name : changes.remove_inputs) {
            m_subgraph->removeNode(name);
        }

        for (auto name : changes.new_outputs) {
            NodeImpl* newNode = m_subgraph->createNode("SubOutput", name);

            if (bSubnetInit) {
                if (name == "data_output") {
                    newNode->set_pos({ 700, 500 });
                }
                else if (name == "Output") {
                    newNode->set_pos({ 0, 500 });
                    //newNode->set_view(true);
                }
            }

            bool exist;
            bool isprim = isPrimitiveType(false, name, exist);
            if (isprim) {
                zeno::ParamPrimitive primitive;
                primitive.bInput = true;
                primitive.name = "port";
                primitive.type = gParamType_Int;
                primitive.socketType = Socket_Primitve;
                primitive.defl = 0;
                newNode->add_input_prim_param(primitive);
            }
            else if (!isprim && exist) {
                zeno::ParamObject paramObj;
                paramObj.bInput = true;
                paramObj.name = "port";
                paramObj.type = gParamType_Geometry;
                paramObj.socketType = Socket_Clone;
                newNode->add_input_obj_param(paramObj);
            }
            params_change_info changes;
            changes.new_inputs.insert("port");
            changes.inputs.push_back("port");
            newNode->update_layout(changes);
        }
        for (const auto& [old_name, new_name] : changes.rename_outputs) {
            m_subgraph->updateNodeName(old_name, new_name);
        }
        for (auto name : changes.remove_outputs) {
            m_subgraph->removeNode(name);
        }
    }
    //prim的输入类型变化时，可能需要更新对应subinput节点port端口的类型
    for (auto _pair : params) {
        if (const auto& pParam = std::get_if<ParamPrimitive>(&_pair.param)) {
            const ParamPrimitive& param = *pParam;
            if (param.bInput && 
                changes.new_inputs.find(param.name) == changes.new_inputs.end() && 
                changes.remove_inputs.find(param.name) == changes.remove_inputs.end()) {
                auto inputnode = m_subgraph->getNode(param.name);
                if (inputnode) {
                    ParamType paramtype;
                    SocketType socketype;
                    bool _wildcard;
                    inputnode->getParamTypeAndSocketType("port", true, false, paramtype, socketype, _wildcard);
                    if (paramtype != param.type) {
                        inputnode->update_param_type("port", true, false, param.type);
                        for (auto& link : inputnode->getLinksByParam(false, "port")) {
                            if (auto linktonode = m_subgraph->getNode(link.inNode)) {
                                ParamType paramType;
                                SocketType socketType;
                                bool bWildcard = false;
                                linktonode->getParamTypeAndSocketType(link.inParam, true, true, paramType, socketType, bWildcard);
                                if (bWildcard) {
                                    m_subgraph->updateWildCardParamTypeRecursive(m_subgraph.get(), linktonode, link.inParam, true, true, param.type);
                                } else if (!outParamTypeCanConvertInParamType(param.type, paramType, Role_OutputPrimitive, Role_InputPrimitive)) {
                                    m_subgraph->removeLink(link);
                                }
                            }
                        }
                        for (auto& link : getLinksByParam(true, param.name)) {
                            if (auto spgraph = m_pGraph) {
                                if (auto linktonode = spgraph->getNode(link.outNode)) {
                                    ParamType paramType;
                                    SocketType socketType;
                                    bool bWildcard = false;
                                    linktonode->getParamTypeAndSocketType(link.outParam, true, false, paramType, socketType, bWildcard);
                                    if (bWildcard) {
                                        spgraph->updateWildCardParamTypeRecursive(spgraph, linktonode, link.outParam, true, false, param.type);
                                    } else if (!outParamTypeCanConvertInParamType(paramType, param.type, Role_OutputPrimitive, Role_InputPrimitive)) {
                                        spgraph->removeLink(link);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return changes;
}

void SubnetNode::mark_subnetdirty(bool bOn)
{
    if (bOn) {
        m_subgraph->markDirtyAll();
    }
}

void SubnetNode::apply() {
    for (auto const &subinput_node: m_subgraph->getSubInputs()) {
        auto subinput = m_subgraph->getNode(subinput_node);
        auto iter = m_inputObjs.find(subinput_node);
        if (iter != m_inputObjs.end()) {
            //object type.
            if (iter->second.spObject) {
                //要拷贝一下才能赋值到SubInput的port参数
                zany spObject = iter->second.spObject->clone();
                spObject->update_key(stdString2zs(subinput->get_uuid_path()));
                bool ret = subinput->set_output("port", spObject);
                assert(ret);
                ret = subinput->set_output("hasValue", std::make_shared<NumericObject>(true));
                assert(ret);
            }
        }
        else {
            //primitive type
            auto iter2 = m_inputPrims.find(subinput_node);
            if (iter2 != m_inputPrims.end()) {
                bool ret = subinput->set_primitive_output("port", iter2->second.result);
                assert(ret);
                ret = subinput->set_output("hasValue", std::make_shared<NumericObject>(true));
                assert(ret);
            }
            else {
                subinput->set_output("port", std::make_shared<DummyObject>());
                subinput->set_output("hasValue", std::make_shared<NumericObject>(false));
            }
        }
    }

    std::set<std::string> nodesToExec;
    for (auto const &suboutput_node: m_subgraph->getSubOutputs()) {
        nodesToExec.insert(suboutput_node);
    }
    m_subgraph->applyNodes(nodesToExec);

    //TODO: 多输出其实是一个问题，不知道view哪一个，所以目前先规定子图只能有一个输出
    auto suboutputs = m_subgraph->getSubOutputs();
    bool bSetOutput = false;
    for (auto const &suboutput_node: suboutputs) {
        auto suboutput = m_subgraph->getNode(suboutput_node);
        //suboutput的结果是放在Input的port上面（因为Suboutput放一个输出参数感觉怪怪的）
        bool bPrimoutput = suboutput->get_input_object_params().empty();
        zany result = suboutput->get_input("port");
        if (auto numobj = std::dynamic_pointer_cast<NumericObject>(result)) {
            int j;
            j = 0;
        }
        if (result) {
            bSetOutput = true;
            zany spObject = result->clone();
            if (!bPrimoutput) {
                spObject->update_key(stdString2zs(get_uuid_path()));
            }
            bool ret = set_output(suboutput_node, spObject);
            assert(ret);
        }
    }
}

NodeData SubnetNode::exportInfo() const {
    //要注意，这里必须要手动cast为SubnetNode才能拿，因为NodeImpl已经和INode分离了
    NodeData node = NodeImpl::exportInfo();
    const Asset& asset = zeno::getSession().assets->getAsset(node.cls);
    if (!asset.m_info.name.empty()) {
        node.asset = asset.m_info;
        node.type = Node_AssetInstance;
    }
    else {
        node.subgraph = m_subgraph->exportGraph();
        node.type = Node_SubgraphNode;
    }
    //node.customUi = m_customUi;
    return node;
}

CustomUI SubnetNode::get_customui() const
{
    return m_customUi;
}

CustomUI SubnetNode::export_customui() const {
    CustomUI exportCustomui = m_customUi;
    if (m_subgraph) {
        for (auto& tab : exportCustomui.inputPrims) {
            for (auto& group : tab.groups) {
                for (auto& param : group.params) {
                    if (auto node = m_subgraph->getNode(param.name)) {
                        ParamType type;
                        SocketType socketype;
                        bool _wildcard;
                        node->getParamTypeAndSocketType("port", true, false, type, socketype, _wildcard);
                        param.type = type;
                    }
                }
            }
        }
        for (auto& param : exportCustomui.inputObjs) {
            if (auto node = m_subgraph->getNode(param.name)) {
                ParamType type;
                SocketType socketype;
                bool _wildcard;
                node->getParamTypeAndSocketType("port", false, false, type, socketype, _wildcard);
                param.type = type;
            }
        }
        for (auto& param : exportCustomui.outputPrims) {
            if (auto node = m_subgraph->getNode(param.name)) {
                ParamType type;
                SocketType socketype;
                bool _wildcard;
                node->getParamTypeAndSocketType("port", true, true, type, socketype, _wildcard);
                param.type = type;
            }
        }
        for (auto& param : exportCustomui.outputObjs) {
            if (auto node = m_subgraph->getNode(param.name)) {
                ParamType type;
                SocketType socketype;
                bool _wildcard;
                node->getParamTypeAndSocketType("port", false, true, type, socketype, _wildcard);
                param.type = type;
            }
        }
    }
    return exportCustomui;
}

void SubnetNode::setCustomUi(const CustomUI& ui)
{
    m_customUi = ui;
    //保证颜色图标
    m_customUi.uistyle.background = "#1D5F51";
    m_customUi.uistyle.iconResPath = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">    <path d=\"M4 7H12L15 10H28V25H4V7Z\" stroke=\"#CCCCCC\" stroke-width=\"2\" stroke-linejoin=\"round\"/>    <line x1=\"4\" y1=\"14\" x2=\"28\" y2=\"14\" stroke=\"#CCCCCC\" stroke-width=\"2\"/></svg>";
}


//TODO：整理DopNetWork，现在暂时不可用，只保证编译
DopNetwork::DopNetwork() 
    : SubnetNode(nullptr)
    , m_bEnableCache(true)
    , m_bAllowCacheToDisk(false)
    , m_maxCacheMemoryMB(5000)
    , m_currCacheMemoryMB(5000)
    , m_totalCacheSizeByte(0)
{
}

void DopNetwork::apply()
{
    auto& sess = zeno::getSession();
    int startFrame = sess.globalState->getStartFrame();
    int currentFarme = sess.globalState->getFrameId();
    zeno::scope_exit sp([&currentFarme, &sess]() {
        sess.globalState->updateFrameId(currentFarme);
    });
    //重新计算
    for (int i = startFrame; i <= currentFarme; i++) {
        if (m_frameCaches.find(i) == m_frameCaches.end()) {
            sess.globalState->updateFrameId(i);
            m_subgraph->markDirtyAll();
            zeno::SubnetNode::apply();

            const ObjectParams& outputObjs = get_output_object_params();
            size_t currentFrameCacheSize = 0;
            for (auto const& objparam : outputObjs) {
                currentFrameCacheSize += getObjSize(get_output_obj(objparam.name).get());
            }
            while (((m_totalCacheSizeByte + currentFrameCacheSize) / 1024 / 1024) > m_currCacheMemoryMB) {
                if (!m_frameCaches.empty()) {
                    auto lastIter = --m_frameCaches.end();
                    if (lastIter->first > currentFarme) {//先从最后一帧删

                        m_totalCacheSizeByte = m_totalCacheSizeByte - m_frameCacheSizes[lastIter->first];
                        CALLBACK_NOTIFY(dopnetworkFrameRemoved, lastIter->first)
                        m_frameCaches.erase(lastIter);
                        m_frameCacheSizes.erase(--m_frameCacheSizes.end());
                    } else {
                        m_totalCacheSizeByte = m_totalCacheSizeByte - m_frameCacheSizes.begin()->second;
                        CALLBACK_NOTIFY(dopnetworkFrameRemoved, m_frameCaches.begin()->first)
                        m_frameCaches.erase(m_frameCaches.begin());
                        m_frameCacheSizes.erase(m_frameCacheSizes.begin());
                    }
                }
                else {
                    break;
                }
            }
            for (auto const& objparam : outputObjs) {
                m_frameCaches[i].insert({ objparam.name, get_output_obj(objparam.name) });
            }
            m_frameCacheSizes[i] = currentFrameCacheSize;
            m_totalCacheSizeByte += currentFrameCacheSize;
            CALLBACK_NOTIFY(dopnetworkFrameCached, i)
        }
        else {
            if (i == currentFarme) {
                for (auto const& [name, obj] : m_frameCaches[i]) {
                    if (obj) {
                        bool ret = set_output(name, obj);
                        assert(ret);
                    }
                }
            }
        }
    }
}

void DopNetwork::setEnableCache(bool enable)
{
    m_bEnableCache = enable;
}

void DopNetwork::setAllowCacheToDisk(bool enable)
{
    m_bAllowCacheToDisk = enable;
}

void DopNetwork::setMaxCacheMemoryMB(int size)
{
    m_maxCacheMemoryMB = size;
}

void DopNetwork::setCurrCacheMemoryMB(int size)
{
    m_currCacheMemoryMB = size;
}

template <class T0>
size_t getAttrVectorSize(zeno::AttrVector<T0> const& arr) {
    size_t totalSize = 0;
    totalSize += sizeof(arr);
    totalSize += sizeof(T0) * arr.values.size();
    for (const auto& pair : arr.attrs) {
        totalSize += sizeof(pair.first) + pair.first.capacity();
        std::visit([&totalSize](auto& val) {
            if (!val.empty()) {
                using T = std::decay_t<decltype(val[0])>;
                totalSize += sizeof(T) * val.size();
            }
        }, pair.second);
    }
    return totalSize;
};

size_t DopNetwork::getObjSize(IObject* obj)
{
    size_t totalSize = 0;
    if (PrimitiveObject* primobj = dynamic_cast<PrimitiveObject*>(obj)) {
        totalSize += sizeof(*primobj);
        totalSize += getAttrVectorSize(primobj->verts);
        totalSize += getAttrVectorSize(primobj->points);
        totalSize += getAttrVectorSize(primobj->lines);
        totalSize += getAttrVectorSize(primobj->tris);
        totalSize += getAttrVectorSize(primobj->quads);
        totalSize += getAttrVectorSize(primobj->loops);
        totalSize += getAttrVectorSize(primobj->polys);
        totalSize += getAttrVectorSize(primobj->edges);
        totalSize += getAttrVectorSize(primobj->uvs);
        if (MaterialObject* mtlPtr = primobj->mtl.get()) {
            totalSize += sizeof(*mtlPtr);
            totalSize += mtlPtr->serializeSize();
        }
        if (InstancingObject* instPtr = primobj->inst.get()) {
            totalSize += sizeof(*instPtr);
            totalSize += instPtr->serializeSize();
        }
    }
    else if (CameraObject* camera = dynamic_cast<CameraObject*>(obj)) {
        totalSize += sizeof(*camera);
        totalSize += sizeof(CameraData);
    }
    else if (LightObject* light = dynamic_cast<LightObject*>(obj)) {
        totalSize += sizeof(*light);
        totalSize += sizeof(LightData);
    }
    else if (MaterialObject* matobj = dynamic_cast<MaterialObject*>(obj)) {
        totalSize += sizeof(*matobj);
        totalSize += matobj->serializeSize();
    }
    else if (ListObject* list = dynamic_cast<ListObject*>(obj)) {
        totalSize += sizeof(*list);
        totalSize += list->m_impl->dirtyIndiceSize() * sizeof(int);
        for (int i = 0; i > list->m_impl->size(); i++) {
            totalSize += getObjSize(list->m_impl->get(i).get());
        }
    }
    else {//dummy obj
    }
    return totalSize;
}

void DopNetwork::resetFrameState()
{
    for (auto& [idx, _] : m_frameCaches) {
        CALLBACK_NOTIFY(dopnetworkFrameRemoved, idx)
    }
    m_frameCaches.clear();
    m_frameCacheSizes.clear();
}

#if 0 //TODO
ZENDEFNODE(DopNetwork, {
    {},
    {},
    {},
    {"dop"},
});
#endif

}
