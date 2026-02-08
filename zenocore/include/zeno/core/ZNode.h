#pragma once


#include <zeno/utils/api.h>
#include <iobject2.h>
#include <inodedata.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <variant>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <zeno/types/ListObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/core/data.h>
#include <zeno/utils/uuid.h>
#include <zeno/utils/safe_at.h>
#include <zeno/core/CoreParam.h>
#include <functional>
#include <reflect/registry.hpp>
#include <zcommon.h>
#include <inodedata.h>
#include <inodeimpl.h>


namespace zeno
{
    class ZNodeParams;
    class ZNodeExecutor;
    class ZNodeStatus;

    class ZNode {
    public:
        ZNode(
            const std::string& class_name,
            const std::string& name,
            INode2* pNode,
            void (*dtor)(INode2*),
            Graph* pGraph,
            const CustomUI& customui
            );
        ZNode(const ZNode& rhs) = delete;
        virtual ~ZNode() = default;

        ZNodeParams* getNodeParams() const;
        ZNodeExecutor* getNodeExecutor() const;
        ZNodeStatus* getNodeStatus() const;

        std::string get_name() const;
        std::string get_path() const;

    private:
        std::unique_ptr<ZNodeParams> m_upParams;
        std::unique_ptr<ZNodeExecutor> m_upNodeExec;
        std::unique_ptr<ZNodeStatus> m_upNodeStatus;
    };

}
