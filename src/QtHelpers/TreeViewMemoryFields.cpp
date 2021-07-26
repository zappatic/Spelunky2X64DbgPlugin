#include "QtHelpers/TreeViewMemoryFields.h"
#include "Data/Entity.h"
#include "QtHelpers/DialogEditSimpleValue.h"
#include "QtHelpers/DialogEditState.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewTextureDB.h"
#include "pluginmain.h"
#include <QDrag>
#include <QMimeData>
#include <QTextCodec>
#include <inttypes.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

S2Plugin::TreeViewMemoryFields::TreeViewMemoryFields(ViewToolbar* toolbar, MemoryMappedData* mmd, QWidget* parent) : QTreeView(parent), mToolbar(toolbar), mMemoryMappedData(mmd)
{
    mHTMLDelegate = std::make_unique<StyledItemDelegateHTML>();
    setItemDelegate(mHTMLDelegate.get());
    setAlternatingRowColors(true);
    mModel = new QStandardItemModel();
    setModel(mModel);

    setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);
    setDragEnabled(true);
    setAcceptDrops(false);

    QObject::connect(this, &QTreeView::clicked, this, &TreeViewMemoryFields::cellClicked);
    QObject::connect(this, &QTreeView::collapsed, this, &TreeViewMemoryFields::cellCollapsed);
}

void S2Plugin::TreeViewMemoryFields::setMemoryMappedData(MemoryMappedData* mmd)
{
    mMemoryMappedData = mmd;
}

QStandardItem* S2Plugin::TreeViewMemoryFields::addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent)
{
    auto createAndInsertItem = [](const MemoryField& field, const std::string& fieldNameUID, QStandardItem* itemParent) -> QStandardItem*
    {
        auto itemFieldName = new QStandardItem();
        itemFieldName->setData(QString::fromStdString(field.name), Qt::DisplayRole);
        itemFieldName->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldName->setData(QVariant::fromValue(field), gsRoleEntireMemoryField);
        itemFieldName->setEditable(false);

        auto itemFieldValue = new QStandardItem();
        itemFieldValue->setData("", Qt::DisplayRole);
        itemFieldValue->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValue->setData(QVariant::fromValue(field.type), gsRoleType); // in case we click on, we can see the type
        itemFieldValue->setData(QVariant::fromValue(field), gsRoleEntireMemoryField);
        itemFieldValue->setEditable(false);

        auto itemFieldValueHex = new QStandardItem();
        itemFieldValueHex->setData("", Qt::DisplayRole);
        itemFieldValueHex->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValueHex->setData(QVariant::fromValue(field.type), gsRoleType); // in case we click on, we can see the type
        itemFieldValueHex->setData(QVariant::fromValue(field), gsRoleEntireMemoryField);
        itemFieldValueHex->setEditable(false);

        auto itemFieldComparisonValue = new QStandardItem();
        itemFieldComparisonValue->setData("", Qt::DisplayRole);
        itemFieldComparisonValue->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldComparisonValue->setEditable(false);

        auto itemFieldComparisonValueHex = new QStandardItem();
        itemFieldComparisonValueHex->setData("", Qt::DisplayRole);
        itemFieldComparisonValueHex->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldComparisonValueHex->setEditable(false);

        auto itemFieldMemoryOffset = new QStandardItem();
        itemFieldMemoryOffset->setData("", Qt::DisplayRole);
        itemFieldMemoryOffset->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldMemoryOffset->setEditable(false);

        auto itemFieldMemoryOffsetDelta = new QStandardItem();
        itemFieldMemoryOffsetDelta->setData("", Qt::DisplayRole);
        itemFieldMemoryOffsetDelta->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldMemoryOffsetDelta->setEditable(false);

        auto itemFieldComment = new QStandardItem();
        itemFieldComment->setData(QString::fromStdString(field.comment).toHtmlEscaped(), Qt::DisplayRole);
        itemFieldComment->setEditable(false);

        auto itemFieldType = new QStandardItem();
        if (field.type == MemoryFieldType::EntitySubclass)
        {
            itemFieldType->setData(QString::fromStdString(field.jsonName), Qt::DisplayRole);
        }
        else if (field.type == MemoryFieldType::PointerType)
        {
            itemFieldType->setData(QString::fromStdString(field.jsonName), Qt::DisplayRole);
        }
        else if (field.type == MemoryFieldType::InlineStructType)
        {
            itemFieldType->setData(QString::fromStdString(field.jsonName), Qt::DisplayRole);
        }
        else if (gsMemoryFieldTypeToStringMapping.count(field.type) > 0)
        {
            itemFieldType->setData(QString::fromStdString(gsMemoryFieldTypeToStringMapping.at(field.type)), Qt::DisplayRole);
        }
        else
        {
            itemFieldType->setData("Unknown field type", Qt::DisplayRole);
        }
        itemFieldType->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldType->setEditable(false);

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
        case MemoryFieldType::ThemeInfoName:
        case MemoryFieldType::Vector:
        case MemoryFieldType::UTF16Char:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::State8:
        case MemoryFieldType::State16:
        case MemoryFieldType::State32:
        case MemoryFieldType::VirtualFunctionTable:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            break;
        }
        case MemoryFieldType::Flags32:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (uint8_t x = 1; x <= 32; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent);
                flagFieldItem->setData(x, gsRoleFlagIndex);
                flagFieldItem->setData(32, gsRoleFlagsSize);
                if (!field.parentPointerJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentPointerJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else if (!field.parentStructJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentStructJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else
                {
                    flagFieldItem->setData(QString::fromStdString(fieldNameOverride), gsRoleFlagFieldName);
                }
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::Flags16:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (uint8_t x = 1; x <= 16; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent);
                flagFieldItem->setData(x, gsRoleFlagIndex);
                flagFieldItem->setData(16, gsRoleFlagsSize);
                if (!field.parentPointerJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentPointerJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else if (!field.parentStructJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentStructJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else
                {
                    flagFieldItem->setData(QString::fromStdString(fieldNameOverride), gsRoleFlagFieldName);
                }
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::Flags8:
        {
            auto flagsParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (uint8_t x = 1; x <= 8; ++x)
            {
                MemoryField flagField;
                flagField.name = "flag_" + std::to_string(x);
                flagField.type = MemoryFieldType::Flag;
                auto flagFieldItem = createAndInsertItem(flagField, fieldNameOverride + "." + flagField.name, flagsParent);
                flagFieldItem->setData(x, gsRoleFlagIndex);
                flagFieldItem->setData(8, gsRoleFlagsSize);
                if (!field.parentPointerJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentPointerJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else if (!field.parentStructJsonName.empty())
                {
                    flagFieldItem->setData(QString::fromStdString(field.parentStructJsonName + "." + field.name), gsRoleFlagFieldName);
                }
                else
                {
                    flagFieldItem->setData(QString::fromStdString(fieldNameOverride), gsRoleFlagFieldName);
                }
            }
            returnField = flagsParent;
            break;
        }
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer("ThemeInfoPointer"))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
        case MemoryFieldType::PointerList:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFields(MemoryFieldType::PointerList))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFieldsOfEntitySubclass(field.jsonName))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
        case MemoryFieldType::PointerType:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer(field.jsonName))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
        case MemoryFieldType::InlineStructType:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFieldsOfInlineStruct(field.jsonName))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
        default:
        {
            returnField = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : mToolbar->configuration()->typeFields(field.type))
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, returnField);
            }
            break;
        }
    }
    return returnField;
}

