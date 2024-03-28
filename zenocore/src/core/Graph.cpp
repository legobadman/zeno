#include <zeno/core/Graph.h>
#include <zeno/core/INode.h>
#include <zeno/core/IObject.h>
#include <zeno/core/Session.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/scope_exit.h>
#include <zeno/core/Descriptor.h>
#include <zeno/core/Assets.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/extra/GraphException.h>
#include <zeno/funcs/LiterialConverter.h>
#include <zeno/extra/GlobalError.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/extra/DirtyChecker.h>
#include <zeno/utils/Error.h>
#include <zeno/utils/log.h>
#include <zeno/core/IParam.h>
#include <zeno/utils/uuid.h>
#include <zeno/utils/helper.h>
#include <iostream>
#include <regex>


namespace zeno {

ZENO_API Context::Context() = default;
ZENO_API Context::~Context() = default;

ZENO_API Context::Context(Context const &other)
    : visited(other.visited)
{}

ZENO_API Graph::Graph(const std::string& name, bool bAssets) : m_name(name), m_bAssets(bAssets) {
    
}

ZENO_API Graph::~Graph() {

}

ZENO_API zany Graph::getNodeInput(std::string const& sn, std::string const& ss) const {
    //todo: deprecated
    auto node = safe_at(m_nodes, sn, "node name").get();
    return node->get_input(ss);
}

ZENO_API void Graph::clearNodes() {
    m_nodes.clear();
}

ZENO_API void Graph::addNode(std::string const &cls, std::string const &id) {
    //todo: deprecated.
#if 0
    if (nodes.find(id) != nodes.end())
        return;  // no add twice, to prevent output object invalid
    auto cl = safe_at(session->nodeClasses, cls, "node class name").get();
    auto node = cl->new_instance(id);
    node->graph = this;
    node->name = id;
    node->nodeClass = cl;
    nodes[id] = std::move(node);
#endif
}

ZENO_API Graph *Graph::getSubnetGraph(std::string const & node_name) const {
    const std::string uuid = safe_at(m_name2uuid, node_name, "uuid");
    auto node = static_cast<SubnetNode *>(safe_at(m_nodes, uuid, "node name").get());
    return node->subgraph.get();
}

ZENO_API void Graph::completeNode(std::string const &node_name) {
    const std::string uuid = safe_at(m_name2uuid, node_name, "uuid");
    safe_at(m_nodes, uuid, "node name")->doComplete();
}

ZENO_API bool Graph::applyNode(std::string const &node_name) {
    const std::string uuid = safe_at(m_name2uuid, node_name, "uuid");
    auto node = safe_at(m_nodes, uuid, "node name").get();
    GraphException::translated([&] {
        if (getSession().is_auto_run())
            node->doOnlyApply();
        else
            node->doApply();
    }, node);
    return true;
}

ZENO_API void Graph::applyNodes(std::set<std::string> const &nodes) {
    ctx = std::make_unique<Context>();

    scope_exit _{[&] {
        ctx = nullptr;
    }};

    for (auto const& node_name: nodes) {
        applyNode(node_name);
    }
}

ZENO_API void Graph::runGraph() {
    log_debug("{} nodes to exec", m_viewnodes.size());
    applyNodes(m_viewnodes);
}

void Graph::onNodeParamUpdated(std::shared_ptr<IParam> spParam, zvariant old_value, zvariant new_value) {
    assert(spParam);
    if (Param_String == spParam->type) {
        auto spNode = spParam->m_wpNode.lock();
        assert(spNode);

        const std::string& nodecls = spNode->get_nodecls();
        const std::string& uuid = spNode->get_uuid();

        std::string oldstr, newstr;
        if (std::holds_alternative<std::string>(old_value))
            oldstr = std::get<std::string>(old_value);
        if (std::holds_alternative<std::string>(new_value))
            newstr = std::get<std::string>(new_value);

        frame_nodes.erase(uuid);

        std::regex pattern("\\$F");
        if (std::regex_search(newstr, pattern, std::regex_constants::match_default)) {
            frame_nodes.insert(uuid);
        }
    }
    else if (Param_Vec2f == spParam->type) {
        auto spNode = spParam->m_wpNode.lock();
        assert(spNode);
        const std::string& uuid = spNode->get_uuid();
        frame_nodes.erase(uuid);
        if (std::holds_alternative<vec2s>(new_value)) {
            auto vec = std::get<vec2s>(new_value);
            std::regex pattern("\\$F");
            for (auto val : vec) {
                if (std::regex_search(val, pattern)) {
                    frame_nodes.insert(uuid);
                }
            }
        }
    }
    else if (Param_Vec3f == spParam->type) {
        auto spNode = spParam->m_wpNode.lock();
        assert(spNode);
        const std::string& uuid = spNode->get_uuid();
        frame_nodes.erase(uuid);
        if (std::holds_alternative<vec3s>(new_value)) {
            auto vec = std::get<vec3s>(new_value);
            std::regex pattern("\\$F");
            for (auto val : vec) {
                if (std::regex_search(val, pattern)) {
                    frame_nodes.insert(uuid);
                }
            }
        }
    }
    else if (Param_Vec4f == spParam->type) {
        auto spNode = spParam->m_wpNode.lock();
        assert(spNode);
        const std::string& uuid = spNode->get_uuid();
        frame_nodes.erase(uuid);
        if (std::holds_alternative<vec4s>(new_value)) {
            auto vec = std::get<vec4s>(new_value);
            std::regex pattern("\\$F");
            for (auto val : vec) {
                if (std::regex_search(val, pattern)) {
                    frame_nodes.insert(uuid);
                }
            }
        }
    }
}

void Graph::viewNodeUpdated(const std::string node, bool bView) {
    if (bView) {
        //TODO: only run calculation chain which associate with `node`.
        //getSession().run_main_graph();
        m_viewnodes.insert(node);
    }
    else {
        m_viewnodes.erase(node);
        //TODO: update objsmanager to hide objs.
    }
}

ZENO_API void Graph::bindNodeInput(std::string const &dn, std::string const &ds,
        std::string const &sn, std::string const &ss) {
    //safe_at(nodes, dn, "node name")->inputBounds[ds] = std::pair(sn, ss);
}

ZENO_API void Graph::setNodeInput(std::string const &id, std::string const &par,
        zany const &val) {
    //todo: deprecated.
    //safe_at(nodes, id, "node name")->inputs[par] = val;
}

ZENO_API void Graph::setKeyFrame(std::string const &id, std::string const &par, zany const &val) {
    //todo: deprecated.
    /*
    safe_at(nodes, id, "node name")->inputs[par] = val;
    safe_at(nodes, id, "node name")->kframes.insert(par);
    */
}

ZENO_API void Graph::setFormula(std::string const &id, std::string const &par, zany const &val) {
    //todo: deprecated.
    /*
    safe_at(nodes, id, "node name")->inputs[par] = val;
    safe_at(nodes, id, "node name")->formulas.insert(par);
    */
}


ZENO_API std::map<std::string, zany> Graph::callSubnetNode(std::string const &id,
        std::map<std::string, zany> inputs) const {
    //todo: deprecated.
    return std::map<std::string, zany>();
}

ZENO_API std::map<std::string, zany> Graph::callTempNode(std::string const &id,
        std::map<std::string, zany> inputs) {

    auto cl = safe_at(getSession().nodeClasses, id, "node class name").get();
    const std::string& name = generateUUID();
    auto se = cl->new_instance(this, name);
    se->graph = const_cast<Graph*>(this);
    se->directly_setinputs(inputs);
    se->doOnlyApply();
    return se->getoutputs();
}

ZENO_API void Graph::addNodeOutput(std::string const& id, std::string const& par) {
    // add "dynamic" output which is not descriped by core.
    //todo: deprecated.
    //safe_at(nodes, id, "node name")->outputs[par] = nullptr;
}

ZENO_API void Graph::setNodeParam(std::string const &id, std::string const &par,
    std::variant<int, float, std::string, zany> const &val) {
    auto parid = par + ":";
    std::visit([&] (auto const &val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, zany>) {
            setNodeInput(id, parid, val);
        } else {
            setNodeInput(id, parid, objectFromLiterial(val));
        }
    }, val);
}

