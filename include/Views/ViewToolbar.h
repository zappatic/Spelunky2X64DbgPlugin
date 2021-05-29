#pragma once

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/LevelGen.h"
#include "Data/ParticleDB.h"
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

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, TextureDB* textureDB, State* state, LevelGen* levelGen, VirtualTableLookup* vtl, StringsTable* stbl, Configuration* config,
                    QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);

        State* state();
        LevelGen* levelGen();
        EntityDB* entityDB();
        ParticleDB* particleDB();
        TextureDB* textureDB();
        VirtualTableLookup* virtualTableLookup();
        StringsTable* stringsTable();
        Configuration* configuration() const noexcept;

        void resetSpelunky2Data();

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        ViewTextureDB* showTextureDB();
        void showState();
        void showLevelGen();
        void showEntities();
        ViewVirtualTable* showVirtualTableLookup();
        void showStringsTable();
        void clearLabels();
        void reloadConfig();

      private:
        EntityDB* mEntityDB;
        ParticleDB* mParticleDB;
        TextureDB* mTextureDB;
        State* mState;
        LevelGen* mLevelGen;
        VirtualTableLookup* mVirtualTableLookup;
        StringsTable* mStringsTable;
        Configuration* mConfiguration;

        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin
