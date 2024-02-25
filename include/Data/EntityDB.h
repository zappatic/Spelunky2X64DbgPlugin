#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    class EntityDB
    {
      public:
        uintptr_t offsetFromIndex(uint32_t idx) const; // as of right now id == index
        bool isValid() const
        {
            return (ptr != 0);
        }

      private:
        uintptr_t ptr{0};

        friend class Spelunky2;
    };
} // namespace S2Plugin
