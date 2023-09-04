#include "Views/ViewStdMap.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

S2Plugin::ViewStdMap::ViewStdMap(ViewToolbar* toolbar, const std::string& keytypeName, const std::string& valuetypeName, size_t mapOffset, QWidget* parent)
    : mMapKeyType(keytypeName), mMapValueType(valuetypeName), mmapOffset(mapOffset), QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    auto m = MemoryMappedData(mToolbar->configuration());
    mMapKeyTypeSize = m.sizeOf(keytypeName);
    mMapValueTypeSize = m.sizeOf(valuetypeName);

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
    mMainTreeView = new TreeViewMemoryFields(mToolbar, nullptr, this);
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
    mAutoRefreshIntervalLineEdit->setText("100");
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
    StdMap the_map{mmapOffset, mMapKeyTypeSize, mMapValueTypeSize};
    auto config = mToolbar->configuration();
    mMainTreeView->clear();
    mMemoryFields.clear();
    mMemoryFields.reserve(the_map.size());

    auto _end = the_map.end();
    auto _cur = the_map.begin();
    for (int x = 0; _cur != _end; ++x, ++_cur)
    {
        MemoryField key_field;

        key_field.name = "key_" + std::to_string(x);
        if (config->isPointer(mMapKeyType))
        {
            key_field.type = MemoryFieldType::PointerType;
            key_field.jsonName = mMapKeyType;
        }
        else if (config->isInlineStruct(mMapKeyType))
        {
            key_field.type = MemoryFieldType::InlineStructType;
            key_field.jsonName = mMapKeyType;
        }
        else if (config->isBuiltInType(mMapKeyType))
        {
            key_field.type = gsJSONStringToMemoryFieldTypeMapping.at(mMapKeyType);
        }
        else
        {
            dprintf("%s is UNKNOWN\n", mMapKeyType.c_str());
            // not implemented
        }
        mMemoryFields.emplace_back(std::make_tuple(key_field, _cur.key_ptr(), nullptr));
        auto key_item = mMainTreeView->addMemoryField(key_field, key_field.name);

        if (mMapValueTypeSize == 0) // StdSet
            continue;

        MemoryField field;
        field.name = "val_" + std::to_string(x);
        if (config->isPointer(mMapValueType))
        {
            field.type = MemoryFieldType::PointerType;
            field.jsonName = mMapValueType;
        }
        else if (config->isInlineStruct(mMapValueType))
        {
            field.type = MemoryFieldType::InlineStructType;
            field.jsonName = mMapValueType;
        }
        else if (config->isBuiltInType(mMapValueType))
        {
            field.type = gsJSONStringToMemoryFieldTypeMapping.at(mMapValueType);
        }
        else
        {
            dprintf("%s is UNKNOWN\n", mMapValueType.c_str());
            // not implemented
        }
        mMemoryFields.emplace_back(std::make_tuple(field, _cur.value_ptr(), key_item));
        mMainTreeView->addMemoryField(field, field.name, key_item);
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
    auto m = MemoryMappedData(mToolbar->configuration());

    for (const auto& field : mMemoryFields)
    {
        const auto& mem_field = std::get<0>(field);
        const auto& mem_offset = std::get<1>(field);
        const auto& parrent = std::get<2>(field);

        m.setOffsetForField(mem_field, mem_field.name, mem_offset, offsets);
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
