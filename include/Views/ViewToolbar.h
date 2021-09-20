#pragma once

#include "Configuration.h"
#include "Data/CharacterDB.h"
#include "Data/EntityDB.h"
#include "Data/GameManager.h"
#include "Data/LevelGen.h"
#include "Data/Online.h"
#include "Data/ParticleDB.h"
#include "Data/SaveGame.h"
#include "Data/State.h"
#include "Data/StringsTable.h"
#include "Data/TextureDB.h"
#include "Data/VirtualTableLookup.h"
#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>

namespace S2Plugin
{
    class ViewEntityDB;
    class ViewParticleDB;
    class ViewVirtualTable;
    class ViewTextureDB;
    class ViewCharacterDB;

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, TextureDB* textureDB, CharacterDB* cdb, GameManager* gm, SaveGame* sg, State* state, LevelGen* levelGen, VirtualTableLookup* vtl,
                    StringsTable* stbl, Online* online, Configuration* config, QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);

        State* state();
        SaveGame* savegame();
        GameManager* gameManager();
        LevelGen* levelGen();
        EntityDB* entityDB();
        ParticleDB* particleDB();
        CharacterDB* characterDB();
        TextureDB* textureDB();
        VirtualTableLookup* virtualTableLookup();
        StringsTable* stringsTable();
        Online* online();
        Configuration* configuration() const noexcept;

        void resetSpelunky2Data();

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        ViewTextureDB* showTextureDB();
        void showState();
        void showGameManager();
        void showLevelGen();
        void showEntities();
        ViewVirtualTable* showVirtualTableLookup();
        void showStringsTable();
        ViewCharacterDB* showCharacterDB();
        void showSaveGame();
        void showLogger();
        void showVirtualFunctions(size_t offset, const std::string& typeName);
        void showOnline();
        void clearLabels();
        void reloadConfig();

      private:
        EntityDB* mEntityDB;
        ParticleDB* mParticleDB;
        TextureDB* mTextureDB;
        CharacterDB* mCharacterDB;
        GameManager* mGameManager;
        SaveGame* mSaveGame;
        State* mState;
        LevelGen* mLevelGen;
        VirtualTableLookup* mVirtualTableLookup;
        StringsTable* mStringsTable;
        Configuration* mConfiguration;
        Online* mOnline;

        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin
