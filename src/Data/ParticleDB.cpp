#include "Data/ParticleDB.h"
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

    mMemoryOffsets.clear();
    mParticleNames.clear();

    // Spelunky 1.20.4d last id = 0xDB 219
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "B8 01 00 00 00 66 89 05");
    instructionOffset = Script::Pattern::FindMem(instructionOffset + 1, afterBundleSize - (instructionOffset - afterBundle), "B8 01 00 00 00 66 89 05");
    mParticleDBPtr = instructionOffset + 12 + Script::Memory::ReadDword(instructionOffset + 8);

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

            // store the name
            auto sheetIndex = Script::Memory::ReadDword(offsets.at("ParticleDB.texture_id"));
            auto nameOffset = Script::Memory::ReadQword(Script::Memory::ReadQword(offsets.at("ParticleDB.texture.name")));
            const char buffer[1000] = {0};
            Script::Memory::Read(nameOffset, (void*)buffer, 1000, nullptr); // it's null terminated in the executable, so doesn't matter that we overshoot
            mParticleNames[particleID] = std::string(buffer) + "[" + std::to_string(sheetIndex) + "]";
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

size_t S2Plugin::ParticleDB::amountOfParticles() const noexcept
{
    return mMemoryOffsets.size();
}

std::string S2Plugin::ParticleDB::nameForIndex(uint32_t particleDBIndex)
{
    if (mParticleNames.count(particleDBIndex) > 0)
    {
        return mParticleNames.at(particleDBIndex);
    }
    return "UNKNOWN PARTICLEDB ID";
}

const std::unordered_map<uint16_t, std::string>& S2Plugin::ParticleDB::particleNames()
{
    return mParticleNames;
}
