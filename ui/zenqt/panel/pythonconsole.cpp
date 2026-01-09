#include "pythonconsole.h"
#include <QTextCursor>
#include <QTextObject>
#include <QScrollBar>
#include <zeno/core/Session.h>
#include "pythonhighlighter.h"


PythonConsole::PythonConsole(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setUndoRedoEnabled(false);
    setWordWrapMode(QTextOption::NoWrap);
    setFont(QFont("Consolas", 11));

    appendPrompt();

    new PythonHighlighter(this->document());

    // ======= 终端暗色主题 =======
    QPalette p = this->palette();

    // 背景色（几乎黑，但不是纯黑，否则对比太强）
    p.setColor(QPalette::Base, QColor(32, 32, 32));  // #202020

    // 文本颜色（略带暖的白）
    p.setColor(QPalette::Text, QColor(220, 220, 220));  // #DCDCDC

    // 光标颜色
    p.setColor(QPalette::Highlight, QColor(90, 150, 255));   // 光标选区
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));

    this->setPalette(p);

    // 去掉边框，让终端更“贴合”
    this->setStyleSheet(R"(
    QPlainTextEdit {
        border: 0px;
        padding: 6px;
        background: #202020;
        color: #DCDCDC;
        selection-background-color: #3a70c7;
        selection-color: #ffffff;
    }
)");

}

void PythonConsole::appendPrompt()
{
    if (m_needMoreInput)
        appendPlainText("... ");
    else
        appendPlainText(">>> ");

    moveCursor(QTextCursor::End);
}

QString PythonConsole::getCurrentInput() const
{
    QTextCursor c = textCursor();
    c.block();
    QString line = c.block().text();

    if (line.startsWith(">>> "))
        return line.mid(4);
    else if (line.startsWith("... "))
        return line.mid(4);

    return line;
}

void PythonConsole::replaceCurrentLine(const QString& text)
{
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::StartOfBlock);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    if (m_needMoreInput)
        c.insertText("... " + text);
    else
        c.insertText(">>> " + text);
}

bool PythonConsole::cursorInEditableArea() const {
    QTextCursor c = textCursor();
    if (!isInLastBlock(c))
        return false;  // 只有最后一行可编辑

    int pos = c.positionInBlock();
    QString line = c.block().text();

    if (line.startsWith(">>> "))
        return pos >= 4;
    if (line.startsWith("... "))
        return pos >= 4;

    return false;
}

bool PythonConsole::isInLastBlock(const QTextCursor& c) const
{
    return c.block() == document()->lastBlock();
}

bool PythonConsole::selectionValid() const
{
    QTextCursor c = textCursor();
    if (!c.hasSelection())
        return true;

    QTextCursor c1 = c;
    c1.setPosition(c.selectionStart());
    QTextCursor c2 = c;
    c2.setPosition(c.selectionEnd());

    auto isEditablePos = [&](const QTextCursor& tc) {
        if (!isInLastBlock(tc))
            return false;

        QString line = tc.block().text();
        int pos = tc.positionInBlock();

        if (line.startsWith(">>> ") || line.startsWith("... "))
            return pos >= 4;

        return false;
        };

    return isEditablePos(c1) && isEditablePos(c2);
}

QString PythonConsole::extractCompletionPrefix()
{
    QTextCursor c = textCursor();
    QString line = c.block().text();

    // 去掉提示符
    if (line.startsWith(">>> "))
        line = line.mid(4);
    else if (line.startsWith("... "))
        line = line.mid(4);

    // 获取光标位置之后的部分无意义，截掉
    int pos = c.positionInBlock();
    pos -= 4; // offset by '>>> ' or '... '

    if (pos < 0 || pos > line.length())
        pos = line.length();

    QString left = line.left(pos);

    /*
        我们支持解析以下前缀：
        - 变量名：      abc
        - 属性链：      obj.field
        - 方法链：      zen.mainGraph().createNode
        - 内置对象：    str.isd
    */

    // 正则：取最后的 (标识符 + 点 + 标识符 + 点 + ... + 标识符)
    QRegularExpression re("([A-Za-z_][A-Za-z0-9_\\.]*$)");
    QRegularExpressionMatch m = re.match(left);

    if (m.hasMatch()) {
        return m.captured(1);
    }

    return "";
}

void PythonConsole::completeFromList()
{
    if (!m_completionListWidget || m_completionListWidget->currentItem() == nullptr)
        return;

    QString text = m_completionListWidget->currentItem()->text();

    QTextCursor c = this->textCursor();

    // 删除现有的 prefix
    QString prefix = extractCompletionPrefix();
    if (!prefix.isEmpty()) {
        c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, prefix.size());
        c.removeSelectedText();
    }

    c.insertText(text);
    this->setTextCursor(c);

    m_completionListWidget->hide();
}

