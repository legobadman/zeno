#pragma once

#include <unordered_set>
#include <zeno/core/common.h>

namespace zeno {

struct CalcContext {
    //代表一条计算链路下的上下文缓存记录，起点通常是view的节点

    std::unordered_set<std::string> uuid_node_params;
    std::unordered_set<std::string> visited_nodes;
    container_elem_update_info cond_info;   //记录当前view链路下的List/Dict元素更新情况
    bool isSubnetApply = false;
    int curr_iter = -1;     //用于调试
};



}