#pragma once

#include <zeno/core/NodeImpl.h>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <zeno/core/Descriptor.h>
#include <zeno/utils/uuid.h>
#include <memory>

namespace zeno
{
	struct ZSubnetInfo {
		std::unique_ptr<Graph> m_subgraph;    //assets subnetnode
		bool m_bLocked{ false };
		bool m_bClearSubnet{ false };
	};
}