#include "Data/MemoryMappedData.h"
#include "pluginmain.h"

size_t MemoryMappedData::setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets)
{
    offsets[fieldNameOverride] = offset;

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
            offset += 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer:
        case MemoryFieldType::EntityPointer:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
            offset += 8;
            break;
        case MemoryFieldType::Rect:
        {
            for (const auto& f : gsRectFields)
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::StateSaturationVignette:
        {
            for (const auto& f : gsStateSaturationVignetteFields)
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::StateIllumination:
        {
            auto illuOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : gsStateIlluminationFields)
            {
                illuOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, illuOffset, offsets);
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::StateItems:
        {
            auto itemsOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : gsStateItemsFields)
            {
                itemsOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, itemsOffset, offsets);
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::Layer:
        {
            auto layersOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : gsLayerFields)
            {
                layersOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, layersOffset, offsets);
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::Vector:
        {
            for (const auto& f : gsVectorFields)
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::Color:
        {
            for (const auto& f : gsColorFields)
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::TexturePointer:
        {
            auto textureOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : gsTextureFields)
            {
                textureOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, textureOffset, offsets);
            }
            offset += 8;
            break;
        }
    }
    return offset;
}
