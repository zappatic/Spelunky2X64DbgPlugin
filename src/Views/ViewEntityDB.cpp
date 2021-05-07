#include "Views/ViewEntityDB.h"
#include "Data/EntityList.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
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
        mEntityNameCompleter = new QCompleter(mToolbar->entityDB()->entityList()->entityNames(), this);
        mEntityNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
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
        mMainTreeView->setVisible(false);
        mMainTreeView->updateTableHeader();
    }

    // COMPARE
    {
        auto topLayout = new QHBoxLayout();
        mCompareFieldComboBox = new QComboBox(this);
        mCompareFieldComboBox->addItem(QString::fromStdString(""), QVariant::fromValue(QString::fromStdString("")));
        for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::EntityDB))
        {
            if (field.type == MemoryFieldType::Skip || field.type == MemoryFieldType::Rect)
            {
                continue;
            }
            mCompareFieldComboBox->addItem(QString::fromStdString(field.name), QVariant::fromValue(field));
        }
        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewEntityDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);
        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(mToolbar->entityDB()->entityList()->highestEntityID(), 3, this);
        mCompareTableWidget->setAlternatingRowColors(true);
        mCompareTableWidget->setHorizontalHeaderLabels(QStringList() << "ID"
                                                                     << "Name"
                                                                     << "Value");
        mCompareTableWidget->verticalHeader()->setVisible(false);
        mCompareTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mCompareTableWidget->setColumnWidth(0, 40);
        mCompareTableWidget->setColumnWidth(1, 325);
        mCompareTableWidget->setColumnWidth(2, 150);

        mTabCompare->layout()->addWidget(mCompareTableWidget);
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

void S2Plugin::ViewEntityDB::comparisonFieldChosen(const QString& fieldName)
{
    mCompareTableWidget->clearContents();
    mCompareTableWidget->setSortingEnabled(false);
    auto comboIndex = mCompareFieldComboBox->currentIndex();
    if (comboIndex == 0)
    {
        return;
    }

    auto field = mCompareFieldComboBox->currentData().value<MemoryField>();
    auto entityDB = mToolbar->entityDB();
    auto entityList = entityDB->entityList();

    for (auto x = 1; x <= entityDB->entityList()->highestEntityID(); ++x)
    {
        if (!entityList->isValidID(x))
        {
            continue;
        }

        auto row = x - 1;
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entityList->nameForID(x))));

        auto offset = entityDB->offsetsForIndex(x).at("EntityDB." + field.name);
        switch (field.type)
        {
            case MemoryFieldType::CodePointer:
            case MemoryFieldType::DataPointer:
            {
                size_t value = Script::Memory::ReadQword(offset);
                mCompareTableWidget->setItem(row, 2, new QTableWidgetItem(QString::asprintf("0x%016llX", value)));
                break;
            }
            case MemoryFieldType::Byte:
            {
                int8_t value = Script::Memory::ReadByte(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%d", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::UnsignedByte:
            {
                uint8_t value = Script::Memory::ReadByte(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%u", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::Word:
            {
                int16_t value = Script::Memory::ReadWord(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%d", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::UnsignedWord:
            {
                uint16_t value = Script::Memory::ReadWord(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%u", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::Dword:
            {
                int32_t value = Script::Memory::ReadDword(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%ld", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::UnsignedDword:
            {
                uint32_t value = Script::Memory::ReadDword(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%lu", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::Qword:
            {
                int64_t value = Script::Memory::ReadQword(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%lld", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::UnsignedQword:
            {
                uint64_t value = Script::Memory::ReadQword(offset);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%llu", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::Float:
            {
                uint32_t dword = Script::Memory::ReadDword(offset);
                float value = reinterpret_cast<float&>(dword);
                auto item = new TableWidgetItemNumeric(QString::asprintf("%f", value));
                item->setData(Qt::UserRole, value);
                mCompareTableWidget->setItem(row, 2, item);
                break;
            }
            case MemoryFieldType::Bool:
            {
                auto b = Script::Memory::ReadByte(offset);
                bool value = reinterpret_cast<bool&>(b);
                mCompareTableWidget->setItem(row, 2, new QTableWidgetItem(value ? "True" : "False"));
                break;
            }
        }
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}
