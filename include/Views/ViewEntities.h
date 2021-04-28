#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

namespace S2Plugin
{
    class ViewEntities : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntities(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntities();
        void filterCheckboxClicked();

      private:
        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;

        QCheckBox* mCheckboxLayer0;
        QCheckBox* mCheckboxLayer1;
        QCheckBox* mCheckboxFLOOR;
        QCheckBox* mCheckboxFLOORSTYLED;
        QCheckBox* mCheckboxDECORATION;
        QCheckBox* mCheckboxEMBED;
        QCheckBox* mCheckboxCHAR;
        QCheckBox* mCheckboxMONS;
        QCheckBox* mCheckboxITEM;
        QCheckBox* mCheckboxACTIVEFLOOR;
        QCheckBox* mCheckboxFX;
        QCheckBox* mCheckboxBG;
        QCheckBox* mCheckboxMIDBG;
        QCheckBox* mCheckboxLOGICAL;
        QCheckBox* mCheckboxMOUNT;
        QCheckBox* mCheckboxLIQUID;

        ViewToolbar* mToolbar;

        void initializeTreeView();
        void initializeRefreshAndFilter();
    };
} // namespace S2Plugin