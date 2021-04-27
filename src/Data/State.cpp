#include "Data/State.h"
#include "pluginmain.h"

void State::loadState()
{
    auto afterBundle = spelunky2AfterBundle();
    if (afterBundle == 0 || mStatePtr != 0)
    {
        return;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, spelunky2AfterBundleSize(), "49 0F 44 C0");
    instructionOffset = Script::Pattern::FindMem(instructionOffset + 1, spelunky2AfterBundleSize(), "49 0F 44 C0");
    instructionOffset = Script::Pattern::FindMem(instructionOffset - 0x10, spelunky2AfterBundleSize(), "48 8B");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 3);
    auto heapOffsetPtr = instructionOffset + pcOffset + 7;
    auto heapOffset = Script::Memory::ReadDword(heapOffsetPtr);

    THREADLIST threadList;
    DbgGetThreadList(&threadList);
    for (auto x = 0; x < threadList.count; ++x)
    {
        auto threadAllInfo = threadList.list[x];
        if (strncmp(threadAllInfo.BasicInfo.threadName, "Main Thread", 11) == 0)
        {
            auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
            auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(size_t)));
            auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
            auto heapBase = Script::Memory::ReadQword(tebAddress11Value + 0x130);
            mStatePtr = heapBase + heapOffset;
            break;
        }
    }

    refreshOffsets();
}

const std::unordered_map<std::string, size_t>& State::offsets()
{
    return mMemoryOffsets;
}

void State::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mStatePtr;
    for (const auto& field : gsStateFields)
    {
        offset = setOffsetForField(field, field.name, offset, mMemoryOffsets);
    }
}

size_t State::offsetForField(const std::string& fieldName) const
{
    if (mMemoryOffsets.count(fieldName) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(fieldName);
}