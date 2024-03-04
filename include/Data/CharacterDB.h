#pragma once

#include <QStringList>
#include <cstdint>

namespace S2Plugin
{
    class CharacterDB
    {
      public:
        static uint8_t charactersCount() noexcept
        {
            return 20;
        }
        const QStringList& characterNamesStringList() const noexcept
        {
            return mCharacterNamesStringList;
        }
        bool isValid() const
        {
            return ptr != 0;
        }
        uintptr_t offsetFromIndex(uint8_t id) const
        {
            constexpr size_t characterSize = 0x2C;
            return ptr + id * characterSize;
        }

      private:
        uintptr_t ptr{0};
        QStringList mCharacterNamesStringList;

        CharacterDB() = default;
        ~CharacterDB(){};
        CharacterDB(const CharacterDB&) = delete;
        CharacterDB& operator=(const CharacterDB&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
