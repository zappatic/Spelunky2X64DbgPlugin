#include "Views/ViewToolbar.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewState.h"
#include "pluginmain.h"
#include <QPushButton>

ViewToolbar::ViewToolbar(EntityDB* entityDB, State* state, QMdiArea* mdiArea, QWidget* parent) : QDockWidget(parent, Qt::WindowFlags()), mEntityDB(entityDB), mState(state), mMDIArea(mdiArea)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);

    mMainLayout = new QVBoxLayout(this);
    auto container = new QWidget(this);
    container->setLayout(mMainLayout);
    setWidget(container);

    setTitleBarWidget(new QWidget(this));

    auto btnEntityDB = new QPushButton(this);
    btnEntityDB->setText("Entity DB");
    mMainLayout->addWidget(btnEntityDB);
    QObject::connect(btnEntityDB, &QPushButton::clicked, this, &ViewToolbar::showEntityDB);

    auto btnState = new QPushButton(this);
    btnState->setText("State");
    mMainLayout->addWidget(btnState);
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showState);

    auto btnEntities = new QPushButton(this);
    btnEntities->setText("Entities");
    mMainLayout->addWidget(btnEntities);
    QObject::connect(btnEntities, &QPushButton::clicked, this, &ViewToolbar::showEntities);

    mMainLayout->addStretch();
}

void ViewToolbar::showEntityDB()
{
    auto w = new ViewEntityDB(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void ViewToolbar::showState()
{
    auto w = new ViewState(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void ViewToolbar::showEntity(size_t offset)
{
    auto w = new ViewEntity(offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void ViewToolbar::showEntities()
{
    auto w = new ViewEntities(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

State* ViewToolbar::state()
{
    mState->loadState();
    return mState;
}

EntityDB* ViewToolbar::entityDB()
{
    mEntityDB->loadEntityDB();
    return mEntityDB;
}
