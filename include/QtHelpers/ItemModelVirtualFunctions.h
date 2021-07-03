#pragma once

#include "Views/ViewToolbar.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    static const uint8_t gsColFunctionIndex = 0;
    static const uint8_t gsColFunctionTableAddress = 1;
    static const uint8_t gsColFunctionFunctionAddress = 2;
    static const uint8_t gsColFunctionSignature = 3;

    static const uint16_t gsRoleFunctionIndex = Qt::UserRole;
    static const uint16_t gsRoleFunctionTableAddress = Qt::UserRole + 1;
    static const uint16_t gsRoleFunctionFunctionAddress = Qt::UserRole + 2;

    class ItemModelVirtualFunctions : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        ItemModelVirtualFunctions(const std::string& typeName, size_t offset, ViewToolbar* toolbar, QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      private:
        std::string mTypeName;
        size_t mMemoryOffset;
        ViewToolbar* mToolbar;
    };

    class SortFilterProxyModelVirtualFunctions : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelVirtualFunctions(const std::string& typeName, ViewToolbar* toolbar, QObject* parent = nullptr);

      protected:
        bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const;

      private:
        std::string mTypeName;
        ViewToolbar* mToolbar;
    };

} // namespace S2Plugin
