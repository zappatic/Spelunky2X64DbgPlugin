#pragma once

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QTableView>
#include <memory>

namespace S2Plugin
{
    class Logger;
    class StyledItemDelegateColorPicker;

    static constexpr uint8_t gsLogFieldColColor = 0;
    static constexpr uint8_t gsLogFieldColMemoryOffset = 1;
    static constexpr uint8_t gsLogFieldColFieldType = 2;
    static constexpr uint8_t gsLogFieldColFieldName = 3;

    class TableViewLogger : public QTableView
    {
        Q_OBJECT
      public:
        TableViewLogger(Logger* logger, QWidget* parent = nullptr);

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

      private slots:
        void cellClicked(const QModelIndex& index);

      private:
        std::unique_ptr<StyledItemDelegateColorPicker> mColorPickerDelegate;
        Logger* mLogger;
    };
} // namespace S2Plugin
