#pragma once

#include "Configuration.h"
#include "Spelunky2.h"
#include <QString>

namespace S2Plugin
{
    struct StringEntry
    {
        uint32_t id;
        size_t memoryOffset;
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
