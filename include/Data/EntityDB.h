#pragma once

#include "Data/MemoryMappedData.h"
#include "EntityList.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

class EntityDB : public MemoryMappedData
{
  public:
    void loadEntityDB();
    EntityList* entityList() const noexcept;

    const std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t entityDBIndex);

  private:
    size_t mEntityDBPtr = 0;
    std::unique_ptr<EntityList> mEntityList;
    std::vector<std::unordered_map<std::string, size_t>> mMemoryOffsets; // list of fieldname -> offset of field value in memory
};
