#include "QtHelpers/TreeViewMemoryFields.h"

#include "Configuration.h"
#include "Data/CharacterDB.h"
#include "Data/Entity.h"
#include "Data/EntityDB.h"
#include "Data/ParticleDB.h"
#include "Data/State.h"
#include "Data/StdString.h"
#include "Data/StringsTable.h"
#include "Data/TextureDB.h"
#include "QtHelpers/DialogEditSimpleValue.h"
#include "QtHelpers/DialogEditState.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "Spelunky2.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewToolbar.h"
#include "make_unsigned_integer.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QDrag>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTextCodec>
#include <inttypes.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

S2Plugin::TreeViewMemoryFields::TreeViewMemoryFields(ViewToolbar* toolbar, QWidget* parent) : QTreeView(parent), mToolbar(toolbar)
{
    mHTMLDelegate = std::make_unique<StyledItemDelegateHTML>();
    setItemDelegate(mHTMLDelegate.get());
    setAlternatingRowColors(true);
    mModel = new QStandardItemModel(this);
    setModel(mModel);

    setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);
    setDragEnabled(true);
    setAcceptDrops(false);

    QObject::connect(this, &QTreeView::clicked, this, &TreeViewMemoryFields::cellClicked);
}

void S2Plugin::TreeViewMemoryFields::addMemoryFields(const std::vector<MemoryField>& fields, const std::string& mainName, uintptr_t structAddr, size_t initialDelta, QStandardItem* parent)
{
    size_t currentOffset = structAddr;
    size_t currentDelta = initialDelta;

    for (auto& field : fields)
    {
        addMemoryField(field, mainName + "." + field.name, currentOffset, currentDelta, parent);
        auto size = field.get_size();
        currentDelta += size;
        if (structAddr != 0)
            currentOffset += size;
    }
}

QStandardItem* S2Plugin::TreeViewMemoryFields::addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, uintptr_t offset, size_t delta, QStandardItem* parent)
{
    auto createAndInsertItem = [&delta](const MemoryField& field, const std::string& fieldNameUID, QStandardItem* itemParent, uintptr_t memOffset, bool showDelta = true) -> QStandardItem*
    {
        auto itemFieldName = new QStandardItem();
        itemFieldName->setEditable(false);
        itemFieldName->setData(QString::fromStdString(field.name), Qt::DisplayRole);
        itemFieldName->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldName->setData(QVariant::fromValue(field.type), gsRoleType);
        itemFieldName->setData(field.isPointer, gsRoleIsPointer);
        itemFieldName->setData(memOffset, gsRoleMemoryOffset);

        auto itemFieldValue = new QStandardItem();
        itemFieldValue->setEditable(false);
        if (field.isPointer == false) // if it's pointer, we set it on first update
            itemFieldValue->setData(memOffset, gsRoleMemoryOffset);
        itemFieldValue->setData("", Qt::DisplayRole);

        auto itemFieldValueHex = new QStandardItem();
        itemFieldValueHex->setEditable(false);
        itemFieldValueHex->setData("", Qt::DisplayRole);

        auto itemFieldComparisonValue = new QStandardItem();
        itemFieldComparisonValue->setEditable(false);
        itemFieldComparisonValue->setData("", Qt::DisplayRole);

        auto itemFieldComparisonValueHex = new QStandardItem();
        itemFieldComparisonValueHex->setEditable(false);
        itemFieldComparisonValueHex->setData("", Qt::DisplayRole);

        auto itemFieldMemoryOffset = new QStandardItem();
        itemFieldMemoryOffset->setEditable(false);
        if (memOffset == 0)
            itemFieldMemoryOffset->setData("", Qt::DisplayRole);
        else
        {
            itemFieldMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memOffset), Qt::DisplayRole);
            // for click event. I could just use the itemFieldName(gsRoleMemoryOffset) for it, but doing it this way for potential itemComparisonFieldMemoryOffset in future
            itemFieldMemoryOffset->setData(memOffset, gsRoleRawValue);
        }

        auto itemFieldMemoryOffsetDelta = new QStandardItem();
        itemFieldMemoryOffsetDelta->setEditable(false);
        if (showDelta)
        {
            itemFieldMemoryOffsetDelta->setData(QString::asprintf("+0x%llX", delta), Qt::DisplayRole);
            itemFieldMemoryOffsetDelta->setData(delta, gsRoleRawValue);
        }
        else
        {
            itemFieldMemoryOffsetDelta->setData("", Qt::DisplayRole); // this should only ever happen for flag field
        }

        auto itemFieldComment = new QStandardItem();
        itemFieldComment->setEditable(false);
        itemFieldComment->setData(QString::fromStdString(field.comment).toHtmlEscaped(), Qt::DisplayRole);

        auto itemFieldType = new QStandardItem();
        itemFieldType->setEditable(false);

        QString typeName = field.isPointer ? "<b>P</b>: " : ""; // TODO: add color?
        if (field.type == MemoryFieldType::EntitySubclass || field.type == MemoryFieldType::DefaultStructType)
            typeName += QString::fromStdString(field.jsonName);
        else if (auto str = Configuration::getTypeDisplayName(field.type); !str.empty())
            typeName += QString::fromUtf8(str.data(), str.size());
        else
            typeName += "Unknown field type";

        itemFieldType->setData(typeName, Qt::DisplayRole);

        itemParent->appendRow(QList<QStandardItem*>() << itemFieldName << itemFieldValue << itemFieldValueHex << itemFieldComparisonValue << itemFieldComparisonValueHex << itemFieldMemoryOffset
                                                      << itemFieldMemoryOffsetDelta << itemFieldType << itemFieldComment);

        return itemFieldName;
    };

    if (parent == nullptr)
    {
        parent = mModel->invisibleRootItem();
    }

    QStandardItem* returnField = nullptr;
    switch (field.type)
    {
        case MemoryFieldType::Skip:
        {
            // TODO: add skip when set in settings
            // save offset with gsRoleSize
            break;
        }
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Double:
        case MemoryFieldType::Bool:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityUIDPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::TextureDBPointer:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::CharacterDBID:
        case MemoryFieldType::LevelGenPointer:
        case MemoryFieldType::ParticleDBPointer:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::LevelGenRoomsPointer:
        case MemoryFieldType::LevelGenRoomsMetaPointer:
        case MemoryFieldType::JournalPagePointer:
        case MemoryFieldType::ThemeInfoName:
        case MemoryFieldType::UTF16Char:
        case MemoryFieldType::IPv4Address:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            returnField->setData(field.size, gsRoleSize);
            break;
        }
        case MemoryFieldType::State8:
        case MemoryFieldType::State16:
        case MemoryFieldType::State32:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            break;
        }
        case MemoryFieldType::Flags32:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent, offset);
            flagsParent->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            for (uint8_t x = 1; x <= 32; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                bool showDelta = false;
                if ((x - 1) % 8 == 0)
                {
                    delta += x == 1 ? 0 : 1;
                    showDelta = true;
                }
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent, 0, showDelta);
                flagFieldItem->setData(x, gsRoleFlagIndex);
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::Flags16:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent, offset, delta);
            flagsParent->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            for (uint8_t x = 1; x <= 16; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                bool showDelta = x == 1 || x == 9;
                delta += x == 9 ? 1 : 0;
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent, 0, showDelta);
                flagFieldItem->setData(x, gsRoleFlagIndex);
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::Flags8:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent, offset, delta);
            flagsParent->setData(QVariant::fromValue(field.firstParameterType), gsRoleRefName);
            for (uint8_t x = 1; x <= 8; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                bool showDelta = x == 1; // for 8 bit flag we only show the first
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent, 0, showDelta);
                flagFieldItem->setData(x, gsRoleFlagIndex);
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct("ThemeInfoPointer"), fieldNameOverride, 0, 0, returnField);
            break;
        }
        case MemoryFieldType::StdVector:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleStdContainerFirstParameterType);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, offset, delta, returnField);

            break;
        }
        case MemoryFieldType::StdMap:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            returnField->setData(QVariant::fromValue(field.firstParameterType), gsRoleStdContainerFirstParameterType);
            returnField->setData(QVariant::fromValue(field.secondParameterType), gsRoleStdContainerSecondParameterType);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, offset, delta, returnField);

            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, 0);
            addMemoryFields(Configuration::get()->typeFieldsOfEntitySubclass(field.jsonName), fieldNameOverride, offset, delta, returnField);
            break;
        }
        case MemoryFieldType::DefaultStructType:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), fieldNameOverride, 0, 0, returnField);
            else
                addMemoryFields(Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), fieldNameOverride, offset, delta, returnField);

            break;
        }
        default:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent, offset);
            if (field.isPointer)
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, 0, 0, returnField);
            else
                addMemoryFields(Configuration::get()->typeFields(field.type), fieldNameOverride, offset, delta, returnField);

            break;
        }
    }
    return returnField;
}

