#include "Views/ViewEntityDB.h"
#include "EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>

ViewEntityDB::ViewEntityDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    initializeSearchLineEdit();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mSearchLineEdit->setFocus();

    mEntityNameCompleter = new QCompleter(mToolbar->entityDB()->entityList()->entityNames(), this);
    mEntityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mSearchLineEdit->setCompleter(mEntityNameCompleter);

    setWindowTitle(QString("Entity DB (%1 entities)").arg(mToolbar->entityDB()->entityList()->highestEntityID()));
    mSearchLineEdit->setVisible(true);
    mMainTreeView->setVisible(true);
    showIndex(index);
}

void ViewEntityDB::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
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
    return QSize(150, 150);
}

void ViewEntityDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    if (isNumeric && enteredID <= mToolbar->entityDB()->entityList()->highestEntityID())
    {
        showIndex(enteredID);
    }
    else
    {
        auto entityID = mToolbar->entityDB()->entityList()->idForName(text.toStdString());
        if (entityID != 0)
        {
            showIndex(entityID);
        }
    }
}

void ViewEntityDB::showIndex(size_t index)
{
    for (const auto& field : gsEntityDBFields)
    {
        mMainTreeView->updateValueForField(field, field.name, mToolbar->entityDB()->offsetsForIndex(index));
    }
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
}
