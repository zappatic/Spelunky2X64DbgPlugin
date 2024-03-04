#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>

namespace S2Plugin
{
    class ViewToolbar;
    class TreeViewMemoryFields;
    class WidgetMemoryView;
    class CPPSyntaxHighlighter;
    class WidgetSpelunkyLevel;

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
        void label();
        void entityOffsetDropped(size_t entityOffset);
        void tabChanged(int index);

      private:
        QVBoxLayout* mMainLayout;
        QHBoxLayout* mTopLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabFields;
        QWidget* mTabMemory;
        QWidget* mTabLevel;
        QWidget* mTabCPP;

        // TOP LAYOUT
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;
        QComboBox* mInterpretAsComboBox;

        // TAB FIELDS
        TreeViewMemoryFields* mMainTreeView;

        // TAB MEMORY
        WidgetMemoryView* mMemoryView;
        WidgetMemoryView* mMemoryComparisonView;
        QScrollArea* mMemoryComparisonScrollArea;

        // TAB LEVEL
        WidgetSpelunkyLevel* mSpelunkyLevel;

        // TAB CPP
        QTextEdit* mCPPTextEdit;
        CPPSyntaxHighlighter* mCPPSyntaxHighlighter;
        uintptr_t mEntityPtr;
        uintptr_t mComparisonEntityPtr{0};
        ViewToolbar* mToolbar;

        void initializeUI();
        void updateMemoryViewOffsetAndSize();
        void updateComparedMemoryViewHighlights();
        void updateLevel();
    };
} // namespace S2Plugin
