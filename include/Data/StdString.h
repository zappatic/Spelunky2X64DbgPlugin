#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <memory>

namespace S2Plugin
{
    // template just in case we want wstring or something, probably basic_string would fit this better
    template <typename T = char>
    struct StdString
    {
        StdString(size_t addr) : offset(addr){};
        size_t size() const
        {
            return Script::Memory::ReadQword(offset + 0x10);
        }
        size_t lenght() const
        {
            return size();
        }
        size_t capacity() const
        {
            return Script::Memory::ReadQword(offset + 0x18);
        }
        size_t begin() const
        {
            return string_ptr();
        }
        size_t end() const
        {
            return string_ptr() + lenght() * sizeof(T);
        }
        bool empty() const
        {
            return lenght() == 0;
        }
        size_t string_ptr() const
        {
            if (capacity() > (16 / sizeof(T)) - 1) // TODO only tested for char type
                return Script::Memory::ReadQword(offset);

            return offset;
        }
        std::basic_string<T> get_string() const
        {
            size_t string_offset = string_ptr();
            size_t string_lenght = lenght();
            std::basic_string<T> buffer;
            buffer.resize(string_lenght);
            if (string_lenght != 0)
            {
                Script::Memory::Read(string_offset, buffer.data(), string_lenght * sizeof(T), nullptr);
            }
            return buffer;
        }
        bool operator==(const StdString<T> other) const
        {
            if (string_ptr() == other.string_ptr())
                return true;

            auto l = lenght();
            auto other_l = other.lenght();
            if (l != other_l)
                return false;

            if (l == 0) // both lengths the same at this point, so both are 0
                return true;

            return get_string() == other.get_string();
        }

      private:
        size_t offset;
    };
} // namespace S2Plugin
