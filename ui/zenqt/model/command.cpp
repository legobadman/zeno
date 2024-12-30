#include "command.h"
#include "variantptr.h"
#include <zeno/utils/helper.h>
#include <zeno/core/typeinfo.h>
#include <zeno/extra/SubnetNode.h>
#include "model/parammodel.h"
#include "util/uihelper.h"
#include "zenoapplication.h"


AddNodeCommand::AddNodeCommand(const QString& cate, zeno::NodeData& nodedata, QStringList& graphPath)
    : QUndoCommand()
    , m_model(zenoApp->graphsManager()->getGraph(graphPath))
    , m_graphPath(graphPath)
    , m_nodeData(nodedata)
    , m_pos(nodedata.uipos)
    , m_cate(cate)
{
    if (zeno::isDerivedFromSubnetNodeName(m_nodeData.cls)) //init subnet default socket
    {
        zeno::ParamTab tab;
        zeno::ParamGroup default;

        zeno::ParamUpdateInfo info;

        zeno::ParamPrimitive param;
        param.bInput = true;
        param.name = "int1";
        param.defl = zeno::reflect::make_any<zeno::PrimVar>(zeno::PrimVar(0));;
        param.type = zeno::types::gParamType_Int;
        param.socketType = zeno::Socket_Primitve;
        param.control = zeno::Lineedit;
        param.bSocketVisible = false;
        info.param = param;
        default.params.push_back(param);

        zeno::ParamPrimitive outputparam;
        outputparam.bInput = false;
        outputparam.name = "output1";
        outputparam.defl = zeno::reflect::Any();
        outputparam.type = Param_Wildcard;
        outputparam.socketType = zeno::Socket_Primitve;
        outputparam.bSocketVisible = false;
        info.param = outputparam;

        zeno::ParamObject objInput;
        objInput.bInput = true;
        objInput.name = "objInput1";
        objInput.type = gParamType_Geometry;

        zeno::ParamObject objOutput;
        objOutput.bInput = false;
        objOutput.name = "objOutput1";
        objOutput.type = gParamType_Geometry;
        objOutput.socketType = zeno::Socket_Output;

        tab.groups.emplace_back(std::move(default));
        m_nodeData.customUi.inputPrims.emplace_back(std::move(tab));
        m_nodeData.customUi.inputObjs.push_back(objInput);
        m_nodeData.customUi.outputPrims.push_back(outputparam);
        m_nodeData.customUi.outputObjs.push_back(objOutput);
    }
}

AddNodeCommand::~AddNodeCommand()
{
}

void AddNodeCommand::redo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model) {
        m_nodeData.uipos = m_pos;
        m_nodeData = m_model->_createNodeImpl(m_cate, m_nodeData, false);
    }
}

void AddNodeCommand::undo()
{
    if (m_model) {
        auto nodename = QString::fromStdString(m_nodeData.name);
        QModelIndex idx = m_model->indexFromName(nodename);
        QPointF pos = idx.data(QtRole::ROLE_OBJPOS).toPointF();
        m_pos = { pos.x(), pos.y() };
        m_model->_removeNodeImpl(nodename);
    }
}


zeno::NodeData AddNodeCommand::getNodeData()
{
    return m_nodeData;
}

RemoveNodeCommand::RemoveNodeCommand(zeno::NodeData& nodeData, QStringList& graphPath)
    : QUndoCommand()
    , m_model(zenoApp->graphsManager()->getGraph(graphPath))
    , m_nodeData(nodeData)
    , m_graphPath(graphPath)
    , m_cate("")
{
    //m_id = m_data[ROLE_OBJID].toString();

    ////all links will be removed when remove node, for caching other type data,
    ////we have to clean the data here.
    //OUTPUT_SOCKETS outputs = m_data[QtRole::ROLE_OUTPUTS].value<OUTPUT_SOCKETS>();
    //INPUT_SOCKETS inputs = m_data[QtRole::ROLE_INPUTS].value<INPUT_SOCKETS>();
    //for (auto it = outputs.begin(); it != outputs.end(); it++)
    //{
    //    it->second.info.links.clear();
    //}
    //for (auto it = inputs.begin(); it != inputs.end(); it++)
    //{
    //    it->second.info.links.clear();
    //}
    //m_data[QtRole::ROLE_INPUTS] = QVariant::fromValue(inputs);
    //m_data[QtRole::ROLE_OUTPUTS] = QVariant::fromValue(outputs);
}

