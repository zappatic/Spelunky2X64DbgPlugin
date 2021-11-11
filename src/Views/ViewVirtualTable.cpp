#include "Views/ViewVirtualTable.h"
#include "QtHelpers/TableWidgetItemNumeric.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QClipBoard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <fstream>

S2Plugin::ViewVirtualTable::ViewVirtualTable(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mHTMLDelegate = std::make_unique<StyledItemDelegateHTML>();
    mModel = std::make_unique<ItemModelVirtualTable>(toolbar->virtualTableLookup(), this);
    mSortFilterProxy = std::make_unique<SortFilterProxyModelVirtualTable>(toolbar->virtualTableLookup(), this);
    mGatherModel = std::make_unique<ItemModelGatherVirtualData>(toolbar, this);
    mGatherSortFilterProxy = std::make_unique<SortFilterProxyModelGatherVirtualData>(toolbar, this);
    mGatherSortFilterProxy->sort(gsColGatherID);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("Virtual Table");
}

void S2Plugin::ViewVirtualTable::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

    mTabData = new QWidget();
    mTabData->setLayout(new QVBoxLayout(mTabData));
    mTabData->layout()->setMargin(10);
    mTabData->setObjectName("datawidget");
    mTabData->setStyleSheet("QWidget#datawidget {border: 1px solid #999;}");

    mTabLookup = new QWidget();
    mTabLookup->setLayout(new QVBoxLayout(mTabLookup));
    mTabLookup->layout()->setMargin(10);
    mTabLookup->setObjectName("lookupwidget");
    mTabLookup->setStyleSheet("QWidget#lookupwidget {border: 1px solid #999;}");

    mTabGather = new QWidget();
    mTabGather->setLayout(new QVBoxLayout(mTabGather));
    mTabGather->layout()->setMargin(10);
    mTabGather->setObjectName("gatherwidget");
    mTabGather->setStyleSheet("QWidget#gatherwidget {border: 1px solid #999;}");

    mMainTabWidget->addTab(mTabData, "Data");
    mMainTabWidget->addTab(mTabLookup, "Lookup");
    mMainTabWidget->addTab(mTabGather, "Gather");

    // TAB DATA
    {
        auto topLayout = new QGridLayout(this);

        auto detectEntitiesBtn = new QPushButton("Detect entities", this);
        QObject::connect(detectEntitiesBtn, &QPushButton::clicked, this, &ViewVirtualTable::detectEntities);
        topLayout->addWidget(detectEntitiesBtn, 0, 0);

        auto showImportedSymbolsCheckBox = new QCheckBox("Show imported symbols", this);
        showImportedSymbolsCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showImportedSymbolsCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showImportedSymbolsCheckBoxStateChanged);
        topLayout->addWidget(showImportedSymbolsCheckBox, 0, 1);

        auto showNonAddressEntriesCheckBox = new QCheckBox("Show non-address entries", this);
        showNonAddressEntriesCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showNonAddressEntriesCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showNonAddressEntriesCheckBoxStateChanged);
        topLayout->addWidget(showNonAddressEntriesCheckBox, 0, 2);

        auto showSymbollessEntriesCheckBox = new QCheckBox("Show symbol-less entries", this);
        showSymbollessEntriesCheckBox->setCheckState(Qt::Checked);
        QObject::connect(showSymbollessEntriesCheckBox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showSymbollessEntriesCheckBoxStateChanged);
        topLayout->addWidget(showSymbollessEntriesCheckBox, 0, 3);

        auto searchFilterLineEdit = new QLineEdit(this);
        searchFilterLineEdit->setPlaceholderText("Search symbol name");
        QObject::connect(searchFilterLineEdit, &QLineEdit::textChanged, this, &ViewVirtualTable::filterTextChanged);
        topLayout->addWidget(searchFilterLineEdit, 1, 1, 1, 3);

        auto tmpLayout = new QHBoxLayout(this);
        tmpLayout->addLayout(topLayout);
        tmpLayout->addStretch();
        dynamic_cast<QVBoxLayout*>(mTabData->layout())->addLayout(tmpLayout);

        mDataTable = new QTableView(this);
        mSortFilterProxy->setSourceModel(mModel.get());
        mDataTable->setModel(mSortFilterProxy.get());
        mDataTable->setAlternatingRowColors(true);
        mDataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        mDataTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mDataTable->verticalHeader()->setDefaultSectionSize(19);
        mDataTable->verticalHeader()->setVisible(false);
        mDataTable->setItemDelegate(mHTMLDelegate.get());
        mDataTable->setColumnWidth(gsColTableOffset, 100);
        mDataTable->setColumnWidth(gsColCodeAddress, 125);
        mDataTable->setColumnWidth(gsColTableAddress, 125);
        mDataTable->horizontalHeader()->setStretchLastSection(true);

        QObject::connect(mDataTable, &QTableView::clicked, this, &ViewVirtualTable::tableEntryClicked);

        dynamic_cast<QVBoxLayout*>(mTabData->layout())->addWidget(mDataTable);
    }

    // TAB LOOKUP
    {
        auto topLayout = new QHBoxLayout(this);
        mLookupAddressLineEdit = new QLineEdit(this);
        mLookupAddressLineEdit->setPlaceholderText("Lookup address");
        topLayout->addWidget(mLookupAddressLineEdit);

        auto lookupBtn = new QPushButton("Lookup", this);
        QObject::connect(lookupBtn, &QPushButton::clicked, this, &ViewVirtualTable::processLookupAddressText);
        topLayout->addWidget(lookupBtn);

        topLayout->addStretch();

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addLayout(topLayout);

        mLookupResultsTable = new QTableWidget(this);
        mLookupResultsTable->setAlternatingRowColors(true);
        mLookupResultsTable->verticalHeader()->setVisible(false);
        mLookupResultsTable->horizontalHeader()->setStretchLastSection(true);
        mLookupResultsTable->setColumnCount(3);
        mLookupResultsTable->setHorizontalHeaderLabels(QStringList() << "Base offset"
                                                                     << "Name"
                                                                     << "Relative offset");
        mLookupResultsTable->setColumnWidth(0, 75);
        mLookupResultsTable->setColumnWidth(1, 325);
        mLookupResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        dynamic_cast<QVBoxLayout*>(mTabLookup->layout())->addWidget(mLookupResultsTable);
    }

    // TAB GATHER
    {
        auto topLayout = new QHBoxLayout(this);

        auto gatherEntitiesBtn = new QPushButton("Gather entities", this);
        QObject::connect(gatherEntitiesBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherEntities);
        topLayout->addWidget(gatherEntitiesBtn);

        auto gatherExtraObjectsBtn = new QPushButton("Gather extra", this);
        QObject::connect(gatherExtraObjectsBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherExtraObjects);
        topLayout->addWidget(gatherExtraObjectsBtn);

        auto gatherVirtsBtn = new QPushButton("Gather virts", this);
        QObject::connect(gatherVirtsBtn, &QPushButton::clicked, this, &ViewVirtualTable::gatherAvailableVirtuals);
        topLayout->addWidget(gatherVirtsBtn);

        mGatherProgressLabel = new QLabel("", this);
        topLayout->addWidget(mGatherProgressLabel);

        mHideCompletedCheckbox = new QCheckBox("Hide completed", this);
        QObject::connect(mHideCompletedCheckbox, &QCheckBox::stateChanged, this, &ViewVirtualTable::showGatherHideCompletedCheckBoxStateChanged);
        topLayout->addWidget(mHideCompletedCheckbox);

        topLayout->addStretch();

        auto exportJSONBtn = new QPushButton("Export JSON", this);
        QObject::connect(exportJSONBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportGatheredData);
        topLayout->addWidget(exportJSONBtn);

        auto exportVirtTableBtn = new QPushButton("Export Virt Table", this);
        QObject::connect(exportVirtTableBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportVirtTable);
        topLayout->addWidget(exportVirtTableBtn);

        auto exportCppEnumBtn = new QPushButton("Export C++ enum", this);
        QObject::connect(exportCppEnumBtn, &QPushButton::clicked, this, &ViewVirtualTable::exportCppEnum);
        topLayout->addWidget(exportCppEnumBtn);

        dynamic_cast<QVBoxLayout*>(mTabGather->layout())->addLayout(topLayout);

        mGatherTable = new QTableView(this);
        mGatherSortFilterProxy->setSourceModel(mGatherModel.get());
        mGatherTable->setModel(mGatherSortFilterProxy.get());
        mGatherTable->setAlternatingRowColors(true);
        mGatherTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGatherTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        mGatherTable->verticalHeader()->setDefaultSectionSize(19);
        mGatherTable->verticalHeader()->setVisible(false);
        mGatherTable->setItemDelegate(mHTMLDelegate.get());
        mGatherTable->setColumnWidth(gsColGatherID, 50);
        mGatherTable->setColumnWidth(gsColGatherName, 200);
        mGatherTable->setColumnWidth(gsColGatherVirtualTableOffset, 125);
        mGatherTable->setColumnWidth(gsColGatherCollision1Present, 75);
        mGatherTable->setColumnWidth(gsColGatherCollision2Present, 75);
        mGatherTable->horizontalHeader()->setStretchLastSection(true);

        dynamic_cast<QVBoxLayout*>(mTabGather->layout())->addWidget(mGatherTable);
        updateGatherProgress();
    }
}

