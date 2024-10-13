#include "assetsmodel.h"
#include "graphmodel.h"
#include <zeno/core/Session.h>
#include <zeno/core/Assets.h>
#include <zeno/io/zdawriter.h>
#include <zeno/utils/log.h>
#include <zeno/io/zdareader.h>
#include "zassert.h"


AssetsModel::AssetsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    std::shared_ptr<zeno::AssetsMgr> assets =  zeno::getSession().assets;
    m_cbCreateAsset = assets->register_createAsset([&](zeno::AssetInfo info) {
        _addAsset(info);
    });

    m_cbRemoveAsset = assets->register_removeAsset([&](const std::string& name) {
        _removeAsset(QString::fromStdString(name));
    });

    m_cbRenameAsset = assets->register_renameAsset([&](const std::string& old_name, const std::string& new_name) {
        //TODO
    });

    for (zeno::Asset asset : assets->getAssets()) {
        _addAsset(asset.m_info);
    }
}

AssetsModel::~AssetsModel()
{
}

void AssetsModel::init(const zeno::AssetsData& assets)
{
    //todo
}

void AssetsModel::clear()
{

}

GraphModel* AssetsModel::getAssetGraph(int i) const
{
    if (i < 0 || i >= m_assets.length())
        return nullptr;
    return m_assets[i].pGraphM;
}

GraphModel* AssetsModel::getAssetGraph(const QString& graphName)
{
    for (int i = 0; i < m_assets.length(); i++) {
        const std::string& assetName = m_assets[i].info.name;
        if (assetName == graphName.toStdString()) {
            GraphModel* pModel = m_assets[i].pGraphM;
            if (!pModel) {
                //delay load
                std::shared_ptr<zeno::AssetsMgr> assets = zeno::getSession().assets;
                std::shared_ptr<zeno::Graph> spAsset = assets->getAssetGraph(assetName, true);
                if (spAsset) {
                    auto pNewAsstModel = new GraphModel(spAsset, nullptr, this);
                    m_assets[i].pGraphM = pNewAsstModel;
                    return pNewAsstModel;
                }
            }
            return pModel;
        }
    }
    return nullptr;
}

zeno::AssetInfo AssetsModel::getAsset(const QString& assetName) const
{
    for (int i = 0; i < m_assets.length(); i++) {
        if (m_assets[i].info.name == assetName.toStdString()) {
            return m_assets[i].info;
        }
    }
    return zeno::AssetInfo();
}

int AssetsModel::rowByName(const QString& name) const {
    for (int i = 0; i < m_assets.size(); i++) {
        if (m_assets[i].info.name == name.toStdString()) {
            return i;
        }
    }
    return -1;
}

int AssetsModel::rowCount(const QModelIndex& parent) const
{
    return m_assets.size();
}

QVariant AssetsModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row >= 0 && row < m_assets.size()) {
        if (Qt::DisplayRole == role || ROLE_CLASS_NAME == role) {
            return QString::fromStdString(m_assets[row].info.name);
        }
    }
    //todo
    return QVariant();
}

bool AssetsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return false;
}