void PythonConsole::updateCompletionFilter()
{
    QString prefix = extractCompletionPrefix().toLower();

    // 从 ListWidget 获取所有选项
    QList<QString> allItems;
    for (int i = 0; i < m_completionListWidget->count(); i++)
        allItems.append(m_completionListWidget->item(i)->text());

    m_completionListWidget->clear();

    for (auto& s : allItems) {
        if (s.toLower().contains(prefix))
            m_completionListWidget->addItem(s);
    }

    if (m_completionListWidget->count() == 0)
        m_completionListWidget->hide();
    else
        m_completionListWidget->setCurrentRow(0);
}

void PythonConsole::showCompletionList(const std::vector<std::string>& items)
{
    if (items.empty())
        return;

    if (!m_completionListWidget) {
        m_completionListWidget = new QListWidget(this);
        m_completionListWidget->setWindowFlags(Qt::Popup);
        m_completionListWidget->setMouseTracking(true);

        // 点击补全
        connect(m_completionListWidget, &QListWidget::itemClicked,
            [this](QListWidgetItem*) {
                completeFromList();
            });
    }

    m_completionListWidget->clear();
    for (auto& s : items)
        m_completionListWidget->addItem(QString::fromStdString(s));

    // 默认选中第一项
    m_completionListWidget->setCurrentRow(0);

    // 放在光标下面
    QRect r = cursorRect();
    QPoint p = mapToGlobal(r.bottomLeft());
    m_completionListWidget->move(p);

    m_completionListWidget->resize(260, 200);

    m_completionListWidget->show();
    m_completionListWidget->raise();
}

void PythonConsole::keyPressEvent(QKeyEvent* e)
{
    // ========= 触发补全：输入 "." =========
    if (0 && e->text() == ".") {
        QPlainTextEdit::keyPressEvent(e); // 先插入“.”

        QString prefix = extractCompletionPrefix();

        std::vector<std::string> out;
        zeno::getSession().completePython(prefix.toStdString(), out);

        showCompletionList(out);
        return;
    }

    // ========= 输入字符后动态更新列表 =========
    if (0 && !e->text().isEmpty() && m_completionListWidget && m_completionListWidget->isVisible()) {
        QPlainTextEdit::keyPressEvent(e); // 先插入字符
        updateCompletionFilter();
        return;
    }

    // ========= 上下键导航补全项 =========
    if (0 && m_completionListWidget && m_completionListWidget->isVisible()) {
        if (e->key() == Qt::Key_Up) {
            int row = m_completionListWidget->currentRow();
            m_completionListWidget->setCurrentRow(qMax(0, row - 1));
            return;
        }
        if (e->key() == Qt::Key_Down) {
            int row = m_completionListWidget->currentRow();
            m_completionListWidget->setCurrentRow(qMin(m_completionListWidget->count() - 1, row + 1));
            return;
        }
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            completeFromList();
            return;
        }
    }

    // --- Backspace 保护 ---
    if (e->key() == Qt::Key_Backspace) {
        QTextCursor c = textCursor();
        QString line = c.block().text();
        int pos = c.positionInBlock();

        // 不是最后一行，禁止删
        if (!isInLastBlock(c)) {
            e->accept();
            return;
        }

        // 光标在提示符区域，禁止删
        bool atPrompt = (line.startsWith(">>> ") && pos <= 4)
            || (line.startsWith("... ") && pos <= 4);

        if (atPrompt || !selectionValid()) {
            e->accept();
            return;
        }
    }

    // --- Delete 保护 ---
    if (e->key() == Qt::Key_Delete) {
        QTextCursor c = textCursor();
        QString line = c.block().text();
        int pos = c.positionInBlock();

        if (!isInLastBlock(c)) {
            e->accept();
            return;
        }

        bool atPrompt = (line.startsWith(">>> ") && pos < 4)
            || (line.startsWith("... ") && pos < 4);

        if (atPrompt || !selectionValid()) {
            e->accept();
            return;
        }
    }

    // Enter/执行
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        QString code = getCurrentInput();
        appendPlainText("");   // 换行

        if (!code.isEmpty()) {
            m_history.append(code);
            m_historyIndex = m_history.size();
        }

        bool needMore = false;
        std::string output;

        bool ok = zeno::getSession().runPythonInteractive(
            code.toStdString(),
            needMore,
            output
        );

        // 显示输出
        if (!output.empty()) {
            this->appendPlainText(QString::fromUtf8(output.c_str()));
        }

        m_needMoreInput = needMore;

        appendPrompt();
        return;
    }

    // Up：历史上一条
    if (e->key() == Qt::Key_Up)
    {
        if (m_history.isEmpty()) return;
        if (m_historyIndex > 0) m_historyIndex--;

        replaceCurrentLine(m_history[m_historyIndex]);
        return;
    }

    // Down：历史下一条
    if (e->key() == Qt::Key_Down)
    {
        if (m_history.isEmpty()) return;
        if (m_historyIndex < m_history.size() - 1) m_historyIndex++;

        replaceCurrentLine(m_history[m_historyIndex]);
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}
