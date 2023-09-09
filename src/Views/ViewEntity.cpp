#include "Views/ViewEntity.h"
#include "Data/CPPGenerator.h"
#include "Data/EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QFont>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mEntity = std::make_unique<Entity>(entityOffset, mMainTreeView, mMemoryView, mMemoryComparisonView, mToolbar->entityDB(), mToolbar->configuration());
    mMainTreeView->setMemoryMappedData(mEntity.get());
    mEntity->populateTreeView();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", mToolbar->configuration()->spelunky2()->getEntityName(entityOffset, toolbar->entityDB()).c_str(), entityOffset));
    mMainTreeView->setVisible(true);

    mEntity->refreshOffsets();
    mEntity->refreshValues();
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 175);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);

    mEntity->populateMemoryView();
    updateMemoryViewOffsetAndSize();

    mSpelunkyLevel->paintEntityMask(0x100, QColor(160, 160, 160)); // 0x100 = FLOOR
    mSpelunkyLevel->paintEntityUID(mEntity->uid(), QColor(222, 52, 235));
    updateLevel();
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewEntity::initializeUI()
{
    mTopLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mTopLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    QObject::connect(mMainTabWidget, &QTabWidget::currentChanged, this, &ViewEntity::tabChanged);
    mMainLayout->addWidget(mMainTabWidget);

    mTabFields = new QWidget();
    mTabMemory = new QWidget();
    mTabLevel = new QWidget();
    mTabCPP = new QWidget();
    mTabFields->setLayout(new QVBoxLayout(mTabFields));
    mTabFields->layout()->setMargin(0);
    mTabMemory->setLayout(new QHBoxLayout(mTabMemory));
    mTabMemory->layout()->setMargin(0);
    mTabLevel->setLayout(new QVBoxLayout(mTabLevel));
    mTabLevel->layout()->setMargin(0);
    mTabCPP->setLayout(new QVBoxLayout(mTabCPP));
    mTabCPP->layout()->setMargin(0);

    mMainTabWidget->addTab(mTabFields, "Fields");
    mMainTabWidget->addTab(mTabMemory, "Memory");
    mMainTabWidget->addTab(mTabLevel, "Level");
    mMainTabWidget->addTab(mTabCPP, "C++");

    // TOP LAYOUT
    mRefreshButton = new QPushButton("Refresh", this);
    mTopLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewEntity::refreshEntity);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewEntity::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mTopLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewEntity::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mTopLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewEntity::autoRefreshIntervalChanged);

    mTopLayout->addWidget(new QLabel("milliseconds", this));

    mTopLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewEntity::label);
    mTopLayout->addWidget(labelButton);

    mTopLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    mInterpretAsComboBox->addItem("");
    mInterpretAsComboBox->addItem("Entity");
    mInterpretAsComboBox->insertSeparator(2);
    std::vector<std::string> classNames;
    for (const auto& [classType, parentClassType] : mToolbar->configuration()->entityClassHierarchy())
    {
        classNames.emplace_back(classType);
    }
    std::sort(classNames.begin(), classNames.end());
    for (const auto& className : classNames)
    {
        mInterpretAsComboBox->addItem(QString::fromStdString(className));
    }
    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewEntity::interpretAsChanged);
    mTopLayout->addWidget(mInterpretAsComboBox);

    // TAB FIELDS
    mMainTreeView = new TreeViewMemoryFields(mToolbar, mEntity.get(), this);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->updateTableHeader();
    mMainTreeView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    mMainTreeView->setAcceptDrops(true);
    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    QObject::connect(mMainTreeView, &TreeViewMemoryFields::entityOffsetDropped, this, &ViewEntity::entityOffsetDropped);
    mTabFields->layout()->addWidget(mMainTreeView);

    // TAB MEMORY
    auto scroll = new QScrollArea(mTabMemory);
    mMemoryView = new WidgetMemoryView(scroll);
    scroll->setStyleSheet("background-color: #fff;");
    scroll->setWidget(mMemoryView);
    scroll->setVisible(true);
    mTabMemory->layout()->addWidget(scroll);

    mMemoryComparisonScrollArea = new QScrollArea(mTabMemory);
    mMemoryComparisonView = new WidgetMemoryView(mMemoryComparisonScrollArea);
    mMemoryComparisonScrollArea->setStyleSheet("background-color: #fff;");
    mMemoryComparisonScrollArea->setWidget(mMemoryComparisonView);
    mMemoryComparisonScrollArea->setVisible(true);
    mTabMemory->layout()->addWidget(mMemoryComparisonScrollArea);
    mMemoryComparisonScrollArea->setVisible(false);

    // TAB LEVEL
    scroll = new QScrollArea(mTabLevel);
    mSpelunkyLevel = new WidgetSpelunkyLevel(scroll);
    scroll->setStyleSheet("background-color: #fff;");
    scroll->setWidget(mSpelunkyLevel);
    scroll->setVisible(true);
    mTabLevel->layout()->addWidget(scroll);

    // TAB CPP
    mCPPTextEdit = new QTextEdit(this);
    mCPPTextEdit->setReadOnly(true);
    auto font = QFont("Courier", 10);
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);
    auto fontMetrics = QFontMetrics(font);
    mCPPTextEdit->setFont(font);
    mCPPTextEdit->setTabStopWidth(4 * fontMetrics.width(' '));
    mCPPTextEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    QPalette palette = mCPPTextEdit->palette();
    palette.setColor(QPalette::Base, QColor("#1E1E1E"));
    palette.setColor(QPalette::Text, QColor("#D4D4D4"));
    mCPPTextEdit->setPalette(palette);
    mCPPTextEdit->document()->setDocumentMargin(10);
    mCPPSyntaxHighlighter = new CPPSyntaxHighlighter(mCPPTextEdit->document());

    mTabCPP->layout()->addWidget(mCPPTextEdit);
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
        mMemoryComparisonView->update();
        mEntity->updateComparedMemoryViewHighlights();
    }
    else if (mMainTabWidget->currentWidget() == mTabLevel)
    {
        updateLevel();
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
        mEntity->interpretAs(textStr);
        updateMemoryViewOffsetAndSize();
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

    auto comparedEntityOffset = mEntity->comparedEntityMemoryOffset();
    mMemoryComparisonView->setOffsetAndSize(comparedEntityOffset, entitySize + mExtraBytesShown);
}

