#pragma once

#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable;
    struct ViewToolbar;
    struct ItemModelStringsTable;
    struct StyledItemDelegateHTML;

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
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        void initializeUI();
    };

} // namespace S2Plugin
