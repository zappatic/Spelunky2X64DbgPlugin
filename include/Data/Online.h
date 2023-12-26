#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class Online
    {
      public:
        bool loadOnline();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;

        void reset();

      private:
        size_t mOnlinePtr = 0;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
