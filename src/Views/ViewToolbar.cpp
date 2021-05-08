#include "Views/ViewToolbar.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewState.h"
#include "pluginmain.h"
#include <QMdiSubWindow>
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

    auto btnClearLabels = new QPushButton(this);
    btnClearLabels->setText("Clear labels");
    mMainLayout->addWidget(btnClearLabels);
    QObject::connect(btnClearLabels, &QPushButton::clicked, this, &ViewToolbar::clearLabels);

    auto btnReloadConfig = new QPushButton(this);
    btnReloadConfig->setText("Reload JSON");
    mMainLayout->addWidget(btnReloadConfig);
    QObject::connect(btnReloadConfig, &QPushButton::clicked, this, &ViewToolbar::reloadConfig);
}

S2Plugin::ViewEntityDB* S2Plugin::ViewToolbar::showEntityDB()
{
    if (mEntityDB->loadEntityDB())
    {
        auto w = new ViewEntityDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showState()
{
    if (mState->loadState())
    {
        auto w = new ViewState(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showEntity(size_t offset)
{
    auto w = new ViewEntity(offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showEntities()
{
    if (mEntityDB->loadEntityDB())
    {
        auto w = new ViewEntities(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::clearLabels()
{
    auto list = BridgeList<Script::Label::LabelInfo>();
    Script::Label::GetList(&list);
    for (auto x = 0; x < list.Count(); ++x)
    {
        auto labelInfo = list[x];
        if (!labelInfo.manual)
        {
            if (!Script::Label::Delete(labelInfo.rva))
            {
                dprintf("Can't delete label %s\n", labelInfo.text);
            }
        }
    }
    list.Cleanup();
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

void S2Plugin::ViewToolbar::reloadConfig()
{
    auto windows = mMDIArea->subWindowList();
    for (const auto& window : windows)
    {
        if (qobject_cast<ViewEntities*>(window->widget()) == nullptr)
        {
            window->close();
        }
    }
    mConfiguration->load();
    if (!mConfiguration->isValid())
    {
        mConfiguration->spelunky2()->displayError(mConfiguration->lastError().c_str());
    }
}

void S2Plugin::ViewToolbar::resetSpelunky2Data()
{
    mMDIArea->closeAllSubWindows();
    mState->reset();
    mEntityDB->reset();
    mConfiguration->spelunky2()->reset();
}