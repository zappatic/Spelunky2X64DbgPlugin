#include "Data/EntityDB.h"
#include "pluginmain.h"

void EntityDB::loadEntityDB()
{
    auto afterBundle = spelunky2AfterBundle();
    if (afterBundle == 0 || mEntityDBPtr != 0)
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
            offset = setOffsetForField(field, field.name, offset, offsets);
        }
        mMemoryOffsets.emplace_back(offsets);
    }
}

EntityList* EntityDB::entityList() const noexcept
{
    return mEntityList.get();
}

const std::unordered_map<std::string, size_t>& EntityDB::offsetsForIndex(uint32_t entityDBIndex)
{
    return mMemoryOffsets.at(entityDBIndex);
}
