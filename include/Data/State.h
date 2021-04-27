#pragma once

#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

class State : MemoryMappedData
{
  public:
    void loadState();

    const std::unordered_map<std::string, size_t>& offsets();
    void refreshOffsets();
    size_t offsetForField(const std::string& fieldName) const;

  private:
    size_t mStatePtr = 0;
    std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
};
