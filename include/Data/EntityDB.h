#pragma once

#include "EntityList.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

class EntityDB
{
  public:
    void loadEntityDB();
    EntityList* entityList() const noexcept;

    size_t offsetForField(uint32_t entityDBIndex, const std::string& fieldName);
    const std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t entityDBIndex);

  private:
    size_t mEntityDBPtr = 0;
    std::vector<std::unordered_map<std::string, size_t>> mMemoryOffsets; // list of fieldname -> offset of field value in memory
    std::unique_ptr<EntityList> mEntityList;

    size_t addOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets);
    size_t addRectOffsets(const std::string& rectName, size_t offset, std::unordered_map<std::string, size_t>& offsets);
};