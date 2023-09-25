#pragma once

#include "Data/MemoryMappedData.h"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct Configuration;

    class State : public MemoryMappedData
    {
      public:
        explicit State(Configuration* config);
        bool loadState();
        void loadThreadSpecificState(size_t offset);

        uint32_t heapOffset();
        uint32_t TEBOffset() const;

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;

        size_t findNextEntity(size_t entityOffset);

        void reset();

      private:
        size_t mStatePtr = 0;
        uint32_t mHeapOffset = 0;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
