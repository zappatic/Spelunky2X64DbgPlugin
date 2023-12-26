#include "Data/ParticleDB.h"
#include "Configuration.h"
#include "Data/ParticleEmittersList.h"
#include "Spelunky2.h"
#include "pluginmain.h"

bool S2Plugin::ParticleDB::loadParticleDB()
{
    auto spel2 = Spelunky2::get();
    const auto afterBundle = spel2->afterBundle;
    const auto afterBundleSize = spel2->afterBundleSize;
    if (afterBundle == 0)
    {
        return false;
    }
    if (mParticleDBPtr != 0)
    {
        return true;
    }

    mParticleEmittersList = std::make_unique<ParticleEmittersList>();

    mMemoryOffsets.clear();

    // Spelunky 1.20.4d, 1.23.1b: last id = 0xDB 219
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "FE FF FF FF 66 C7 05");
    mParticleDBPtr = instructionOffset + 13 + (duint)Script::Memory::ReadDword(instructionOffset + 7);

    auto counter = 0;
    size_t particleDBEntrySize = 0xA0;
    auto config = Configuration::get();
    while (counter < 250)
    {
        size_t startOffset = mParticleDBPtr + (counter * particleDBEntrySize);
        uint32_t particleID = Script::Memory::ReadDword(startOffset);
        if (particleID - 1 == counter) // this does not account for gaps in the id's, if there are/will be any
        {
            size_t offset = startOffset;
            std::unordered_map<std::string, size_t> offsets;
            for (const auto& field : config->typeFields(MemoryFieldType::ParticleDB))
            {
                offset = config->setOffsetForField(field, "ParticleDB." + field.name, offset, offsets);
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
