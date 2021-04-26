#include "Data/EntityDB.h"
#include "pluginmain.h"

void EntityDB::loadEntityDB()
{
    auto afterBundle = spelunky2AfterBundle();
    if (afterBundle == 0)
    {
        return;
    }

    mEntityList = std::make_unique<EntityList>();

    mMemoryOffsets.clear();
    auto instructionEntitiesPtr = Script::Pattern::FindMem(afterBundle, spelunky2AfterBundleSize(), "48 B8 02 55 A7 74 52 9D 51 43");
    auto entitiesPtr = instructionEntitiesPtr + Script::Memory::ReadDword(instructionEntitiesPtr - 4);
    mEntityDBPtr = Script::Memory::ReadQword(entitiesPtr);

    auto offset = mEntityDBPtr;

    for (auto x = 0; x < mEntityList->highestEntityID() + 1; ++x)
    {
        std::unordered_map<std::string, size_t> offsets;
        for (const auto& field : gsEntityDBFields)
        {
            offset = addOffsetForField(field, field.name, offset, offsets);
        }
        mMemoryOffsets.emplace_back(offsets);
    }
}

EntityList* EntityDB::entityList() const noexcept
{
    return mEntityList.get();
}

size_t EntityDB::offsetForField(uint32_t entityDBIndex, const std::string& fieldName)
{
    auto offsets = mMemoryOffsets.at(entityDBIndex);
    return offsets.at(fieldName);
}

const std::unordered_map<std::string, size_t>& EntityDB::offsetsForIndex(uint32_t entityDBIndex)
{
    return mMemoryOffsets.at(entityDBIndex);
}

size_t EntityDB::addOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets)
{
    offsets[fieldNameOverride] = offset;
    switch (field.type)
    {
        case MemoryFieldType::Rect:
            offset = addRectOffsets(field.name, offset, offsets);
            break;
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
            offset += 4;
            break;
        case MemoryFieldType::Pointer:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
            offset += 8;
            break;
    }
    return offset;
}

size_t EntityDB::addRectOffsets(const std::string& rectName, size_t offset, std::unordered_map<std::string, size_t>& offsets)
{
    for (const auto& field : gsRectFields)
    {
        offset = addOffsetForField(field, rectName + "." + field.name, offset, offsets);
    }
    return offset;
}
