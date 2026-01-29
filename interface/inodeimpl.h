#ifndef __INF_INODE_IMPL_H__
#define __INF_INODE_IMPL_H__

#include <zenum.h>
#include <inodedata.h>

namespace zeno {
	struct INode2 {
		virtual void apply(INodeData* ptrNodeData) = 0;
		virtual zeno::NodeType type() const = 0;
	};
}


#endif