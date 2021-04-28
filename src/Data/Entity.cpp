#include "Data/Entity.h"
#include "pluginmain.h"
#include <regex>

Entity::Entity(size_t offset, TreeViewMemoryFields* tree, EntityDB* entityDB) : mEntityPtr(offset), mTree(tree)
{
    auto entityID = getEntityTypeID(offset);
    auto entityName = getEntityName(offset, entityDB);
    for (const auto& [regexStr, entityClassType] : gsDefaultEntityClassTypes)
    {
        auto r = std::regex(regexStr);
        if (std::regex_match(entityName, r))
        {
            mEntityType = entityClassType;
            break;
        }
    }
}

const std::unordered_map<std::string, size_t>& Entity::offsets()
{
    return mMemoryOffsets;
}

void Entity::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        for (const auto& field : gsEntityClassFields.at(c))
        {
            offset = setOffsetForField(field, field.name, offset, mMemoryOffsets);
        }
    }
}

void Entity::refreshValues()
{
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        for (const auto& field : gsEntityClassFields.at(c))
        {
            mTree->updateValueForField(field, field.name, mMemoryOffsets, mTreeViewSectionItems.at(c));
        }
    }
}

void Entity::populateTreeView()
{
    mTreeViewSectionItems.clear();
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        MemoryField headerField;
        headerField.name = "<b>" + gsMemoryFieldTypeToStringMapping.at(c) + "</b>";
        headerField.type = c;
        auto item = mTree->addMemoryField(headerField, "");
        mTree->expandItem(item);
        for (const auto& field : gsEntityClassFields.at(c))
        {
            mTree->addMemoryField(field, field.name, item);
        }
        mTreeViewSectionItems[c] = item;
    }
}

void Entity::interpretAs(MemoryFieldType classType)
{
    mEntityType = classType;
    mTree->clear();
    populateTreeView();
    refreshOffsets();
    refreshValues();
    mTree->updateTableHeader();
}

std::deque<MemoryFieldType> Entity::classHierarchy() const
{
    std::deque<MemoryFieldType> hierarchy;
    auto t = mEntityType;
    while (t != MemoryFieldType::ClassEntity)
    {
        hierarchy.push_front(t);
        t = gsEntityClassHierarchy.at(t);
    }
    hierarchy.push_front(MemoryFieldType::ClassEntity);
    return hierarchy;
}
