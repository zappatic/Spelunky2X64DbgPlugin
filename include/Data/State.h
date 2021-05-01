#pragma once

#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class State : MemoryMappedData
    {
      public:
        explicit State(Configuration* config);
        bool loadState();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;

        size_t findNextEntity(size_t entityOffset);

      private:
        size_t mStatePtr = 0;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin