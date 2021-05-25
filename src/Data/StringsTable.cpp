#include "Data/StringsTable.h"
#include "pluginmain.h"

S2Plugin::StringsTable::StringsTable(Configuration* config) : mConfiguration(config) {}

bool S2Plugin::StringsTable::loadStringsTable()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mStringsTablePtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "9C 00 00 00 48 8D 05");
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 7);
    mStringsTablePtr = instructionOffset + 11 + relativeOffset;

    mStringEntries.clear();

    for (auto stringIndex = 0; stringIndex < 5000; ++stringIndex)
    {
        size_t stringPointer = Script::Memory::ReadQword(mStringsTablePtr + (stringIndex * sizeof(size_t)));
        if (!Script::Memory::IsValidPtr(stringPointer))
        {
            break;
        }
        uint16_t buffer[2048] = {0};
        uint16_t charBuffer = 0;
        size_t charCounter = 0;
        do
        {
            charBuffer = Script::Memory::ReadWord(stringPointer + (charCounter * sizeof(uint16_t)));
            buffer[charCounter++] = charBuffer;
        } while (charBuffer != 0);

        StringEntry e;
        e.id = stringIndex;
        e.memoryOffset = stringPointer;
        e.str = QString::fromUtf16(buffer);
        mStringEntries[e.id] = e;
    }
    return true;
}

void S2Plugin::StringsTable::reset()
{
    mStringsTablePtr = 0;
    mStringEntries.clear();
}

const std::unordered_map<uint32_t, S2Plugin::StringEntry>& S2Plugin::StringsTable::entries()
{
    return mStringEntries;
}

S2Plugin::StringEntry S2Plugin::StringsTable::entryForID(uint32_t id)
{
    return mStringEntries.at(id);
}

QString S2Plugin::StringsTable::nameForID(uint32_t id)
{
    if (mStringEntries.count(id) == 0)
    {
        return QString("INVALID OR NOT APPLICABLE");
    }
    return mStringEntries.at(id).str;
}
