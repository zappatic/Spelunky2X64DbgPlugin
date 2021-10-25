#pragma once

#include "Data/StringsTable.h"
#include "Views/ViewToolbar.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace S2Plugin
{
    static const uint8_t gsColStringID = 0;
    static const uint8_t gsColStringTableOffset = 1;
    static const uint8_t gsColStringMemoryOffset = 2;
    static const uint8_t gsColStringValue = 3;

    class ItemModelStringsTable : public QAbstractItemModel
    {
        Q_OBJECT

      public:
        ItemModelStringsTable(StringsTable* stbl, QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      private:
        StringsTable* mStringsTable;
    };

    class SortFilterProxyModelStringsTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelStringsTable(StringsTable* stbl, QObject* parent = nullptr);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setFilterString(const QString& f);

      private:
        StringsTable* mStringsTable;
        QString mFilterString = "";
    };

} // namespace S2Plugin
