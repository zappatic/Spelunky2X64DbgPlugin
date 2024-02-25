#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QVBoxLayout>
#include <memory>
#include <vector>

namespace S2Plugin
{
    struct MemoryField;
    struct ViewToolbar;
    struct TreeViewMemoryFields;

    class ViewStdMap : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdMap(ViewToolbar* toolbar, const std::string& keytypeName, const std::string& valuetypeName, uintptr_t mapOffset, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshMapContents();
        void refreshData();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        uintptr_t mmapOffset;
        size_t mMapKeyTypeSize;
        size_t mMapValueTypeSize;
        uint8_t mMapKeyAlignment;
        uint8_t mMapValueAlignment;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        ViewToolbar* mToolbar;
        QPushButton* mRefreshDataButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeTreeView();
        void initializeRefreshLayout();
    };
} // namespace S2Plugin
