#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>

using nlohmann::ordered_json;

S2Plugin::Configuration::Configuration()
{
    mSpelunky2 = std::make_unique<Spelunky2>();
    load();
}

void S2Plugin::Configuration::load()
{
    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto path = QFileInfo(QString(buffer)).dir().filePath("plugins/Spelunky2.json");
    if (!QFile(path).exists())
    {
        mErrorString = "Could not find " + path.toStdString();
        mIsValid = false;
        return;
    }

    try
    {
        std::ifstream fp(path.toStdString());
        std::string jsonString((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
        processJSON(jsonString);
    }
    catch (const ordered_json::exception& e)
    {
        mErrorString = "Exception while parsing Spelunky2.json: " + std::string(e.what());
        mIsValid = false;
        return;
    }
    catch (const std::exception& e)
    {
        mErrorString = "Exception while parsing Spelunky2.json: " + std::string(e.what());
        mIsValid = false;
        return;
    }
    catch (...)
    {
        mErrorString = "Unknown exception while parsing Spelunky2.json";
        mIsValid = false;
        return;
    }
    mIsValid = true;
}

bool S2Plugin::Configuration::isValid() const noexcept
{
    return mIsValid;
}

std::string S2Plugin::Configuration::lastError() const noexcept
{
    return mErrorString;
}

void S2Plugin::Configuration::processJSON(const std::string& str)
{
    auto j = ordered_json::parse(str, nullptr, true, true);
    mEntityClassHierarchy.clear();
    const auto& entityClassHierarchy = j["entity_class_hierarchy"];
    for (const auto& [key, jsonValue] : entityClassHierarchy.items())
    {
        auto value = jsonValue.get<std::string>();
        if (key != value)
        {
            mEntityClassHierarchy[key] = value;
        }
    }

    mDefaultEntityClassTypes.clear();
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

    mTypeFieldsEntitySubclasses.clear();
    mTypeFields.clear();
    mTypeFieldsPointers.clear();
    mTypeFieldsInlineStructs.clear();
    mVirtualFunctions.clear();

    const auto& fields = j["fields"];
    for (const auto& [key, jsonArray] : fields.items())
    {
        auto isEntitySubclass = isKnownEntitySubclass(key);
        auto isPointer = (pointerTypes.count(key) > 0);
        auto isInlineStruct = (inlineStructTypes.count(key) > 0);
        if (gsJSONStringToMemoryFieldTypeMapping.count(key) == 0 && !isEntitySubclass && !isPointer && !isInlineStruct)
        {
            throw std::runtime_error("Unknown type specified in fields(1): " + key);
        }
        std::vector<MemoryField> vec;
        for (const auto& field : jsonArray)
        {
            if (field.contains("vftablefunctions"))
            {
                for (const auto& [funcIndex, func] : field["vftablefunctions"].items())
                {
                    VirtualFunction f;
                    f.index = std::stoll(funcIndex);
                    f.name = func.contains("name") ? func["name"].get<std::string>() : "unnamed function";
                    f.params = func.contains("params") ? func["params"].get<std::string>() : "";
                    f.returnValue = func.contains("return") ? func["return"].get<std::string>() : "";
                    f.type = key;
                    mVirtualFunctions[key].emplace_back(f);
                }

                continue;
            }

            MemoryField memField;
            if (isPointer)
            {
                memField.parentPointerJsonName = key;
            }
            if (isInlineStruct)
            {
                memField.parentStructJsonName = key;
            }
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
            if (pointerTypes.count(fieldTypeStr))
            {
                memField.type = MemoryFieldType::PointerType;
                memField.jsonName = fieldTypeStr;
            }
            else if (inlineStructTypes.count(fieldTypeStr))
            {
                memField.type = MemoryFieldType::InlineStructType;
                memField.jsonName = fieldTypeStr;
            }
            else if (fieldTypeStr == "VirtualFunctionTable")
            {
                memField.type = MemoryFieldType::VirtualFunctionTable;
                memField.virtualFunctionTableType = key;
            }
            else if (fieldTypeStr == "StdVector")
            {
                memField.type = MemoryFieldType::StdVector;
                if (field.contains("vectortype"))
                {
                    memField.firstParameterType = field["vectortype"].get<std::string>();
                }
                else
                {
                    memField.firstParameterType = "UnsignedQword";
                    dprintf("No vectortype specified for StdVector %s\n", key.c_str());
                }
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
                if (gsJSONStringToMemoryFieldTypeMapping.count(fieldTypeStr) == 0)
                {
                    throw std::runtime_error("Unknown type specified in fields(2): " + fieldTypeStr);
                }
                memField.type = gsJSONStringToMemoryFieldTypeMapping.at(fieldTypeStr);
            }

            if ((memField.type == MemoryFieldType::Flags32 || memField.type == MemoryFieldType::Flags16 || memField.type == MemoryFieldType::Flags8) &&
                (field.contains("flags") || field.contains("ref")))
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
                mFlagTitles[key + "." + memField.name] = flagTitles;
            }

            if ((memField.type == MemoryFieldType::State8 || memField.type == MemoryFieldType::State16 || memField.type == MemoryFieldType::State32) &&
                (field.contains("states") || field.contains("ref")))
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

            if (memField.type == MemoryFieldType::VirtualFunctionTable && field.contains("functions"))
            {
                for (const auto& [funcIndex, func] : field["functions"].items())
                {
                    VirtualFunction f;
                    f.index = std::stoll(funcIndex);
                    f.name = func.contains("name") ? func["name"].get<std::string>() : "unnamed function";
                    f.params = func.contains("params") ? func["params"].get<std::string>() : "";
                    f.returnValue = func.contains("return") ? func["return"].get<std::string>() : "";
                    f.type = key;
                    mVirtualFunctions[key].emplace_back(f);
                }
            }

            vec.emplace_back(memField);
        }

        if (isPointer)
        {
            mTypeFieldsPointers[key] = vec;
        }
        else if (isInlineStruct)
        {
            mTypeFieldsInlineStructs[key] = vec;
        }
        else if (isEntitySubclass)
        {
            mTypeFieldsEntitySubclasses[key] = vec;
        }
        else
        {
            mTypeFields[gsJSONStringToMemoryFieldTypeMapping.at(key)] = vec;
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
    if (mTypeFieldsPointers.count(type) == 0)
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfPointer() (t=%s)\n", type.c_str());
    }
    return mTypeFieldsPointers.at(type);
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfInlineStruct(const std::string& type) const
{
    if (mTypeFieldsInlineStructs.count(type) == 0)
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfInlineStruct() (t=%s)\n", type.c_str());
    }
    return mTypeFieldsInlineStructs.at(type);
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFields(const MemoryFieldType& type) const
{
    if (mTypeFields.count(type) == 0)
    {
        dprintf("unknown key requested in Configuration::typeFields() (t=%s id=%d)\n", gsMemoryFieldTypeToStringMapping.at(type).c_str(), type);
    }
    return mTypeFields.at(type);
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFieldsOfEntitySubclass(const std::string& type) const
{
    if (mTypeFieldsEntitySubclasses.count(type) == 0)
    {
        dprintf("unknown key requested in Configuration::typeFieldsOfEntitySubclass() (t=%s)\n", type.c_str());
    }
    return mTypeFieldsEntitySubclasses.at(type);
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
    return (gsJSONStringToMemoryFieldTypeMapping.count(type) > 0);
}

S2Plugin::Spelunky2* S2Plugin::Configuration::spelunky2() const noexcept
{
    return mSpelunky2.get();
}

std::string S2Plugin::Configuration::flagTitle(const std::string& fieldName, uint8_t flagNumber)
{
    if (mFlagTitles.count(fieldName) > 0 && flagNumber > 0 && flagNumber <= 32)
    {
        auto& flags = mFlagTitles.at(fieldName);
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
    if (mStateTitles.count(fieldName) > 0)
    {
        auto& states = mStateTitles.at(fieldName);
        if (states.count(state) > 0)
        {
            auto& stateStr = states.at(state);
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
    return mStateTitles.at(fieldName);
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
            if (mVirtualFunctions.count(currentType) > 0)
            {
                for (const auto& f : mVirtualFunctions.at(currentType))
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
        switch (gsJSONStringToMemoryFieldTypeMapping.at(typeName))
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