void S2Plugin::TreeViewMemoryFields::updateTableHeader(bool restoreColumnWidths)
{
    mModel->setHeaderData(gsColField, Qt::Horizontal, "Field", Qt::DisplayRole);
    mModel->setHeaderData(gsColValue, Qt::Horizontal, "Value", Qt::DisplayRole);
    mModel->setHeaderData(gsColValueHex, Qt::Horizontal, "Value (hex)", Qt::DisplayRole);
    mModel->setHeaderData(gsColComparisonValue, Qt::Horizontal, "Comparison value", Qt::DisplayRole);
    mModel->setHeaderData(gsColComparisonValueHex, Qt::Horizontal, "Comparison value (hex)", Qt::DisplayRole);
    mModel->setHeaderData(gsColType, Qt::Horizontal, "Type", Qt::DisplayRole);
    mModel->setHeaderData(gsColMemoryOffset, Qt::Horizontal, "Memory offset", Qt::DisplayRole);
    mModel->setHeaderData(gsColMemoryOffsetDelta, Qt::Horizontal, "Δ", Qt::DisplayRole);
    mModel->setHeaderData(gsColComment, Qt::Horizontal, "Comment", Qt::DisplayRole);

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

QStandardItem* S2Plugin::TreeViewMemoryFields::lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent)
{
    if (parent == nullptr)
    {
        parent = mModel->invisibleRootItem();
    }
    for (size_t x = 0; x < parent->rowCount(); ++x)
    {
        auto child = parent->child(x, column);
        if (child->data(gsRoleUID).toString().compare(QString::fromStdString(fieldName)) == 0)
        {
            return child;
        }
    }
    return nullptr;
}

