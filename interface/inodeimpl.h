#ifndef __INF_INODE_IMPL_H__
#define __INF_INODE_IMPL_H__

#include <zenum.h>
#include <inodedata.h>

namespace zeno {
    struct INode2 {
        virtual ZErrorCode apply(INodeData* ptrNodeData) = 0;
        virtual NodeType type() const = 0;
        virtual void clearCalcResults() = 0;
        virtual float time() const = 0;
    };
}

#define DEF_OVERRIDE_FOR_INODE \
        NodeType type() const { return Node_Normal; } \
        void clearCalcResults() {} \
        float time() const { return 1.0f; }

#endif