#include <zeno/io/zdareader.h>
#include <zeno/utils/string.h>
#include <zeno/io/iotags.h>
#include <zeno/utils/helper.h>


namespace zenoio
{
    ZENO_API ZdaReader::ZdaReader() : m_bDelayReadGraphData(false) {

    }

    ZENO_API void ZdaReader::setDelayReadGraph(bool bDelay) {
        m_bDelayReadGraphData = bDelay;
    }

    static int _statistic_nodes(const rapidjson::Value& graph) {
        const auto& nodes = graph["nodes"];
        if (nodes.IsNull())
            return 0;

        int cnt = 0;
        for (const auto& node : nodes.GetObject()) {
            cnt++;
        }
        return cnt;
    }

    bool ZdaReader::_parseMainGraph(const rapidjson::Document& doc, zeno::GraphData& ret) {
        if (!doc.HasMember("name") ||
            !doc.HasMember("version") ||
            !doc.HasMember("graph") ||
            !doc.HasMember("Parameters"))
        {
            return false;
        }

        if (!doc["name"].IsString() ||
            !doc["version"].IsString() ||
            !doc["graph"].IsObject() ||
            !doc["Parameters"].IsObject())
        {
            return false;
        }

        m_asset.info.name = doc["name"].GetString();
        const std::string& ver = doc["version"].GetString();
        std::vector<std::string> vervec = zeno::split_str(ver, '.');
        if (vervec.size() == 1) {
            m_asset.info.majorVer = std::stoi(vervec[0]);
        }
        else if (vervec.size() == 2) {
            m_asset.info.majorVer = std::stoi(vervec[0]);
            m_asset.info.minorVer = std::stoi(vervec[1]);
        }

        zeno::AssetsData assets;
        if (m_bDelayReadGraphData) {
            m_num_of_nodes = _statistic_nodes(doc["graph"]);
        }
        else
        {
            if (!_parseGraph(doc["graph"], assets, ret))
                return false;
        }

        zeno::NodeData tmp;
        _parseParams(doc["Parameters"], tmp);

        m_asset.object_inputs = tmp.customUi.inputObjs;
        m_asset.primitive_inputs = customUiToParams(tmp.customUi.inputPrims);
        m_asset.primitive_outputs = tmp.customUi.outputPrims;
        m_asset.object_outputs = tmp.customUi.outputObjs;

        if (doc.HasMember("subnet-customUi"))
        {
            m_asset.m_customui = _parseCustomUI(doc["subnet-customUi"]);
            m_asset.m_customui.inputObjs = tmp.customUi.inputObjs;
            m_asset.m_customui.outputPrims = tmp.customUi.outputPrims;
            m_asset.m_customui.outputObjs = tmp.customUi.outputObjs;
        }

        ret.type = zeno::Subnet_Normal;
        ret.name = m_asset.info.name;
        if (!m_bDelayReadGraphData)
            m_asset.optGraph = ret;
        return true;
    }

    ZENO_API zeno::ZenoAsset ZdaReader::getParsedAsset() const
    {
        return m_asset;
    }

    void ZdaReader::_parseParams(const rapidjson::Value& paramsObj, zeno::NodeData& ret)
    {
        if (paramsObj.HasMember(iotags::params::node_inputs_objs))
        {
            for (const auto& inObj : paramsObj[iotags::params::node_inputs_objs].GetObject())
            {
                const std::string& inSock = inObj.name.GetString();
                const auto& inputObj = inObj.value;

                if (inputObj.IsNull())
                {
                    zeno::ParamObject param;
                    param.name = inSock;
                    ret.customUi.inputObjs.push_back(param);
                }
                else if (inputObj.IsObject())
                {
                    zeno::LinksData links;
                    zeno::ReferencesData refs;
                    _parseSocket(true, true, true, "", "", inSock, inputObj, ret, links, refs);
                }
                else
                {
                    //TODO
                }
            }
        }
        if (paramsObj.HasMember(iotags::params::node_inputs_primitive))
        {
            for (const auto& inObj : paramsObj[iotags::params::node_inputs_primitive].GetObject())
            {
                const std::string& inSock = inObj.name.GetString();
                const auto& inputObj = inObj.value;

                if (inputObj.IsNull())
                {
                }
                else if (inputObj.IsObject())
                {
                    zeno::LinksData links;
                    zeno::ReferencesData refs;
                    _parseSocket(true, true, false, "", "", inSock, inputObj, ret, links, refs);
                }
                else
                {
                }
            }
        }
        if (paramsObj.HasMember(iotags::params::node_outputs_primitive))
        {
            for (const auto& outObj : paramsObj[iotags::params::node_outputs_primitive].GetObject())
            {
                const std::string& outSock = outObj.name.GetString();
                const auto& outputObj = outObj.value;
                if (outputObj.IsNull())
                {
                }
                else if (outputObj.IsObject())
                {
                    zeno::LinksData links;
                    zeno::ReferencesData refs;
                    _parseSocket(false, true, false, "", "", outSock, outputObj, ret, links, refs);
                }
                else
                {
                }
            }
        }
        if (paramsObj.HasMember(iotags::params::node_outputs_objs))
        {
            for (const auto& outObj : paramsObj[iotags::params::node_outputs_objs].GetObject())
            {
                const std::string& outSock = outObj.name.GetString();
                const auto& outputObj = outObj.value;
                if (outputObj.IsNull())
                {
                    zeno::ParamObject param;
                    param.name = outSock;
                    ret.customUi.outputObjs.push_back(param);
                }
                else if (outputObj.IsObject())
                {
                    zeno::LinksData links;
                    zeno::ReferencesData refs;
                    _parseSocket(false, true, true, "", "", outSock, outputObj, ret, links, refs);
                }
                else
                {
                }
            }
        }
    }
}