#pragma once

#include <QSortFilterProxyModel>
#include <QString>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelStringsTable(QObject* parent = nullptr);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setFilterString(const QString& f);

      private:
        QString mFilterString = "";
    };

} // namespace S2Plugin
