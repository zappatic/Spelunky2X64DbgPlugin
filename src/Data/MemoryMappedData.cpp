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
        case MemoryFieldType::Flag:
            break;
        case MemoryFieldType::Skip:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
            offset += field.extraInfo;
            break;
        case MemoryFieldType::Bool:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        case MemoryFieldType::State8:
        case MemoryFieldType::CharacterDBID:
            offset += 1;
            break;
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::State16:
        case MemoryFieldType::UTF16Char:
            offset += 2;
            break;
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Flags32:
        case MemoryFieldType::State32:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::IPv4Address:
            offset += 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer:          // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::TextureDBPointer:         // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::EntityPointer:            // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::EntityUIDPointer:         // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::ParticleDBPointer:        // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenPointer:          // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenRoomsPointer:     // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenRoomsMetaPointer: // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::JournalPagePointer:       // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::ThemeInfoName:            // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::VirtualFunctionTable:
            offset += 8;
            break;
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : mConfiguration->typeFieldsOfPointer("ThemeInfoPointer"))
            {
                auto newOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, pointerOffset, offsets);
                if (pointerOffset != 0)
                {
                    pointerOffset = newOffset;
                }
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::PointerType:
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : mConfiguration->typeFieldsOfPointer(field.jsonName))
            {
                auto newOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, pointerOffset, offsets);
                if (pointerOffset != 0)
                {
                    pointerOffset = newOffset;
                }
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::InlineStructType:
        {
            for (const auto& f : mConfiguration->typeFieldsOfInlineStruct(field.jsonName))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            for (const auto& f : mConfiguration->typeFieldsOfEntitySubclass(field.jsonName))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        default:
        {
            for (const auto& f : mConfiguration->typeFields(field.type))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
    }
    return offset;
}

size_t S2Plugin::MemoryMappedData::sizeOf(const std::string& typeName)
{
    if (mConfiguration->isPointer(typeName))
    {
        return sizeof(size_t);
    }
    else if (mConfiguration->isBuiltInType(typeName))
    {
        MemoryField tmp;
        tmp.type = gsJSONStringToMemoryFieldTypeMapping.at(typeName);
        std::unordered_map<std::string, size_t> offsetsDummy;
        return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
    }
    else if (mConfiguration->isInlineStruct(typeName))
    {
        MemoryField tmp;
        tmp.type = MemoryFieldType::InlineStructType;
        tmp.jsonName = typeName;
        std::unordered_map<std::string, size_t> offsetsDummy;
        return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
    }
    else
    {
        return 0;
    }
}

uint8_t S2Plugin::MemoryMappedData::alignmentOf(const std::string& typeName)
{
    bool check_aligment = false;
    if (mConfiguration->isPointer(typeName))
    {
        return sizeof(size_t);
    }
    else if (mConfiguration->isBuiltInType(typeName))
    {
        switch (gsJSONStringToMemoryFieldTypeMapping.at(typeName))
        {
                /*case MemoryFieldType::EntitySubclass:
                case MemoryFieldType::Flag:*/

            case MemoryFieldType::Skip:
            {
                dprintf("Cannot determinate alignment of \"Skip\" element!\n");
                return 0;
            }
            case MemoryFieldType::Byte:
            case MemoryFieldType::UnsignedByte:
            case MemoryFieldType::Bool:
            case MemoryFieldType::Flags8:
            case MemoryFieldType::State8:
            case MemoryFieldType::CharacterDBID:
            case MemoryFieldType::UTF8StringFixedSize:
                return sizeof(char);
            case MemoryFieldType::Word:
            case MemoryFieldType::UnsignedWord:
            case MemoryFieldType::State16:
            case MemoryFieldType::Flags16:
            case MemoryFieldType::UTF16StringFixedSize:
            case MemoryFieldType::UTF16Char:
                return sizeof(int16_t);
            case MemoryFieldType::Dword:
            case MemoryFieldType::UnsignedDword:
            case MemoryFieldType::Float:
            case MemoryFieldType::Flags32:
            case MemoryFieldType::State32:
            case MemoryFieldType::EntityDBID:
            case MemoryFieldType::ParticleDBID:
            case MemoryFieldType::EntityUID:
            case MemoryFieldType::TextureDBID:
            case MemoryFieldType::StringsTableID:
            case MemoryFieldType::IPv4Address:
            case MemoryFieldType::CharacterDB: // biggest variable is 4
                return sizeof(int32_t);

            case MemoryFieldType::Online:
            case MemoryFieldType::TextureDB:
            case MemoryFieldType::ParticleDB:
            case MemoryFieldType::EntityDB:
            case MemoryFieldType::LevelGen:
            case MemoryFieldType::GameManager:
            case MemoryFieldType::State:
            case MemoryFieldType::SaveGame:
            case MemoryFieldType::PointerType:
            case MemoryFieldType::ThemeInfoName:
            case MemoryFieldType::UndeterminedThemeInfoPointer:
            case MemoryFieldType::LevelGenRoomsPointer:
            case MemoryFieldType::LevelGenRoomsMetaPointer:
            case MemoryFieldType::JournalPagePointer:
            case MemoryFieldType::LevelGenPointer:
            case MemoryFieldType::VirtualFunctionTable:
            case MemoryFieldType::EntityPointer:
            case MemoryFieldType::EntityUIDPointer:
            case MemoryFieldType::EntityDBPointer:
            case MemoryFieldType::ParticleDBPointer:
            case MemoryFieldType::TextureDBPointer:
            case MemoryFieldType::ConstCharPointer:
            case MemoryFieldType::ConstCharPointerPointer:
            case MemoryFieldType::Vector:
            case MemoryFieldType::StdVector:
            case MemoryFieldType::StdMap:
            case MemoryFieldType::StdSet:
            case MemoryFieldType::CodePointer:
            case MemoryFieldType::DataPointer:
            case MemoryFieldType::Qword:
            case MemoryFieldType::UnsignedQword:
                return sizeof(size_t);
            case MemoryFieldType::InlineStructType:
            default:
            {
                check_aligment = true;
            }
        }
    }
    if (check_aligment || mConfiguration->isInlineStruct(typeName))
    {

        // get alignment
        dprintf("Alignment not found for '%s'\n", typeName.c_str());
    }

    return 0;
}