void S2Plugin::TreeViewMemoryFields::updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, std::unordered_map<std::string, size_t>& offsets,
                                                         size_t memoryOffsetDeltaReference, QStandardItem* parent, bool disableChangeHighlightingForField)
{
    size_t memoryOffset = 0;
    size_t comparisonMemoryOffset = 0;
    if (offsets.count(fieldNameOverride) != 0)
    {
        memoryOffset = offsets.at(fieldNameOverride);
    }
    if (offsets.count("comparison." + fieldNameOverride) != 0)
    {
        comparisonMemoryOffset = offsets.at("comparison." + fieldNameOverride);
    }

    QStandardItem* itemField = nullptr;
    QStandardItem* itemValue = nullptr;
    QStandardItem* itemValueHex = nullptr;
    QStandardItem* itemComparisonValue = nullptr;
    QStandardItem* itemComparisonValueHex = nullptr;
    QStandardItem* itemMemoryOffset = nullptr;
    QStandardItem* itemMemoryOffsetDelta = nullptr;
    if (field.type != MemoryFieldType::Skip)
    {
        itemField = lookupTreeViewItem(fieldNameOverride, gsColField, parent);
        itemValue = lookupTreeViewItem(fieldNameOverride, gsColValue, parent);
        itemValueHex = lookupTreeViewItem(fieldNameOverride, gsColValueHex, parent);
        itemComparisonValue = lookupTreeViewItem(fieldNameOverride, gsColComparisonValue, parent);
        itemComparisonValueHex = lookupTreeViewItem(fieldNameOverride, gsColComparisonValueHex, parent);
        itemMemoryOffset = lookupTreeViewItem(fieldNameOverride, gsColMemoryOffset, parent);
        itemMemoryOffsetDelta = lookupTreeViewItem(fieldNameOverride, gsColMemoryOffsetDelta, parent);

        if (itemField == nullptr || itemValue == nullptr || itemValueHex == nullptr || itemMemoryOffset == nullptr)
        {
            dprintf("ERROR: tried to updateValueForField('%s', '%s', ...) but did not find items in treeview\n", field.name.c_str(), fieldNameOverride.c_str());
            return;
        }

        itemField->setData(QString::fromStdString(fieldNameOverride), gsRoleFieldName);
        itemField->setData(memoryOffset, gsRoleMemoryOffset);
        itemMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memoryOffset), Qt::DisplayRole);
        itemMemoryOffset->setData(memoryOffset, gsRoleRawValue);
        itemMemoryOffsetDelta->setData(QString::asprintf("+0x%llX", memoryOffset - memoryOffsetDeltaReference), Qt::DisplayRole);
        itemMemoryOffsetDelta->setData(memoryOffset, gsRoleRawValue);
        itemValue->setData(memoryOffset, gsRoleMemoryOffset);
        itemValue->setData(QString::fromStdString(fieldNameOverride), gsRoleFieldName);
        itemComparisonValue->setData(comparisonMemoryOffset, gsRoleMemoryOffset);
        itemComparisonValue->setData(QString::fromStdString("comparison." + fieldNameOverride), gsRoleFieldName);
    }

    auto modelIndex = mModel->indexFromItem(itemField);
    auto shouldUpdateChildren = false;
    if (modelIndex.isValid())
    {
        shouldUpdateChildren = (itemField->hasChildren() && isExpanded(modelIndex));
    }

    auto highlightColor = mEnableChangeHighlighting ? QColor::fromRgb(255, 184, 184) : Qt::transparent;
    auto comparisonDifferenceColor = QColor::fromRgb(255, 221, 184);
    if (disableChangeHighlightingForField)
    {
        highlightColor = Qt::transparent;
    }

    switch (field.type)
    {
        case MemoryFieldType::CodePointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto newHexValue = QString::asprintf("<font color='green'><u>0x%016llX</u></font>", value);
            itemValue->setData(newHexValue, Qt::DisplayRole);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto hexComparisonValue = QString::asprintf("<font color='green'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValue->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::DataPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemValue->setData(newHexValue, Qt::DisplayRole);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValue->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Byte:
        {
            int8_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", static_cast<uint8_t>(value));
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int8_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%d", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%02X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            itemValue->setData(QString::asprintf("%u", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint8_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%u", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%02X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%04X", static_cast<uint16_t>(value));
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int16_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadWord(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%d", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%04X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            uint16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            itemValue->setData(QString::asprintf("%u", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%04X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint16_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadWord(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%u", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%04X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            itemValue->setData(QString::asprintf("%ld", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%ld", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UnsignedDword:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            itemValue->setData(QString::asprintf("%lu", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%lu", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            itemValue->setData(QString::asprintf("%lld", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%016llX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int64_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%lld", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%016llX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            itemValue->setData(QString::asprintf("%llu", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%016llX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint64_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("%llu", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%016llX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Float:
        {
            uint32_t dword = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            float value = reinterpret_cast<float&>(dword);
            itemValue->setData(QString::asprintf("%f", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", dword);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonDword = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            float comparisonValue = reinterpret_cast<float&>(comparisonDword);
            itemComparisonValue->setData(QString::asprintf("%f", comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonDword);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(dword != comparisonDword ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(dword != comparisonDword ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Bool:
        {
            uint8_t b = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            bool value = reinterpret_cast<bool&>(b);
            itemValue->setData(value ? "True" : "False", Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", b);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint8_t comparisonB = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            bool comparisonValue = reinterpret_cast<bool&>(comparisonB);
            itemComparisonValue->setData(comparisonValue ? "True" : "False", Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%02X", comparisonB);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(b != comparisonB ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(b != comparisonB ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Flags32:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            std::stringstream ss;
            auto counter = 0;
            for (auto x = 31; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemValue->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemField->setData(value, gsRoleRawValue);            // so we can access in MemoryFieldType::Flag
            itemField->setData(memoryOffset, gsRoleMemoryOffset); // so we can access in MemoryFieldType::Flag
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            std::stringstream ss2;
            counter = 0;
            for (auto x = 31; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss2 << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss2 << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss2 << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemComparisonValue->setData(QString::fromStdString(ss2.str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemField->setData(comparisonValue, gsRoleRawComparisonValue);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 32; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Flags16:
        {
            uint16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            std::stringstream ss;
            auto counter = 0;
            for (auto x = 15; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemValue->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%04lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemField->setData(value, gsRoleRawValue);            // so we can access in MemoryFieldType::Flag
            itemField->setData(memoryOffset, gsRoleMemoryOffset); // so we can access in MemoryFieldType::Flag
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint16_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadWord(comparisonMemoryOffset));
            std::stringstream ss2;
            counter = 0;
            for (auto x = 15; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss2 << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss2 << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss2 << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemComparisonValue->setData(QString::fromStdString(ss2.str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%04lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemField->setData(comparisonValue, gsRoleRawComparisonValue);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 16; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Flags8:
        {
            uint8_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            std::stringstream ss;
            auto counter = 0;
            for (auto x = 7; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemValue->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemField->setData(value, gsRoleRawValue);            // so we can access in MemoryFieldType::Flag
            itemField->setData(memoryOffset, gsRoleMemoryOffset); // so we can access in MemoryFieldType::Flag
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint8_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            std::stringstream ss2;
            counter = 0;
            for (auto x = 7; x >= 0; --x)
            {
                if (counter % 4 == 0)
                {
                    ss2 << (x + 1) << ": ";
                }
                if ((value & (1 << x)) == (1 << x))
                {
                    ss2 << "<font color='green'>Y</font> ";
                }
                else
                {
                    ss2 << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemComparisonValue->setData(QString::fromStdString(ss2.str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%02X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemField->setData(comparisonValue, gsRoleRawComparisonValue);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 8; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::State8:
        {
            std::string stateFieldName = "";
            if (!field.parentPointerJsonName.empty())
            {
                stateFieldName = field.parentPointerJsonName + "." + field.name;
            }
            else if (!field.parentStructJsonName.empty())
            {
                stateFieldName = field.parentStructJsonName + "." + field.name;
            }
            else
            {
                stateFieldName = fieldNameOverride;
            }

            int8_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            auto stateTitle = QString::fromStdString(std::to_string(value) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, value));
            itemValue->setData(stateTitle, Qt::DisplayRole);
            itemValue->setData(QString::fromStdString(stateFieldName), gsRoleBaseFieldName);
            auto newHexValue = QString::asprintf("0x%02X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int8_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            auto comparisonStateTitle = QString::fromStdString(std::to_string(comparisonValue) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, comparisonValue));
            itemComparisonValue->setData(comparisonStateTitle, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::State16:
        {
            std::string stateFieldName = "";
            if (!field.parentPointerJsonName.empty())
            {
                stateFieldName = field.parentPointerJsonName + "." + field.name;
            }
            else if (!field.parentStructJsonName.empty())
            {
                stateFieldName = field.parentStructJsonName + "." + field.name;
            }
            else
            {
                stateFieldName = fieldNameOverride;
            }

            int16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            auto stateTitle = QString::fromStdString(std::to_string(value) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, value));
            itemValue->setData(stateTitle, Qt::DisplayRole);
            itemValue->setData(QString::fromStdString(stateFieldName), gsRoleBaseFieldName);
            auto newHexValue = QString::asprintf("0x%04X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int16_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadWord(comparisonMemoryOffset));
            auto comparisonStateTitle = QString::fromStdString(std::to_string(comparisonValue) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, comparisonValue));
            itemComparisonValue->setData(comparisonStateTitle, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::State32:
        {
            std::string stateFieldName = "";
            if (!field.parentPointerJsonName.empty())
            {
                stateFieldName = field.parentPointerJsonName + "." + field.name;
            }
            else if (!field.parentStructJsonName.empty())
            {
                stateFieldName = field.parentStructJsonName + "." + field.name;
            }
            else
            {
                stateFieldName = fieldNameOverride;
            }

            int32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            auto stateTitle = QString::fromStdString(std::to_string(value) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, value));
            itemValue->setData(stateTitle, Qt::DisplayRole);
            itemValue->setData(QString::fromStdString(stateFieldName), gsRoleBaseFieldName);
            auto newHexValue = QString::asprintf("0x%08X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            auto comparisonStateTitle = QString::fromStdString(std::to_string(comparisonValue) + ": " + mToolbar->configuration()->stateTitle(stateFieldName, comparisonValue));
            itemComparisonValue->setData(comparisonStateTitle, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::Vector:
        {
            if (itemField->hasChildren())
            {
                itemField->removeRows(0, itemField->rowCount());
            }

            auto vectorCount = (memoryOffset == 0 ? 0 : (std::min)(50u, Script::Memory::ReadDword(memoryOffset + 20)));
            auto vectorItemsOffset = Script::Memory::ReadQword(memoryOffset + 8);
            for (auto x = 0; x < vectorCount; ++x)
            {
                MemoryField f;
                f.name = std::to_string(x);
                f.type = MemoryFieldType::EntityUID;
                auto subItemOffset = vectorItemsOffset + (x * sizeof(uint32_t));
                offsets[fieldNameOverride + "." + f.name] = subItemOffset;

                auto subItemValue = addMemoryField(f, fieldNameOverride + "." + f.name, itemField);
                subItemValue->setData(subItemOffset, gsRoleRawValue);

                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffset, itemField, true);
            }
            break;
        }
        case MemoryFieldType::Flag:
        {
            auto flagIndex = itemField->data(gsRoleFlagIndex).toUInt();
            auto value = itemField->parent()->data(gsRoleRawValue).toUInt();
            auto mask = (1 << (flagIndex - 1));
            auto flagSet = ((value & mask) == mask);
            auto flagFieldName = itemField->data(gsRoleFlagFieldName).toString().toStdString();
            auto flagTitle = QString::fromStdString(mToolbar->configuration()->flagTitle(flagFieldName, flagIndex));
            auto caption = QString("<font color='%1'>%2</font>").arg(flagSet ? "green" : "red", flagTitle);
            itemValue->setData(caption, Qt::DisplayRole);
            itemValue->setData(flagIndex, gsRoleFlagIndex);
            itemMemoryOffset->setData("", Qt::DisplayRole);
            itemMemoryOffsetDelta->setData("", Qt::DisplayRole);

            auto comparisonValue = itemField->parent()->data(gsRoleRawComparisonValue).toUInt();
            auto comparisonFlagSet = ((comparisonValue & mask) == mask);
            auto comparisonCaption = QString("<font color='%1'>%2</font>").arg(comparisonFlagSet ? "green" : "red", flagTitle);
            itemComparisonValue->setData(comparisonCaption, Qt::DisplayRole);
            itemComparisonValue->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(flagSet != comparisonFlagSet ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UTF16Char:
        {
            uint16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            auto valueByteArray = QByteArray((const char*)(&value), 2);
            itemValue->setData(QString("'<b>%1</b>' (%2)").arg(QString(valueByteArray)).arg(value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%04X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint16_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadWord(comparisonMemoryOffset));
            auto comparisonValueByteArray = QByteArray((const char*)(&comparisonValue), 2);
            itemComparisonValue->setData(QString("<b>%1</b>' (%2)").arg(QString(comparisonValueByteArray)).arg(comparisonValue), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%04X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::UTF16StringFixedSize:
        {
            char buffer[1024] = {0};
            Script::Memory::Read(memoryOffset, buffer, field.extraInfo, nullptr);
            auto valueString = QString::fromUtf16(reinterpret_cast<const ushort*>(buffer));
            itemValue->setData(valueString, Qt::DisplayRole);
            itemValueHex->setData("", Qt::DisplayRole);

            char comparisonBuffer[1024] = {0};
            Script::Memory::Read(comparisonMemoryOffset, comparisonBuffer, field.extraInfo, nullptr);
            auto comparisonValueString = QString::fromUtf16(reinterpret_cast<const ushort*>(comparisonBuffer));
            itemComparisonValue->setData(comparisonValueString, Qt::DisplayRole);
            itemComparisonValueHex->setData("", Qt::DisplayRole);
            itemComparisonValue->setBackground(valueString != comparisonValueString ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::EntityDBID:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            itemValue->setData(QString::asprintf("<font color='blue'><u>%lu (%s)</u></font>", value, mToolbar->entityDB()->entityList()->nameForID(value).c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%lu (%s)</u></font>", comparisonValue, mToolbar->entityDB()->entityList()->nameForID(comparisonValue).c_str()),
                                         Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::TextureDBID:
        {
            int32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            if (value < 0)
            {
                itemValue->setData(QString::asprintf("<font color='blue'><u>%ld (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", value), Qt::DisplayRole);
            }
            else
            {
                itemValue->setData(QString::asprintf("<font color='blue'><u>%ld (%s)</u></font>", value, mToolbar->textureDB()->nameForID(value).c_str()), Qt::DisplayRole);
            }
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            if (value < 0)
            {
                itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%ld (dynamically applied in ThemeInfo->get_dynamic_floor_texture_id())</u></font>", comparisonValue),
                                             Qt::DisplayRole);
            }
            else
            {
                itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%ld (%s)</u></font>", comparisonValue, mToolbar->textureDB()->nameForID(comparisonValue).c_str()),
                                             Qt::DisplayRole);
            }
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::StringsTableID:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            itemValue->setData(QString("%1: %2").arg(value).arg(mToolbar->stringsTable()->nameForID(value)), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            itemComparisonValue->setData(QString("%1: %2").arg(comparisonValue).arg(mToolbar->stringsTable()->nameForID(comparisonValue)), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::ParticleDBID:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            itemValue->setData(QString::asprintf("<font color='blue'><u>%lu (%s)</u></font>", value, mToolbar->particleDB()->particleEmittersList()->nameForID(value).c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            itemComparisonValue->setData(
                QString::asprintf("<font color='blue'><u>%lu (%s)</u></font>", comparisonValue, mToolbar->particleDB()->particleEmittersList()->nameForID(comparisonValue).c_str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::EntityUID:
        {
            int32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
            if (value <= 0)
            {
                itemValue->setData("Nothing", Qt::DisplayRole);
            }
            else
            {
                auto entityOffset = Entity::findEntityByUID(value, mToolbar->state());
                if (entityOffset != 0)
                {
                    auto entityName = mToolbar->configuration()->spelunky2()->getEntityName(entityOffset, mToolbar->entityDB());
                    itemValue->setData(QString::asprintf("<font color='blue'><u>UID %lu (%s)</u></font>", value, entityName.c_str()), Qt::DisplayRole);
                }
                else
                {
                    itemValue->setData("UNKNOWN ENTITY", Qt::DisplayRole);
                }
            }
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(comparisonMemoryOffset));
            if (comparisonValue <= 0)
            {
                itemComparisonValue->setData("Nothing", Qt::DisplayRole);
            }
            else
            {
                auto comparisonEntityOffset = Entity::findEntityByUID(comparisonValue, mToolbar->state());
                if (comparisonEntityOffset != 0)
                {
                    auto entityName = mToolbar->configuration()->spelunky2()->getEntityName(comparisonEntityOffset, mToolbar->entityDB());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>UID %lu (%s)</u></font>", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                }
                else
                {
                    itemComparisonValue->setData("UNKNOWN ENTITY", Qt::DisplayRole);
                }
            }
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            break;
        }
        case MemoryFieldType::EntityUIDPointer:
        {
            int32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(Script::Memory::ReadQword(memoryOffset)));
            if (value <= 0)
            {
                itemValue->setData("Nothing", Qt::DisplayRole);
            }
            else
            {
                auto entityOffset = Entity::findEntityByUID(value, mToolbar->state());
                if (entityOffset != 0)
                {
                    auto entityName = mToolbar->configuration()->spelunky2()->getEntityName(entityOffset, mToolbar->entityDB());
                    itemValue->setData(QString::asprintf("<font color='blue'><u>UID %lu (%s)</u></font>", value, entityName.c_str()), Qt::DisplayRole);
                }
                else
                {
                    itemValue->setData("UNKNOWN ENTITY", Qt::DisplayRole);
                }
            }
            auto newHexValue = QString::asprintf("0x%08lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            int32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadDword(Script::Memory::ReadQword(comparisonMemoryOffset)));
            if (comparisonValue <= 0)
            {
                itemComparisonValue->setData("Nothing", Qt::DisplayRole);
            }
            else
            {
                auto comparisonEntityOffset = Entity::findEntityByUID(comparisonValue, mToolbar->state());
                if (comparisonEntityOffset != 0)
                {
                    auto entityName = mToolbar->configuration()->spelunky2()->getEntityName(comparisonEntityOffset, mToolbar->entityDB());
                    itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>UID %lu (%s)</u></font>", comparisonValue, entityName.c_str()), Qt::DisplayRole);
                }
                else
                {
                    itemComparisonValue->setData("UNKNOWN ENTITY", Qt::DisplayRole);
                }
            }
            auto hexComparisonValue = QString::asprintf("0x%08lX", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);

            break;
        }
        case MemoryFieldType::EntityPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto entityName = mToolbar->configuration()->spelunky2()->getEntityName(value, mToolbar->entityDB());
            itemValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", entityName.c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto comparisonEntityName = mToolbar->configuration()->spelunky2()->getEntityName(comparisonValue, mToolbar->entityDB());
            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", comparisonEntityName.c_str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::EntityDBPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto id = Script::Memory::ReadDword(value + 20);
            auto entityName = mToolbar->entityDB()->entityList()->nameForID(id);
            itemValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", id, entityName.c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto comparisonID = Script::Memory::ReadDword(comparisonValue + 20);
            auto comparisonEntityName = mToolbar->entityDB()->entityList()->nameForID(comparisonID);
            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>EntityDB %d %s</u></font>", comparisonID, comparisonEntityName.c_str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::TextureDBPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto id = Script::Memory::ReadQword(value);
            auto textureName = mToolbar->textureDB()->nameForID(id);
            itemValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", id, textureName.c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto comparisonID = Script::Memory::ReadQword(comparisonValue);
            auto comparisonTextureName = mToolbar->textureDB()->nameForID(comparisonID);
            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>TextureDB %d %s</u></font>", comparisonID, comparisonTextureName.c_str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::LevelGenPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto id = Script::Memory::ReadDword(value + 20);
            itemValue->setData("<font color='blue'><u>Show level gen</u></font>", Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            // no comparison in Entity
            break;
        }
        case MemoryFieldType::ParticleDBPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto id = Script::Memory::ReadDword(value);
            auto particleName = mToolbar->particleDB()->particleEmittersList()->nameForID(id);
            itemValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", id, particleName.c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            auto comparisonID = Script::Memory::ReadDword(comparisonValue);
            auto comparisonParticleName = mToolbar->particleDB()->particleEmittersList()->nameForID(comparisonID);
            itemComparisonValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", comparisonID, comparisonParticleName.c_str()), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            itemValue->setData("<font color='blue'><u>Show functions</u></font>", Qt::DisplayRole);
            itemField->setBackground(Qt::transparent);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            size_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadQword(comparisonMemoryOffset));
            itemComparisonValue->setData("", Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(Qt::transparent);
            itemComparisonValueHex->setBackground(Qt::transparent);
            break;
        }
        case MemoryFieldType::CharacterDBID:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            itemValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(value).arg(mToolbar->characterDB()->characterNames().at(value)), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            uint32_t comparisonValue = (comparisonMemoryOffset == 0 ? 0 : Script::Memory::ReadByte(comparisonMemoryOffset));
            itemComparisonValue->setData(QString("<font color='blue'><u>%1 (%2)</u></font>").arg(comparisonValue).arg(mToolbar->characterDB()->characterNames().at(comparisonValue)), Qt::DisplayRole);
            auto hexComparisonValue = QString::asprintf("0x%02X", comparisonValue);
            itemComparisonValueHex->setData(hexComparisonValue, Qt::DisplayRole);
            itemComparisonValue->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            itemComparisonValueHex->setBackground(value != comparisonValue ? comparisonDifferenceColor : Qt::transparent);
            break;
        }
        case MemoryFieldType::PointerList:
        {
            itemField->setData(QString::fromStdString(field.pointerListPointerType), gsRolePointerListPointerType);

            auto value = Script::Memory::ReadQword(memoryOffset);
            if (value == 0)
            {
                itemValue->setData("<font color='#aaa'>nullptr</font>", Qt::DisplayRole);
            }
            else
            {
                itemValue->setData("<font color='#aaa'>Refresh items by collapsing and reopening the items node</font>", Qt::DisplayRole);
            }

            if (shouldUpdateChildren)
            {
                for (const auto& f : mToolbar->configuration()->typeFields(MemoryFieldType::PointerList))
                {
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffset, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::PointerListItems:
        {
            itemField->setData(QString::fromStdString(gsMemoryFieldTypeToStringMapping.at(MemoryFieldType::PointerListItems)), gsRoleFieldType);

            auto parentFieldName = itemField->parent()->data(gsRoleFieldName).toString().toStdString();
            auto pointerType = itemField->parent()->data(gsRolePointerListPointerType).toString().toStdString();

            if (offsets.count(parentFieldName + ".begin") > 0 && offsets.count(parentFieldName + ".end") > 0)
            {
                auto offsetBegin = Script::Memory::ReadQword(offsets.at(parentFieldName + ".begin"));
                auto offsetEnd = Script::Memory::ReadQword(offsets.at(parentFieldName + ".end"));

                auto addChildren = (!itemField->hasChildren());

                auto count = (offsetEnd - offsetBegin) / sizeof(size_t);
                for (auto x = 0; x < count; ++x)
                {
                    MemoryField f;
                    f.name = std::to_string(x);
                    if (gsJSONStringToMemoryFieldTypeMapping.count(pointerType) > 0)
                    {
                        f.type = gsJSONStringToMemoryFieldTypeMapping.at(pointerType);
                    }
                    else
                    {
                        f.type = MemoryFieldType::PointerType;
                        f.isPointer = true;
                        f.jsonName = pointerType;
                    }
                    auto subItemOffset = offsetBegin + (x * sizeof(size_t));
                    if (addChildren)
                    {
                        addMemoryField(f, fieldNameOverride + "." + f.name, itemField);
                    }

                    mMemoryMappedData->setOffsetForField(f, fieldNameOverride + "." + f.name, subItemOffset, offsets);

                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffset, itemField, true);
                }
            }
            break;
        }
        case MemoryFieldType::ConstCharPointerPointer:
        {
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
            constexpr uint16_t bufferSize = 1024;
            char buffer[bufferSize] = {0};

            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            if (value != 0)
            {
                size_t chararray = value;
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
                size_t chararray = comparisonValue;
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
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            if (memoryOffset == 0)
            {
                itemValue->setData("NO THEME", Qt::DisplayRole);
            }
            else
            {
                size_t themeInfoPointer = Script::Memory::ReadQword(memoryOffset);
                itemValue->setData(QString::fromStdString(mToolbar->levelGen()->themeNameOfOffset(themeInfoPointer)), Qt::DisplayRole);
            }

            for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer("ThemeInfoPointer"))
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffset, itemField);
            }

            // no comparison in Entity
            break;
        }
        case MemoryFieldType::LevelGenRoomsPointer:
        case MemoryFieldType::LevelGenRoomsMetaPointer:
        {
            itemValue->setData("<font color='blue'><u>Show rooms</u></font>", Qt::DisplayRole);
            // no comparison in Entity
            break;
        }
        case MemoryFieldType::ThemeInfoName:
        {
            if (memoryOffset == 0)
            {
                itemValue->setData("n/a", Qt::DisplayRole);
            }
            else
            {
                size_t themeInfoPointer = Script::Memory::ReadQword(memoryOffset);
                if (themeInfoPointer == 0)
                {
                    itemValue->setData("n/a", Qt::DisplayRole);
                }
                else
                {
                    itemValue->setData(QString::fromStdString(mToolbar->levelGen()->themeNameOfOffset(themeInfoPointer)), Qt::DisplayRole);
                }
            }
            // no comparison in Entity
            break;
        }
        case MemoryFieldType::Skip:
        {
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            if (shouldUpdateChildren)
            {
                for (const auto& f : mToolbar->configuration()->typeFieldsOfEntitySubclass(field.jsonName))
                {
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::PointerType:
        {
            auto value = Script::Memory::ReadQword(memoryOffset);
            if (value == 0)
            {
                itemValue->setData("<font color='#aaa'>nullptr</font>", Qt::DisplayRole);
            }
            else
            {
                itemValue->setData("", Qt::DisplayRole);
            }

            if (shouldUpdateChildren)
            {
                for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer(field.jsonName))
                {
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, Script::Memory::ReadQword(memoryOffset), itemField);
                }
            }
            break;
        }
        case MemoryFieldType::InlineStructType:
        {
            if (shouldUpdateChildren)
            {
                for (const auto& f : mToolbar->configuration()->typeFieldsOfInlineStruct(field.jsonName))
                {
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
        default:
        {
            if (shouldUpdateChildren)
            {
                for (const auto& f : mToolbar->configuration()->typeFields(field.type))
                {
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, memoryOffsetDeltaReference, itemField);
                }
            }
            break;
        }
    }
}

void S2Plugin::TreeViewMemoryFields::cellClicked(const QModelIndex& index)
{
    auto column = index.column();
    auto clickedItem = mModel->itemFromIndex(index);
    switch (column)
    {
        case gsColMemoryOffset:
        case gsColMemoryOffsetDelta:
        {
            GuiDumpAt(clickedItem->data(gsRoleRawValue).toULongLong());
            GuiShowCpu();
            break;
        }
        case gsColValue:
        case gsColValueHex:
        {
            auto dataType = clickedItem->data(gsRoleType).value<MemoryFieldType>();
            switch (dataType)
            {
                case MemoryFieldType::CodePointer:
                {
                    GuiDisasmAt(clickedItem->data(gsRoleRawValue).toULongLong(), GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
                case MemoryFieldType::ConstCharPointerPointer:
                case MemoryFieldType::ConstCharPointer:
                case MemoryFieldType::DataPointer:
                {
                    GuiDumpAt(clickedItem->data(gsRoleRawValue).toULongLong());
                    GuiShowCpu();
                    break;
                }
                case MemoryFieldType::EntityPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        mToolbar->showEntity(offset);
                    }
                    break;
                }
                case MemoryFieldType::EntityUID:
                case MemoryFieldType::EntityUIDPointer:
                {
                    auto uid = clickedItem->data(gsRoleRawValue).toUInt();
                    if (uid != 0)
                    {
                        auto offset = Entity::findEntityByUID(uid, mToolbar->state());
                        if (offset != 0)
                        {
                            mToolbar->showEntity(offset);
                        }
                    }
                    break;
                }
                case MemoryFieldType::EntityDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id != -1)
                    {
                        auto view = mToolbar->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::CharacterDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id != -1)
                    {
                        auto view = mToolbar->showCharacterDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id >= 0)
                    {
                        auto view = mToolbar->showTextureDB();
                        if (view != nullptr)
                        {
                            view->showID(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBID:
                {
                    auto id = clickedItem->data(gsRoleRawValue).toUInt();
                    if (id != -1)
                    {
                        auto view = mToolbar->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::EntityDBPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        auto id = Script::Memory::ReadDword(offset + 20);
                        auto view = mToolbar->showEntityDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::TextureDBPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        auto id = Script::Memory::ReadQword(offset);
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
                    auto offset = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = clickedItem->data(gsRoleBaseFieldName).toString();
                        auto dialog = new DialogEditState(mToolbar->configuration(), fieldName, offset, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        mToolbar->showLevelGen();
                    }
                    break;
                }
                case MemoryFieldType::ParticleDBPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        auto id = Script::Memory::ReadDword(offset);
                        auto view = mToolbar->showParticleDB();
                        if (view != nullptr)
                        {
                            view->showIndex(id);
                        }
                    }
                    break;
                }
                case MemoryFieldType::VirtualFunctionTable:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        auto vftType = clickedItem->data(gsRoleEntireMemoryField).value<MemoryField>().virtualFunctionTableType;
                        if (vftType == "Entity") // in case of Entity, we have to see what the entity is interpreted as, and show those functions
                        {
                            auto entity = dynamic_cast<Entity*>(mMemoryMappedData);
                            if (entity != nullptr)
                            {
                                mToolbar->showVirtualFunctions(offset, entity->entityType());
                            }
                        }
                        else
                        {
                            mToolbar->showVirtualFunctions(offset, vftType);
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
                    auto flagIndex = clickedItem->data(gsRoleFlagIndex).toUInt();
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
                case MemoryFieldType::UTF16Char:
                case MemoryFieldType::StringsTableID:
                {
                    auto offset = clickedItem->data(gsRoleMemoryOffset).toULongLong();
                    if (offset != 0)
                    {
                        auto fieldName = clickedItem->data(gsRoleFieldName).toString();
                        auto dialog = new DialogEditSimpleValue(fieldName, offset, dataType, this);
                        dialog->exec();
                    }
                    break;
                }
                case MemoryFieldType::LevelGenRoomsPointer:
                case MemoryFieldType::LevelGenRoomsMetaPointer:
                {
                    emit levelGenRoomsPointerClicked(clickedItem->data(gsRoleFieldName).toString());
                    break;
                }
            }
            emit memoryFieldValueUpdated(clickedItem->data(gsRoleFieldName).toString());
        }
    }
}

void S2Plugin::TreeViewMemoryFields::cellCollapsed(const QModelIndex& index)
{
    auto item = mModel->itemFromIndex(index);
    auto fieldName = item->data(gsRoleFieldType).toString().toStdString();
    if (fieldName == gsMemoryFieldTypeToStringMapping.at(MemoryFieldType::PointerListItems))
    {
        item->removeRows(0, item->rowCount());
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
    auto data = event->mimeData()->data("spelunky/entityoffset");
    emit entityOffsetDropped(data.toULongLong());
    event->acceptProposedAction();
}

void S2Plugin::TreeViewMemoryFields::startDrag(Qt::DropActions supportedActions)
{
    auto ix = selectedIndexes();
    if (ix.count() == 0)
    {
        return;
    }

    QDrag* drag = new QDrag(this);
    auto mimeData = new QMimeData();

    auto index = ix.at(0);

    // for spelunky/entityoffset: dragging an entity from ViewEntities on top of ViewEntity for comparison
    auto entityItem = mModel->item(index.row(), gsColMemoryOffset);
    if (entityItem != nullptr)
    {
        auto entityData = entityItem->data(gsRoleRawValue);
        if (entityData.isValid())
        {
            mimeData->setData("spelunky/entityoffset", QByteArray().setNum(Script::Memory::ReadQword(entityData.toULongLong())));
        }
    }

    // for spelunky/memoryfield: dragging any memoryfield onto ViewLogger
    auto selectedItem = mModel->itemFromIndex(index);
    auto memoryField = selectedItem->data(gsRoleEntireMemoryField).value<MemoryField>();
    auto uniqueFieldName = selectedItem->data(gsRoleUID).toString().toStdString();
    auto memoryOffset = selectedItem->data(gsRoleMemoryOffset).toULongLong();

    nlohmann::json o;
    o[gsJSONDragDropMemoryField_UID] = uniqueFieldName;
    o[gsJSONDragDropMemoryField_Offset] = memoryOffset;
    o[gsJSONDragDropMemoryField_Type] = memoryField.type;
    auto json = QString::fromStdString(o.dump());

    auto codec = QTextCodec::codecForName("UTF-8");
    mimeData->setData("spelunky/memoryfield", codec->fromUnicode(json));

    drag->setMimeData(mimeData);
    drag->exec();
}
