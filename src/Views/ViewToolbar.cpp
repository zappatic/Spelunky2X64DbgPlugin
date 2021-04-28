#include "Views/ViewToolbar.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewState.h"
#include "pluginmain.h"
#include <QPushButton>

S2Plugin::ViewToolbar::ViewToolbar(EntityDB* entityDB, State* state, Configuration* config, QMdiArea* mdiArea, QWidget* parent)
    : QDockWidget(parent, Qt::WindowFlags()), mEntityDB(entityDB), mState(state), mConfiguration(config), mMDIArea(mdiArea)
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

S2Plugin::ViewEntityDB* S2Plugin::ViewToolbar::showEntityDB()
{
    auto w = new ViewEntityDB(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
    return w;
}

void S2Plugin::ViewToolbar::showState()
{
    auto w = new ViewState(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showEntity(size_t offset)
{
    auto w = new ViewEntity(offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showEntities()
{
    auto w = new ViewEntities(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

S2Plugin::State* S2Plugin::ViewToolbar::state()
{
    mState->loadState();
    return mState;
}

S2Plugin::EntityDB* S2Plugin::ViewToolbar::entityDB()
{
    mEntityDB->loadEntityDB();
    return mEntityDB;
}

S2Plugin::Configuration* S2Plugin::ViewToolbar::configuration() const noexcept
{
    return mConfiguration;
}
