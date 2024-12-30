#include "zsocketlayout.h"
#include "zenosocketitem.h"
#include "zlayoutbackground.h"
#include "socketbackground.h"
#include "zgraphicstextitem.h"
#include "zassert.h"
#include "style/zenostyle.h"
#include "zdictpanel.h"
#include "variantptr.h"
#include "control/common_id.h"
#include "nodeeditor/gv/zenoparamwidget.h"
#include "zitemfactory.h"
#include "util/uihelper.h"
#include "model/parammodel.h"
#include "declmetatype.h"


ZSocketLayout::ZSocketLayout(const QPersistentModelIndex& viewSockIdx, bool bInput, SocketBackgroud* parentItem)
    : ZGraphicsLayout(true)
    , m_text(nullptr)
    , m_control(nullptr)
    , m_socket(nullptr)
    , m_bInput(bInput)
    , m_bEditable(false)
    , m_paramIdx(viewSockIdx)
    , m_parentItem(parentItem)
{
    //TODO: deprecated or refactor.
#if 0
    QObject::connect(pModel, &IGraphsModel::updateCommandParamSignal, [=](const QString& path) {
        if (!m_text)
            return;
        QString socketPath = m_viewSockIdx.data(QtRole::ROLE_OBJPATH).toString();
        if (socketPath != path)
            return;
        m_text->update();
    });
#endif
}

ZSocketLayout::~ZSocketLayout()
{
}

