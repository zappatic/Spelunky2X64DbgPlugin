#include "Views/ViewVirtualFunctions.h"
#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

S2Plugin::ViewVirtualFunctions::ViewVirtualFunctions(const std::string& typeName, size_t offset, ViewToolbar* toolbar, QWidget* parent)
    : QWidget(parent), mTypeName(typeName), mMemoryOffset(offset), mToolbar(toolbar)
{
    mModel = std::make_unique<ItemModelVirtualFunctions>(typeName, offset, toolbar, this);
    mSortFilterProxy = std::make_unique<SortFilterProxyModelVirtualFunctions>(typeName, toolbar, this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle(QString("Virtual Functions of %1").arg(QString::fromStdString(typeName)));
}

void S2Plugin::ViewVirtualFunctions::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    mTopLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);

    mTopLayout->addWidget(new QLabel("Jump to function at index:", this));

    mJumpToLineEdit = new QLineEdit(this);
    mJumpToLineEdit->setValidator(new QIntValidator(0, 5000, this));
    mJumpToLineEdit->setFixedWidth(100);
    mTopLayout->addWidget(mJumpToLineEdit);

    auto jumpBtn = new QPushButton("Jump", this);
    QObject::connect(jumpBtn, &QPushButton::clicked, this, &ViewVirtualFunctions::jumpToFunction);
    mTopLayout->addWidget(jumpBtn);
    mTopLayout->addStretch();

    mHTMLDelegate.setCenterVertically(true);

    mFunctionsTable = new QTableView(this);
    mSortFilterProxy->setSourceModel(mModel.get());
    mFunctionsTable->setModel(mSortFilterProxy.get());
    mFunctionsTable->setAlternatingRowColors(true);
    mFunctionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mFunctionsTable->horizontalHeader()->setStretchLastSection(true);
    mFunctionsTable->setItemDelegate(&mHTMLDelegate);
    mFunctionsTable->setColumnWidth(gsColFunctionIndex, 50);
    mFunctionsTable->setColumnWidth(gsColFunctionTableAddress, 130);
    mFunctionsTable->setColumnWidth(gsColFunctionFunctionAddress, 130);
    mSortFilterProxy->sort(0);
    mMainLayout->addWidget(mFunctionsTable);

    QObject::connect(mFunctionsTable, &QTableView::clicked, this, &ViewVirtualFunctions::tableEntryClicked);
}

void S2Plugin::ViewVirtualFunctions::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewVirtualFunctions::sizeHint() const
{
    return QSize(650, 450);
}

QSize S2Plugin::ViewVirtualFunctions::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewVirtualFunctions::tableEntryClicked(const QModelIndex& index)
{
    auto mappedIndex = mSortFilterProxy->mapToSource(index);

    auto column = mappedIndex.column();
    switch (column)
    {
        case gsColFunctionIndex:
        case gsColFunctionSignature:
            // nop
            break;
        case gsColFunctionFunctionAddress:
        {
            auto address = mModel->data(mappedIndex, gsRoleFunctionFunctionAddress).value<size_t>();
            GuiDisasmAt(address, GetContextData(UE_CIP));
            GuiShowCpu();
            break;
        }
        case gsColFunctionTableAddress:
        {
            auto address = mModel->data(mappedIndex, gsRoleFunctionTableAddress).value<size_t>();
            GuiDumpAt(address);
            GuiShowCpu();
            break;
        }
    }
}

void S2Plugin::ViewVirtualFunctions::jumpToFunction(bool b)
{
    auto address = Script::Memory::ReadQword(mMemoryOffset + (mJumpToLineEdit->text().toUInt() * 8));
    GuiDisasmAt(address, GetContextData(UE_CIP));
    GuiShowCpu();
}
