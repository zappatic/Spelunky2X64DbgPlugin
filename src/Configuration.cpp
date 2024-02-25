#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QString>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>

using nlohmann::ordered_json;

S2Plugin::Configuration* S2Plugin::Configuration::ptr = nullptr;

namespace S2Plugin
{
    class MemoryFieldData
    {
      public:
        struct Data
        {
            std::string_view display_name;
            std::string_view cpp_type_name;
            uint32_t size;
            bool isPointer{false};
        };

        using map_type = std::unordered_map<MemoryFieldType, Data>;

        MemoryFieldData(std::initializer_list<std::tuple<MemoryFieldType, const char*, const char*, const char*, uint32_t, bool>> init)
        {
            // MemoryFieldType type, const char* d_name, const char* cpp_type, const char* j_name, uint32_t size
            for (auto& val : init)
            {
                auto it = fields.emplace(std::get<0>(val), Data{std::get<1>(val), std::get<2>(val), std::get<4>(val), std::get<5>(val)});
                if (it.second)
                {
                    auto size = strlen(std::get<3>(val));
                    if (size != 0)
                    {
                        json_names_map.emplace(std::string_view(std::get<3>(val), size), it.first);
                    }
                }
            }
        };

        map_type::const_iterator find(const MemoryFieldType key) const
        {
            return fields.find(key);
        }
        map_type::const_iterator end() const
        {
            return fields.end();
        }
        map_type::const_iterator begin() const
        {
            return fields.begin();
        }
        const Data& at(const MemoryFieldType key) const
        {
            return fields.at(key);
        }
        const Data& at(const std::string_view key) const
        {
            return json_names_map.at(key)->second;
        }
        bool contains(const MemoryFieldType key) const
        {
            return fields.count(key) != 0;
        }
        bool contains(const std::string_view key) const
        {
            return json_names_map.count(key) != 0;
        }

        map_type fields;
        std::unordered_map<std::string_view, map_type::const_iterator> json_names_map;
    };

