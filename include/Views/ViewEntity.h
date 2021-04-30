#pragma once

#include "Data/Entity.h"
#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetMemoryView.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

namespace S2Plugin
{
    class ViewEntity : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntity(size_t entityOffset, ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntity();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);
        void interpretAsChanged(const QString& text);

      private:
        QVBoxLayout* mMainLayout;
        QHBoxLayout* mTopLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabFields;
        QWidget* mTabMemory;

        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;
        QComboBox* mInterpretAsComboBox;

        TreeViewMemoryFields* mMainTreeView;

        WidgetMemoryView* mMemoryView;
        uint32_t mExtraBytesShown = 100;

        std::unique_ptr<Entity> mEntity;
        ViewToolbar* mToolbar;

        void initializeUI();
        void updateMemoryViewOffsetAndSize();
    };
} // namespace S2Plugin