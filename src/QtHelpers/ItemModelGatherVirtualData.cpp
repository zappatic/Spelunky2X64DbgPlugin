#include "QtHelpers/ItemModelGatherVirtualData.h"
#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/GameManager.h"
#include "Data/LevelGen.h"
#include "Data/State.h"
#include "Data/VirtualTableLookup.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

S2Plugin::ItemModelGatherVirtualData::ItemModelGatherVirtualData(ViewToolbar* toolbar, QObject* parent) : QAbstractItemModel(parent), mToolbar(toolbar)
{
    parseJSON();
}

Qt::ItemFlags S2Plugin::ItemModelGatherVirtualData::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant S2Plugin::ItemModelGatherVirtualData::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto& entry = mEntries.at(index.row());
        switch (index.column())
        {
            case gsColGatherID:
                return entry.id;
            case gsColGatherName:
                return entry.name;
            case gsColGatherVirtualTableOffset:
                return entry.virtualTableOffset;
            case gsColGatherCollision1Present:
                return entry.collision1Present ? "Yes" : "";
            case gsColGatherCollision2Present:
                return entry.collision2Present ? "Yes" : "";
            case gsColGatherOpenPresent:
                return entry.openPresent ? "Yes" : "";
            case gsColGatherDamagePresent:
                return entry.damagePresent ? "Yes" : "";
            case gsColGatherKillPresent:
                return entry.killPresent ? "Yes" : "";
            case gsColGatherDestroyPresent:
                return entry.destroyPresent ? "Yes" : "";
            case gsColGatherStatemachinePresent:
                return entry.statemachinePresent ? "Yes" : "";
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelGatherVirtualData::rowCount(const QModelIndex& parent) const
{
    return mEntries.size();
}

int S2Plugin::ItemModelGatherVirtualData::columnCount(const QModelIndex& parent) const
{
    return 10;
}

QModelIndex S2Plugin::ItemModelGatherVirtualData::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex S2Plugin::ItemModelGatherVirtualData::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

QVariant S2Plugin::ItemModelGatherVirtualData::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColGatherID:
                return "ID";
            case gsColGatherName:
                return "Name";
            case gsColGatherVirtualTableOffset:
                return "Vftable start";
            case gsColGatherCollision1Present:
                return "Collision 1";
            case gsColGatherCollision2Present:
                return "Collision 2";
            case gsColGatherOpenPresent:
                return "Open";
            case gsColGatherDamagePresent:
                return "Damage";
            case gsColGatherKillPresent:
                return "Kill";
            case gsColGatherDestroyPresent:
                return "Destroy";
            case gsColGatherStatemachinePresent:
                return "State Machine";
        }
    }
    return QVariant();
}

void S2Plugin::ItemModelGatherVirtualData::gatherEntities()
{
    auto state = mToolbar->state();
    auto spel2 = Spelunky2::get();
    auto entityDB = mToolbar->entityDB();
    auto vtl = mToolbar->virtualTableLookup();

    auto processEntities = [&](size_t layerEntities, uint32_t count)
    {
        size_t maximum = (std::min)(count, 10000u);
        for (auto x = 0; x < maximum; ++x)
        {
            auto entityPtr = layerEntities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityVTableOffset = Script::Memory::ReadQword(entity);

            auto entityType = Script::Memory::ReadDword(Script::Memory::ReadQword(entity + 0x8) + 0x14);

            for (auto& entry : mEntries)
            {
                if (entry.virtualTableOffset == 0 && entry.id == entityType)
                {
                    auto tableOffset = (entityVTableOffset - vtl->tableStartAddress()) / sizeof(size_t);
                    entry.virtualTableOffset = tableOffset;
                }
            }
        }
    };

    beginResetModel();
    auto layer0 = Script::Memory::ReadQword(state->offsetForField("layer0"));
    auto layer0Count = Script::Memory::ReadDword(layer0 + 28);
    auto layer0Entities = Script::Memory::ReadQword(layer0 + 8);
    processEntities(layer0Entities, layer0Count);

    auto layer1 = Script::Memory::ReadQword(state->offsetForField("layer1"));
    auto layer1Count = Script::Memory::ReadDword(layer1 + 28);
    auto layer1Entities = Script::Memory::ReadQword(layer1 + 8);
    processEntities(layer1Entities, layer1Count);
    endResetModel();
}