void S2Plugin::TreeViewMemoryFields::updateTableHeader(bool restoreColumnWidths)
{
    mModel->setHorizontalHeaderLabels({"Field", "Value", "Value (hex)", "Comparison value", "Comparison value (hex)", "Memory offset", "Δ", "Type", "Comment"});

    setColumnHidden(gsColField, !activeColumns.test(gsColField));
    setColumnHidden(gsColValue, !activeColumns.test(gsColValue));
    setColumnHidden(gsColValueHex, !activeColumns.test(gsColValueHex));
    setColumnHidden(gsColComparisonValue, !activeColumns.test(gsColComparisonValue));
    setColumnHidden(gsColComparisonValueHex, !activeColumns.test(gsColComparisonValueHex));
    setColumnHidden(gsColMemoryOffset, !activeColumns.test(gsColMemoryOffset));
    setColumnHidden(gsColMemoryOffsetDelta, !activeColumns.test(gsColMemoryOffsetDelta));
    setColumnHidden(gsColType, !activeColumns.test(gsColType));
    setColumnHidden(gsColComment, !activeColumns.test(gsColComment));

    if (restoreColumnWidths)
    {
        if (mSavedColumnWidths[gsColField] != 0)
        {
            setColumnWidth(gsColField, mSavedColumnWidths[gsColField]);
        }
        if (mSavedColumnWidths[gsColValue] != 0)
        {
            setColumnWidth(gsColValue, mSavedColumnWidths[gsColValue]);
        }
        if (mSavedColumnWidths[gsColValueHex] != 0)
        {
            setColumnWidth(gsColValueHex, mSavedColumnWidths[gsColValueHex]);
        }
        if (mSavedColumnWidths[gsColComparisonValue] != 0)
        {
            setColumnWidth(gsColComparisonValue, mSavedColumnWidths[gsColComparisonValue]);
        }
        if (mSavedColumnWidths[gsColComparisonValueHex] != 0)
        {
            setColumnWidth(gsColComparisonValueHex, mSavedColumnWidths[gsColComparisonValueHex]);
        }
        if (mSavedColumnWidths[gsColMemoryOffset] != 0)
        {
            setColumnWidth(gsColMemoryOffset, mSavedColumnWidths[gsColMemoryOffset]);
        }
        if (mSavedColumnWidths[gsColMemoryOffsetDelta] != 0)
        {
            setColumnWidth(gsColMemoryOffsetDelta, mSavedColumnWidths[gsColMemoryOffsetDelta]);
        }
        if (mSavedColumnWidths[gsColType] != 0)
        {
            setColumnWidth(gsColType, mSavedColumnWidths[gsColType]);
        }
        if (mSavedColumnWidths[gsColComment] != 0)
        {
            setColumnWidth(gsColComment, mSavedColumnWidths[gsColComment]);
        }
    }
}

void S2Plugin::TreeViewMemoryFields::updateTree(uintptr_t newAddr, uintptr_t newComparisonAddr, bool initial)
{
    for (int row = 0; row < mModel->invisibleRootItem()->rowCount(); ++row)
    {
        updateRow(row, newAddr == 0 ? std::nullopt : std::optional<uintptr_t>(newAddr), newComparisonAddr == 0 ? std::nullopt : std::optional<uintptr_t>(newComparisonAddr), nullptr, initial);
    }
}

// this would be much better as lambda function, but lamba with templates is C++20 thing
// hope that the compiler can inline and optimise all of this 🙏
template <typename T>
inline std::optional<T> updateField(QStandardItem* itemField, uintptr_t memoryOffset, QStandardItem* itemValue, const char* valueFormat, QStandardItem* itemValueHex, bool isPointer,
                                    const char* hexFormat, bool updateBackground, bool resetBackgroundToTransparent, QColor& background)
{
    std::optional<T> value;
    if (memoryOffset == 0)
    {
        itemValue->setData("", Qt::DisplayRole);
        if (!isPointer)
            itemValueHex->setData("", Qt::DisplayRole);

        itemValue->setData(QVariant{}, S2Plugin::gsRoleRawValue);
    }
    else
    {
        T valueTmp = S2Plugin::Read<T>(memoryOffset);
        value = valueTmp;
        auto data = itemValue->data(S2Plugin::gsRoleRawValue);
        T valueOld = data.value<T>();
        if (data.isNull() || value.value() != valueOld)
        {
            if (updateBackground)
                itemField->setBackground(background);

            if (valueFormat != nullptr)
            {
                itemValue->setData(QString::asprintf(valueFormat, value.value()), Qt::DisplayRole);
            }
            if (!isPointer)
            {
                using unsignedT = make_uint<T>;
                auto newHexValue = QString::asprintf(hexFormat, reinterpret_cast<unsignedT&>(value.value()));
                itemValueHex->setData(newHexValue, Qt::DisplayRole);
            }
            itemValue->setData(value.value(), S2Plugin::gsRoleRawValue);
        }
        else if (updateBackground && resetBackgroundToTransparent)
            itemField->setBackground(Qt::transparent);
    }
    return value;
}

