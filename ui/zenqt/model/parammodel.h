#ifndef __PARAMS_MODEL_H__
#define __PARAMS_MODEL_H__

#include <QObject>
#include <QStandardItemModel>
#include <QQuickItem>
#include "uicommon.h"

class GraphModel;
class ParamModelImpl;

namespace zeno
{
    class INode;
}

struct ParamItem
{
    //BEGIN: temp cache on ui model, the actual value has been stored in m_wpParam.
    QString name;
    zeno::ParamType type = Param_Null;
    zeno::reflect::Any value;
    //END
    //std::weak_ptr<zeno::CoreParam> m_wpParam;

    bool bInput = true;
    zeno::ParamControl control = zeno::NullControl;
    zeno::SocketType connectProp = zeno::NoSocket;
    zeno::SocketProperty sockProp;
    zeno::NodeDataGroup group;
    zeno::reflect::Any optCtrlprops;
    QList<QPersistentModelIndex> links;
    bool bSocketVisible = true;
    bool bVisible = true;
    bool bEnable = true;
    bool bWildcard = false;
};

class ParamsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    ParamsModel(zeno::INode* spNode, QObject* parent = nullptr);

    Q_INVOKABLE int indexFromName(const QString& name, bool bInput) const;
    Q_INVOKABLE QVariant getIndexList(bool bInput) const;
    Q_INVOKABLE QStandardItemModel* customParamModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    void getDegrees(int& inDegrees, int& outDegrees);
    bool hasVisiblePrimParam() const;

    //api:
    void setNodeIdx(const QModelIndex& nodeIdx);
    QModelIndex paramIdx(const QString& name, bool bInput) const;
    void addLink(const QModelIndex& paramIdx, const QPersistentModelIndex& linkIdx);
    int removeLink(const QModelIndex& paramIdx);
    QModelIndex removeOneLink(const QModelIndex& paramIdx, const zeno::EdgeInfo& link);
    bool removeSpecificLink(const QModelIndex& paramIdx, const QModelIndex& linkIdx);
    void addParam(const ParamItem& param);
    GraphModel* getGraph() const;

    PARAMS_INFO getInputs();
    PARAMS_INFO getOutputs();

    //temp:

    void batchModifyParams(const zeno::ParamsUpdateInfo& params, bool bSubnetInit = false);
    void updateUiLinksSockets(zeno::params_change_info& changes);
    void resetCustomUi(const zeno::CustomUI& customui);

    int getParamlinkCount(const QModelIndex& paramIdx);
    int numOfInputParams() const;

signals:
    void linkAboutToBeInserted(const zeno::EdgeInfo& link);
    void linkAboutToBeRemoved(const zeno::EdgeInfo& link);
    void portTypeChanged(const QModelIndex& idx, bool bPrime);
    void enabledVisibleChanged();

private:
    void initParamItems();
    void initCustomUI(const zeno::CustomUI& customui);
    void updateCustomUiModelIncremental(const zeno::params_change_info& params, const zeno::CustomUI& customui); //增量更新m_customParamsM，防止zenoproppanl接收不到数据
    GraphModel* parentGraph() const;
    void test_customparamsmodel() const;
    void updateParamData(const QString& name, const QVariant& val, int role, bool bInput = true);
    QStandardItemModel* constructProxyModel();

    QPersistentModelIndex m_nodeIdx;
    QVector<ParamItem> m_items;

    QStandardItemModel* m_customParamsM;

    zeno::INode* m_wpNode;    //直接用裸指针，反正如果核心没了这个model肯定也没了
    std::string cbUpdateParam;
    mutable bool m_bReentry = false;
};


#endif