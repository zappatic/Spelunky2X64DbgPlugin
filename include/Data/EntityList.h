#pragma once

#include "Spelunky2.h"
#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class EntityList
    {
      public:
        explicit EntityList(Spelunky2* spel2);

        uint32_t idForName(const std::string& name);
        std::string nameForID(uint32_t id);
        uint32_t highestEntityID() const noexcept;
        QStringList entityNames() const noexcept;

      private:
        std::unordered_map<uint32_t, std::string> mEntities;
        QStringList mEntityNames;
        uint32_t mHighestEntityID = 0;
    };
} // namespace S2Plugin