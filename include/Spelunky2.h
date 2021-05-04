#pragma once

#include <QMetaEnum>
#include <cstdint>
#include <qnamespace.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace S2Plugin
{
    class EntityDB;

    static constexpr uint8_t gsColField = 0;
    static constexpr uint8_t gsColValue = 1;
    static constexpr uint8_t gsColValueHex = 2;
    static constexpr uint8_t gsColMemoryOffset = 3;
    static constexpr uint8_t gsColType = 4;
    static constexpr uint8_t gsColComment = 5;

    static const uint16_t gsRoleField = Qt::UserRole + gsColField;
    static const uint16_t gsRoleValue = Qt::UserRole + gsColValue;
    static const uint16_t gsRoleValueHex = Qt::UserRole + gsColValueHex;
    static const uint16_t gsRoleType = Qt::UserRole + gsColMemoryOffset;
    static const uint16_t gsRoleMemoryOffset = Qt::UserRole + gsColType;
    static const uint16_t gsRoleRawValue = Qt::UserRole + 10;
    static const uint16_t gsRoleUID = Qt::UserRole + 11;
    static const uint16_t gsRoleFlagIndex = Qt::UserRole + 12;
    static const uint16_t gsRoleFlagFieldName = Qt::UserRole + 13;
    static const uint16_t gsRoleFlagsSize = Qt::UserRole + 14;
    static const uint16_t gsRoleFieldName = Qt::UserRole + 15;

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
        Skip,
        Rect,
        State,
        EntityDB,
        EntityPointer,
        EntityDBPointer,
        EntityDBID,
        EntityUID,
        SaturationAperture,
        Vector,
        Color,
        ConstCharPointerPointer,
        Map,
        EntitySubclass, // a subclass of an entity defined in json
        PointerType,    // a pointer defined in json
    };

    // clang-format off

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
        {MemoryFieldType::Rect, "Rectangle"},
        {MemoryFieldType::State, "State"},
        {MemoryFieldType::EntityDB, "EntityDB"},
        {MemoryFieldType::SaturationAperture, "Saturation/Aperture"},
        {MemoryFieldType::EntityPointer, "Entity pointer"},
        {MemoryFieldType::EntityDBPointer, "EntityDB pointer"},
        {MemoryFieldType::EntityDBID, "EntityDB ID"},
        {MemoryFieldType::EntityUID, "Entity UID"},
        {MemoryFieldType::Vector, "Vector"},
        {MemoryFieldType::Color, "Color"},
        {MemoryFieldType::ConstCharPointerPointer, "Const char**"},
        {MemoryFieldType::Map, "std::map<>"},
        {MemoryFieldType::PointerType, "Pointer"},
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
        {"Rect", MemoryFieldType::Rect},
        {"State", MemoryFieldType::State},
        {"EntityDB", MemoryFieldType::EntityDB},
        {"SaturationAperture", MemoryFieldType::SaturationAperture},
        {"EntityPointer", MemoryFieldType::EntityPointer},
        {"EntityDBPointer", MemoryFieldType::EntityDBPointer},
        {"EntityDBID", MemoryFieldType::EntityDBID},
        {"EntityUID", MemoryFieldType::EntityUID},
        {"Vector", MemoryFieldType::Vector},
        {"Color", MemoryFieldType::Color},
        {"ConstCharPointerPointer", MemoryFieldType::ConstCharPointerPointer},
        {"Map", MemoryFieldType::Map},
    };
    // clang-format on

    struct MemoryField
    {
        std::string name;
        MemoryFieldType type;
        uint64_t extraInfo = 0;
        // jsonName only if applicable: if a type is not a MemoryFieldType, but fully defined in the json file
        // then save its name so we can compare later
        std::string jsonName;
        std::string comment;
        bool isPointer = false;
    };
    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType)

    class Spelunky2
    {
      public:
        size_t spelunky2AfterBundle();
        size_t spelunky2AfterBundleSize();
        std::string getEntityName(size_t offset, EntityDB* entityDB);
        uint32_t getEntityTypeID(size_t offset);

        void displayError(const char* fmt, ...);
        void findSpelunky2InMemory();
        // size_t findEntityListMapOffset();

      private:
        bool mInitErrorShown = false;
    };

} // namespace S2Plugin