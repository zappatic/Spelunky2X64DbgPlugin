#pragma once

#include "Configuration.h"

namespace S2Plugin
{
    struct VirtualTableEntry
    {
        size_t value;
        size_t offset; // offset in table
        bool isValidAddress;

        bool isAutoSymbol = false; // whether the symbol name was added from x64dbg
        std::unordered_set<std::string> symbols;

        void addSymbol(const std::string& s);
    };

    class VirtualTableLookup
    {
      public:
        explicit VirtualTableLookup(Configuration* config);
        bool loadTable();

        const VirtualTableEntry& entryForOffset(size_t tableOffset);
        size_t tableAddressForEntry(const VirtualTableEntry& entry);

        void setSymbolNameForOffsetAddress(size_t offsetAddress, const std::string& name);

        size_t count() const noexcept;
        void reset();

      private:
        Configuration* mConfiguration;
        std::unordered_map<uint32_t, VirtualTableEntry> mOffsetToTableEntries;
        size_t mTableStartAddress = 0;
    };
} // namespace S2Plugin
