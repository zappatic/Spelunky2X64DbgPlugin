#pragma once

#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewEntityDB;
    class ViewParticleDB;
    class ViewVirtualTable;
    class ViewTextureDB;
    class ViewCharacterDB;
    struct EntityDB;
    struct ParticleDB;
    struct TextureDB;
    struct CharacterDB;
    struct GameManager;
    struct SaveGame;
    struct State;
    struct LevelGen;
    struct VirtualTableLookup;
    struct StringsTable;
    struct Online;
    struct Configuration;

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, TextureDB* textureDB, CharacterDB* cdb, GameManager* gm, SaveGame* sg, State* state, LevelGen* levelGen, VirtualTableLookup* vtl,
                    StringsTable* stbl, Online* online, Configuration* config, QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);
        void showState(State* state);

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
        void showMainThreadState();
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
        void showStdVector(size_t offset, const std::string& typeName);
        void showStdMap(size_t offset, const std::string& keytypeName, const std::string& valuetypeName);
        void showJournalPage(size_t offset, const std::string& pageType);
        void showThreads();
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
