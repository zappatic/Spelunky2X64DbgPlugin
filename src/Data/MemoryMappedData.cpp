#include "Data/MemoryMappedData.h"
#include "pluginmain.h"

S2Plugin::MemoryMappedData::MemoryMappedData(Configuration* config) : mConfiguration(config) {}

size_t S2Plugin::MemoryMappedData::setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets,
                                                     bool advanceOffset)
{
    offsets[fieldNameOverride] = offset;
    if (!advanceOffset)
    {
        return offset;
    }

    switch (field.type)
    {
        case MemoryFieldType::Skip:
            offset += field.extraInfo;
            break;
        case MemoryFieldType::Bool:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
            offset += 1;
            break;
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
            offset += 2;
            break;
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Flags32:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
            offset += 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer: // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::EntityPointer:   // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
            offset += 8;
            break;
        default: // it's either a pointer or an inline struct
        {
            if (gsPointerTypes.count(field.type) > 0)
            {
                auto pointerOffset = Script::Memory::ReadQword(offset);
                for (const auto& f : mConfiguration->typeFields(field.type))
                {
                    pointerOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, pointerOffset, offsets);
                }
                offset += 8;
            }
            else
            {
                for (const auto& f : mConfiguration->typeFields(field.type))
                {
                    offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
                }
            }
            break;
        }
    }
    return offset;
}
