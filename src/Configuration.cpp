#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <fstream>

S2Plugin::Configuration::Configuration()
{
    mSpelunky2 = std::make_unique<Spelunky2>();

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
        auto j = ordered_json::parse(jsonString, nullptr, true, true);
        processJSON(j);
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

void S2Plugin::Configuration::processJSON(const ordered_json& j)
{
    mEntityClassHierarchy.clear();
    auto entityClassHierarchy = j["entity_class_hierarchy"];
    for (const auto& [key, jsonValue] : entityClassHierarchy.items())
    {
        if (gsJSONStringToMemoryFieldTypeMapping.count(key) == 0)
        {
            throw std::runtime_error("Unknown type specified in entity_class_hierarchy: " + key);
        }
        auto value = jsonValue.get<std::string>();
        if (gsJSONStringToMemoryFieldTypeMapping.count(value) == 0)
        {
            throw std::runtime_error("Unknown type specified in entity_class_hierarchy: " + value);
        }

        mEntityClassHierarchy[gsJSONStringToMemoryFieldTypeMapping.at(key)] = gsJSONStringToMemoryFieldTypeMapping.at(value);
    }

    auto defaultEntityTypes = j["default_entity_types"];
    for (const auto& [key, jsonValue] : defaultEntityTypes.items())
    {
        auto value = jsonValue.get<std::string>();
        if (gsJSONStringToMemoryFieldTypeMapping.count(value) == 0)
        {
            throw std::runtime_error("Unknown type specified in default_entity_types: " + value);
        }
        mDefaultEntityClassTypes[key] = gsJSONStringToMemoryFieldTypeMapping.at(value);
    }

    auto fields = j["fields"];
    for (const auto& [key, jsonArray] : fields.items())
    {
        if (gsJSONStringToMemoryFieldTypeMapping.count(key) == 0)
        {
            throw std::runtime_error("Unknown type specified in fields: " + key);
        }
        std::vector<MemoryField> vec;
        for (const auto& field : jsonArray)
        {
            MemoryField memField;
            memField.name = field["field"].get<std::string>();
            if (field.contains("offset"))
            {
                memField.extraInfo = field["offset"].get<uint64_t>();
            }
            auto fieldTypeStr = field["type"].get<std::string>();
            if (gsJSONStringToMemoryFieldTypeMapping.count(fieldTypeStr) == 0)
            {
                throw std::runtime_error("Unknown type specified in fields: " + fieldTypeStr);
            }
            memField.type = gsJSONStringToMemoryFieldTypeMapping.at(fieldTypeStr);

            if (memField.type == MemoryFieldType::Flags32 && field.contains("flags"))
            {
                auto flagsObject = field["flags"];
                std::unordered_map<uint8_t, std::string> flagTitles;
                for (const auto& [flagNumber, flagTitle] : flagsObject.items())
                {
                    flagTitles[std::stoi(flagNumber)] = flagTitle.get<std::string>();
                }
                mFlagTitles[key + "." + memField.name] = flagTitles;
            }

            vec.emplace_back(memField);
        }
        mTypeFields[gsJSONStringToMemoryFieldTypeMapping.at(key)] = vec;
    }
}

const std::unordered_map<S2Plugin::MemoryFieldType, S2Plugin::MemoryFieldType>& S2Plugin::Configuration::entityClassHierarchy() const noexcept
{
    return mEntityClassHierarchy;
}

const std::unordered_map<std::string, S2Plugin::MemoryFieldType>& S2Plugin::Configuration::defaultEntityClassTypes() const noexcept
{
    return mDefaultEntityClassTypes;
}

const std::vector<S2Plugin::MemoryField>& S2Plugin::Configuration::typeFields(const MemoryFieldType& type) const
{
    if (mTypeFields.count(type) == 0)
    {
        dprintf("unknown key requested in Configuration::typeFields() (t=%s id=%d)\n", gsMemoryFieldTypeToStringMapping.at(type).c_str(), type);
    }
    return mTypeFields.at(type);
}

S2Plugin::Spelunky2* S2Plugin::Configuration::spelunky2() const noexcept
{
    return mSpelunky2.get();
}

std::string S2Plugin::Configuration::flagTitle(const std::string& fieldName, uint8_t flagNumber)
{
    if (mFlagTitles.count(fieldName) > 0 && flagNumber > 0 && flagNumber <= 32)
    {
        auto flags = mFlagTitles.at(fieldName);
        auto flagStr = flags.at(flagNumber);
        if (flagStr.empty())
        {
            return "Unknown";
        }
        return flagStr;
    }
    return "Unknown flag (" + fieldName + ":" + std::to_string(flagNumber) + ")";
}
