#include "Data/LevelGen.h"
#include "Configuration.h"
#include "Data/State.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

static const QColor gsDefaultColor = QColor(Qt::lightGray);

S2Plugin::LevelGen::LevelGen(State* state) : mState(state)
{
    processJSON();
}

bool S2Plugin::LevelGen::loadLevelGen()
{
    if (!Spelunky2::is_loaded() || mState == nullptr)
    {
        return false;
    }
    if (mLevelGenPtr != 0)
    {
        return true;
    }

    mLevelGenPtr = Script::Memory::ReadQword(mState->offsetForField("level_gen"));

    refreshOffsets();
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::LevelGen::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::LevelGen::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mLevelGenPtr;
    auto config = Configuration::get();
    for (const auto& field : config->typeFields(MemoryFieldType::LevelGen))
    {
        offset = config->setOffsetForField(field, "LevelGen." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::LevelGen::offsetForField(const std::string& fieldName) const
{
    auto full = "LevelGen." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::LevelGen::reset()
{
    mLevelGenPtr = 0;
    mMemoryOffsets.clear();
    // processJSON();
}

std::string S2Plugin::LevelGen::themeNameOfOffset(size_t offset) const
{
    if (offset == offsetForField("theme_dwelling.__vftable"))
    {
        return "DWELLING";
    }
    else if (offset == offsetForField("theme_jungle.__vftable"))
    {
        return "JUNGLE";
    }
    else if (offset == offsetForField("theme_volcana.__vftable"))
    {
        return "VOLCANA";
    }
    else if (offset == offsetForField("theme_olmec.__vftable"))
    {
        return "OLMEC";
    }
    else if (offset == offsetForField("theme_tidepool.__vftable"))
    {
        return "TIDE POOL";
    }
    else if (offset == offsetForField("theme_temple.__vftable"))
    {
        return "TEMPLE";
    }
    else if (offset == offsetForField("theme_icecaves.__vftable"))
    {
        return "ICE CAVES";
    }
    else if (offset == offsetForField("theme_neobabylon.__vftable"))
    {
        return "NEO BABYLON";
    }
    else if (offset == offsetForField("theme_sunkencity.__vftable"))
    {
        return "SUNKEN CITY";
    }
    else if (offset == offsetForField("theme_cosmicocean.__vftable"))
    {
        return "COSMIC OCEAN";
    }
    else if (offset == offsetForField("theme_city_of_gold.__vftable"))
    {
        return "CITY OF GOLD";
    }
    else if (offset == offsetForField("theme_duat.__vftable"))
    {
        return "DUAT";
    }
    else if (offset == offsetForField("theme_abzu.__vftable"))
    {
        return "ABZU";
    }
    else if (offset == offsetForField("theme_tiamat.__vftable"))
    {
        return "TIAMAT";
    }
    else if (offset == offsetForField("theme_eggplantworld.__vftable"))
    {
        return "EGGPLANT WORLD";
    }
    else if (offset == offsetForField("theme_hundun.__vftable"))
    {
        return "HUNDUN";
    }
    else if (offset == offsetForField("theme_basecamp.__vftable"))
    {
        return "BASE CAMP";
    }
    else if (offset == offsetForField("theme_arena.__vftable"))
    {
        return "ARENA";
    }
    return "UNKNOWN THEME";
}

void S2Plugin::LevelGen::processJSON()
{
    mRoomCodes.clear();
    using nlohmann::ordered_json;

    std::unordered_map<std::string, QColor> colors;

    auto getColor = [&colors](const std::string& colorName) -> const QColor&
    {
        if (colors.count(colorName) > 0)
        {
            return colors.at(colorName);
        }
        return gsDefaultColor;
    };

    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath(QString::fromStdString("plugins/Spelunky2RoomCodes.json"));
    if (!QFile(pathQStr).exists())
    {
        displayError("Spelunky2RoomCodes.json not found");
        return;
    }

    try
    {
        std::ifstream fp(pathQStr.toStdString());
        std::string jsonString((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
        auto j = ordered_json::parse(jsonString, nullptr, true, true);

        if (j.contains("colors"))
        {
            for (const auto& [colorName, colorDetails] : j["colors"].items())
            {
                QColor c;
                c.setRed(colorDetails["r"].get<uint8_t>());
                c.setGreen(colorDetails["g"].get<uint8_t>());
                c.setBlue(colorDetails["b"].get<uint8_t>());
                c.setAlpha(colorDetails["a"].get<uint8_t>());
                colors[colorName] = c;
            }
        }
        if (j.contains("roomcodes"))
        {
            for (const auto& [roomCodeStr, roomDetails] : j["roomcodes"].items())
            {
                RoomCode rc;
                rc.id = std::stoi(roomCodeStr, 0, 16);
                rc.name = roomDetails.contains("name") ? roomDetails["name"].get<std::string>() : "Unnamed room code";
                rc.color = roomDetails.contains("color") ? getColor(roomDetails["color"].get<std::string>()) : gsDefaultColor;
                mRoomCodes[rc.id] = rc;
            }
        }
    }
    catch (const ordered_json::exception& e)
    {
        displayError(("Exception while parsing Spelunky2RoomCodes.json: " + std::string(e.what())).c_str());
    }
    catch (const std::exception& e)
    {
        displayError(("Exception while parsing Spelunky2RoomCodes.json: " + std::string(e.what())).c_str());
    }
    catch (...)
    {
        displayError("Unknown exception while parsing Spelunky2RoomCodes.json");
    }
}

S2Plugin::RoomCode S2Plugin::LevelGen::roomCodeForID(uint16_t code) const
{
    if (mRoomCodes.count(code) > 0)
    {
        return mRoomCodes.at(code);
    }
    RoomCode rc;
    rc.id = code;
    rc.name = "Unknown room code";
    rc.color = gsDefaultColor;
    return rc;
}
