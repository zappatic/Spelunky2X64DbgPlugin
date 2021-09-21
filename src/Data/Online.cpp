#include "Data/Online.h"
#include "pluginmain.h"

S2Plugin::Online::Online(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::Online::loadOnline()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mOnlinePtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "4C 8B 35 FA 61 35 00");
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
