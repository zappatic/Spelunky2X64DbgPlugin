#include "Views/ViewLevelGen.h"
#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/EntityList.h"
#include "Data/LevelGen.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetSpelunkyRooms.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>

S2Plugin::ViewLevelGen::ViewLevelGen(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("LevelGen");
    refreshLevelGen();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewLevelGen::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

    // TOP REFRESH LAYOUT
    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewLevelGen::refreshLevelGen);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewLevelGen::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewLevelGen::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("2000");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewLevelGen::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewLevelGen::label);
    mRefreshLayout->addWidget(labelButton);

    // TABS
    mTabData = new QWidget();
    mTabRooms = new QWidget();
    mTabData->setLayout(new QVBoxLayout(mTabData));
    mTabData->layout()->setMargin(0);
    mTabData->setObjectName("datawidget");
    mTabRooms->setLayout(new QVBoxLayout(mTabRooms));
    mTabRooms->layout()->setMargin(0);

    mMainTabWidget->addTab(mTabData, "Data");
    mMainTabWidget->addTab(mTabRooms, "Rooms");

    // TAB DATA
    {
        mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
        {
            mMainTreeView->addMemoryField(field, "LevelGen." + field.name);
        }
        mTabData->layout()->addWidget(mMainTreeView);

        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        mMainTreeView->setColumnHidden(gsColComparisonValue, true);
        mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::levelGenRoomsPointerClicked, this, &ViewLevelGen::levelGenRoomsPointerClicked);
    }

    // TAB ROOMS
    {
        auto scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        auto containerWidget = new QWidget(this);
        scroll->setWidget(containerWidget);
        auto containerLayout = new QVBoxLayout(containerWidget);

        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
        {
            if (field.type == MemoryFieldType::LevelGenRoomsPointer || field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
            {
                auto roomWidget = new WidgetSpelunkyRooms(field.name, mToolbar, this);
                if (field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
                {
                    roomWidget->setIsMetaData();
                }
                mRoomsWidgets[field.name] = roomWidget;
                containerLayout->addWidget(roomWidget);
            }
        }
        dynamic_cast<QVBoxLayout*>(mTabRooms->layout())->addWidget(scroll);
    }

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewLevelGen::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewLevelGen::refreshLevelGen()
{
    mToolbar->levelGen()->refreshOffsets();
    auto& offsets = mToolbar->levelGen()->offsets();
    auto deltaReference = offsets.at("LevelGen.data");
    for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
    {
        mMainTreeView->updateValueForField(field, "LevelGen." + field.name, offsets, deltaReference);
        if (mMainTabWidget->currentWidget() == mTabRooms && (field.type == MemoryFieldType::LevelGenRoomsPointer || field.type == MemoryFieldType::LevelGenRoomsMetaPointer))
        {
            auto pointerOffset = mToolbar->levelGen()->offsetForField(field.name);
            if (pointerOffset != 0)
            {
                size_t offset = Script::Memory::ReadQword(pointerOffset);
                mRoomsWidgets.at(field.name)->setOffset(offset);
            }
        }
    }
}

void S2Plugin::ViewLevelGen::toggleAutoRefresh(int newLevelGen)
{
    if (newLevelGen == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::ViewLevelGen::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewLevelGen::autoRefreshTimerTrigger()
{
    refreshLevelGen();
}

QSize S2Plugin::ViewLevelGen::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewLevelGen::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewLevelGen::label()
{
    for (const auto& [fieldName, offset] : mToolbar->levelGen()->offsets())
    {
        DbgSetAutoLabelAt(offset, fieldName.c_str());
    }
}

void S2Plugin::ViewLevelGen::levelGenRoomsPointerClicked(const QString& fieldName)
{
    mMainTabWidget->setCurrentWidget(mTabRooms);
}
