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
        // MemoryFieldEnum, Name for desplay, c++ type name, name in json
        {MemoryFieldType::CodePointer, "Code pointer", "size_t*", "CodePointer"},
        {MemoryFieldType::DataPointer, "Data pointer", "size_t*", "DataPointer"},
        {MemoryFieldType::Byte, "8-bit", "int8_t", "Byte"},
        {MemoryFieldType::UnsignedByte, "8-bit unsigned", "uint8_t", "UnsignedByte"},
        {MemoryFieldType::Word, "16-bit", "int16_t", "Word"},
        {MemoryFieldType::UnsignedWord, "16-bit unsigned", "uint16_t", "UnsignedWord"},
        {MemoryFieldType::Dword, "32-bit", "int32_t", "Dword"},
        {MemoryFieldType::UnsignedDword, "32-bit unsigned", "uint32_t", "UnsignedDword"},
        {MemoryFieldType::Qword, "64-bit", "int64_t", "Qword"},
        {MemoryFieldType::UnsignedQword, "64-bit unsigned", "uint64_t", "UnsignedQword"},
        {MemoryFieldType::Float, "Float", "float", "Float"},
        {MemoryFieldType::Bool, "Bool", "bool", "Bool"},
        {MemoryFieldType::Flag, "Flag", "", ""},
        {MemoryFieldType::Flags32, "32-bit flags", "uint32_t", "Flags32"},
        {MemoryFieldType::Flags16, "16-bit flags", "uint16_t", "Flags16"},
        {MemoryFieldType::Flags8, "8-bit flags", "uint8_t", "Flags8"},
        {MemoryFieldType::State8, "8-bit state", "int8_t", "State8"},
        {MemoryFieldType::State16, "16-bit state", "int16_t", "State16"},
        {MemoryFieldType::State32, "32-bit state", "int32_t", "State32"},
        {MemoryFieldType::Skip, "skip", "uint8_t", "Skip"},
        {MemoryFieldType::GameManager, "GameManager", "", "GameManager"},
        {MemoryFieldType::State, "State", "", "State"},
        {MemoryFieldType::SaveGame, "SaveGame", "", "SaveGame"},
        {MemoryFieldType::LevelGen, "LevelGen", "", "LevelGen"},
        {MemoryFieldType::EntityDB, "EntityDB", "", "EntityDB"},
        {MemoryFieldType::EntityPointer, "Entity pointer", "Entity*", "EntityPointer"},
        {MemoryFieldType::EntityUIDPointer, "Entity UID pointer", "uint32_t*", "EntityUIDPointer"},
        {MemoryFieldType::EntityDBPointer, "EntityDB pointer", "EntityDB*", "EntityDBPointer"},
        {MemoryFieldType::EntityDBID, "EntityDB ID", "uint32_t", "EntityDBID"},
        {MemoryFieldType::EntityUID, "Entity UID", "int32_t", "EntityUID"},
        {MemoryFieldType::ParticleDB, "ParticleDB", "ParticleDB*", "ParticleDB"},
        {MemoryFieldType::ParticleDBID, "ParticleDB ID", "uint32_t", "ParticleDBID"},
        {MemoryFieldType::ParticleDBPointer, "ParticleDB pointer", "ParticleDB**", "ParticleDBPointer"},
        {MemoryFieldType::TextureDB, "TextureDB", "TextureDB*", "TextureDB"},
        {MemoryFieldType::TextureDBID, "TextureDB ID", "uint32_t", "TextureDBID"},
        {MemoryFieldType::TextureDBPointer, "TextureDB pointer", "Texture*", "TextureDBPointer"},
        {MemoryFieldType::Vector, "Vector", "Vector", "Vector"},
        {MemoryFieldType::StdVector, "StdVector", "std::vector<T>", "StdVector"},
        {MemoryFieldType::StdMap, "StdMap", "std::map<K, V>", "StdMap"},
        {MemoryFieldType::StdSet, "StdSet", "std::set<T>", "StdSet"},
        {MemoryFieldType::ConstCharPointerPointer, "Const char**", "const char**", "ConstCharPointerPointer"},
        {MemoryFieldType::ConstCharPointer, "Const char*", "const char*", "ConstCharPointer"},
        {MemoryFieldType::StdString, "StdString", "std::string", "StdString"},
        {MemoryFieldType::StdWstring, "StdWstring", "std::wstring", "StdWstring"},
        //{MemoryFieldType::EntitySubclass, "", "", ""},
        {MemoryFieldType::PointerType, "Pointer", "", ""},
        {MemoryFieldType::InlineStructType, "Inline struct", "", ""},
        {MemoryFieldType::UndeterminedThemeInfoPointer, "UndeterminedThemeInfoPointer", "ThemeInfo*", "UndeterminedThemeInfoPointer"},
        {MemoryFieldType::LevelGenRoomsPointer, "LevelGenRoomsPointer", "LevelGenRooms*", "LevelGenRoomsPointer"},
        {MemoryFieldType::LevelGenRoomsMetaPointer, "LevelGenRoomsMetaPointer", "LevelGenRoomsMeta*", "LevelGenRoomsMetaPointer"},
        {MemoryFieldType::JournalPagePointer, "JournalPagePointer", "JournalPage*", "JournalPagePointer"},
        {MemoryFieldType::ThemeInfoName, "ThemeInfoName", "ThemeInfo*", "ThemeInfoName"},
        {MemoryFieldType::LevelGenPointer, "LevelGenPointer", "LevelGen*", "LevelGenPointer"},
        {MemoryFieldType::UTF16Char, "UTF16Char", "char16_t", "UTF16Char"},
        {MemoryFieldType::UTF16StringFixedSize, "UTF16StringFixedSize", "std::array<char16_t, S>", "UTF16StringFixedSize"},
        {MemoryFieldType::UTF8StringFixedSize, "UTF8StringFixedSize", "std::array<char, S>", "UTF8StringFixedSize"},
        {MemoryFieldType::StringsTableID, "StringsTableID", "uint32_t", "StringsTableID"},
        {MemoryFieldType::CharacterDB, "CharacterDB", "CharacterDB*", "CharacterDB"},
        {MemoryFieldType::CharacterDBID, "CharacterDBID", "uint8_t", "CharacterDBID"},
        {MemoryFieldType::VirtualFunctionTable, "VirtualFunctionTable", "size_t*", "VirtualFunctionTable"},
        {MemoryFieldType::Online, "Online", "", "Online"},
        {MemoryFieldType::IPv4Address, "IPv4Address", "uint32_t", "IPv4Address"},
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

