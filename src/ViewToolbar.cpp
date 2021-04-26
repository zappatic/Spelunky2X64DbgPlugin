#include "ViewToolbar.h"
#include "ViewEntityDB.h"
#include "pluginmain.h"
#include <QPushButton>

ViewToolbar::ViewToolbar(EntityDB* entityDB, QMdiArea* mdiArea, QWidget* parent) : QDockWidget(parent, Qt::WindowFlags()), mEntityDB(entityDB), mMDIArea(mdiArea)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);

    mMainLayout = new QVBoxLayout();
    auto container = new QWidget(this);
    container->setLayout(mMainLayout);
    setWidget(container);

    setTitleBarWidget(new QWidget(this));

    auto btnEntityDB = new QPushButton(this);
    btnEntityDB->setText("Entity DB");
    mMainLayout->addWidget(btnEntityDB);

    QObject::connect(btnEntityDB, &QPushButton::clicked, this, &ViewToolbar::showEntityDB);
}

void ViewToolbar::showEntityDB()
{
    mEntityDB->loadEntityDB();
    auto w = new ViewEntityDB(mEntityDB, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}