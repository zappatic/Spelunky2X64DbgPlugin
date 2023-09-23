#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace S2Plugin
{
    class ViewEntityDB : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntityDB(ViewToolbar* toolbar, size_t index = 1, QWidget* parent = nullptr);
        void showIndex(size_t index);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void searchFieldReturnPressed();
        void searchFieldCompleterActivated(const QString& text);
        void label();
        void fieldUpdated(const QString& fieldName);
        void fieldExpanded(const QModelIndex& index);
        void comparisonFieldChosen(const QString& fieldName);
        void compareGroupByCheckBoxClicked(int state);
        void comparisonCellClicked(int row, int column);
        void groupedComparisonItemClicked(QTreeWidgetItem* item, int column);

      private:
        ViewToolbar* mToolbar;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        QVBoxLayout* mMainLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabLookup;
        QWidget* mTabCompare;

        // LOOKUP
        size_t mLookupIndex;
        TreeViewMemoryFields* mMainTreeView;
        QLineEdit* mSearchLineEdit;
        QCompleter* mEntityNameCompleter;

        // COMPARE
        QComboBox* mCompareFieldComboBox;
        QTableWidget* mCompareTableWidget;
        QTreeWidget* mCompareTreeWidget;

        void initializeUI();
        void updateFieldValues();
        void populateComparisonCombobox(const std::string& prefix, const std::vector<S2Plugin::MemoryField>& fields);
        void populateComparisonTableWidget();
        void populateComparisonTreeWidget();
        std::pair<QString, QVariant> valueForField(const std::string& prefix, const MemoryField& field, size_t entityDBIndex);
    };
} // namespace S2Plugin
