#include "Data/EntityList.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <filesystem>
#include <fstream>
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList(Spelunky2* spel2)
{
    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2Entities.txt");
    if (!QFile(pathQStr).exists())
    {
        spel2->displayError("Spelunky2Entities.txt not found in plugins folder");
        return;
    }

    std::ifstream fp(pathQStr.toStdString());
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
    if (mEntities.count(id) > 0)
    {
        return mEntities.at(id);
    }
    return "UNKNOWN ID: " + std::to_string(id);
}

uint32_t S2Plugin::EntityList::highestEntityID() const noexcept
{
    return mHighestEntityID;
}

QStringList S2Plugin::EntityList::entityNames() const noexcept
{
    return mEntityNames;
}

bool S2Plugin::EntityList::isValidID(uint32_t id)
{
    return (mEntities.count(id) > 0);
}