    const MemoryFieldData gsMemoryFieldType = {
        // MemoryFieldEnum, Name for desplay, c++ type name, name in json, size (if 0 will be determinated from json struct), is pointer

        // Basic types
        {MemoryFieldType::CodePointer, "Code pointer", "size_t*", "CodePointer", 8, true},
        {MemoryFieldType::DataPointer, "Data pointer", "size_t*", "DataPointer", 8, true},
        {MemoryFieldType::Byte, "8-bit", "int8_t", "Byte", 1, false},
        {MemoryFieldType::UnsignedByte, "8-bit unsigned", "uint8_t", "UnsignedByte", 1, false},
        {MemoryFieldType::Word, "16-bit", "int16_t", "Word", 2, false},
        {MemoryFieldType::UnsignedWord, "16-bit unsigned", "uint16_t", "UnsignedWord", 2, false},
        {MemoryFieldType::Dword, "32-bit", "int32_t", "Dword", 4, false},
        {MemoryFieldType::UnsignedDword, "32-bit unsigned", "uint32_t", "UnsignedDword", 4, false},
        {MemoryFieldType::Qword, "64-bit", "int64_t", "Qword", 8, false},
        {MemoryFieldType::UnsignedQword, "64-bit unsigned", "uint64_t", "UnsignedQword", 8, false},
        {MemoryFieldType::Float, "Float", "float", "Float", 4, false},
        {MemoryFieldType::Double, "Double", "double", "Double", 8, false},
        {MemoryFieldType::Bool, "Bool", "bool", "Bool", 1, false},
        {MemoryFieldType::Flags8, "8-bit flags", "uint8_t", "Flags8", 1, false},
        {MemoryFieldType::Flags16, "16-bit flags", "uint16_t", "Flags16", 2, false},
        {MemoryFieldType::Flags32, "32-bit flags", "uint32_t", "Flags32", 4, false},
        {MemoryFieldType::State8, "8-bit state", "int8_t", "State8", 1, false},
        {MemoryFieldType::State16, "16-bit state", "int16_t", "State16", 2, false},
        {MemoryFieldType::State32, "32-bit state", "int32_t", "State32", 4, false},
        {MemoryFieldType::UTF16Char, "UTF16Char", "char16_t", "UTF16Char", 2, false},
        {MemoryFieldType::UTF16StringFixedSize, "UTF16StringFixedSize", "std::array<char16_t, S>", "UTF16StringFixedSize", 0, false},
        {MemoryFieldType::UTF8StringFixedSize, "UTF8StringFixedSize", "std::array<char, S>", "UTF8StringFixedSize", 0, false},
        {MemoryFieldType::Skip, "skip", "uint8_t", "Skip", 0, false},
        // STD lib
        {MemoryFieldType::StdVector, "StdVector", "std::vector<T>", "StdVector", 24, false},
        {MemoryFieldType::StdMap, "StdMap", "std::map<K, V>", "StdMap", 16, false},
        {MemoryFieldType::StdSet, "StdSet", "std::set<T>", "StdSet", 16, false}, // TODO
        {MemoryFieldType::StdString, "StdString", "std::string", "StdString", 32, false},
        {MemoryFieldType::StdWstring, "StdWstring", "std::wstring", "StdWstring", 32, false},
        // Game Main structs
        {MemoryFieldType::GameManager, "GameManager", "", "GameManager", 0, false},
        {MemoryFieldType::State, "State", "", "State", 0, false},
        {MemoryFieldType::SaveGame, "SaveGame", "", "SaveGame", 0, false},
        {MemoryFieldType::LevelGen, "LevelGen", "", "LevelGen", 0, false},
        {MemoryFieldType::EntityDB, "EntityDB", "", "EntityDB", 0, false},
        {MemoryFieldType::ParticleDB, "ParticleDB", "", "ParticleDB", 0, false},
        {MemoryFieldType::TextureDB, "TextureDB", "", "TextureDB", 0, false},
        {MemoryFieldType::CharacterDB, "CharacterDB", "", "CharacterDB", 0, false},
        {MemoryFieldType::Online, "Online", "", "Online", 0, false},
        // Special Types
        {MemoryFieldType::EntityPointer, "Entity pointer", "Entity*", "EntityPointer", 8, true},
        {MemoryFieldType::EntityUIDPointer, "Entity UID pointer", "uint32_t*", "EntityUIDPointer", 8, true},
        {MemoryFieldType::EntityDBPointer, "EntityDB pointer", "EntityDB*", "EntityDBPointer", 8, true},
        {MemoryFieldType::EntityDBID, "EntityDB ID", "uint32_t", "EntityDBID", 4, false},
        {MemoryFieldType::EntityUID, "Entity UID", "int32_t", "EntityUID", 4, false},
        {MemoryFieldType::ParticleDBID, "ParticleDB ID", "uint32_t", "ParticleDBID", 4, false},
        {MemoryFieldType::ParticleDBPointer, "ParticleDB pointer", "ParticleDB*", "ParticleDBPointer", 8, true},
        {MemoryFieldType::TextureDBID, "TextureDB ID", "int32_t", "TextureDBID", 4, false},
        {MemoryFieldType::TextureDBPointer, "TextureDB pointer", "Texture*", "TextureDBPointer", 8, true},
        {MemoryFieldType::ConstCharPointer, "Const char*", "const char*", "ConstCharPointer", 8, true},
        {MemoryFieldType::ConstCharPointerPointer, "Const char**", "const char**", "ConstCharPointerPointer", 8, true},                         // there is more then just pointer to pointer?
        {MemoryFieldType::UndeterminedThemeInfoPointer, "UndeterminedThemeInfoPointer", "ThemeInfo*", "UndeterminedThemeInfoPointer", 8, true}, // display theme name and add ThemeInfo fields
        {MemoryFieldType::ThemeInfoName, "ThemeInfoName", "ThemeInfo*", "ThemeInfoName", 8, true},                                              // just theme name
        {MemoryFieldType::LevelGenRoomsPointer, "LevelGenRoomsPointer", "LevelGenRooms*", "LevelGenRoomsPointer", 8, true},
        {MemoryFieldType::LevelGenRoomsMetaPointer, "LevelGenRoomsMetaPointer", "LevelGenRoomsMeta*", "LevelGenRoomsMetaPointer", 8, true},
        {MemoryFieldType::JournalPagePointer, "JournalPagePointer", "JournalPage*", "JournalPagePointer", 8, true},
        {MemoryFieldType::LevelGenPointer, "LevelGenPointer", "LevelGen*", "LevelGenPointer", 8, true},
        {MemoryFieldType::StringsTableID, "StringsTable ID", "uint32_t", "StringsTableID", 4, false},
        {MemoryFieldType::CharacterDBID, "CharacterDBID", "uint8_t", "CharacterDBID", 1, false},
        {MemoryFieldType::VirtualFunctionTable, "VirtualFunctionTable", "size_t*", "VirtualFunctionTable", 8, true},
        {MemoryFieldType::IPv4Address, "IPv4Address", "uint32_t", "IPv4Address", 4, false},
        // Other
        //{MemoryFieldType::EntitySubclass, "", "", "", 0},
        //{MemoryFieldType::DefaultStructType, "", "", "", 0},
        {MemoryFieldType::Flag, "Flag", "", "", 0, false},
    };
} // namespace S2Plugin

