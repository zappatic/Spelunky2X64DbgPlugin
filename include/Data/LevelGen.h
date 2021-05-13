#pragma once

#include "Data/MemoryMappedData.h"
#include "Data/State.h"

namespace S2Plugin
{
    class LevelGen : public MemoryMappedData
    {
      public:
        LevelGen(Configuration* config, State* state);
        void setState(State* state);
        bool loadLevelGen();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;
        std::string themeNameOfOffset(size_t offset);

        void reset();

      private:
        size_t mLevelGenPtr = 0;
        State* mState;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin