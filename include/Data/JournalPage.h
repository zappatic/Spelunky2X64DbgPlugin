#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class JournalPage
    {
      public:
        explicit JournalPage(size_t offset, const std::string& pageType);

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;
        void interpretAs(const std::string& classType);

      private:
        size_t mJournalPagePtr = 0;
        std::string mJournalPageType;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