S2Plugin::Configuration* S2Plugin::Configuration::get()
{
    if (ptr == nullptr)
    {
        auto new_config = new Configuration{};
        if (new_config->initialisedCorrectly)
            ptr = new_config;
        else
            delete new_config;
    }
    return ptr;
}

bool S2Plugin::Configuration::reload()
{
    auto new_config = new Configuration{};
    if (new_config->initialisedCorrectly)
    {
        delete ptr;
        ptr = new_config;
        return true;
    }

    delete new_config;
    return false;
}

bool S2Plugin::Configuration::is_loaded()
{
    return get() != nullptr;
}

S2Plugin::Configuration::Configuration()
{
    char buffer[MAX_PATH + 1] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    static const auto path = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2.json");
    static const auto pathENT = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2Entities.json");
    static const auto pathRC = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2RoomCodes.json");
    if (!QFile(path).exists())
    {
        displayError("Could not find " + path.toStdString());
        initialisedCorrectly = false;
        return;
    }
    if (!QFile(pathENT).exists())
    {
        displayError("Could not find " + pathENT.toStdString());
        initialisedCorrectly = false;
        return;
    }
    if (!QFile(pathRC).exists())
    {
        displayError("Could not find " + pathRC.toStdString());
        initialisedCorrectly = false;
        return;
    }

    try
    {
        std::ifstream fpRC(pathRC.toStdString());
        auto jRC = ordered_json::parse(fpRC, nullptr, true, true);
        processRoomCodesJSON(jRC);
        fpRC.close();

        std::ifstream fp(path.toStdString());
        auto j = ordered_json::parse(fp, nullptr, true, true);
        processJSON(j);
        fp.close();

        std::ifstream fpENT(pathENT.toStdString());
        auto jENT = ordered_json::parse(fpENT, nullptr, true, true);
        processEntitiesJSON(jENT);

        static std::vector<std::pair<int64_t, std::string>> unknown_flags = {
            {1, "unknown_01"},  {2, "unknown_02"},  {3, "unknown_03"},  {4, "unknown_04"},  {5, "unknown_05"},  {6, "unknown_06"},  {7, "unknown_07"},  {8, "unknown_08"},
            {9, "unknown_09"},  {10, "unknown_10"}, {11, "unknown_11"}, {12, "unknown_12"}, {13, "unknown_13"}, {14, "unknown_14"}, {15, "unknown_15"}, {16, "unknown_16"},
            {17, "unknown_17"}, {18, "unknown_18"}, {19, "unknown_19"}, {20, "unknown_20"}, {21, "unknown_21"}, {22, "unknown_22"}, {23, "unknown_23"}, {24, "unknown_24"},
            {25, "unknown_25"}, {26, "unknown_26"}, {27, "unknown_27"}, {28, "unknown_28"}, {29, "unknown_29"}, {30, "unknown_30"}, {31, "unknown_31"}, {32, "unknown_32"}};

        mRefs.emplace("unknown", unknown_flags);
    }
    catch (const ordered_json::exception& e)
    {
        displayError("Exception while parsing json: " + std::string(e.what()));
        initialisedCorrectly = false;
        return;
    }
    catch (const std::exception& e)
    {
        displayError("Exception while parsing json: " + std::string(e.what()));
        initialisedCorrectly = false;
        return;
    }
    catch (...)
    {
        displayError("Unknown exception while parsing json");
        initialisedCorrectly = false;
        return;
    }
    initialisedCorrectly = true;
}

template <class T>
inline T value_or(const nlohmann::ordered_json& j, const std::string name, T value_if_not_found)
{
    return j.contains(name) ? j[name].get<T>() : value_if_not_found;
}

