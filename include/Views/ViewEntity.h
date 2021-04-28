#pragma once

#include "Data/Entity.h"
#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

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
    TreeViewMemoryFields* mMainTreeView;
    QPushButton* mRefreshButton;
    QCheckBox* mAutoRefreshCheckBox;
    QLineEdit* mAutoRefreshIntervalLineEdit;
    std::unique_ptr<QTimer> mAutoRefreshTimer;
    QComboBox* mInterpretAsComboBox;

    std::unique_ptr<Entity> mEntity;
    ViewToolbar* mToolbar;

    void initializeUI();
};
