#pragma once

#include <cstdint>
#include <qnamespace.h>
#include <string>
#include <vector>

static constexpr uint8_t gsColField = 0;
static constexpr uint8_t gsColValue = 1;
static constexpr uint8_t gsColValueHex = 2;
static constexpr uint8_t gsColMemoryOffset = 3;
static constexpr uint8_t gsColType = 4;

static const uint16_t gsRoleField = Qt::UserRole + gsColField;
static const uint16_t gsRoleValue = Qt::UserRole + gsColValue;
static const uint16_t gsRoleValueHex = Qt::UserRole + gsColValueHex;
static const uint16_t gsRoleType = Qt::UserRole + gsColMemoryOffset;
static const uint16_t gsRoleMemoryOffset = Qt::UserRole + gsColType;
static const uint16_t gsRoleRawValue = Qt::UserRole + 10;
static const uint16_t gsRoleUID = Qt::UserRole + 11;

enum class MemoryFieldType
{
    Pointer,
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
    Flags32,
    Rect,
    Skip
};

struct MemoryField
{
    std::string name;
    MemoryFieldType type;
    uint64_t extraInfo = 0;
};

// clang-format off
const std::vector<MemoryField> gsEntityDBFields = {
    {"create_func", MemoryFieldType::Pointer}, 
    {"destroy_func", MemoryFieldType::Pointer},
    {"field_10", MemoryFieldType::UnsignedDword},
    {"id", MemoryFieldType::UnsignedDword},
    {"search_flags", MemoryFieldType::Flags32},
    {"width", MemoryFieldType::Float},
    {"height", MemoryFieldType::Float},
    {"draw_depth", MemoryFieldType::UnsignedByte},
    {"default_b3f", MemoryFieldType::UnsignedByte},
    {"field_26", MemoryFieldType::Word},
    {"rect_collision", MemoryFieldType::Rect},
    {"field_3C", MemoryFieldType::Dword},
    {"field_40", MemoryFieldType::Dword},
    {"field_44", MemoryFieldType::Dword},
    {"default_flags", MemoryFieldType::Flags32},
    {"default_more_flags", MemoryFieldType::Flags32},
    {"properties_flags", MemoryFieldType::Flags32},
    {"friction", MemoryFieldType::Float},
    {"elasticity", MemoryFieldType::Float},
    {"weight", MemoryFieldType::Float},
    {"field_60", MemoryFieldType::UnsignedByte},
    {"field_61", MemoryFieldType::UnsignedByte},
    {"field_62", MemoryFieldType::UnsignedByte},
    {"field_63", MemoryFieldType::UnsignedByte},
    {"acceleration", MemoryFieldType::Float},
    {"max_speed", MemoryFieldType::Float},
    {"sprint_factor", MemoryFieldType::Float},
    {"jump", MemoryFieldType::Float},
    {"_a", MemoryFieldType::Float},
    {"_b", MemoryFieldType::Float},
    {"_c", MemoryFieldType::Float},
    {"_d", MemoryFieldType::Float},
    {"texture", MemoryFieldType::Dword},
    {"technique", MemoryFieldType::Dword},
    {"tile_x", MemoryFieldType::Dword},
    {"tile_y", MemoryFieldType::Dword},
    {"damage", MemoryFieldType::UnsignedByte},
    {"life", MemoryFieldType::UnsignedByte},
    {"field_96", MemoryFieldType::UnsignedByte},
    {"field_97", MemoryFieldType::UnsignedByte},
    {"field_98", MemoryFieldType::UnsignedByte},
    {"field_99", MemoryFieldType::UnsignedByte},
    {"field_9A", MemoryFieldType::UnsignedByte},
    {"field_9B", MemoryFieldType::UnsignedByte},
    {"description", MemoryFieldType::Dword},
    {"field_a0", MemoryFieldType::Dword},
    {"field_a4", MemoryFieldType::Dword},
    {"field_a8", MemoryFieldType::Dword},
    {"field_AC", MemoryFieldType::Dword},
    {"-", MemoryFieldType::Skip, 0x40},
    {"attachOffsetX", MemoryFieldType::Float},
    {"attachOffsetY", MemoryFieldType::Float},
    {"init", MemoryFieldType::UnsignedByte},
    {"field_19", MemoryFieldType::UnsignedByte},
    {"field_1a", MemoryFieldType::UnsignedByte},
    {"field_1b", MemoryFieldType::UnsignedByte},
    {"field_1c", MemoryFieldType::Dword},
};

static const std::vector<MemoryField> gsRectFields = {
    {"masks", MemoryFieldType::UnsignedDword}, 
    {"up_minus_down", MemoryFieldType::Float}, 
    {"side", MemoryFieldType::Float}, 
    {"up_plus_down", MemoryFieldType::Float}, 
    {"field_10", MemoryFieldType::UnsignedByte}, 
    {"field_11", MemoryFieldType::UnsignedByte}, 
    {"field_12", MemoryFieldType::UnsignedWord}
};
// clang-format on

size_t spelunky2AfterBundle();
size_t spelunky2AfterBundleSize();