S2Plugin::MemoryField S2Plugin::Configuration::populateMemoryField(const nlohmann::ordered_json& field, const std::string& struct_name)
{
    using namespace std::string_literals;

    MemoryField memField;
    memField.name = field["field"].get<std::string>();
    memField.comment = value_or(field, "commment", ""s);
    memField.type = MemoryFieldType::DefaultStructType; // just initial
    std::string fieldTypeStr = field["type"].get<std::string>();

    bool knownPointer = mPointerTypes.find(fieldTypeStr) != mPointerTypes.end();

    if (knownPointer || value_or(field, "pointer", false))
    {
        memField.isPointer = true;
        memField.size = sizeof(uintptr_t);
        memField.jsonName = fieldTypeStr;
    }
    // check if it's pre-defined type
    if (auto it = gsMemoryFieldType.json_names_map.find(fieldTypeStr); it != gsMemoryFieldType.json_names_map.end())
    {
        memField.type = it->second->first;
        memField.size = it->second->second.size;
    }

    if (field.contains("offset"))
        memField.size = field["offset"].get<size_t>();

    // exception since StdSet is just StdMap without the value
    if (fieldTypeStr == "StdMap")
    {
        memField.type = MemoryFieldType::StdMap;
        if (field.contains("keytype"))
        {
            memField.firstParameterType = field["keytype"].get<std::string>();
        }
        else
        {
            memField.firstParameterType = "UnsignedQword";
            dprintf("no keytype specified for StdMap (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
        }
        if (field.contains("valuetype"))
        {
            memField.secondParameterType = field["valuetype"].get<std::string>();
        }
        else
        {
            memField.secondParameterType = "UnsignedQword";
            dprintf("no valuetype specified for StdMap (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
        }
    }
    else if (fieldTypeStr == "StdSet")
    {
        memField.type = MemoryFieldType::StdMap;
        if (field.contains("keytype"))
        {
            memField.firstParameterType = field["keytype"].get<std::string>();
            memField.secondParameterType = "";
        }
        else
        {
            memField.firstParameterType = "UnsignedQword";
            memField.secondParameterType = "";
            dprintf("no keytype specified for StdSet (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
        }
    }
    switch (memField.type)
    {
        case MemoryFieldType::Skip:
        {
            if (memField.isPointer)
                throw std::runtime_error("skip elment cannot be marked as pointer (" + struct_name + "." + memField.name + ")");
            break;
        }
        case MemoryFieldType::StdVector:
        {
            if (field.contains("vectortype"))
            {
                memField.firstParameterType = field["vectortype"].get<std::string>();
            }
            else
            {
                memField.firstParameterType = "UnsignedQword";
                dprintf("no vectortype specified for StdVector (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
            }
            break;
        }
        case MemoryFieldType::Flags32:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::Flags8:
        {
            if (field.contains("ref"))
            {
                memField.firstParameterType = field["ref"].get<std::string>(); // using first param to hold the ref name
            }
            else if (field.contains("flags"))
            {
                std::vector<std::pair<int64_t, std::string>> flagTitles;
                flagTitles.reserve(field["flags"].size());
                for (const auto& [flagNumber, flagTitle] : field["flags"].items())
                    flagTitles.emplace_back(std::stoll(flagNumber), flagTitle.get<std::string>());

                std::string refName = struct_name + "." + memField.name;
                memField.firstParameterType = refName;
                mRefs.emplace(std::move(refName), std::move(flagTitles));
            }
            else
            {
                memField.firstParameterType = "unknown";
                dprintf("missing `flags` or `ref` in field: (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
            }
            break;
        }
        case MemoryFieldType::State8:
        case MemoryFieldType::State16:
        case MemoryFieldType::State32:
        {
            if (field.contains("ref"))
            {
                memField.firstParameterType = field["ref"].get<std::string>(); // using first param to hold the ref name
            }
            else if (field.contains("states"))
            {
                std::vector<std::pair<int64_t, std::string>> stateTitles;
                stateTitles.reserve(field["states"].size());
                for (const auto& [state, stateTitle] : field["states"].items())
                    stateTitles.emplace_back(std::stoll(state), stateTitle.get<std::string>());

                std::string refName = struct_name + "." + memField.name;
                memField.firstParameterType = refName;
                mRefs.emplace(std::move(refName), std::move(stateTitles));
            }
            else
            {
                dprintf("missing `states` or `ref` in field (%s.%s)\n", struct_name.c_str(), memField.name.c_str());
            }
            break;
        }
        case MemoryFieldType::VirtualFunctionTable:
        {
            memField.firstParameterType = struct_name; // use firstParameterType to hold the parrent type of the vtable
            if (field.contains("functions"))
            {
                auto& vector = mVirtualFunctions[struct_name];
                vector.reserve(field["functions"].size());
                for (const auto& [funcIndex, func] : field["functions"].items())
                {
                    size_t index = std::stoll(funcIndex);
                    std::string name = value_or(func, "name", "unnamed function"s);
                    std::string params = value_or(func, "params", ""s);
                    std::string returnValue = value_or(func, "return", "void"s);
                    std::string type = struct_name;
                    vector.emplace_back(index, std::move(name), std::move(params), std::move(returnValue), std::move(type));
                }
            }
            break;
        }
        case MemoryFieldType::DefaultStructType:
            memField.jsonName = fieldTypeStr;
            break;
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            memField.jsonName = "ThemeInfoPointer";
            break;
        }
    }
    if (isPointerType(memField.type))
        memField.isPointer = true;

    return memField;
}

void S2Plugin::Configuration::processEntitiesJSON(ordered_json& j)
{
    using namespace std::string_literals;

    for (const auto& [key, jsonValue] : j["entity_class_hierarchy"].items())
    {
        auto value = jsonValue.get<std::string>();
        if (key != value)
        {
            mEntityClassHierarchy[key] = std::move(value);
        }
    }
    for (const auto& [key, jsonValue] : j["default_entity_types"].items())
    {
        mDefaultEntityClassTypes.emplace_back(key, jsonValue.get<std::string>());
    }
    for (const auto& [key, jsonArray] : j["fields"].items())
    {
        std::vector<MemoryField> vec;
        vec.reserve(jsonArray.size());
        for (const auto& field : jsonArray)
        {
            if (field.contains("vftablefunctions")) // for the vtable in entity subclasses
            {
                auto& vector = mVirtualFunctions[key];
                vector.reserve(field["vftablefunctions"].size());
                for (const auto& [funcIndex, func] : field["vftablefunctions"].items())
                {
                    size_t index = std::stoll(funcIndex);
                    std::string name = value_or(func, "name", "unnamed function"s);
                    std::string params = value_or(func, "params", ""s);
                    std::string returnValue = value_or(func, "return", "void"s);
                    std::string type = key;
                    vector.emplace_back(index, std::move(name), std::move(params), std::move(returnValue), std::move(type));
                }
                continue;
            }
            MemoryField memField = populateMemoryField(field, key);
            if (std::find(vec.begin(), vec.end(), memField) != vec.end())
                throw std::runtime_error("Struct (" + key + ") contains duplicate field name: (" + memField.name + ")");

            vec.emplace_back(std::move(memField));
        }
        mTypeFieldsEntitySubclasses[key] = std::move(vec);
    }
}

void S2Plugin::Configuration::processJSON(ordered_json& j)
{
    for (const auto& t : j["pointer_types"])
    {
        mPointerTypes.emplace(t.get<std::string>());
    }
    for (const auto& [key, jsonValue] : j["struct_alignments"].items())
    {
        mAlignments.insert({key, jsonValue.get<uint8_t>()});
    }
    for (const auto& [key, jsonArray] : j["refs"].items())
    {
        std::vector<std::pair<int64_t, std::string>> vec;
        vec.reserve(jsonArray.size());
        for (const auto& [value, name] : jsonArray.items())
        {
            vec.emplace_back(std::stoll(value), name);
        }
        mRefs[key] = std::move(vec);
    }

    for (const auto& [key, jsonArray] : j["fields"].items())
    {
        std::vector<MemoryField> vec;
        vec.reserve(jsonArray.size());
        for (const auto& jsonField : jsonArray)
        {
            MemoryField memField = populateMemoryField(jsonField, key);
            if (std::find(vec.begin(), vec.end(), memField) != vec.end())
                throw std::runtime_error("Struct (" + key + ") contains duplicate field name: (" + memField.name + ")");

            vec.emplace_back(std::move(memField));
        }

        auto it = gsMemoryFieldType.json_names_map.find(key);
        if (it != gsMemoryFieldType.json_names_map.end())
        {
            mTypeFieldsMain.emplace(it->second->first, std::move(vec));
        }
        else
        {
            mTypeFieldsStructs.try_emplace(key, std::move(vec));
        }
    }
    // TODO: maybe add check for unused structs?
}

const std::unordered_map<std::string, std::string>& S2Plugin::Configuration::entityClassHierarchy() const noexcept
{
    return mEntityClassHierarchy;
}

const std::vector<std::pair<std::string, std::string>>& S2Plugin::Configuration::defaultEntityClassTypes() const noexcept
{
    return mDefaultEntityClassTypes;
}

std::vector<std::string> S2Plugin::Configuration::classHierarchyOfEntity(const std::string& entityName) const
{
    std::vector<std::string> returnVec;
    std::string entityClass;
    for (const auto& [regexStr, entityClassType] : mDefaultEntityClassTypes)
    {
        auto r = std::regex(regexStr);
        if (std::regex_match(entityName, r))
        {
            entityClass = entityClassType;
            break;
        }
    }
    if (!entityClass.empty())
    {
        std::string p = std::move(entityClass);
        while (p != "Entity" && !p.empty())
        {
            returnVec.emplace_back(p);
            p = mEntityClassHierarchy.at(p); // TODO: (at) will throw exception if the element is not found
        }
    }
    returnVec.emplace_back("Entity");
    return returnVec;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfDefaultStruct(const std::string& type) const
{
    auto it = mTypeFieldsStructs.find(type);
    if (it == mTypeFieldsStructs.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfDefaultStruct() (t=%s)\n", type.c_str());
        static std::vector<S2Plugin::MemoryField> empty; // just to return valid object
        return empty;
    }
    return it->second;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFields(const MemoryFieldType& type) const
{
    auto it = mTypeFieldsMain.find(type);
    if (it == mTypeFieldsMain.end())
    {
        // no error since we can use this to check if type is a struct
        // dprintf("unknown key requested in Configuration::typeFields() (t=%s id=%d)\n", gsMemoryFieldType.at(type).display_name.data(), type);
        static std::vector<S2Plugin::MemoryField> empty; // just to return valid object
        return empty;
    }
    return it->second;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfEntitySubclass(const std::string& type) const
{
    auto it = mTypeFieldsEntitySubclasses.find(type);
    if (it == mTypeFieldsEntitySubclasses.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfEntitySubclass() (t=%s)\n", type.c_str());
        static std::vector<S2Plugin::MemoryField> empty; // just to return valid object
        return empty;
    }
    return it->second;
}

bool S2Plugin::Configuration::isEntitySubclass(const std::string& type) const
{
    // TODO: does not count (type == "Entity") as ent subclass, is that correct?
    return (mTypeFieldsEntitySubclasses.count(type) > 0);
}

S2Plugin::MemoryFieldType S2Plugin::Configuration::getBuiltInType(const std::string& type)
{
    auto it = gsMemoryFieldType.json_names_map.find(type);
    if (it == gsMemoryFieldType.json_names_map.end())
        return MemoryFieldType::None;

    return it->second->first;
}

std::string S2Plugin::Configuration::flagTitle(const std::string& fieldName, uint8_t flagNumber) const
{
    if (auto it = mRefs.find(fieldName); it != mRefs.end() && flagNumber > 0 && flagNumber <= 32)
    {
        auto& refs = it->second;
        for (auto& pair : refs)
        {
            if (pair.first == flagNumber)
            {
                return pair.second;
            }
        }
    }
    return "";
}

std::string S2Plugin::Configuration::stateTitle(const std::string& fieldName, int64_t state) const
{
    if (auto it = mRefs.find(fieldName); it != mRefs.end())
    {
        auto& refs = it->second;
        for (auto& pair : refs)
        {
            if (pair.first == state)
            {
                return pair.second;
            }
        }
    }
    return "UNKNOWN STATE";
}

const std::vector<std::pair<int64_t, std::string>>& S2Plugin::Configuration::refTitlesOfField(const std::string& fieldName) const
{
    auto it = mRefs.find(fieldName);
    if (it == mRefs.end())
    {
        dprintf("unknown ref requested in Configuration::refTitlesOfField() (%s)\n", fieldName);
        static std::vector<std::pair<int64_t, std::string>> empty;
        return empty;
    }
    return it->second;
}

std::vector<S2Plugin::VirtualFunction> S2Plugin::Configuration::virtualFunctionsOfType(const std::string& type) const
{
    bool isKnownEntitySubclass = false;
    if (type == "Entity")
        isKnownEntitySubclass = true;
    else
        isKnownEntitySubclass = mEntityClassHierarchy.count(type) != 0;

    if (isKnownEntitySubclass)
    {
        std::vector<S2Plugin::VirtualFunction> functions;
        std::string currentType = type;
        while (true)
        {
            if (auto it = mVirtualFunctions.find(currentType); it != mVirtualFunctions.end())
                functions.insert(functions.end(), it->second.begin(), it->second.end());

            if (currentType == "Entity")
            {
                break;
            }
            currentType = mEntityClassHierarchy.at(currentType);
        }
        return functions;
    }
    else
    {
        return mVirtualFunctions.at(type);
    }
}

int S2Plugin::Configuration::getAlingment(const std::string& typeName) const
{
    if (isPermanentPointer(typeName))
    {
        return sizeof(uintptr_t);
    }
    if (auto type = getBuiltInType(typeName); type != MemoryFieldType::None)
    {
        if (isPointerType(type))
            return sizeof(uintptr_t);

        switch (type)
        {
            case MemoryFieldType::Skip:
            {
                dprintf("cannot determinate alignment of (Skip) type!\n");
                return sizeof(uintptr_t);
            }
            case MemoryFieldType::Byte:
            case MemoryFieldType::UnsignedByte:
            case MemoryFieldType::Bool:
            case MemoryFieldType::Flags8:
            case MemoryFieldType::State8:
            case MemoryFieldType::CharacterDBID:
            case MemoryFieldType::UTF8StringFixedSize:
                return sizeof(char);
            case MemoryFieldType::Word:
            case MemoryFieldType::UnsignedWord:
            case MemoryFieldType::State16:
            case MemoryFieldType::Flags16:
            case MemoryFieldType::UTF16StringFixedSize:
            case MemoryFieldType::UTF16Char:
                return sizeof(int16_t);
            case MemoryFieldType::Dword:
            case MemoryFieldType::UnsignedDword:
            case MemoryFieldType::Float:
            case MemoryFieldType::Flags32:
            case MemoryFieldType::State32:
            case MemoryFieldType::EntityDBID:
            case MemoryFieldType::ParticleDBID:
            case MemoryFieldType::EntityUID:
            case MemoryFieldType::TextureDBID:
            case MemoryFieldType::StringsTableID:
            case MemoryFieldType::IPv4Address:
            case MemoryFieldType::CharacterDB: // biggest type is 4
                return sizeof(int32_t);

            case MemoryFieldType::Online:
            case MemoryFieldType::TextureDB:
            case MemoryFieldType::ParticleDB:
            case MemoryFieldType::EntityDB:
            case MemoryFieldType::LevelGen:
            case MemoryFieldType::GameManager:
            case MemoryFieldType::State:
            case MemoryFieldType::SaveGame:
            case MemoryFieldType::StdVector:
            case MemoryFieldType::StdMap:
            case MemoryFieldType::StdSet:
            case MemoryFieldType::Qword:
            case MemoryFieldType::UnsignedQword:
            case MemoryFieldType::Double:
                return sizeof(uintptr_t);
        }
    }
    auto itr = mAlignments.find(typeName);
    if (itr == mAlignments.end())
    {
        dprintf("alignment not found for (%s)\n", typeName.c_str());
        return sizeof(uintptr_t);
    }
    else
    {
        if (itr->second > sizeof(uintptr_t))
        {
            dprintf("wrong alignment provided (%d) for struct (%s), allowed range: 0-8\n", itr->second, itr->first);
            return sizeof(uintptr_t);
        }
        return itr->second;
    }
}

size_t S2Plugin::Configuration::getTypeSize(const std::string& typeName, bool entitySubclass)
{
    if (typeName.empty())
        return 0;

    if (isPermanentPointer(typeName))
        return sizeof(uintptr_t);

    if (auto it = mTypeFieldsStructsSizes.find(typeName); it != mTypeFieldsStructsSizes.end())
        return it->second;

    auto& structs = entitySubclass ? mTypeFieldsEntitySubclasses : mTypeFieldsStructs;

    auto it = structs.find(typeName);
    if (it == structs.end())
    {
        auto json_it = gsMemoryFieldType.json_names_map.find(typeName);
        if (json_it != gsMemoryFieldType.json_names_map.end())
        {
            size_t new_size = json_it->second->second.size;
            if (new_size == 0)
            {
                for (auto& field : Configuration::get()->typeFields(json_it->second->first))
                    new_size += field.get_size();
            }
            return new_size;
        }
        dprintf("could not determinate size for (%s)\n", typeName);
        return 0;
    }

    size_t struct_size{0};
    for (auto& field : it->second)
        struct_size += field.get_size();

    // cache the size
    mTypeFieldsStructsSizes[typeName] = struct_size;
    return struct_size;
}

size_t S2Plugin::MemoryField::get_size() const
{
    if (isPointer)
        return sizeof(uintptr_t);

    if (size == 0)
    {
        // no entity sub class

        if (jsonName.empty())
        {
            size_t new_size = 0;
            for (auto& field : Configuration::get()->typeFields(type))
            {
                new_size += field.get_size();
            }
            const_cast<MemoryField*>(this)->size = new_size;
            return size;
        }

        const_cast<MemoryField*>(this)->size = Configuration::get()->getTypeSize(jsonName, type == MemoryFieldType::EntitySubclass);
    }
    return size;
}

bool S2Plugin::Configuration::isPermanentPointer(const std::string& type) const
{
    return mPointerTypes.find(type) != mPointerTypes.end();
}

bool S2Plugin::Configuration::isJsonStruct(const std::string type) const
{
    auto it = mTypeFieldsStructs.find(type);
    if (it != mTypeFieldsStructs.end())
        return true;

    return false;
}

std::string_view S2Plugin::Configuration::getCPPTypeName(MemoryFieldType type)
{
    auto it = gsMemoryFieldType.find(type);
    if (it == gsMemoryFieldType.end())
        return {};

    return it->second.cpp_type_name;
}

std::string_view S2Plugin::Configuration::getTypeDisplayName(MemoryFieldType type)
{
    auto it = gsMemoryFieldType.find(type);
    if (it == gsMemoryFieldType.end())
        return {};

    return it->second.display_name;
}

void S2Plugin::Configuration::processRoomCodesJSON(nlohmann::ordered_json& j)
{
    using namespace std::string_literals;
    std::unordered_map<std::string, QColor> colors;

    auto getColor = [&colors](std::string colorName) -> QColor
    {
        if (auto it = colors.find(colorName); it != colors.end())
        {
            return it->second;
        }
        return QColor(Qt::lightGray);
    };

    for (const auto& [colorName, colorDetails] : j["colors"].items())
    {
        QColor c;
        c.setRed(colorDetails["r"].get<uint8_t>());
        c.setGreen(colorDetails["g"].get<uint8_t>());
        c.setBlue(colorDetails["b"].get<uint8_t>());
        c.setAlpha(colorDetails["a"].get<uint8_t>());
        colors[colorName] = c;
    }
    for (const auto& [roomCodeStr, roomDetails] : j["roomcodes"].items())
    {
        auto id = std::stoi(roomCodeStr, 0, 16);
        QColor color = roomDetails.contains("color") ? getColor(roomDetails["color"].get<std::string>()) : QColor(Qt::lightGray);
        mRoomCodes.emplace(id, RoomCode(id, value_or(roomDetails, "name", "Unnamed room code"s), std::move(color)));
    }
}

S2Plugin::RoomCode S2Plugin::Configuration::roomCodeForID(uint16_t code) const
{
    if (auto it = mRoomCodes.find(code); it != mRoomCodes.end())
    {
        return it->second;
    }
    return RoomCode(code, "Unknown room code", QColor(Qt::lightGray));
}

std::string S2Plugin::Configuration::getEntityName(uint32_t type) const
{
    std::string entityName = "UNKNOWN/DEAD ENTITY";

    if (type > 0 && type <= entityList().highestID())
    {
        entityName = entityList().nameForID(type);
    }
    return entityName;
}

uintptr_t S2Plugin::Configuration::offsetForField(MemoryFieldType type, std::string_view fieldUID, uintptr_t addr) const
{
    return offsetForField(typeFields(type), fieldUID, addr);
}

uintptr_t S2Plugin::Configuration::offsetForField(const std::vector<MemoryField>& fields, std::string_view fieldUID, uintptr_t addr) const
{
    bool last = false;
    size_t currentDelimiter = fieldUID.find('.');

    if (currentDelimiter == std::string::npos)
    {
        last = true;
        currentDelimiter = fieldUID.length();
    }

    auto currentLookupName = fieldUID.substr(0, currentDelimiter);

    auto offset = addr;

    for (auto& field : fields)
    {
        if (field.name == currentLookupName)
        {
            if (last)
            {
                return offset;
            }
            if (field.isPointer)
                offset = Script::Memory::ReadQword(offset);

            if (field.jsonName.empty())
            {
                return offsetForField(typeFieldsOfDefaultStruct(field.jsonName), fieldUID.substr(currentDelimiter + 1), offset);
            }
            else
            {
                return offsetForField(typeFields(field.type), fieldUID.substr(currentDelimiter + 1), offset);
            }
        }
        offset += field.get_size();
    }
    return 0;
}

bool S2Plugin::Configuration::isPointerType(MemoryFieldType type)
{
    auto it = gsMemoryFieldType.find(type);
    if (it == gsMemoryFieldType.end())
        return false;

    return it->second.isPointer;
}
