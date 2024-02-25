#pragma once

#include <cstdint>

namespace S2Plugin
{
    struct ParticleEmittersList;

    class ParticleDB
    {
      public:
        uintptr_t offsetForIndex(uint32_t particleDBIndex) const
        {
            if (ptr == 0)
                return 0;

            constexpr size_t particleDBEntrySize = 0xA0;
            return ptr + particleDBIndex * particleDBEntrySize;
        }
        bool isValid() const
        {
            return (ptr != 0);
        }

      private:
        uintptr_t ptr{0};

        friend class Spelunky2;
    };
} // namespace S2Plugin
