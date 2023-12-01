#include "Data/Online.h"
#include "Configuration.h"
#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <cstdint>

S2Plugin::Online::Online(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::Online::loadOnline()
{
    auto spel2 = Spelunky2::get();
    auto afterBundle = spel2->afterBundle;
    auto afterBundleSize = spel2->afterBundleSize;
    if (afterBundle == 0)
    {
        return false;
    }
    if (mOnlinePtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8B 05 ?? ?? ?? ?? 80 B8 00 02 00 00 FF");
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mOnlinePtr = Script::Memory::ReadQword(instructionOffset + 7 + relativeOffset);
    refreshOffsets();
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::Online::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::Online::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mOnlinePtr;
    for (const auto& field : mConfiguration->typeFields(MemoryFieldType::Online))
    {
        offset = setOffsetForField(field, "Online." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::Online::offsetForField(const std::string& fieldName) const
{
    auto full = "Online." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::Online::reset()
{
    mOnlinePtr = 0;
    mMemoryOffsets.clear();
}
