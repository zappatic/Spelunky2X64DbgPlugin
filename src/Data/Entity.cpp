#include "Data/Entity.h"
#include "Configuration.h"
#include "pluginmain.h"
#include <regex>

S2Plugin::Entity::Entity(size_t offset, TreeViewMemoryFields* tree, EntityDB* entityDB, S2Plugin::Configuration* config) : MemoryMappedData(config), mEntityPtr(offset), mTree(tree)
{
    auto entityID = getEntityTypeID(offset);
    auto entityName = getEntityName(offset, entityDB);
    for (const auto& [regexStr, entityClassType] : mConfiguration->defaultEntityClassTypes())
    {
        auto r = std::regex(regexStr);
        if (std::regex_match(entityName, r))
        {
            mEntityType = entityClassType;
            break;
        }
    }
}

const std::unordered_map<std::string, size_t>& S2Plugin::Entity::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::Entity::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        auto headerIdentifier = "<" + gsMemoryFieldTypeToStringMapping.at(c) + ">";
        MemoryField headerField;
        headerField.name = "<b>" + gsMemoryFieldTypeToStringMapping.at(c) + "</b>";
        headerField.type = c;
        offset = setOffsetForField(headerField, headerIdentifier, offset, mMemoryOffsets, false);

        for (const auto& field : mConfiguration->typeFields(c))
        {
            offset = setOffsetForField(field, headerIdentifier + "." + field.name, offset, mMemoryOffsets);
        }
    }
}

void S2Plugin::Entity::refreshValues()
{
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        MemoryField headerField;
        headerField.name = "<b>" + gsMemoryFieldTypeToStringMapping.at(c) + "</b>";
        headerField.type = c;
        mTree->updateValueForField(headerField, "<" + gsMemoryFieldTypeToStringMapping.at(c) + ">", mMemoryOffsets);
    }
}

void S2Plugin::Entity::populateTreeView()
{
    mTreeViewSectionItems.clear();
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        MemoryField headerField;
        headerField.name = "<b>" + gsMemoryFieldTypeToStringMapping.at(c) + "</b>";
        headerField.type = c;
        auto item = mTree->addMemoryField(headerField, "<" + gsMemoryFieldTypeToStringMapping.at(c) + ">");
        mTree->expandItem(item);
        mTreeViewSectionItems[c] = item;
    }
}

void S2Plugin::Entity::interpretAs(MemoryFieldType classType)
{
    mEntityType = classType;
    mTree->clear();
    populateTreeView();
    refreshOffsets();
    refreshValues();
    mTree->updateTableHeader();
}

std::deque<S2Plugin::MemoryFieldType> S2Plugin::Entity::classHierarchy() const
{
    auto ech = mConfiguration->entityClassHierarchy();
    std::deque<MemoryFieldType> hierarchy;
    auto t = mEntityType;
    while (t != MemoryFieldType::ClassEntity)
    {
        hierarchy.push_front(t);
        if (ech.count(t) == 0)
        {
            dprintf("unknown key requested in Entity::classHierarchy() (t=%s)\n", gsMemoryFieldTypeToStringMapping.at(t).c_str());
        }
        t = ech.at(t);
    }
    hierarchy.push_front(MemoryFieldType::ClassEntity);
    return hierarchy;
}
