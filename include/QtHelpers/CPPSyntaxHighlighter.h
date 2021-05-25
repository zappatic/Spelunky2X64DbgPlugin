#pragma once

#include "Configuration.h"
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

namespace S2Plugin
{
    enum class HighlightColor
    {
        ReservedKeyword,
        Variable,
        Type,
        Comment,
        Text,
        Number
    };

    struct CPPHighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    class CPPSyntaxHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

      public:
        explicit CPPSyntaxHighlighter(QTextDocument* parent = nullptr);
        void addRule(const QString& pattern, HighlightColor color);
        void finalCustomRuleAdded();
        void clearRules();

      protected:
        void highlightBlock(const QString& text) override;

      private:
        QVector<CPPHighlightingRule> mRules;
        QSet<QString> mSeenTypes; // keep track of added types and variables, so we don't balloon the rules
        QSet<QString> mSeenVariables;

        QTextCharFormat mFormatReservedKeywords;
        QTextCharFormat mFormatVariables;
        QTextCharFormat mFormatTypes;
        QTextCharFormat mFormatComments;
        QTextCharFormat mFormatText;
        QTextCharFormat mFormatNumber;
    };
} // namespace S2Plugin
