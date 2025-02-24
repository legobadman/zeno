#pragma once

#ifndef __ZPYGRAPH_H__
#define __ZPYGRAPH_H__

#include <string>
#include <zeno/core/Graph.h>
#include "zpynode.h"

class Zpy_Graph {
public:
    Zpy_Graph(std::shared_ptr<zeno::Graph> graph);
    Zpy_Node newNode(const std::string& nodecls, const std::string& originalname = "", bool bAssets = false);
    Zpy_Node getNode(const std::string& name);
    void removeNode(const std::string& name);
    std::string getName() const;
    void addEdge(const std::string& out_node, const std::string& out_param,
        const std::string& in_node, const std::string& in_param);

private:
    std::weak_ptr<zeno::Graph> m_wpGraph;
};

#endif