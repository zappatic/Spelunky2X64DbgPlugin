#pragma once

#include "QtHelpers/StyledItemDelegateHTML.h"
#include <QLineEdit>
#include <QTableView>
#include <QWidget>
#include <cstdint>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable;

    constexpr uint8_t gsColStringID = 0;
    constexpr uint8_t gsColStringTableOffset = 1;
    constexpr uint8_t gsColStringMemoryOffset = 2;
    constexpr uint8_t gsColStringValue = 3;

    class ViewStringsTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewStringsTable(QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        void cellClicked(const QModelIndex& index);
        void filterTextChanged(const QString& text);
        void reload();

      private:
        QLineEdit* mFilterLineEdit;
        QTableView* mMainTableView;
        SortFilterProxyModelStringsTable* mModelProxy;
        StyledItemDelegateHTML mHTMLDelegate;

        void initializeUI();
    };

} // namespace S2Plugin
