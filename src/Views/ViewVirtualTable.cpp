#include "Views/ViewVirtualTable.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>

S2Plugin::ViewVirtualTable::ViewVirtualTable(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mHTMLDelegate = std::make_unique<HTMLDelegate>();
    mModel = std::make_unique<ItemModelVirtualTable>(toolbar->virtualTableLookup(), this);
    mSortFilterProxy = std::make_unique<SortFilterProxyModelVirtualTable>(toolbar->virtualTableLookup(), this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("Virtual Table");
}

void S2Plugin::ViewVirtualTable::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

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
    mMainLayout->addLayout(tmpLayout);

    mMainTable = new QTableView(this);
    mSortFilterProxy->setSourceModel(mModel.get());
    mMainTable->setModel(mSortFilterProxy.get());
    mMainTable->setAlternatingRowColors(true);
    mMainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mMainTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mMainTable->verticalHeader()->setDefaultSectionSize(19);
    mMainTable->verticalHeader()->setVisible(false);
    mMainTable->setItemDelegate(mHTMLDelegate.get());
    mMainTable->setColumnWidth(gsColTableOffset, 100);
    mMainTable->setColumnWidth(gsColCodeAddress, 125);
    mMainTable->setColumnWidth(gsColTableAddress, 125);
    mMainTable->setColumnWidth(gsColSymbolName, 250);

    QObject::connect(mMainTable, &QTableView::clicked, this, &ViewVirtualTable::tableEntryClicked);

    mMainLayout->addWidget(mMainTable);
}

void S2Plugin::ViewVirtualTable::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewVirtualTable::sizeHint() const
{
    return QSize(550, 650);
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
                mMainTable->selectRow(x);
                dprintf("-> row %d  (%s)\n", x, indexData.toString().toStdString().c_str());
                break;
            }
        }
    }
}
