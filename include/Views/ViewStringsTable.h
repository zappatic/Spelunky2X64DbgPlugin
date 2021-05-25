#pragma once

#include "Data/StringsTable.h"
#include "QtHelpers/HTMLDelegate.h"
#include "QtHelpers/ItemModelStringsTable.h"
#include "Views/ViewToolbar.h"
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable;

    class ViewStringsTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewStringsTable(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        void cellClicked(const QModelIndex& index);
        void filterTextChanged(const QString& text);

      private:
        ViewToolbar* mToolbar;
        QVBoxLayout* mMainLayout;
        QLineEdit* mFilterLineEdit;
        QTableView* mMainTableView;
        ItemModelStringsTable* mModel;
        SortFilterProxyModelStringsTable* mModelProxy;
        std::unique_ptr<HTMLDelegate> mHTMLDelegate;

        void initializeUI();
    };

} // namespace S2Plugin
