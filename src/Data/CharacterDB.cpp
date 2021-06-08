#include "Data/CharacterDB.h"
#include "pluginmain.h"

S2Plugin::CharacterDB::CharacterDB(Configuration* config) : MemoryMappedData(config) {}

bool S2Plugin::CharacterDB::loadCharacters()
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

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "0F B6 F2 4C 8D 2D");
    mCharactersPtr = instructionOffset + 10 + Script::Memory::ReadDword(instructionOffset + 6);

    size_t characterSize = 0x6C;
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
                char buffer[1024] = {0};
                Script::Memory::Read(offset, buffer, field.extraInfo, nullptr);
                characterName = QString::fromUtf16(reinterpret_cast<const ushort*>(buffer));
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
