#pragma once

#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

class EntityList
{
  public:
    EntityList();

    uint32_t idForName(const std::string& name);
    std::string nameForID(uint32_t id);
    uint32_t highestEntityID() const noexcept;
    QStringList entityNames() const noexcept;

  private:
    std::unordered_map<uint32_t, std::string> mEntities;
    QStringList mEntityNames;
    uint32_t mHighestEntityID = 0;
};