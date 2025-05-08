#include "zpygraph.h"


#define THROW_WHEN_CORE_DESTROYED \
auto spGraph = m_wpGraph.lock();\
if (!spGraph) {\
    throw std::runtime_error("the node has been destroyed in core data");\
}


Zpy_Graph::Zpy_Graph(std::shared_ptr<zeno::Graph> graph)
    : m_wpGraph(graph)
{

}

Zpy_Node Zpy_Graph::createNode(const std::string& nodecls, const std::string& originalname, bool bAssets)
{
    THROW_WHEN_CORE_DESTROYED
    auto spNode = spGraph->createNode(nodecls, originalname, bAssets);
    //TODO: register api notify into core
    return Zpy_Node(spNode);
}

std::string Zpy_Graph::getName() const
{
    THROW_WHEN_CORE_DESTROYED
    return spGraph->getName();
}

Zpy_Node Zpy_Graph::getNode(const std::string& name) {
    THROW_WHEN_CORE_DESTROYED
    auto spNode = spGraph->getNode(name);
    return Zpy_Node(spNode);
}

void Zpy_Graph::removeNode(const std::string& name) {
    THROW_WHEN_CORE_DESTROYED
    spGraph->removeNode(name);
}

void Zpy_Graph::addEdge(const std::string& out_node, const std::string& out_param,
    const std::string& in_node, const std::string& in_param)
{
    THROW_WHEN_CORE_DESTROYED
    zeno::EdgeInfo edge;
    edge.bObjLink = true;
    edge.outNode = out_node;
    edge.outParam = out_param;
    edge.inNode = in_node;
    edge.inParam = in_param;
    spGraph->addLink(edge);
}

void Zpy_Graph::removeEdge(const std::string& out_node, const std::string& out_param,
    const std::string& in_node, const std::string& in_param)
{
    THROW_WHEN_CORE_DESTROYED
    zeno::EdgeInfo edge;
    edge.bObjLink = true;
    edge.outNode = out_node;
    edge.outParam = out_param;
    edge.inNode = in_node;
    edge.inParam = in_param;
    spGraph->removeLink(edge);
}

py::object Zpy_Graph::getCamera() const {
    THROW_WHEN_CORE_DESTROYED
    auto vecNodes = spGraph->getNodesByClass("MakeCamera");
    if (vecNodes.empty()) {
        return py::none();
    }

    Zpy_Camera apiCamera(vecNodes[0]);
    //Ҫ�����ýڵ��Ƿ���obj������ֻ�нڵ����objΥ�������obj��Ƶĳ���
    if (apiCamera.getCamera()) {
        return py::cast(apiCamera);
    }
    return py::none();
}

py::object Zpy_Graph::getLight() const {
    THROW_WHEN_CORE_DESTROYED
    auto vecNodes = spGraph->getNodesByClass("LightNode");
    if (vecNodes.empty()) {
        return py::none();
    }
    Zpy_Light apiLight(vecNodes[0]);
    if (apiLight.getLight()) {
        return py::cast(apiLight);
    }
    return py::none();
}