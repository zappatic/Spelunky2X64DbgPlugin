#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct State
    {
        State(uintptr_t addr) : mStatePtr(addr){};

        uintptr_t ptr() const
        {
            return mStatePtr;
        }
        uintptr_t findEntitybyUID(uint32_t uid) const;

      private:
        uintptr_t mStatePtr = 0;
    };
} // namespace S2Plugin
