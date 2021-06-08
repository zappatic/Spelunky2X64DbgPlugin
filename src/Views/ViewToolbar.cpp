#include "Views/ViewToolbar.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewLevelGen.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewSaveGame.h"
#include "Views/ViewState.h"
#include "Views/ViewStringsTable.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewVirtualTable.h"
#include "pluginmain.h"
#include <QMdiSubWindow>
#include <QPushButton>

S2Plugin::ViewToolbar::ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, TextureDB* textureDB, CharacterDB* cdb, GameManager* gm, SaveGame* sg, State* state, LevelGen* levelGen,
                                   VirtualTableLookup* vtl, StringsTable* stbl, Configuration* config, QMdiArea* mdiArea, QWidget* parent)
    : QDockWidget(parent, Qt::WindowFlags()), mEntityDB(entityDB), mParticleDB(particleDB), mTextureDB(textureDB), mCharacterDB(cdb), mGameManager(gm), mSaveGame(sg), mState(state),
      mLevelGen(levelGen), mVirtualTableLookup(vtl), mStringsTable(stbl), mConfiguration(config), mMDIArea(mdiArea)
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
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showState);

    auto btnEntities = new QPushButton(this);
    btnEntities->setText("Entities");
    mMainLayout->addWidget(btnEntities);
    QObject::connect(btnEntities, &QPushButton::clicked, this, &ViewToolbar::showEntities);

    auto btnLevelGen = new QPushButton(this);
    btnLevelGen->setText("LevelGen");
    mMainLayout->addWidget(btnLevelGen);
    QObject::connect(btnLevelGen, &QPushButton::clicked, this, &ViewToolbar::showLevelGen);

    auto btnSaveGame = new QPushButton(this);
    btnSaveGame->setText("SaveGame");
    mMainLayout->addWidget(btnSaveGame);
    QObject::connect(btnSaveGame, &QPushButton::clicked, this, &ViewToolbar::showSaveGame);

    auto btnVirtualTable = new QPushButton(this);
    btnVirtualTable->setText("Virtual Table");
    mMainLayout->addWidget(btnVirtualTable);
    QObject::connect(btnVirtualTable, &QPushButton::clicked, this, &ViewToolbar::showVirtualTableLookup);

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

S2Plugin::ViewParticleDB* S2Plugin::ViewToolbar::showParticleDB()
{
    if (mParticleDB->loadParticleDB())
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
    if (mTextureDB->loadTextureDB())
    {
        auto w = new ViewTextureDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showCharacterDB()
{
    if (mCharacterDB->loadCharacters())
    {
        auto w = new ViewCharacterDB(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
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

void S2Plugin::ViewToolbar::showLevelGen()
{
    if (mState->loadState() && mLevelGen->loadLevelGen())
    {
        auto w = new ViewLevelGen(this);
        mMDIArea->addSubWindow(w);
        w->setVisible(true);
    }
}

S2Plugin::ViewVirtualTable* S2Plugin::ViewToolbar::showVirtualTableLookup()
{
    if (mVirtualTableLookup->loadTable())
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
    if (mStringsTable->loadStringsTable())
    {
        auto w = new ViewStringsTable(this);
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

void S2Plugin::ViewToolbar::showSaveGame()
{
    if (mSaveGame->loadSaveGame())
    {
        auto w = new ViewSaveGame(this);
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

S2Plugin::SaveGame* S2Plugin::ViewToolbar::savegame()
{
    mSaveGame->loadSaveGame();
    return mSaveGame;
}

S2Plugin::LevelGen* S2Plugin::ViewToolbar::levelGen()
{
    mState->loadState();
    mLevelGen->loadLevelGen();
    return mLevelGen;
}

S2Plugin::EntityDB* S2Plugin::ViewToolbar::entityDB()
{
    mEntityDB->loadEntityDB();
    return mEntityDB;
}

S2Plugin::ParticleDB* S2Plugin::ViewToolbar::particleDB()
{
    mParticleDB->loadParticleDB();
    return mParticleDB;
}

S2Plugin::CharacterDB* S2Plugin::ViewToolbar::characterDB()
{
    mCharacterDB->loadCharacters();
    return mCharacterDB;
}

S2Plugin::TextureDB* S2Plugin::ViewToolbar::textureDB()
{
    mTextureDB->loadTextureDB();
    return mTextureDB;
}

S2Plugin::VirtualTableLookup* S2Plugin::ViewToolbar::virtualTableLookup()
{
    mVirtualTableLookup->loadTable();
    return mVirtualTableLookup;
}

S2Plugin::StringsTable* S2Plugin::ViewToolbar::stringsTable()
{
    mStringsTable->loadStringsTable();
    return mStringsTable;
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
    mState->reset();
    mGameManager->reset();
    mSaveGame->reset();
    mLevelGen->reset();
    mEntityDB->reset();
    mParticleDB->reset();
    mTextureDB->reset();
    mCharacterDB->reset();
}

void S2Plugin::ViewToolbar::resetSpelunky2Data()
{
    mMDIArea->closeAllSubWindows();
    mState->reset();
    mGameManager->reset();
    mSaveGame->reset();
    mLevelGen->reset();
    mEntityDB->reset();
    mParticleDB->reset();
    mTextureDB->reset();
    mVirtualTableLookup->reset();
    mStringsTable->reset();
    mCharacterDB->reset();
    mConfiguration->spelunky2()->reset();
}
