#include "Data/State.h"
#include "pluginmain.h"

S2Plugin::State::State(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::State::loadState()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mStatePtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "49 0F 44 C0");
    instructionOffset = Script::Pattern::FindMem(instructionOffset + 1, afterBundleSize, "49 0F 44 C0");
    instructionOffset = Script::Pattern::FindMem(instructionOffset - 0x10, afterBundleSize, "48 8B");
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
    return true;
}

const std::unordered_map<std::string, size_t>& S2Plugin::State::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::State::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mStatePtr;
    for (const auto& field : mConfiguration->typeFields(MemoryFieldType::ClassState))
    {
        offset = setOffsetForField(field, "ClassState." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::State::offsetForField(const std::string& fieldName) const
{
    auto full = "ClassState." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}