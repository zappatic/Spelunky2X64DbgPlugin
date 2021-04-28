#include "Data/EntityList.h"
#include "pluginmain.h"
#include <filesystem>
#include <fstream>
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList()
{
    const std::string pathStr = "C:/Users/main/Downloads/Spelunky2-1.20.4d/entities.txt";
    auto path = std::filesystem::path(pathStr);
    if (std::filesystem::exists(path))
    {
        std::ifstream fp(pathStr);
        while (fp)
        {
            std::string line;
            if (!std::getline(fp, line))
            {
                break;
            }
            std::smatch m;
            if (std::regex_match(line, m, regexEntityLine))
            {
                auto entityID = std::stoi(m[1].str());
                auto entityName = m[2].str();
                mEntities[entityID] = entityName;
                mEntityNames << QString::fromStdString(entityName);
                if (entityID > mHighestEntityID)
                {
                    mHighestEntityID = entityID;
                }
            }
        }
        fp.close();
    }
    else
    {
        // TODO
        dprintf("ERROR: entities.txt not found\n");
    }
}

uint32_t S2Plugin::EntityList::idForName(const std::string& searchName)
{
    for (const auto& [id, name] : mEntities)
    {
        if (name == searchName)
        {
            return id;
        }
    }
    return 0;
}

std::string S2Plugin::EntityList::nameForID(uint32_t id)
{
    return mEntities.at(id);
}

uint32_t S2Plugin::EntityList::highestEntityID() const noexcept
{
    return mHighestEntityID;
}

QStringList S2Plugin::EntityList::entityNames() const noexcept
{
    return mEntityNames;
}