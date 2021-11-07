#pragma once

#include "Data/EntityDB.h"
#include "Data/State.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

namespace S2Plugin
{
    class ViewState : public QWidget
    {
        Q_OBJECT
      public:
        ViewState(ViewToolbar* toolbar, State* state, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshState();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);
        void label();

      private:
        ViewToolbar* mToolbar;
        State* mState;

        QVBoxLayout* mMainLayout;
        QHBoxLayout* mRefreshLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeUI();
    };
} // namespace S2Plugin
