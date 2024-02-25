#pragma once

#include <QString>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class StringsTable
    {
      public:
        QString stringForIndex(uint32_t id) const;
        bool isValid() const
        {
            return (ptr != 0);
        }
        uintptr_t offsetForIndex(size_t idx) const;
        size_t count() const
        {
            return size;
        }

      private:
        uintptr_t ptr{0};
        size_t size{0};
        // TODO add some cache
        friend class Spelunky2;
    };
} // namespace S2Plugin
