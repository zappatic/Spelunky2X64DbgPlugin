#pragma once

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/State.h"
#include <QDockWidget>
#include <QMdiArea>
#include <QVBoxLayout>

namespace S2Plugin
{
    class ViewEntityDB;

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(EntityDB* entityDB, State* state, Configuration* config, QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(size_t offset);

        State* state();
        EntityDB* entityDB();
        Configuration* configuration() const noexcept;

      public slots:
        ViewEntityDB* showEntityDB();
        void showState();
        void showEntities();
        void clearLabels();

      private:
        EntityDB* mEntityDB;
        State* mState;
        Configuration* mConfiguration;

        QMdiArea* mMDIArea;
        QVBoxLayout* mMainLayout;
    };
} // namespace S2Plugin