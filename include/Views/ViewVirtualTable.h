#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QTableView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <memory>

namespace S2Plugin
{
    class ItemModelVirtualTable;
    class SortFilterProxyModelVirtualTable;
    class StyledItemDelegateHTML;
    class ItemModelGatherVirtualData;
    class SortFilterProxyModelGatherVirtualData;

    class ViewVirtualTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualTable(QWidget* parent = nullptr);
        void showLookupAddress(size_t address);
        void updateGatherProgress();

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
        void gatherEntities();
        void gatherExtraObjects();
        void gatherAvailableVirtuals();
        void exportGatheredData();
        void exportVirtTable();
        void exportCppEnum();
        void showGatherHideCompletedCheckBoxStateChanged(int state);

      private:
        QVBoxLayout* mMainLayout;

        QTabWidget* mMainTabWidget;
        QWidget* mTabData;
        QWidget* mTabLookup;
        QWidget* mTabGather;

        // DATA
        QTableView* mDataTable;
        std::unique_ptr<ItemModelVirtualTable> mModel;
        std::unique_ptr<SortFilterProxyModelVirtualTable> mSortFilterProxy;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        // LOOKUP
        QLineEdit* mLookupAddressLineEdit;
        QTableWidget* mLookupResultsTable;

        // GATHER
        QTableView* mGatherTable;
        std::unique_ptr<ItemModelGatherVirtualData> mGatherModel;
        std::unique_ptr<SortFilterProxyModelGatherVirtualData> mGatherSortFilterProxy;
        QLabel* mGatherProgressLabel;
        QCheckBox* mHideCompletedCheckbox;

        void initializeUI();
        void lookupAddress(size_t address);
    };
} // namespace S2Plugin
