#include "QtHelpers/ItemModelStringsTable.h"
#include "Data/StringsTable.h"
#include "pluginmain.h"

// S2Plugin::ItemModelStringsTable::ItemModelStringsTable(StringsTable* stbl, QObject* parent) : QAbstractItemModel(parent), mStringsTable(stbl) {}
//
// Qt::ItemFlags S2Plugin::ItemModelStringsTable::flags(const QModelIndex& index) const
//{
//     return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
// }
//
// QVariant S2Plugin::ItemModelStringsTable::data(const QModelIndex& index, int role) const
//{
//     if (role == Qt::DisplayRole)
//     {
//         switch (index.column())
//         {
//             case gsColStringID:
//                 return index.row();
//             case gsColStringTableOffset:
//                 return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", entry.stringTableOffset);
//             case gsColStringMemoryOffset:
//                 return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", entry.memoryOffset);
//             case gsColStringValue:
//                 return entry.str;
//         }
//     }
//     return QVariant();
// }
//
// int S2Plugin::ItemModelStringsTable::rowCount(const QModelIndex& parent) const
//{
//     return mStringsTable->entries().size();
// }
//
// int S2Plugin::ItemModelStringsTable::columnCount(const QModelIndex& parent) const
//{
//     return 4;
// }
//
// QModelIndex S2Plugin::ItemModelStringsTable::index(int row, int column, const QModelIndex& parent) const
//{
//     return createIndex(row, column);
// }
//
// QModelIndex S2Plugin::ItemModelStringsTable::parent(const QModelIndex& index) const
//{
//     return QModelIndex();
// }
//
// QVariant S2Plugin::ItemModelStringsTable::headerData(int section, Qt::Orientation orientation, int role) const
//{
//     if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
//     {
//         switch (section)
//         {
//             case gsColStringID:
//                 return "ID";
//             case gsColStringTableOffset:
//                 return "Table offset";
//             case gsColStringMemoryOffset:
//                 return "Memory offset";
//             case gsColStringValue:
//                 return "Value";
//         }
//     }
//     return QVariant();
// }

S2Plugin::SortFilterProxyModelStringsTable::SortFilterProxyModelStringsTable(QStringList& stbl, QObject* parent) : QSortFilterProxyModel(parent), mStringsList(stbl) {}

bool S2Plugin::SortFilterProxyModelStringsTable::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (mFilterString.isEmpty())
    {
        return true;
    }

    if (sourceRow < 0 || sourceRow >= mStringsList.count())
    {
        return false;
    }

    bool isNumeric = false;
    auto enteredID = mFilterString.toUInt(&isNumeric);
    if (isNumeric)
    {
        return enteredID == sourceRow;
    }
    else
    {
        auto& str = mStringsList.at(sourceRow);
        return str.contains(mFilterString, Qt::CaseInsensitive);
    }
    return false;
}

void S2Plugin::SortFilterProxyModelStringsTable::setFilterString(const QString& f)
{
    mFilterString = f;
    invalidateFilter();
}
