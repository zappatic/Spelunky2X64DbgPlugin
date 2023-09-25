#include "Views/ViewSaveGame.h"
#include "Configuration.h"
#include "Data/SaveGame.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewSaveGame::ViewSaveGame(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("SaveGame");
    refreshSaveGame();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewSaveGame::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewSaveGame::refreshSaveGame);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewSaveGame::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewSaveGame::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewSaveGame::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewSaveGame::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, mToolbar->savegame(), this);
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::SaveGame))
    {
        mMainTreeView->addMemoryField(field, "SaveGame." + field.name);
    }
    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->updateTableHeader();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewSaveGame::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewSaveGame::refreshSaveGame()
{
    mToolbar->savegame()->refreshOffsets();
    auto& offsets = mToolbar->savegame()->offsets();
    auto deltaReference = offsets.at("SaveGame.places");
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::SaveGame))
    {
        mMainTreeView->updateValueForField(field, "SaveGame." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewSaveGame::toggleAutoRefresh(int newState)
{
    if (newState == Qt::Unchecked)
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

void S2Plugin::ViewSaveGame::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewSaveGame::autoRefreshTimerTrigger()
{
    refreshSaveGame();
}

QSize S2Plugin::ViewSaveGame::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewSaveGame::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewSaveGame::label()
{
    for (const auto& [fieldName, offset] : mToolbar->savegame()->offsets())
    {
        DbgSetAutoLabelAt(offset, fieldName.c_str());
    }
}
