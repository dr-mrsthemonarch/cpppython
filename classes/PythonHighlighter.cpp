#include "../classes/PythonHighlighter.h"

PythonHighlighter::PythonHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    // Keywords
    keywordFormat.setForeground(QColor(86, 156, 214));  // Blue
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bdef\\b"), QStringLiteral("\\bclass\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"),
        QStringLiteral("\\belif\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\btry\\b"),
        QStringLiteral("\\bexcept\\b"), QStringLiteral("\\bfinally\\b"),
        QStringLiteral("\\breturn\\b"), QStringLiteral("\\bbreak\\b"),
        QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\bpass\\b"),
        QStringLiteral("\\braise\\b"), QStringLiteral("\\bin\\b"),
        QStringLiteral("\\bis\\b"), QStringLiteral("\\bNone\\b"),
        QStringLiteral("\\bTrue\\b"), QStringLiteral("\\bFalse\\b"),
        QStringLiteral("\\band\\b"), QStringLiteral("\\bor\\b"),
        QStringLiteral("\\bnot\\b"), QStringLiteral("\\bwith\\b"),
        QStringLiteral("\\bas\\b"), QStringLiteral("\\bassert\\b"),
        QStringLiteral("\\bimport\\b"), QStringLiteral("\\bfrom\\b"),
        QStringLiteral("\\bglobal\\b"), QStringLiteral("\\bnonlocal\\b")
    };

    for (const QString& pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Functions
    functionFormat.setForeground(QColor(220, 220, 170));  // Light yellow
    HighlightingRule rule;
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Class names
    classFormat.setForeground(QColor(78, 201, 176));  // Teal
    classFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("\\bclass\\s+[A-Za-z0-9_]+\\b"));
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Strings
    stringFormat.setForeground(QColor(214, 157, 133));  // Light red
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = stringFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    highlightingRules.append(rule);

    // Numbers
    numberFormat.setForeground(QColor(181, 206, 168));  // Light green
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Comments
    commentFormat.setForeground(QColor(87, 166, 74));  // Green
    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

void PythonHighlighter::highlightBlock(const QString& text) {
    for (const HighlightingRule& rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}