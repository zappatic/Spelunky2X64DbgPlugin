#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

namespace S2Plugin
{
    struct ViewToolbar;
    struct StyledItemDelegateHTML;
    struct State;

    class ViewThreads : public QWidget
    {
        Q_OBJECT
      public:
        ViewThreads(ViewToolbar* toolbar);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void cellClicked(int row, int column);
        void refreshThreads();

      private:
        ViewToolbar* mToolbar;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        QVBoxLayout* mMainLayout;
        QTableWidget* mMainTable;
        std::vector<std::unique_ptr<State>> mThreadStates;

        void initializeUI();
    };
} // namespace S2Plugin
