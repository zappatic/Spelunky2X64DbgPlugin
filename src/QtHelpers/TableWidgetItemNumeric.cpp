#include "QtHelpers/TableWidgetItemNumeric.h"
#include "pluginmain.h"

S2Plugin::TableWidgetItemNumeric::TableWidgetItemNumeric(const QString& s) : QTableWidgetItem(s, 0) {}

bool S2Plugin::TableWidgetItemNumeric::operator<(const QTableWidgetItem& other) const
{
    return data(Qt::UserRole) < other.data(Qt::UserRole);
}
