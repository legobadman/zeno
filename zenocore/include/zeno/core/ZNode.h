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
#include <zeno/core/ZNodeExecutor.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/ZNodeStatus.h>
#include <zeno/core/ZSubnetInfo.h>
#include <optional>


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

        void init(const NodeData& dat);

        ZNodeParams& getNodeParams();
        ZNodeExecutor& getNodeExecutor();
        ZNodeStatus& getNodeStatus();
        std::optional<ZSubnetInfo>& getSubnetInfo();

        std::string get_name() const;
        std::string get_path() const;
        std::string get_uuid() const;

    private:
        ZNodeParams m_upParams;
        ZNodeExecutor m_upNodeExec;
        ZNodeStatus m_upNodeStatus;
        std::optional<ZSubnetInfo> m_opt_subnet_info;
    };

}
