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

S2Plugin::ViewStdMap::ViewStdMap(ViewToolbar* toolbar, const std::string& keytypeName, const std::string& valuetypeName, size_t mapOffset, QWidget* parent)
    : mMapKeyType(keytypeName), mMapValueType(valuetypeName), mmapOffset(mapOffset), QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    auto config = Configuration::get();
    mMapKeyTypeSize = config->sizeOf(keytypeName);
    mMapValueTypeSize = config->sizeOf(valuetypeName);

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
    StdMap the_map{mmapOffset, mMapKeyAlignment, mMapValueAlignment, mMapKeyTypeSize};
    auto config = Configuration::get();
    mMainTreeView->clear();
    mMemoryFields.clear();
    mMemoryFields.reserve(the_map.size());

    bool add_parrent_object = false;
    MemoryField parent_field;
    parent_field.type = MemoryFieldType::Byte;

    MemoryField key_field;
    key_field.name = "key";
    if (config->isPointer(mMapKeyType))
    {
        key_field.type = MemoryFieldType::PointerType;
        key_field.jsonName = mMapKeyType;
        add_parrent_object = true;
    }
    else if (config->isInlineStruct(mMapKeyType))
    {
        key_field.type = MemoryFieldType::InlineStructType;
        key_field.jsonName = mMapKeyType;
        add_parrent_object = true;
    }
    else if (config->isBuiltInType(mMapKeyType))
    {
        key_field.type = gsMemoryFieldType.find(mMapKeyType)->first;
        // check for the line below
        // add_parrent_object = true;
    }
    else
    {
        dprintf("%s is UNKNOWN\n", mMapKeyType.c_str());
        // not implemented
    }

    MemoryField value_field;
    if (mMapValueTypeSize != 0) // if not StdSet
    {
        value_field.name = "value";
        if (config->isPointer(mMapValueType))
        {
            value_field.type = MemoryFieldType::PointerType;
            value_field.jsonName = mMapValueType;
        }
        else if (config->isInlineStruct(mMapValueType))
        {
            value_field.type = MemoryFieldType::InlineStructType;
            value_field.jsonName = mMapValueType;
        }
        else if (config->isBuiltInType(mMapValueType))
        {
            value_field.type = gsMemoryFieldType.find(mMapValueType)->first;
        }
        else
        {
            dprintf("%s is UNKNOWN\n", mMapValueType.c_str());
            // not implemented
        }
    }

    auto _end = the_map.end();
    auto _cur = the_map.begin();
    for (int x = 0; _cur != _end && x < 100; ++x, ++_cur)
    {
        QStandardItem* parent{nullptr};
        if (add_parrent_object)
        {
            parent_field.name = "obj_" + std::to_string(x);
            parent = mMainTreeView->addMemoryField(parent_field, parent_field.name);
        }
        else
            key_field.name = "key_" + std::to_string(x);

        mMemoryFields.emplace_back(std::make_tuple(key_field, _cur.key_ptr(), parent));
        auto key_StandardItem = mMainTreeView->addMemoryField(key_field, key_field.name, parent);
        if (!add_parrent_object)
            parent = key_StandardItem;

        if (mMapValueTypeSize == 0) // StdSet
            continue;

        mMemoryFields.emplace_back(std::make_tuple(value_field, _cur.value_ptr(), parent));
        mMainTreeView->addMemoryField(value_field, value_field.name, parent);
    }
    refreshData();

    mMainTreeView->updateTableHeader();

    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    mMainTreeView->setColumnHidden(gsColMemoryOffsetDelta, true);
    mMainTreeView->setColumnHidden(gsColComment, true);
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
}

void S2Plugin::ViewStdMap::refreshData()
{
    std::unordered_map<std::string, size_t> offsets;
    auto config = Configuration::get();

    for (const auto& field : mMemoryFields)
    {
        const auto& mem_field = std::get<0>(field);
        const auto& mem_offset = std::get<1>(field);
        const auto& parrent = std::get<2>(field);

        config->setOffsetForField(mem_field, mem_field.name, mem_offset, offsets);
        mMainTreeView->updateValueForField(mem_field, mem_field.name, offsets, 0, parrent);
    }
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
