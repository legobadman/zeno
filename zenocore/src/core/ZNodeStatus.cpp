#include <zeno/core/ZNodeStatus.h>

namespace zeno {

    ZNodeStatus::ZNodeStatus(
        ZNode* pNodeRepo,
        const std::string& class_name,
        const std::string& name)
        : m_name(name)
        , m_nodecls(class_name)
    {
        (void)pNodeRepo;
    }

    // ---------------- basic info ----------------

    void ZNodeStatus::initDat(const NodeData& dat)
    {
        (void)dat;
    }

    std::string ZNodeStatus::get_nodecls() const
    {
        return m_nodecls;
    }

    std::string ZNodeStatus::get_ident() const
    {
        return {};
    }

    std::string ZNodeStatus::get_show_name() const
    {
        return m_name;
    }

    std::string ZNodeStatus::get_show_icon() const
    {
        return {};
    }

    ObjPath ZNodeStatus::get_path() const
    {
        return {};
    }

    ObjPath ZNodeStatus::get_graph_path() const
    {
        return {};
    }

    ObjPath ZNodeStatus::get_uuid_path() const
    {
        return {};
    }

    std::string ZNodeStatus::get_uuid() const
    {
        return m_uuid;
    }

    void ZNodeStatus::initUuid(Graph* pGraph, const std::string nodecls)
    {
        (void)pGraph;
        (void)nodecls;
    }

    // ---------------- virtuals ----------------

    NodeType ZNodeStatus::nodeType() const
    {
        return NodeType{};
    }

    bool ZNodeStatus::is_locked() const
    {
        return false;
    }

    void ZNodeStatus::set_locked(bool b)
    {
        (void)b;
    }

    void ZNodeStatus::convert_to_assetinst(const std::string& asset_name)
    {
        (void)asset_name;
    }

    // ---------------- flags ----------------

    void ZNodeStatus::set_view(bool bOn)
    {
        m_bView = bOn;
    }

    bool ZNodeStatus::is_view() const
    {
        return m_bView;
    }

    void ZNodeStatus::set_bypass(bool bOn)
    {
        m_bypass = bOn;
    }

    bool ZNodeStatus::is_bypass() const
    {
        return m_bypass;
    }

    void ZNodeStatus::set_nocache(bool bOn)
    {
        m_nocache = bOn;
    }

    bool ZNodeStatus::is_nocache() const
    {
        return m_nocache;
    }

    // ---------------- name / pos ----------------

    void ZNodeStatus::set_name(const std::string& name)
    {
        m_name = name;
    }

    std::string ZNodeStatus::get_name() const
    {
        return m_name;
    }

    void ZNodeStatus::set_pos(std::pair<float, float> pos)
    {
        m_pos = pos;
    }

    std::pair<float, float> ZNodeStatus::get_pos() const
    {
        return m_pos;
    }

} // namespace zeno
