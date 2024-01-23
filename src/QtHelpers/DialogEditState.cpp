#include "QtHelpers/DialogEditState.h"
#include "Configuration.h"
#include "QtHelpers/ItemModelStates.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::DialogEditState::DialogEditState(const QString& fieldName, const std::string& refName, size_t memoryOffset, MemoryFieldType type, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), mMemoryOffset(memoryOffset), mFieldType(type)
{
    setModal(true);
    setWindowTitle("Change value");
    setWindowIcon(QIcon(":/icons/caveman.png"));
    auto layout = new QVBoxLayout(this);

    // STATES
    auto gridLayout = new QGridLayout(this);

    gridLayout->addWidget(new QLabel(QString("Change value of %1").arg(fieldName), this), 0, 0, 1, 2);

    gridLayout->addWidget(new QLabel("New state:", this), 1, 0);

    // look up current state in memory
    int64_t currentState = 0;
    switch (type)
    {
        case MemoryFieldType::State8:
        {
            currentState = Script::Memory::ReadByte(memoryOffset);
            break;
        }
        case MemoryFieldType::State16:
        {
            currentState = Script::Memory::ReadWord(memoryOffset);
            break;
        }
        case MemoryFieldType::State32:
        {
            currentState = Script::Memory::ReadDword(memoryOffset);
            break;
        }
    }

    const auto& states = Configuration::get()->refTitlesOfField(refName);
    mStatesComboBox = new QComboBox(this);
    mStatesModel = new ItemModelStates(states, this);
    mStatesSortProxy = new SortFilterProxyModelStates(states, this);
    mStatesSortProxy->setSourceModel(mStatesModel);
    mStatesComboBox->setModel(mStatesSortProxy);
    mStatesSortProxy->sort(0);
    auto i = mStatesComboBox->findData(currentState);
    if (i != -1)
    {
        mStatesComboBox->setCurrentIndex(i);
    }
    QObject::connect(mStatesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogEditState::stateComboBoxChanged);
    gridLayout->addWidget(mStatesComboBox, 1, 1);

    gridLayout->addWidget(new QLabel("Custom state:", this), 2, 0);

    mStateLineEdit = new QLineEdit(this);
    gridLayout->addWidget(mStateLineEdit, 2, 1);

    // BUTTONS
    auto buttonLayout = new QHBoxLayout();

    auto cancelBtn = new QPushButton("Cancel", this);
    QObject::connect(cancelBtn, &QPushButton::clicked, this, &DialogEditState::cancelBtnClicked);
    cancelBtn->setAutoDefault(false);
    auto changeBtn = new QPushButton("Change", this);
    QObject::connect(changeBtn, &QPushButton::clicked, this, &DialogEditState::changeBtnClicked);
    changeBtn->setAutoDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(changeBtn);

    layout->addLayout(gridLayout);
    layout->addStretch();
    layout->addLayout(buttonLayout);

    setLayout(layout);
}

QSize S2Plugin::DialogEditState::minimumSizeHint() const
{

    return QSize(350, 150);
}

QSize S2Plugin::DialogEditState::sizeHint() const
{
    return minimumSizeHint();
}

void S2Plugin::DialogEditState::cancelBtnClicked()
{
    reject();
}

void S2Plugin::DialogEditState::changeBtnClicked()
{
    switch (mFieldType)
    {
        case MemoryFieldType::State8:
        {
            bool success = false;
            int8_t v = mStateLineEdit->text().toInt(&success);
            if (success)
            {
                Script::Memory::WriteByte(mMemoryOffset, v);
            }
            break;
        }
        case MemoryFieldType::State16:
        {
            bool success = false;
            int16_t v = mStateLineEdit->text().toInt(&success);
            if (success)
            {
                Script::Memory::WriteWord(mMemoryOffset, v);
            }
            break;
        }
        case MemoryFieldType::State32:
        {
            bool success = false;
            int32_t v = mStateLineEdit->text().toInt(&success);
            if (success)
            {
                Script::Memory::WriteDword(mMemoryOffset, v);
            }
            break;
        }
    }
    accept();
}

void S2Plugin::DialogEditState::stateComboBoxChanged(int index)
{
    auto currentState = mStatesComboBox->currentData().toString();
    mStateLineEdit->setText(currentState);
}