RemoveNodeCommand::~RemoveNodeCommand()
{
}

void RemoveNodeCommand::redo()
{
    if (m_model) {
        auto nodename = QString::fromStdString(m_nodeData.name);
        QModelIndex idx = m_model->indexFromName(nodename);
        zeno::NodeType type = (zeno::NodeType)idx.data(QtRole::ROLE_NODETYPE).toInt();
        if (type == zeno::Node_AssetInstance) {
            m_cate = "assets";
        }
        else {
            m_cate = "";
        }
        //if (auto subnetNode = dynamic_cast<zeno::SubnetNode*>(spNode)) {   //if is subnet/assets，record cate
        //    m_cate = subnetNode->isAssetsNode() ? "assets" : "";
        //}
        m_model->_removeNodeImpl(QString::fromStdString(m_nodeData.name));
    }
}

void RemoveNodeCommand::undo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model)
        m_nodeData = m_model->_createNodeImpl(m_cate, m_nodeData, false);
}

LinkCommand::LinkCommand(bool bAddLink, const zeno::EdgeInfo& link, QStringList& graphPath)
    : QUndoCommand()
    , m_bAdd(bAddLink)
    , m_link(link)
    , m_model(zenoApp->graphsManager()->getGraph(graphPath))
    , m_graphPath(graphPath)
{
}

void LinkCommand::redo()
{
    if (m_bAdd)
    {
        m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
        if (m_model) {
            QStringList outNodeLinkedNodes = UiHelper::findAllLinkdNodes(m_model, QString::fromStdString(m_link.outNode));
            bool outNodeChainHasViewNode = false;
            for (auto& node : outNodeLinkedNodes) {
                auto nodeidx = m_model->indexFromName(node);
                if (nodeidx.data(QtRole::ROLE_NODE_ISVIEW).toBool()) {
                    outNodeChainHasViewNode = true;
                    break;
                }
            }
            QStringList intNodeLinkedNodes = UiHelper::findAllLinkdNodes(m_model, QString::fromStdString(m_link.inNode));
            for (auto& node : intNodeLinkedNodes) {
                auto nodeidx = m_model->indexFromName(node);
                if (outNodeChainHasViewNode && nodeidx.data(QtRole::ROLE_NODE_ISVIEW).toBool()) {
                    m_lastViewNodeName = nodeidx.data(QtRole::ROLE_NODE_NAME).toString();
                    m_model->_setViewImpl(nodeidx, false);
                    break;
                }
            }
            m_model->_addLinkImpl(m_link);
    }
    }
    else
    {
        if (m_model)
            m_model->_removeLinkImpl(m_link);
    }
}

void LinkCommand::undo()
{
    if (m_bAdd)
    {
        if (m_model) {
            if (!m_lastViewNodeName.isEmpty()) {
                auto lastViewNodeIdx = m_model->indexFromName(m_lastViewNodeName);
                m_model->_setViewImpl(lastViewNodeIdx, true);
            }
            m_model->_removeLinkImpl(m_link);
    }
    }
    else
    {
        m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
        if (m_model)
            m_model->_addLinkImpl(m_link);
    }
}

