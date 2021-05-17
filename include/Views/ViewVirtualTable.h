#pragma once

#include "Data/VirtualTableLookup.h"
#include "QtHelpers/HTMLDelegate.h"
#include "QtHelpers/ItemModelVirtualTable.h"
#include "Views/ViewToolbar.h"
#include <QTableView>
#include <memory>

namespace S2Plugin
{
    class ViewVirtualTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualTable(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void tableEntryClicked(const QModelIndex& index);
        void detectEntities();
        void showImportedSymbolsCheckBoxStateChanged(int state);
        void showNonAddressEntriesCheckBoxStateChanged(int state);
        void showSymbollessEntriesCheckBoxStateChanged(int state);
        void filterTextChanged(const QString& text);

      private:
        ViewToolbar* mToolbar;
        QVBoxLayout* mMainLayout;
        QTableView* mMainTable;
        std::unique_ptr<ItemModelVirtualTable> mModel;
        std::unique_ptr<SortFilterProxyModelVirtualTable> mSortFilterProxy;
        std::unique_ptr<HTMLDelegate> mHTMLDelegate;

        void initializeUI();
    };
} // namespace S2Plugin
