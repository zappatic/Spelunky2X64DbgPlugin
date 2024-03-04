#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace S2Plugin
{
    struct VirtualTableEntry
    {
        size_t value;
        size_t offset; // offset in table
        bool isValidAddress;

        bool isAutoSymbol = false; // whether the symbol name was added from x64dbg
        std::unordered_set<std::string> symbols;

        void addSymbol(const std::string& s)
        {
            symbols.insert(s);
        }
    };

    class VirtualTableLookup
    {
      public:
        const VirtualTableEntry& entryForOffset(size_t tableOffset) const
        {
            return mOffsetToTableEntries.at(tableOffset);
        }
        std::unordered_set<size_t> tableOffsetForFunctionAddress(size_t functionAddress) const;
        VirtualTableEntry findPrecedingEntryWithSymbols(size_t tableOffset) const;
        size_t tableAddressForEntry(const VirtualTableEntry& entry) const;

        void setSymbolNameForOffsetAddress(size_t offsetAddress, const std::string& name) const;

        constexpr size_t count() const noexcept
        {
            constexpr size_t gsAmountOfPointers = 53243;
            return gsAmountOfPointers;
        }
        size_t tableStartAddress() const noexcept
        {
            return mTableStartAddress;
        }
        bool isValid() const
        {
            return (mTableStartAddress != 0);
        }

      private:
        std::unordered_map<size_t, VirtualTableEntry> mOffsetToTableEntries;
        uintptr_t mTableStartAddress{0};

        VirtualTableLookup() = default;
        ~VirtualTableLookup(){};
        VirtualTableLookup(const VirtualTableLookup&) = delete;
        VirtualTableLookup& operator=(const VirtualTableLookup&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
