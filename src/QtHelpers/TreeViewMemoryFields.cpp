#include "QtHelpers/TreeViewMemoryFields.h"
#include "pluginmain.h"
#include <inttypes.h>
#include <sstream>

TreeViewMemoryFields::TreeViewMemoryFields(EntityDB* entityDB, QWidget* parent) : QTreeView(parent), mEntityDB(entityDB)
{
    mHTMLDelegate = std::make_unique<HTMLDelegate>();
    setItemDelegate(mHTMLDelegate.get());
    setAlternatingRowColors(true);

    mModel = new QStandardItemModel();
    setModel(mModel);

    QObject::connect(this, &QTreeView::clicked, this, &TreeViewMemoryFields::cellClicked);
}

void TreeViewMemoryFields::addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent)
{
    auto createAndInsertItem = [](const std::string& fieldName, const std::string& fieldNameUID, const std::string& fieldType, QStandardItem* itemParent) -> QStandardItem* {
        auto itemFieldName = new QStandardItem();
        itemFieldName->setData(QString::fromStdString(fieldName), Qt::DisplayRole);
        itemFieldName->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldName->setEditable(false);

        auto itemFieldValue = new QStandardItem();
        itemFieldValue->setData("", Qt::DisplayRole);
        itemFieldValue->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValue->setData(QString::fromStdString(fieldType), gsRoleType); // in case we click on, we can see the type
        itemFieldValue->setEditable(false);

        auto itemFieldValueHex = new QStandardItem();
        itemFieldValueHex->setData("", Qt::DisplayRole);
        itemFieldValueHex->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldValueHex->setEditable(false);

        auto itemFieldMemoryOffset = new QStandardItem();
        itemFieldMemoryOffset->setData("", Qt::DisplayRole);
        itemFieldMemoryOffset->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldMemoryOffset->setEditable(false);

        auto itemFieldType = new QStandardItem();
        itemFieldType->setData(QString::fromStdString(fieldType), Qt::DisplayRole);
        itemFieldType->setData(QString::fromStdString(fieldNameUID), gsRoleUID);
        itemFieldType->setEditable(false);

        itemParent->appendRow(QList<QStandardItem*>() << itemFieldName << itemFieldValue << itemFieldValueHex << itemFieldMemoryOffset << itemFieldType);

        return itemFieldName;
    };

    if (parent == nullptr)
    {
        parent = mModel->invisibleRootItem();
    }

    switch (field.type)
    {
        case MemoryFieldType::Skip:
            break;
        case MemoryFieldType::CodePointer:
            createAndInsertItem(field.name, fieldNameOverride, "Code pointer", parent);
            break;
        case MemoryFieldType::DataPointer:
            createAndInsertItem(field.name, fieldNameOverride, "Data pointer", parent);
            break;
        case MemoryFieldType::Byte:
            createAndInsertItem(field.name, fieldNameOverride, "8-bit", parent);
            break;
        case MemoryFieldType::UnsignedByte:
            createAndInsertItem(field.name, fieldNameOverride, "8-bit unsigned", parent);
            break;
        case MemoryFieldType::Word:
            createAndInsertItem(field.name, fieldNameOverride, "16-bit", parent);
            break;
        case MemoryFieldType::UnsignedWord:
            createAndInsertItem(field.name, fieldNameOverride, "16-bit unsigned", parent);
            break;
        case MemoryFieldType::Dword:
            createAndInsertItem(field.name, fieldNameOverride, "32-bit", parent);
            break;
        case MemoryFieldType::UnsignedDword:
            createAndInsertItem(field.name, fieldNameOverride, "32-bit unsigned", parent);
            break;
        case MemoryFieldType::Qword:
            createAndInsertItem(field.name, fieldNameOverride, "64-bit", parent);
            break;
        case MemoryFieldType::UnsignedQword:
            createAndInsertItem(field.name, fieldNameOverride, "64-bit unsigned", parent);
            break;
        case MemoryFieldType::Float:
            createAndInsertItem(field.name, fieldNameOverride, "Float", parent);
            break;
        case MemoryFieldType::Bool:
            createAndInsertItem(field.name, fieldNameOverride, "Bool", parent);
            break;
        case MemoryFieldType::Flags32:
            createAndInsertItem(field.name, fieldNameOverride, "32-bit flags", parent);
            break;
        case MemoryFieldType::Rect:
        {
            auto rectangleParent = createAndInsertItem(field.name, fieldNameOverride, "Rectangle", parent);
            for (const auto& f : gsRectFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, rectangleParent);
            }
            break;
        }
        case MemoryFieldType::StateIllumination:
        {
            auto structParent = createAndInsertItem(field.name, fieldNameOverride, "Illumination pointer", parent);
            for (const auto& f : gsStateIlluminationFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::StateSaturationVignette:
        {
            auto structParent = createAndInsertItem(field.name, fieldNameOverride, "Saturation/Vignette", parent);
            for (const auto& f : gsStateSaturationVignetteFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::StateItems:
        {
            auto structParent = createAndInsertItem(field.name, fieldNameOverride, "Items (players)", parent);
            for (const auto& f : gsStateItemsFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::Layer:
        {
            auto structParent = createAndInsertItem(field.name, fieldNameOverride, "Layer", parent);
            for (const auto& f : gsLayerFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
    }
}

void TreeViewMemoryFields::addEntityDBMemoryFields(QStandardItem* parent)
{
    for (const auto& field : gsEntityDBFields)
    {
        addMemoryField(field, field.name, parent);
    }
    updateTableHeader();
}

void TreeViewMemoryFields::addStateMemoryFields(QStandardItem* parent)
{
    for (const auto& field : gsStateFields)
    {
        addMemoryField(field, field.name, parent);
    }
    updateTableHeader();
}

void TreeViewMemoryFields::updateTableHeader()
{
    mModel->setHeaderData(gsColField, Qt::Horizontal, "Field", Qt::DisplayRole);
    mModel->setHeaderData(gsColValue, Qt::Horizontal, "Value", Qt::DisplayRole);
    mModel->setHeaderData(gsColValueHex, Qt::Horizontal, "Value (hex)", Qt::DisplayRole);
    mModel->setHeaderData(gsColType, Qt::Horizontal, "Type", Qt::DisplayRole);
    mModel->setHeaderData(gsColMemoryOffset, Qt::Horizontal, "Memory offset", Qt::DisplayRole);
}

QStandardItem* TreeViewMemoryFields::lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent)
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

void TreeViewMemoryFields::updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, const std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent)
{
    auto memoryOffset = offsets.at(fieldNameOverride);

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
        itemMemoryOffset->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", memoryOffset), Qt::DisplayRole);
        itemMemoryOffset->setData(memoryOffset, gsRoleRawValue);
    }

    switch (field.type)
    {
        case MemoryFieldType::CodePointer:
        {
            size_t value = Script::Memory::ReadQword(memoryOffset);
            itemValue->setData(QString::asprintf("<font color='green'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("<font color='green'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
            break;
        }
        case MemoryFieldType::DataPointer:
        {
            size_t value = Script::Memory::ReadQword(memoryOffset);
            itemValue->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
            break;
        }
        case MemoryFieldType::Byte:
        {
            int8_t value = Script::Memory::ReadByte(memoryOffset);
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%02X", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t value = Script::Memory::ReadByte(memoryOffset);
            itemValue->setData(QString::asprintf("%u", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%02X", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t value = Script::Memory::ReadWord(memoryOffset);
            itemValue->setData(QString::asprintf("%d", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%04X", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::UnsignedWord:
        {
            uint16_t value = Script::Memory::ReadWord(memoryOffset);
            itemValue->setData(QString::asprintf("%u", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%04X", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t value = Script::Memory::ReadDword(memoryOffset);
            itemValue->setData(QString::asprintf("%ld", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%08lX", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::UnsignedDword:
        {
            uint32_t value = Script::Memory::ReadDword(memoryOffset);
            if (fieldNameOverride == "id")
            {
                itemValue->setData(QString::asprintf("%lu (%s)", value, mEntityDB->entityList()->nameForID(value).c_str()), Qt::DisplayRole);
            }
            else
            {
                itemValue->setData(QString::asprintf("%lu", value), Qt::DisplayRole);
            }
            itemValueHex->setData(QString::asprintf("0x%08lX", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t value = Script::Memory::ReadQword(memoryOffset);
            itemValue->setData(QString::asprintf("%lld", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%016llX", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t value = Script::Memory::ReadQword(memoryOffset);
            itemValue->setData(QString::asprintf("%llu", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%016llX", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Float:
        {
            uint32_t dword = Script::Memory::ReadDword(memoryOffset);
            float value = reinterpret_cast<float&>(dword);
            itemValue->setData(QString::asprintf("%f", value), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%08lX", dword), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Bool:
        {
            uint8_t b = Script::Memory::ReadByte(memoryOffset);
            bool value = reinterpret_cast<bool&>(b);
            itemValue->setData(value ? "True" : "False", Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%02X", b), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::Flags32:
        {
            uint32_t value = Script::Memory::ReadDword(memoryOffset);
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
                    ss << "<font color='green' alt='bebebeb'>Y</font> ";
                }
                else
                {
                    ss << "<font color='red'>N</font> ";
                }
                counter++;
            }
            itemValue->setData(QString::fromStdString(ss.str()), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%08X", value), Qt::DisplayRole);

            break;
        }
        case MemoryFieldType::Rect:
        {
            for (const auto& f : gsRectFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::StateIllumination:
        {
            for (const auto& f : gsStateIlluminationFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::StateSaturationVignette:
        {
            for (const auto& f : gsStateSaturationVignetteFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::StateItems:
        {
            for (const auto& f : gsStateItemsFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::Layer:
        {
            for (const auto& f : gsLayerFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::Skip:
        {
            break;
        }
    }
}

void TreeViewMemoryFields::cellClicked(const QModelIndex& index)
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
            auto dataType = clickedItem->data(gsRoleType).toString();
            if (dataType == "Code pointer")
            {
                GuiDisasmAt(clickedItem->data(gsRoleRawValue).toULongLong(), GetContextData(UE_CIP));
                GuiShowCpu();
            }
            else if (dataType == "Data pointer")
            {
                GuiDumpAt(clickedItem->data(gsRoleRawValue).toULongLong());
                GuiShowCpu();
            }
            break;
        }
    }
}