void AssetsModel::newAsset(const zeno::AssetInfo info)
{
    auto& assets = zeno::getSession().assets;
    zeno::ZenoAsset asset;
    asset.info = info;

    zeno::GraphData sample;
    sample.type = zeno::Subnet_Main;
    sample.name = info.name;

    zeno::NodeData input1;
    input1.name = "input1";
    input1.cls = "SubInput";
    input1.uipos = { 0, 0 };

    zeno::NodeData objInput1;
    objInput1.name = "objInput1";
    objInput1.cls = "SubInput";
    objInput1.uipos = { 0,700 };

    zeno::NodeData output1;
    output1.name = "output1";
    output1.cls = "SubOutput";
    output1.uipos = { 1300, 250 };

    zeno::NodeData objOutput1;
    objOutput1.name = "objOutput1";
    objOutput1.cls = "SubOutput";
    objOutput1.uipos = { 1300, 900 };

    sample.nodes.insert(std::make_pair("input1", input1));
    sample.nodes.insert(std::make_pair("objInput1", objInput1));
    sample.nodes.insert(std::make_pair("output1", output1));
    sample.nodes.insert(std::make_pair("objOutput1", objOutput1));

    asset.optGraph = sample;

    std::vector<zeno::ParamPrimitive>& inputs = asset.primitive_inputs;
    std::vector<zeno::ParamPrimitive>& outputs = asset.primitive_outputs;
    std::vector<zeno::ParamObject>& objInputs = asset.object_inputs;
    std::vector<zeno::ParamObject>& objOutputs = asset.object_outputs;

    zeno::ParamGroup defaultGroup;

    zeno::ParamPrimitive param;
    param.bInput = true;
    param.name = "input1";
    zeno::PrimVar def = int(0);
    param.defl = zeno::reflect::make_any<zeno::PrimVar>(def);
    size_t s = param.defl.type().hash_code();
    size_t s1 = gParamType_Int;
    size_t s2 = gParamType_PrimVariant;
    param.type = zeno::types::gParamType_Int;
    param.bSocketVisible = false;
    inputs.push_back(param);
    defaultGroup.params.push_back(param);
    zeno::ParamPrimitive outputparam;
    outputparam.bInput = false;
    outputparam.name = "output1";
    outputparam.defl = zeno::reflect::Any();
    outputparam.type = Param_Wildcard;
    outputparam.socketType = zeno::Socket_WildCard;
    outputparam.bSocketVisible = false;
    outputs.push_back(outputparam);
    zeno::ParamObject objInput;
    objInput.bInput = true;
    objInput.name = "objInput1";
    objInput.type = Obj_Wildcard;
    objInput.socketType = zeno::Socket_WildCard;
    objInputs.push_back(objInput);
    zeno::ParamObject objOutput;
    objOutput.bInput = false;
    objOutput.name = "objOutput1";
    objOutput.type = Obj_Wildcard;
    objOutput.socketType = zeno::Socket_WildCard;
    objOutputs.push_back(objOutput);

    zeno::ParamTab tab;
    tab.groups.emplace_back(std::move(defaultGroup));
    asset.m_customui.inputPrims.emplace_back(std::move(tab));
    asset.m_customui.outputPrims = outputs;
    asset.m_customui.inputObjs = objInputs;
    asset.m_customui.outputObjs = objOutputs;
    assets->createAsset(asset, true);

    saveAsset(QString::fromStdString(info.name));
}

void AssetsModel::addAsset(const zeno::GraphData& graph)
{
}

void AssetsModel::removeAsset(const QString& assetName)
{
    zeno::getSession().assets->removeAsset(assetName.toStdString());
}

void AssetsModel::saveAsset(const QString& name)
{
    auto& assets = zeno::getSession().assets;
    zeno::Asset asset = assets->getAsset(name.toStdString());

    zeno::ZenoAsset zasset;
    zasset.info = asset.m_info;
    zasset.optGraph = asset.sharedGraph->exportGraph();
    zasset.primitive_inputs = asset.primitive_inputs;
    zasset.primitive_outputs = asset.primitive_outputs;
    zasset.object_inputs = asset.object_inputs;
    zasset.object_outputs = asset.object_outputs;
    zasset.m_customui = asset.m_customui;

    zenoio::ZdaWriter writer;
    const std::string& content = writer.dumpAsset(zasset);

    QString filePath = QString::fromStdString(zasset.info.path);
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << Q_FUNC_INFO << "Failed to open" << filePath << f.errorString();
        zeno::log_error("Failed to open file for write: {} ({})", filePath.toStdString(),
            f.errorString().toStdString());
        return;
    }

    f.write(content.c_str());
    f.close();
    zeno::log_info("saved '{}' successfully", filePath.toStdString());
}

void AssetsModel::_addAsset(zeno::AssetInfo info)
{
    int nRows = m_assets.size();
    beginInsertRows(QModelIndex(), nRows, nRows);

    _AssetItem item;
    item.info = info;

    std::shared_ptr<zeno::AssetsMgr> asts = zeno::getSession().assets;
    std::shared_ptr<zeno::Graph> spAsset = asts->getAsset(info.name).sharedGraph;
    if (spAsset) {
        auto pNewAsstModel = new GraphModel(spAsset, nullptr, this);
        item.pGraphM = pNewAsstModel;
    }

    m_assets.append(item);
    endInsertRows();
}

void AssetsModel::_removeAsset(const QString& name)
{
    //this is a private impl method, called by callback function.
    int row = rowByName(name);

    beginRemoveRows(QModelIndex(), row, row);

    GraphModel* pDelete = m_assets[row].pGraphM;
    m_assets.removeAt(row);
    delete pDelete;

    endRemoveRows();
}

QModelIndexList AssetsModel::match(const QModelIndex& start, int role,
    const QVariant& value, int hits,
    Qt::MatchFlags flags) const
{
    //todo
    return QModelIndexList();
}

bool AssetsModel::removeRows(int row, int count, const QModelIndex& parent)
{
    return false;
}

QHash<int, QByteArray> AssetsModel::roleNames() const
{
    return QHash<int, QByteArray>();
}