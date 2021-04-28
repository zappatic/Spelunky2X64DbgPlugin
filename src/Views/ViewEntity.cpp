#include "Views/ViewEntity.h"
#include "Data/EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mEntity = std::make_unique<Entity>(entityOffset, mMainTreeView, mToolbar->entityDB(), mToolbar->configuration());
    mEntity->populateTreeView();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", getEntityName(entityOffset, toolbar->entityDB()).c_str(), entityOffset));
    mMainTreeView->setVisible(true);

    mEntity->refreshOffsets();
    mEntity->refreshValues();
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
}

void S2Plugin::ViewEntity::initializeUI()
{
    mTopLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mTopLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewEntity::refreshEntity);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewEntity::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mTopLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewEntity::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("500");
    mTopLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewEntity::autoRefreshIntervalChanged);

    mTopLayout->addWidget(new QLabel("milliseconds", this));

    mTopLayout->addStretch();

    mTopLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    mInterpretAsComboBox->addItem("");
    mInterpretAsComboBox->addItem("Entity");
    for (const auto& [classType, parentClassType] : mToolbar->configuration()->entityClassHierarchy())
    {
        mInterpretAsComboBox->addItem(QString::fromStdString(gsMemoryFieldTypeToStringMapping.at(classType)));
    }
    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewEntity::interpretAsChanged);
    mTopLayout->addWidget(mInterpretAsComboBox);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->updateTableHeader();
}

void S2Plugin::ViewEntity::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewEntity::refreshEntity()
{
    mEntity->refreshValues();
    mMainTreeView->updateTableHeader(false);
}

void S2Plugin::ViewEntity::toggleAutoRefresh(int newState)
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

void S2Plugin::ViewEntity::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewEntity::autoRefreshTimerTrigger()
{
    refreshEntity();
}

QSize S2Plugin::ViewEntity::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewEntity::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewEntity::interpretAsChanged(const QString& text)
{
    if (!text.isEmpty())
    {
        auto textStr = text.toStdString();
        for (const auto& [classType, name] : gsMemoryFieldTypeToStringMapping)
        {
            if (textStr == name)
            {
                mEntity->interpretAs(classType);
                break;
            }
        }
        mInterpretAsComboBox->setCurrentText("");
    }
}
