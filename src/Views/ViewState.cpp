#include "Views/ViewState.h"
#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/EntityList.h"
#include "Data/State.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewState::ViewState(ViewToolbar* toolbar, State* state, QWidget* parent) : QWidget(parent), mToolbar(toolbar), mState(state)
{
    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("State");
    refreshState();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewState::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewState::refreshState);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewState::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewState::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewState::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewState::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, mState, this);
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::State))
    {
        mMainTreeView->addMemoryField(field, "State." + field.name);
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

void S2Plugin::ViewState::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewState::refreshState()
{
    //mState->refreshOffsets();
    auto& offsets = mState->offsets();
    auto deltaReference = offsets.at("State.p00");
    for (const auto& field : mToolbar->configuration()->typeFields(MemoryFieldType::State))
    {
        mMainTreeView->updateValueForField(field, "State." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewState::toggleAutoRefresh(int newState)
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

void S2Plugin::ViewState::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewState::autoRefreshTimerTrigger()
{
    refreshState();
}

QSize S2Plugin::ViewState::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewState::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewState::label()
{
    for (const auto& [fieldName, offset] : mState->offsets())
    {
        DbgSetAutoLabelAt(offset, fieldName.c_str());
    }
}
