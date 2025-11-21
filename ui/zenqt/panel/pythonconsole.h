#pragma once

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QStringList>
#include <QListWidget>

class PythonConsole : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit PythonConsole(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    void appendPrompt();
    QString getCurrentInput() const;   // 获取当前行用户输入的部分
    void replaceCurrentLine(const QString& text);
    bool cursorInEditableArea() const;
    bool selectionValid() const;
    bool isInLastBlock(const QTextCursor& c) const;
    QString extractCompletionPrefix();
    void showCompletionList(const std::vector<std::string>& items);
    void updateCompletionFilter();
    void completeFromList();

private:
    QListWidget* m_completionListWidget = nullptr;
    bool m_needMoreInput = false;   // 是否进入多行模式（对应 ... 提示）
    QStringList m_history;
    int m_historyIndex = -1;
};
