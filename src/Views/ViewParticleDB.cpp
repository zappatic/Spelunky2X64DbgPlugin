#include "Views/ViewParticleDB.h"
#include "Configuration.h"
#include "Data/ParticleDB.h"
#include "QtHelpers/DatabaseHelper.h"
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

S2Plugin::ViewParticleDB::ViewParticleDB(ViewToolbar* toolbar, size_t index, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mParticleDBPtr = Spelunky2::get()->get_ParticleDB().offsetForIndex(0);
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Particle DB (%1 particles)").arg(Configuration::get()->particleEmittersList().count()));
    showIndex(index);
}

void S2Plugin::ViewParticleDB::initializeUI()
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
    auto config = Configuration::get();
    // LOOKUP
    {
        auto topLayout = new QHBoxLayout();

        mSearchLineEdit = new QLineEdit();
        mSearchLineEdit->setPlaceholderText("Search id");
        topLayout->addWidget(mSearchLineEdit);
        QObject::connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &ViewParticleDB::searchFieldReturnPressed);
        mSearchLineEdit->setVisible(false);
        mParticleNameCompleter = new QCompleter(config->particleEmittersList().names(), this);
        mParticleNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mParticleNameCompleter->setFilterMode(Qt::MatchContains);
        QObject::connect(mParticleNameCompleter, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::activated), this, &ViewParticleDB::searchFieldCompleterActivated);
        mSearchLineEdit->setCompleter(mParticleNameCompleter);

        auto labelButton = new QPushButton("Label", this);
        QObject::connect(labelButton, &QPushButton::clicked, this, &ViewParticleDB::label);
        topLayout->addWidget(labelButton);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
        mMainTreeView->setEnableChangeHighlighting(false);

        mMainTreeView->addMemoryFields(config->typeFields(MemoryFieldType::ParticleDB), "ParticleDB", 0);

        QObject::connect(mMainTreeView, &TreeViewMemoryFields::memoryFieldValueUpdated, this, &ViewParticleDB::fieldUpdated);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::expanded, this, &ViewParticleDB::fieldExpanded);
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
        DB::populateComparisonCombobox(mCompareFieldComboBox, config->typeFields(MemoryFieldType::ParticleDB));

        QObject::connect(mCompareFieldComboBox, &QComboBox::currentTextChanged, this, &ViewParticleDB::comparisonFieldChosen);
        topLayout->addWidget(mCompareFieldComboBox);

        auto groupCheckbox = new QCheckBox("Group by value", this);
        QObject::connect(groupCheckbox, &QCheckBox::stateChanged, this, &ViewParticleDB::compareGroupByCheckBoxClicked);
        topLayout->addWidget(groupCheckbox);

        dynamic_cast<QVBoxLayout*>(mTabCompare->layout())->addLayout(topLayout);

        mCompareTableWidget = new QTableWidget(config->particleEmittersList().count(), 3, this);
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
        QObject::connect(mCompareTableWidget, &QTableWidget::cellClicked, this, &ViewParticleDB::comparisonCellClicked);

        mCompareTreeWidget = new QTreeWidget(this);
        mCompareTreeWidget->setAlternatingRowColors(true);
        mCompareTreeWidget->headerItem()->setHidden(true);
        mCompareTreeWidget->setHidden(true);
        mCompareTreeWidget->setItemDelegate(mHTMLDelegate.get());
        QObject::connect(mCompareTreeWidget, &QTreeWidget::itemClicked, this, &ViewParticleDB::groupedComparisonItemClicked);

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

void S2Plugin::ViewParticleDB::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewParticleDB::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewParticleDB::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewParticleDB::searchFieldReturnPressed()
{
    auto text = mSearchLineEdit->text();
    bool isNumeric = false;
    auto enteredID = text.toUInt(&isNumeric);
    auto& particleEmittersList = Configuration::get()->particleEmittersList();

    if (isNumeric && enteredID <= particleEmittersList.highestID())
    {
        showIndex(enteredID);
    }
    else
    {
        auto entityID = particleEmittersList.idForName(text.toStdString());
        if (entityID != 0)
        {
            showIndex(entityID);
        }
    }
}

