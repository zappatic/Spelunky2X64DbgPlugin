#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    class ItemModelStates : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        ItemModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;

      private:
        std::vector<std::pair<int64_t, std::string>> mStates;
    };

    class SortFilterProxyModelStates : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent = nullptr);

      protected:
        bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const;

      private:
        std::vector<std::pair<int64_t, std::string>> mStates;
    };

} // namespace S2Plugin
