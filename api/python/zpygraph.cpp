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

Zpy_Node Zpy_Graph::newNode(const std::string& nodecls, const std::string& originalname, bool bAssets)
{
    THROW_WHEN_CORE_DESTROYED
    std::shared_ptr<zeno::INode> spNode = spGraph->createNode(nodecls, originalname, bAssets);
    return Zpy_Node(spNode);
}

std::string Zpy_Graph::getName() const
{
    THROW_WHEN_CORE_DESTROYED
    return spGraph->getName();
}

Zpy_Node Zpy_Graph::getNode(const std::string& name) {
    THROW_WHEN_CORE_DESTROYED
    std::shared_ptr<zeno::INode> spNode = spGraph->getNode(name);
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