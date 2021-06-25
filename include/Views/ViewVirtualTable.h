#pragma once

#include "Data/VirtualTableLookup.h"
#include "QtHelpers/ItemModelVirtualTable.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Views/ViewToolbar.h"
#include <QTableView>
#include <QTableWidget>
#include <memory>


namespace S2Plugin
{
    class ViewVirtualTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualTable(ViewToolbar* toolbar, QWidget* parent = nullptr);
        void showLookupAddress(size_t address);

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
        void processLookupAddressText();

      private:
        ViewToolbar* mToolbar;
        QVBoxLayout* mMainLayout;

        QTabWidget* mMainTabWidget;
        QWidget* mTabData;
        QWidget* mTabLookup;

        // DATA
        QTableView* mDataTable;
        std::unique_ptr<ItemModelVirtualTable> mModel;
        std::unique_ptr<SortFilterProxyModelVirtualTable> mSortFilterProxy;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        // LOOKUP
        QLineEdit* mLookupAddressLineEdit;
        QTableWidget* mLookupResultsTable;

        void initializeUI();
        void lookupAddress(size_t address);
    };
} // namespace S2Plugin
