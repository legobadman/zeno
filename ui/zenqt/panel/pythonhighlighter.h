#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class PythonHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit PythonHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> rules;

    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat builtinFormat;

    QTextCharFormat classFormat;
    QTextCharFormat funcFormat;

    QRegularExpression tripleSingle;
    QRegularExpression tripleDouble;

    void applyFormat(const QString& text, const QRegularExpression& pattern, const QTextCharFormat& fmt);
};