void S2Plugin::TreeViewMemoryFields::updateRow(int row, std::optional<uintptr_t> newAddr, std::optional<uintptr_t> newAddrComparison, QStandardItem* parent, bool disableChangeHighlighting)
{
    if (parent == nullptr)
        parent = mModel->invisibleRootItem();

    QStandardItem* itemField = parent->child(row, gsColField);
    QStandardItem* itemValue = parent->child(row, gsColValue);
    QStandardItem* itemValueHex = parent->child(row, gsColValueHex);
    QStandardItem* itemComparisonValue = parent->child(row, gsColComparisonValue);
    QStandardItem* itemComparisonValueHex = parent->child(row, gsColComparisonValueHex);

    if (itemField == nullptr)
    {
        dprintf("ERROR: tried to updateRow(%d) but did not find itemField in treeview\n", row);
        return;
    }
    else if (itemValue == nullptr || itemValueHex == nullptr || itemComparisonValue == nullptr || itemComparisonValueHex == nullptr)
    {
        dprintf("ERROR: tried to updateRow(%d), field '%s', but did not find items in treeview\n", row, itemField->data(gsRoleUID).toString().toStdString().c_str());
        return;
    }

    MemoryFieldType fieldType = itemField->data(gsRoleType).value<MemoryFieldType>();
    if (fieldType == MemoryFieldType::Skip) // TODO: change when setting for it is available
        return;

    if (fieldType == MemoryFieldType::None)
    {
        dprintf("ERROR: unknown type in updateRow('%s' row: %d)\n", itemField->data(gsRoleUID).toString().toStdString().c_str(), row);
        return;
    }

    bool isPointer = itemField->data(gsRoleIsPointer).toBool();
    uintptr_t memoryOffset = 0;
    uintptr_t comparisonMemoryOffset = 0;
    const auto comparisonDifferenceColor = QColor::fromRgb(255, 221, 184);
    QColor highlightColor = (mEnableChangeHighlighting && !disableChangeHighlighting) ? QColor::fromRgb(255, 184, 184) : Qt::transparent;
    // updating memory offset
    if (newAddr.has_value()) // if (fieldType != MemoryFieldType::Flag && fieldType != MemoryFieldType::EntitySubclass) // there should never be a situation when they get the memoryoffset updated
    {
        QStandardItem* itemMemoryOffset = parent->child(row, gsColMemoryOffset);
        QStandardItem* itemMemoryOffsetDelta = parent->child(row, gsColMemoryOffsetDelta);
        auto deltaData = itemMemoryOffsetDelta->data(gsRoleRawValue);
        memoryOffset = newAddr.value() == 0 ? 0 : newAddr.value() + deltaData.toULongLong();
        if (!deltaData.isNull())
        {
            itemMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memoryOffset), Qt::DisplayRole);
            itemMemoryOffset->setData(memoryOffset, gsRoleRawValue);
        }
        itemField->setData(memoryOffset, gsRoleMemoryOffset);
        if (isPointer == false)
            itemValue->setData(memoryOffset, gsRoleMemoryOffset);
    }
    else
        memoryOffset = itemField->data(gsRoleMemoryOffset).toULongLong();

    // updating memory offset for comparison
    if (newAddrComparison.has_value())
    {
        QStandardItem* itemMemoryOffsetDelta = parent->child(row, gsColMemoryOffsetDelta);
        comparisonMemoryOffset = newAddrComparison.value() == 0 ? 0 : newAddrComparison.value() + itemMemoryOffsetDelta->data(gsRoleRawValue).toULongLong();
        itemField->setData(comparisonMemoryOffset, gsRoleComparisonMemoryOffset);
        if (isPointer == false)
            itemComparisonValue->setData(comparisonMemoryOffset, gsRoleMemoryOffset);
    }
    else
        comparisonMemoryOffset = itemField->data(gsRoleComparisonMemoryOffset).toULongLong();

    bool pointerUpdate = false;
    bool comparisonPointerUpdate = false;
    bool comparisonPointerDifference = false;
    bool comparisonActive = activeColumns.test(gsColComparisonValue) || activeColumns.test(gsColComparisonValueHex);
    uintptr_t newPointer = 0;
    uintptr_t newComparisonPointer = 0;
    uintptr_t valueMemoryOffset = memoryOffset;                     // 0, memory offset or pointer value (no bad values)
    uintptr_t valueComparisonMemoryOffset = comparisonMemoryOffset; // 0, memory offset or pointer value (no bad values)

    if (isPointer)
    {
        // dealing with itemValueHex and itemComparisonValueHex for all pointers and check if the pointer changed
        auto checkAndUpdatePointer = [fieldType](uintptr_t& pointerValue, QStandardItem* valueHexField) -> bool
        {
            auto oldData = valueHexField->data(gsRoleRawValue);
            uintptr_t oldPointer = oldData.toULongLong();
            auto pointertmp = pointerValue;
            if (oldData.isNull() || oldPointer != pointerValue)
            {
                QString newHexValue;
                if (pointerValue == 0)
                    newHexValue = "<font color='#aaa'>nullptr</font>";
                else if (!Script::Memory::IsValidPtr(pointerValue))
                {
                    newHexValue = "<font color='#aaa'>bad ptr</font>";
                    pointerValue = 0;
                }
                else
                    newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", pointerValue);

                // TODO: if we could set the color separately, we could avoid this check, (we would also then not need the newPointer and newComparisonPointer to have scope on the whole function?)
                if (fieldType != MemoryFieldType::CodePointer) // for Code pointer we paint it green instead
                {
                    valueHexField->setData(newHexValue, Qt::DisplayRole);
                }
                valueHexField->setData(pointertmp, gsRoleRawValue);
                return true;
            }
            return false;
        };
        newPointer = Script::Memory::ReadQword(memoryOffset);
        valueMemoryOffset = newPointer;
        pointerUpdate = checkAndUpdatePointer(valueMemoryOffset, itemValueHex);
        itemValue->setData(valueMemoryOffset, gsRoleMemoryOffset);

        if (pointerUpdate)
            itemField->setBackground(highlightColor);
        else
            itemField->setBackground(Qt::transparent); // can be updated later if needed

        if (comparisonActive)
        {
            newComparisonPointer = Script::Memory::ReadQword(comparisonMemoryOffset);
            valueComparisonMemoryOffset = newComparisonPointer;
            comparisonPointerUpdate = checkAndUpdatePointer(valueComparisonMemoryOffset, itemComparisonValueHex);
            itemComparisonValue->setData(valueComparisonMemoryOffset, gsRoleMemoryOffset);

            comparisonPointerDifference = newPointer != newComparisonPointer;
            itemComparisonValueHex->setBackground(comparisonPointerDifference ? comparisonDifferenceColor : Qt::transparent);
        }
    }

    auto shouldUpdateChildren = false;
    if (auto modelIndex = mModel->indexFromItem(itemField); modelIndex.isValid())
    {
        if (itemField->hasChildren())
        {
            shouldUpdateChildren = isExpanded(modelIndex);

            // always update if memory offset was changed
            if (pointerUpdate || comparisonPointerUpdate || newAddr.has_value() || newAddrComparison.has_value())
                shouldUpdateChildren = true;
        }
    }

    auto flagsString = [](uint32_t value, uint8_t size)
    {
        std::stringstream ss;
        uint8_t counter = 0;
        for (uint8_t x = size - 1;; --x)
        {
            if (counter % 4 == 0)
            {
                ss << (x + 1) << ": ";
            }
            if ((value & (1u << x)) == (1u << x))
            {
                ss << "<font color='green'>Y</font> ";
            }
            else
            {
                ss << "<font color='red'>N</font> ";
            }
            counter++;
            if (x == 0)
                break;
        }
        return ss;
    };

    switch (fieldType)
    {
        case MemoryFieldType::CodePointer:
        {
            if (pointerUpdate)
            {
                QString newValue;
                if (newPointer == 0)
                    newValue = "<font color='#aaa'>nullptr</font>";
                else if (valueMemoryOffset == 0) // trick to not use Script::Memory::IsValidPtr(newPointer) again
                    newValue = "<font color='#aaa'>bad ptr</font>";
                else
                    newValue = QString::asprintf("<font color='green'><u>0x%016llX</u></font>", newPointer);

                itemValue->setData(newValue, Qt::DisplayRole);
                itemValueHex->setData(newValue, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (comparisonPointerUpdate)
                {
                    QString newComparisonValue;
                    if (newComparisonPointer == 0)
                        newComparisonValue = "<font color='#aaa'>nullptr</font>";
                    else if (valueComparisonMemoryOffset == 0)
                        newComparisonValue = "<font color='#aaa'>bad ptr</font>";
                    else
                        newComparisonValue = QString::asprintf("<font color='green'><u>0x%016llX</u></font>", newComparisonPointer);

                    itemComparisonValue->setData(newComparisonValue, Qt::DisplayRole);
                    itemComparisonValueHex->setData(newComparisonValue, Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::DataPointer:
        {
            if (pointerUpdate)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);

            if (comparisonActive)
            {
                if (comparisonPointerUpdate)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::Byte:
        {
            std::optional<int8_t> value;
            value = updateField<int8_t>(itemField, valueMemoryOffset, itemValue, "%d", itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int8_t> comparisonValue;
                comparisonValue = updateField<int8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            std::optional<uint8_t> value;
            value = updateField<uint8_t>(itemField, valueMemoryOffset, itemValue, "%u", itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint8_t> comparisonValue;
                comparisonValue = updateField<uint8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Word:
        {
            std::optional<int16_t> value;
            value = updateField<int16_t>(itemField, valueMemoryOffset, itemValue, "%d", itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int16_t> comparisonValue;
                comparisonValue = updateField<int16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, "%u", itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue = updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%d", itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Dword:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, "%ld", itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%ld", itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedDword:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, "%u", itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue = updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%u", itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Qword:
        {
            std::optional<int64_t> value;
            value = updateField<int64_t>(itemField, valueMemoryOffset, itemValue, "%lld", itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<int64_t> comparisonValue;
                comparisonValue =
                    updateField<int64_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%lld", itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            std::optional<uint64_t> value;
            value = updateField<uint64_t>(itemField, valueMemoryOffset, itemValue, "%llu", itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<uint64_t> comparisonValue;
                comparisonValue =
                    updateField<uint64_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%llu", itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Float:
        {
            std::optional<float> value;
            value = updateField<float>(itemField, valueMemoryOffset, itemValue, "%f", itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<float> comparisonValue;
                comparisonValue = updateField<float>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%f", itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Double:
        {
            std::optional<double> value;
            value = updateField<double>(itemField, valueMemoryOffset, itemValue, "%lf", itemValueHex, isPointer, "0x%016llX", true, !pointerUpdate, highlightColor);

            if (comparisonActive)
            {
                std::optional<double> comparisonValue;
                comparisonValue = updateField<double>(itemField, valueComparisonMemoryOffset, itemComparisonValue, "%lf", itemComparisonValueHex, isPointer, "0x%016llX", false, false, highlightColor);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Bool:
        {
            std::optional<bool> value;
            value = updateField<bool>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(value.value() ? "<font color='green'>True</font>" : "<font color='red'>False</font>", Qt::DisplayRole); // maybe color them green/red as well?

            if (comparisonActive)
            {
                std::optional<bool> comparisonValue;
                comparisonValue = updateField<bool>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(comparisonValue.value() ? "True" : "False", Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::Flags32:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 32).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 32).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, std::nullopt, std::nullopt, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flags16:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 16).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue =
                    updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 16).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, 0, 0, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flags8:
        {
            std::optional<uint8_t> value;
            value = updateField<uint8_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString::fromStdString(flagsString(value.value(), 8).str()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint8_t> comparisonValue;
                comparisonValue = updateField<uint8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString::fromStdString(flagsString(comparisonValue.value(), 8).str()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, std::nullopt, std::nullopt, itemField, disableChangeHighlighting);
                }
            }
            break;
        }
        case MemoryFieldType::Flag: // can't be pointer, always have parent
        {
            constexpr auto getDataFrom = [](const QModelIndex& idx, int col, int role)
            {
                auto mod = idx.model();
                auto parentIndex = idx.parent();
                return mod->data(mod->index(idx.row(), col, parentIndex), role);
            };

            auto flagIndex = itemField->data(gsRoleFlagIndex).toUInt();
            uint mask = (1 << (flagIndex - 1));
            auto flagRef = qvariant_cast<std::string>(itemField->parent()->data(gsRoleRefName));
            auto flagName = Configuration::get()->flagTitle(flagRef, flagIndex);

            auto value = getDataFrom(itemField->parent()->index(), gsColValue, gsRoleRawValue).toUInt();
            auto flagSet = ((value & mask) == mask);
            auto flagTitle = QString::fromStdString(flagName.empty() ? Configuration::get()->flagTitle("unknown", flagIndex) : flagName); // TODO: don't show empty unless it was chosen in settings
            // TODO: would love to instead get the names and save them in addMemoryField and then just use itemValue->setForeground or itemValue->setData(Qt::TextColorRole) for the color
            // but it doesn't work with HTML delagate, and i don't know how to edit it to make it work
            auto caption = QString("<font color='%1'>%2</font>").arg(flagSet ? "green" : "red", flagTitle);
            itemValue->setData(caption, Qt::DisplayRole);

            auto comparisonValue = getDataFrom(itemField->parent()->index(), gsColComparisonValue, gsRoleRawValue).toUInt();
            auto comparisonFlagSet = ((comparisonValue & mask) == mask);
            auto comparisonTitle = QString("<font color='%1'>%2</font>").arg(comparisonFlagSet ? "green" : "red", flagTitle);
            itemComparisonValue->setData(comparisonTitle, Qt::DisplayRole);

            itemComparisonValue->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::State8:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int8_t> value;
            value = updateField<int8_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%02X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int8_t> comparisonValue;
                comparisonValue = updateField<int8_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%02X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::State16:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int16_t> value;
            value = updateField<int16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int16_t> comparisonValue;
                comparisonValue = updateField<int16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::State32:
        {
            std::string stateRef = itemField->data(gsRoleRefName).value<std::string>();
            auto config = Configuration::get();

            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto stateTitle = QString::fromStdString(std::to_string(value.value()) + ": " + config->stateTitle(stateRef, value.value()));
                itemValue->setData(stateTitle, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto stateTitle = QString::fromStdString(std::to_string(comparisonValue.value()) + ": " + config->stateTitle(stateRef, comparisonValue.value()));
                    itemValue->setData(stateTitle, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UTF16Char:
        {
            std::optional<uint16_t> value;
            value = updateField<uint16_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%04X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString("'<b>%1</b>' (%2)").arg(QChar(value.value())).arg(value.value()), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint16_t> comparisonValue;
                comparisonValue =
                    updateField<uint16_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%04X", false, false, highlightColor);
                if (comparisonValue.has_value())
                    itemComparisonValue->setData(QString("'<b>%1</b>' (%2)").arg(QChar(comparisonValue.value())).arg(comparisonValue.value()), Qt::DisplayRole);

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        {
            // size in bytes
            auto size = itemField->data(gsRoleSize).toULongLong();
            char buffer[1024] = {0};
            if (valueMemoryOffset == 0)
            {
                itemValue->setData("", Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData("", Qt::DisplayRole);
            }
            else
            {
                Script::Memory::Read(valueMemoryOffset, buffer, size, nullptr);
                auto buffer_w = reinterpret_cast<const ushort*>(buffer);
                auto valueString = QString::fromUtf16(buffer_w);

                QString valueOld = itemValue->data(Qt::DisplayRole).toString(); // no need for gsRoleRawValue
                if (valueString != valueOld)
                {
                    itemField->setBackground(highlightColor);
                    itemValue->setData(valueString, Qt::DisplayRole);
                    if (!isPointer)
                    {
                        std::stringstream ss;
                        ss << "0x" << std::hex << std::setfill('0');
                        for (int i = 0; i < std::min(size / 2, 10ull) && buffer_w[i] != 0; ++i)
                            ss << std::setw(4) << static_cast<uint16_t>(buffer_w[i]);

                        itemValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                    }
                }
                else if (!pointerUpdate)
                    itemField->setBackground(Qt::transparent);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData("", Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData("", Qt::DisplayRole);
                }
                else
                {
                    Script::Memory::Read(valueComparisonMemoryOffset, buffer, size, nullptr);
                    auto buffer_w = reinterpret_cast<const ushort*>(buffer);
                    auto valueString = QString::fromUtf16(buffer_w);

                    QString valueOld = itemComparisonValue->data(Qt::DisplayRole).toString(); // no need for gsRoleRawValue
                    if (valueString != valueOld)
                    {
                        itemField->setBackground(highlightColor);
                        itemComparisonValue->setData(valueString, Qt::DisplayRole);
                        if (!isPointer)
                        {
                            std::stringstream ss;
                            ss << "0x" << std::hex << std::setfill('0');
                            for (int i = 0; i < std::min(size / 2, 10ull) && buffer_w[i] != 0; ++i)
                                ss << std::setw(4) << static_cast<uint16_t>(buffer_w[i]);

                            itemComparisonValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                        }
                    }
                    else if (!pointerUpdate)
                        itemField->setBackground(Qt::transparent);
                }
            }
            break;
        }
        case MemoryFieldType::UTF8StringFixedSize:
        {
            auto size = itemField->data(gsRoleSize).toULongLong();
            char buffer[1024] = {0};
            if (valueMemoryOffset == 0)
            {
                itemValue->setData("", Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData("", Qt::DisplayRole);
            }
            else
            {
                Script::Memory::Read(valueMemoryOffset, buffer, size, nullptr);
                auto valueString = QString::fromUtf8(buffer);

                QString valueOld = itemValue->data(Qt::DisplayRole).toString(); // no need for gsRoleRawValue
                if (valueString != valueOld)
                {
                    itemField->setBackground(highlightColor);
                    itemValue->setData(valueString, Qt::DisplayRole);
                    if (!isPointer)
                    {
                        std::stringstream ss;
                        ss << "0x" << std::hex << std::setfill('0');
                        for (int i = 0; i < std::min(size, 10ull) && buffer[i] != 0; ++i)
                            ss << std::setw(2) << static_cast<uint8_t>(buffer[i]);

                        itemValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                    }
                }
                else if (!pointerUpdate)
                    itemField->setBackground(Qt::transparent);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData("", Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData("", Qt::DisplayRole);
                }
                else
                {
                    Script::Memory::Read(valueComparisonMemoryOffset, buffer, size, nullptr);
                    auto valueString = QString::fromUtf8(buffer);

                    QString valueOld = itemComparisonValue->data(Qt::DisplayRole).toString(); // no need for gsRoleRawValue
                    if (valueString != valueOld)
                    {
                        itemField->setBackground(highlightColor);
                        itemComparisonValue->setData(valueString, Qt::DisplayRole);
                        if (!isPointer)
                        {
                            std::stringstream ss;
                            ss << "0x" << std::hex << std::setfill('0');
                            for (int i = 0; i < std::min(size, 10ull) && buffer[i] != 0; ++i)
                                ss << std::setw(2) << static_cast<uint8_t>(buffer[i]);

                            itemComparisonValueHex->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
                        }
                    }
                    else if (!pointerUpdate)
                        itemField->setBackground(Qt::transparent);
                }
            }
            break;
        }
        case MemoryFieldType::EntityDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto entityName = Configuration::get()->entityList().nameForID(value.value());
                if (entityName._Starts_with("UNKNOWN ID: "))
                    itemValue->setData(QString::asprintf("%u (%s)", value, entityName.c_str()), Qt::DisplayRole);
                else
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", value, entityName.c_str()), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto entityName = Configuration::get()->entityList().nameForID(comparisonValue.value());
                    if (entityName._Starts_with("UNKNOWN ID: "))
                        itemComparisonValue->setData(QString::asprintf("%u (%s)", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                    else
                        itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::TextureDBID:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                if (value.value() < 0)
                {
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%d (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", value.value()), Qt::DisplayRole);
                }
                else
                {
                    itemValue->setData(QString::asprintf("<font color='blue'><u>%d (%s)</u></font>", value.value(), Spelunky2::get()->get_TextureDB().nameForID(value.value()).c_str()),
                                       Qt::DisplayRole);
                }
            }
            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    if (comparisonValue.value() < 0)
                    {
                        itemComparisonValue->setData(
                            QString::asprintf("<font color='blue'><u>%d (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", comparisonValue.value()), Qt::DisplayRole);
                    }
                    else
                    {
                        itemComparisonValue->setData(
                            QString::asprintf("<font color='blue'><u>%d (%s)</u></font>", comparisonValue.value(), Spelunky2::get()->get_TextureDB().nameForID(comparisonValue.value()).c_str()),
                            Qt::DisplayRole);
                    }
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::StringsTableID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
                itemValue->setData(QString("%1: %2").arg(value.value()).arg(Spelunky2::get()->get_StringsTable().stringForIndex(value.value())), Qt::DisplayRole);

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    itemComparisonValue->setData(QString("%1: %2").arg(comparisonValue.value()).arg(Spelunky2::get()->get_StringsTable().stringForIndex(comparisonValue.value())), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::ParticleDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                std::string particleName = Configuration::get()->particleEmittersList().nameForID(value.value());
                itemValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", value.value(), particleName.c_str()), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    std::string particleName = Configuration::get()->particleEmittersList().nameForID(comparisonValue.value());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%u (%s)</u></font>", comparisonValue.value(), particleName.c_str()), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::EntityUID:
        {
            std::optional<int32_t> value;
            value = updateField<int32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                if (value.value() < 0)
                {
                    itemValue->setData(QString::asprintf("(%d) Nothing", value.value()), Qt::DisplayRole);
                    itemValue->setData(0, gsRoleEntityOffset);
                }
                else
                {
                    uintptr_t entityOffset = S2Plugin::State{Spelunky2::get()->get_StatePtr()}.findEntitybyUID(value.value());
                    if (entityOffset != 0)
                    {
                        auto entityName = Configuration::get()->getEntityName(Entity{entityOffset}.entityTypeID());
                        itemValue->setData(QString::asprintf("<font color='blue'><u>UID %u (%s)</u></font>", value.value(), entityName.c_str()), Qt::DisplayRole);
                        itemValue->setData(entityOffset, gsRoleEntityOffset);
                    }
                    else
                    {
                        itemValue->setData(QString::asprintf("(%d) UNKNOWN ENTITY", value.value()), Qt::DisplayRole);
                        itemValue->setData(0, gsRoleEntityOffset);
                    }
                }
            }

            if (comparisonActive)
            {
                std::optional<int32_t> comparisonValue;
                comparisonValue = updateField<int32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    if (comparisonValue.value() < 0)
                    {
                        itemComparisonValue->setData(QString::asprintf("(%d) Nothing", comparisonValue.value()), Qt::DisplayRole);
                        itemComparisonValue->setData(0, gsRoleEntityOffset);
                    }
                    else
                    {
                        uintptr_t comparisonEntityOffset = S2Plugin::State{Spelunky2::get()->get_StatePtr()}.findEntitybyUID(comparisonValue.value());
                        if (comparisonEntityOffset != 0)
                        {
                            auto entityName = Configuration::get()->getEntityName(Entity{comparisonEntityOffset}.entityTypeID());
                            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>UID %u (%s)</u></font>", comparisonValue.value(), entityName.c_str()), Qt::DisplayRole);
                            itemComparisonValue->setData(comparisonEntityOffset, gsRoleEntityOffset);
                        }
                        else
                        {
                            itemComparisonValue->setData(QString::asprintf("(%d) UNKNOWN ENTITY", comparisonValue.value()), Qt::DisplayRole);
                            itemComparisonValue->setData(0, gsRoleEntityOffset);
                        }
                    }
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::EntityUIDPointer:
        {
            // TODO pending deletion
            break;
        }
        case MemoryFieldType::EntityPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                // TODO: unknown entity?
                auto entityName = Configuration::get()->getEntityName(Entity{valueMemoryOffset}.entityTypeID());
                itemValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", entityName.c_str()), Qt::DisplayRole);
                itemValue->setData(valueMemoryOffset, gsRoleRawValue); // set to 0/clear when unknown entity?
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonEntityName = Configuration::get()->getEntityName(Entity{valueComparisonMemoryOffset}.entityTypeID());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", comparisonEntityName.c_str()), Qt::DisplayRole);
                    itemComparisonValue->setData(valueComparisonMemoryOffset, gsRoleRawValue); // set to 0/clear when unknown entity?
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::EntityDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadDword(valueMemoryOffset + 20); // TODO hex offset
                auto entityName = Configuration::get()->entityList().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", id, entityName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadDword(valueComparisonMemoryOffset + 20);
                    auto comparisonEntityName = Configuration::get()->entityList().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", comparisonID, comparisonEntityName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::TextureDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadQword(valueMemoryOffset);
                auto& textureName = Spelunky2::get()->get_TextureDB().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", id, textureName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadQword(valueComparisonMemoryOffset);
                    auto& comparisonTextureName = Spelunky2::get()->get_TextureDB().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", comparisonID, comparisonTextureName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::LevelGenPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show level gen</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show level gen</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::ParticleDBPointer:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
            {
                auto id = Script::Memory::ReadDword(valueMemoryOffset);
                auto particleName = Configuration::get()->particleEmittersList().nameForID(id);
                itemValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", id, particleName.c_str()), Qt::DisplayRole);
            }
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                {
                    auto comparisonID = Script::Memory::ReadQword(valueComparisonMemoryOffset);
                    auto comparisonParticleName = Configuration::get()->particleEmittersList().nameForID(comparisonID);
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", comparisonID, comparisonParticleName.c_str()), Qt::DisplayRole);
                }
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            if (valueMemoryOffset == 0) // nullptr or bad ptr
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show functions</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole));
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show functions</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::CharacterDBID:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                auto& characterDB = Spelunky2::get()->get_CharacterDB();
                bool isValidCharacter = value.value() < characterDB.charactersCount();
                auto& characterName = isValidCharacter ? Spelunky2::get()->get_CharacterDB().characterNamesStringList().at(value.value()) : "";
                itemValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(value.value()).arg(characterName), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);
                if (comparisonValue.has_value())
                {
                    auto& characterDB = Spelunky2::get()->get_CharacterDB();
                    bool isValidCharacter = comparisonValue.value() < characterDB.charactersCount();
                    auto& characterName = isValidCharacter ? Spelunky2::get()->get_CharacterDB().characterNamesStringList().at(comparisonValue.value()) : "";
                    itemComparisonValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(comparisonValue.value()).arg(characterName), Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::ConstCharPointerPointer:
        {
            // TODO: probably delete? it's actually a struct not just a pointer?
            constexpr uint16_t bufferSize = 1024;
            char buffer[bufferSize] = {0};

            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            if (value != 0)
            {
                size_t chararray = Script::Memory::ReadQword(value);
                char c = 0;
                uint16_t counter = 0;
                do
                {
                    c = Script::Memory::ReadByte(chararray + counter);
                    buffer[counter++] = c;
                } while (c != 0 && counter < bufferSize);
            }

            itemValue->setData(QString(buffer), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            char comparisonBuffer[bufferSize] = {0};
            size_t comparisonValue = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            if (comparisonValue != 0)
            {
                size_t chararray = Script::Memory::ReadQword(comparisonValue);
                char c = 0;
                uint16_t counter = 0;
                do
                {
                    c = Script::Memory::ReadByte(chararray + counter);
                    comparisonBuffer[counter++] = c;
                } while (c != 0 && counter < bufferSize);
            }
            itemComparisonValue->setData(QString(comparisonBuffer), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            break;
        }
        case MemoryFieldType::ConstCharPointer:
        {
            if (valueMemoryOffset == 0)
            {
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            }
            else
            {
                std::string str = ReadConstString(valueMemoryOffset);
                itemValue->setData(QString::fromStdString(str), Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                }
                else
                {
                    std::string comparisonStr = ReadConstString(valueComparisonMemoryOffset);
                    itemComparisonValue->setData(QString::fromStdString(comparisonStr), Qt::DisplayRole);
                }
                // pointer compare
                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::StdString:
        {
            std::optional<std::string> stringValue;
            std::optional<std::string> comparisonStringValue;
            if (valueMemoryOffset == 0)
            {
                itemValue->setData("", Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData("", Qt::DisplayRole);
            }
            else
            {
                StdString string{valueMemoryOffset};
                // i don't think we will have pointer to std::string, but note just in case: this would override the pointer value in hex
                auto ptr = string.string_ptr();
                itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                itemValueHex->setData(ptr, gsRoleRawValue);

                stringValue = string.get_string();
                auto displayValue = QString::fromStdString(stringValue.value());
                itemField->setBackground(itemValue->data(Qt::DisplayRole).toString() == displayValue ? Qt::transparent : highlightColor);
                itemValue->setData(displayValue, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData("", Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData("", Qt::DisplayRole);
                }
                else
                {
                    StdString comparisonString{valueComparisonMemoryOffset};
                    comparisonStringValue = comparisonString.get_string();
                    itemComparisonValue->setData(QString::fromStdString(comparisonStringValue.value()), Qt::DisplayRole);

                    auto ptr = comparisonString.string_ptr();
                    itemComparisonValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                    itemComparisonValueHex->setData(ptr, gsRoleRawValue);
                }
                bool compare = stringValue != comparisonStringValue;
                itemComparisonValue->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
                // for the hex should probably compare the actual value displayed, but those will never be the same unless it's the exact same string object
                // if(!isPointer)
                itemComparisonValueHex->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::StdWstring:
        {
            std::optional<std::u16string> stringValue;
            std::optional<std::u16string> comparisonStringValue;
            if (valueMemoryOffset == 0)
            {
                itemValue->setData("", Qt::DisplayRole);
                if (!isPointer)
                    itemValueHex->setData("", Qt::DisplayRole);
            }
            else
            {
                StdString<char16_t> string{valueMemoryOffset};
                // i don't think we will have pointer to std::string, but note just in case: this would override the pointer value in hex
                auto ptr = string.string_ptr();
                itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                itemValueHex->setData(ptr, gsRoleRawValue);

                stringValue = string.get_string();
                // auto displayValue = QString::fromStdU16String(stringValue.value());
                // itemField->setBackground(itemValue->data(Qt::DisplayRole).toString() == displayValue ? Qt::transparent : highlightColor);
                // itemValue->setData(displayValue, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                {
                    itemComparisonValue->setData("", Qt::DisplayRole);
                    if (!isPointer)
                        itemComparisonValueHex->setData("", Qt::DisplayRole);
                }
                else
                {
                    StdString<char16_t> comparisonString{valueComparisonMemoryOffset};
                    comparisonStringValue = comparisonString.get_string();
                    // itemComparisonValue->setData(QString::fromStdU16String(comparisonStringValue.value()), Qt::DisplayRole);

                    auto ptr = comparisonString.string_ptr();
                    itemComparisonValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", ptr), Qt::DisplayRole);
                    itemComparisonValueHex->setData(ptr, gsRoleRawValue);
                }
                bool compare = stringValue != comparisonStringValue;
                itemComparisonValue->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
                // for the hex should probably compare the actual value displayed, but those will never be the same unless it's the exact same string object
                itemComparisonValueHex->setBackground(compare ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::ThemeInfoName:
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData("n/a", Qt::DisplayRole);
            else
                itemValue->setData(QString::fromStdString(Spelunky2::get()->themeNameOfOffset(valueMemoryOffset)), Qt::DisplayRole);

            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData("n/a", Qt::DisplayRole);
                else
                    itemComparisonValue->setData(QString::fromStdString(Spelunky2::get()->themeNameOfOffset(valueMemoryOffset)), Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }

            if (shouldUpdateChildren && fieldType == MemoryFieldType::UndeterminedThemeInfoPointer)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? std::optional<uintptr_t>(valueMemoryOffset) : std::nullopt;
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? std::optional<uintptr_t>(valueComparisonMemoryOffset) : std::nullopt;
                    updateRow(x, valueMemoryOffset, valueComparisonMemoryOffset, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::LevelGenRoomsPointer:
        case MemoryFieldType::LevelGenRoomsMetaPointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show rooms</u></font>", Qt::DisplayRole);

            if (comparisonActive)
            {
                // we can't show comparison version since it's a tab, not a new window
                // TODO: maybe add comparison in the show rooms tab?
                itemComparisonValue->setData("", Qt::DisplayRole);
                itemComparisonValue->setData(0, gsRoleMemoryOffset);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::JournalPagePointer:
        {
            if (valueMemoryOffset == 0)
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
            else
                itemValue->setData("<font color='blue'><u>Show journal page</u></font>", Qt::DisplayRole);

            // just for completness, there probably won't be comparison with this
            if (comparisonActive)
            {
                if (valueComparisonMemoryOffset == 0)
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                else
                    itemComparisonValue->setData("<font color='blue'><u>Show journal page</u></font>", Qt::DisplayRole);

                itemComparisonValue->setBackground(itemComparisonValueHex->background());
            }
            break;
        }
        case MemoryFieldType::IPv4Address:
        {
            std::optional<uint32_t> value;
            value = updateField<uint32_t>(itemField, valueMemoryOffset, itemValue, nullptr, itemValueHex, isPointer, "0x%08X", true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                uint32_t ipaddr = value.value();
                QString ipaddrString = QString("%1.%2.%3.%4")
                                           .arg((unsigned char)(ipaddr & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 8 & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 16 & 0xFF))
                                           .arg((unsigned char)(ipaddr >> 24 & 0xFF));

                itemValue->setData(ipaddrString, Qt::DisplayRole);
            }

            if (comparisonActive)
            {
                std::optional<uint32_t> comparisonValue;
                comparisonValue =
                    updateField<uint32_t>(itemField, valueComparisonMemoryOffset, itemComparisonValue, nullptr, itemComparisonValueHex, isPointer, "0x%08X", false, false, highlightColor);

                if (comparisonValue.has_value())
                {
                    uint32_t ipaddr = comparisonValue.value();
                    QString ipaddrString = QString("%1.%2.%3.%4")
                                               .arg((unsigned char)(ipaddr & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 8 & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 16 & 0xFF))
                                               .arg((unsigned char)(ipaddr >> 24 & 0xFF));

                    itemComparisonValue->setData(ipaddrString, Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            break;
        }
        case MemoryFieldType::StdVector:
        {
            std::optional<uintptr_t> value;
            // we use the end pointer to check if it was changed
            value = updateField<uintptr_t>(itemField, valueMemoryOffset == 0 ? 0 : valueMemoryOffset + 0x8, itemValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                // TODO maybe show hex as the begin pointer ?
            }

            if (comparisonActive)
            {
                std::optional<uintptr_t> comparisonValue;
                auto addr = valueComparisonMemoryOffset == 0 ? 0 : valueComparisonMemoryOffset + 0x8;
                value = updateField<uintptr_t>(itemField, addr, itemComparisonValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
                if (value.has_value())
                {
                    itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                }

                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::StdMap:
        {
            std::optional<size_t> value;
            // we use the size to check if it was changed
            value = updateField<size_t>(itemField, valueMemoryOffset == 0 ? 0 : valueMemoryOffset + 0x8, itemValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
            if (value.has_value())
            {
                itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                // TODO maybe show hex as the pointer ?
            }

            if (comparisonActive)
            {
                std::optional<size_t> comparisonValue;
                auto addr = valueComparisonMemoryOffset == 0 ? 0 : valueComparisonMemoryOffset + 0x8;
                value = updateField<size_t>(itemField, addr, itemComparisonValue, nullptr, nullptr, true, nullptr, true, !pointerUpdate, highlightColor);
                if (value.has_value())
                {
                    itemValue->setData("<font color='blue'><u>Show contents</u></font>", Qt::DisplayRole);
                }
                // TODO maybe it should be based on the pointer not size?
                itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
                if (isPointer == false)
                    itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Skip:
        {
            // TODO
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            // can't be a pointer, nothing to do here
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    updateRow(x, newAddr, newAddrComparison, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::DefaultStructType:
        {
            // TODO add setting "open in new vindow"
            if (isPointer)
            {
                itemValue->setData(itemValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                if (comparisonActive)
                {
                    itemComparisonValue->setData(itemComparisonValueHex->data(Qt::DisplayRole), Qt::DisplayRole);
                    itemComparisonValue->setBackground(itemComparisonValueHex->background());
                }
            }
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Dummy:
        {
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                    updateRow(x, newAddr, newAddrComparison, itemField);
            }
            break;
        }
        default:
        {
            // just for debug
            dprintf("WARNING: type %d not handled in TreeViewMemoryFields::updateRow ('%s' row: %d)\n", fieldType, itemField->data(gsRoleUID).toString().toStdString().c_str(), row);
            if (shouldUpdateChildren)
            {
                for (uint8_t x = 0; x < itemField->rowCount(); ++x)
                {
                    std::optional<uintptr_t> addr = pointerUpdate ? valueMemoryOffset : (isPointer ? std::nullopt : newAddr);
                    std::optional<uintptr_t> comparisonAddr = comparisonPointerUpdate ? valueComparisonMemoryOffset : (isPointer ? std::nullopt : newAddrComparison);
                    updateRow(x, addr, comparisonAddr, itemField);
                }
            }
            break;
        }
    }
}

void S2Plugin::TreeViewMemoryFields::cellClicked(const QModelIndex& index)
{
    auto clickedItem = mModel->itemFromIndex(index);
    constexpr auto getDataFrom = [](const QModelIndex& idx, int col, int role)
    {
        auto mod = idx.model();
        auto parentIndex = idx.parent();
        return mod->data(mod->index(idx.row(), col, parentIndex), role);
    };

    switch (index.column())
    {
        case gsColMemoryOffset:
        {
            GuiDumpAt(clickedItem->data(gsRoleRawValue).toULongLong());
            GuiShowCpu();
            break;
        }
        case gsColValueHex:
        case gsColComparisonValueHex:
        {
            // only pointers have gsRoleRawValue in Hex field, no value will result in 0
            auto addr = clickedItem->data(gsRoleRawValue).toULongLong();
            if (Script::Memory::IsValidPtr(addr))
            {
                // exception since there is no point in showing code address in memory dump
                if (getDataFrom(index, gsColField, gsRoleType).value<MemoryFieldType>() == MemoryFieldType::CodePointer)
                {
                    GuiDisasmAt(addr, GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
                GuiDumpAt(addr);
                GuiShowCpu();
            }
            break;
        }
        case gsColValue:
        case gsColComparisonValue:
        {
            auto dataType = getDataFrom(index, gsColField, gsRoleType).value<MemoryFieldType>();
            switch (dataType)
            {
                case MemoryFieldType::CodePointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    GuiDisasmAt(rawValue, GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
                case MemoryFieldType::DataPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        GuiDumpAt(rawValue);
                        GuiShowCpu();
                    }
                    break;
                }
                case MemoryFieldType::EntityPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleRawValue).toULongLong(); // TODO check if valid ptr here or in update
                    if (rawValue != 0)
                    {
                        mToolbar->showEntity(rawValue);
                    }
                    break;
                }
                case MemoryFieldType::EntityUID:
                case MemoryFieldType::EntityUIDPointer:
                {
                    auto offset = clickedItem->data(gsRoleEntityOffset).toULongLong();
                    if (offset != 0)
                    {
                        mToolbar->showEntity(offset);
                    }
                    break;
                }
                case MemoryFieldType::EntityDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id != 0)
                    {
                        auto view = mToolbar->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showID(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::CharacterDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull())
                    {
                        auto view = mToolbar->showCharacterDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull())
                    {
                        auto view = mToolbar->showTextureDB();
                        if (view != nullptr)
                        {
                            view->showID(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue);
                    if (!id.isNull() && id.toUInt() != -1)
                    {
                        auto view = mToolbar->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showID(id.toUInt());
                        }
                    }
                    break;
                }
                case MemoryFieldType::EntityDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        auto id = Script::Memory::ReadDword(rawValue + 20);
                        auto view = mToolbar->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showID(id); // TODO: use pointer, not ID
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        auto id = Script::Memory::ReadQword(rawValue);
                        auto view = mToolbar->showTextureDB();
                        if (view != nullptr)
                        {
                            view->showID(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::State8:
                case MemoryFieldType::State16:
                case MemoryFieldType::State32:
                {
                    auto addr = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (addr != 0)
                    {
                        auto name = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto refName = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleRefName));
                        auto dialog = new DialogEditState(name, refName, addr, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        mToolbar->showLevelGen(); // TODO: use pointer
                    }
                    break;
                }
                case MemoryFieldType::StdVector:
                {
                    auto addr = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    auto fieldType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerFirstParameterType));
                    if (addr != 0)
                    {
                        mToolbar->showStdVector(addr, fieldType);
                    }
                    break;
                }
                case MemoryFieldType::StdMap:
                {
                    auto addr = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    auto fieldkeyType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerFirstParameterType));
                    auto fieldvalueType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleStdContainerSecondParameterType));
                    if (addr != 0)
                    {
                        mToolbar->showStdMap(addr, fieldkeyType, fieldvalueType);
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        auto id = Script::Memory::ReadDword(rawValue); // TODO: use pointer
                        auto view = mToolbar->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showID(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::VirtualFunctionTable:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        // TODO: maybe use the "entity interpret as" for vtable? (it's doable)
                        auto vftType = qvariant_cast<std::string>(getDataFrom(index, gsColField, gsRoleRefName));
                        if (vftType == "Entity") // in case of Entity, we have to see what the entity is interpreted as, and show those functions
                        {
                            // rare case, we need the address not the pointer value to get entity
                            auto ent = Entity{getDataFrom(index, gsColMemoryOffset, gsRoleRawValue).toULongLong()};
                            mToolbar->showVirtualFunctions(rawValue, ent.entityClassName());
                        }
                        else
                        {
                            mToolbar->showVirtualFunctions(rawValue, vftType);
                        }
                    }
                    break;
                }
                case MemoryFieldType::Bool:
                {
                    auto offset = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (offset != 0)
                    {
                        auto currentValue = clickedItem->data(gsRoleRawValue).toBool();
                        Script::Memory::WriteByte(offset, !currentValue);
                    }
                    break;
                }
                case MemoryFieldType::Flag:
                {
                    auto flagIndex = getDataFrom(index, gsColField, gsRoleFlagIndex).toUInt();
                    auto offset = clickedItem->parent()->data(gsRoleMemoryOffset).toULongLong();
                    if (offset != 0)
                    {
                        auto currentValue = Script::Memory::ReadDword(offset);
                        Script::Memory::WriteDword(offset, currentValue ^ (1U << (flagIndex - 1)));
                    }
                    break;
                }
                case MemoryFieldType::Byte:
                case MemoryFieldType::UnsignedByte:
                case MemoryFieldType::Word:
                case MemoryFieldType::UnsignedWord:
                case MemoryFieldType::Dword:
                case MemoryFieldType::UnsignedDword:
                case MemoryFieldType::Qword:
                case MemoryFieldType::UnsignedQword:
                case MemoryFieldType::Float:
                case MemoryFieldType::Double:
                case MemoryFieldType::UTF16Char:
                case MemoryFieldType::StringsTableID:
                {
                    auto offset = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = getDataFrom(index, gsColField, gsRoleUID).toString();
                        auto dialog = new DialogEditSimpleValue(fieldName, offset, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenRoomsPointer:
                case MemoryFieldType::LevelGenRoomsMetaPointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        emit levelGenRoomsPointerClicked();
                    }
                    break;
                }
                case MemoryFieldType::JournalPagePointer:
                {
                    auto rawValue = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (rawValue != 0)
                    {
                        mToolbar->showJournalPage(rawValue, "JournalPage");
                    }
                    break;
                }
                case MemoryFieldType::DefaultStructType:
                {
                    bool isPointer = getDataFrom(index, gsColField, gsRoleIsPointer).toBool();
                    if (isPointer)
                    {
                        auto addr = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                        if (addr != 0)
                        {
                            GuiDumpAt(addr);
                            GuiShowCpu();
                        }
                    }
                }
            }
            emit memoryFieldValueUpdated(getDataFrom(index, gsColField, gsRoleUID).toString());
        }
    }
}

void S2Plugin::TreeViewMemoryFields::clear()
{
    mSavedColumnWidths[gsColField] = columnWidth(gsColField);
    mSavedColumnWidths[gsColValue] = columnWidth(gsColValue);
    mSavedColumnWidths[gsColValueHex] = columnWidth(gsColValueHex);
    mSavedColumnWidths[gsColMemoryOffset] = columnWidth(gsColMemoryOffset);
    mSavedColumnWidths[gsColMemoryOffsetDelta] = columnWidth(gsColMemoryOffsetDelta);
    mSavedColumnWidths[gsColType] = columnWidth(gsColType);
    mSavedColumnWidths[gsColComment] = columnWidth(gsColComment);
    mModel->clear();
}

void S2Plugin::TreeViewMemoryFields::expandItem(QStandardItem* item)
{
    expand(mModel->indexFromItem(item));
}

void S2Plugin::TreeViewMemoryFields::setEnableChangeHighlighting(bool b) noexcept
{
    mEnableChangeHighlighting = b;
}

void S2Plugin::TreeViewMemoryFields::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("spelunky/entityoffset"))
    {
        event->acceptProposedAction();
    }
}

void S2Plugin::TreeViewMemoryFields::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("spelunky/entityoffset"))
    {
        event->accept();
    }
}

void S2Plugin::TreeViewMemoryFields::dropEvent(QDropEvent* event)
{
    auto dropData = event->mimeData()->data("spelunky/entityoffset");
    emit entityOffsetDropped(dropData.toULongLong());
    event->acceptProposedAction();
}

void S2Plugin::TreeViewMemoryFields::startDrag(Qt::DropActions supportedActions)
{
    auto ix = selectedIndexes();
    if (ix.count() == 0)
    {
        return;
    }
    constexpr auto getDataFrom = [](const QModelIndex& idx, int col, int role)
    {
        auto mod = idx.model();
        auto parentIndex = idx.parent();
        return mod->data(mod->index(idx.row(), col, parentIndex), role);
    };

    QDrag* drag = new QDrag(this);
    auto mimeData = new QMimeData();

    auto& index = ix.at(0);

    // for spelunky/entityoffset: dragging an entity from ViewEntities on top of ViewEntity for comparison
    auto entityItem = mModel->item(index.row(), gsColMemoryOffset); // TODO: maybe not needed? try to use spelunky/memoryfield data, also allow only entity pointer?
    if (entityItem != nullptr)
    {
        auto entityData = entityItem->data(gsRoleRawValue);
        if (entityData.isValid())
        {
            mimeData->setData("spelunky/entityoffset", QByteArray().setNum(Script::Memory::ReadQword(entityData.toULongLong())));
        }
    }

    // for spelunky/memoryfield: dragging any memoryfield onto ViewLogger

    nlohmann::json o;
    o[gsJSONDragDropMemoryField_UID] = getDataFrom(index, gsColField, gsRoleUID).toString().toStdString();
    o[gsJSONDragDropMemoryField_Offset] = getDataFrom(index, gsColField, gsRoleMemoryOffset).toULongLong();
    o[gsJSONDragDropMemoryField_Type] = getDataFrom(index, gsColField, gsRoleType).value<MemoryFieldType>();
    auto json = QString::fromStdString(o.dump());

    auto codec = QTextCodec::codecForName("UTF-8");
    mimeData->setData("spelunky/memoryfield", codec->fromUnicode(json));

    drag->setMimeData(mimeData);
    drag->exec();
}

void S2Plugin::TreeViewMemoryFields::labelAll()
{
    // TODO
    // DbgSetAutoLabelAt(offset, (entityName + "." + fieldName).c_str());
}
