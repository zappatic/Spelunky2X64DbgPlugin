#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "pluginmain.h"

S2Plugin::TreeWidgetItemNumeric::TreeWidgetItemNumeric(QTreeWidgetItem* parent, const QString& caption) : QTreeWidgetItem(parent, QStringList(caption)) {}

bool S2Plugin::TreeWidgetItemNumeric::operator<(const QTreeWidgetItem& other) const
{
    return data(0, Qt::UserRole) < other.data(0, Qt::UserRole);
}
