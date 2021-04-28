#pragma once

#include "Configuration.h"
#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>


namespace S2Plugin
{
    class Texture : MemoryMappedData
    {
      public:
        Texture(size_t offset, Configuration* config);

        const std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();

      private:
        size_t mTexturePtr = 0;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin