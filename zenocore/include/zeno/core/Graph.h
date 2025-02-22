#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/IObject.h>
#include <zeno/core/INode.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/types/UserData.h>
#include <functional>
#include <variant>
#include <memory>
#include <unordered_map>
#include <string>
#include <set>
#include <map>
#include <zeno/core/data.h>
#include <zeno/utils/uuid.h>


namespace zeno {

struct Session;
struct SubgraphNode;
struct SubnetNode;
struct DirtyChecker;
struct INode;

struct Context {
    std::set<std::string> visited;

    inline void mergeVisited(Context const &other) {
        visited.insert(other.visited.begin(), other.visited.end());
    }

    Context();
    Context(Context const &other);
    ~Context();
};

struct ZENO_API Graph : public std::enable_shared_from_this<Graph> {
    Session* session = nullptr;

    int beginFrameNumber = 0, endFrameNumber = 0;  // only use by runnermain.cpp

    std::map<std::string, std::string> portalIns;   //todo: deprecated, but need to keep compatible with old zsg.
    std::map<std::string, zany> portals;

    std::unique_ptr<Context> ctx;

    std::optional<SubnetNode*> optParentSubgNode;

    Graph(const std::string& name, bool bAssets = false);
    ~Graph();

    Graph(Graph const &) = delete;
    Graph &operator=(Graph const &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(Graph &&) = delete;

    //BEGIN NEW STANDARD API
    void init(const GraphData& graph);

    std::shared_ptr<INode> createNode(std::string const& cls, const std::string& orgin_name = "", bool bAssets = false, std::pair<float, float> pos = {});
    CALLBACK_REGIST(createNode, void, const std::string&, std::weak_ptr<zeno::INode>)

    bool removeNode(std::string const& name);
    CALLBACK_REGIST(removeNode, void, const std::string&)

    bool addLink(const EdgeInfo& edge);
    CALLBACK_REGIST(addLink, bool, EdgeInfo)

    bool removeLink(const EdgeInfo& edge);
    CALLBACK_REGIST(removeLink, bool, EdgeInfo)

    bool removeLinks(const std::string nodename, bool bInput, const std::string paramname);
    CALLBACK_REGIST(removeLinks, bool, std::string, bool, std::string)

    bool updateLink(const EdgeInfo& edge, bool bInput, const std::string oldkey, const std::string newkey);
    bool moveUpLinkKey(const EdgeInfo& edge, bool bInput, const std::string keyName);

    bool hasNode(std::string const& uuid_node_path);
    std::shared_ptr<INode> getNode(std::string const& name);
    std::shared_ptr<INode> getNodeByUuidPath(ObjPath path);
    std::shared_ptr<zeno::INode> getNodeByPath(const std::string& path);
    std::shared_ptr<Graph> getGraphByPath(const std::string& path);
    std::map<std::string, std::shared_ptr<INode>> getNodes() const;
    std::set<std::string> get_viewnodes() const;

    GraphData exportGraph() const;

    LinksData exportLinks() const;

    std::string getName() const;
    void setName(const std::string& name);

    std::string updateNodeName(const std::string oldName, const std::string newName = "");
    CALLBACK_REGIST(updateNodeName, void, std::string, std::string)

    void clear();
    CALLBACK_REGIST(clear, void)

    bool isAssets() const;
    std::set<std::string> searchByClass(const std::string& name) const;

    void clearNodes();
    void runGraph();
    void applyNodes(std::set<std::string> const &ids);
    void addNode(std::string const &cls, std::string const &id);
    Graph *addSubnetNode(std::string const &id);
    Graph *getSubnetGraph(std::string const &id) const;
    void completeNode(std::string const &id);
    void bindNodeInput(std::string const &dn, std::string const &ds,
        std::string const &sn, std::string const &ss);

    //容易有歧义：这个input是defl value，还是实质的对象？按原来zeno的语义，是指defl value
    void setNodeInput(std::string const &id, std::string const &par, zany const &val);

    void setKeyFrame(std::string const &id, std::string const &par, zany const &val);
    void setFormula(std::string const &id, std::string const &par, zany const &val);
    void addNodeOutput(std::string const &id, std::string const &par);
    zany getNodeInput(std::string const &sn, std::string const &ss) const;
    void setNodeParam(std::string const &id, std::string const &par,
        std::variant<int, float, std::string, zany> const &val);  /* to be deprecated */
    std::map<std::string, zany> callSubnetNode(std::string const &id,
            std::map<std::string, zany> inputs) const;
    std::map<std::string, zany> callTempNode(std::string const &id,
            std::map<std::string, zany> inputs);

    std::set<std::string> getSubInputs();
    std::set<std::string> getSubOutputs();
    void viewNodeUpdated(const std::string node, bool bView);
    void markDirtyWhenFrameChanged();
    void markDirtyAll();
    void onNodeParamUpdated(PrimitiveParam* spParam, zeno::reflect::Any old_value, zeno::reflect::Any new_value);
    void parseNodeParamDependency(PrimitiveParam* spParam, zeno::reflect::Any& new_value);

    bool isFrameNode(std::string uuid);

    //增/删边之后更新wildCard端口的类型
    void updateWildCardParamTypeRecursive(std::shared_ptr<Graph> spCurrGarph, std::shared_ptr<INode> spNode, std::string paramName, bool bPrim, bool bInput, ParamType newtype);

private:
    std::string generateNewName(const std::string& node_cls, const std::string& origin_name = "", bool bAssets = false);
    //增/删边之后更新wildCard端口的类型
    void removeLinkWhenUpdateWildCardParam(const std::string& outNode, const std::string& inNode, EdgeInfo& edge);
    void resetWildCardParamsType(bool bWildcard, std::shared_ptr<INode>& node, const std::string& paramName, const bool& bPrimType, const bool& bInput);
    std::shared_ptr<Graph> _getGraphByPath(std::vector<std::string> items);
    bool isLinkValid(const EdgeInfo& edge);
    bool applyNode(std::string const& id);


    std::map<std::string, std::shared_ptr<INode>> m_nodes;  //based on uuid.
    std::set<std::string> nodesToExec;

    std::map<std::string, std::string> subInputNodes;
    std::map<std::string, std::string> subOutputNodes;

    std::map<std::string, std::string> m_name2uuid;

    std::map<std::string, std::set<std::string>> node_set;
    std::set<std::string> frame_nodes;      //record all nodes depended on frame num.
    std::set<std::string> subnet_nodes;
    std::set<std::string> asset_nodes;
    std::set<std::string> subinput_nodes;
    std::set<std::string> suboutput_nodes;

    std::set<std::string> m_viewnodes;
    std::string m_name;

    const bool m_bAssets;
};

}
