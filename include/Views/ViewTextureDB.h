#pragma once

#include "Data/TextureDB.h"
#include "QtHelpers/HTMLDelegate.h"
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
    class ViewTextureDB : public QWidget
    {
        Q_OBJECT
      public:
        ViewTextureDB(ViewToolbar* toolbar, size_t index = 1, QWidget* parent = nullptr);
        void showID(size_t id);

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
        std::unique_ptr<HTMLDelegate> mHTMLDelegate;

        QVBoxLayout* mMainLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabLookup;
        QWidget* mTabCompare;

        // LOOKUP
        size_t mLookupID;
        TreeViewMemoryFields* mMainTreeView;
        QLineEdit* mSearchLineEdit;
        QCompleter* mParticleNameCompleter;

        // COMPARE
        QComboBox* mCompareFieldComboBox;
        QTableWidget* mCompareTableWidget;
        QTreeWidget* mCompareTreeWidget;

        void initializeUI();
        void updateFieldValues();
        void populateComparisonTableWidget();
        void populateComparisonTreeWidget();
        std::pair<QString, QVariant> valueForField(const MemoryField& field, size_t textureDBIndex);
    };
} // namespace S2Plugin
