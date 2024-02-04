#pragma once

#include "pluginmain.h"
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
            return offset;
        }
        size_t end() const
        {
            return offset + lenght() * sizeof(T);
        }
        bool empty() const
        {
            return lenght() == 0;
        }
        size_t string_ptr() const
        {
            if (capacity() > 15)
                return Script::Memory::ReadQword(offset);

            return offset;
        }
        std::unique_ptr<T[]> get_string() const
        {
            size_t string_offset = string_ptr();
            size_t string_lenght = lenght();
            std::unique_ptr<T[]> data = std::make_unique<T[]>(string_lenght + 1);
            if (string_lenght != 0)
            {
                Script::Memory::Read(string_offset, data.get(), sizeof(T) * string_lenght, nullptr);
            }
            data.get()[string_lenght] = (T)NULL;
            return data;
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

            auto this_str = get_string();
            auto other_str = other.get_string();
            return memcmp(this_str.get(), other_str.get(), l * sizeof(T)) == 0;
        }
        bool operator!=(const StdString<T> other) const
        {
            return !operator==(other);
        }

      private:
        size_t offset;
    };
} // namespace S2Plugin
