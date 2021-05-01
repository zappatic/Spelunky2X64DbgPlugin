#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

namespace S2Plugin
{
    class ViewEntityDB : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntityDB(ViewToolbar* toolbar, size_t index = 1, QWidget* parent = nullptr);
        void showIndex(size_t index);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void searchFieldReturnPressed();
        void label();

      private:
        ViewToolbar* mToolbar;
        size_t mIndex;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        QLineEdit* mSearchLineEdit;
        QCompleter* mEntityNameCompleter;

        void initializeUI();
    };
} // namespace S2Plugin