#include "Views/ViewCharacterDB.h"
#include "Configuration.h"
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

S2Plugin::ViewCharacterDB::ViewCharacterDB(ViewToolbar* toolbar, uint8_t index, QWidget* parent) : QWidget(parent)
{
    mMainTreeView = new TreeViewMemoryFields(toolbar, this);
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("Character DB");
    showIndex(index);
}

void S2Plugin::ViewCharacterDB::initializeUI()
{
    auto mainLayout = new QVBoxLayout();
    mainLayout->setMargin(5);
    setLayout(mainLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    mTabLookup = new QWidget();
    mTabCompare = new QWidget();
    mTabLookup->setLayout(new QVBoxLayout(mTabLookup));
    mTabLookup->layout()->setMargin(10);
    mTabLookup->setObjectName("lookupwidget");
    mTabLookup->setStyleSheet("QWidget#lookupwidget {border: 1px solid #999;}");
    mTabCompare->setLayout(new QVBoxLayout(mTabCompare));
    mTabCompare->layout()->setMargin(10);
    mTabCompare->setObjectName("comparewidget");
    mTabCompare->setStyleSheet("QWidget#comparewidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(mTabLookup, "Lookup");
    mMainTabWidget->addTab(mTabCompare, "Compare");
    auto config = Configuration::get();
    // LOOKUP
    {
        auto topLayout = new QHBoxLayout();

        mSearchLineEdit = new QLineEdit(); // TODO: maybe change into ComboBox since there is only 19 of them?
        mSearchLineEdit->setPlaceholderText("Search id");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewCharacterDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        auto characterNameCompleter = new QCompleter(Spelunky2::get()->get_CharacterDB().characterNamesStringList(), this);
        characterNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        characterNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(characterNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewCharacterDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(characterNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewCharacterDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView->setEnableChangeHighlighting(false);
        mMainTreeView->addMemoryFields(config->typeFields(MemoryFieldType::CharacterDB), "CharacterDB", 0);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewCharacterDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewCharacterDB::fieldExpanded);
        mTabLookup->layout()->addWidget(mMainTreeView);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader();
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox(this);
        mCompareFieldComboBox->addItem(QString::fromStdString(""), QVariant::fromValue(QString::fromStdString("")));
        DB::populateComparisonCombobox(mCompareFieldComboBox, config->typeFields(MemoryFieldType::CharacterDB));

        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewCharacterDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewCharacterDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(Spelunky2::get()->get_CharacterDB().charactersCount(), 3, this);
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
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewCharacterDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(&mHTMLDelegate);
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &ViewCharacterDB::groupedComparisonItemClicked);

        mTabCompare->layout()->addWidget(mCompareTableWidget);
        mTabCompare->layout()->addWidget(mCompareTreeWidget);
    }

    mSearchLineEdit->setVisible(true);
    mSearchLineEdit->setFocus();
    mMainTreeView->setVisible(true);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewCharacterDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewCharacterDB::sizeHint() const
{
    return QSize(750, 650);
}

QSize S2Plugin::ViewCharacterDB::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewCharacterDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    auto& characterDB = Spelunky2::get()->get_CharacterDB();
    if (isNumeric && enteredID < characterDB.charactersCount())
    {
        showIndex(enteredID);
    }
    else
    {
        uint8_t cID = 0;
        for (const auto& cName : characterDB.characterNamesStringList())
        {
            if (text == cName)
            {
                showIndex(cID);
            }
            cID++;
        }
    }
}

void S2Plugin::ViewCharacterDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewCharacterDB::showIndex(uint8_t index)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    auto offset = Spelunky2::get()->get_CharacterDB().offsetFromIndex(index);
    mMainTreeView->updateTree(offset);
}

void S2Plugin::ViewCharacterDB::label()
{
    mMainTreeView->labelAll(); // TODO: label all indexes, not only the visible one
}

void S2Plugin::ViewCharacterDB::fieldUpdated(const QString& fieldName)
{
    updateFieldValues();
}

void S2Plugin::ViewCharacterDB::fieldExpanded(const QModelIndex& index)
{
    updateFieldValues();
}

void S2Plugin::ViewCharacterDB::updateFieldValues()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewCharacterDB::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::ViewCharacterDB::comparisonFieldChosen(const QString& fieldName)
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

void S2Plugin::ViewCharacterDB::populateComparisonTableWidget()
{
    mCompareTableWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& characterDB = Spelunky2::get()->get_CharacterDB();

    size_t row = 0;
    for (auto x = 0; x < characterDB.charactersCount(); ++x)
    {
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        const auto& name = characterDB.characterNamesStringList().at(x);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(name)));

        auto [caption, value] = DB::valueForField(comboboxData, characterDB.offsetFromIndex(x));
        auto item = new TableWidgetItemNumeric(caption);
        item->setData(Qt::UserRole, value);
        mCompareTableWidget->setItem(row, 2, item);

        row++;
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}

void S2Plugin::ViewCharacterDB::populateComparisonTreeWidget()
{
    mCompareTreeWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& characterDB = Spelunky2::get()->get_CharacterDB();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<character id's>
    for (uint32_t x = 0; x < characterDB.charactersCount(); ++x)
    {
        auto [caption, value] = DB::valueForField(comboboxData, characterDB.offsetFromIndex(x));
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

    for (const auto& [groupString, characterIDs] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& characterId : characterIDs)
        {
            const auto& characterName = characterDB.characterNamesStringList().at(characterId);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(characterName);
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, characterId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void S2Plugin::ViewCharacterDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        mSearchLineEdit->clear();
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toUInt();
        showIndex(clickedID);
    }
}

void S2Plugin::ViewCharacterDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        mSearchLineEdit->clear();
        showIndex(item->data(0, Qt::UserRole).toUInt());
    }
}