void S2Plugin::ViewEntity::updateLevel()
{
    auto layerName = "layer0";
    auto entityCameraLayer = mEntity->cameraLayer();
    if (entityCameraLayer == 1)
    {
        layerName = "layer1";
    }

    if (mEntity->comparedEntityMemoryOffset() != 0)
    {
        if (entityCameraLayer == mEntity->comparisonCameraLayer())
        {
            mSpelunkyLevel->paintEntityUID(mEntity->comparisonUid(), QColor(232, 134, 30));
        }
        else
        {
            mSpelunkyLevel->paintEntityUID(mEntity->comparisonUid(), Qt::transparent);
        }
    }

    auto layer = Script::Memory::ReadQword(mToolbar->state()->offsetForField(layerName));
    auto entityCount = (std::min)(Script::Memory::ReadDword(layer + 28), 10000u);
    auto entities = Script::Memory::ReadQword(layer + 8);

    mSpelunkyLevel->loadEntities(entities, entityCount);

    mSpelunkyLevel->update();
}

void S2Plugin::ViewEntity::label()
{
    mEntity->label();
}

void S2Plugin::ViewEntity::entityOffsetDropped(size_t entityOffset)
{
    if (mEntity->comparedEntityMemoryOffset() != 0)
    {
        mSpelunkyLevel->clearPaintedEntityUID(mEntity->comparisonUid());
    }

    mEntity->compareToEntity(entityOffset);
    mMainTreeView->setColumnHidden(gsColComparisonValue, false);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, false);
    mMemoryComparisonScrollArea->setVisible(true);
    updateMemoryViewOffsetAndSize();
}

void S2Plugin::ViewEntity::tabChanged(int index)
{
    if (mMainTabWidget->currentWidget() == mTabCPP)
    {
        mCPPSyntaxHighlighter->clearRules();
        CPPGenerator g(mToolbar->configuration());
        g.generate(mEntity->entityType(), mCPPSyntaxHighlighter);
        mCPPSyntaxHighlighter->finalCustomRuleAdded();
        mCPPTextEdit->setText(QString::fromStdString(g.result()));
    }
}

S2Plugin::Entity* S2Plugin::ViewEntity::entity() const
{
    return mEntity.get();
}