void S2Plugin::ViewParticleDB::searchFieldCompleterActivated(const QString& text)
{
    searchFieldReturnPressed();
}

void S2Plugin::ViewParticleDB::showIndex(size_t index)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    mLookupIndex = index;

    mMainTreeView->updateTree(Spelunky2::get()->get_ParticleDB().offsetForIndex(index));
}

void S2Plugin::ViewParticleDB::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewParticleDB::fieldUpdated(const QString& fieldName)
{
    updateFieldValues();
}

void S2Plugin::ViewParticleDB::fieldExpanded(const QModelIndex& index)
{
    updateFieldValues();
}

void S2Plugin::ViewParticleDB::updateFieldValues()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewParticleDB::compareGroupByCheckBoxClicked(int state)
{
    mCompareTableWidget->setHidden(state == Qt::Checked);
    mCompareTreeWidget->setHidden(state == Qt::Unchecked);
}

void S2Plugin::ViewParticleDB::comparisonFieldChosen(const QString& fieldName)
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

void S2Plugin::ViewParticleDB::populateComparisonTableWidget()
{
    mCompareTableWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& particleList = Configuration::get()->particleEmittersList();
    auto& particleDB = Spelunky2::get()->get_ParticleDB();

    size_t row = 0;
    for (auto x = 1; x <= particleList.count(); ++x)
    {
        auto item0 = new QTableWidgetItem(QString::asprintf("%03d", x));
        item0->setTextAlignment(Qt::AlignCenter);
        mCompareTableWidget->setItem(row, 0, item0);
        auto name = QString::fromStdString(particleList.nameForID(x));
        mCompareTableWidget->setItem(row, 1, new QTableWidgetItem(QString("<font color='blue'><u>%1</u></font>").arg(name)));

        auto [caption, value] = DB::valueForField(comboboxData, particleDB.offsetForIndex(x));
        auto item = new TableWidgetItemNumeric(caption);
        item->setData(Qt::UserRole, value);
        mCompareTableWidget->setItem(row, 2, item);

        row++;
    }
    mCompareTableWidget->setSortingEnabled(true);
    mCompareTableWidget->sortItems(0);
}

void S2Plugin::ViewParticleDB::populateComparisonTreeWidget()
{
    mCompareTreeWidget->setSortingEnabled(false);

    auto comboboxData = mCompareFieldComboBox->currentData();
    auto& particleDB = Spelunky2::get()->get_ParticleDB();
    auto& particleEmitters = Configuration::get()->particleEmittersList();

    std::unordered_map<std::string, QVariant> rootValues;
    std::unordered_map<std::string, std::unordered_set<uint32_t>> groupedValues; // valueString -> set<particle id's>
    for (uint32_t x = 1; x <= particleEmitters.count(); ++x)
    {
        auto [caption, value] = DB::valueForField(comboboxData, particleDB.offsetForIndex(x));
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

    for (const auto& [groupString, particleIds] : groupedValues)
    {
        auto rootItem = new TreeWidgetItemNumeric(nullptr, QString::fromStdString(groupString));
        rootItem->setData(0, Qt::UserRole, rootValues.at(groupString));
        mCompareTreeWidget->insertTopLevelItem(0, rootItem);
        for (const auto& particleId : particleIds)
        {
            auto particleName = particleEmitters.nameForID(particleId);
            auto caption = QString("<font color='blue'><u>%1</u></font>").arg(QString::fromStdString(particleName));
            auto childItem = new QTreeWidgetItem(rootItem, QStringList(caption));
            childItem->setData(0, Qt::UserRole, particleId);
            mCompareTreeWidget->insertTopLevelItem(0, childItem);
        }
    }

    mCompareTreeWidget->setSortingEnabled(true);
    mCompareTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void S2Plugin::ViewParticleDB::comparisonCellClicked(int row, int column)
{
    if (column == 1)
    {
        auto clickedID = mCompareTableWidget->item(row, 0)->data(Qt::DisplayRole).toULongLong();
        showIndex(clickedID);
    }
}

void S2Plugin::ViewParticleDB::groupedComparisonItemClicked(QTreeWidgetItem* item, int column)
{
    if (item->childCount() == 0)
    {
        showIndex(item->data(0, Qt::UserRole).toUInt());
    }
}