bool S2Plugin::Configuration::reset()
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
    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto path = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2.json");
    if (!QFile(path).exists())
    {
        displayError("Could not find " + path.toStdString());
        initialisedCorrectly = false;
        return;
    }

    try
    {
        std::ifstream fp(path.toStdString());
        auto j = ordered_json::parse(fp, nullptr, true, true);
        processJSON(j);
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

void S2Plugin::Configuration::processJSON(ordered_json& j)
{
    using namespace std::string_literals;

    const auto& entityClassHierarchy = j["entity_class_hierarchy"];
    for (const auto& [key, jsonValue] : entityClassHierarchy.items())
    {
        auto value = jsonValue.get<std::string>();
        if (key != value)
        {
            mEntityClassHierarchy[key] = value;
        }
    }
    const auto& defaultEntityTypes = j["default_entity_types"];
    for (const auto& [key, jsonValue] : defaultEntityTypes.items())
    {
        auto value = jsonValue.get<std::string>();
        mDefaultEntityClassTypes.emplace_back(std::make_pair(key, value));
    }

    std::unordered_set<std::string> pointerTypes;
    for (const auto& t : j["pointer_types"])
    {
        pointerTypes.insert(t.get<std::string>());
    }

    std::unordered_set<std::string> inlineStructTypes;
    for (const auto& t : j["inline_struct_types"])
    {
        inlineStructTypes.insert(t.get<std::string>());
    }
    for (const auto& [key, jsonValue] : j["struct_alignments"].items())
    {
        mAlignments.insert({key, jsonValue.get<uint8_t>()});
    }

    const auto& fields = j["fields"];
    for (const auto& [key, jsonArray] : fields.items())
    {
        auto isEntitySubclass = isKnownEntitySubclass(key);
        auto isPointer = (pointerTypes.count(key) > 0);
        auto isInlineStruct = (inlineStructTypes.count(key) > 0);
        // if (!gsMemoryFieldType.contains(key) && !isEntitySubclass && !isPointer && !isInlineStruct)
        //{
        //     throw std::runtime_error("Type not declared in pointer_types nor inline_struct_types: " + key);
        // }
        std::vector<MemoryField> vec;
        for (const auto& field : jsonArray)
        {
            if (field.contains("vftablefunctions")) // for the vtable in entity subclasses
            {
                for (const auto& [funcIndex, func] : field["vftablefunctions"].items())
                {
                    VirtualFunction f;
                    f.index = std::stoll(funcIndex);
                    f.name = value_or(func, "name", "unnamed function"s);
                    f.name = value_or(func, "name", "unnamed function"s);
                    f.params = value_or(func, "params", ""s);
                    f.returnValue = value_or(func, "return", "void"s);
                    f.type = key;
                    mVirtualFunctions[key].emplace_back(f);
                }
                continue;
            }

            MemoryField memField;

            memField.name = field["field"].get<std::string>();
            if (field.contains("offset"))
            {
                memField.extraInfo = field["offset"].get<uint64_t>();
            }
            if (field.contains("comment"))
            {
                memField.comment = field["comment"].get<std::string>();
            }

            auto fieldTypeStr = field["type"].get<std::string>();
            if (pointerTypes.count(fieldTypeStr) || value_or(field, "Pointer", false))
            {
                memField.type = MemoryFieldType::PointerType;
                memField.jsonName = fieldTypeStr;
            }
            else if (inlineStructTypes.count(fieldTypeStr))
            {
                memField.type = MemoryFieldType::InlineStructType;
                memField.jsonName = fieldTypeStr;
            }
            else if (fieldTypeStr == "StdMap")
            {
                memField.type = MemoryFieldType::StdMap;
                if (field.contains("keytype"))
                {
                    memField.firstParameterType = field["keytype"].get<std::string>();
                }
                else
                {
                    memField.firstParameterType = "UnsignedQword";
                    dprintf("No keytype specified for StdMap %s\n", key.c_str());
                }
                if (field.contains("valuetype"))
                {
                    memField.secondParameterType = field["valuetype"].get<std::string>();
                }
                else
                {
                    memField.secondParameterType = "UnsignedQword";
                    dprintf("No valuetype specified for StdMap %s\n", key.c_str());
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
                    dprintf("No keytype specified for StdSet %s\n", key.c_str());
                }
            }
            else
            {
                auto it = gsMemoryFieldType.find(fieldTypeStr);
                if (it == gsMemoryFieldType.end())
                {
                    throw std::runtime_error("Unknown type specified in fields(2): " + fieldTypeStr);
                }
                memField.type = it->first;
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
                        dprintf("No vectortype specified for StdVector %s\n", key.c_str());
                    }
                    break;
                }
                case MemoryFieldType::Skip:
                {
                    memField.size = memField.extraInfo;
                    break;
                }
                case MemoryFieldType::Flags32:
                case MemoryFieldType::Flags16:
                case MemoryFieldType::Flags8:
                {
                    if (field.contains("flags") || field.contains("ref")) // TODO: what if they are missing?
                    {
                        nlohmann::json flagsObject;
                        if (field.contains("flags"))
                        {
                            flagsObject = field["flags"];
                        }
                        else
                        {
                            auto ref = field["ref"].get<std::string>();
                            if (j["refs"].contains(ref))
                            {
                                flagsObject = j["refs"][ref];
                            }
                        }

                        std::unordered_map<uint8_t, std::string> flagTitles;
                        for (const auto& [flagNumber, flagTitle] : flagsObject.items())
                        {
                            flagTitles[std::stoi(flagNumber)] = flagTitle.get<std::string>();
                        }
                        mFlagTitles[key + "." + memField.name] = flagTitles; // TODO save inside memfield?
                    }
                    break;
                }
                case MemoryFieldType::State8:
                case MemoryFieldType::State16:
                case MemoryFieldType::State32:
                {
                    if (field.contains("states") || field.contains("ref")) // TODO: what if they are missing?
                    {
                        nlohmann::json statesObject;
                        if (field.contains("states"))
                        {
                            statesObject = field["states"];
                        }
                        else
                        {
                            auto ref = field["ref"].get<std::string>();
                            if (j["refs"].contains(ref))
                            {
                                statesObject = j["refs"][ref];
                            }
                        }
                        std::unordered_map<int64_t, std::string> stateTitles;
                        for (const auto& [state, stateTitle] : statesObject.items())
                        {
                            stateTitles[std::stoll(state)] = stateTitle.get<std::string>();
                        }
                        mStateTitles[key + "." + memField.name] = stateTitles;
                    }
                    break;
                }
                case MemoryFieldType::VirtualFunctionTable:
                {
                    memField.virtualFunctionTableType = key;
                    if (field.contains("functions"))
                    {
                        for (const auto& [funcIndex, func] : field["functions"].items())
                        {
                            VirtualFunction f;
                            f.index = std::stoll(funcIndex);
                            f.name = value_or(func, "name", "unnamed function"s);
                            f.params = value_or(func, "params", ""s);
                            f.returnValue = value_or(func, "return", "void"s);
                            f.type = key;
                            mVirtualFunctions[key].emplace_back(f);
                        }
                    }
                    break;
                }
            }
            vec.emplace_back(memField);
        }

        if (isPointer)
        {
            mTypeFieldsPointers[key] = std::move(vec);
        }
        else if (isInlineStruct)
        {
            mTypeFieldsInlineStructs[key] = std::move(vec);
        }
        else if (isEntitySubclass)
        {
            mTypeFieldsEntitySubclasses[key] = std::move(vec);
        }
        else
        {
            mTypeFields[gsMemoryFieldType.find(key)->first] = std::move(vec);
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

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfPointer(const std::string& type) const
{
    auto it = mTypeFieldsPointers.find(type);
    if (it == mTypeFieldsPointers.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfPointer() (t=%s)\n", type.c_str());
    }
    return it->second;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfInlineStruct(const std::string& type) const
{
    auto it = mTypeFieldsInlineStructs.find(type);
    if (it == mTypeFieldsInlineStructs.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfInlineStruct() (t=%s)\n", type.c_str());
    }
    return it->second;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFields(const MemoryFieldType& type) const
{
    auto it = mTypeFields.find(type);
    if (it == mTypeFields.end())
    {
        dprintf("unknown key requested in Configuration::typeFields() (t=%s id=%d)\n", gsMemoryFieldType.at(type).display_name, type);
    }
    return it->second;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfEntitySubclass(const std::string& type) const
{
    auto it = mTypeFieldsEntitySubclasses.find(type);
    if (it == mTypeFieldsEntitySubclasses.end())
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfEntitySubclass() (t=%s)\n", type.c_str());
    }
    return it->second;
}

bool S2Plugin::Configuration::isEntitySubclass(const std::string& type) const
{
    return (mTypeFieldsEntitySubclasses.count(type) > 0);
}

bool S2Plugin::Configuration::isPointer(const std::string& type) const
{
    return (mTypeFieldsPointers.count(type) > 0);
}

bool S2Plugin::Configuration::isInlineStruct(const std::string& type) const
{
    return (mTypeFieldsInlineStructs.count(type) > 0);
}

bool S2Plugin::Configuration::isBuiltInType(const std::string& type) const
{
    return gsMemoryFieldType.contains(type);
}

std::string S2Plugin::Configuration::flagTitle(const std::string& fieldName, uint8_t flagNumber)
{
    if (auto it = mFlagTitles.find(fieldName); it != mFlagTitles.end() && flagNumber > 0 && flagNumber <= 32)
    {
        auto& flags = it->second;
        auto& flagStr = flags.at(flagNumber);
        if (flagStr.empty())
        {
            return "Unknown";
        }
        return flagStr;
    }
    return "Unknown flag (" + fieldName + ":" + std::to_string(flagNumber) + ")";
}

std::string S2Plugin::Configuration::stateTitle(const std::string& fieldName, int64_t state)
{
    if (auto it = mStateTitles.find(fieldName); it != mStateTitles.end())
    {
        auto& states = it->second;
        if (auto states_it = states.find(state); states_it != states.end())
        {
            auto& stateStr = states_it->second;
            if (!stateStr.empty())
            {
                return stateStr;
            }
        }
    }
    return "UNKNOWN STATE";
}

const std::unordered_map<int64_t, std::string>& S2Plugin::Configuration::stateTitlesOfField(const std::string& fieldName)
{
    return mStateTitles.at(fieldName); // no error handling?
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
    bool check_aligment = false;
    if (isPointer(typeName))
    {
        return sizeof(size_t);
    }
    else if (isBuiltInType(typeName))
    {
        switch (gsMemoryFieldType.find(typeName)->first)
        {
                /*case MemoryFieldType::EntitySubclass:
                case MemoryFieldType::Flag:*/

            case MemoryFieldType::Skip:
            {
                dprintf("Cannot determinate alignment of \"Skip\" element!\n");
                return 0;
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
            case MemoryFieldType::PointerType:
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
            case MemoryFieldType::Vector:
            case MemoryFieldType::StdVector:
            case MemoryFieldType::StdMap:
            case MemoryFieldType::StdSet:
            case MemoryFieldType::CodePointer:
            case MemoryFieldType::DataPointer:
            case MemoryFieldType::Qword:
            case MemoryFieldType::UnsignedQword:
                return sizeof(size_t);
            case MemoryFieldType::InlineStructType:
            default:
            {
                check_aligment = true;
            }
        }
    }
    if (check_aligment || isInlineStruct(typeName))
    {
        auto itr = mAlignments.find(typeName);
        if (itr == mAlignments.end())
            dprintf("Alignment not found for '%s'\n", typeName.c_str());
        else
        {
            if (itr->second > sizeof(size_t))
            {
                dprintf("Wrong alignment provided (%d) for struct (%s), allowed range: 0-8\n", itr->second, itr->first);
                return sizeof(size_t);
            }
            return itr->second;
        }
    }
    return 0;
}

size_t S2Plugin::Configuration::setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets,
                                                  bool advanceOffset) const
{
    offsets[fieldNameOverride] = offset;
    if (!advanceOffset)
    {
        return offset;
    }

    switch (field.type)
    {
        case MemoryFieldType::Flag:
            break;
        case MemoryFieldType::Skip:
        case MemoryFieldType::UTF16StringFixedSize:
        case MemoryFieldType::UTF8StringFixedSize:
            offset += field.extraInfo;
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
            offset += 8;
            break;
        case MemoryFieldType::UndeterminedThemeInfoPointer:
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : typeFieldsOfPointer("ThemeInfoPointer"))
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
        case MemoryFieldType::PointerType:
        {
            size_t pointerOffset = Script::Memory::ReadQword(offset);
            for (const auto& f : typeFieldsOfPointer(field.jsonName))
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
        case MemoryFieldType::InlineStructType:
        {
            for (const auto& f : typeFieldsOfInlineStruct(field.jsonName))
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

size_t S2Plugin::Configuration::sizeOf(const std::string& typeName) const
{
    if (isPointer(typeName))
    {
        return sizeof(size_t);
    }
    else if (isBuiltInType(typeName))
    {
        MemoryField tmp;
        tmp.type = gsMemoryFieldType.find(typeName)->first;
        std::unordered_map<std::string, size_t> offsetsDummy;
        return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
    }
    else if (isInlineStruct(typeName))
    {
        MemoryField tmp;
        tmp.type = MemoryFieldType::InlineStructType;
        tmp.jsonName = typeName;
        std::unordered_map<std::string, size_t> offsetsDummy;
        return setOffsetForField(tmp, "dummy", 0, offsetsDummy, true);
    }
    else
    {
        return 0;
    }
}
