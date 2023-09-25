#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct Configuration;
    struct MemoryField;

    class MemoryMappedData
    {
      public:
        explicit MemoryMappedData(Configuration* config);
        virtual ~MemoryMappedData() = default;

        size_t setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets, bool advanceOffset = true);

        size_t sizeOf(const std::string& typeName);

      protected:
        Configuration* mConfiguration;
    };
} // namespace S2Plugin
