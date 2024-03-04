#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>

namespace S2Plugin
{
    class ViewToolbar;
    class TreeViewMemoryFields;

    class ViewGameManager : public QWidget
    {
        Q_OBJECT
      public:
        ViewGameManager(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshGameManager();
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
