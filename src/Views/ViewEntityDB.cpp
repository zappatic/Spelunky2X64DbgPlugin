#include "Views/ViewEntityDB.h"
#include "Data/EntityList.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidgetItem>

S2Plugin::ViewEntityDB::ViewEntityDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Entity DB (%1 entities)").arg(mToolbar->entityDB()->entityList()->highestID()));
    showIndex(index);
}

void S2Plugin::ViewEntityDB::initializeUI()
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
        mSearchLineEdit->setPlaceholderText("Search");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewEntityDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        mEntityNameCompleter = new QCompleter(mToolbar->entityDB()->entityList()->names(), this);
        mEntityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mEntityNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(mEntityNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewEntityDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(mEntityNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntityDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
        mMainTreeView->setEnableChangeHighlighting(false);
        for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
        {
            mMainTreeView->addMemoryField(field, "EntityDB." + field.name);
        }
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewEntityDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewEntityDB::fieldExpanded);
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
        for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
        {
            switch (field.type)
            {
                case MemoryFieldType::Skip:
                case MemoryFieldType::PointerType:
                case MemoryFieldType::InlineStructType: // todo, maybe
                    continue;
                case MemoryFieldType::Flags32:
                case MemoryFieldType::Flags16:
                case MemoryFieldType::Flags8:
                {
                    mCompareFieldComboBox->addItem(QString::fromStdString(field.name), QVariant::fromValue(field));
                    auto flagCount = (field.type == MemoryFieldType::Flags16 ? 16 : (field.type == MemoryFieldType::Flags8 ? 8 : 32));
                    for (uint8_t x = 1; x <= flagCount; ++x)
                    {
                        MemoryField flagField;
                        flagField.name = field.name; // + ".flag_" + std::to_string(x);
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
        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewEntityDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewEntityDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(mToolbar->entityDB()->entityList()->count(), 3, this);
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
        mHTMLDelegate = std::make_unique<HTMLDelegate>();
        mCompareTableWidget->setItemDelegate(mHTMLDelegate.get());
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewEntityDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(mHTMLDelegate.get());
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
    if (isNumeric && enteredID <= mToolbar->entityDB()->entityList()->highestID())
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

void S2Plugin::ViewEntityDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewEntityDB::showIndex(size_t index)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    mLookupIndex = index;
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
    auto entityName = entityDB->entityList()->nameForID(mLookupIndex);
    for (const auto& [fieldName, offset] : entityDB->offsetsForIndex(mLookupIndex))
    {
        DbgSetAutoLabelAt(offset, (entityName + "." + fieldName).c_str());
    }
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
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
    {
        mMainTreeView->updateValueForField(field, "EntityDB." + field.name, mToolbar->entityDB()->offsetsForIndex(mLookupIndex));
    }
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

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto entityDB = mToolbar->entityDB();
    auto entityList = entityDB->entityList();

    size_t row = 0;
    for (auto x = 1; x <= entityDB->entityList()->highestID(); ++x)
    {
        if (!entityList->isValidID(x))
        {
            continue;
        }

        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(QString::fromStdString(entityList->nameForID(x)))));

        auto [caption, value] = valueForField(field, x);
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

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto entityDB = mToolbar->entityDB();
    auto entityList = entityDB->entityList();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<entity id's>
    for (uint32_t x = 1; x <= entityDB->entityList()->highestID(); ++x)
    {
        if (!entityList->isValidID(x))
        {
            continue;
        }

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

    for (const auto& [groupString, entityIds] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& entityId : entityIds)
        {
            auto entityName = entityList->nameForID(entityId);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(QString::fromStdString(entityName));
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, entityId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

std::pair<QString, QVariant> S2Plugin::ViewEntityDB::valueForField(const MemoryField& field, size_t entityDBIndex)
{
    auto offset = mToolbar->entityDB()->offsetsForIndex(entityDBIndex).at("EntityDB." + field.name);
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
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::StringsTableID:
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

void S2Plugin::ViewEntityDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showIndex(clickedID);
    }
}

void S2Plugin::ViewEntityDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        showIndex(item->data(0, Qt::UserRole).toUInt());
    }
}
