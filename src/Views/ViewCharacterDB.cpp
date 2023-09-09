#include "Views/ViewCharacterDB.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidgetItem>

S2Plugin::ViewCharacterDB::ViewCharacterDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("Character DB");
    showIndex(index);
}

void S2Plugin::ViewCharacterDB::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

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

    // LOOKUP
    {
        auto topLayout = new QHBoxLayout();

        mSearchLineEdit = new QLineEdit();
        mSearchLineEdit->setPlaceholderText("Search id");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewCharacterDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        mCharacterNameCompleter = new QCompleter(mToolbar->characterDB()->characterNamesStringList(), this);
        mCharacterNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mCharacterNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(mCharacterNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewCharacterDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(mCharacterNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewCharacterDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(mToolbar, mToolbar->characterDB(), this);
        mMainTreeView->setEnableChangeHighlighting(false);
        for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::CharacterDB))
        {
            mMainTreeView->addMemoryField(field, "CharacterDB." + field.name);
        }
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewCharacterDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewCharacterDB::fieldExpanded);
        mTabLookup->layout()->addWidget(mMainTreeView);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        mMainTreeView->setColumnHidden(gsColComparisonValue, true);
        mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox(this);
        mCompareFieldComboBox->addItem(QString::fromStdString(""), QVariant::fromValue(QString::fromStdString("")));
        for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::CharacterDB))
        {
            switch (field.type)
            {
                case MemoryFieldType::Skip:
                    continue;
                case MemoryFieldType::Flags32:
                case MemoryFieldType::Flags16:
                case MemoryFieldType::Flags8:
                {
                    mCompareFieldComboBox->addItem(QString::fromStdString(field.name), QVariant::fromValue(field));
                    uint8_t flagCount = (field.type == MemoryFieldType::Flags16 ? 16 : (field.type == MemoryFieldType::Flags8 ? 8 : 32));
                    for (uint8_t x = 1; x <= flagCount; ++x)
                    {
                        MemoryField flagField;
                        flagField.name = field.name;
                        flagField.type = MemoryFieldType::Flag;
                        flagField.extraInfo = x - 1;
                        flagField.comment = std::to_string(flagCount); // abuse the comment field to transmit the size to fetch
                        mCompareFieldComboBox->addItem(QString::fromStdString(field.name + ".flag_" + std::to_string(x)), QVariant::fromValue(flagField));
                    }
                    break;
                }
                default:
                {
                    mCompareFieldComboBox->addItem(QString::fromStdString(field.name), QVariant::fromValue(field));
                    break;
                }
            }
        }
        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewCharacterDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewCharacterDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(mToolbar->characterDB()->charactersCount(), 3, this);
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
        mHTMLDelegate = std::make_unique<StyledItemDelegateHTML>();
        mCompareTableWidget->setItemDelegate(mHTMLDelegate.get());
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewCharacterDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(mHTMLDelegate.get());
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
    if (isNumeric && enteredID < mToolbar->characterDB()->charactersCount())
    {
        showIndex(enteredID);
    }
    else
    {
        for (const auto& [cID, cName] : mToolbar->characterDB()->characterNames())
        {
            if (text == cName)
            {
                showIndex(cID);
            }
        }
    }
}

void S2Plugin::ViewCharacterDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewCharacterDB::showIndex(size_t index)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    mLookupIndex = index;
    auto& offsets = mToolbar->characterDB()->offsetsForIndex(mLookupIndex);
    auto deltaReference = offsets.at("CharacterDB.is_female");
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::CharacterDB))
    {
        mMainTreeView->updateValueForField(field, "CharacterDB." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewCharacterDB::label()
{
    auto characterDB = mToolbar->characterDB();
    for (const auto& [fieldName, offset] : characterDB->offsetsForIndex(mLookupIndex))
    {
        DbgSetAutoLabelAt(offset, (mToolbar->characterDB()->characterNames().at(mLookupIndex).toStdString() + "." + fieldName).c_str());
    }
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
    auto& offsets = mToolbar->characterDB()->offsetsForIndex(mLookupIndex);
    auto deltaReference = offsets.at("CharacterDB.is_female");
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::CharacterDB))
    {
        mMainTreeView->updateValueForField(field, "CharacterDB." + field.name, offsets, deltaReference);
    }
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

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto characterDB = mToolbar->characterDB();

    size_t row = 0;
    for (auto x = 0; x < characterDB->charactersCount(); ++x)
    {
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        const auto& name = mToolbar->characterDB()->characterNames().at(x);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(name)));

        auto [caption, value] = valueForField(field, x);
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

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto characterDB = mToolbar->characterDB();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<character id's>
    for (uint32_t x = 0; x < mToolbar->characterDB()->charactersCount(); ++x)
    {
        auto [caption, value] = valueForField(field, x);
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
            const auto& characterName = mToolbar->characterDB()->characterNames().at(characterId);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(characterName);
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, characterId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

std::pair<QString, QVariant> S2Plugin::ViewCharacterDB::valueForField(const MemoryField& field, size_t characterDBIndex)
{
    auto offset = mToolbar->characterDB()->offsetsForIndex(characterDBIndex).at("CharacterDB." + field.name);
    switch (field.type)
    {
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        {
            size_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("0x%016llX", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Byte:
        case MemoryFieldType::State8:
        {
            int8_t value = Script::Memory::ReadByte(offset);
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        {
            uint8_t value = Script::Memory::ReadByte(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Word:
        case MemoryFieldType::State16:
        {
            int16_t value = Script::Memory::ReadWord(offset);
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        {
            uint16_t value = Script::Memory::ReadWord(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Dword:
        case MemoryFieldType::State32:
        {
            int32_t value = Script::Memory::ReadDword(offset);
            return std::make_pair(QString::asprintf("%ld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Flags32:
        {
            uint32_t value = Script::Memory::ReadDword(offset);
            return std::make_pair(QString::asprintf("%lu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Qword:
        {
            int64_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("%lld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("%llu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Float:
        {
            uint32_t dword = Script::Memory::ReadDword(offset);
            float value = reinterpret_cast<float&>(dword);
            return std::make_pair(QString::asprintf("%f", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Bool:
        {
            auto b = Script::Memory::ReadByte(offset);
            bool value = reinterpret_cast<bool&>(b);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(b));
        }
        case MemoryFieldType::Flag:
        {
            uint8_t flagToCheck = field.extraInfo;
            bool isFlagSet = false;
            if (field.comment == "32")
            {
                isFlagSet = ((Script::Memory::ReadDword(offset) & (1 << flagToCheck)) > 0);
            }
            else if (field.comment == "16")
            {
                isFlagSet = ((Script::Memory::ReadWord(offset) & (1 << flagToCheck)) > 0);
            }
            else if (field.comment == "8")
            {
                isFlagSet = ((Script::Memory::ReadByte(offset) & (1 << flagToCheck)) > 0);
            }

            bool value = reinterpret_cast<bool&>(isFlagSet);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(isFlagSet));
        }
    }
    return std::make_pair("unknown", 0);
}

void S2Plugin::ViewCharacterDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showIndex(clickedID);
    }
}

void S2Plugin::ViewCharacterDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        showIndex(item->data(0, Qt::UserRole).toUInt());
    }
}
