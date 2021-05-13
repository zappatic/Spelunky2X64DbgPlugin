#include "Data/LevelGen.h"
#include "Data/State.h"
#include "pluginmain.h"

S2Plugin::LevelGen::LevelGen(Configuration* config, State* state) : MemoryMappedData(config), mState(state) {}

bool S2Plugin::LevelGen::loadLevelGen()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0 || mState == nullptr)
    {
        return false;
    }
    if (mLevelGenPtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "88 41 6A 48 8B 81");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 6);
    mLevelGenPtr = Script::Memory::ReadQword(mState->offsetForField("p00") + pcOffset);

    refreshOffsets();
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::LevelGen::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::LevelGen::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mLevelGenPtr;
    for (const auto& field : mConfiguration->typeFields(MemoryFieldType::LevelGen))
    {
        offset = setOffsetForField(field, "LevelGen." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::LevelGen::offsetForField(const std::string& fieldName) const
{
    auto full = "LevelGen." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::LevelGen::reset()
{
    mLevelGenPtr = 0;
    mMemoryOffsets.clear();
}

std::string S2Plugin::LevelGen::themeNameOfOffset(size_t offset)
{
    if (offset == offsetForField("theme_dwelling.__vftable"))
    {
        return "DWELLING";
    }
    else if (offset == offsetForField("theme_jungle.__vftable"))
    {
        return "JUNGLE";
    }
    else if (offset == offsetForField("theme_volcana.__vftable"))
    {
        return "VOLCANA";
    }
    else if (offset == offsetForField("theme_olmec.__vftable"))
    {
        return "OLMEC";
    }
    else if (offset == offsetForField("theme_tidepool.__vftable"))
    {
        return "TIDE POOL";
    }
    else if (offset == offsetForField("theme_temple.__vftable"))
    {
        return "TEMPLE";
    }
    else if (offset == offsetForField("theme_icecaves.__vftable"))
    {
        return "ICE CAVES";
    }
    else if (offset == offsetForField("theme_neobabylon.__vftable"))
    {
        return "NEO BABYLON";
    }
    else if (offset == offsetForField("theme_sunkencity.__vftable"))
    {
        return "SUNKEN CITY";
    }
    else if (offset == offsetForField("theme_cosmicocean.__vftable"))
    {
        return "COSMIC OCEAN";
    }
    else if (offset == offsetForField("theme_city_of_gold.__vftable"))
    {
        return "CITY OF GOLD";
    }
    else if (offset == offsetForField("theme_duat.__vftable"))
    {
        return "DUAT";
    }
    else if (offset == offsetForField("theme_abzu.__vftable"))
    {
        return "ABZU";
    }
    else if (offset == offsetForField("theme_tiamat.__vftable"))
    {
        return "TIAMAT";
    }
    else if (offset == offsetForField("theme_eggplantworld.__vftable"))
    {
        return "EGGPLANT WORLD";
    }
    else if (offset == offsetForField("theme_hundun.__vftable"))
    {
        return "HUNDUN";
    }
    else if (offset == offsetForField("theme_basecamp.__vftable"))
    {
        return "BASE CAMP";
    }
    else if (offset == offsetForField("theme_arena.__vftable"))
    {
        return "ARENA";
    }
    return "UNKNOWN THEME";
}