ZENO_API DirtyChecker &Graph::getDirtyChecker() {
    if (!dirtyChecker) 
        dirtyChecker = std::make_unique<DirtyChecker>();
    return *dirtyChecker;
}

ZENO_API void Graph::init(const GraphData& graph) {
    auto& sess = getSession();
    sess.setApiLevelEnable(false);
    zeno::scope_exit([&]() {
        sess.setApiLevelEnable(true);
    });

    m_name = graph.name;
    //import nodes first.
    for (const auto& [name, node] : graph.nodes) {
        std::string cate = node.asset.has_value() ? "assets" : "";
        std::shared_ptr<INode> spNode = createNode(node.cls, name, cate);
        spNode->init(node);
        if (node.cls == "SubInput") {
            //TODO
        }
        else if (node.cls == "SubOutput") {
            //TODO
        }
    }
    //import edges
    for (const auto& link : graph.links) {
        std::shared_ptr<INode> outNode = getNode(link.outNode);
        std::shared_ptr<INode> inNode = getNode(link.inNode);
        assert(outNode && inNode);

        std::shared_ptr<IParam> outParam = outNode->get_output_param(link.outParam);
        if (!outParam) {
            zeno::log_warn("no output param `{}` on node `{}`", link.outParam, link.outNode);
            continue;
        }

        std::shared_ptr<IParam> inParam = inNode->get_input_param(link.inParam);
        if (!inParam) {
            zeno::log_warn("no input param `{}` on node `{}`", link.inParam, link.inNode);
            continue;
        }

        std::shared_ptr<ILink> spLink = std::make_shared<zeno::ILink>();
        spLink->fromparam = outParam;
        spLink->toparam = inParam;
        spLink->fromkey = link.outKey;
        spLink->tokey = link.inKey;
        spLink->lnkProp = link.lnkfunc;
        outParam->links.emplace_back(spLink);
        inParam->links.emplace_back(spLink);
    }
}

