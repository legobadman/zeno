#include "pythonhighlighter.h"

PythonHighlighter::PythonHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // ========== 配色方案（适合暗色背景） ==========
    keywordFormat.setForeground(QColor("#569CD6"));    // 蓝色
    stringFormat.setForeground(QColor("#D69D85"));     // 沙色
    commentFormat.setForeground(QColor("#6A9955"));    // 绿色
    numberFormat.setForeground(QColor("#B5CEA8"));     // 淡绿
    builtinFormat.setForeground(QColor("#4EC9B0"));    // 青绿

    classFormat.setForeground(QColor("#4EC9B0"));
    classFormat.setFontWeight(QFont::Bold);

    funcFormat.setForeground(QColor("#DCDCAA"));

    // ========== Python 关键字 ==========
    QStringList keywords = {
        "False", "class", "finally", "is", "return",
        "None", "continue", "for", "lambda", "try",
        "True", "def", "from", "nonlocal", "while",
        "and", "del", "global", "not", "with",
        "as", "elif", "if", "or", "yield",
        "assert", "else", "import", "pass",
        "break", "except", "in", "raise"
    };

    for (const QString& kw : keywords) {
        HighlightRule r;
        r.pattern = QRegularExpression("\\b" + kw + "\\b");
        r.format = keywordFormat;
        rules.append(r);
    }

    // ========== Python 内置函数 ==========
    QStringList builtins = {
        "print", "len", "range", "str", "int", "float", "type",
        "dir", "list", "dict", "set", "tuple", "open", "isinstance"
    };

    for (const QString& b : builtins) {
        HighlightRule r;
        r.pattern = QRegularExpression("\\b" + b + "\\b");
        r.format = builtinFormat;
        rules.append(r);
    }

    // ========== 类名 ==========
    rules.append({
        QRegularExpression("\\bclass\\s+(\\w+)"),
        classFormat
    });

    // ========== 函数名 ==========
    rules.append({
        QRegularExpression("\\bdef\\s+(\\w+)"),
        funcFormat
    });

    // ========== 字符串 ==========
    rules.append({
        QRegularExpression(R"("(?:[^"\\]|\\.)*")"),
        stringFormat
    });
    rules.append({
        QRegularExpression(R"('(?:[^'\\]|\\.)*')"),
        stringFormat
    });

    // 三引号字符串
    tripleSingle = QRegularExpression("'''");
    tripleDouble = QRegularExpression("\"\"\"");

    // ========== 注释 ==========
    rules.append({
        QRegularExpression("#[^\n]*"),
        commentFormat
    });

    // ========== 数字 ==========
    rules.append({
        QRegularExpression("\\b[0-9]+\\b"),
        numberFormat
    });
}

void PythonHighlighter::highlightBlock(const QString& text)
{
    // 普通规则
    for (const HighlightRule& r : rules)
        applyFormat(text, r.pattern, r.format);

    // 三引号字符串（多行）
    setCurrentBlockState(0);

    int start = 0;
    if (previousBlockState() == 1) {
        QRegularExpression end("'''");
        auto m = end.match(text);
        if (!m.hasMatch()) {
            setFormat(0, text.length(), stringFormat);
            setCurrentBlockState(1);
            return;
        } else {
            int endIndex = m.capturedStart();
            setFormat(0, endIndex + 3, stringFormat);
            start = endIndex + 3;
        }
    }

    while (true) {
        auto m = tripleSingle.match(text, start);
        if (!m.hasMatch()) break;
        int startIndex = m.capturedStart();

        auto m2 = tripleSingle.match(text, startIndex + 3);
        int endIndex = m2.hasMatch() ? m2.capturedStart() : -1;

        if (endIndex == -1) {
            setFormat(startIndex, text.length() - startIndex, stringFormat);
            setCurrentBlockState(1);
            return;
        } else {
            setFormat(startIndex, endIndex - startIndex + 3, stringFormat);
            start = endIndex + 3;
        }
    }
}

void PythonHighlighter::applyFormat(const QString& text, const QRegularExpression& pattern, const QTextCharFormat& fmt)
{
    auto it = pattern.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), fmt);
    }
}
