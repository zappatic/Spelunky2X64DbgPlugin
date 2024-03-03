#include "Data/IDNameList.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <filesystem>
#include <fstream>

S2Plugin::IDNameList::IDNameList(const std::string& relFilePath, const std::regex& regex)
{
    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath(QString::fromStdString(relFilePath));
    if (!QFile(pathQStr).exists())
    {
        displayError((relFilePath + " not found").c_str());
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
        if (std::regex_match(line, m, regex))
        {
            uint32_t id = std::stoi(m[1].str());
            auto name = m[2].str();
            mEntries[id] = name;
            mNames << QString::fromStdString(name);
            mHighestID = std::max(mHighestID, id);
        }
    }
    fp.close();
}

uint32_t S2Plugin::IDNameList::idForName(const std::string& searchName) const
{
    for (const auto& [id, name] : mEntries)
        if (name == searchName)
            return id;

    return 0;
}

std::string S2Plugin::IDNameList::nameForID(uint32_t id) const
{
    if (auto it = mEntries.find(id); it != mEntries.end())
    {
        return it->second;
    }
    return "UNKNOWN ID: " + std::to_string(id);
}

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList() : IDNameList("plugins/Spelunky2Entities.txt", regexEntityLine) {}

static const std::regex regexParticleLine("^([0-9]+): PARTICLEEMITTER_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::ParticleEmittersList::ParticleEmittersList() : IDNameList("plugins/Spelunky2ParticleEmitters.txt", regexParticleLine) {}
