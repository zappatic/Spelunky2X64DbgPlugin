#include "Data/Entity.h"
#include "pluginmain.h"

Entity::Entity(size_t offset)
{
    mEntityPtr = offset;
    refreshOffsets();
}

const std::unordered_map<std::string, size_t>& Entity::offsets()
{
    return mMemoryOffsets;
}

void Entity::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mEntityPtr;
    for (const auto& field : gsEntityFields)
    {
        offset = setOffsetForField(field, field.name, offset, mMemoryOffsets);
    }
}