#pragma once

#include "Configuration.h"
#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"

namespace S2Plugin
{
    class CharacterDB : public MemoryMappedData
    {
      public:
        explicit CharacterDB(Configuration* config);
        bool loadCharacters();
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