void Graph::markDirtyWhenFrameChanged()
{
    for (const std::string& uuid : frame_nodes) {
        m_nodes[uuid]->mark_dirty(true);
    }
    std::set<std::string> nodes = subnet_nodes;
    nodes.insert(asset_nodes.begin(), asset_nodes.end());
    for (const std::string& uuid : nodes) {
        auto spSubnetNode = std::dynamic_pointer_cast<SubnetNode>(m_nodes[uuid]);
        spSubnetNode->subgraph->markDirtyWhenFrameChanged();
    }
}

std::string Graph::generateNewName(const std::string& node_cls)
{
    if (node_set.find(node_cls) == node_set.end())
        node_set.insert(std::make_pair(node_cls, std::set<std::string>()));

    auto& nodes = node_set[node_cls];
    int i = 1;
    while (true) {
        std::string new_name = node_cls + std::to_string(i++);
        if (nodes.find(new_name) == nodes.end()) {
            nodes.insert(new_name);
            return new_name;
        }
    }
    return "";
}

std::string Graph::generateNewName(const std::string& node_cls, std::string specific_name)
{
    if (node_set.find(node_cls) == node_set.end())
        node_set.insert(std::make_pair(node_cls, std::set<std::string>()));

    std::string new_name = specific_name;
    auto& nodes = node_set[node_cls];
    int i = 1;
    while (true) {
        
        if (nodes.find(new_name) == nodes.end()) {
            nodes.insert(new_name);
            return new_name;
        }
        new_name = specific_name + "(" + std::to_string(i++) + ")";
    }
    return "";
}

ZENO_API bool Graph::isAssets() const
{
    return m_bAssets;
}

ZENO_API std::set<std::string> Graph::searchByClass(const std::string& name) const
{
    auto it = node_set.find(name);
    if (it == node_set.end())
        return {};
    return it->second;
}

ZENO_API std::string Graph::updateNodeName(const std::string oldName, const std::string newName)
{
    if (oldName == newName)
        return "";

    CORE_API_BATCH

    auto sync_to_set = [=](std::set<std::string>& nodes, std::string oldName, std::string newName) {
        if (nodes.find(oldName) != nodes.end()) {
            nodes.erase(oldName);
            nodes.insert(newName);
        }
    };

    if (newName.empty())
        return "";

    const std::string uuid = safe_at(m_name2uuid, oldName, "uuid");
    if (m_nodes.find(uuid) == m_nodes.end()) {
        return "";
    }

    auto spNode = m_nodes[uuid];
    std::string name = newName;
    if (m_name2uuid.find(name) != m_name2uuid.end()) {
        name = generateNewName(spNode->get_nodecls());
    }
    spNode->set_name(name);

    m_name2uuid[name] = m_name2uuid[oldName];
    m_name2uuid.erase(oldName);

    sync_to_set(m_viewnodes, oldName, name);

    //sync_to_set(frame_nodes, oldName, newName);
    //sync_to_set(subnet_nodes, oldName, newName);
    //sync_to_set(asset_nodes, oldName, newName);
    //sync_to_set(subinput_nodes, oldName, newName);
    //sync_to_set(suboutput_nodes, oldName, newName);

    CALLBACK_NOTIFY(updateNodeName, oldName, name)
    return name;
}

