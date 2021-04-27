#pragma once

#include "Data/EntityDB.h"
#include "Data/State.h"
#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>

class ViewToolbar : public QDockWidget
{
    Q_OBJECT
  public:
    ViewToolbar(EntityDB* entityDB, State* state, QMdiArea* mdiArea, QWidget* parent = nullptr);
    void showEntity(size_t offset);

    State* state();
    EntityDB* entityDB();

  private slots:
    void showEntityDB();
    void showState();
    void showEntities();

  private:
    EntityDB* mEntityDB;
    State* mState;

    QMdiArea* mMDIArea;
    QVBoxLayout* mMainLayout;
};