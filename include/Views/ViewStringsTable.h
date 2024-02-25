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

    static const uint8_t gsColStringID = 0;
    static const uint8_t gsColStringTableOffset = 1;
    static const uint8_t gsColStringMemoryOffset = 2;
    static const uint8_t gsColStringValue = 3;

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
        ViewToolbar* mToolbar;
        QVBoxLayout* mMainLayout;
        QLineEdit* mFilterLineEdit;
        QTableView* mMainTableView;
        QStandardItemModel* mModel;
        SortFilterProxyModelStringsTable* mModelProxy;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        QStringList mStringList;

        void initializeUI();
    };

} // namespace S2Plugin
