#pragma once

#include "data/IDNameList.h"
#include <QColor>
#include <QMetaEnum>
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
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

    /*
     * [[ Roles explanation: ]]
     * The first 5 roles are all saved to the name field
     * those are used as information about the row
     * memory offsets in the name field are used just for row update and should not be used for anything else
     *
     * value, comparison value, memoryoffset and delta fields all should contain the `gsRoleRawValue` data
     * (may differ with some special types)
     *
     * valueHex and comparison valuehex contain `gsRoleRawValue` only when it's a pointer (only used for update check)
     * value and comparison value also contain `gsRoleMemoryOffset` for field editing purposes
     * for pointers, that will be the pointer itself, not memory offset of the pointer
     *
     * The rest of the roles are type specific
     */

    constexpr uint16_t gsRoleType = Qt::UserRole + 0;
    constexpr uint16_t gsRoleMemoryOffset = Qt::UserRole + 1;
    constexpr uint16_t gsRoleComparisonMemoryOffset = Qt::UserRole + 2;
    constexpr uint16_t gsRoleIsPointer = Qt::UserRole + 3;
    constexpr uint16_t gsRoleUID = Qt::UserRole + 4;
    constexpr uint16_t gsRoleRawValue = Qt::UserRole + 5;

    constexpr uint16_t gsRoleFlagIndex = Qt::UserRole + 6;
    constexpr uint16_t gsRoleRefName = Qt::UserRole + 7; // ref name for flags and states and vtable
    constexpr uint16_t gsRoleStdContainerFirstParameterType = Qt::UserRole + 8;
    constexpr uint16_t gsRoleStdContainerSecondParameterType = Qt::UserRole + 9;
    constexpr uint16_t gsRoleSize = Qt::UserRole + 10;
    constexpr uint16_t gsRoleEntityOffset = Qt::UserRole + 11; // for entity uid to not look for the uid twice

    constexpr char* gsJSONDragDropMemoryField_UID = "uid";
    constexpr char* gsJSONDragDropMemoryField_Offset = "offset";
    constexpr char* gsJSONDragDropMemoryField_Type = "type";

    // new types need to be added to
    // - the MemoryFieldType enum
    // - gsMemoryFieldType in Configuration.cpp
    // - optionally in Spelunky2.json if they have static structure
    // - handling of the json is done in populateMemoryField in Configuration.cpp
    // - displaying the data and handling the click event is done in TreeViewMemoryFields.cpp
    // - if it's common use/basic type, you may also want to add it in getAlingment function
    // - there are some specific conditions for comparison in database handled in DatabaseHelper.cpp
    // new subclasses of Entity can just be added to the class hierarchy in Spelunky2Entities.json
    // and have its fields defined there

    enum class MemoryFieldType
    {
        None = 0, // special type just for error handling
        Dummy,    // dummy type for uses like fake parent type in StdMap
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
        ConstCharPointer,
        ConstCharPointerPointer,
        StdString,
        StdWstring,
        EntitySubclass,               // a subclass of an entity defined in json
        DefaultStructType,            // a struct defined in json
        UndeterminedThemeInfoPointer, // used to look up the theme pointer in the levelgen and show the correct theme name
        ThemeInfoName,                // same as above, but does not add struct tree
        LevelGenRoomsPointer,         // used to make the level gen rooms title clickable
        LevelGenRoomsMetaPointer,     // used to make the level gen rooms title clickable
        JournalPagePointer,           // used to make journal page in vector clickable
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

    struct VirtualFunction
    {
        size_t index;
        std::string name;
        std::string params;
        std::string returnValue;
        std::string type;

        VirtualFunction(size_t i, std::string n, std::string p, std::string r, std::string t) : index(i), name(n), params(p), returnValue(r), type(t){};
    };

    struct MemoryField
    {
        std::string name;
        size_t size{0};

        MemoryFieldType type{MemoryFieldType::None};
        bool isPointer{false};

        // jsonName only if applicable: if a type is not a MemoryFieldType, but fully defined in the json file
        // then save its name so we can compare later
        std::string jsonName;
        std::string firstParameterType;
        std::string secondParameterType;
        std::string comment;
        size_t get_size() const;

        // For checking duplicate names
        bool operator==(const MemoryField& other) const
        {
            return name == other.name;
        }
    };

    struct RoomCode
    {
        uint16_t id;
        std::string name;
        QColor color;
        RoomCode(uint16_t _id, std::string _name, QColor _color) : id(_id), name(_name), color(_color){};
    };

    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType)
    Q_DECLARE_METATYPE(std::string)

    struct VirtualFunction;
    struct Spelunky2;

    class Configuration
    {
      public:
        static Configuration* get();
        static bool reload();
        // tries to load Configuration if not loaded already
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
        static bool isPointerType(MemoryFieldType type);

        uintptr_t offsetForField(const std::vector<MemoryField>& fields, std::string_view fieldUID, uintptr_t addr = 0) const;
        uintptr_t offsetForField(MemoryFieldType type, std::string_view fieldUID, uintptr_t addr = 0) const;

        // equivalent to alignof operator
        int getAlingment(const std::string& type) const;
        bool isPermanentPointer(const std::string& type) const;
        bool isJsonStruct(const std::string type) const;

        std::string flagTitle(const std::string& fieldName, uint8_t flagNumber) const;
        std::string stateTitle(const std::string& fieldName, int64_t state) const;
        const std::vector<std::pair<int64_t, std::string>>& refTitlesOfField(const std::string& fieldName) const;

        size_t getTypeSize(const std::string& typeName, bool entitySubclass = false);
        const EntityList& entityList() const
        {
            return entityNames;
        };

        const ParticleEmittersList& particleEmittersList() const
        {
            return particleEmitters;
        }

        RoomCode roomCodeForID(uint16_t code) const;
        std::string getEntityName(uint32_t type) const;

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

        std::unordered_map<uint16_t, RoomCode> mRoomCodes;

        void processEntitiesJSON(nlohmann::ordered_json& json);
        void processJSON(nlohmann::ordered_json& json);
        void processRoomCodesJSON(nlohmann::ordered_json& json);
        MemoryField populateMemoryField(const nlohmann::ordered_json& field, const std::string& struct_name);

        EntityList entityNames;
        ParticleEmittersList particleEmitters;

        Configuration();
        ~Configuration(){};
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;
    };
} // namespace S2Plugin
