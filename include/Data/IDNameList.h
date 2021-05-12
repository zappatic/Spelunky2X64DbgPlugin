#pragma once

#include "Spelunky2.h"
#include <QStringList>
#include <cstdint>
#include <regex>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class IDNameList
    {
      public:
        IDNameList(Spelunky2* spel2, const std::string& relFilePath, const std::regex& regex);

        uint32_t idForName(const std::string& name);
        std::string nameForID(uint32_t id);
        uint32_t highestID() const noexcept;
        size_t count() const noexcept;
        QStringList names() const noexcept;
        bool isValidID(uint32_t id);

      private:
        std::unordered_map<uint32_t, std::string> mEntries;
        QStringList mNames;
        uint32_t mHighestID = 0;
    };
} // namespace S2Plugin