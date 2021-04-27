#include "QtHelpers/TreeViewMemoryFields.h"
#include "Views/ViewEntityDB.h"
#include "pluginmain.h"
#include <inttypes.h>
#include <sstream>

TreeViewMemoryFields::TreeViewMemoryFields(ViewToolbar* toolbar, QWidget* parent) : QTreeView(parent), mToolbar(toolbar)
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

        auto itemFieldType = new QStandardItem();
        itemFieldType->setData(QString::fromStdString(gsMemoryFieldTypeToStringMapping.at(field.type)), Qt::DisplayRole);
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
        case MemoryFieldType::Flags32:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::EntityDBPointer:
            createAndInsertItem(field, fieldNameOverride, parent);
            break;
        case MemoryFieldType::Rect:
        {
            auto rectangleParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsRectFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, rectangleParent);
            }
            break;
        }
        case MemoryFieldType::StateIllumination:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsStateIlluminationFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::StateSaturationVignette:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsStateSaturationVignetteFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::StateItems:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsStateItemsFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::Layer:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsLayerFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::Vector:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsVectorFields)
            {
                addMemoryField(f, fieldNameOverride + "." + f.name, structParent);
            }
            break;
        }
        case MemoryFieldType::Color:
        {
            auto structParent = createAndInsertItem(field, fieldNameOverride, parent);
            for (const auto& f : gsColorFields)
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

void TreeViewMemoryFields::addEntityFields(QStandardItem* parent)
{
    for (const auto& field : gsEntityFields)
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

        if (itemField == nullptr || itemValue == nullptr || itemValueHex == nullptr || itemMemoryOffset == nullptr)
        {
            dprintf("ERROR: tried to updateValueForField(%s, %s, ...) but did not find items in treeview", field.name, fieldNameOverride);
            return;
        }

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
            itemValue->setData(QString::asprintf("%lu", value), Qt::DisplayRole);
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
                    ss << "<font color='green'>Y</font> ";
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
        case MemoryFieldType::EntityDBID:
        {
            uint32_t value = Script::Memory::ReadDword(memoryOffset);
            itemValue->setData(QString::asprintf("%lu (%s)", value, mToolbar->entityDB()->entityList()->nameForID(value).c_str()), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("0x%08lX", value), Qt::DisplayRole);
            break;
        }
        case MemoryFieldType::EntityPointer:
        {
            size_t value = Script::Memory::ReadQword(memoryOffset);
            auto entityName = getEntityName(value, mToolbar->entityDB());
            itemValue->setData(QString::asprintf("<font color='blue'><u>%s</u></font>", entityName.c_str()), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
            break;
        }
        case MemoryFieldType::EntityDBPointer:
        {
            size_t value = Script::Memory::ReadQword(memoryOffset);
            auto id = Script::Memory::ReadDword(value + 20);
            itemValue->setData(QString::asprintf("<font color='blue'><u>Entity DB id %d</u></font>", id), Qt::DisplayRole);
            itemValueHex->setData(QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", value), Qt::DisplayRole);
            itemValue->setData(value, gsRoleRawValue);
            itemValueHex->setData(value, gsRoleRawValue);
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
        case MemoryFieldType::Vector:
        {
            for (const auto& f : gsVectorFields)
            {
                updateValueForField(f, fieldNameOverride + "." + f.name, offsets, itemField);
            }
            break;
        }
        case MemoryFieldType::Color:
        {
            for (const auto& f : gsColorFields)
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
            auto dataType = clickedItem->data(gsRoleType).value<MemoryFieldType>();
            switch (dataType)
            {
                case MemoryFieldType::CodePointer:
                {
                    GuiDisasmAt(clickedItem->data(gsRoleRawValue).toULongLong(), GetContextData(UE_CIP));
                    GuiShowCpu();
                    break;
                }
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
                case MemoryFieldType::EntityDBPointer:
                {
                    auto offset = clickedItem->data(gsRoleRawValue).toULongLong();
                    if (offset != 0)
                    {
                        auto id = Script::Memory::ReadDword(offset + 20);
                        auto view = mToolbar->showEntityDB();
                        view->showIndex(id);
                    }
                    break;
                }
            }
        }
    }
}

void TreeViewMemoryFields::clear()
{
    mSavedColumnWidths[gsColField] = columnWidth(gsColField);
    mSavedColumnWidths[gsColValue] = columnWidth(gsColValue);
    mSavedColumnWidths[gsColValueHex] = columnWidth(gsColValueHex);
    mSavedColumnWidths[gsColMemoryOffset] = columnWidth(gsColMemoryOffset);
    mSavedColumnWidths[gsColType] = columnWidth(gsColType);
    mModel->clear();
}