ZENO_API void Graph::clear()
{
    m_nodes.clear();
    m_name2uuid.clear();
    nodesToExec.clear();
    node_set.clear();
    optParentSubgNode = std::nullopt;
    ctx.reset();
    dirtyChecker.reset();
    portalIns.clear();
    portals.clear();
    //m_name = "";  keep name.

    CALLBACK_NOTIFY(clear)
}

ZENO_API std::shared_ptr<INode> Graph::createNode(std::string const& cls, std::string name, std::string cate, std::pair<float, float> pos)
{
    CORE_API_BATCH

    if (name.empty())
        name = generateNewName(cls);
    else
        name = generateNewName(cls, name);

    std::string uuid;
    std::shared_ptr<INode> node;
    if (cate != "assets") {
        auto& nodeClass = getSession().nodeClasses;
        std::string nodecls = cls;
        auto it = nodeClass.find(nodecls);
        if (it == nodeClass.end()) {
            nodecls = "DeprecatedNode";
        }
        auto cl = safe_at(getSession().nodeClasses, nodecls, "node class name").get();
        node = cl->new_instance(this, name);
        node->nodeClass = cl;
        uuid = node->get_uuid();
    }
    else {
        bool isCurrentGraphAsset = getSession().assets->isAssetGraph(shared_from_this());
        node = getSession().assets->newInstance(this, cls, name, isCurrentGraphAsset);
        uuid = node->get_uuid();
        asset_nodes.insert(uuid);
    }

    if (cls == "GetFrameNum") {
        frame_nodes.insert(uuid);
    }
    if (cls == "Subnet") {
        subnet_nodes.insert(uuid);
    }
    if (cls == "SubInput") {
        subinput_nodes.insert(uuid);
    }
    if (cls == "SubOutput") {
        suboutput_nodes.insert(uuid);
    }

    node->graph = this;
    node->m_pos = pos;
    node->mark_dirty(true);
    m_name2uuid[name] = uuid;
    m_nodes[uuid] = node;

    CALLBACK_NOTIFY(createNode, name, node)
    return node;
}

ZENO_API std::shared_ptr<INode> Graph::createSubnetNode(std::string const& cls)
{
    //todo: deprecated
    auto subcl = std::make_unique<ImplSubnetNodeClass>();
    std::string const& name = generateNewName(cls);
    auto node = subcl->new_instance(this, name);
    node->graph = this;
    node->nodeClass = subcl.get();

    auto subnetnode = std::dynamic_pointer_cast<SubnetNode>(node);
    //subnetnode->subnetClass = std::move(subcl);

    m_nodes[node->get_uuid()] = node;
    CALLBACK_NOTIFY(createSubnetNode, subnetnode)
    return node;
}

ZENO_API Graph* Graph::addSubnetNode(std::string const& id) {
    //deprecated:
    return nullptr;
#if 0
    auto subcl = std::make_unique<ImplSubnetNodeClass>();
    auto node = subcl->new_instance(id);
    node->graph = this;
    node->name = id;
    node->nodeClass = subcl.get();
    auto subnode = static_cast<SubnetNode*>(node.get());
    subnode->subgraph->session = this->session;
    subnode->subnetClass = std::move(subcl);
    auto subg = subnode->subgraph.get();
    nodes[id] = std::move(node);
    return subg;
#endif
}

std::set<std::string> Graph::getSubInputs()
{
    std::set<std::string> inputs;
    for (auto&[name, uuid] : m_name2uuid)
    {
        if (subinput_nodes.find(uuid) != subinput_nodes.end())
            inputs.insert(name);
    }
    return inputs;
}

std::set<std::string> Graph::getSubOutputs()
{
    std::set<std::string> outputs;
    for (auto& [name, uuid] : m_name2uuid)
    {
        if (suboutput_nodes.find(uuid) != suboutput_nodes.end())
            outputs.insert(name);
    }
    return outputs;
}

ZENO_API std::shared_ptr<INode> Graph::getNode(std::string const& name) {
    if (m_name2uuid.find(name) == m_name2uuid.end()) {
        return nullptr;
    }
    const std::string& uuid = m_name2uuid[name];
    return safe_at(m_nodes, uuid, "");
}

ZENO_API std::shared_ptr<INode> Graph::getNode(ObjPath path) {
    if (path.empty())
        return nullptr;

    std::string uuid = path.front();
    auto it = m_nodes.find(uuid);
    if (it == m_nodes.end()) {
        return nullptr;
    }
    path.pop_front();

    if (!path.empty())
    {
        //subnet
        if (std::shared_ptr<SubnetNode> subnetNode = std::dynamic_pointer_cast<SubnetNode>(it->second))
        {
            return subnetNode->graph->getNode(path);
        }
    }
    return it->second;
}

