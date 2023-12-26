#pragma once

#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct StringsTable;

    class CharacterDB 
    {
      public:
        bool loadCharacters(StringsTable* stringsTable);
        uint8_t charactersCount() const noexcept;

        std::unordered_map<std::string, size_t>& offsetsForIndex(uint8_t characterIndex);
        const std::unordered_map<uint8_t, QString>& characterNames() const noexcept;
        QStringList characterNamesStringList() const noexcept;

        void reset();

      private:
        size_t mCharactersPtr = 0;
        std::unordered_map<uint8_t, std::unordered_map<std::string, size_t>> mMemoryOffsets; // map of character id -> ( fieldname -> offset ) of field value in memory
        std::unordered_map<uint8_t, QString> mCharacterNames;
        QStringList mCharacterNamesStringList;
    };
} // namespace S2Plugin
