#include "Views/ViewStdVector.h"
#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

S2Plugin::ViewStdVector::ViewStdVector(ViewToolbar* toolbar, const std::string& vectorType, uintptr_t vectorOffset, QWidget* parent)
    : mVectorType(vectorType), mVectorOffset(vectorOffset), QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    mVectorTypeSize = Configuration::get()->getTypeSize(mVectorType);

    initializeRefreshLayout();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString("std::vector<%1>").arg(QString::fromStdString(vectorType)));
    mMainTreeView->setVisible(true);

    refreshVectorContents();
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewStdVector::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment);
    mMainLayout->addWidget(mMainTreeView);
}

void S2Plugin::ViewStdVector::initializeRefreshLayout()
{
    auto refreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(refreshLayout);

    auto refreshVectorButton = new QPushButton("Refresh vector", this);
    QObject::connect(refreshVectorButton, &QPushButton::clicked, this, &ViewStdVector::refreshVectorContents);
    refreshLayout->addWidget(refreshVectorButton);

    mRefreshDataButton = new QPushButton("Refresh data", this);
    refreshLayout->addWidget(mRefreshDataButton);
    QObject::connect(mRefreshDataButton, &QPushButton::clicked, this, &ViewStdVector::refreshData);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewStdVector::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh data every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    refreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewStdVector::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    refreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewStdVector::autoRefreshIntervalChanged);

    refreshLayout->addWidget(new QLabel("milliseconds", this));
    refreshLayout->addStretch();
}

void S2Plugin::ViewStdVector::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewStdVector::refreshVectorContents()
{
    auto config = Configuration::get();
    mMainTreeView->clear();

    uintptr_t vectorBegin = Script::Memory::ReadQword(mVectorOffset);
    uintptr_t vectorEnd = Script::Memory::ReadQword(mVectorOffset + sizeof(uintptr_t));
    if (vectorBegin > vectorEnd && Script::Memory::IsValidPtr(vectorBegin) && Script::Memory::IsValidPtr(vectorEnd))
        return;

    auto vectorItemCount = (vectorEnd - vectorBegin) / mVectorTypeSize;
    if ((vectorEnd - vectorBegin) % mVectorTypeSize != 0)
        return; // TODO display error

    // limit big vectors
    vectorItemCount = std::min(300ull, vectorItemCount);

    MemoryField field;
    if (config->isPermanentPointer(mVectorType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mVectorType;
        field.isPointer = true;
    }
    else if (config->isJsonStruct(mVectorType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mVectorType;
    }
    else if (auto type = config->getBuiltInType(mVectorType); type != MemoryFieldType::None)
    {
        field.type = type;
        if (Configuration::isPointerType(type))
            field.isPointer = true;
    }
    else
    {
        dprintf("unknown type in ViewStdVector::refreshVectorContents() (%s)\n", mVectorType.c_str());
        return;
    }
    for (auto x = 0; x < vectorItemCount; ++x)
    {
        field.name = "obj_" + std::to_string(x);
        mMainTreeView->addMemoryField(field, field.name, vectorBegin + x * mVectorTypeSize, x * mVectorTypeSize);
    }
    refreshData();

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
}

void S2Plugin::ViewStdVector::refreshData()
{
    mMainTreeView->updateTree();
}

QSize S2Plugin::ViewStdVector::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewStdVector::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewStdVector::toggleAutoRefresh(int newState)
{
    if (newState == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshDataButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshDataButton->setEnabled(false);
    }
}

void S2Plugin::ViewStdVector::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewStdVector::autoRefreshTimerTrigger()
{
    refreshData();
}
