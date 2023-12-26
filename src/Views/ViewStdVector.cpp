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

S2Plugin::ViewStdVector::ViewStdVector(ViewToolbar* toolbar, const std::string& vectorType, size_t vectorOffset, QWidget* parent)
    : mVectorType(vectorType), mVectorOffset(vectorOffset), QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    mVectorTypeSize = Configuration::get()->sizeOf(mVectorType);

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
    mMainTreeView->setEnableChangeHighlighting(false);

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
    mMemoryFields.clear();

    mVectorBegin = Script::Memory::ReadQword(mVectorOffset);
    auto vectorEnd = Script::Memory::ReadQword(mVectorOffset + sizeof(size_t));
    auto vectorItemCount = (vectorEnd - mVectorBegin) / mVectorTypeSize;

    for (auto x = 0; x < vectorItemCount; ++x)
    {
        MemoryField field;
        field.name = "obj_" + std::to_string(x);
        if (config->isPointer(mVectorType))
        {
            field.type = MemoryFieldType::PointerType;
            field.jsonName = mVectorType;
        }
        else if (config->isInlineStruct(mVectorType))
        {
            field.type = MemoryFieldType::InlineStructType;
            field.jsonName = mVectorType;
        }
        else if (config->isBuiltInType(mVectorType))
        {
            field.type = gsMemoryFieldType.find(mVectorType)->first;
        }
        else
        {
            dprintf("%s is UNKNOWN\n", mVectorType.c_str());
            // not implemented
        }
        mMemoryFields.emplace_back(field);
        mMainTreeView->addMemoryField(field, field.name);
    }
    refreshData();

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    mMainTreeView->setColumnHidden(gsColMemoryOffsetDelta, true);
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
}

void S2Plugin::ViewStdVector::refreshData()
{
    size_t counter = 0;
    std::unordered_map<std::string, size_t> offsets;
    auto config = Configuration::get();

    for (const auto& field : mMemoryFields)
    {
        config->setOffsetForField(field, field.name, mVectorBegin + (counter++ * mVectorTypeSize), offsets);

        mMainTreeView->updateValueForField(field, field.name, offsets);
    }
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
