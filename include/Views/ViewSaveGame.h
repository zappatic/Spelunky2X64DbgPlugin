#pragma once

#include "Data/SaveGame.h"
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
    class ViewSaveGame : public QWidget
    {
        Q_OBJECT
      public:
        ViewSaveGame(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshSaveGame();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);
        void label();

      private:
        QVBoxLayout* mMainLayout;
        QHBoxLayout* mRefreshLayout;
        TreeViewMemoryFields* mMainTreeView;
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        ViewToolbar* mToolbar;

        void initializeUI();
    };
} // namespace S2Plugin
