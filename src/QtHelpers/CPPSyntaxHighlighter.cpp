#include "QtHelpers/CPPSyntaxHighlighter.h"
#include "pluginmain.h"

S2Plugin::CPPSyntaxHighlighter::CPPSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    mFormatReservedKeywords.setForeground(QColor("#569CD6"));
    mFormatVariables.setForeground(QColor("#9CDCFE"));
    mFormatTypes.setForeground(QColor("#4EC9B0"));
    mFormatComments.setForeground(QColor("#6A9955"));
    mFormatText.setForeground(QColor("#FFFFFF"));
    mFormatNumber.setForeground(QColor("#B5CEA8"));

    clearRules();
}

void S2Plugin::CPPSyntaxHighlighter::addRule(const QString& pattern, HighlightColor color)
{
    switch (color)
    {
        case HighlightColor::ReservedKeyword:
            mRules.append({QRegularExpression(pattern), mFormatReservedKeywords});
            break;
        case HighlightColor::Variable:
            if (!mSeenVariables.contains(pattern))
            {
                mRules.append({QRegularExpression(pattern), mFormatVariables});
                mSeenVariables.insert(pattern);
            }
            break;
        case HighlightColor::Type:
            if (!mSeenTypes.contains(pattern))
            {
                mRules.append({QRegularExpression(pattern), mFormatTypes});
                mSeenTypes.insert(pattern);
            }
            break;
        case HighlightColor::Comment:
            mRules.append({QRegularExpression(pattern), mFormatComments});
            break;
        case HighlightColor::Text:
            mRules.append({QRegularExpression(pattern), mFormatText});
            break;
        case HighlightColor::Number:
            mRules.append({QRegularExpression(pattern), mFormatNumber});
            break;
    }
}

void S2Plugin::CPPSyntaxHighlighter::highlightBlock(const QString& text)
{
    for (const auto& rule : mRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void S2Plugin::CPPSyntaxHighlighter::clearRules()
{
    mRules.clear();
    mSeenTypes.clear();
    mSeenVariables.clear();
    addRule(R"(\bclass\b)", HighlightColor::ReservedKeyword);
    addRule(R"(\bpublic\b)", HighlightColor::ReservedKeyword);
}

void S2Plugin::CPPSyntaxHighlighter::finalCustomRuleAdded()
{
    addRule(";", HighlightColor::Text);
    addRule("\\[[0-9]+\\]", HighlightColor::Number);
    addRule("\\[", HighlightColor::Text);
    addRule("\\]", HighlightColor::Text);
    addRule(R"(\/\/.*?$)", HighlightColor::Comment);
}
