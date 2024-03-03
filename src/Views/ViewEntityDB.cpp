#include "Views/ViewEntityDB.h"
#include "Configuration.h"
#include "Data/EntityDB.h"
#include "QtHelpers/DatabaseHelper.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include <QCheckBox>
#include <QCompleter>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewEntityDB::ViewEntityDB(ViewToolbar* toolbar, uint32_t id, QWidget* parent) : QWidget(parent)
{
    mMainTreeView = new TreeViewMemoryFields(toolbar, this);
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Entity DB (%1 entities)").arg(Configuration::get()->entityList().highestID()));
    showID(id);
}

void S2Plugin::ViewEntityDB::initializeUI()
{
    auto mainLayout = new QVBoxLayout();
    mainLayout->setMargin(5);
    setLayout(mainLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    mTabLookup = new QWidget();
    mTabCompare = new QWidget();
    mTabLookup->setLayout(new QVBoxLayout());
    mTabLookup->layout()->setMargin(10);
    mTabLookup->setObjectName("lookupwidget");
    mTabLookup->setStyleSheet("QWidget#lookupwidget {border: 1px solid #999;}");
    mTabCompare->setLayout(new QVBoxLayout());
    mTabCompare->layout()->setMargin(10);
    mTabCompare->setObjectName("comparewidget");
    mTabCompare->setStyleSheet("QWidget#comparewidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(mTabLookup, "Lookup");
    mMainTabWidget->addTab(mTabCompare, "Compare");

    auto config = Configuration::get();

    // LOOKUP
    {
        auto topLayout = new QHBoxLayout();

        mSearchLineEdit = new QLineEdit(this);
        mSearchLineEdit->setPlaceholderText("Search");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewEntityDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        auto entityNameCompleter = new QCompleter(config->entityList().names(), this);
        entityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        entityNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(entityNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewEntityDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(entityNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntityDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView->setEnableChangeHighlighting(false);
        mMainTreeView->addMemoryFields(config->typeFields(MemoryFieldType::EntityDB), "EntityDB", 0);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewEntityDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewEntityDB::fieldExpanded);
        mTabLookup->layout()->addWidget(mMainTreeView);
        mMainTreeView->setColumnWidth(gsColField, 125);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->setColumnWidth(gsColValueHex, 125);
        mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
        mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
        mMainTreeView->setColumnWidth(gsColType, 100);
        mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader();
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox(this);
        mCompareFieldComboBox->addItem(QString::fromStdString(""), QVariant::fromValue(QString::fromStdString("")));
        DB::populateComparisonCombobox(mCompareFieldComboBox, config->typeFields(MemoryFieldType::EntityDB));

        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewEntityDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewEntityDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(config->entityList().count(), 3, this);
        mCompareTableWidget->setAlternatingRowColors(true);
        mCompareTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mCompareTableWidget->setHorizontalHeaderLabels(QStringList() << "ID"
                                                                     << "Name"
                                                                     << "Value");
        mCompareTableWidget->verticalHeader()->setVisible(false);
        mCompareTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mCompareTableWidget->verticalHeader()->setDefaultSectionSize(20);
        mCompareTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mCompareTableWidget->setColumnWidth(0, 40);
        mCompareTableWidget->setColumnWidth(1, 325);
        mCompareTableWidget->setColumnWidth(2, 150);
        mCompareTableWidget->setItemDelegate(&mHTMLDelegate);
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewEntityDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(&mHTMLDelegate);
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &ViewEntityDB::groupedComparisonItemClicked);

        mTabCompare->layout()->addWidget(mCompareTableWidget);
        mTabCompare->layout()->addWidget(mCompareTreeWidget);
    }

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
    auto& entityList = Configuration::get()->entityList();

    if (isNumeric && enteredID <= entityList.highestID())
    {
        showID(enteredID);
    }
    else
    {
        auto entityID = entityList.idForName(text.toStdString());
        if (entityID != 0)
        {
            showID(entityID);
        }
    }
}

void S2Plugin::ViewEntityDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewEntityDB::showID(uint32_t id)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    // id == 0 is valid, but not used
    mMainTreeView->updateTree(Spelunky2::get()->get_EntityDB().offsetForIndex(id), 0);
}

void S2Plugin::ViewEntityDB::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewEntityDB::fieldUpdated(const QString& fieldName)
{
    updateFieldValues();
}

void S2Plugin::ViewEntityDB::fieldExpanded(const QModelIndex& index)
{
    updateFieldValues();
}

void S2Plugin::ViewEntityDB::updateFieldValues()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewEntityDB::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::ViewEntityDB::comparisonFieldChosen(const QString& fieldName)
{
    mCompareTableWidget->clearContents();
    mCompareTreeWidget->clear();

    auto comboIndex = mCompareFieldComboBox->currentIndex();
    if (comboIndex == 0)
    {
        return;
    }

    populateComparisonTableWidget();
    populateComparisonTreeWidget();
}

void S2Plugin::ViewEntityDB::populateComparisonTableWidget()
{
    mCompareTableWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& entityDB = Spelunky2::get()->get_EntityDB();
    auto& entityList = Configuration::get()->entityList();

    size_t row = 0;
    for (auto x = 1; x <= entityList.highestID(); ++x)
    {
        if (!entityList.isValidID(x))
        {
            continue;
        }

        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(QString::fromStdString(entityList.nameForID(x)))));

        auto [caption, value] = DB::valueForField(comboboxData, entityDB.offsetForIndex(x));
        auto item = new TableWidgetItemNumeric(caption);
        item->setData(Qt::UserRole, value);
        mCompareTableWidget->setItem(row, 2, item);

        row++;
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}

void S2Plugin::ViewEntityDB::populateComparisonTreeWidget()
{
    mCompareTreeWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& entityDB = Spelunky2::get()->get_EntityDB();
    auto& entityList = Configuration::get()->entityList();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<entity id's>
    for (uint32_t x = 1; x <= entityList.highestID(); ++x)
    {
        if (!entityList.isValidID(x))
        {
            continue;
        }

        auto [caption, value] = DB::valueForField(comboboxData, entityDB.offsetForIndex(x));
        auto captionStr = caption.toStdString();
        rootValues[captionStr] = value;

        if (groupedValues.count(captionStr) == 0)
        {
            groupedValues[captionStr] = {x};
        }
        else
        {
            groupedValues[captionStr].insert(x);
        }
    }

    for (const auto& [groupString, entityIds] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& entityId : entityIds)
        {
            auto entityName = entityList.nameForID(entityId);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(QString::fromStdString(entityName));
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, entityId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void S2Plugin::ViewEntityDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        mSearchLineEdit->clear();
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showID(clickedID);
    }
}

void S2Plugin::ViewEntityDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        mSearchLineEdit->clear();
        showID(item->data(0, Qt::UserRole).toUInt());
    }
}
