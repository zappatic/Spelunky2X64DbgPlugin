#pragma once

#include "Configuration.h"
#include "Spelunky2.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    class MemoryMappedData
    {
      public:
        explicit MemoryMappedData(Configuration* config);

      protected:
        Configuration* mConfiguration;

        size_t setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets, bool advanceOffset = true);
        size_t updateOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets);
    };
} // namespace S2Plugin