ZENO_API std::map<std::string, std::shared_ptr<INode>> Graph::getNodes() const {
    return m_nodes;
}

ZENO_API GraphData Graph::exportGraph() const {
    GraphData graph;
    graph.name = m_name;
    if ("main" == graph.name) {
        graph.type = Subnet_Main;
    }
    else {
        graph.type = Subnet_Normal;
    }

    for (auto& [uuid, node] : m_nodes) {
        zeno::NodeData nodeinfo = node->exportInfo();
        graph.nodes.insert(std::make_pair(node->get_name(), nodeinfo));
    }
    return graph;
}

ZENO_API LinksData Graph::exportLinks() const
{
    LinksData links;
    for (auto& [uuid, node] : m_nodes) {
        zeno::NodeData nodeinfo = node->exportInfo();
        for (ParamInfo param : nodeinfo.inputs) {
            links.insert(links.end(), param.links.begin(), param.links.end());
        }
    }
    return links;
}

ZENO_API std::string Graph::getName() const {
    if (optParentSubgNode.has_value()) {
        SubnetNode* pSubnetNode = optParentSubgNode.value();
        return pSubnetNode->get_name();
    }
    return m_name;
}

ZENO_API void Graph::setName(const std::string& na) {
    m_name = na;
}

ZENO_API bool Graph::removeNode(std::string const& name) {
    auto it = m_name2uuid.find(name);
    std::string uuid = safe_at(m_name2uuid, name, "get uuid when calling removeNode");
    auto spNode = safe_at(m_nodes, uuid, "");

    //remove links first
    std::vector<EdgeInfo> remLinks;
    for (const auto& [_, spParam] : spNode->inputs_) {
        for (std::shared_ptr<ILink> spLink : spParam->links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    for (const auto& [_, spParam] : spNode->outputs_) {
        for (std::shared_ptr<ILink> spLink : spParam->links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    for (auto edge : remLinks) {
        removeLink(edge);
    }

    spNode->mark_dirty_objs();

    const std::string nodecls = spNode->get_nodecls();

    node_set[nodecls].erase(name);
    m_nodes.erase(uuid);

    frame_nodes.erase(uuid);
    subnet_nodes.erase(uuid);
    asset_nodes.erase(uuid);
    m_viewnodes.erase(name);

    CALLBACK_NOTIFY(removeNode, name)
    return true;
}

ZENO_API bool Graph::addLink(const EdgeInfo& edge) {
    //����������dict/list��
    //�ⲿ�������ڵ��ô�apiʱ�������¹���
    //1.�������������dictlist������û��ָ��key������Ϊ��ֱ�������������(����Ϊdictlist)
    //2.�������������dictlist������ָ����key������Ϊ������dictlist�ڲ�����Ϊ����˵��ӳ�Ա��
    //3.������������Ƿ�dictlist������û��ָ��key������Ϊ�����������dictlist����Ϊ����˵��ڲ��ӳ�Ա��
    CORE_API_BATCH

    std::shared_ptr<INode> outNode = getNode(edge.outNode);
    if (!outNode)
        return false;
    std::shared_ptr<INode> inNode = getNode(edge.inNode);
    if (!inNode)
        return false;

    std::shared_ptr<IParam> outParam = outNode->get_output_param(edge.outParam);
    std::shared_ptr<IParam> inParam = inNode->get_input_param(edge.inParam);
    if (!outParam || !inParam)
        return false;

    EdgeInfo adjustEdge = edge;

    bool bRemOldLinks = true, bConnectWithKey = false;
    adjustEdge.inKey = edge.inKey;
    if (inParam->type == Param_Dict || inParam->type == Param_List) {
        bool bSameType = inParam->type == outParam->type;
        if (bSameType) {
            //ֱ�����ӣ���ȥ�������ԭ���Ĳ���.
            if (edge.inKey.empty()) {
                bRemOldLinks = true;
                bConnectWithKey = false;
            }
            else {
                bRemOldLinks = false;
                bConnectWithKey = true;
            }
        }
        else {
            bRemOldLinks = false;
            bConnectWithKey = true;
        }
        if (bConnectWithKey) {
            std::set<std::string> ss;
            for (auto spLink : inParam->links) {
                ss.insert(spLink->tokey);
            }

            if (adjustEdge.inKey.empty())
                adjustEdge.inKey = "obj0";

            int i = 0;
            while (ss.find(adjustEdge.inKey) != ss.end()) {
                i++;
                adjustEdge.inKey = "obj" + std::to_string(i);
            }
        }
    }

    if (bRemOldLinks)
        removeLinks(inNode->get_name(), true, inParam->name);

    std::shared_ptr<ILink> spLink = std::make_shared<ILink>();
    spLink->fromparam = outParam;
    spLink->toparam = inParam;
    spLink->fromkey = adjustEdge.outKey;
    spLink->tokey = adjustEdge.inKey;
    spLink->lnkProp = adjustEdge.lnkfunc;

    outParam->links.push_back(spLink);
    inParam->links.push_back(spLink);

    inNode->mark_dirty(true);

    CALLBACK_NOTIFY(addLink, adjustEdge);
    return true;
}

ZENO_API bool Graph::removeLink(const EdgeInfo& edge) {
    CORE_API_BATCH

    std::shared_ptr<INode> outNode = getNode(edge.outNode);
    if (!outNode)
        return false;
    std::shared_ptr<INode> inNode = getNode(edge.inNode);
    if (!inNode)
        return false;

    std::shared_ptr<IParam> outParam = outNode->get_output_param(edge.outParam);
    std::shared_ptr<IParam> inParam = inNode->get_input_param(edge.inParam);

    outParam->links.remove_if([&](std::shared_ptr<ILink> spLink) {
        auto _out_param = spLink->fromparam.lock();
        auto _in_param = spLink->toparam.lock();
        if (_out_param == outParam && _in_param == inParam) {
            return true;
        }
        return false;
    });

    inParam->links.remove_if([&](std::shared_ptr<ILink> spLink) {
        auto _out_param = spLink->fromparam.lock();
        auto _in_param = spLink->toparam.lock();
        if (_out_param == outParam && _in_param == inParam) {
            return true;
        }
        return false;
    });
    inNode->mark_dirty(true);

    CALLBACK_NOTIFY(removeLink, edge)
    return true;
}

ZENO_API bool Graph::removeLinks(const std::string nodename, bool bInput, const std::string paramname)
{
    CORE_API_BATCH

    std::shared_ptr<INode> spNode = getNode(nodename);
    std::shared_ptr<IParam> spParam;
    if (bInput)
        spParam = spNode->get_input_param(paramname);
    else
        spParam = spNode->get_output_param(paramname);

    if (!spParam)
        return false;

    std::vector<EdgeInfo> links;
    for (auto spLink : spParam->links)
    {
        links.push_back(getEdgeInfo(spLink));
    }

    for (auto link : links)
        removeLink(link);

    CALLBACK_NOTIFY(removeLinks, nodename, bInput, paramname)
    return true;
}

ZENO_API bool Graph::updateLink(const EdgeInfo& edge, bool bInput, const std::string oldkey, const std::string newkey)
{
    CORE_API_BATCH

    std::shared_ptr<INode> outNode = getNode(edge.outNode);
    if (!outNode)
        return false;
    std::shared_ptr<INode> inNode = getNode(edge.inNode);
    if (!inNode)
        return false;

    std::shared_ptr<IParam> outParam = outNode->get_output_param(edge.outParam);
    std::shared_ptr<IParam> inParam = inNode->get_input_param(edge.inParam);

    if (bInput) {
        for (auto spLink : inParam->links) {
            if (spLink->tokey == oldkey) {
                spLink->tokey = newkey;
                return true;
            }
        }
    }
    return false;
}

ZENO_API bool Graph::moveUpLinkKey(const EdgeInfo& edge, bool bInput, const std::string keyName)
{
    CORE_API_BATCH

    std::shared_ptr<INode> outNode = getNode(edge.outNode);
    if (!outNode)
        return false;
    std::shared_ptr<INode> inNode = getNode(edge.inNode);
    if (!inNode)
        return false;

    std::shared_ptr<IParam> outParam = outNode->get_output_param(edge.outParam);
    std::shared_ptr<IParam> inParam = inNode->get_input_param(edge.inParam);
    if (!inParam || !outParam)
        return false;

    if (bInput) {
        for (auto it = inParam->links.begin(); it != inParam->links.end(); it++)
        {
            if ((*it)->tokey == keyName && it != inParam->links.begin()) {
                auto it_ = std::prev(it);
                std::swap(*it, *it_);
                return true;
            }
        }
    }
    return false;

}

}
