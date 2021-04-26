#include "ViewToolbar.h"
#include "ViewEntityDB.h"
#include "ViewState.h"
#include "pluginmain.h"
#include <QPushButton>

ViewToolbar::ViewToolbar(EntityDB* entityDB, State* state, QMdiArea* mdiArea, QWidget* parent) : QDockWidget(parent, Qt::WindowFlags()), mEntityDB(entityDB), mState(state), mMDIArea(mdiArea)
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

    auto btnState = new QPushButton(this);
    btnState->setText("State");
    mMainLayout->addWidget(btnState);
    mMainLayout->addStretch();

    QObject::connect(btnEntityDB, &QPushButton::clicked, this, &ViewToolbar::showEntityDB);
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showState);
}

void ViewToolbar::showEntityDB()
{
    mEntityDB->loadEntityDB();
    auto w = new ViewEntityDB(mEntityDB, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void ViewToolbar::showState()
{
    mEntityDB->loadEntityDB();
    mState->loadState();
    auto w = new ViewState(mState, mEntityDB, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}