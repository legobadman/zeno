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
    std::string getName() const;

private:
    std::weak_ptr<zeno::Graph> m_wpGraph;
};

#endif