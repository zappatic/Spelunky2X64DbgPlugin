#include "ViewEntity.h"
#include "EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

ViewEntity::ViewEntity(size_t entityOffset, EntityDB* entityDB, ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mEntityDB(entityDB), mToolbar(toolbar)
{
    mEntity = std::make_unique<Entity>(entityOffset);
    mMainLayout = new QVBoxLayout();

    initializeRefreshStuff();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString::asprintf("Entity 0x%016llX", entityOffset));
    mMainTreeView->setVisible(true);

    refreshEntity();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void ViewEntity::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mEntityDB, mToolbar, this);
    mMainTreeView->addEntityFields();

    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
}

void ViewEntity::initializeRefreshStuff()
{
    mRefreshLayout = new QHBoxLayout();
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewEntity::refreshEntity);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewEntity::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewEntity::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("500");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewEntity::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();
}

void ViewEntity::closeEvent(QCloseEvent* event)
{
    delete this;
}

void ViewEntity::refreshEntity()
{
    mEntity->refreshOffsets();
    for (const auto& field : gsEntityFields)
    {
        mMainTreeView->updateValueForField(field, field.name, mEntity->offsets());
    }
}

void ViewEntity::toggleAutoRefresh(int newState)
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

void ViewEntity::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void ViewEntity::autoRefreshTimerTrigger()
{
    refreshEntity();
}

QSize ViewEntity::sizeHint() const
{
    return QSize(750, 1050);
}

QSize ViewEntity::minimumSizeHint() const
{
    return QSize(750, 1050);
}
