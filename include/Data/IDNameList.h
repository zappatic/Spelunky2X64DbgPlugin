#pragma once

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
        IDNameList(const std::string& relFilePath, const std::regex& regex);

        uint32_t idForName(const std::string& name) const;
        std::string nameForID(uint32_t id) const;
        uint32_t highestID() const noexcept;
        size_t count() const noexcept;
        QStringList names() const noexcept;
        bool isValidID(uint32_t id) const;
        const std::unordered_map<uint32_t, std::string>& entries() const;

      private:
        std::unordered_map<uint32_t, std::string> mEntries;
        QStringList mNames;
        uint32_t mHighestID = 0;
    };
    class ParticleEmittersList : public IDNameList
    {
      public:
        explicit ParticleEmittersList();
    };
    class EntityList : public IDNameList
    {
      public:
        explicit EntityList();
    };
} // namespace S2Plugin
