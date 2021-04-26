#include "ViewEntityDB.h"
#include "EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>

ViewEntityDB::ViewEntityDB(EntityDB* entityDB, QWidget* parent) : QWidget(parent), mEntityDB(entityDB)
{
    mMainLayout = new QVBoxLayout();

    initializeSearchLineEdit();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mSearchLineEdit->setFocus();

    mEntityNameCompleter = new QCompleter(mEntityDB->entityList()->entityNames(), this);
    mEntityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mSearchLineEdit->setCompleter(mEntityNameCompleter);

    setWindowTitle(QString("Entity DB (%1 entities)").arg(mEntityDB->entityList()->highestEntityID()));
    mSearchLineEdit->setVisible(true);
    mMainTreeView->setVisible(true);
    showEntityDB(1);
}

void ViewEntityDB::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mEntityDB, this);
    mMainTreeView->addEntityDBMemoryFields();

    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
}

void ViewEntityDB::initializeSearchLineEdit()
{
    mSearchLineEdit = new QLineEdit();
    mSearchLineEdit->setPlaceholderText("Search");
    mMainLayout->addWidget(mSearchLineEdit);
    QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewEntityDB::searchFieldReturnPressed);
    mSearchLineEdit->setVisible(false);
}

void ViewEntityDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize ViewEntityDB::sizeHint() const
{
    return QSize(750, 1050);
}

QSize ViewEntityDB::minimumSizeHint() const
{
    return QSize(750, 1050);
}

void ViewEntityDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    if (isNumeric && enteredID <= mEntityDB->entityList()->highestEntityID())
    {
        showEntityDB(enteredID);
    }
    else
    {
        auto entityID = mEntityDB->entityList()->idForName(text.toStdString());
        if (entityID != 0)
        {
            showEntityDB(entityID);
        }
    }
}

void ViewEntityDB::showEntityDB(size_t index)
{
    for (const auto& field : gsEntityDBFields)
    {
        mMainTreeView->updateValueForField(field, field.name, mEntityDB->offsetsForIndex(index));
    }
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
}
