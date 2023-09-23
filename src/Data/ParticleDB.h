#pragma once

#include "Configuration.h"
#include "Data/MemoryMappedData.h"
#include "Data/ParticleEmittersList.h"
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
        ParticleEmittersList* particleEmittersList() const noexcept;

        std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t particleDBIndex);

        void reset();

      private:
        size_t mParticleDBPtr = 0;
        std::unique_ptr<ParticleEmittersList> mParticleEmittersList;
        std::unordered_map<uint16_t, std::unordered_map<std::string, size_t>> mMemoryOffsets; // map of particleDBID -> ( fieldname -> offset ) of field value in memory
    };
} // namespace S2Plugin
