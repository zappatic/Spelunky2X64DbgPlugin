#pragma once

#include <QString>
#include <cstdint>
#include <unordered_map>

namespace S2Plugin
{
    struct Configuration;

    struct StringEntry
    {
        uint32_t id;
        size_t stringTableOffset; // pointer into the strings table
        size_t memoryOffset;      // pointer to the string itself
        QString str;
    };

    class StringsTable
    {
      public:
        explicit StringsTable(Configuration* config);
        bool loadStringsTable();

        const std::unordered_map<uint32_t, StringEntry>& entries();
        StringEntry entryForID(uint32_t id);
        QString nameForID(uint32_t id);

        void reset();

      private:
        Configuration* mConfiguration;
        size_t mStringsTablePtr = 0;
        std::unordered_map<uint32_t, StringEntry> mStringEntries;
    };
} // namespace S2Plugin
