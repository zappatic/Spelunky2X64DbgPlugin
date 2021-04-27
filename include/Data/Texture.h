#pragma once

#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

class Texture : MemoryMappedData
{
  public:
    explicit Texture(size_t offset);

    const std::unordered_map<std::string, size_t>& offsets();
    void refreshOffsets();

  private:
    size_t mTexturePtr = 0;
    std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
};