ModelDataCommand::ModelDataCommand(const QModelIndex& index, const QVariant& oldData, const QVariant& newData, int role, QStringList& graphPath)
    : m_oldData(oldData)
    , m_newData(newData)
    , m_role(role)
    , m_graphPath(graphPath)
    , m_model(nullptr)
    , m_nodeName("")
    , m_paramName("")
{
    m_nodeName = index.data(QtRole::ROLE_NODE_NAME).toString();
    if (m_role == QtRole::ROLE_PARAM_VALUE)  //index of paramsModel, need record paramName
        m_paramName = index.data(QtRole::ROLE_PARAM_NAME).toString();
}

void ModelDataCommand::redo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model)
    {
        auto nodeIdx = m_model->indexFromName(m_nodeName);
        if (nodeIdx.isValid())
        {
            if (m_role == QtRole::ROLE_OBJPOS || m_role == QtRole::ROLE_COLLASPED)
            {
                m_model->setData(nodeIdx, m_newData, m_role);
            }else if (m_role == QtRole::ROLE_PARAM_VALUE)
            {
                if (ParamsModel* paramsModel = QVariantPtr<ParamsModel>::asPtr(nodeIdx.data(QtRole::ROLE_PARAMS)))
                {
                    auto paramIdx = paramsModel->paramIdx(m_paramName, true);
                    paramsModel->setData(paramIdx, m_newData, m_role);
                }
            }
        }
    }
}

void ModelDataCommand::undo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model)
    {
        auto nodeIdx = m_model->indexFromName(m_nodeName);
        if (nodeIdx.isValid())
        {
            if (m_role == QtRole::ROLE_OBJPOS || m_role == QtRole::ROLE_COLLASPED)
            {
                m_model->setData(nodeIdx, m_oldData, m_role);
            }
            else if (m_role == QtRole::ROLE_PARAM_VALUE)
            {
                if (ParamsModel* paramsModel = QVariantPtr<ParamsModel>::asPtr(nodeIdx.data(QtRole::ROLE_PARAMS)))
                {
                    auto paramIdx = paramsModel->paramIdx(m_paramName, true);
                    paramsModel->setData(paramIdx, m_oldData, m_role);
                }
            }
        }
    }
}

NodeStatusCommand::NodeStatusCommand(bool isSetView, const QString& name, bool bOn, QStringList& graphPath)
    : m_On(bOn)
    , m_graphPath(graphPath)
    , m_model(nullptr)
    , m_nodeName(name)
    , m_isSetView(isSetView)
{
}

void NodeStatusCommand::redo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model)
    {
        auto idx = m_model->indexFromName(m_nodeName);
        if (idx.isValid()) {
            if (m_isSetView)
            {
                if (m_On) {
                    QStringList linkedNodes = UiHelper::findAllLinkdNodes(m_model, m_nodeName);
                    for (auto& node : linkedNodes) {
                        auto nodeidx = m_model->indexFromName(node);
                        if (nodeidx.data(QtRole::ROLE_NODE_ISVIEW).toBool() && nodeidx.data(QtRole::ROLE_NODE_NAME).toString() != m_nodeName) {
                            m_lastViewNodeName = nodeidx.data(QtRole::ROLE_NODE_NAME).toString();
                            m_model->_setViewImpl(nodeidx, !m_On);
                            break;
                        }
                    }
                }
                m_model->_setViewImpl(idx, m_On);
            }
            else {
                m_model->_setMuteImpl(idx, m_On);
            }
        }
    }
}

void NodeStatusCommand::undo()
{
    m_model = zenoApp->graphsManager()->getGraph(m_graphPath);
    if (m_model)
    {
        auto idx = m_model->indexFromName(m_nodeName);
        if (idx.isValid()) {
            if (m_isSetView)
            {
                if (m_On && !m_lastViewNodeName.isEmpty()) {
                    auto lastViewNodeIdx = m_model->indexFromName(m_lastViewNodeName);
                    m_model->_setViewImpl(lastViewNodeIdx, m_On);
                }
                m_model->_setViewImpl(idx, !m_On);
            }
            else {
                m_model->_setMuteImpl(idx, !m_On);
            }
        }
    }
}
