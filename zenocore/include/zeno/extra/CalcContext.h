#pragma once

#include <unordered_set>

namespace zeno {

struct CalcContext {
    std::unordered_set<std::string> uuid_node_params;
    std::unordered_set<std::string> visited_nodes;
    bool isSubnetApply = false;
    int curr_iter = -1;     //用于调试
};

}