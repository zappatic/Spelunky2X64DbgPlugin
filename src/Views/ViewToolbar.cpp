#include "Views/ViewToolbar.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewGameManager.h"
#include "Views/ViewJournalPage.h"
#include "Views/ViewLevelGen.h"
#include "Views/ViewLogger.h"
#include "Views/ViewOnline.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewSaveGame.h"
#include "Views/ViewState.h"
#include "Views/ViewStdMap.h"
#include "Views/ViewStdVector.h"
#include "Views/ViewStringsTable.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewThreads.h"
#include "Views/ViewVirtualFunctions.h"
#include "Views/ViewVirtualTable.h"
#include "pluginmain.h"
#include <QMdiSubWindow>
#include <QPushButton>
#include <QString>

S2Plugin::ViewToolbar::ViewToolbar(QMdiArea* mdiArea, QWidget* parent) : QDockWidget(parent, Qt::WindowFlags()), mMDIArea(mdiArea)
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

    auto btnTextureDB = new QPushButton(this);
    btnTextureDB->setText("Texture DB");
    mMainLayout->addWidget(btnTextureDB);
    QObject::connect(btnTextureDB, &QPushButton::clicked, this, &ViewToolbar::showTextureDB);

    auto btnParticleDB = new QPushButton(this);
    btnParticleDB->setText("Particle DB");
    mMainLayout->addWidget(btnParticleDB);
    QObject::connect(btnParticleDB, &QPushButton::clicked, this, &ViewToolbar::showParticleDB);

    auto btnStringsTable = new QPushButton(this);
    btnStringsTable->setText("Strings DB");
    mMainLayout->addWidget(btnStringsTable);
    QObject::connect(btnStringsTable, &QPushButton::clicked, this, &ViewToolbar::showStringsTable);

    auto btnCharacterDB = new QPushButton(this);
    btnCharacterDB->setText("Character DB");
    mMainLayout->addWidget(btnCharacterDB);
    QObject::connect(btnCharacterDB, &QPushButton::clicked, this, &ViewToolbar::showCharacterDB);

    auto divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    mMainLayout->addWidget(divider);

    auto btnState = new QPushButton(this);
    btnState->setText("State");
    mMainLayout->addWidget(btnState);
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showMainThreadState);

    auto btnEntities = new QPushButton(this);
    btnEntities->setText("Entities");
    mMainLayout->addWidget(btnEntities);
    QObject::connect(btnEntities, &QPushButton::clicked, this, &ViewToolbar::showEntities);

    auto btnLevelGen = new QPushButton(this);
    btnLevelGen->setText("LevelGen");
    mMainLayout->addWidget(btnLevelGen);
    QObject::connect(btnLevelGen, &QPushButton::clicked, this, &ViewToolbar::showLevelGen);

    auto btnGameManager = new QPushButton(this);
    btnGameManager->setText("GameManager");
    mMainLayout->addWidget(btnGameManager);
    QObject::connect(btnGameManager, &QPushButton::clicked, this, &ViewToolbar::showGameManager);

    auto btnSaveGame = new QPushButton(this);
    btnSaveGame->setText("SaveGame");
    mMainLayout->addWidget(btnSaveGame);
    QObject::connect(btnSaveGame, &QPushButton::clicked, this, &ViewToolbar::showSaveGame);

    auto btnOnline = new QPushButton(this);
    btnOnline->setText("Online");
    mMainLayout->addWidget(btnOnline);
    QObject::connect(btnOnline, &QPushButton::clicked, this, &ViewToolbar::showOnline);

    auto btnVirtualTable = new QPushButton(this);
    btnVirtualTable->setText("Virtual Table");
    mMainLayout->addWidget(btnVirtualTable);
    QObject::connect(btnVirtualTable, &QPushButton::clicked, this, &ViewToolbar::showVirtualTableLookup);

    auto divider2 = new QFrame(this);
    divider2->setFrameShape(QFrame::HLine);
    divider2->setFrameShadow(QFrame::Sunken);
    mMainLayout->addWidget(divider2);

    auto btnLogger = new QPushButton(this);
    btnLogger->setText("Logger");
    mMainLayout->addWidget(btnLogger);
    QObject::connect(btnLogger, &QPushButton::clicked, this, &ViewToolbar::showLogger);

    auto btnThreads = new QPushButton(this);
    btnThreads->setText("Threads");
    mMainLayout->addWidget(btnThreads);
    QObject::connect(btnThreads, &QPushButton::clicked, this, &ViewToolbar::showThreads);

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
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_EntityDB().isValid())
    {
        auto w = new ViewEntityDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewParticleDB* S2Plugin::ViewToolbar::showParticleDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_ParticleDB().isValid())
    {
        auto w = new ViewParticleDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewTextureDB* S2Plugin::ViewToolbar::showTextureDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_TextureDB().isValid())
    {
        auto w = new ViewTextureDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewCharacterDB* S2Plugin::ViewToolbar::showCharacterDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_StringsTable().isValid() && Spelunky2::get()->get_CharacterDB().isValid())
    {
        auto w = new ViewCharacterDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showMainThreadState()
{
    if (Spelunky2::is_loaded())
    {
        auto statePtr = Spelunky2::get()->get_StatePtr();
        if (statePtr != 0)
            showState(statePtr);
    }
}

void S2Plugin::ViewToolbar::showState(uintptr_t state)
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto w = new ViewState(this, state);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showGameManager()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_GameManagerPtr() != 0)
    {
        auto w = new ViewGameManager(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showLevelGen()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto w = new ViewLevelGen(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

S2Plugin::ViewVirtualTable* S2Plugin::ViewToolbar::showVirtualTableLookup()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_VirtualTableLookup().isValid())
    {
        auto w = new ViewVirtualTable(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showStringsTable()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_StringsTable().isValid())
    {
        auto w = new ViewStringsTable(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showOnline()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_OnlinePtr() != 0)
    {
        auto w = new ViewOnline(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showEntity(uintptr_t offset)
{
    auto w = new ViewEntity(offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showEntities()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_EntityDB().isValid())
    {
        auto w = new ViewEntities(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showSaveGame()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_SaveDataPtr() != 0)
    {
        auto w = new ViewSaveGame(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

void S2Plugin::ViewToolbar::showLogger()
{
    auto w = new ViewLogger(this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showVirtualFunctions(size_t offset, const std::string& typeName)
{
    auto w = new ViewVirtualFunctions(typeName, offset, this, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showStdVector(size_t offset, const std::string& typeName)
{
    auto w = new ViewStdVector(this, typeName, offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showStdMap(size_t offset, const std::string& keytypeName, const std::string& valuetypeName)
{
    auto w = new ViewStdMap(this, keytypeName, valuetypeName, offset, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showJournalPage(size_t offset, const std::string& pageType)
{
    auto w = new ViewJournalPage(this, offset, pageType, this);
    mMDIArea->addSubWindow(w);
    w->setVisible(true);
}

void S2Plugin::ViewToolbar::showThreads()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto w = new ViewThreads(this);
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
        const auto& labelInfo = list[x];
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
    Configuration::reload();
}

void S2Plugin::ViewToolbar::resetSpelunky2Data()
{
    mMDIArea->closeAllSubWindows();
    Spelunky2::reset();
}
