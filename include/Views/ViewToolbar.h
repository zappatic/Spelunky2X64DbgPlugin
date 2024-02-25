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

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(uintptr_t offset);
        void showState(uintptr_t statePtr);
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
        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin
