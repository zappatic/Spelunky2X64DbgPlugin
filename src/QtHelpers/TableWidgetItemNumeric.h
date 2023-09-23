#pragma once
#include <QTableWidgetItem>

namespace S2Plugin
{
    class TableWidgetItemNumeric : public QTableWidgetItem
    {
      public:
        TableWidgetItemNumeric(const QString& s);
        bool operator<(const QTableWidgetItem& other) const;
    };

} // namespace S2Plugin