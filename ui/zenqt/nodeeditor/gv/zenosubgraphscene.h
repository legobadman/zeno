#ifndef __ZENO_SUBGRAPH_SCENE_H__
#define __ZENO_SUBGRAPH_SCENE_H__

#include <QtWidgets>
#include <QUuid>
#include "util/ztfutil.h"
#include "nodeeditor/gv/nodesys_common.h"


class ZenoParamWidget;
class ZenoNodeBase;
class ZenoFullLink;
class ZenoTempLink;
class ZenoSocketItem;
class NodeGridItem;
class GraphModel;
class ZenoSubGraphView;

class ZenoSubGraphScene : public QGraphicsScene
{
	Q_OBJECT
public:
    ZenoSubGraphScene(QObject* parent = nullptr);
    ~ZenoSubGraphScene();
    void initModel(GraphModel* pGraphM);
    void undo();
    void redo();
    void copy();
    void save();
    void paste(QPointF pos);
    QRectF nodesBoundingRect() const;
    QModelIndex subGraphIndex() const;
    QModelIndexList selectNodesIndice() const;
    QModelIndexList selectLinkIndice() const;
    void select(const QString& id);
    void select(const std::vector<QString>& nodes);
    void select(const QStringList& nodes);
    void select(const QModelIndexList &nodes);
    QList<ZenoNodeBase*> getNodesItem() const;
    ZenoSubGraphView* getView();
    QGraphicsItem* getNode(const QString& nodeName);
    void collectNodeSelChanged(const QString& name, bool bSelected);
    GraphModel* getGraphModel() const;
    void onNodePositionChanged(const ZenoNodeBase* pNode);
    void rearrangeGraph(bool bHorional);

    // FIXME temp function for merge
    void selectObjViaNodes();
    void updateKeyFrame();
    void clearMark() {};

signals:
    void nodePosChanged(const ZenoNodeBase* pNode);
    void nodeInserted(const ZenoNodeBase* pNode);
    void nodeAboutToRemoved(const ZenoNodeBase* pNode);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

public slots:
    void onZoomed(qreal factor);

    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onViewTransformChanged(qreal factor);
    void onNameUpdated(const QModelIndex& nodeIdx, const QString& oldName);
    void onLinkInserted(const QModelIndex&, int first, int last);
    void onLinkAboutToBeRemoved(const QModelIndex&, int first, int last);

private slots:
    void reload(const QModelIndex& subGpIdx);
    void clearLayout(const QModelIndex& subGpIdx);
    void onSocketClicked(ZenoSocketItem* pSocketItem);
    void onNodePosChanged();

private:
    void afterSelectionChanged();
    void onSocketAbsorted(const QPointF& mousePos);
    void viewAddLink(const QModelIndex& linkIdx);
    void viewRemoveLink(const QModelIndex& linkIdx);
    void onTempLinkClosed();
    ZenoNodeBase* createNode(const QModelIndex& idx, const NodeUtilParam& params);
    void initLink(const QModelIndex& linkIdx);
    void updateNodeStatus(int option);
    bool isLinkValid(const ZenoSocketItem* fixedSockItem, const ZenoSocketItem* targetSockItem, bool bFixedInput);

    NodeUtilParam m_nodeParams;
    QPersistentModelIndex m_subgIdx;      //index to the subgraphmodel or node in "graphsModel"
    std::map<QString, ZenoNodeBase*> m_nodes;   //TODO: 是否可考虑用uuid?
    QStringList m_errNodes;        //the nodes which have been marked "error" at run time.
    QHash<QUuid, ZenoFullLink*> m_links;
    //QMap<QString, ZenoFullLink*> m_links;
    ZenoTempLink* m_tempLink;
    GraphModel* m_model;

    QVector<QPair<QString, bool>> m_selChanges;

    ZenoNodeBase* m_pUnfoldNode;
};

#endif
