#include "Views/ViewEntity.h"
#include "Data/EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mEntity = std::make_unique<Entity>(entityOffset, mMainTreeView, mMemoryView, mToolbar->entityDB(), mToolbar->configuration());
    mEntity->populateTreeView();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", mToolbar->configuration()->spelunky2()->getEntityName(entityOffset, toolbar->entityDB()).c_str(), entityOffset));
    mMainTreeView->setVisible(true);

    mEntity->refreshOffsets();
    mEntity->refreshValues();
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);

    mEntity->populateMemoryView();
    updateMemoryViewOffsetAndSize();
}

void S2Plugin::ViewEntity::initializeUI()
{
    mTopLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

    mTabFields = new QWidget();
    mTabMemory = new QWidget();
    mTabFields->setLayout(new QVBoxLayout(mTabFields));
    mTabFields->layout()->setMargin(0);
    mTabMemory->setLayout(new QVBoxLayout(mTabMemory));
    mTabMemory->layout()->setMargin(0);

    mMainTabWidget->addTab(mTabFields, "Fields");
    mMainTabWidget->addTab(mTabMemory, "Memory");

    // TOP LAYOUT
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
    mInterpretAsComboBox->insertSeparator(2);
    std::vector<std::string> classNames;
    for (const auto& [classType, parentClassType] : mToolbar->configuration()->entityClassHierarchy())
    {
        classNames.emplace_back(gsMemoryFieldTypeToStringMapping.at(classType));
    }
    std::sort(classNames.begin(), classNames.end());
    for (const auto& className : classNames)
    {
        mInterpretAsComboBox->addItem(QString::fromStdString(className));
    }
    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewEntity::interpretAsChanged);
    mTopLayout->addWidget(mInterpretAsComboBox);

    // TAB FIELDS
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->updateTableHeader();
    mTabFields->layout()->addWidget(mMainTreeView);

    // TAB MEMORY
    auto scroll = new QScrollArea(mTabMemory);
    mMemoryView = new WidgetMemoryView(scroll);
    scroll->setStyleSheet("background-color: #fff;");
    scroll->setWidget(mMemoryView);
    scroll->setVisible(true);
    mTabMemory->layout()->addWidget(scroll);
}

void S2Plugin::ViewEntity::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewEntity::refreshEntity()
{
    mEntity->refreshValues();
    mMainTreeView->updateTableHeader(false);
    if (mMainTabWidget->currentWidget() == mTabMemory)
    {
        mMemoryView->update();
    }
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
                updateMemoryViewOffsetAndSize();
                break;
            }
        }
        mInterpretAsComboBox->setCurrentText("");
    }
}

void S2Plugin::ViewEntity::updateMemoryViewOffsetAndSize()
{
    static const size_t defaultExtraBytesShown = 500;
    auto entityOffset = mEntity->memoryOffset();
    auto entitySize = mEntity->totalMemorySize();
    auto nextEntityOffset = mToolbar->state()->findNextEntity(entityOffset);
    if (nextEntityOffset != 0)
    {
        mExtraBytesShown = (std::min)(defaultExtraBytesShown, nextEntityOffset - (entityOffset + entitySize));
    }
    else
    {
        mExtraBytesShown = defaultExtraBytesShown;
    }
    mMemoryView->setOffsetAndSize(entityOffset, entitySize + mExtraBytesShown);
}