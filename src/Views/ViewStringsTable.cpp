#include "Views/ViewStringsTable.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QModelIndex>
#include <QTableWidgetItem>

S2Plugin::ViewStringsTable::ViewStringsTable(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Strings table (%1 strings)").arg(mToolbar->stringsTable()->entries().size()));
    initializeUI();
}

void S2Plugin::ViewStringsTable::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    auto stringsTable = mToolbar->stringsTable();
    const auto& entries = stringsTable->entries();

    auto topLayout = new QHBoxLayout();
    mFilterLineEdit = new QLineEdit(this);
    mFilterLineEdit->setPlaceholderText("Search id or text");
    QObject::connect(mFilterLineEdit, &QLineEdit::textChanged, this, &ViewStringsTable::filterTextChanged);
    topLayout->addWidget(mFilterLineEdit);
    mMainLayout->addLayout(topLayout);

    mMainTableView = new QTableView(this);
    mMainTableView->setAlternatingRowColors(true);
    mMainTableView->verticalHeader()->setVisible(false);
    mMainTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mMainTableView->verticalHeader()->setDefaultSectionSize(30);
    mMainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mMainTableView->horizontalHeader()->setStretchLastSection(true);
    mMainTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mHTMLDelegate = std::make_unique<HTMLDelegate>();
    mHTMLDelegate->setCenterVertically(true);
    mMainTableView->setItemDelegateForColumn(gsColStringMemoryOffset, mHTMLDelegate.get());
    QObject::connect(mMainTableView, &QTableView::clicked, this, &ViewStringsTable::cellClicked);
    mMainLayout->addWidget(mMainTableView);

    mModel = new ItemModelStringsTable(mToolbar->stringsTable());
    mModelProxy = new SortFilterProxyModelStringsTable(mToolbar->stringsTable(), this);
    mModelProxy->setSourceModel(mModel);
    mMainTableView->setModel(mModelProxy);
    mMainTableView->setColumnWidth(gsColStringID, 40);
    mMainTableView->setColumnWidth(gsColStringMemoryOffset, 125);
    mMainTableView->setWordWrap(true);
}

QSize S2Plugin::ViewStringsTable::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewStringsTable::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewStringsTable::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewStringsTable::cellClicked(const QModelIndex& index)
{
    auto mappedIndex = mModelProxy->mapToSource(index);
    auto id = mappedIndex.row();
    const auto entry = mToolbar->stringsTable()->entryForID(id);
    auto column = mappedIndex.column();

    if (column == gsColStringMemoryOffset)
    {
        GuiDumpAt(entry.memoryOffset);
        GuiShowCpu();
    }
}

void S2Plugin::ViewStringsTable::filterTextChanged(const QString& text)
{
    mModelProxy->setFilterString(text);
}
