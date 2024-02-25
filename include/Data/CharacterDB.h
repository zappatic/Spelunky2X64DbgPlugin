#pragma once

#include <QStringList>
#include <cstdint>
#include <unordered_map>

namespace S2Plugin
{
    class CharacterDB
    {
      public:
        static uint8_t charactersCount() noexcept
        {
            return 20;
        }
        const std::unordered_map<uint8_t, QString>& characterNames() const noexcept
        {
            return mCharacterNames;
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
        std::unordered_map<uint8_t, QString> mCharacterNames;
        QStringList mCharacterNamesStringList;

        CharacterDB() = default;
        ~CharacterDB(){};
        CharacterDB(const CharacterDB&) = delete;
        CharacterDB& operator=(const CharacterDB&) = delete;

        friend class Spelunky2;
    };
} // namespace S2Plugin
