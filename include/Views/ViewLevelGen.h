#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>
#include <unordered_map>

namespace S2Plugin
{
    class ViewToolbar;
    class TreeViewMemoryFields;
    class WidgetSpelunkyRooms;

    class ViewLevelGen : public QWidget
    {
        Q_OBJECT
      public:
        ViewLevelGen(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshLevelGen();
        void toggleAutoRefresh(int newLevelGen);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);
        void label();
        void levelGenRoomsPointerClicked();

      private:
        QVBoxLayout* mMainLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabData;
        QWidget* mTabRooms;

        // TOP REFRESH LAYOUT
        QHBoxLayout* mRefreshLayout;
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        // TAB DATA
        TreeViewMemoryFields* mMainTreeView;

        // TAB LEVEL
        std::unordered_map<std::string, WidgetSpelunkyRooms*> mRoomsWidgets; // field_name -> widget*

        ViewToolbar* mToolbar;

        void initializeUI();
    };
} // namespace S2Plugin
