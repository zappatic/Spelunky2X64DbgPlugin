#include "Data/Texture.h"
#include "pluginmain.h"

Texture::Texture(size_t offset)
{
    mTexturePtr = offset;
    refreshOffsets();
}

const std::unordered_map<std::string, size_t>& Texture::offsets()
{
    return mMemoryOffsets;
}

void Texture::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mTexturePtr;
    for (const auto& field : gsTextureFields)
    {
        offset = setOffsetForField(field, field.name, offset, mMemoryOffsets);
    }
}