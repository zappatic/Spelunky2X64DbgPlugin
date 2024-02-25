#include "Data/Entity.h"
#include "Configuration.h"
#include "pluginmain.h"
#include <regex>

std::string S2Plugin::Entity::entityTypeName() const
{
    return Configuration::get()->getEntityName(entityTypeID());
}

std::string S2Plugin::Entity::entityClassName() const
{
    auto entityName = entityTypeName();
    for (const auto& [regexStr, entityClassType] : Configuration::get()->defaultEntityClassTypes())
    {
        auto r = std::regex(regexStr);
        if (std::regex_match(entityName, r))
            return entityClassType;
    }
    return "Entity";
}

uint32_t S2Plugin::Entity::entityTypeID() const
{
    uintptr_t entityDBPtr = Script::Memory::ReadQword(mEntityPtr + ENTITY_OFFSETS::TYPE_PTR);
    if (entityDBPtr == 0)
    {
        return 0;
    }
    return Script::Memory::ReadDword(entityDBPtr + ENTITY_OFFSETS::DB_TYPE_ID);
}

std::vector<std::string> S2Plugin::Entity::classHierarchy(std::string validClassName)
{
    auto& ech = Configuration::get()->entityClassHierarchy();
    std::vector<std::string> hierarchy;
    std::string t = validClassName;
    while (t != "Entity")
    {
        hierarchy.push_back(t);
        auto ech_it = ech.find(t);
        if (ech_it == ech.end())
        {
            dprintf("unknown key requested in Entity::classHierarchy() (t=%s)\n", t.c_str());
        }
        t = ech_it->second;
    }
    hierarchy.push_back("Entity");
    return hierarchy;
}

uint32_t S2Plugin::Entity::uid() const
{
    return Script::Memory::ReadDword(mEntityPtr + ENTITY_OFFSETS::UID);
}

uint8_t S2Plugin::Entity::cameraLayer() const
{
    return Script::Memory::ReadByte(mEntityPtr + ENTITY_OFFSETS::LAYER);
}

std::pair<float, float> S2Plugin::Entity::position() const
{
    auto entityPosition = Script::Memory::ReadQword(mEntityPtr + ENTITY_OFFSETS::POS);
    // illegal :)
    auto returnValue = reinterpret_cast<std::pair<float, float>&>(entityPosition);
    return returnValue;
}