void ZSocketLayout::initUI(const CallbackForSocket& cbSock)
{
    QString sockName;
    QString toolTip;
    zeno::SocketProperty sockProp = zeno::Socket_Normal;
    bool bEnableNode = false;
    if (!m_paramIdx.isValid())
    {
        //test case.
        sockName = "test";
    }
    else
    {
        sockName = m_paramIdx.data(QtRole::ROLE_PARAM_NAME).toString();
        sockProp = m_paramIdx.data(QtRole::ROLE_PARAM_SOCKPROP).value<zeno::SocketProperty>();
        m_bEditable = sockProp & SOCKPROP_EDITABLE;
        toolTip = m_paramIdx.data(QtRole::ROLE_PARAM_TOOLTIP).toString();

        QModelIndex nodeIdx = m_paramIdx.data(QtRole::ROLE_NODE_IDX).toModelIndex();
        if (nodeIdx.data(QtRole::ROLE_NODETYPE) != zeno::NoVersionNode &&
            SOCKPROP_LEGACY != m_paramIdx.data(QtRole::ROLE_PARAM_SOCKPROP))
        {
            bEnableNode = true;
        }
    }

    setContentsMargin(0, 15, 0, 0);

    QSizeF szSocket(14, 14);//(10, 20);
    m_socket = new ZenoSocketItem(m_paramIdx, ZenoStyle::dpiScaledSize(szSocket));
    zeno::SocketType sockType = (zeno::SocketType)m_paramIdx.data(QtRole::ROLE_SOCKET_TYPE).toInt();
    m_socket->setVisible(sockType != zeno::NoSocket && sockProp != zeno::Socket_Disable);
    m_socket->setZValue(ZVALUE_ELEMENT);
    m_socket->setEnabled(bEnableNode);
    if (bEnableNode) {
        QObject::connect(m_socket, &ZenoSocketItem::clicked, [=](bool bInput) {
            cbSock.cbOnSockClicked(m_socket);
        });
        if (ParamsModel* paramsM = QVariantPtr<ParamsModel>::asPtr(m_paramIdx.data(QtRole::ROLE_NODE_IDX).value<QModelIndex>().data(QtRole::ROLE_PARAMS))) {
            //TODO: 范围大，可以考虑根据是否显示决定优化
            QObject::connect(paramsM, &QStandardItemModel::dataChanged, m_socket, &ZenoSocketItem::onCustomParamDataChanged);
        }
    }

    if (m_bEditable && bEnableNode)
    {
        Callback_EditContentsChange cbFuncRenameSock = [=](QString oldText, QString newText) {
            UiHelper::qIndexSetData(m_paramIdx, newText, QtRole::ROLE_PARAM_NAME);
        };
        ZSocketEditableItem *pItem = new ZSocketEditableItem(m_paramIdx, sockName, m_bInput, cbSock.cbOnSockClicked, cbFuncRenameSock);
        if (!m_bInput) 
        {
            pItem->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        }
        m_text = pItem;
    }
    else
    {
        m_text = new ZSocketPlainTextItem(m_paramIdx, sockName, m_bInput, cbSock.cbOnSockClicked);
        m_text->setData(GVKEY_SIZEHINT, ZenoStyle::dpiScaledSize(QSizeF(0, zenoui::g_ctrlHeight)));
        m_text->setData(GVKEY_SIZEPOLICY, QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    }
    m_text->setToolTip(toolTip);

    m_parentItem->setSocketItem(m_socket);

    if (m_bInput)
    {
        //addItem(m_socket, Qt::AlignVCenter);
        addItem(m_text, Qt::AlignVCenter);
        int xoffset = 0;
        int yoffset = 0;
        m_socket->setPos(xoffset, yoffset);
    }
    else
    {
        addSpacing(-1, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        addItem(m_text, Qt::AlignVCenter);
        int xoffset = 0;//12 + m_text->boundingRect().width();
        int yoffset = 0;//4;
        m_socket->setPos(xoffset, yoffset);
        //addItem(m_socket, Qt::AlignVCenter);
    }

    setSpacing(ZenoStyle::dpiScaled(8));
}

void ZSocketLayout::setControl(QGraphicsItem* pControl)
{
    removeItem(m_control);
    m_control = pControl;

    Qt::Alignment align = (Qt::Alignment)pControl->data(GVKEY_ALIGNMENT).toInt();
    if (align == Qt::AlignLeft)
        addItem(m_control, Qt::AlignLeft | Qt::AlignVCenter);
    else
        addItem(m_control, Qt::AlignVCenter);//, Qt::AlignRight);
}

QGraphicsItem* ZSocketLayout::socketText() const
{
    return m_text;
}

QGraphicsItem* ZSocketLayout::control() const
{
    return m_control;
}

ZenoSocketItem* ZSocketLayout::socketItem() const
{
    return m_socket;
}

QPointF ZSocketLayout::getSocketPos(const QModelIndex& sockIdx, const QString keyName, bool& exist)
{
    exist = false;
    if (m_paramIdx == sockIdx && m_socket)
    {
        exist = true;
        return m_socket->center();
    }
    return QPointF();
}

ZenoSocketItem* ZSocketLayout::socketItemByIdx(const QModelIndex& sockIdx, const QString keyName) const
{
    if (m_paramIdx == sockIdx)
    {
        return m_socket;
    }
    return nullptr;
}

QPersistentModelIndex ZSocketLayout::viewSocketIdx() const
{
    return m_paramIdx;
}

void ZSocketLayout::setSocketVisible(bool bVisible)
{
    m_socket->setVisible(bVisible);
}

void ZSocketLayout::setVisible(bool bVisible) 
{
    if (m_socket->sockStatus() != ZenoSocketItem::STATUS_CONNECTED && m_control) {
        m_control->setVisible(bVisible);
    }
    if (m_text) {
        m_text->setVisible(bVisible);
    }
}

void ZSocketLayout::updateSockName(const QString& name)
{
    if (m_bEditable)
    {
        ZSocketEditableItem* pEdit = static_cast<ZSocketEditableItem*>(m_text);
        if (pEdit)
            pEdit->updateSockName(name);
    }
    else
    {
        ZSocketPlainTextItem* pGroup = static_cast<ZSocketPlainTextItem*>(m_text);
        if (pGroup)
            pGroup->setText(name);
    }
}

void ZSocketLayout::updateSockNameToolTip(const QString &tip) 
{
    if (m_text)
        m_text->setToolTip(tip);
}


////////////////////////////////////////////////////////////////////////////////////
ZGroupSocketLayout::ZGroupSocketLayout(const QPersistentModelIndex &viewSockIdx, bool bInput) :
    ZSocketLayout(viewSockIdx, bInput, nullptr)
{
}

ZGroupSocketLayout::~ZGroupSocketLayout() {
}

void ZGroupSocketLayout::initUI(const CallbackForSocket &cbSock) 
{
    setContentsMargin(0, ZenoStyle::dpiScaled(6), 0, 0);
    bool bInput = m_paramIdx.data(QtRole::ROLE_ISINPUT).toBool();
    const QString &name = m_paramIdx.data(QtRole::ROLE_PARAM_NAME).toString();

    m_pGroupLine = new ZenoParamGroupLine(name);
    m_pGroupLine->setData(GVKEY_SIZEHINT, ZenoStyle::dpiScaledSize(QSizeF(0, 32)));
    m_pGroupLine->setData(GVKEY_SIZEPOLICY, QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));    
    addItem(m_pGroupLine);
}

ZenoSocketItem *ZGroupSocketLayout::socketItemByIdx(const QModelIndex &sockIdx, const QString keyName) const {
    return nullptr;
}

QPointF ZGroupSocketLayout::getSocketPos(const QModelIndex &sockIdx, const QString keyName, bool &exist) {
    return QPointF();
}

void ZGroupSocketLayout::updateSockName(const QString &name) 
{
    m_pGroupLine->setText(name);
}

void ZGroupSocketLayout::setVisible(bool bVisible) 
{
    m_pGroupLine->setVisible(bVisible);
}
