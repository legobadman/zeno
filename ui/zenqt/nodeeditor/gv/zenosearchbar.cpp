#include "zenosearchbar.h"
#include <widgets/ziconbutton.h>
#include "uicommon.h"
#include "style/zenostyle.h"
#include "zenoapplication.h"
#include "model/graphsmanager.h"
#include "model/GraphModel.h"
#include "zassert.h"


ZenoSearchBar::ZenoSearchBar(GraphModel* pModel, QWidget *parentWidget)
    : QWidget(parentWidget)
    , m_idx(0)
    , m_pGraphM(pModel)
{
    setWindowFlag(Qt::SubWindow);
    setWindowFlag(Qt::FramelessWindowHint);

    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(36, 36, 36));
    setPalette(pal);

    m_pLineEdit = new QLineEdit;
    m_pLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_pLineEdit->setObjectName("searchEdit");
    m_pLineEdit->setFixedWidth(200);
    QFont font = QApplication::font();
    m_pLineEdit->setFont(font);
    ZIconButton *pCloseBtn = new ZIconButton(
        QIcon(":/icons/closebtn.svg"),
        ZenoStyle::dpiScaledSize(QSize(20, 20)),
        QColor(61, 61, 61),
        QColor(66, 66, 66));
    ZIconButton *pSearchBackward = new ZIconButton(
        QIcon(":/icons/search_arrow_backward.svg"),
        ZenoStyle::dpiScaledSize(QSize(20, 20)),
        QColor(61, 61, 61),
        QColor(66, 66, 66));
    ZIconButton *pSearchForward = new ZIconButton(
        QIcon(":/icons/search_arrow.svg"),
        ZenoStyle::dpiScaledSize(QSize(20, 20)),
        QColor(61, 61, 61),
        QColor(66, 66, 66));
    QHBoxLayout *pEditLayout = new QHBoxLayout;
    
    pEditLayout->addWidget(m_pLineEdit);
    pEditLayout->addWidget(pSearchBackward);
    pEditLayout->addWidget(pSearchForward);
    pEditLayout->addWidget(pCloseBtn);
    pEditLayout->setContentsMargins(
        ZenoStyle::dpiScaled(10),
        ZenoStyle::dpiScaled(6),
        ZenoStyle::dpiScaled(10),
        ZenoStyle::dpiScaled(6));

    setLayout(pEditLayout);

    connect(m_pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onSearchExec(const QString&)));
    connect(pSearchForward, SIGNAL(clicked()), this, SLOT(onSearchForward()));
    connect(pSearchBackward, SIGNAL(clicked()), this, SLOT(onSearchBackward()));
    connect(pCloseBtn, SIGNAL(clicked()), this, SLOT(close()));
}

SEARCH_RECORD ZenoSearchBar::_getRecord()
{
    QModelIndex idx = m_results[m_idx];
    const QString &nodeid = idx.data(QtRole::ROLE_NODE_NAME).toString();
    const QPointF &pos = idx.data(QtRole::ROLE_OBJPOS).toPointF();
    return {nodeid, pos};
}

void ZenoSearchBar::onSearchExec(const QString& content)
{
    if (content.isEmpty()) {
        return;
    }

    ZASSERT_EXIT(m_pGraphM);
    QList<SEARCH_RESULT> results = m_pGraphM->search(content, SEARCH_ARGS | SEARCH_NODECLS | SEARCH_NODEID | SEARCH_CUSTOM_NAME, SEARCH_FUZZ);

    for (auto res : results)
    {
        m_results.append(res.targetIdx);
    }
    if (!m_results.isEmpty())
    {
        m_idx = 0;
        SEARCH_RECORD rec = _getRecord();
        emit searchReached(rec);
    }
}

void ZenoSearchBar::onSearchForward()
{
    if (++m_idx == m_results.size())
        m_idx = 0;
    if (!m_results.isEmpty() && m_idx < m_results.size())
    {
        SEARCH_RECORD rec = _getRecord();
        emit searchReached(rec);
    }
}

void ZenoSearchBar::onSearchBackward()
{
    if (--m_idx < 0)
        m_idx = m_results.size() - 1;

    if (!m_results.isEmpty() && m_idx < m_results.size())
    {
        SEARCH_RECORD rec = _getRecord();
        emit searchReached(rec);
    }
}

void ZenoSearchBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (QWidget* par = parentWidget())
    {
		QSize sz = event->size();
		int w = par->width();
		int h = par->height();
		setGeometry(w - sz.width(), 0, sz.width(), sz.height());
    }
}

void ZenoSearchBar::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
    if (event->key() == Qt::Key_Escape)
    {
        hide();
    }
    else if (event->key() == Qt::Key_F3)
    {
        onSearchForward();
    }
    else if ((event->modifiers() & Qt::ShiftModifier) && event->key() == Qt::Key_F3)
    {
        onSearchBackward();
    }
}

void ZenoSearchBar::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_pLineEdit->setFocus();
}

void ZenoSearchBar::activate()
{
    show();
    m_pLineEdit->setFocus();
}
