#include "zpynode.h"
#include <zeno/core/Graph.h>


Zpy_Node::Zpy_Node(std::shared_ptr<zeno::INode> spNode)
    : m_wpNode(spNode)
{
}

void Zpy_Node::set_name(const std::string& name)
{
    auto spNode = m_wpNode.lock();
    if (spNode) {
        auto oldname = spNode->get_name();
        if (auto spGraph = spNode->getGraph().lock()) {
            spGraph->updateNodeName(oldname, name);
        }
    }
    else {
        //throw exception?
    }
}

std::string Zpy_Node::get_name() const
{
    auto spNode = m_wpNode.lock();
    if (spNode) {
        return spNode->get_name();
    }
    else {
        return "";
    }
}