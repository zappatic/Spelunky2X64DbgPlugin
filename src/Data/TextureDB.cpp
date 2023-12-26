#include "Data/TextureDB.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"


bool S2Plugin::TextureDB::loadTextureDB()
{
    auto spel2 = Spelunky2::get();
    const auto afterBundle = spel2->afterBundle;
    const auto afterBundleSize = spel2->afterBundleSize;
    if (afterBundle == 0)
    {
        return false;
    }
    if (mTextureDBPtr != 0)
    {
        return true;
    }

    mMemoryOffsets.clear();
    mTextureNames.clear();
    mTextureNamesStringList.clear();

    auto instructionPtr = Script::Pattern::FindMem(afterBundle, afterBundleSize, "4C 89 C6 41 89 CF 8B 1D");
    auto textureStartAddress = instructionPtr + 12 + (duint)Script::Memory::ReadDword(instructionPtr + 8);
    auto textureCount = Script::Memory::ReadQword(textureStartAddress);
    mTextureDBPtr = textureStartAddress + 0x8;

    auto offset = mTextureDBPtr;
    auto config = Configuration::get();

    for (auto x = 0; x < (std::min)(1000ull, textureCount); ++x)
    {
        std::unordered_map<std::string, size_t> offsets;
        for (const auto& field : config->typeFields(MemoryFieldType::TextureDB))
        {
            offset = config->setOffsetForField(field, "TextureDB." + field.name, offset, offsets);
        }
        auto textureID = Script::Memory::ReadQword(offsets.at("TextureDB.id"));
        mMemoryOffsets[textureID] = offsets;

        auto nameOffset = offsets.at("TextureDB.name");
        constexpr uint16_t bufferSize = 1024;
        char buffer[bufferSize] = {0};

        size_t value = (nameOffset == 0 ? 0 : Script::Memory::ReadQword(nameOffset));
        if (value != 0)
        {
            size_t chararray = Script::Memory::ReadQword(value);
            char c = 0;
            uint16_t counter = 0;
            do
            {
                c = Script::Memory::ReadByte(chararray + counter);
                buffer[counter++] = c;
            } while (c != 0 && counter < bufferSize);
            mTextureNames[textureID] = std::string(buffer);
            mTextureNamesStringList << QString("Texture %1 (%2)").arg(textureID).arg(QString::fromStdString(mTextureNames[textureID]));
        }
    }
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::TextureDB::offsetsForTextureID(uint32_t textureDBID)
{
    if (mMemoryOffsets.count(textureDBID) > 0)
    {
        return mMemoryOffsets.at(textureDBID);
    }
    return mMemoryOffsets.at(0);
}

void S2Plugin::TextureDB::reset()
{
    mTextureDBPtr = 0;
}

size_t S2Plugin::TextureDB::count()
{
    return mMemoryOffsets.size();
}

std::string S2Plugin::TextureDB::nameForID(uint32_t id) const
{
    if (mTextureNames.count(id) > 0)
    {
        return mTextureNames.at(id);
    }
    return "UNKNOWN TEXTURE NAME";
}

const QStringList& S2Plugin::TextureDB::namesStringList() const noexcept
{
    return mTextureNamesStringList;
}
