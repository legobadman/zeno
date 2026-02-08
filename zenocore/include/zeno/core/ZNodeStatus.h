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


namespace zeno {

    class ZNode;

    class ZNodeStatus {
    public:
        ZNodeStatus() = delete;
        ZNodeStatus(
            ZNode* pNodeRepop,
            const std::string& class_name,
            const std::string& name
        );
        ZNodeStatus(const ZNodeStatus&) = delete;
        void initDat(const NodeData& dat);
        std::string get_nodecls() const;
        std::string get_ident() const;
        std::string get_show_name() const;
        std::string get_show_icon() const;
        ObjPath get_path() const;
        ObjPath get_graph_path() const;
        ObjPath get_uuid_path() const;
        std::string get_uuid() const;
        void initUuid(Graph* pGraph, const std::string nodecls);
        virtual NodeType nodeType() const;
        virtual bool is_locked() const;
        virtual void set_locked(bool);
        virtual void convert_to_assetinst(const std::string& asset_name);

        void set_view(bool bOn);
        CALLBACK_REGIST(set_view, void, bool)
        bool is_view() const;

        void set_bypass(bool bOn);
        CALLBACK_REGIST(set_bypass, void, bool)
        bool is_bypass() const;

        void set_nocache(bool bOn);
        CALLBACK_REGIST(set_nocache, void, bool)
        bool is_nocache() const;

        void set_name(const std::string& name);
        std::string get_name() const;

        void set_pos(std::pair<float, float> pos);
        CALLBACK_REGIST(set_pos, void, std::pair<float, float>)
        std::pair<float, float> get_pos() const;

        Graph* getGraph() const { return m_pGraph; }

    private:
        std::string m_name;
        std::string m_nodecls;
        std::string m_uuid;
        std::pair<float, float> m_pos;
        std::string m_uuidPath;
        Graph* m_pGraph = nullptr;
        bool m_bView = false;
        bool m_bypass = false;
        bool m_nocache = false;
    };

}

