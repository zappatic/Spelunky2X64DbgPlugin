#include "Data/ParticleDB.h"
#include "Data/ParticleEmittersList.h"
#include "pluginmain.h"

S2Plugin::ParticleDB::ParticleDB(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::ParticleDB::loadParticleDB()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mParticleDBPtr != 0)
    {
        return true;
    }
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();

    mParticleEmittersList = std::make_unique<ParticleEmittersList>(mConfiguration->spelunky2());

    mMemoryOffsets.clear();

    // Spelunky 1.20.4d, 1.23.1b: last id = 0xDB 219
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "FE FF FF FF 66 C7 05");
    mParticleDBPtr = instructionOffset + 13 + Script::Memory::ReadDword(instructionOffset + 7);

    auto counter = 0;
    size_t particleDBEntrySize = 0xA0;
    while (counter < 250)
    {
        size_t startOffset = mParticleDBPtr + (counter * particleDBEntrySize);
        uint32_t particleID = Script::Memory::ReadDword(startOffset);
        if (particleID - 1 == counter) // this does not account for gaps in the id's, if there are/will be any
        {
            size_t offset = startOffset;
            std::unordered_map<std::string, size_t> offsets;
            for (const auto& field : mConfiguration->typeFields(MemoryFieldType::ParticleDB))
            {
                offset = setOffsetForField(field, "ParticleDB." + field.name, offset, offsets);
            }
            mMemoryOffsets[particleID] = offsets;
        }
        else
        {
            break;
        }
        counter++;
    }
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::ParticleDB::offsetsForIndex(uint32_t particleDBIndex)
{
    if (mMemoryOffsets.count(particleDBIndex) > 0)
    {
        return mMemoryOffsets.at(particleDBIndex);
    }
    static auto empty = std::unordered_map<std::string, size_t>();
    return empty;
}

void S2Plugin::ParticleDB::reset()
{
    mParticleDBPtr = 0;
}

S2Plugin::ParticleEmittersList* S2Plugin::ParticleDB::particleEmittersList() const noexcept
{
    return mParticleEmittersList.get();
}
