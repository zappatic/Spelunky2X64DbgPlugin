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
    instructionOffset = Script::Pattern::FindMem(instructionOffset - 25, afterBundleSize, "48 8B");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 3);
    auto heapOffsetPtr = instructionOffset + pcOffset + 7;
    mHeapOffset = Script::Memory::ReadDword(heapOffsetPtr);

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
            auto heapBase = Script::Memory::ReadQword(tebAddress11Value + TEBOffset());
            mStatePtr = heapBase + mHeapOffset;
            break;
        }
    }

    refreshOffsets();
    return true;
}

void S2Plugin::State::loadThreadSpecificState(size_t offset)
{
    mStatePtr = offset;
    refreshOffsets();
}

uint32_t S2Plugin::State::heapOffset()
{
    loadState();
    return mHeapOffset;
}

uint32_t S2Plugin::State::TEBOffset() const
{
    return 0x190;
}

std::unordered_map<std::string, size_t>& S2Plugin::State::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::State::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mStatePtr;
    for (const auto& field : mConfiguration->typeFields(MemoryFieldType::State))
    {
        offset = setOffsetForField(field, "State." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::State::offsetForField(const std::string& fieldName) const
{
    auto full = "State." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

size_t S2Plugin::State::findNextEntity(size_t entityOffset)
{
    size_t nextOffset = (std::numeric_limits<size_t>::max)();

    auto loopEntities = [&nextOffset, entityOffset](size_t entities, uint32_t entityCount)
    {
        for (auto x = 0; x < (std::min)(10000u, entityCount); ++x)
        {
            auto entityPtr = Script::Memory::ReadQword(entities + (x * sizeof(size_t)));
            if (entityPtr <= entityOffset)
            {
                continue;
            }
            if (entityPtr < nextOffset)
            {
                nextOffset = entityPtr;
            }
        }
    };

    auto layer0Entities = Script::Memory::ReadQword(offsetForField("layer0.first_entity*"));
    auto layer0EntityCount = Script::Memory::ReadDword(offsetForField("layer0.size"));
    loopEntities(layer0Entities, layer0EntityCount);

    auto layer1Entities = Script::Memory::ReadQword(offsetForField("layer1.first_entity*"));
    auto layer1EntityCount = Script::Memory::ReadDword(offsetForField("layer1.size"));
    loopEntities(layer1Entities, layer1EntityCount);

    if (nextOffset == (std::numeric_limits<size_t>::max)())
    {
        return 0;
    }
    return nextOffset;
}

void S2Plugin::State::reset()
{
    mStatePtr = 0;
    mMemoryOffsets.clear();
}
