#pragma once

#ifndef __ZPYNODE_H__
#define __ZPYNODE_H__

#include <zeno/core/INode.h>

class Zpy_Node {
public:
    Zpy_Node(std::shared_ptr<zeno::INode> spNode);

    void set_name(const std::string& name);
    std::string get_name() const;

private:
    std::weak_ptr<zeno::INode> m_wpNode;
};

#endif