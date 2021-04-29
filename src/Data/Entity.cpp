#include "Data/Entity.h"
#include "Configuration.h"
#include "pluginmain.h"
#include <regex>

S2Plugin::Entity::Entity(size_t offset, TreeViewMemoryFields* tree, EntityDB* entityDB, S2Plugin::Configuration* config) : MemoryMappedData(config), mEntityPtr(offset), mTree(tree)
{
    auto entityID = config->spelunky2()->getEntityTypeID(offset);
    auto entityName = config->spelunky2()->getEntityName(offset, entityDB);
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

void S2Plugin::Entity::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mEntityPtr;
    auto hierarchy = classHierarchy();
    for (auto c : hierarchy)
    {
        auto headerIdentifier = gsMemoryFieldTypeToStringMapping.at(c);
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
        mTree->updateValueForField(headerField, gsMemoryFieldTypeToStringMapping.at(c), mMemoryOffsets);
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
        auto item = mTree->addMemoryField(headerField, gsMemoryFieldTypeToStringMapping.at(c));
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

size_t S2Plugin::Entity::findEntityByUID(uint32_t uidToSearch, State* state)
{
    auto searchUID = [state](uint32_t uid, size_t layerOffset) -> size_t {
        auto entityCount = (std::min)(Script::Memory::ReadDword(layerOffset + 28), 10000u);
        auto entities = Script::Memory::ReadQword(layerOffset + 8);

        for (auto x = 0; x < entityCount; ++x)
        {
            auto entityPtr = entities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityUid = Script::Memory::ReadDword(entity + 56);
            if (entityUid == uid)
            {
                return entity;
            }
        }
        return 0;
    };
    auto layer = Script::Memory::ReadQword(state->offsetForField("layer0"));
    auto result = searchUID(uidToSearch, layer);
    if (result != 0)
    {
        return result;
    }

    layer = Script::Memory::ReadQword(state->offsetForField("layer1"));
    result = searchUID(uidToSearch, layer);
    if (result != 0)
    {
        return result;
    }
    return 0;
}