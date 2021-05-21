#include "Views/ViewEntities.h"
#include "Data/EntityList.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>

S2Plugin::ViewEntities::ViewEntities(ViewToolbar* toolbar, QWidget* parent) : QWidget(parent), mToolbar(toolbar)
{
    mMainLayout = new QVBoxLayout(this);

    initializeRefreshAndFilter();
    initializeTreeView();
    setWindowIcon(QIcon(":/icons/caveman.png"));

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    setWindowTitle("Entities");
    mMainTreeView->setVisible(true);

    refreshEntities();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mFilterLineEdit->setFocus();
}

void S2Plugin::ViewEntities::initializeTreeView()
{
    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    mMainTreeView->setEnableChangeHighlighting(false);
    mMainTreeView->setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);
    mMainTreeView->setDragEnabled(true);
    mMainTreeView->setAcceptDrops(false);

    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
}

void S2Plugin::ViewEntities::initializeRefreshAndFilter()
{
    auto filterLayout = new QGridLayout(this);

    auto refreshButton = new QPushButton("Refresh", this);
    QObject::connect(refreshButton, &QPushButton::clicked, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(refreshButton, 0, 0);

    auto label = new QLabel("Filter:", this);
    filterLayout->addWidget(label, 0, 1);

    mFilterLineEdit = new QLineEdit(this);
    QObject::connect(mFilterLineEdit, &QLineEdit::textChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mFilterLineEdit, 0, 2, 1, 6);

    mCheckboxLayer0 = new QCheckBox("Front layer", this);
    mCheckboxLayer0->setChecked(true);
    QObject::connect(mCheckboxLayer0, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLayer0, 2, 2);

    mCheckboxLayer1 = new QCheckBox("Back layer", this);
    QObject::connect(mCheckboxLayer1, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLayer1, 2, 3);

    mCheckboxFLOOR = new QCheckBox("FLOOR*", this);
    QObject::connect(mCheckboxFLOOR, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxFLOOR, 3, 2);

    mCheckboxFLOORSTYLED = new QCheckBox("FLOORSTYLED*", this);
    QObject::connect(mCheckboxFLOORSTYLED, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxFLOORSTYLED, 3, 3);

    mCheckboxDECORATION = new QCheckBox("DECORATION*", this);
    QObject::connect(mCheckboxDECORATION, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxDECORATION, 3, 4);

    mCheckboxEMBED = new QCheckBox("EMBED*", this);
    QObject::connect(mCheckboxEMBED, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxEMBED, 3, 5);

    mCheckboxCHAR = new QCheckBox("CHAR*", this);
    mCheckboxCHAR->setChecked(true);
    QObject::connect(mCheckboxCHAR, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxCHAR, 3, 6);

    mCheckboxMONS = new QCheckBox("MONS*", this);
    mCheckboxMONS->setChecked(true);
    QObject::connect(mCheckboxMONS, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxMONS, 3, 7);

    mCheckboxITEM = new QCheckBox("ITEM*", this);
    mCheckboxITEM->setChecked(true);
    QObject::connect(mCheckboxITEM, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxITEM, 3, 8);

    mCheckboxACTIVEFLOOR = new QCheckBox("ACTIVEFLOOR*", this);
    QObject::connect(mCheckboxACTIVEFLOOR, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxACTIVEFLOOR, 4, 2);

    mCheckboxFX = new QCheckBox("FX*", this);
    QObject::connect(mCheckboxFX, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxFX, 4, 3);

    mCheckboxBG = new QCheckBox("BG*", this);
    QObject::connect(mCheckboxBG, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxBG, 4, 4);

    mCheckboxMIDBG = new QCheckBox("MIDBG*", this);
    QObject::connect(mCheckboxMIDBG, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxMIDBG, 4, 5);

    mCheckboxLOGICAL = new QCheckBox("LOGICAL*", this);
    QObject::connect(mCheckboxLOGICAL, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLOGICAL, 4, 6);

    mCheckboxMOUNT = new QCheckBox("MOUNT*", this);
    QObject::connect(mCheckboxMOUNT, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxMOUNT, 4, 7);

    mCheckboxLIQUID = new QCheckBox("LIQUID*", this);
    QObject::connect(mCheckboxLIQUID, &QCheckBox::stateChanged, this, &ViewEntities::refreshEntities);
    filterLayout->addWidget(mCheckboxLIQUID, 4, 8);

    auto horLayout = new QHBoxLayout(this);
    horLayout->addLayout(filterLayout);
    horLayout->addStretch();
    mMainLayout->addLayout(horLayout);
}

void S2Plugin::ViewEntities::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewEntities::refreshEntities()
{
    mMainTreeView->clear();
    std::unordered_map<std::string, size_t> offsets;

    auto insertEntities = [&](size_t layerEntities, uint32_t count) -> size_t
    {
        size_t maximum = (std::min)(count, 10000u);
        size_t counter = 0;
        for (auto x = 0; x < maximum; ++x)
        {
            auto entityPtr = layerEntities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityUid = Script::Memory::ReadDword(entity + 56);
            auto entityName = QString::fromStdString(mToolbar->configuration()->spelunky2()->getEntityName(entity, mToolbar->entityDB()));

            auto matchesFilter = false;
            if (mCheckboxFLOOR->checkState() == Qt::Checked && entityName.startsWith("FLOOR_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxFLOORSTYLED->checkState() == Qt::Checked && entityName.startsWith("FLOORSTYLED_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxDECORATION->checkState() == Qt::Checked && entityName.startsWith("DECORATION_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxEMBED->checkState() == Qt::Checked && entityName.startsWith("EMBED_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxCHAR->checkState() == Qt::Checked && entityName.startsWith("CHAR_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxMONS->checkState() == Qt::Checked && entityName.startsWith("MONS_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxITEM->checkState() == Qt::Checked && entityName.startsWith("ITEM_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxACTIVEFLOOR->checkState() == Qt::Checked && entityName.startsWith("ACTIVEFLOOR_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxFX->checkState() == Qt::Checked && entityName.startsWith("FX_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxBG->checkState() == Qt::Checked && entityName.startsWith("BG_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxMIDBG->checkState() == Qt::Checked && entityName.startsWith("MIDBG_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxLOGICAL->checkState() == Qt::Checked && entityName.startsWith("LOGICAL_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxMOUNT->checkState() == Qt::Checked && entityName.startsWith("MOUNT_"))
            {
                matchesFilter = true;
            }
            else if (mCheckboxLIQUID->checkState() == Qt::Checked && entityName.startsWith("LIQUID_"))
            {
                matchesFilter = true;
            }
            if (matchesFilter && !mFilterLineEdit->text().isEmpty())
            {
                if (!entityName.contains(mFilterLineEdit->text(), Qt::CaseInsensitive))
                {
                    matchesFilter = false;
                }
            }

            if (matchesFilter)
            {
                MemoryField field;
                field.name = "entity_uid_" + std::to_string(entityUid);
                field.type = MemoryFieldType::EntityPointer;
                mMainTreeView->addMemoryField(field, field.name);
                offsets[field.name] = entityPtr;
                mMainTreeView->updateValueForField(field, field.name, offsets);
                counter++;
            }
        }
        return counter;
    };

    size_t totalEntities = 0;
    if (mCheckboxLayer0->checkState() == Qt::Checked)
    {
        auto layer0 = Script::Memory::ReadQword(mToolbar->state()->offsetForField("layer0"));
        auto layer0Count = Script::Memory::ReadDword(layer0 + 28);
        auto layerEntities = Script::Memory::ReadQword(layer0 + 8);
        totalEntities += insertEntities(layerEntities, layer0Count);
    }

    if (mCheckboxLayer1->checkState() == Qt::Checked)
    {
        auto layer1 = Script::Memory::ReadQword(mToolbar->state()->offsetForField("layer1"));
        auto layer1Count = Script::Memory::ReadDword(layer1 + 28);
        auto layerEntities = Script::Memory::ReadQword(layer1 + 8);
        totalEntities += insertEntities(layerEntities, layer1Count);
    }
    setWindowTitle(QString("%1 Entities").arg(totalEntities));

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
}

QSize S2Plugin::ViewEntities::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewEntities::minimumSizeHint() const
{
    return QSize(150, 150);
}
