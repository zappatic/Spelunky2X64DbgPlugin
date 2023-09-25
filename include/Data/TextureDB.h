#pragma once

#include "Data/MemoryMappedData.h"
#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct Configuration;

    class TextureDB : public MemoryMappedData
    {
      public:
        explicit TextureDB(Configuration* config);
        bool loadTextureDB();

        std::unordered_map<std::string, size_t>& offsetsForTextureID(uint32_t textureDBID);
        std::string nameForID(uint32_t id) const; // id != index !!
        const QStringList& namesStringList() const noexcept;
        size_t count();
        void reset();

      private:
        size_t mTextureDBPtr = 0;
        std::unordered_map<uint32_t, std::unordered_map<std::string, size_t>> mMemoryOffsets; // texture id -> (fieldname -> offset of field value in memory)
        std::unordered_map<uint32_t, std::string> mTextureNames;                              // id -> name
        QStringList mTextureNamesStringList;
    };
} // namespace S2Plugin
