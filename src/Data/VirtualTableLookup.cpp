#include "Data/VirtualTableLookup.h"

std::unordered_set<uint32_t> S2Plugin::VirtualTableLookup::tableOffsetForFunctionAddress(size_t functionAddress) const
{
    std::unordered_set<uint32_t> offsets;
    for (const auto& [tableOffset, tableEntry] : mOffsetToTableEntries)
    {
        if (tableEntry.value == functionAddress)
        {
            offsets.insert(tableOffset);
        }
    }
    return offsets;
}

size_t S2Plugin::VirtualTableLookup::tableAddressForEntry(const VirtualTableEntry& entry) const
{
    return mTableStartAddress + (entry.offset * sizeof(size_t));
}

void S2Plugin::VirtualTableLookup::setSymbolNameForOffsetAddress(size_t offsetAddress, const std::string& name) const
{
    auto tableOffset = (offsetAddress - mTableStartAddress) / sizeof(size_t);
    auto& entry = mOffsetToTableEntries.at(tableOffset);
    const_cast<VirtualTableEntry&>(entry).addSymbol(name);
}

S2Plugin::VirtualTableEntry S2Plugin::VirtualTableLookup::findPrecedingEntryWithSymbols(size_t tableOffset) const
{
    size_t counter = tableOffset;
    while (counter > 0)
    {
        const auto& entry = mOffsetToTableEntries.at(counter);
        if (!entry.isAutoSymbol && entry.isValidAddress && entry.symbols.size() > 0)
        {
            return entry;
        }
        counter--;
    }

    VirtualTableEntry dummy;
    return dummy;
}
