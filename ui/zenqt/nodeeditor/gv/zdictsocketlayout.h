#ifndef __ZDICTSOCKETLAYOUT_H__
#define __ZDICTSOCKETLAYOUT_H__

#include "zsocketlayout.h"

class ZenoImageItem;
class ZDictPanel;
class ZLayoutBackground;
class SocketBackgroud;

class ZDictSocketLayout : public ZSocketLayout
{
public:
    ZDictSocketLayout(const QPersistentModelIndex& paramIdx, bool bInput, SocketBackgroud* parentItem);
    ~ZDictSocketLayout();
    void initUI(const CallbackForSocket& cbSock) override;
    ZenoSocketItem* socketItemByIdx(const QModelIndex& sockIdx, const QString keyName) const override;
    QPointF getSocketPos(const QModelIndex& sockIdx, const QString keyName, bool& exist) override;
    void setCollasped(bool bCollasped);
    void setVisible(bool bVisible);
    QGraphicsItem* control() const override;

private:
    ZDictPanel* m_panel;
    ZenoImageItem* m_collaspeBtn;
};

#endif