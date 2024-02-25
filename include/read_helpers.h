#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <string>

namespace S2Plugin
{
    // Helper struct
    template <typename T, size_t Size>
    struct _ReadMem;

    // Specialization for 1-byte types
    template <typename T>
    struct _ReadMem<T, 1>
    {
        T operator()(uintptr_t addr) const
        {
            auto ret = Script::Memory::ReadByte(addr);
            return reinterpret_cast<T&>(ret);
        }
    };

    // Specialization for 2-byte types
    template <typename T>
    struct _ReadMem<T, 2>
    {
        T operator()(uintptr_t addr) const
        {
            auto ret = Script::Memory::ReadWord(addr);
            return reinterpret_cast<T&>(ret);
        }
    };

    // Specialization for 4-byte types
    template <typename T>
    struct _ReadMem<T, 4>
    {
        T operator()(uintptr_t addr) const
        {
            auto ret = Script::Memory::ReadDword(addr);
            return reinterpret_cast<T&>(ret);
        }
    };

    // Specialization for 8-byte types
    template <typename T>
    struct _ReadMem<T, 8>
    {
        T operator()(uintptr_t addr) const
        {
            auto ret = Script::Memory::ReadQword(addr);
            return reinterpret_cast<T&>(ret);
        }
    };

    template <typename T>
    T Read(uintptr_t addr)
    {
        return _ReadMem<T, sizeof(T)>()(addr);
    }

    template <typename T>
    std::basic_string<T> ReadConstBasicString(uintptr_t addr)
    {
        if (addr == 0)
            return {};
        // reads thru the characters twice but avoids static buffers
        constexpr auto char_size = sizeof(T);

        size_t size = 0;
        T c = Read<T>(addr);
        while (c != 0)
        {
            size++;
            c = Read<T>(addr + (size * char_size));
        }
        std::basic_string<T> str;
        str.resize(size);
        size_t read_size = 0;
        Script::Memory::Read(addr, str.data(), size * char_size, &read_size);
        if (size * char_size != read_size)
            dprintf("wtf %d -> %d", size * char_size, read_size);
        return str;
    }
    // for some reason i can't put the body of the function here
    std::string ReadConstString(uintptr_t addr);
} // namespace S2Plugin
