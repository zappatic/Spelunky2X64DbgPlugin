#pragma once

#include "Data/EntityDB.h"
#include "Data/State.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

class ViewState : public QWidget
{
    Q_OBJECT
  public:
    ViewState(State* state, EntityDB* entityDB, QWidget* parent = nullptr);

  protected:
    void closeEvent(QCloseEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  private slots:
    void refreshState();
    void toggleAutoRefresh(int newState);
    void autoRefreshTimerTrigger();
    void autoRefreshIntervalChanged(const QString& text);

  private:
    QVBoxLayout* mMainLayout;
    QHBoxLayout* mRefreshLayout;
    TreeViewMemoryFields* mMainTreeView;
    QPushButton* mRefreshButton;
    QCheckBox* mAutoRefreshCheckBox;
    QLineEdit* mAutoRefreshIntervalLineEdit;
    std::unique_ptr<QTimer> mAutoRefreshTimer;

    State* mState;
    EntityDB* mEntityDB;

    void initializeTreeView();
    void initializeRefreshStuff();
};