void S2Plugin::ViewVirtualTable::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewVirtualTable::sizeHint() const
{
    return QSize(800, 650);
}

QSize S2Plugin::ViewVirtualTable::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewVirtualTable::tableEntryClicked(const QModelIndex& index)
{
    auto mappedIndex = mSortFilterProxy->mapToSource(index);
    auto offset = mappedIndex.row();
    const auto entry = mToolbar->virtualTableLookup()->entryForOffset(offset);
    auto column = mappedIndex.column();
    switch (column)
    {
        case gsColTableOffset:
        case gsColSymbolName:
            // nop
            break;
        case gsColCodeAddress:
            if (entry.isValidAddress)
            {
                GuiDisasmAt(entry.value, GetContextData(UE_CIP));
                GuiShowCpu();
            }
            break;
        case gsColTableAddress:
            GuiDumpAt(mToolbar->virtualTableLookup()->tableAddressForEntry(entry));
            GuiShowCpu();
            break;
    }
}

void S2Plugin::ViewVirtualTable::detectEntities()
{
    mModel->detectEntities(mToolbar);
}

void S2Plugin::ViewVirtualTable::showImportedSymbolsCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowImportedSymbols(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::showNonAddressEntriesCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowNonAddressEntries(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::showSymbollessEntriesCheckBoxStateChanged(int state)
{
    mSortFilterProxy->setShowSymbollessEntries(state == Qt::Checked);
}

void S2Plugin::ViewVirtualTable::filterTextChanged(const QString& text)
{
    mSortFilterProxy->setFilterString(text);
    if (mSortFilterProxy->symbollessEntriesShown())
    {
        for (auto x = 0; x < mSortFilterProxy->rowCount(); ++x)
        {
            auto indexData = mSortFilterProxy->index(x, gsColSymbolName).data(Qt::DisplayRole);
            if (indexData.toString().contains(text, Qt::CaseInsensitive))
            {
                mDataTable->selectRow(x);
                break;
            }
        }
    }
}

void S2Plugin::ViewVirtualTable::processLookupAddressText()
{
    auto enteredAddress = mLookupAddressLineEdit->text();
    bool conversionOK = false;
    size_t address = enteredAddress.toULongLong(&conversionOK, 16);
    if (conversionOK)
    {
        lookupAddress(address);
    }
}

void S2Plugin::ViewVirtualTable::showLookupAddress(size_t address)
{
    mMainTabWidget->setCurrentWidget(mTabLookup);
    mLookupAddressLineEdit->setText(QString::asprintf("%016llX", address));
    lookupAddress(address);
}

void S2Plugin::ViewVirtualTable::lookupAddress(size_t address)
{
    mLookupResultsTable->setSortingEnabled(false);
    mLookupResultsTable->clearContents();
    auto vtl = mToolbar->virtualTableLookup();
    std::vector<std::vector<QTableWidgetItem*>> items;
    auto tableOffsets = vtl->tableOffsetForFunctionAddress(address);

    if (tableOffsets.size() == 0)
    {
        mLookupResultsTable->setRowCount(1);
        mLookupResultsTable->setItem(0, 1, new QTableWidgetItem("Address not found in virtual table"));
    }
    else
    {
        for (const auto& tableOffset : tableOffsets)
        {
            auto precedingEntry = vtl->findPrecedingEntryWithSymbols(tableOffset);
            if (precedingEntry.symbols.size() > 0)
            {
                for (const auto& symbol : precedingEntry.symbols)
                {
                    std::vector<QTableWidgetItem*> tmp;
                    tmp.emplace_back(new QTableWidgetItem(QString::fromStdString(std::to_string(precedingEntry.offset))));
                    tmp.emplace_back(new QTableWidgetItem(QString::fromStdString(symbol)));
                    auto item = new TableWidgetItemNumeric(QString::fromStdString("+" + std::to_string(tableOffset - precedingEntry.offset)));
                    item->setData(Qt::UserRole, tableOffset - precedingEntry.offset);
                    tmp.emplace_back(item);
                    items.emplace_back(tmp);
                }
            }
        }

        if (items.size() == 0)
        {
            mLookupResultsTable->setRowCount(tableOffsets.size());
            auto counter = 0;
            for (const auto& tableOffset : tableOffsets)
            {
                mLookupResultsTable->setItem(counter, 1, new QTableWidgetItem("No preceding functions found with a name"));
                mLookupResultsTable->setItem(counter++, 2, new TableWidgetItemNumeric(QString::fromStdString("+" + std::to_string(tableOffset))));
            }
        }
        else
        {
            mLookupResultsTable->setRowCount(items.size());
            auto counter = 0;
            for (const auto& item : items)
            {
                mLookupResultsTable->setItem(counter, 0, item.at(0));
                mLookupResultsTable->setItem(counter, 1, item.at(1));
                mLookupResultsTable->setItem(counter++, 2, item.at(2));
            }
        }
    }
    mLookupResultsTable->setSortingEnabled(true);
    mLookupResultsTable->sortItems(2);
}

void S2Plugin::ViewVirtualTable::gatherEntities()
{
    mGatherModel->gatherEntities();
    updateGatherProgress();
}

void S2Plugin::ViewVirtualTable::gatherExtraObjects()
{
    mGatherModel->gatherExtraObjects();
    updateGatherProgress();
}

void S2Plugin::ViewVirtualTable::gatherAvailableVirtuals()
{
    mGatherModel->gatherAvailableVirtuals();
}

void S2Plugin::ViewVirtualTable::updateGatherProgress()
{
    auto progress = mGatherModel->completionPercentage();
    mGatherProgressLabel->setText(QString("%1% complete").arg(progress));
}

void S2Plugin::ViewVirtualTable::exportGatheredData()
{
    auto fileName = QFileDialog::getSaveFileName(this, "Save gathered data", "Spelunky2VirtualTableData.json", "JSON files (*.json)");
    if (!fileName.isEmpty())
    {
        try
        {
            auto fp = std::ofstream(fileName.toStdString());
            fp << mGatherModel->dumpJSON();
        }
        catch (...)
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
            msgBox.setText("The file could not be written");
            msgBox.setWindowTitle("Spelunky2");
            msgBox.exec();
        }
    }
}

void S2Plugin::ViewVirtualTable::exportVirtTable()
{
    auto tbl = mGatherModel->dumpVirtTable();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::fromStdString(tbl));

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
    msgBox.setText("The table was copied to the clipboard");
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
}

void S2Plugin::ViewVirtualTable::exportCppEnum()
{
    auto tbl = mGatherModel->dumpCppEnum();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::fromStdString(tbl));

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
    msgBox.setText("The C++ enum was copied to the clipboard");
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
}

void S2Plugin::ViewVirtualTable::showGatherHideCompletedCheckBoxStateChanged(int state)
{
    mGatherSortFilterProxy->setHideCompleted(state == Qt::Checked);
}
