#pragma once

#include "Data/EntityDB.h"
#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>

class ViewToolbar : public QDockWidget
{
    Q_OBJECT
  public:
    ViewToolbar(EntityDB* entityDB, QMdiArea* mdiArea, QWidget* parent = nullptr);

  private slots:
    void showEntityDB();

  private:
    EntityDB* mEntityDB;
    QMdiArea* mMDIArea;
    QVBoxLayout* mMainLayout;
};