void S2Plugin::ItemModelGatherVirtualData::gatherExtraObjects()
{
    beginResetModel();
    auto vtl = mToolbar->virtualTableLookup();

    auto themes = std::vector<std::string>{"theme_dwelling", "theme_jungle",     "theme_volcana",       "theme_olmec",       "theme_tidepool",     "theme_temple",
                                           "theme_icecaves", "theme_neobabylon", "theme_sunkencity",    "theme_cosmicocean", "theme_city_of_gold", "theme_duat",
                                           "theme_abzu",     "theme_tiamat",     "theme_eggplantworld", "theme_hundun",      "theme_basecamp",     "theme_arena"};
    size_t index = 1000;
    for (const auto& themeName : themes)
    {
        auto themeAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(mToolbar->levelGen()->offsetForField(themeName)));
        auto tableOffset = (themeAddress - vtl->tableStartAddress()) / sizeof(size_t);
        bool foundInEntries = false;
        for (auto& entry : mEntries)
        {
            if (entry.id == index)
            {
                entry.virtualTableOffset = tableOffset;
                foundInEntries = true;
            }
        }
        if (!foundInEntries)
        {
            auto g = GatheredDataEntry();
            g.id = index;
            std::string themeUpper = themeName;
            std::transform(themeUpper.begin(), themeUpper.end(), themeUpper.begin(), ::toupper);
            g.name = QString::fromStdString(themeUpper);
            g.virtualTableOffset = tableOffset;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
        index++;
    }

    auto logics = std::vector<std::string>{
        "tutorial",
        "ouroboros",
        "basecamp_speedrun",
        "ghost_trigger",
        "ghost_toast_trigger",
        "tun_aggro",
        "diceshop",
        "tun_pre_challenge",
        "tun_moon_challenge",
        "tun_star_challenge",
        "tun_sun_challenge",
        "volcana_related",
        "water_related",
        "olmec_cutscene",
        "tiamat_cutscene",
        "apep_trigger",
        "city_of_gold_ankh_sacrifice",
        "duat_bosses_trigger",
        "tiamat",
        "tusk_pleasure_palace",
        "discovery_info",
        "black_market",
        "cosmic_ocean",
        "arena_1",
        "arena_2",
        "arena_3",
        "arena_alien_blast",
        "arena_loose_bombs",
    };
    index = 2000;
    for (const auto& logicName : logics)
    {
        auto logicAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(mToolbar->state()->offsetForField("logic." + logicName)));
        auto tableOffset = 0;
        if (logicAddress != 0)
        {
            tableOffset = (logicAddress - vtl->tableStartAddress()) / sizeof(size_t);
        }
        bool foundInEntries = false;
        for (auto& entry : mEntries)
        {
            if (entry.id == index)
            {
                if (tableOffset != 0)
                {
                    entry.virtualTableOffset = tableOffset;
                }
                foundInEntries = true;
            }
        }
        if (!foundInEntries)
        {
            auto g = GatheredDataEntry();
            g.id = index;
            auto themeUpper = "LOGIC_" + logicName;
            std::transform(themeUpper.begin(), themeUpper.end(), themeUpper.begin(), ::toupper);
            g.name = QString::fromStdString(themeUpper);
            g.virtualTableOffset = tableOffset;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
        index++;
    }

    auto screens_gamemanager = std::vector<std::string>{
        "screen_logo",         "screen_intro",      "screen_prologue", "screen_title", "screen_menu",           "screen_options",      "screen_player_profile",
        "screen_leaderboards", "screen_seed_input", "screen_camp",     "screen_level", "screen_online_loading", "screen_online_lobby",
    };
    index = 3000;
    for (const auto& screenName : screens_gamemanager)
    {
        auto screenAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(mToolbar->gameManager()->offsetForField(screenName)));
        auto tableOffset = 0;
        if (screenAddress != 0)
        {
            tableOffset = (screenAddress - vtl->tableStartAddress()) / sizeof(size_t);
        }
        bool foundInEntries = false;
        for (auto& entry : mEntries)
        {
            if (entry.id == index)
            {
                if (tableOffset != 0)
                {
                    entry.virtualTableOffset = tableOffset;
                }
                foundInEntries = true;
            }
        }
        if (!foundInEntries)
        {
            auto g = GatheredDataEntry();
            g.id = index;
            std::string themeUpper = screenName;
            std::transform(themeUpper.begin(), themeUpper.end(), themeUpper.begin(), ::toupper);
            g.name = QString::fromStdString(themeUpper);
            g.virtualTableOffset = tableOffset;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
        index++;
    }

    auto screens_state = std::vector<std::string>{
        "screen_character_select",
        "screen_team_select",
        "screen_transition",
        "screen_death",
        "screen_win",
        "screen_credits",
        "screen_scores",
        "screen_constellation",
        "screen_recap",
        "screen_arena_menu",
        "screen_arena_stages_select1",
        "screen_arena_items",
        "screen_arena_intro",
        "screen_arena_level",
        "screen_arena_score",
    };
    index = 3500;
    for (const auto& screenName : screens_state)
    {
        auto screenAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(mToolbar->state()->offsetForField(screenName)));
        auto tableOffset = 0;
        if (screenAddress != 0)
        {
            tableOffset = (screenAddress - vtl->tableStartAddress()) / sizeof(size_t);
        }
        bool foundInEntries = false;
        for (auto& entry : mEntries)
        {
            if (entry.id == index)
            {
                if (tableOffset != 0)
                {
                    entry.virtualTableOffset = tableOffset;
                }
                foundInEntries = true;
            }
        }
        if (!foundInEntries)
        {
            auto g = GatheredDataEntry();
            g.id = index;
            std::string themeUpper = screenName;
            std::transform(themeUpper.begin(), themeUpper.end(), themeUpper.begin(), ::toupper);
            g.name = QString::fromStdString(themeUpper);
            g.virtualTableOffset = tableOffset;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
        index++;
    }

    auto quests = std::vector<std::string>{
        "yang", "jungle_sisters", "van_horsing", "sparrow", "madame_tusk", "beg",
    };
    index = 4000;
    for (const auto& questName : quests)
    {
        auto questAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(mToolbar->state()->offsetForField("quests." + questName)));
        auto tableOffset = 0;
        if (questAddress != 0)
        {
            tableOffset = (questAddress - vtl->tableStartAddress()) / sizeof(size_t);
        }
        bool foundInEntries = false;
        for (auto& entry : mEntries)
        {
            if (entry.id == index)
            {
                if (tableOffset != 0)
                {
                    entry.virtualTableOffset = tableOffset;
                }
                foundInEntries = true;
            }
        }
        if (!foundInEntries)
        {
            auto g = GatheredDataEntry();
            g.id = index;
            auto themeUpper = "QUEST_" + questName;
            std::transform(themeUpper.begin(), themeUpper.end(), themeUpper.begin(), ::toupper);
            g.name = QString::fromStdString(themeUpper);
            g.virtualTableOffset = tableOffset;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
        index++;
    }

    endResetModel();
}

