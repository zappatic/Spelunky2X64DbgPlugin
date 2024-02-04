#pragma once

#include <QStandardItemModel>
#include <QTreeView>
#include <array>
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct ViewToolbar;
    struct MemoryField;
    class StyledItemDelegateHTML;

    struct ColumnFilter
    {
        constexpr ColumnFilter& enable(uint8_t h)
        {
            activeColumns = activeColumns | (1U << h);
            return *this;
        }
        constexpr ColumnFilter& disable(uint8_t h)
        {
            activeColumns = activeColumns & ~(1U << h);
            return *this;
        }
        constexpr bool test(uint8_t h) const
        {
            return (activeColumns & (1U << h)) != 0;
        }

      private:
        uint16_t activeColumns{0xFFFF};
    };

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
        void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, const std::unordered_map<std::string, size_t>& offsets, size_t memoryOffsetDeltaReference = 0,
                                 QStandardItem* parent = nullptr, bool disableChangeHighlightingForField = false);

        ColumnFilter activeColumns;

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void startDrag(Qt::DropActions supportedActions) override;

      signals:
        void memoryFieldValueUpdated(const QString& fieldName);
        void levelGenRoomsPointerClicked();
        void entityOffsetDropped(size_t offset);

      private slots:
        void cellClicked(const QModelIndex& index);

      private:
        int lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);

        ViewToolbar* mToolbar;
        QStandardItemModel* mModel;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;
        std::array<uint32_t, 9> mSavedColumnWidths = {0};
        bool mEnableChangeHighlighting = true;
    };
} // namespace S2Plugin
