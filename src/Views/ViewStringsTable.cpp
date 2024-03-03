#include "Views/ViewStringsTable.h"
#include "Data/StringsTable.h"
#include "QtHelpers/SortFilterProxyModelStringsTable.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QModelIndex>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

constexpr uint32_t gsRoleRawValue = 1;

S2Plugin::ViewStringsTable::ViewStringsTable(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Strings table (%1 strings)").arg(Spelunky2::get()->get_StringsTable().count()));
    initializeUI();
}

void S2Plugin::ViewStringsTable::initializeUI()
{
    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto topLayout = new QHBoxLayout();
    auto reloadButton = new QPushButton("Reload", this);
    topLayout->addWidget(reloadButton);
    QObject::connect(reloadButton, &QPushButton::clicked, this, &ViewStringsTable::reload);

    mFilterLineEdit = new QLineEdit(this);
    mFilterLineEdit->setPlaceholderText("Search id or text");
    QObject::connect(mFilterLineEdit, &QLineEdit::textChanged, this, &ViewStringsTable::filterTextChanged);
    topLayout->addWidget(mFilterLineEdit);
    mainLayout->addLayout(topLayout);

    mMainTableView = new QTableView(this);
    mMainTableView->setAlternatingRowColors(true);
    mMainTableView->verticalHeader()->setVisible(false);
    mMainTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mMainTableView->verticalHeader()->setDefaultSectionSize(30);
    mMainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mMainTableView->horizontalHeader()->setStretchLastSection(true);
    mMainTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mHTMLDelegate.setCenterVertically(true);
    mMainTableView->setItemDelegateForColumn(gsColStringTableOffset, &mHTMLDelegate);
    mMainTableView->setItemDelegateForColumn(gsColStringMemoryOffset, &mHTMLDelegate);
    QObject::connect(mMainTableView, &QTableView::clicked, this, &ViewStringsTable::cellClicked);
    mainLayout->addWidget(mMainTableView);

    auto mModel = Spelunky2::get()->get_StringsTable().modelCache();
    if (mModel->rowCount() == 0) // there is probably a better way to check if it's "empty"
        reload();

    mModelProxy = new SortFilterProxyModelStringsTable(this);
    mModelProxy->setSourceModel(mModel);
    mMainTableView->setModel(mModelProxy);
    mMainTableView->setColumnWidth(gsColStringID, 50);
    mMainTableView->setColumnWidth(gsColStringTableOffset, 130);
    mMainTableView->setColumnWidth(gsColStringMemoryOffset, 130);
    mMainTableView->setWordWrap(true);
}
void S2Plugin::ViewStringsTable::reload()
{
    auto& stringTable = Spelunky2::get()->get_StringsTable();
    stringTable.modelCache()->clear();
    stringTable.modelCache()->setHorizontalHeaderLabels({"ID", "Table offset", "Memory offset", "Value"});
    auto parrent = stringTable.modelCache()->invisibleRootItem();

    for (size_t idx = 0; idx < stringTable.count(); ++idx)
    {
        QStandardItem* fieldID = new QStandardItem(QString::number(idx));
        auto offset = stringTable.offsetForIndex(idx);
        QStandardItem* fieldTableOfset = new QStandardItem(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", offset));
        fieldTableOfset->setData(offset, gsRoleRawValue);
        auto stringOffset = Script::Memory::ReadQword(offset);
        QStandardItem* fieldMemoryOffset = new QStandardItem(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", stringOffset));
        fieldMemoryOffset->setData(stringOffset, gsRoleRawValue);
        QString str = stringTable.stringForIndex(idx);
        QStandardItem* fieldValue = new QStandardItem(str);

        parrent->appendRow(QList<QStandardItem*>() << fieldID << fieldTableOfset << fieldMemoryOffset << fieldValue);
    }
    // [Known Issue]: Because we use the same model for potentially multiple StringsTable windows
    // the size will only be updated for this window when using "Reload" button
    mMainTableView->setColumnWidth(gsColStringID, 50);
    mMainTableView->setColumnWidth(gsColStringTableOffset, 130);
    mMainTableView->setColumnWidth(gsColStringMemoryOffset, 130);
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
    if (index.column() == gsColStringTableOffset || index.column() == gsColStringMemoryOffset)
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
