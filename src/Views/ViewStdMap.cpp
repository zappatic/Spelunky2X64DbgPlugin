#include <windows.h>

#include "Configuration.h"
#include "Data/StdMap.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewStdMap.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <Qlayout>

S2Plugin::ViewStdMap::ViewStdMap(ViewToolbar* toolbar, const std::string& keytypeName, const std::string& valuetypeName, uintptr_t mapOffset, QWidget* parent)
    : mMapKeyType(keytypeName), mMapValueType(valuetypeName), mmapOffset(mapOffset), QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    auto config = Configuration::get();
    mMapKeyTypeSize = config->getTypeSize(keytypeName);
    mMapValueTypeSize = config->getTypeSize(valuetypeName);

    mMapKeyAlignment = config->getAlingment(keytypeName);
    mMapValueAlignment = config->getAlingment(valuetypeName);

    initializeRefreshLayout();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    if (mMapValueTypeSize == 0)
        setWindowTitle(QString("std::set<%1>").arg(QString::fromStdString(keytypeName)));
    else
        setWindowTitle(QString("std::map<%1, %2>").arg(QString::fromStdString(keytypeName), QString::fromStdString(valuetypeName)));
    mMainTreeView->setVisible(true);

    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColMemoryOffsetDelta).disable(gsColComment);
    refreshMapContents();
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewStdMap::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setEnableChangeHighlighting(false);

    mMainLayout->addWidget(mMainTreeView);
}

void S2Plugin::ViewStdMap::initializeRefreshLayout()
{
    auto refreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(refreshLayout);

    auto refreshVectorButton = new QPushButton("Refresh map", this);
    QObject::connect(refreshVectorButton, &QPushButton::clicked, this, &ViewStdMap::refreshMapContents);
    refreshLayout->addWidget(refreshVectorButton);

    mRefreshDataButton = new QPushButton("Refresh data", this);
    refreshLayout->addWidget(mRefreshDataButton);
    QObject::connect(mRefreshDataButton, &QPushButton::clicked, this, &ViewStdMap::refreshData);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewStdMap::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh data every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    refreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewStdMap::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("3000");
    refreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewStdMap::autoRefreshIntervalChanged);

    refreshLayout->addWidget(new QLabel("milliseconds", this));
    refreshLayout->addStretch();
}

void S2Plugin::ViewStdMap::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewStdMap::refreshMapContents()
{
    if (!Script::Memory::IsValidPtr(Script::Memory::ReadQword(mmapOffset)))
        return;

    StdMap the_map{mmapOffset, mMapKeyAlignment, mMapValueAlignment, mMapKeyTypeSize};
    auto config = Configuration::get();
    mMainTreeView->clear();

    MemoryField key_field;
    key_field.name = "key";
    if (config->isPermanentPointer(mMapKeyType))
    {
        key_field.type = MemoryFieldType::DefaultStructType;
        key_field.jsonName = mMapKeyType;
        key_field.isPointer = true;
    }
    else if (config->isJsonStruct(mMapKeyType))
    {
        key_field.type = MemoryFieldType::DefaultStructType;
        key_field.jsonName = mMapKeyType;
    }
    else if (auto type = config->getBuiltInType(mMapKeyType); type != MemoryFieldType::None)
    {
        key_field.type = type;
        if (Configuration::isPointerType(type))
            key_field.isPointer = true;
    }
    else
    {
        dprintf("unknown type in ViewStdMap::refreshMapContents() (%s)\n", mMapKeyType.c_str());
        return;
    }

    MemoryField value_field;
    if (mMapValueTypeSize != 0) // if not StdSet
    {
        value_field.name = "value";
        if (config->isPermanentPointer(mMapValueType))
        {
            value_field.type = MemoryFieldType::DefaultStructType;
            value_field.jsonName = mMapValueType;
            value_field.isPointer = true;
        }
        else if (config->isJsonStruct(mMapValueType))
        {
            value_field.type = MemoryFieldType::DefaultStructType;
            value_field.jsonName = mMapValueType;
        }
        else if (auto type = config->getBuiltInType(mMapValueType); type != MemoryFieldType::None)
        {
            value_field.type = type;
            if (Configuration::isPointerType(type))
                value_field.isPointer = true;
        }
        else
        {
            dprintf("unknown type in ViewStdMap::refreshMapContents() (%s)\n", mMapValueType.c_str());
            return;
        }
    }

    auto _end = the_map.end();
    auto _cur = the_map.begin();
    MemoryField parent_field;
    parent_field.type = MemoryFieldType::Dummy;
    for (int x = 0; _cur != _end && x < 300; ++x, ++_cur)
    {
        QStandardItem* parent{nullptr};
        parent_field.name = "obj_" + std::to_string(x);
        parent = mMainTreeView->addMemoryField(parent_field, parent_field.name, 0, 0);

        auto key_StandardItem = mMainTreeView->addMemoryField(key_field, key_field.name, _cur.key_ptr(), 0, parent);

        if (mMapValueTypeSize == 0) // StdSet
            continue;

        mMainTreeView->addMemoryField(value_field, value_field.name, _cur.value_ptr(), 0, parent);
    }
    refreshData();

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
}

void S2Plugin::ViewStdMap::refreshData()
{
    mMainTreeView->updateTree();
}

QSize S2Plugin::ViewStdMap::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewStdMap::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewStdMap::toggleAutoRefresh(int newState)
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

void S2Plugin::ViewStdMap::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewStdMap::autoRefreshTimerTrigger()
{
    refreshData();
}
