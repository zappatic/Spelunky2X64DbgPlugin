#pragma once
#include <QTreeWidgetItem>

namespace S2Plugin
{
    class TreeWidgetItemNumeric : public QTreeWidgetItem
    {
      public:
        TreeWidgetItemNumeric(QTreeWidgetItem* parent, const QString& caption);
        bool operator<(const QTreeWidgetItem& other) const;
    };

} // namespace S2Plugin