#include "zcodeeditor.h"
#include <QGLSLCompleter>
#include <QLuaCompleter>
#include "ZPythonCompleter.h"
#include <QZfxHighlighter>
#include <QGLSLHighlighter>
#include <QXMLHighlighter>
#include <QJSONHighlighter>
#include <QLuaHighlighter>
#include <QPythonHighlighter>
#include <QSyntaxStyle>
#include <zeno/core/data.h>
#include <zeno/core/Session.h>
#include <zeno/core/FunctionManager.h>
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include "style/zenostyle.h"


ZCodeEditor::ZCodeEditor(const QString& text, QWidget *parent)
    : QCodeEditor(parent), m_zfxHighLighter(new QZfxHighlighter), m_descLabel(nullptr)
{
    setCompleter(new ZPythonCompleter(this));
    setHighlighter(m_zfxHighLighter);
    setText(text);
    initUI();

    connect(this, &QTextEdit::cursorPositionChanged, this, &ZCodeEditor::slt_showFuncDesc);
}

void ZCodeEditor::setFuncDescLabel(ZenoFuncDescriptionLabel* descLabel)
{
    m_descLabel = descLabel;
}

void ZCodeEditor::focusOutEvent(QFocusEvent* e)
{
    QCodeEditor::focusOutEvent(e);
    Qt::FocusReason reason = e->reason();
    if (reason != Qt::ActiveWindowFocusReason)
        emit editFinished(toPlainText());
}

void ZCodeEditor::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape && m_descLabel->isVisible()) {
        m_descLabel->hide();
    }
    QCodeEditor::keyPressEvent(e);
}

void ZCodeEditor::slt_showFuncDesc()
{
    if (!m_descLabel)
        return;

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString currLine = cursor.selectedText();
    int positionInLine = textCursor().positionInBlock();

    QRegularExpression functionPattern = (QRegularExpression(R"(\b([_a-zA-Z][_a-zA-Z0-9]*\s+)?((?:[_a-zA-Z][_a-zA-Z0-9]*\s*::\s*)*[_a-zA-Z][_a-zA-Z0-9]*)(?=\s*\())"));
    auto matchIterator = functionPattern.globalMatch(currLine);
    while (matchIterator.hasNext())
    {
        auto match = matchIterator.next();
        if (match.capturedStart(2) + match.capturedLength(2) + 1 == positionInLine) {
            zeno::FUNC_INFO info = zeno::getSession().funcManager->getFuncInfo(currLine.mid(match.capturedStart(2), match.capturedLength(2)).toStdString());
            assert(zenoApp->getMainWindow());
            if (!info.name.empty()) {
                m_descLabel->setDesc(info, 0);
                m_descLabel->setCurrentFuncName(info.name);

                m_descLabel->updateParent();
                //m_descLabel->move(m_descLabel->calculateNewPos(this, currLine.left(positionInLine)));
                auto cursRect = cursorRect();
                auto globalpos = this->mapTo(zenoApp->getMainWindow(), { cursRect.x() + cursRect.width() + qCeil(ZenoStyle::dpiScaled(30)), cursRect.y() + cursRect.height()});
                if (zenoApp->getMainWindow()->width() < globalpos.x() + m_descLabel->width()) {
                    globalpos.setX(zenoApp->getMainWindow()->width() - m_descLabel->width());
                }
                if (zenoApp->getMainWindow()->height() < globalpos.y() + m_descLabel->height()) {
                    globalpos.setY(zenoApp->getMainWindow()->height() - m_descLabel->height());
                }
                m_descLabel->move(globalpos);
                m_descLabel->show();
                return;
            }
        }
    }
    if (m_descLabel->isVisible()) {
        //m_descLabel->hide();
    }
}

void ZCodeEditor::initUI()
{
    setSyntaxStyle(loadStyle(":/stylesheet/drakula.xml"));
    setWordWrapMode(QTextOption::WordWrap);
    setAutoIndentation(true);
    setTabReplace(true);
    setTabReplaceSize(4);
}

QSyntaxStyle* ZCodeEditor::loadStyle(const QString& path)
{
    QFile fl(path);

    if (!fl.open(QIODevice::ReadOnly))
    {
        return QSyntaxStyle::defaultStyle();
    }

    auto style = new QSyntaxStyle(this);

    if (!style->load(fl.readAll()))
    {
        delete style;
        return QSyntaxStyle::defaultStyle();
    }

    return style;
}