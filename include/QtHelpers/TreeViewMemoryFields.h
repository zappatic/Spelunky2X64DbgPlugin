#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/HTMLDelegate.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include <QStandardItem>
#include <QTreeView>
#include <array>
#include <memory>
#include <unordered_map>

namespace S2Plugin
{
    class TreeViewMemoryFields : public QTreeView
    {
        Q_OBJECT
      public:
        TreeViewMemoryFields(ViewToolbar* toolbar, QWidget* parent = nullptr);
        QStandardItem* addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent = nullptr);
        void clear();
        void updateTableHeader(bool restoreColumnWidths = true);
        void setEnableChangeHighlighting(bool b) noexcept;

        void expandItem(QStandardItem* item);
        QStandardItem* lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);
        void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent = nullptr,
                                 bool disableChangeHighlightingForField = false);

      private slots:
        void cellClicked(const QModelIndex& index);

      private:
        QStandardItemModel* mModel;
        ViewToolbar* mToolbar;
        std::unique_ptr<HTMLDelegate> mHTMLDelegate;
        std::array<uint32_t, 5> mSavedColumnWidths = {0};
        bool mEnableChangeHighlighting = true;
    };
} // namespace S2Plugin