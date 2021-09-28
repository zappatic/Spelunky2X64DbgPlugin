#include "Data/CharacterDB.h"
#include "pluginmain.h"

S2Plugin::CharacterDB::CharacterDB(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::CharacterDB::loadCharacters(StringsTable* stringsTable)
{
    auto afterBundle = mConfiguration->spelunky2()->spelunky2AfterBundle();
    if (afterBundle == 0)
    {
        return false;
    }
    if (mCharactersPtr != 0)
    {
        return true;
    }
    auto afterBundleSize = mConfiguration->spelunky2()->spelunky2AfterBundleSize();

    mMemoryOffsets.clear();
    mCharacterNames.clear();
    mCharacterNamesStringList.clear();

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 6B C3 2C 48 8D 15 ?? ?? ?? ?? 48");
    mCharactersPtr = instructionOffset + 11 + Script::Memory::ReadDword(instructionOffset + 7);

    size_t characterSize = 0x2C;
    for (size_t x = 0; x < 20; ++x)
    {
        size_t startOffset = mCharactersPtr + (x * characterSize);
        size_t offset = startOffset;
        QString characterName;
        std::unordered_map<std::string, size_t> offsets;
        for (const auto& field : mConfiguration->typeFields(MemoryFieldType::CharacterDB))
        {
            if (field.name == "full_name")
            {
                characterName = stringsTable->nameForID(Script::Memory::ReadDword(offset));
            }
            offset = setOffsetForField(field, "CharacterDB." + field.name, offset, offsets);
        }
        mMemoryOffsets[x] = offsets;
        mCharacterNames[x] = characterName;
        mCharacterNamesStringList << characterName;
    }
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::CharacterDB::offsetsForIndex(uint8_t characterIndex)
{
    if (mMemoryOffsets.count(characterIndex) > 0)
    {
        return mMemoryOffsets.at(characterIndex);
    }
    static auto empty = std::unordered_map<std::string, size_t>();
    return empty;
}

void S2Plugin::CharacterDB::reset()
{
    mCharactersPtr = 0;
}

uint8_t S2Plugin::CharacterDB::charactersCount() const noexcept
{
    return 20;
};

const std::unordered_map<uint8_t, QString>& S2Plugin::CharacterDB::characterNames() const noexcept
{
    return mCharacterNames;
}

QStringList S2Plugin::CharacterDB::characterNamesStringList() const noexcept
{
    return mCharacterNamesStringList;
}
