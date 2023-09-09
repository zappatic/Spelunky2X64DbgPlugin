#include "Views/ViewGameManager.h"
#include "Configuration.h"
#include "Data/GameManager.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewGameManager::ViewGameManager(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("GameManager");
    refreshGameManager();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewGameManager::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewGameManager::refreshGameManager);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewGameManager::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewGameManager::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewGameManager::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewGameManager::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, mToolbar->gameManager(), this);
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::GameManager))
    {
        mMainTreeView->addMemoryField(field, "GameManager." + field.name);
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

void S2Plugin::ViewGameManager::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewGameManager::refreshGameManager()
{
    mToolbar->gameManager()->refreshOffsets();
    auto& offsets = mToolbar->gameManager()->offsets();
    auto deltaReference = offsets.at("GameManager.backgroundmusic");
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::GameManager))
    {
        mMainTreeView->updateValueForField(field, "GameManager." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewGameManager::toggleAutoRefresh(int newState)
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

void S2Plugin::ViewGameManager::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewGameManager::autoRefreshTimerTrigger()
{
    refreshGameManager();
}

QSize S2Plugin::ViewGameManager::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewGameManager::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewGameManager::label()
{
    for (const auto& [fieldName, offset] : mToolbar->gameManager()->offsets())
    {
        DbgSetAutoLabelAt(offset, fieldName.c_str());
    }
}
