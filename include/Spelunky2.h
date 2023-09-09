#pragma once

#include <QMetaEnum>
#include <cstdint>
#include <qnamespace.h>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class EntityDB;

    static constexpr uint8_t gsColField = 0;
    static constexpr uint8_t gsColValue = 1;
    static constexpr uint8_t gsColValueHex = 2;
    static constexpr uint8_t gsColComparisonValue = 3;
    static constexpr uint8_t gsColComparisonValueHex = 4;
    static constexpr uint8_t gsColMemoryOffset = 5;
    static constexpr uint8_t gsColMemoryOffsetDelta = 6;
    static constexpr uint8_t gsColType = 7;
    static constexpr uint8_t gsColComment = 8;

    static const uint16_t gsRoleField = Qt::UserRole + gsColField;
    static const uint16_t gsRoleValue = Qt::UserRole + gsColValue;
    static const uint16_t gsRoleValueHex = Qt::UserRole + gsColValueHex;
    static const uint16_t gsRoleComparisonValue = Qt::UserRole + gsColComparisonValue;
    static const uint16_t gsRoleComparisonValueHex = Qt::UserRole + gsColComparisonValueHex;
    static const uint16_t gsRoleType = Qt::UserRole + gsColMemoryOffset;
    static const uint16_t gsRoleMemoryOffset = Qt::UserRole + gsColType;
    static const uint16_t gsRoleRawValue = Qt::UserRole + 10;
    static const uint16_t gsRoleRawComparisonValue = Qt::UserRole + 11;
    static const uint16_t gsRoleUID = Qt::UserRole + 12;
    static const uint16_t gsRoleFlagIndex = Qt::UserRole + 13;
    static const uint16_t gsRoleFlagFieldName = Qt::UserRole + 14;
    static const uint16_t gsRoleFlagsSize = Qt::UserRole + 15;
    static const uint16_t gsRoleFieldName = Qt::UserRole + 16;
    static const uint16_t gsRoleStdContainerFirstParameterType = Qt::UserRole + 17;
    static const uint16_t gsRoleBaseFieldName = Qt::UserRole + 18;
    static const uint16_t gsRoleEntireMemoryField = Qt::UserRole + 19;
    static const uint16_t gsRoleStdContainerSecondParameterType = Qt::UserRole + 20;

    static const char* gsJSONDragDropMemoryField_UID = "uid";
    static const char* gsJSONDragDropMemoryField_Offset = "offset";
    static const char* gsJSONDragDropMemoryField_Type = "type";

    // new types need to be added to
    // - the MemoryFieldType enum
    // - the string representation of the type in gsMemoryFieldTypeToStringMapping
    // - the json name of the type in gsJSONStringToMemoryFieldTypeMapping
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

    // for display purposes
    const static std::unordered_map<MemoryFieldType, std::string> gsMemoryFieldTypeToStringMapping = {
        {MemoryFieldType::CodePointer, "Code pointer"},
        {MemoryFieldType::DataPointer, "Data pointer"},
        {MemoryFieldType::Byte, "8-bit"},
        {MemoryFieldType::UnsignedByte, "8-bit unsigned"},
        {MemoryFieldType::Word, "16-bit"},
        {MemoryFieldType::UnsignedWord, "16-bit unsigned"},
        {MemoryFieldType::Dword, "32-bit"},
        {MemoryFieldType::UnsignedDword, "32-bit unsigned"},
        {MemoryFieldType::Qword, "64-bit"},
        {MemoryFieldType::UnsignedQword, "64-bit unsigned"},
        {MemoryFieldType::Float, "Float"},
        {MemoryFieldType::Bool, "Bool"},
        {MemoryFieldType::Flag, "Flag"},
        {MemoryFieldType::Flags32, "32-bit flags"},
        {MemoryFieldType::Flags16, "16-bit flags"},
        {MemoryFieldType::Flags8, "8-bit flags"},
        {MemoryFieldType::State8, "8-bit state"},
        {MemoryFieldType::State16, "16-bit state"},
        {MemoryFieldType::State32, "32-bit state"},
        {MemoryFieldType::GameManager, "GameManager"},
        {MemoryFieldType::State, "State"},
        {MemoryFieldType::SaveGame, "SaveGame"},
        {MemoryFieldType::LevelGen, "LevelGen"},
        {MemoryFieldType::EntityDB, "EntityDB"},
        {MemoryFieldType::EntityPointer, "Entity pointer"},
        {MemoryFieldType::EntityUIDPointer, "Entity UID pointer"},
        {MemoryFieldType::EntityDBPointer, "EntityDB pointer"},
        {MemoryFieldType::EntityDBID, "EntityDB ID"},
        {MemoryFieldType::EntityUID, "Entity UID"},
        {MemoryFieldType::ParticleDBID, "ParticleDB ID"},
        {MemoryFieldType::ParticleDB, "ParticleDB"},
        {MemoryFieldType::ParticleDBPointer, "ParticleDB pointer"},
        {MemoryFieldType::TextureDB, "TextureDB"},
        {MemoryFieldType::TextureDBID, "TextureDB ID"},
        {MemoryFieldType::TextureDBPointer, "TextureDB pointer"},
        {MemoryFieldType::Vector, "Vector"},
        {MemoryFieldType::StdVector, "StdVector"},
        {MemoryFieldType::StdMap, "StdMap"},
        {MemoryFieldType::StdSet, "StdSet"},
        {MemoryFieldType::ConstCharPointerPointer, "Const char**"},
        {MemoryFieldType::ConstCharPointer, "Const char*"},
        {MemoryFieldType::StdString, "StdString"},
        {MemoryFieldType::StdWstring, "StdWstring"},
        {MemoryFieldType::PointerType, "Pointer"},
        {MemoryFieldType::InlineStructType, "Inline struct"},
        {MemoryFieldType::UndeterminedThemeInfoPointer, "UndeterminedThemeInfoPointer"},
        {MemoryFieldType::LevelGenRoomsPointer, "LevelGenRoomsPointer"},
        {MemoryFieldType::LevelGenRoomsMetaPointer, "LevelGenRoomsMetaPointer"},
        {MemoryFieldType::JournalPagePointer, "JournalPagePointer"},
        {MemoryFieldType::ThemeInfoName, "ThemeInfoName"},
        {MemoryFieldType::LevelGenPointer, "LevelGenPointer"},
        {MemoryFieldType::UTF16Char, "UTF16Char"},
        {MemoryFieldType::UTF16StringFixedSize, "UTF16StringFixedSize"},
        {MemoryFieldType::UTF8StringFixedSize, "UTF8StringFixedSize"},
        {MemoryFieldType::StringsTableID, "StringsTableID"},
        {MemoryFieldType::CharacterDB, "CharacterDB"},
        {MemoryFieldType::CharacterDBID, "CharacterDBID"},
        {MemoryFieldType::VirtualFunctionTable, "VirtualFunctionTable"},
        {MemoryFieldType::Online, "Online"},
        {MemoryFieldType::IPv4Address, "IPv4Address"},
    };

    // for C++ header generation
    const static std::unordered_map<MemoryFieldType, std::string> gsMemoryFieldTypeToCPPTypeMapping = {
        {MemoryFieldType::CodePointer, "size_t"},
        {MemoryFieldType::DataPointer, "size_t"},
        {MemoryFieldType::Byte, "int8_t"},
        {MemoryFieldType::UnsignedByte, "uint8_t"},
        {MemoryFieldType::Word, "int16_t"},
        {MemoryFieldType::UnsignedWord, "uint16_t"},
        {MemoryFieldType::Dword, "int32_t"},
        {MemoryFieldType::UnsignedDword, "uint32_t"},
        {MemoryFieldType::Qword, "int64_t"},
        {MemoryFieldType::UnsignedQword, "uint64_t"},
        {MemoryFieldType::Float, "float"},
        {MemoryFieldType::Bool, "bool"},
        {MemoryFieldType::Flags32, "uint32_t"},
        {MemoryFieldType::Flags16, "uint16_t"},
        {MemoryFieldType::Flags8, "uint8_t"},
        {MemoryFieldType::State8, "int8_t"},
        {MemoryFieldType::State16, "int16_t"},
        {MemoryFieldType::State32, "int32_t"},
        {MemoryFieldType::EntityPointer, "Entity*"},
        {MemoryFieldType::EntityUIDPointer, "uint32_t*"},
        {MemoryFieldType::EntityDBPointer, "EntityDB*"},
        {MemoryFieldType::EntityDBID, "uint32_t"},
        {MemoryFieldType::EntityUID, "int32_t"},
        {MemoryFieldType::ParticleDBID, "uint32_t"},
        {MemoryFieldType::ParticleDBPointer, "ParticleDB*"},
        {MemoryFieldType::TextureDBID, "uint32_t"},
        {MemoryFieldType::TextureDBPointer, "Texture*"},
        {MemoryFieldType::Vector, "Vector"},
        {MemoryFieldType::StdVector, "std::vector<T>"},
        {MemoryFieldType::StdMap, "std::map<K, V>"},
        {MemoryFieldType::StdSet, "std::set<T>"},
        {MemoryFieldType::ConstCharPointerPointer, "const char**"},
        {MemoryFieldType::ConstCharPointer, "const char*"},
        {MemoryFieldType::StdString, "std::string"},
        {MemoryFieldType::StdWstring, "std::wstring"},
        {MemoryFieldType::UndeterminedThemeInfoPointer, "ThemeInfo*"},
        {MemoryFieldType::LevelGenRoomsPointer, "LevelGenRooms*"},
        {MemoryFieldType::LevelGenRoomsMetaPointer, "LevelGenRoomsMeta*"},
        {MemoryFieldType::JournalPagePointer, "JournalPage*"},
        {MemoryFieldType::LevelGenPointer, "LevelGen*"},
        {MemoryFieldType::UTF16Char, "uint16_t"},
        {MemoryFieldType::UTF16StringFixedSize, "// "},
        {MemoryFieldType::UTF8StringFixedSize, "// "},
        {MemoryFieldType::StringsTableID, "uint32_t"},
        {MemoryFieldType::CharacterDB, "CharacterDB*"},
        {MemoryFieldType::CharacterDBID, "uint8_t"},
        {MemoryFieldType::VirtualFunctionTable, "size_t"},
    };

    // the type strings as they occur in Spelunky2.json
    const static std::unordered_map<std::string, MemoryFieldType> gsJSONStringToMemoryFieldTypeMapping = {
        {"Skip", MemoryFieldType::Skip},
        {"CodePointer", MemoryFieldType::CodePointer},
        {"DataPointer", MemoryFieldType::DataPointer},
        {"Byte", MemoryFieldType::Byte},
        {"UnsignedByte", MemoryFieldType::UnsignedByte},
        {"Word", MemoryFieldType::Word},
        {"UnsignedWord", MemoryFieldType::UnsignedWord},
        {"Dword", MemoryFieldType::Dword},
        {"UnsignedDword", MemoryFieldType::UnsignedDword},
        {"Qword", MemoryFieldType::Qword},
        {"UnsignedQword", MemoryFieldType::UnsignedQword},
        {"Float", MemoryFieldType::Float},
        {"Bool", MemoryFieldType::Bool},
        {"Flags32", MemoryFieldType::Flags32},
        {"Flags16", MemoryFieldType::Flags16},
        {"Flags8", MemoryFieldType::Flags8},
        {"State8", MemoryFieldType::State8},
        {"State16", MemoryFieldType::State16},
        {"State32", MemoryFieldType::State32},
        {"GameManager", MemoryFieldType::GameManager},
        {"State", MemoryFieldType::State},
        {"SaveGame", MemoryFieldType::SaveGame},
        {"LevelGen", MemoryFieldType::LevelGen},
        {"EntityDB", MemoryFieldType::EntityDB},
        {"EntityPointer", MemoryFieldType::EntityPointer},
        {"EntityUIDPointer", MemoryFieldType::EntityUIDPointer},
        {"EntityDBPointer", MemoryFieldType::EntityDBPointer},
        {"EntityDBID", MemoryFieldType::EntityDBID},
        {"EntityUID", MemoryFieldType::EntityUID},
        {"ParticleDB", MemoryFieldType::ParticleDB},
        {"ParticleDBID", MemoryFieldType::ParticleDBID},
        {"ParticleDBPointer", MemoryFieldType::ParticleDBPointer},
        {"TextureDB", MemoryFieldType::TextureDB},
        {"TextureDBID", MemoryFieldType::TextureDBID},
        {"TextureDBPointer", MemoryFieldType::TextureDBPointer},
        {"Vector", MemoryFieldType::Vector},
        {"StdVector", MemoryFieldType::StdVector},
        {"StdMap", MemoryFieldType::StdMap},
        {"StdSet", MemoryFieldType::StdSet},
        {"ConstCharPointerPointer", MemoryFieldType::ConstCharPointerPointer},
        {"ConstCharPointer", MemoryFieldType::ConstCharPointer},
        {"StdString", MemoryFieldType::StdString},
        {"StdWstring", MemoryFieldType::StdWstring},
        {"UndeterminedThemeInfoPointer", MemoryFieldType::UndeterminedThemeInfoPointer},
        {"LevelGenRoomsPointer", MemoryFieldType::LevelGenRoomsPointer},
        {"LevelGenRoomsMetaPointer", MemoryFieldType::LevelGenRoomsMetaPointer},
        {"JournalPagePointer", MemoryFieldType::JournalPagePointer},
        {"ThemeInfoName", MemoryFieldType::ThemeInfoName},
        {"LevelGenPointer", MemoryFieldType::LevelGenPointer},
        {"UTF16Char", MemoryFieldType::UTF16Char},
        {"UTF16StringFixedSize", MemoryFieldType::UTF16StringFixedSize},
        {"UTF8StringFixedSize", MemoryFieldType::UTF8StringFixedSize},
        {"StringsTableID", MemoryFieldType::StringsTableID},
        {"CharacterDB", MemoryFieldType::CharacterDB},
        {"CharacterDBID", MemoryFieldType::CharacterDBID},
        {"VirtualFunctionTable", MemoryFieldType::VirtualFunctionTable},
        {"Online", MemoryFieldType::Online},
        {"IPv4Address", MemoryFieldType::IPv4Address},
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
    Q_DECLARE_METATYPE(S2Plugin::VirtualFunction)
    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType)
    Q_DECLARE_METATYPE(S2Plugin::MemoryField)

    class Spelunky2
    {
      public:
        size_t spelunky2AfterBundle();
        size_t spelunky2AfterBundleSize();
        std::string getEntityName(size_t offset, EntityDB* entityDB) const;
        uint32_t getEntityTypeID(size_t offset) const;

        void displayError(const char* fmt, ...);
        void findSpelunky2InMemory();
        void reset();
        // size_t findEntityListMapOffset();

      private:
        bool mInitErrorShown = false; // TODO: change
    };

} // namespace S2Plugin
