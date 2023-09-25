#pragma once

#include "Data/MemoryMappedData.h"
#include "EntityList.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct Configuration;

    class EntityDB : public MemoryMappedData
    {
      public:
        explicit EntityDB(Configuration* config);
        bool loadEntityDB();
        EntityList* entityList() const noexcept;

        std::unordered_map<std::string, size_t>& offsetsForIndex(uint32_t entityDBIndex);

        void reset();

      private:
        size_t mEntityDBPtr = 0;
        std::unique_ptr<EntityList> mEntityList;
        std::vector<std::unordered_map<std::string, size_t>> mMemoryOffsets; // list of fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
