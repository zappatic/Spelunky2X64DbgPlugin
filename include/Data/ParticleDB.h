#pragma once

#include "Configuration.h"
#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class ParticleDB : public MemoryMappedData
    {
      public:
        explicit ParticleDB(Configuration* config);
        bool loadParticleDB();
        size_t amountOfParticles() const noexcept;

        std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t particleDBIndex);
        std::string nameForIndex(uint32_t particleDBIndex);
        const std::unordered_map<uint16_t, std::string>& particleNames();

        void reset();

      private:
        size_t mParticleDBPtr = 0;
        std::unordered_map<uint16_t, std::unordered_map<std::string, size_t>> mMemoryOffsets; // map of particleDBID -> ( fieldname -> offset ) of field value in memory
        std::unordered_map<uint16_t, std::string> mParticleNames;
    };
} // namespace S2Plugin
