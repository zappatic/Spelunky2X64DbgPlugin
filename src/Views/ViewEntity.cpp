#include "Views/ViewEntity.h"
#include "Configuration.h"
#include "Data/CPPGenerator.h"
#include "Data/Entity.h"
#include "QtHelpers/CPPSyntaxHighlighter.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetMemoryView.h"
#include "QtHelpers/WidgetSpelunkyLevel.h"
#include "Views/ViewToolbar.h"
#include <QCloseEvent>
#include <QFont>
#include <QLabel>
#include <string>

S2Plugin::ViewEntity::ViewEntity(size_t entityOffset, ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar), mEntityPtr(entityOffset)
{
    mMainLayout = new QVBoxLayout(this);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle(QString::asprintf("Entity %s 0x%016llX", Entity{mEntityPtr}.entityTypeName().c_str(), entityOffset));
    mMainTreeView->setVisible(true);

    mMainTreeView->setColumnWidth(gsColField, 175);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    updateMemoryViewOffsetAndSize();

    mSpelunkyLevel->paintEntityMask(0x100, QColor(160, 160, 160)); // 0x100 = FLOOR // TODO: add paint floor and use grid entity matrix
    mSpelunkyLevel->paintEntityUID(Entity{mEntityPtr}.uid(), QColor(222, 52, 235));
    updateLevel();
    toggleAutoRefresh(Qt::Checked);
    mInterpretAsComboBox->setCurrentText(QString::fromStdString(Entity{mEntityPtr}.entityClassName()));
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
    // mInterpretAsComboBox->addItem("");
    mInterpretAsComboBox->addItem("Entity");
    mInterpretAsComboBox->insertSeparator(2);
    std::vector<std::string> classNames;
    for (const auto& [classType, parentClassType] : Configuration::get()->entityClassHierarchy())
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
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setVisible(false);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->updateTableHeader();
    mMainTreeView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    mMainTreeView->setAcceptDrops(true);
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
    // mMainTreeView->updateTableHeader(false);
    mMainTreeView->updateTree();
    if (mMainTabWidget->currentWidget() == mTabMemory)
    {
        mMemoryView->update();
        mMemoryComparisonView->update();
        updateComparedMemoryViewHighlights();
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
    refreshEntity(); // TODO maybe connect with refreshEntity directly?
}

QSize S2Plugin::ViewEntity::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewEntity::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewEntity::interpretAsChanged(const QString& classType)
{
    if (!classType.isEmpty())
    {
        auto textStr = classType.toStdString();
        static const auto colors = {QColor(255, 214, 222), QColor(232, 206, 227), QColor(199, 186, 225), QColor(187, 211, 236), QColor(236, 228, 197), QColor(193, 219, 204)};
        auto config = Configuration::get();

        Entity entity{mEntityPtr};
        // populate tree view
        auto hierarchy = Entity::classHierarchy(classType.toStdString());

        mMainTreeView->clear();
        mMemoryView->clearHighlights();
        mMainTreeView->updateTableHeader();
        uint8_t counter = 0;
        size_t delta = 0;
        uint8_t colorIndex = 0;
        auto recursiveHighlight = [&](std::string prefix, const std::vector<MemoryField>& fields, auto&& self) -> void
        {
            for (auto& field : fields)
            {
                if (!field.isPointer)
                {
                    if (field.type == MemoryFieldType::DefaultStructType)
                    {
                        self(prefix + field.name + ".", config->typeFieldsOfDefaultStruct(field.jsonName), self);
                        continue;
                    }
                    else if (auto& typeFields = config->typeFields(field.type); !typeFields.empty())
                    {
                        // paint elements of types like map, vector etc.
                        self(prefix + field.name + ".", typeFields, self);
                        continue;
                    }
                }

                auto size = field.get_size();
                if (size == 0)
                    continue;
                mMemoryView->addHighlightedField(prefix + field.name, mEntityPtr + delta, size, *(colors.begin() + colorIndex));
                delta += size;
            }
        };

        for (auto it = hierarchy.rbegin(); it != hierarchy.rend(); ++it, ++colorIndex)
        {
            if (colorIndex > colors.size())
                colorIndex = 0;

            MemoryField headerField;
            headerField.name = "<b>" + *it + "</b>";
            headerField.type = MemoryFieldType::EntitySubclass;
            headerField.jsonName = *it;
            auto item = mMainTreeView->addMemoryField(headerField, *it, mEntityPtr + delta, delta);
            // delta = mMainTreeView->getLastItemDelta();
            if (++counter == hierarchy.size())
            {
                mMainTreeView->expandItem(item);
            }

            recursiveHighlight(*it + ".", config->typeFieldsOfEntitySubclass(*it), recursiveHighlight);
        }

        // updateMemoryViewOffsetAndSize();
        // mInterpretAsComboBox->setCurrentText("");
        mMainTreeView->updateTree(0, 0, true);
    }
}

void S2Plugin::ViewEntity::updateMemoryViewOffsetAndSize()
{
    static const size_t defaultBytesShown = 0x188; // max Movable size, non movable size: 0xD0

    mMemoryView->setOffsetAndSize(mEntityPtr, defaultBytesShown);
    mMemoryComparisonView->setOffsetAndSize(mComparisonEntityPtr, defaultBytesShown);
}

void S2Plugin::ViewEntity::updateComparedMemoryViewHighlights()
{
    // TODO
    // const auto color = QColor(255, 221, 184);
    // take difference from mMainTreeView
    // mMemoryComparisonView->addHighlightedField(fieldNameOverride, mMemoryOffsets.at("comparison." + fieldNameOverride), fieldSize, color);
}

void S2Plugin::ViewEntity::updateLevel()
{
    // TODO
    //
    // auto layerName = "layer0";
    // auto entityCameraLayer = mEntity->cameraLayer();
    // if (entityCameraLayer == 1)
    //{
    //     layerName = "layer1";
    // }

    // if (mEntity->comparedEntityMemoryOffset() != 0)
    //{
    //     if (entityCameraLayer == mEntity->comparisonCameraLayer())
    //     {
    //         mSpelunkyLevel->paintEntityUID(mEntity->comparisonUid(), QColor(232, 134, 30));
    //     }
    //     else
    //     {
    //         mSpelunkyLevel->paintEntityUID(mEntity->comparisonUid(), Qt::transparent);
    //     }
    // }

    // auto layer = Script::Memory::ReadQword(mToolbar->state()->offsetForField(layerName));
    // auto entityCount = (std::min)(Script::Memory::ReadDword(layer + 28), 10000u);
    // auto entities = Script::Memory::ReadQword(layer + 8);

    // mSpelunkyLevel->loadEntities(entities, entityCount);

    mSpelunkyLevel->update();
}

void S2Plugin::ViewEntity::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewEntity::entityOffsetDropped(size_t entityOffset)
{
    if (mComparisonEntityPtr != 0)
    {
        mSpelunkyLevel->clearPaintedEntityUID(Entity{mComparisonEntityPtr}.uid());
    }

    mComparisonEntityPtr = entityOffset;
    mMainTreeView->activeColumns.enable(gsColComparisonValue).enable(gsColComparisonValueHex);
    mMainTreeView->setColumnHidden(gsColComparisonValue, false);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, false);
    mMemoryComparisonScrollArea->setVisible(true);
    mMainTreeView->updateTree(0, mComparisonEntityPtr);
    updateMemoryViewOffsetAndSize();
}

void S2Plugin::ViewEntity::tabChanged(int index)
{
    if (mMainTabWidget->currentWidget() == mTabCPP)
    {
        mCPPSyntaxHighlighter->clearRules();
        CPPGenerator g{};
        g.generate(mInterpretAsComboBox->currentText().toStdString(), mCPPSyntaxHighlighter);
        mCPPSyntaxHighlighter->finalCustomRuleAdded();
        mCPPTextEdit->setText(QString::fromStdString(g.result()));
    }
}
