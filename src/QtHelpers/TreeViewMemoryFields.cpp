#include "QtHelpers/TreeViewMemoryFields.h"
#include "Data/Entity.h"
#include "QtHelpers/DialogEditSimpleValue.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewParticleDB.h"
#include "pluginmain.h"
#include <inttypes.h>
#include <sstream>

S2Plugin::TreeViewMemoryFields::TreeViewMemoryFields(ViewToolbar* toolbar, QWidget* parent) : QTreeView(parent), mToolbar(toolbar)
{
    mHTMLDelegate = std::make_unique<HTMLDelegate>();
    setItemDelegate(mHTMLDelegate.get());
    setAlternatingRowColors(true);

    mModel = new QStandardItemModel();
    setModel(mModel);

    QObject::connect(this, &QTreeView::clicked, this, &TreeViewMemoryFields::cellClicked);
}

QStandardItem* S2Plugin::TreeViewMemoryFields::addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent)
{
    auto createAndInsertItem = [](const MemoryField& field, const std::string& fieldNameUID, QStandardItem* itemParent) -> QStandardItem* {
        auto itemFieldName = new QStandardItem();
        itemFieldName->setData(QString::fromStdString(field.name), Qt::DisplayRole);
        itemFieldName->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldName->setEditable(false);

        auto itemFieldValue = new QStandardItem();
        itemFieldValue->setData("", Qt::DisplayRole);
        itemFieldValue->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValue->setData(QVariant::fromValue(field.type), gsRoleType); // in case we click on, we can see the type
        itemFieldValue->setEditable(false);

        auto itemFieldValueHex = new QStandardItem();
        itemFieldValueHex->setData("", Qt::DisplayRole);
        itemFieldValueHex->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValueHex->setData(QVariant::fromValue(field.type), gsRoleType); // in case we click on, we can see the type
        itemFieldValueHex->setEditable(false);

        auto itemFieldMemoryOffset = new QStandardItem();
        itemFieldMemoryOffset->setData("", Qt::DisplayRole);
        itemFieldMemoryOffset->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldMemoryOffset->setEditable(false);

        auto itemFieldComment = new QStandardItem();
        itemFieldComment->setData(QString::fromStdString(field.comment), Qt::DisplayRole);
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

        itemParent->appendRow(QList<QStandardItem*>() << itemFieldName << itemFieldValue << itemFieldValueHex << itemFieldMemoryOffset << itemFieldType << itemFieldComment);

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
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::ParticleDBPointer:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::Vector:
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
                flagFieldItem->setData(QString::fromStdString(fieldNameOverride), gsRoleFlagFieldName);
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
                flagFieldItem->setData(QString::fromStdString(fieldNameOverride), gsRoleFlagFieldName);
            }
            returnField = flagsParent;
            break;
        }
        default: // default is assumed to be a container
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            if (field.type == MemoryFieldType::EntitySubclass)
            {
                for (const auto& f : mToolbar->configuration()->typeFieldsOfEntitySubclass(field.jsonName))
                {
                    addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
                }
            }
            else if (field.type == MemoryFieldType::PointerType)
            {
                for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer(field.jsonName))
                {
                    addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
                }
            }
            else
            {
                for (const auto& f : mToolbar->configuration()->typeFields(field.type))
                {
                    addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
                }
            }
            returnField = structParent;
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
    mModel->setHeaderData(gsColType, Qt::Horizontal, "Type", Qt::DisplayRole);
    mModel->setHeaderData(gsColMemoryOffset, Qt::Horizontal, "Memory offset", Qt::DisplayRole);
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
        if (mSavedColumnWidths[gsColMemoryOffset] != 0)
        {
            setColumnWidth(gsColMemoryOffset, mSavedColumnWidths[gsColMemoryOffset]);
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

void S2Plugin::TreeViewMemoryFields::updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent,
                                                         bool disableChangeHighlightingForField)
{
    size_t memoryOffset = 0;
    if (offsets.count(fieldNameOverride) != 0)
    {
        memoryOffset = offsets.at(fieldNameOverride);
    }

    QStandardItem* itemField = nullptr;
    QStandardItem* itemValue = nullptr;
    QStandardItem* itemValueHex = nullptr;
    QStandardItem* itemMemoryOffset = nullptr;
    if (field.type != MemoryFieldType::Skip)
    {
        itemField = lookupTreeViewItem(fieldNameOverride, gsColField, parent);
        itemValue = lookupTreeViewItem(fieldNameOverride, gsColValue, parent);
        itemValueHex = lookupTreeViewItem(fieldNameOverride, gsColValueHex, parent);
        itemMemoryOffset = lookupTreeViewItem(fieldNameOverride, gsColMemoryOffset, parent);

        if (itemField == nullptr || itemValue == nullptr || itemValueHex == nullptr || itemMemoryOffset == nullptr)
        {
            dprintf("ERROR: tried to updateValueForField('%s', '%s', ...) but did not find items in treeview\n", field.name.c_str(), fieldNameOverride.c_str());
            return;
        }

        itemMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memoryOffset), Qt::DisplayRole);
        itemMemoryOffset->setData(memoryOffset, gsRoleRawValue);
        itemValue->setData(memoryOffset, gsRoleMemoryOffset);
        itemValue->setData(QString::fromStdString(fieldNameOverride), gsRoleFieldName);
    }

    auto modelIndex = mModel->indexFromItem(itemField);
    auto shouldUpdateChildren = false;
    if (modelIndex.isValid())
    {
        shouldUpdateChildren = (itemField->hasChildren() && isExpanded(modelIndex));
    }

    auto highlightColor = mEnableChangeHighlighting ? QColor::fromRgb(255, 184, 184) : Qt::transparent;
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
            break;
        }
        case MemoryFieldType::Byte:
        {
            int8_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadByte(memoryOffset));
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%02X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
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
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadWord(memoryOffset));
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("0x%04X", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
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

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 32; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Flags16:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
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

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 16; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Flags8:
        {
            uint32_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadDword(memoryOffset));
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
            auto newHexValue = QString::asprintf("0x%02lX", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemField->setData(value, gsRoleRawValue);            // so we can access in MemoryFieldType::Flag
            itemField->setData(memoryOffset, gsRoleMemoryOffset); // so we can access in MemoryFieldType::Flag
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);

            if (shouldUpdateChildren)
            {
                for (uint8_t x = 1; x <= 8; ++x)
                {
                    MemoryField f;
                    f.name = "flag_" + std::to_string(x);
                    f.type = MemoryFieldType::Flag;
                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                }
            }
            break;
        }
        case MemoryFieldType::Vector:
        {
            if (shouldUpdateChildren)
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

                    updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField, true);
                }
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
            break;
        }
        case MemoryFieldType::ParticleDBPointer:
        {
            size_t value = (memoryOffset == 0 ? 0 : Script::Memory::ReadQword(memoryOffset));
            auto id = Script::Memory::ReadDword(value);
            auto particleName = mToolbar->particleDB()->nameForIndex(id);
            itemValue->setData(QString::asprintf("<font color='blue'><u>ParticleDB %d %s</u></font>", id, particleName.c_str()), Qt::DisplayRole);
            auto newHexValue = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value);
            itemField->setBackground(itemValueHex->data(Qt::DisplayRole) == newHexValue ? Qt::transparent : highlightColor);
            itemValueHex->setData(newHexValue, Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
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
            break;
        }
        case MemoryFieldType::Skip:
        {
            break;
        }
        default: // default is assumed to be a container
        {
            if (shouldUpdateChildren)
            {
                if (field.type == MemoryFieldType::EntitySubclass)
                {
                    for (const auto& f : mToolbar->configuration()->typeFieldsOfEntitySubclass(field.jsonName))
                    {
                        updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                    }
                }
                else if (field.type == MemoryFieldType::PointerType)
                {
                    for (const auto& f : mToolbar->configuration()->typeFieldsOfPointer(field.jsonName))
                    {
                        updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                    }
                }
                else
                {
                    for (const auto& f : mToolbar->configuration()->typeFields(field.type))
                    {
                        updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
                    }
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
                    auto uid = clickedItem->data(gsRoleRawValue).toUInt();
                    if (uid != -1)
                    {
                        auto offset = Entity::findEntityByUID(uid, mToolbar->state());
                        if (offset != 0)
                        {
                            auto view = mToolbar->showEntityDB();
                            if (view != nullptr)
                            {
                                view->showIndex(uid);
                            }
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
            }
            emit memoryFieldValueUpdated(clickedItem->data(gsRoleFieldName).toString());
        }
    }
}

void S2Plugin::TreeViewMemoryFields::clear()
{
    mSavedColumnWidths[gsColField] = columnWidth(gsColField);
    mSavedColumnWidths[gsColValue] = columnWidth(gsColValue);
    mSavedColumnWidths[gsColValueHex] = columnWidth(gsColValueHex);
    mSavedColumnWidths[gsColMemoryOffset] = columnWidth(gsColMemoryOffset);
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
