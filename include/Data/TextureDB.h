#pragma once

#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class TextureDB
    {
      public:
        bool isValid() const
        {
            return (ptr != 0);
        }
        const std::string& nameForID(uint32_t id) const // id != index since there is no id 325 and the order is different
        {
            if (auto it = mTextures.find(id); it != mTextures.end())
            {
                return it->second.first;
            }
            static std::string unknownName("UNKNOWN TEXTURE NAME");
            return unknownName;
        }
        uintptr_t offsetForID(uint32_t id) const
        {
            if (auto it = mTextures.find(id); it != mTextures.end())
            {
                return it->second.second;
            }
            return 0;
        }
        const QStringList& namesStringList() const noexcept
        {
            return mTextureNamesStringList;
        }
        size_t count() const
        {
            return mTextures.size();
        }
        bool isValidID(uint32_t id) const
        {
            return mTextures.count(id) != 0;
        }
        const auto textures() const
        {
            return mTextures;
        }

      private:
        size_t ptr{0};
        std::unordered_map<uint32_t, std::pair<std::string, uintptr_t>> mTextures; // id -> name, offset
        QStringList mTextureNamesStringList;

        TextureDB() = default;
        ~TextureDB(){};
        TextureDB(const TextureDB&) = delete;
        TextureDB& operator=(const TextureDB&) = delete;

        friend class Spelunky2;
    };
} // namespace S2Plugin
