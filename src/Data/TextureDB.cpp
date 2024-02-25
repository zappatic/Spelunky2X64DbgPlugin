#include "Data/TextureDB.h"

const std::string& S2Plugin::TextureDB::nameForID(uint32_t id) const
{
    if (auto it = mTextures.find(id); it != mTextures.end())
    {
        return it->second.first;
    }
    static std::string unknownName("UNKNOWN TEXTURE NAME");
    return unknownName;
}

uintptr_t S2Plugin::TextureDB::offsetForID(uint32_t id) const
{
    if (auto it = mTextures.find(id); it != mTextures.end())
    {
        return it->second.second;
    }
    return 0;
}
