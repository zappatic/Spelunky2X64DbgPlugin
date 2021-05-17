#include "Data/VirtualTableLookup.h"
#include "pluginmain.h"

static constexpr size_t gsAmountOfPointers = 70000;

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

    // find the base of the spel2.exe process, so we can add it to the RVA of the first pointer in the table
    size_t base = Script::Module::BaseFromName("spel2.exe");

    // find the address of symbol d3dcompiler_47.D3DCompile, which should be the very first pointer
    // in the giant list, of which we base all relative offsets within the table
    size_t d3dCompileRVA = 0;
    std::unordered_map<size_t, std::string> knownSymbols;
    auto symbolList = BridgeList<Script::Symbol::SymbolInfo>();
    Script::Symbol::GetList(&symbolList);
    for (auto x = 0; x < symbolList.Count(); ++x)
    {
        auto symbolInfo = symbolList[x];
        if (strncmp(symbolInfo.mod, "spel2.exe", 9) == 0)
        {
            if (strncmp(symbolInfo.name, "D3DCompile", 10) == 0)
            {
                d3dCompileRVA = symbolInfo.rva;
            }
            knownSymbols[Script::Memory::ReadQword(base + symbolInfo.rva)] = std::string(symbolInfo.name);
        }
    }
    symbolList.Cleanup();
    mTableStartAddress = base + d3dCompileRVA;

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
        if (knownSymbols.count(pointer) > 0)
        {
            e.addSymbol(knownSymbols.at(pointer));
            e.isAutoSymbol = true;
        }
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
