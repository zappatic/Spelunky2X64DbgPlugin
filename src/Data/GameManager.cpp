#include "Data/GameManager.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"

S2Plugin::GameManager::GameManager(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::GameManager::loadGameManager()
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mGameManagerPtr != 0)
    {
        return true;
    }

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "C6 80 39 01 00 00 00 48");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 10);
    auto offsetPtr = instructionOffset + pcOffset + 14;
    mGameManagerPtr = Script::Memory::ReadQword(offsetPtr);
    auto heapOffsetSaveGame = Script::Memory::ReadQword(Script::Memory::ReadQword(mGameManagerPtr + 8));

    THREADLIST threadList;
    DbgGetThreadList(&threadList);
    for (auto x = 0; x < threadList.count; ++x)
    {
        auto threadAllInfo = threadList.list[x];
        if (strncmp(threadAllInfo.BasicInfo.threadName, "Main Thread", 11) == 0 || strncmp(threadAllInfo.BasicInfo.threadName, "MainThrd", 8) == 0)
        {
            auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
            auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(size_t)));
            auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
            auto heapBase = Script::Memory::ReadQword(tebAddress11Value + 0x120);
            mSaveGamePtr = heapBase + heapOffsetSaveGame;
            break;
        }
    }

    refreshOffsets();
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::GameManager::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::GameManager::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mGameManagerPtr;
    for (const auto& field : mConfiguration->typeFields(MemoryFieldType::GameManager))
    {
        offset = setOffsetForField(field, "GameManager." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::GameManager::offsetForField(const std::string& fieldName) const
{
    auto full = "GameManager." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::GameManager::reset()
{
    mGameManagerPtr = 0;
    mMemoryOffsets.clear();
}

size_t S2Plugin::GameManager::saveGameOffset()
{
    loadGameManager();
    return mSaveGamePtr;
}
