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
    const MemoryFieldData gsMemoryFieldType = {
        // MemoryFieldEnum, Name for desplay, c++ type name, name in json, size (if 0 will be determinated from struct)

        // Basic types
        {MemoryFieldType::CodePointer, "Code pointer", "size_t*", "CodePointer", 8},
        {MemoryFieldType::DataPointer, "Data pointer", "size_t*", "DataPointer", 8},
        {MemoryFieldType::Byte, "8-bit", "int8_t", "Byte", 1},
        {MemoryFieldType::UnsignedByte, "8-bit unsigned", "uint8_t", "UnsignedByte", 1},
        {MemoryFieldType::Word, "16-bit", "int16_t", "Word", 2},
        {MemoryFieldType::UnsignedWord, "16-bit unsigned", "uint16_t", "UnsignedWord", 2},
        {MemoryFieldType::Dword, "32-bit", "int32_t", "Dword", 4},
        {MemoryFieldType::UnsignedDword, "32-bit unsigned", "uint32_t", "UnsignedDword", 4},
        {MemoryFieldType::Qword, "64-bit", "int64_t", "Qword", 8},
        {MemoryFieldType::UnsignedQword, "64-bit unsigned", "uint64_t", "UnsignedQword", 8},
        {MemoryFieldType::Float, "Float", "float", "Float", 4},
        {MemoryFieldType::Double, "Double", "double", "Double", 8},
        {MemoryFieldType::Bool, "Bool", "bool", "Bool", 1},
        {MemoryFieldType::Flags8, "8-bit flags", "uint8_t", "Flags8", 1},
        {MemoryFieldType::Flags16, "16-bit flags", "uint16_t", "Flags16", 2},
        {MemoryFieldType::Flags32, "32-bit flags", "uint32_t", "Flags32", 4},
        {MemoryFieldType::State8, "8-bit state", "int8_t", "State8", 1},
        {MemoryFieldType::State16, "16-bit state", "int16_t", "State16", 2},
        {MemoryFieldType::State32, "32-bit state", "int32_t", "State32", 4},
        {MemoryFieldType::UTF16Char, "UTF16Char", "char16_t", "UTF16Char", 2},
        {MemoryFieldType::UTF16StringFixedSize, "UTF16StringFixedSize", "std::array<char16_t, S>", "UTF16StringFixedSize", 0},
        {MemoryFieldType::UTF8StringFixedSize, "UTF8StringFixedSize", "std::array<char, S>", "UTF8StringFixedSize", 0},
        {MemoryFieldType::Skip, "skip", "uint8_t", "Skip", 0},
        // STD lib
        {MemoryFieldType::StdVector, "StdVector", "std::vector<T>", "StdVector", 24},
        {MemoryFieldType::StdMap, "StdMap", "std::map<K, V>", "StdMap", 16},
        {MemoryFieldType::StdSet, "StdSet", "std::set<T>", "StdSet", 16},
        {MemoryFieldType::StdString, "StdString", "std::string", "StdString", 32},
        {MemoryFieldType::StdWstring, "StdWstring", "std::wstring", "StdWstring", 32},
        // Main structs
        {MemoryFieldType::GameManager, "GameManager", "", "GameManager", 0},
        {MemoryFieldType::State, "State", "", "State", 0},
        {MemoryFieldType::SaveGame, "SaveGame", "", "SaveGame", 0},
        {MemoryFieldType::LevelGen, "LevelGen", "", "LevelGen", 0},
        {MemoryFieldType::EntityDB, "EntityDB", "", "EntityDB", 0},
        {MemoryFieldType::ParticleDB, "ParticleDB", "", "ParticleDB", 0},
        {MemoryFieldType::TextureDB, "TextureDB", "", "TextureDB", 0},
        {MemoryFieldType::CharacterDB, "CharacterDB", "", "CharacterDB", 0},
        {MemoryFieldType::Online, "Online", "", "Online", 0},
        // Special Types
        {MemoryFieldType::EntityPointer, "Entity pointer", "Entity*", "EntityPointer", 8},
        {MemoryFieldType::EntityUIDPointer, "Entity UID pointer", "uint32_t*", "EntityUIDPointer", 8},
        {MemoryFieldType::EntityDBPointer, "EntityDB pointer", "EntityDB*", "EntityDBPointer", 8},
        {MemoryFieldType::EntityDBID, "EntityDB ID", "uint32_t", "EntityDBID", 4},
        {MemoryFieldType::EntityUID, "Entity UID", "int32_t", "EntityUID", 4},
        {MemoryFieldType::ParticleDBID, "ParticleDB ID", "uint32_t", "ParticleDBID", 4},
        {MemoryFieldType::ParticleDBPointer, "ParticleDB pointer", "ParticleDB*", "ParticleDBPointer", 8},
        {MemoryFieldType::TextureDBID, "TextureDB ID", "uint32_t", "TextureDBID", 4},
        {MemoryFieldType::TextureDBPointer, "TextureDB pointer", "Texture*", "TextureDBPointer", 8},
        {MemoryFieldType::ConstCharPointer, "Const char*", "const char*", "ConstCharPointer", 8},
        {MemoryFieldType::ConstCharPointerPointer, "Const char**", "const char**", "ConstCharPointerPointer", 8},                         // there is more then just pointer to pointer?
        {MemoryFieldType::UndeterminedThemeInfoPointer, "UndeterminedThemeInfoPointer", "ThemeInfo*", "UndeterminedThemeInfoPointer", 8}, // display theme name and
        {MemoryFieldType::ThemeInfoName, "ThemeInfoName", "ThemeInfo*", "ThemeInfoName", 8},
        {MemoryFieldType::LevelGenRoomsPointer, "LevelGenRoomsPointer", "LevelGenRooms*", "LevelGenRoomsPointer", 8},
        {MemoryFieldType::LevelGenRoomsMetaPointer, "LevelGenRoomsMetaPointer", "LevelGenRoomsMeta*", "LevelGenRoomsMetaPointer", 8},
        {MemoryFieldType::JournalPagePointer, "JournalPagePointer", "JournalPage*", "JournalPagePointer", 8},
        {MemoryFieldType::LevelGenPointer, "LevelGenPointer", "LevelGen*", "LevelGenPointer", 8},
        {MemoryFieldType::StringsTableID, "StringsTableID", "uint32_t", "StringsTableID", 4},
        {MemoryFieldType::CharacterDBID, "CharacterDBID", "uint8_t", "CharacterDBID", 1},
        {MemoryFieldType::VirtualFunctionTable, "VirtualFunctionTable", "size_t*", "VirtualFunctionTable", 8},
        {MemoryFieldType::IPv4Address, "IPv4Address", "uint32_t", "IPv4Address", 4},
        // Other
        //{MemoryFieldType::EntitySubclass, "", "", "", 0},
        //{MemoryFieldType::DefaultStructType, "", "", "", 0},
        {MemoryFieldType::Flag, "Flag", "", "", 0},
    };
}

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
    auto path = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2.json");
    auto pathENT = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2Entities.json");
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

    try
    {
        std::ifstream fp(path.toStdString());
        auto j = ordered_json::parse(fp, nullptr, true, true);
        processJSON(j);
        fp.close();
        std::ifstream fpENT(pathENT.toStdString());
        auto jENT = ordered_json::parse(fpENT, nullptr, true, true);
        processEntitiesJSON(jENT);
    }
    catch (const ordered_json::exception& e)
    {
        displayError("Exception while parsing Spelunky2.json: " + std::string(e.what()));
        initialisedCorrectly = false;
        return;
    }
    catch (const std::exception& e)
    {
        displayError("Exception while parsing Spelunky2.json: " + std::string(e.what()));
        initialisedCorrectly = false;
        return;
    }
    catch (...)
    {
        displayError("Unknown exception while parsing Spelunky2.json");
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

    bool knownPointer = mPointerTypes.count(fieldTypeStr) != 0;

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
            // memField.isPointer = true;
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

            // case MemoryFieldType::CodePointer:
            // case MemoryFieldType::DataPointer:
            // case MemoryFieldType::EntityPointer:
            // case MemoryFieldType::EntityUIDPointer:
            // case MemoryFieldType::EntityDBPointer:
            // case MemoryFieldType::ParticleDBPointer:
            // case MemoryFieldType::TextureDBPointer:
            // case MemoryFieldType::ConstCharPointer:
            // case MemoryFieldType::ConstCharPointerPointer:
            // case MemoryFieldType::UndeterminedThemeInfoPointer:
            // case MemoryFieldType::ThemeInfoName:
            // case MemoryFieldType::LevelGenRoomsPointer:
            // case MemoryFieldType::LevelGenRoomsMetaPointer:
            // case MemoryFieldType::JournalPagePointer:
            // case MemoryFieldType::LevelGenPointer:
            //     memField.isPointer = true;
    }
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
            mEntityClassHierarchy[key] = value;
        }
    }
    for (const auto& [key, jsonValue] : j["default_entity_types"].items())
    {
        auto value = jsonValue.get<std::string>();
        mDefaultEntityClassTypes.emplace_back(key, std::move(value));
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
    std::vector<std::string> returnSet;
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
        std::string p = entityClass;
        while (p != "Entity" && p != "")
        {
            returnSet.emplace_back(p);
            p = mEntityClassHierarchy.at(p);
        }
    }
    returnSet.emplace_back("Entity");
    return returnSet;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfDefaultStruct(const std::string& type) const
{
    auto it = mTypeFieldsStructs.find(type);
    if (it == mTypeFieldsStructs.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfPointer() (t=%s)\n", type.c_str());
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
        dprintf("unknown key requested in Configuration::typeFields() (t=%s id=%d)\n", gsMemoryFieldType.at(type).display_name, type);
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

bool S2Plugin::Configuration::isKnownEntitySubclass(const std::string& typeName) const
{
    if (typeName == "Entity")
    {
        return true;
    }
    return (mEntityClassHierarchy.count(typeName) > 0);
}

std::vector<S2Plugin::VirtualFunction> S2Plugin::Configuration::virtualFunctionsOfType(const std::string& type) const
{
    if (isKnownEntitySubclass(type))
    {
        std::vector<S2Plugin::VirtualFunction> functions;
        std::string currentType = type;
        while (true)
        {
            if (auto it = mVirtualFunctions.find(currentType); it != mVirtualFunctions.end())
            {
                for (const auto& f : it->second)
                {
                    functions.emplace_back(f);
                }
            }
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
        return sizeof(intptr_t);
    }
    if (auto type = getBuiltInType(typeName); type != MemoryFieldType::None)
    {
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
            case MemoryFieldType::CharacterDB: // biggest variable is 4
                return sizeof(int32_t);

            case MemoryFieldType::Online:
            case MemoryFieldType::TextureDB:
            case MemoryFieldType::ParticleDB:
            case MemoryFieldType::EntityDB:
            case MemoryFieldType::LevelGen:
            case MemoryFieldType::GameManager:
            case MemoryFieldType::State:
            case MemoryFieldType::SaveGame:
            case MemoryFieldType::ThemeInfoName:
            case MemoryFieldType::UndeterminedThemeInfoPointer:
            case MemoryFieldType::LevelGenRoomsPointer:
            case MemoryFieldType::LevelGenRoomsMetaPointer:
            case MemoryFieldType::JournalPagePointer:
            case MemoryFieldType::LevelGenPointer:
            case MemoryFieldType::VirtualFunctionTable:
            case MemoryFieldType::EntityPointer:
            case MemoryFieldType::EntityUIDPointer:
            case MemoryFieldType::EntityDBPointer:
            case MemoryFieldType::ParticleDBPointer:
            case MemoryFieldType::TextureDBPointer:
            case MemoryFieldType::ConstCharPointer:
            case MemoryFieldType::ConstCharPointerPointer:
            case MemoryFieldType::StdVector:
            case MemoryFieldType::StdMap:
            case MemoryFieldType::StdSet:
            case MemoryFieldType::CodePointer:
            case MemoryFieldType::DataPointer:
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

// TODO review this function and it's uses
size_t S2Plugin::Configuration::setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets,
                                                  bool advanceOffset) const
{
    offsets[fieldNameOverride] = offset;
    if (!advanceOffset)
    {
        return offset;
    }

    if (field.isPointer)
    {
        if (field.type == MemoryFieldType::DefaultStructType)
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : typeFieldsOfDefaultStruct(field.jsonName))
            {
                auto newOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, pointerOffset, offsets);
                if (pointerOffset != 0)
                {
                    pointerOffset = newOffset;
                }
            }
        }
        offset += 8;
        return offset;
    }

    switch (field.type)
    {
        case MemoryFieldType::Flag:
            break;
        case MemoryFieldType::Skip:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
            offset += field.size;
            break;
        case MemoryFieldType::Bool:
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        case MemoryFieldType::State8:
        case MemoryFieldType::CharacterDBID:
            offset += 1;
            break;
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::State16:
        case MemoryFieldType::UTF16Char:
            offset += 2;
            break;
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
            offset += 4;
            break;
        case MemoryFieldType::CodePointer:
        case MemoryFieldType::DataPointer:
        case MemoryFieldType::EntityDBPointer:          // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::TextureDBPointer:         // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::EntityPointer:            // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::EntityUIDPointer:         // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::ParticleDBPointer:        // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenPointer:          // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenRoomsPointer:     // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::LevelGenRoomsMetaPointer: // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::JournalPagePointer:       // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::ThemeInfoName:            // not shown inline in the treeview, so just skip sizeof(size_t)
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::ConstCharPointerPointer:
        case MemoryFieldType::ConstCharPointer:
        case MemoryFieldType::VirtualFunctionTable:
        case MemoryFieldType::Double:
            offset += 8;
            break;
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : typeFieldsOfDefaultStruct("ThemeInfoPointer"))
            {
                auto newOffset = setOffsetForField(f, fieldNameOverride + "." + f.name, pointerOffset, offsets);
                if (pointerOffset != 0)
                {
                    pointerOffset = newOffset;
                }
            }
            offset += 8;
            break;
        }
        case MemoryFieldType::DefaultStructType:
        {
            for (const auto& f : typeFieldsOfDefaultStruct(field.jsonName))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        case MemoryFieldType::EntitySubclass:
        {
            for (const auto& f : typeFieldsOfEntitySubclass(field.jsonName))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
        default:
        {
            for (const auto& f : typeFields(field.type))
            {
                offset = setOffsetForField(f, fieldNameOverride + "." + f.name, offset, offsets);
            }
            break;
        }
    }
    return offset;
}

// size_t S2Plugin::Configuration::sizeOf(const std::string& typeName) const
//{
//     if (isPointer(typeName))
//     {
//         return sizeof(size_t);
//     }
//     else if (isBuiltInType(typeName))
//     {
//         MemoryField tmp;
//         tmp.type = gsMemoryFieldType.find(typeName)->first;
//         std::unordered_map<std::string, size_t> offsetsDummy;
//         return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
//     }
//     else if (isInlineStruct(typeName))
//     {
//         MemoryField tmp;
//         tmp.type = MemoryFieldType::InlineStructType;
//         tmp.jsonName = typeName;
//         std::unordered_map<std::string, size_t> offsetsDummy;
//         return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
//     }
//     else
//     {
//         return 0;
//     }
// }

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
            return json_it->second->second.size;

        return 0;
    }

    size_t struct_size{0};
    for (auto& field : it->second)
        struct_size += field.get_size();

    mTypeFieldsStructsSizes[typeName] = struct_size;
    return struct_size;
}

size_t S2Plugin::MemoryField::get_size()
{
    if (isPointer)
        return sizeof(uintptr_t);

    if (size == 0)
    {
        if (type == MemoryFieldType::EntitySubclass)
            size = Configuration::get()->getTypeSize(jsonName, true);
        else
            size = Configuration::get()->getTypeSize(jsonName, false);
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
