#pragma once

#include <QMetaEnum>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    constexpr uint8_t gsColField = 0;
    constexpr uint8_t gsColValue = 1;
    constexpr uint8_t gsColValueHex = 2;
    constexpr uint8_t gsColComparisonValue = 3;
    constexpr uint8_t gsColComparisonValueHex = 4;
    constexpr uint8_t gsColMemoryOffset = 5;
    constexpr uint8_t gsColMemoryOffsetDelta = 6;
    constexpr uint8_t gsColType = 7;
    constexpr uint8_t gsColComment = 8;

    constexpr uint16_t gsRoleField = Qt::UserRole + gsColField;
    constexpr uint16_t gsRoleValue = Qt::UserRole + gsColValue;
    constexpr uint16_t gsRoleValueHex = Qt::UserRole + gsColValueHex;
    constexpr uint16_t gsRoleComparisonValue = Qt::UserRole + gsColComparisonValue;
    constexpr uint16_t gsRoleComparisonValueHex = Qt::UserRole + gsColComparisonValueHex;
    constexpr uint16_t gsRoleType = Qt::UserRole + gsColMemoryOffset;
    constexpr uint16_t gsRoleMemoryOffset = Qt::UserRole + gsColType;
    constexpr uint16_t gsRoleRawValue = Qt::UserRole + 10;
    constexpr uint16_t gsRoleRawComparisonValue = Qt::UserRole + 11;
    constexpr uint16_t gsRoleUID = Qt::UserRole + 12;
    constexpr uint16_t gsRoleFlagIndex = Qt::UserRole + 13;
    constexpr uint16_t gsRoleFlagFieldName = Qt::UserRole + 14;
    constexpr uint16_t gsRoleFlagsSize = Qt::UserRole + 15;
    constexpr uint16_t gsRoleFieldName = Qt::UserRole + 16;
    constexpr uint16_t gsRoleStdContainerFirstParameterType = Qt::UserRole + 17;
    constexpr uint16_t gsRoleBaseFieldName = Qt::UserRole + 18;
    constexpr uint16_t gsRoleEntireMemoryField = Qt::UserRole + 19;
    constexpr uint16_t gsRoleStdContainerSecondParameterType = Qt::UserRole + 20;

    constexpr char* gsJSONDragDropMemoryField_UID = "uid";
    constexpr char* gsJSONDragDropMemoryField_Offset = "offset";
    constexpr char* gsJSONDragDropMemoryField_Type = "type";

    // new types need to be added to
    // - the MemoryFieldType enum
    // - gsMemoryFieldType in .cpp
    // - Spelunky2.json
    // new subclasses of Entity can just be added to the class hierarchy in Spelunky2.json
    // and have its fields defined there

    enum class MemoryFieldType
    {
        CodePointer,
        DataPointer,
        Byte,
        UnsignedByte,
        Word,
        UnsignedWord,
        Dword,
        UnsignedDword,
        Qword,
        UnsignedQword,
        Float,
        Bool,
        Flag,
        Flags32,
        Flags16,
        Flags8,
        State8,  // this is signed, can be negative!
        State16, // this is signed, can be negative!
        State32, // this is signed, can be negative!
        Skip,
        GameManager,
        State,
        SaveGame,
        LevelGen,
        EntityDB,
        EntityPointer,
        EntityUIDPointer,
        EntityDBPointer,
        EntityDBID,
        EntityUID,
        ParticleDB,
        ParticleDBID,
        ParticleDBPointer,
        TextureDB,
        TextureDBID,
        TextureDBPointer,
        Vector,
        StdVector,
        StdMap,
        StdSet,
        ConstCharPointer,
        ConstCharPointerPointer,
        StdString,
        StdWstring,
        EntitySubclass,               // a subclass of an entity defined in json
        PointerType,                  // a pointer defined in json
        InlineStructType,             // an inline struct defined in json
        UndeterminedThemeInfoPointer, // used to look up the theme pointer in the levelgen and show the correct theme name
        LevelGenRoomsPointer,         // used to make the level gen rooms title clickable
        LevelGenRoomsMetaPointer,     // used to make the level gen rooms title clickable
        JournalPagePointer,           // used to make journal page in vector clickable
        ThemeInfoName,
        LevelGenPointer,
        UTF16Char,
        UTF16StringFixedSize,
        UTF8StringFixedSize,
        StringsTableID,
        CharacterDB,
        CharacterDBID,
        VirtualFunctionTable,
        Online,
        IPv4Address,
    };

    class MemoryFieldData
    {
      public:
        struct Data
        {
            std::string_view display_name;
            std::string_view cpp_type_name;
        };

        using value_type = std::tuple<MemoryFieldType, const char*, const char*, const char*>;
        using map_type = std::map<MemoryFieldType, Data>;

        MemoryFieldData(std::initializer_list<value_type> init)
        {
            // MemoryFieldType type, const char* d_name, const char* cpp_type, const char* j_name
            for (auto& val : init)
            {
                auto it = fields.emplace(std::get<0>(val), Data{std::get<1>(val), std::get<2>(val)});
                if (it.second)
                {
                    auto size = strlen(std::get<3>(val));
                    if (size != 0)
                    {
                        json_names_map.emplace(std::get<3>(val), it.first);
                    }
                }
            }
        };

        map_type::const_iterator find(const MemoryFieldType& key) const
        {
            return fields.find(key);
        }
        map_type::const_iterator find(const std::string_view key) const
        {
            auto it = json_names_map.find(key);
            if (it == json_names_map.end())
                return fields.end();

            return it->second;
        }
        map_type::const_iterator end() const
        {
            return fields.end();
        }
        map_type::const_iterator begin() const
        {
            return fields.begin();
        }
        const Data& at(const MemoryFieldType& key) const
        {
            return fields.at(key);
        }
        const Data& at(const std::string_view key) const
        {
            return json_names_map.at(key)->second;
        }
        bool contains(const MemoryFieldType& key) const
        {
            return fields.count(key) != 0;
        }
        bool contains(const std::string_view key) const
        {
            return json_names_map.count(key) != 0;
        }

        map_type fields;
        std::unordered_map<std::string_view, map_type::iterator> json_names_map;
    };

    struct VirtualFunction
    {
        size_t index;
        std::string name;
        std::string params;
        std::string returnValue;
        std::string type;
    };

    enum class VIRT_FUNC
    {
        ENTITY_STATEMACHINE = 2,
        ENTITY_KILL = 3,
        ENTITY_COLLISION1 = 4,
        ENTITY_DESTROY = 5,
        ENTITY_OPEN = 24,
        ENTITY_COLLISION2 = 26,
        MOVABLE_DAMAGE = 48,
    };

    Q_DECLARE_METATYPE(S2Plugin::VirtualFunction)

    extern const MemoryFieldData gsMemoryFieldType; // TODO: make functions to get what's needed, not relay on global

    struct MemoryField
    {
        std::string name;
        MemoryFieldType type;
        uint64_t extraInfo = 0;
        // jsonName only if applicable: if a type is not a MemoryFieldType, but fully defined in the json file
        // then save its name so we can compare later
        std::string jsonName;
        std::string comment;
        std::string parentPointerJsonName;
        std::string parentStructJsonName;
        std::string virtualFunctionTableType;
        std::string firstParameterType;
        std::string secondParameterType;
        bool isPointer = false;
        bool isInlineStruct = false;
    };
    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType)
    Q_DECLARE_METATYPE(S2Plugin::MemoryField)

    struct VirtualFunction;
    struct Spelunky2;

    class Configuration
    {
      public:
        static Configuration* get();
        static bool reset();
        static bool is_loaded();

        const std::unordered_map<std::string, std::string>& entityClassHierarchy() const noexcept;
        const std::vector<std::pair<std::string, std::string>>& defaultEntityClassTypes() const noexcept;
        std::vector<std::string> classHierarchyOfEntity(const std::string& entityName) const;

        const std::vector<MemoryField>& typeFields(const MemoryFieldType& type) const;
        const std::vector<MemoryField>& typeFieldsOfEntitySubclass(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfPointer(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfInlineStruct(const std::string& type) const;
        std::vector<VirtualFunction> virtualFunctionsOfType(const std::string& field) const;

        bool isEntitySubclass(const std::string& type) const;
        bool isPointer(const std::string& type) const;
        bool isInlineStruct(const std::string& type) const;
        bool isBuiltInType(const std::string& type) const;
        int getAlingment(const std::string& type) const;

        std::string flagTitle(const std::string& fieldName, uint8_t flagNumber);
        std::string stateTitle(const std::string& fieldName, int64_t state);
        const std::unordered_map<int64_t, std::string>& stateTitlesOfField(const std::string& fieldName);

        size_t setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets, bool advanceOffset = true) const;
        size_t sizeOf(const std::string& typeName) const;

      private:
        static Configuration* ptr;
        bool mIsValid = false;

        std::unordered_map<std::string, std::string> mEntityClassHierarchy;
        std::vector<std::pair<std::string, std::string>> mDefaultEntityClassTypes;
        std::unordered_map<MemoryFieldType, std::vector<MemoryField>> mTypeFields;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsEntitySubclasses;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsPointers;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsInlineStructs;
        std::unordered_map<std::string, std::unordered_map<uint8_t, std::string>> mFlagTitles;  // fieldname => (flagnr 1-based => title)
        std::unordered_map<std::string, std::unordered_map<int64_t, std::string>> mStateTitles; // fieldname => (state => title)
        std::unordered_map<std::string, std::vector<VirtualFunction>> mVirtualFunctions;
        std::unordered_map<std::string, uint8_t> mAlignments;

        void processJSON(nlohmann::ordered_json& json);
        bool isKnownEntitySubclass(const std::string& typeName) const;

        Configuration();
        ~Configuration(){};
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;
    };
} // namespace S2Plugin
