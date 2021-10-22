#include "Data/VirtualTableLookup.h"
#include "pluginmain.h"

static constexpr size_t gsAmountOfPointers = 53200;

void S2Plugin::VirtualTableEntry::addSymbol(const std::string& s)
{
    symbols.insert(s);
}

S2Plugin::VirtualTableLookup::VirtualTableLookup(Configuration* configuration) : mConfiguration(configuration) {}

bool S2Plugin::VirtualTableLookup::loadTable()
{
    if (mOffsetToTableEntries.size() != 0)
    {
        return true;
    }
    mOffsetToTableEntries.reserve(gsAmountOfPointers);

    // From 1.23.2 on, the base isn't on D3Dcompile any more, so just look up the first pointer by pattern
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0)
    {
        return false;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8D 0D ?? ?? ?? ?? 48 89 0D ?? ?? ?? ?? 48 C7 05");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mTableStartAddress = instructionOffset + pcOffset + 7;

    // import the pointers
    size_t buffer[gsAmountOfPointers] = {0};
    Script::Memory::Read(mTableStartAddress, buffer, gsAmountOfPointers * sizeof(size_t), nullptr);
    for (auto x = 0; x < gsAmountOfPointers; ++x)
    {
        size_t pointer = buffer[x];
        VirtualTableEntry e;
        e.isValidAddress = Script::Memory::IsValidPtr(pointer);
        e.offset = x;
        e.value = pointer;
        mOffsetToTableEntries[x] = e;
    }
    return true;
}

void S2Plugin::VirtualTableLookup::reset()
{
    mOffsetToTableEntries.clear();
    mTableStartAddress = 0;
}

const S2Plugin::VirtualTableEntry& S2Plugin::VirtualTableLookup::entryForOffset(size_t tableOffset)
{
    return mOffsetToTableEntries.at(tableOffset);
}

std::unordered_set<uint32_t> S2Plugin::VirtualTableLookup::tableOffsetForFunctionAddress(size_t functionAddress)
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

size_t S2Plugin::VirtualTableLookup::count() const noexcept
{
    return gsAmountOfPointers;
}

size_t S2Plugin::VirtualTableLookup::tableAddressForEntry(const VirtualTableEntry& entry)
{
    return mTableStartAddress + (entry.offset * sizeof(size_t));
}

void S2Plugin::VirtualTableLookup::setSymbolNameForOffsetAddress(size_t offsetAddress, const std::string& name)
{
    auto tableOffset = (offsetAddress - mTableStartAddress) / sizeof(size_t);
    mOffsetToTableEntries.at(tableOffset).addSymbol(name);
}

S2Plugin::VirtualTableEntry S2Plugin::VirtualTableLookup::findPrecedingEntryWithSymbols(size_t tableOffset)
{
    size_t counter = tableOffset;
    while (counter > 0)
    {
        auto entry = mOffsetToTableEntries.at(counter);
        if (!entry.isAutoSymbol && entry.isValidAddress && entry.symbols.size() > 0)
        {
            return entry;
        }
        counter--;
    }

    VirtualTableEntry dummy;
    return dummy;
}