float S2Plugin::ItemModelGatherVirtualData::completionPercentage() const
{
    float completed = 0.0;
    for (const auto& e : mEntries)
    {
        if (e.virtualTableOffset != 0)
        {
            completed += 1.0;
        }
    }
    return (completed / mEntries.size()) * 100;
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpJSON() const
{
    nlohmann::json root;
    for (const auto& e : mEntries)
    {
        nlohmann::json j;
        j["id"] = e.id;
        j["name"] = e.name.toStdString();
        j["offset"] = e.virtualTableOffset;
        j["collision1"] = e.collision1Present;
        j["collision2"] = e.collision2Present;
        j["open"] = e.openPresent;
        j["damage"] = e.damagePresent;
        j["kill"] = e.killPresent;
        j["destroy"] = e.destroyPresent;
        j["statemachine"] = e.statemachinePresent;

        root.push_back(j);
    }
    return root.dump(4);
}

void S2Plugin::ItemModelGatherVirtualData::parseJSON()
{
    using nlohmann::ordered_json;

    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath(QString::fromStdString("plugins/Spelunky2VirtualTableData.json"));
    if (QFile(pathQStr).exists())
    {
        try
        {
            std::ifstream fp(pathQStr.toStdString());
            std::string jsonString((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
            auto j = ordered_json::parse(jsonString, nullptr, true, true);

            for (const auto& jsonEntry : j)
            {
                auto g = GatheredDataEntry();
                g.id = jsonEntry["id"].get<uint32_t>();
                g.name = QString::fromStdString(jsonEntry["name"].get<std::string>());
                g.virtualTableOffset = jsonEntry["offset"].get<size_t>();
                g.collision1Present = jsonEntry["collision1"].get<bool>();
                g.collision2Present = jsonEntry["collision2"].get<bool>();
                g.openPresent = jsonEntry["open"].get<bool>();
                g.damagePresent = jsonEntry["damage"].get<bool>();
                g.killPresent = jsonEntry["kill"].get<bool>();
                g.destroyPresent = jsonEntry["destroy"].get<bool>();
                g.statemachinePresent = jsonEntry["statemachine"].get<bool>();
                mEntries.emplace_back(std::move(g));
            }
        }
        catch (const ordered_json::exception& e)
        {
            displayError(("Exception while parsing Spelunky2VirtualTableData.json: " + std::string(e.what())).c_str());
        }
        catch (const std::exception& e)
        {
            displayError(("Exception while parsing Spelunky2VirtualTableData.json: " + std::string(e.what())).c_str());
        }
        catch (...)
        {
            displayError("Unknown exception while parsing Spelunky2VirtualTableData.json");
        }

        std::sort(mEntries.begin(), mEntries.end(), [](const GatheredDataEntry& a, const GatheredDataEntry& b) { return a.id < b.id; });
    }
    else
    {
        auto entitiesList = mToolbar->entityDB()->entityList();
        for (const auto& [entityID, entityName] : entitiesList->entries())
        {
            auto g = GatheredDataEntry();
            g.id = entityID;
            g.name = QString::fromStdString(entityName);
            g.virtualTableOffset = 0;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
    }
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpVirtTable() const
{
    std::stringstream ss;
    ss << "Entity|on_collision1|on_collision2|on_open|on_damage|on_kill|on_destroy|statemachine\n";
    ss << "---|---|---|---|---|---|---|---\n";
    for (const auto& e : mEntries)
    {
        if (e.id < 1000)
        {
            if (e.virtualTableOffset == 0)
            {
                ss << e.name.toStdString() << "|?|?|?|?|?|?|?\n";
            }
            else
            {
                ss << e.name.toStdString() << "|";
                ss << (e.collision1Present ? "col1" : "") << "|";
                ss << (e.collision2Present ? "col2" : "") << "|";
                ss << (e.openPresent ? "open" : "") << "|";
                ss << (e.damagePresent ? "dmg" : "") << "|";
                ss << (e.killPresent ? "kill" : "") << "|";
                ss << (e.destroyPresent ? "dstry" : "") << "|";
                ss << (e.statemachinePresent ? "stmchn" : "");
                ss << "\n";
            }
        }
    }
    return ss.str();
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpCppEnum() const
{
    std::stringstream ss;
    ss << "enum class VTABLE_OFFSET\n";
    ss << "{\n";
    ss << "    NONE = 0,\n";
    for (const auto& e : mEntries)
    {
        if (e.virtualTableOffset == 0)
        {
            ss << "    // " << e.name.toStdString() << " = UNKNOWN\n";
        }
        else
        {
            ss << "    " + e.name.toStdString() << " = " << e.virtualTableOffset << ",\n";
        }
    }
    ss << "};\n";
    return ss.str();
}

bool S2Plugin::ItemModelGatherVirtualData::isEntryCompleted(size_t index) const
{
    return mEntries.at(index).virtualTableOffset != 0;
}

void S2Plugin::ItemModelGatherVirtualData::gatherAvailableVirtuals()
{
    auto vtl = mToolbar->virtualTableLookup();

    auto isVirtImplemented = [=](size_t functionIndex)
    {
        auto tableAddress = vtl->tableAddressForEntry(vtl->entryForOffset(functionIndex));
        auto functionStart = Script::Memory::ReadQword(tableAddress);
        auto firstByte = Script::Memory::ReadByte(functionStart);

        // Check a simple `ret` opcode
        if (firstByte == 0xC3)
        {
            return false;
        }

        // Check for `xor eax, eax ; ret`
        auto secondByte = Script::Memory::ReadByte(functionStart + 1);
        auto thirdByte = Script::Memory::ReadByte(functionStart + 2);
        if (firstByte == 0x31 && secondByte == 0xC0 && thirdByte == 0xC3)
        {
            return false;
        }

        // Check for `mov al, 1 ; ret`
        if (firstByte == 0xB0 && secondByte == 0x01 && thirdByte == 0xC3)
        {
            return false;
        }

        return true;
    };

    beginResetModel();
    auto config = Configuration::get();
    for (auto& entry : mEntries)
    {
        if (entry.id < 1000 && entry.virtualTableOffset != 0)
        {
            auto entityClassHierarchy = config->classHierarchyOfEntity(entry.name.toStdString());
            bool isMovable = false;
            for (const auto& c : entityClassHierarchy)
            {
                if (c == "Movable")
                {
                    isMovable = true;
                    break;
                }
            }

            entry.collision1Present = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_COLLISION1));
            entry.collision2Present = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_COLLISION2));
            entry.openPresent = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_OPEN));
            entry.killPresent = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_KILL));
            entry.damagePresent = isMovable ? isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::MOVABLE_DAMAGE)) : false;
            entry.destroyPresent = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_DESTROY));
            entry.statemachinePresent = isVirtImplemented(entry.virtualTableOffset + static_cast<uint32_t>(VIRT_FUNC::ENTITY_STATEMACHINE));
        }
    }
    endResetModel();
}

S2Plugin::SortFilterProxyModelGatherVirtualData::SortFilterProxyModelGatherVirtualData(ViewToolbar* toolbar, QObject* parent) : QSortFilterProxyModel(parent), mToolbar(toolbar) {}

bool S2Plugin::SortFilterProxyModelGatherVirtualData::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (mHideCompleted && dynamic_cast<ItemModelGatherVirtualData*>(sourceModel())->isEntryCompleted(sourceRow))
    {
        return false;
    }
    return true;
}

void S2Plugin::SortFilterProxyModelGatherVirtualData::setHideCompleted(bool b)
{
    beginResetModel();
    mHideCompleted = b;
    endResetModel();
}
