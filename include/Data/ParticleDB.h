#pragma once

#include <cstdint>

namespace S2Plugin
{
    class ParticleDB
    {
      public:
        uintptr_t offsetForIndex(uint32_t particleDBIndex) const;
        bool isValid() const
        {
            return (ptr != 0);
        }

      private:
        uintptr_t ptr{0};

        friend struct Spelunky2;
    };
} // namespace S2Plugin
