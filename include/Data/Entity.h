#pragma once

#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

class Entity : MemoryMappedData
{
  public:
    explicit Entity(size_t offset);

    const std::unordered_map<std::string, size_t>& offsets();
    void refreshOffsets();

  private:
    size_t mEntityPtr = 0;
    std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
};
