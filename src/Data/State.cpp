#include "Data/State.h"
#include "Configuration.h"
#include "pluginmain.h"

static uint32_t lowbias32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// Just for refrence
// struct RobinHoodTableEntry
//{
//    uint32_t uid_plus_one;
//    uint32_t padding;
//    Entity* entity;
//};

uintptr_t S2Plugin::State::findEntitybyUID(uint32_t uid) const
{
    // ported from overlunky
    if (uid == ~0)
    {
        return 0;
    }

    static size_t mask_offset = Configuration::get()->offsetForField(MemoryFieldType::State, "uid_to_entity_mask");
    const uint32_t mask = Script::Memory::ReadDword(mStatePtr + mask_offset);
    const uint32_t target_uid_plus_one = lowbias32(uid + 1u);
    uint32_t cur_index = target_uid_plus_one & mask;
    const uintptr_t uid_to_entity_data = Script::Memory::ReadQword(mStatePtr + mask_offset + 0x8u);

    auto getEntry = [uid_to_entity_data](size_t index)
    {
        constexpr size_t robinHoodTableEntrySize = 0x10u;
        return uid_to_entity_data + index * robinHoodTableEntrySize;
    };

    while (true)
    {
        auto entry = getEntry(cur_index);
        auto uid_plus_one = Script::Memory::ReadDword(entry);
        if (uid_plus_one == target_uid_plus_one)
        {
            return Script::Memory::ReadQword(entry + 0x8u);
        }

        if (uid_plus_one == 0)
        {
            return 0;
        }

        if (((cur_index - target_uid_plus_one) & mask) > ((cur_index - uid_plus_one) & mask))
        {
            return 0;
        }

        cur_index = (cur_index + 1u) & mask;
    }
}
