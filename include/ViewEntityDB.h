#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>


class ViewEntityDB : public QWidget
{
    Q_OBJECT
  public:
    ViewEntityDB(EntityDB* entityDB, ViewToolbar* toolbar, QWidget* parent = nullptr);
    void showEntityDB(size_t index);

  protected:
    void closeEvent(QCloseEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  private slots:
    void searchFieldReturnPressed();

  private:
    QVBoxLayout* mMainLayout;
    TreeViewMemoryFields* mMainTreeView;
    QLineEdit* mSearchLineEdit;
    QCompleter* mEntityNameCompleter;

    EntityDB* mEntityDB;
    ViewToolbar* mToolbar;

    void initializeTreeView();
    void initializeSearchLineEdit();
};
