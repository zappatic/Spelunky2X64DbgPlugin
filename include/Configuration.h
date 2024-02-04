#pragma once

#include <QMetaEnum>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

    constexpr uint16_t gsRoleType = Qt::UserRole + 0;
    constexpr uint16_t gsRoleMemoryOffset = Qt::UserRole + 1;
    constexpr uint16_t gsRoleRawValue = Qt::UserRole + 2;
    constexpr uint16_t gsRoleIsPointer = Qt::UserRole + 3;
    constexpr uint16_t gsRoleUID = Qt::UserRole + 4;

    constexpr uint16_t gsRoleFlagIndex = Qt::UserRole + 5;
    constexpr uint16_t gsRoleFieldName = Qt::UserRole + 6; // vtable
    constexpr uint16_t gsRoleRefName = Qt::UserRole + 7;   // ref name for flags and states
    constexpr uint16_t gsRoleStdContainerFirstParameterType = Qt::UserRole + 8;
    constexpr uint16_t gsRoleStdContainerSecondParameterType = Qt::UserRole + 9;
    constexpr uint16_t gsRoleRawComparisonValue = Qt::UserRole + 10; // just for flag field

    constexpr char* gsJSONDragDropMemoryField_UID = "uid";
    constexpr char* gsJSONDragDropMemoryField_Offset = "offset";
    constexpr char* gsJSONDragDropMemoryField_Type = "type";

    // new types need to be added to
    // - the MemoryFieldType enum
    // - gsMemoryFieldType in Configuration.cpp
    // - optionally in Spelunky2.json
    // - handling of the json is done in populateMemoryField in Configuration.cpp
    // - displaying the data and handling the click event is done in TreeViewMemoryFields.cpp
    // - if it's common use/basic type, you may also want to add it in getAlingment function
    // new subclasses of Entity can just be added to the class hierarchy in Spelunky2Entities.json
    // and have its fields defined there

    enum class MemoryFieldType
    {
        None, // special type just for error handling
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
        EntityUIDPointer, // TODO: remove
        EntityDBPointer,
        EntityDBID,
        EntityUID,
        ParticleDB,
        ParticleDBID,
        ParticleDBPointer,
        TextureDB,
        TextureDBID,
        TextureDBPointer,
        StdVector,
        StdMap,
        StdSet,
        ConstCharPointer,
        ConstCharPointerPointer,
        StdString,
        StdWstring,
        EntitySubclass,               // a subclass of an entity defined in json
        DefaultStructType,            // a struct defined in json
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
        Double,
    };

    class MemoryFieldData
    {
      public:
        struct Data
        {
            std::string_view display_name;
            std::string_view cpp_type_name;
            uint32_t size;
        };

        using map_type = std::unordered_map<MemoryFieldType, Data>;

        MemoryFieldData(std::initializer_list<std::tuple<MemoryFieldType, const char*, const char*, const char*, uint32_t>> init)
        {
            // MemoryFieldType type, const char* d_name, const char* cpp_type, const char* j_name, uint32_t size
            for (auto& val : init)
            {
                auto it = fields.emplace(std::get<0>(val), Data{std::get<1>(val), std::get<2>(val), std::get<4>(val)});
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

    struct VirtualFunction // TODO
    {
        size_t index;
        std::string name;
        std::string params;
        std::string returnValue;
        std::string type;

        VirtualFunction(size_t i, std::string n, std::string p, std::string r, std::string t) : index(i), name(n), params(p), returnValue(r), type(t){};
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

    // Q_DECLARE_METATYPE(S2Plugin::VirtualFunction) // TODO hope this is not nedded

    struct MemoryField
    {
        std::string name;
        size_t size{0};

        MemoryFieldType type{MemoryFieldType::None};
        bool isPointer{false};
        uint8_t flag_parrent_size{0};
        uint8_t flag_index;

        // jsonName only if applicable: if a type is not a MemoryFieldType, but fully defined in the json file
        // then save its name so we can compare later
        std::string jsonName;
        std::string firstParameterType;
        std::string secondParameterType;
        std::string comment;
        size_t get_size();
    };
    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType)
    Q_DECLARE_METATYPE(S2Plugin::MemoryField)
    Q_DECLARE_METATYPE(std::string)

    struct VirtualFunction;
    struct Spelunky2;

    class Configuration
    {
      public:
        static Configuration* get();
        static bool reload();
        static bool is_loaded();

        const std::unordered_map<std::string, std::string>& entityClassHierarchy() const noexcept;
        const std::vector<std::pair<std::string, std::string>>& defaultEntityClassTypes() const noexcept;
        std::vector<std::string> classHierarchyOfEntity(const std::string& entityName) const;

        const std::vector<MemoryField>& typeFields(const MemoryFieldType& type) const;
        const std::vector<MemoryField>& typeFieldsOfEntitySubclass(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfDefaultStruct(const std::string& type) const;
        std::vector<VirtualFunction> virtualFunctionsOfType(const std::string& field) const;

        bool isEntitySubclass(const std::string& type) const;

        static MemoryFieldType getBuiltInType(const std::string& type);
        static std::string_view getCPPTypeName(MemoryFieldType type);
        static std::string_view getTypeDisplayName(MemoryFieldType type);

        // equivalent to alignof operator
        int getAlingment(const std::string& type) const;
        bool isPermanentPointer(const std::string& type) const;
        bool isJsonStruct(const std::string type) const;

        std::string flagTitle(const std::string& fieldName, uint8_t flagNumber) const;
        std::string stateTitle(const std::string& fieldName, int64_t state) const;
        const std::vector<std::pair<int64_t, std::string>>& refTitlesOfField(const std::string& fieldName) const;

        size_t setOffsetForField(const MemoryField& field, const std::string& fieldNameOverride, size_t offset, std::unordered_map<std::string, size_t>& offsets, bool advanceOffset = true) const;
        size_t getTypeSize(const std::string& typeName, bool entitySubclass = false);

      private:
        static Configuration* ptr;
        bool initialisedCorrectly = false;

        std::unordered_map<std::string, std::string> mEntityClassHierarchy;
        std::vector<std::pair<std::string, std::string>> mDefaultEntityClassTypes;

        // for build in types defined in json
        std::unordered_map<MemoryFieldType, std::vector<MemoryField>> mTypeFieldsMain;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsEntitySubclasses;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsStructs;
        std::unordered_set<std::string> mPointerTypes; // pointers defined in pointer_types in json

        std::unordered_map<std::string, size_t> mTypeFieldsStructsSizes;

        std::unordered_map<std::string, std::vector<VirtualFunction>> mVirtualFunctions;
        std::unordered_map<std::string, uint8_t> mAlignments;
        std::unordered_map<std::string, std::vector<std::pair<int64_t, std::string>>> mRefs; // for flags and states

        void processEntitiesJSON(nlohmann::ordered_json& json);
        void processJSON(nlohmann::ordered_json& json);
        MemoryField populateMemoryField(const nlohmann::ordered_json& field, const std::string& struct_name);
        bool isKnownEntitySubclass(const std::string& typeName) const;

        Configuration();
        ~Configuration(){};
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;
    };
} // namespace S2Plugin
