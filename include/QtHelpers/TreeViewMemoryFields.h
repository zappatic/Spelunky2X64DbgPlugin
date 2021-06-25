#pragma once

#include "Data/EntityDB.h"
#include "Data/MemoryMappedData.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include <QDragEnterEvent>
#include <QDropEvent>
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
        TreeViewMemoryFields(ViewToolbar* toolbar, MemoryMappedData* mmd, QWidget* parent = nullptr);
        QStandardItem* addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent = nullptr);
        void clear();
        void updateTableHeader(bool restoreColumnWidths = true);
        void setEnableChangeHighlighting(bool b) noexcept;

        void expandItem(QStandardItem* item);
        QStandardItem* lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);
        void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent = nullptr,
                                 bool disableChangeHighlightingForField = false);

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void startDrag(Qt::DropActions supportedActions) override;

      signals:
        void memoryFieldValueUpdated(const QString& fieldName);
        void levelGenRoomsPointerClicked(const QString& fieldName);
        void entityOffsetDropped(size_t offset);

      private slots:
        void cellClicked(const QModelIndex& index);
        void cellCollapsed(const QModelIndex& index);

      private:
        ViewToolbar* mToolbar;
        MemoryMappedData* mMemoryMappedData;
        QStandardItemModel* mModel;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;
        std::array<uint32_t, 6> mSavedColumnWidths = {0};
        bool mEnableChangeHighlighting = true;
    };
} // namespace S2Plugin
