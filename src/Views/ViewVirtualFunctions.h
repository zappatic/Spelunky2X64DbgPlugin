#pragma once

#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Views/ViewToolbar.h"
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>
#include <memory>


namespace S2Plugin
{
    class ViewVirtualFunctions : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualFunctions(const std::string& typeName, size_t offset, ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void tableEntryClicked(const QModelIndex& index);
        void jumpToFunction(bool b);

      private:
        std::string mTypeName;
        size_t mMemoryOffset;
        ViewToolbar* mToolbar;
        QVBoxLayout* mMainLayout;

        QHBoxLayout* mTopLayout;
        QLineEdit* mJumpToLineEdit;

        QTableView* mFunctionsTable;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;
        std::unique_ptr<ItemModelVirtualFunctions> mModel;
        std::unique_ptr<SortFilterProxyModelVirtualFunctions> mSortFilterProxy;

        void initializeUI();
    };
} // namespace S2Plugin
