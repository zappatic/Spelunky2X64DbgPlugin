#include "Data/Texture.h"
#include "pluginmain.h"

S2Plugin::Texture::Texture(size_t offset, Configuration* config) : MemoryMappedData(config)
{
    mTexturePtr = offset;
    refreshOffsets();
}

const std::unordered_map<std::string, size_t>& S2Plugin::Texture::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::Texture::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mTexturePtr;
    for (const auto& field : mConfiguration->entityClassFields(MemoryFieldType::TexturePointer))
    {
        offset = setOffsetForField(field, field.name, offset, mMemoryOffsets);
    }
}