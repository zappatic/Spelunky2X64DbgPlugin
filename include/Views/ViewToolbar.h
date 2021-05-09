#pragma once

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/ParticleDB.h"
#include "Data/State.h"
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
        ViewToolbar(EntityDB* entityDB, ParticleDB* particleDB, State* state, Configuration* config, QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);

        State* state();
        EntityDB* entityDB();
        ParticleDB* particleDB();
        Configuration* configuration() const noexcept;

        void resetSpelunky2Data();

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        void showState();
        void showEntities();
        void clearLabels();
        void reloadConfig();

      private:
        EntityDB* mEntityDB;
        ParticleDB* mParticleDB;
        State* mState;
        Configuration* mConfiguration;

        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin