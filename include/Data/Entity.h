#pragma once

#include "Data/EntityDB.h"
#include "Data/MemoryMappedData.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>


class Entity : MemoryMappedData
{
  public:
    Entity(size_t offset, TreeViewMemoryFields* tree, EntityDB* entityDB);

    const std::unordered_map<std::string, size_t>& offsets();
    void refreshOffsets();
    void refreshValues();
    void interpretAs(MemoryFieldType classType);
    std::deque<MemoryFieldType> classHierarchy() const;
    void populateTreeView();

  private:
    size_t mEntityPtr = 0;
    TreeViewMemoryFields* mTree;
    MemoryFieldType mEntityType = MemoryFieldType::ClassEntity;
    std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    std::unordered_map<MemoryFieldType, QStandardItem*> mTreeViewSectionItems;
};
