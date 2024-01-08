#include "Views/ViewTextureDB.h"
#include "Configuration.h"
#include "Data/TextureDB.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/TreeWidgetItemNumeric.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidgetItem>

S2Plugin::ViewTextureDB::ViewTextureDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Texture DB (%1 textures)").arg(mToolbar->textureDB()->count()));
    showID(index);
}

void S2Plugin::ViewTextureDB::initializeUI()
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
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewTextureDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        mTextureNameCompleter = new QCompleter(mToolbar->textureDB()->namesStringList(), this);
        mTextureNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mTextureNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(mTextureNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewTextureDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(mTextureNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewTextureDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
        mMainTreeView->setEnableChangeHighlighting(false);
        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::TextureDB))
        {
            mMainTreeView->addMemoryField(field, "TextureDB." + field.name);
        }
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewTextureDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewTextureDB::fieldExpanded);
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
        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::TextureDB))
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
                        flagField.flag_index = x - 1;
                        flagField.flag_parrent_size = flagCount;
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
        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewTextureDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewTextureDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(mToolbar->textureDB()->count(), 3, this);
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
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewTextureDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(mHTMLDelegate.get());
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &ViewTextureDB::groupedComparisonItemClicked);

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

void S2Plugin::ViewTextureDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewTextureDB::sizeHint() const
{
    return QSize(750, 375);
}

QSize S2Plugin::ViewTextureDB::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewTextureDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    if (isNumeric && enteredID < mToolbar->textureDB()->count())
    {
        showID(enteredID);
    }
    else
    {
        static const QRegularExpression r("^Texture ([0-9]+)");
        auto m = r.match(text);
        if (m.isValid())
        {
            auto textureID = m.captured(1).toUInt();
            showID(textureID);
        }
    }
}

void S2Plugin::ViewTextureDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewTextureDB::showID(size_t id)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    mLookupID = id;
    auto& offsets = mToolbar->textureDB()->offsetsForTextureID(mLookupID);
    auto deltaReference = offsets.at("TextureDB.id");
    for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::TextureDB))
    {
        mMainTreeView->updateValueForField(field, "TextureDB." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewTextureDB::label()
{
    auto textureDB = mToolbar->textureDB();
    for (const auto& [fieldName, offset] : textureDB->offsetsForTextureID(mLookupID))
    {
        DbgSetAutoLabelAt(offset, ("Texture" + std::to_string(mLookupID) + "." + fieldName).c_str());
    }
}

void S2Plugin::ViewTextureDB::fieldUpdated(const QString& fieldName)
{
    updateFieldValues();
}

void S2Plugin::ViewTextureDB::fieldExpanded(const QModelIndex& index)
{
    updateFieldValues();
}

void S2Plugin::ViewTextureDB::updateFieldValues()
{
    auto& offsets = mToolbar->textureDB()->offsetsForTextureID(mLookupID);
    auto deltaReference = offsets.at("TextureDB.id");
    for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::TextureDB))
    {
        mMainTreeView->updateValueForField(field, "TextureDB." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewTextureDB::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::ViewTextureDB::comparisonFieldChosen(const QString& fieldName)
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

void S2Plugin::ViewTextureDB::populateComparisonTableWidget()
{
    mCompareTableWidget->setSortingEnabled(false);

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto textureDB = mToolbar->textureDB();

    size_t row = 0;
    for (auto x = 0; x < textureDB->count(); ++x)
    {
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        auto name = QString("Texture %1 (%2)").arg(x).arg(QString::fromStdString(mToolbar->textureDB()->nameForID(x)));
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

void S2Plugin::ViewTextureDB::populateComparisonTreeWidget()
{
    mCompareTreeWidget->setSortingEnabled(false);

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto textureDB = mToolbar->textureDB();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<texture id's>
    for (uint32_t x = 0; x < textureDB->count(); ++x)
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

    for (const auto& [groupString, textureIds] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& textureId : textureIds)
        {
            auto textureName = QString("Texture %1 (%2)").arg(textureId).arg(QString::fromStdString(mToolbar->textureDB()->nameForID(textureId)));
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(textureName);
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, textureId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

std::pair<QString, QVariant> S2Plugin::ViewTextureDB::valueForField(const MemoryField& field, size_t textureDBIndex)
{
    auto offset = mToolbar->textureDB()->offsetsForTextureID(textureDBIndex).at("TextureDB." + field.name);
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
        case MemoryFieldType::Double:
        {
            size_t qword = Script::Memory::ReadQword(offset);
            double value = reinterpret_cast<double&>(qword);
            return std::make_pair(QString::asprintf("%lf", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Bool:
        {
            auto b = Script::Memory::ReadByte(offset);
            bool value = reinterpret_cast<bool&>(b);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(b));
        }
        case MemoryFieldType::Flag:
        {
            uint8_t flagToCheck = field.flag_index;
            bool isFlagSet = false;
            switch (field.flag_parrent_size)
            {
                case 32:
                    isFlagSet = ((Script::Memory::ReadDword(offset) & (1 << flagToCheck)) > 0);
                    break;
                case 16:
                    isFlagSet = ((Script::Memory::ReadWord(offset) & (1 << flagToCheck)) > 0);
                    break;
                case 8:
                    isFlagSet = ((Script::Memory::ReadByte(offset) & (1 << flagToCheck)) > 0);
                    break;
            }

            bool value = reinterpret_cast<bool&>(isFlagSet);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(isFlagSet));
        }
    }
    return std::make_pair("unknown", 0);
}

void S2Plugin::ViewTextureDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showID(clickedID);
    }
}

void S2Plugin::ViewTextureDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        showID(item->data(0, Qt::UserRole).toUInt());
    }
}
