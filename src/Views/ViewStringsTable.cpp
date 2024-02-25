#include "Views/ViewStringsTable.h"
#include "Configuration.h"
#include "Data/StringsTable.h"
#include "QtHelpers/ItemModelStringsTable.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QTableWidgetItem>
#include <string>

S2Plugin::ViewStringsTable::ViewStringsTable(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Strings table (%1 strings)").arg(Spelunky2::get()->get_StringsTable().count()));
    initializeUI();
    reload();
}

void S2Plugin::ViewStringsTable::initializeUI()
{
    // TODO: add reload button
    mMainLayout = new QVBoxLayout();
    setLayout(mMainLayout);

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
    mHTMLDelegate = std::make_unique<StyledItemDelegateHTML>();
    mHTMLDelegate->setCenterVertically(true);
    mMainTableView->setItemDelegateForColumn(gsColStringTableOffset, mHTMLDelegate.get());
    mMainTableView->setItemDelegateForColumn(gsColStringMemoryOffset, mHTMLDelegate.get());
    QObject::connect(mMainTableView, &QTableView::clicked, this, &ViewStringsTable::cellClicked);
    mMainLayout->addWidget(mMainTableView);

    // mModel = new ItemModelStringsTable(mToolbar->stringsTable());
    mModel = new QStandardItemModel(this);
    mModelProxy = new SortFilterProxyModelStringsTable(mStringList, this); // TODO: maybe use the Model itself instead of StringList since the Value column contains all the strings?
    mModelProxy->setSourceModel(mModel);
    mMainTableView->setModel(mModelProxy);
    mMainTableView->setColumnWidth(gsColStringID, 20);
    mMainTableView->setColumnWidth(gsColStringTableOffset, 300);
    mMainTableView->setColumnWidth(gsColStringMemoryOffset, 5);
    mMainTableView->setWordWrap(true);
}
void S2Plugin::ViewStringsTable::reload()
{
    mModel->clear();
    mStringList.clear();
    mModel->setHorizontalHeaderLabels({"ID", "Table offset", "Memory offset", "Value"});
    auto parrent = mModel->invisibleRootItem();
    auto& stringTable = Spelunky2::get()->get_StringsTable();
    mStringList.reserve(stringTable.count());

    for (size_t idx = 0; idx < stringTable.count(); ++idx)
    {
        QStandardItem* fieldID = new QStandardItem(QString("%1").arg(idx));
        auto offset = stringTable.offsetForIndex(idx);
        QStandardItem* fieldTableOfset = new QStandardItem(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", offset));
        fieldTableOfset->setData(offset, gsRoleRawValue);
        auto stringOffset = Script::Memory::ReadQword(offset);
        QStandardItem* fieldMemoryOffset = new QStandardItem(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", stringOffset));
        fieldMemoryOffset->setData(stringOffset, gsRoleRawValue);
        QString str = stringTable.stringForIndex(idx);
        QStandardItem* fieldValue = new QStandardItem(str);

        parrent->appendRow(QList<QStandardItem*>() << fieldID << fieldTableOfset << fieldMemoryOffset << fieldValue);
        mStringList << str;
    }
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
    if (index.column() == gsColStringTableOffset)
    {
        uintptr_t offset = index.data(gsRoleRawValue).toULongLong();
        GuiDumpAt(offset);
        GuiShowCpu();
    }
    else if (index.column() == gsColStringMemoryOffset)
    {
        uintptr_t offset = index.data(gsRoleRawValue).toULongLong();
        GuiDumpAt(offset);
        GuiShowCpu();
    }
}

void S2Plugin::ViewStringsTable::filterTextChanged(const QString& text)
{
    mModelProxy->setFilterString(text);
}
