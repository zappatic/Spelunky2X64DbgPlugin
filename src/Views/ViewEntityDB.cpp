#include "Views/ViewEntityDB.h"
#include "Data/EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>

S2Plugin::ViewEntityDB::ViewEntityDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Entity DB (%1 entities)").arg(mToolbar->entityDB()->entityList()->highestEntityID()));
    showIndex(index);
}

void S2Plugin::ViewEntityDB::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);

    auto topLayout = new QHBoxLayout();

    mSearchLineEdit = new QLineEdit();
    mSearchLineEdit->setPlaceholderText("Search");
    topLayout->addWidget(mSearchLineEdit);
    QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewEntityDB::searchFieldReturnPressed);
    mSearchLineEdit->setVisible(false);
    mEntityNameCompleter = new QCompleter(mToolbar->entityDB()->entityList()->entityNames(), this);
    mEntityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mSearchLineEdit->setCompleter(mEntityNameCompleter);

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntityDB::label);
    topLayout->addWidget(labelButton);

    mMainLayout->addLayout(topLayout);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setEnableChangeHighlighting(false);
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
    {
        mMainTreeView->addMemoryField(field, "EntityDB." + field.name);
    }
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->updateTableHeader();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mSearchLineEdit->setVisible(true);
    mSearchLineEdit->setFocus();
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewEntityDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewEntityDB::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewEntityDB::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewEntityDB::searchFieldReturnPressed()
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

void S2Plugin::ViewEntityDB::showIndex(size_t index)
{
    mIndex = index;
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
    {
        mMainTreeView->updateValueForField(field, "EntityDB." + field.name, mToolbar->entityDB()->offsetsForIndex(index));
    }
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewEntityDB::label()
{
    auto entityDB = mToolbar->entityDB();
    auto entityName = entityDB->entityList()->nameForID(mIndex);
    for (const auto& [fieldName, offset] : entityDB->offsetsForIndex(mIndex))
    {
        DbgSetAutoLabelAt(offset, (entityName + "." + fieldName).c_str());
    }
}