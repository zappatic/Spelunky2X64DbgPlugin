#pragma once

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/LevelGen.h"
#include "Data/ParticleDB.h"
#include "Data/State.h"
#include "Data/VirtualTableLookup.h"
#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>

namespace S2Plugin
{
    class ViewEntityDB;
    class ViewParticleDB;

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, State* state, LevelGen* levelGen, VirtualTableLookup* vtl, Configuration* config, QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);

        State* state();
        LevelGen* levelGen();
        EntityDB* entityDB();
        ParticleDB* particleDB();
        VirtualTableLookup* virtualTableLookup();
        Configuration* configuration() const noexcept;

        void resetSpelunky2Data();

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        void showState();
        void showLevelGen();
        void showEntities();
        void showVirtualTableLookup();
        void clearLabels();
        void reloadConfig();

      private:
        EntityDB* mEntityDB;
        ParticleDB* mParticleDB;
        State* mState;
        LevelGen* mLevelGen;
        VirtualTableLookup* mVirtualTableLookup;
        Configuration* mConfiguration;

        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin
