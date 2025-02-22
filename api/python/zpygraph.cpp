#include "zpygraph.h"


Zpy_Graph::Zpy_Graph(std::shared_ptr<zeno::Graph> graph)
    : m_wpGraph(graph)
{

}

Zpy_Node Zpy_Graph::newNode(const std::string& nodecls, const std::string& originalname, bool bAssets)
{
    auto spGraph = m_wpGraph.lock();
    if (spGraph) {
        std::shared_ptr<zeno::INode> spNode = spGraph->createNode(nodecls, originalname, bAssets);
        return Zpy_Node(spNode);
    }
    else {
        return Zpy_Node(0);
    }
}

std::string Zpy_Graph::getName() const
{
    auto spGraph = m_wpGraph.lock();
    if (spGraph) {
        return spGraph->getName();
    }
    else {
        return "";
    }
}