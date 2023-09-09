#pragma once

#include "Data/MemoryMappedData.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct Configuration;
    struct ParticleEmittersList;

    class ParticleDB : public MemoryMappedData
    {
      public:
        explicit ParticleDB(Configuration* config);
        bool loadParticleDB();
        ParticleEmittersList* particleEmittersList() const noexcept;

        std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t particleDBIndex);

        void reset();

      private:
        size_t mParticleDBPtr = 0;
        std::unique_ptr<ParticleEmittersList> mParticleEmittersList;
        std::unordered_map<uint16_t, std::unordered_map<std::string, size_t>> mMemoryOffsets; // map of particleDBID -> ( fieldname -> offset ) of field value in memory
    };
} // namespace S2Plugin
