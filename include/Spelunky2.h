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

// new types need to be added to
// - the MemoryFieldType enum below
// - its fields in a vector<MemoryField> below
// - MemoryMappedData.h/cpp (setOffsetForField)
// - TreeViewMemoryFields.cpp (updateValueForField and addMemoryField)

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
    Flags32,
    Skip,
    Rect,
    StateIllumination,
    StateSaturationVignette,
    StateItems,
    Layer,
};

struct MemoryField
{
    std::string name;
    MemoryFieldType type;
    uint64_t extraInfo = 0;
};

// clang-format off
const std::vector<MemoryField> gsEntityDBFields = {
    {"create_func", MemoryFieldType::CodePointer}, 
    {"destroy_func", MemoryFieldType::CodePointer},
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

static const std::vector<MemoryField> gsStateFields = {
    {"p00", MemoryFieldType::DataPointer}, 
    {"screen_last", MemoryFieldType::UnsignedDword}, 
    {"screen", MemoryFieldType::UnsignedDword}, 
    {"screen_next", MemoryFieldType::UnsignedDword}, 
    {"loading", MemoryFieldType::UnsignedDword},
    {"illumination", MemoryFieldType::StateIllumination},
    {"i20", MemoryFieldType::Dword},
    {"fadeout", MemoryFieldType::UnsignedDword},
    {"fadein", MemoryFieldType::UnsignedDword},
    {"i2c", MemoryFieldType::Dword},
    {"ingame", MemoryFieldType::Bool},
    {"playing", MemoryFieldType::Bool},
    {"pause", MemoryFieldType::Bool},
    {"b33", MemoryFieldType::UnsignedByte},
    {"i34", MemoryFieldType::Dword},
    {"quest_flags", MemoryFieldType::Flags32},
    {"i3c", MemoryFieldType::Dword},
    {"i40", MemoryFieldType::Dword},
    {"i44", MemoryFieldType::Dword},
    {"w", MemoryFieldType::UnsignedDword},
    {"h", MemoryFieldType::UnsignedDword},
    {"kali_favor", MemoryFieldType::Byte},
    {"kali_status", MemoryFieldType::Byte},
    {"kali_altars_destroyed", MemoryFieldType::Byte},
    {"b4f", MemoryFieldType::UnsignedByte},
    {"i50", MemoryFieldType::Dword},
    {"i54", MemoryFieldType::Dword},
    {"world_start", MemoryFieldType::UnsignedByte},
    {"level_start", MemoryFieldType::UnsignedByte},
    {"theme_start", MemoryFieldType::UnsignedByte},
    {"b5f", MemoryFieldType::UnsignedByte},
    {"seed", MemoryFieldType::UnsignedDword},
    {"time_total", MemoryFieldType::UnsignedDword},
    {"world", MemoryFieldType::UnsignedByte},
    {"world_next", MemoryFieldType::UnsignedByte},
    {"level", MemoryFieldType::UnsignedByte},
    {"level_next", MemoryFieldType::UnsignedByte},
    {"i6c", MemoryFieldType::Dword},
    {"i70", MemoryFieldType::Dword},
    {"theme", MemoryFieldType::UnsignedByte},
    {"theme_next", MemoryFieldType::UnsignedByte},
    {"win_state", MemoryFieldType::UnsignedByte},
    {"b73", MemoryFieldType::UnsignedByte},
    {"i74", MemoryFieldType::Dword},
    {"shoppie_aggro", MemoryFieldType::UnsignedByte},
    {"shoppie_aggro_levels", MemoryFieldType::UnsignedByte},
    {"merchant_aggro", MemoryFieldType::UnsignedByte},
    {"merchant_pad", MemoryFieldType::UnsignedByte},
    {"b7c", MemoryFieldType::UnsignedByte},
    {"b7d", MemoryFieldType::UnsignedByte},
    {"kills_npc", MemoryFieldType::UnsignedByte},
    {"level_count", MemoryFieldType::UnsignedByte},
    {"-", MemoryFieldType::Skip, 0x970},
    {"journal_flags", MemoryFieldType::Flags32},
    {"i9f0", MemoryFieldType::Dword},
    {"i9f4", MemoryFieldType::Dword},
    {"time_last_level", MemoryFieldType::UnsignedDword},
    {"time_level", MemoryFieldType::UnsignedDword},
    {"ia00", MemoryFieldType::Dword},
    {"money_total", MemoryFieldType::UnsignedDword},
    {"hud_flags", MemoryFieldType::Flags32},
    {"-", MemoryFieldType::Skip, 0x12b0 - 0xa14},
    {"items", MemoryFieldType::StateItems},
    {"-", MemoryFieldType::Skip, 8},
    {"layer0", MemoryFieldType::Layer}, 
    {"layer1", MemoryFieldType::Layer}, 
};

static const std::vector<MemoryField> gsStateIlluminationFields = {
    {"saturation_vignette_0", MemoryFieldType::StateSaturationVignette}, 
    {"saturation_vignette_1", MemoryFieldType::StateSaturationVignette}, 
    {"saturation_vignette_2", MemoryFieldType::StateSaturationVignette}, 
    {"saturation_vignette_3", MemoryFieldType::StateSaturationVignette}, 
    {"brightness1", MemoryFieldType::Float}, 
    {"brightness2", MemoryFieldType::Float}, 
    {"something_min", MemoryFieldType::Float}, 
    {"something_max", MemoryFieldType::Float}, 
    {"unknown_empty", MemoryFieldType::UnsignedQword}, 
    {"unknown_float", MemoryFieldType::Float}, 
    {"unknown_nan", MemoryFieldType::Float}, 
    {"unknown_timer", MemoryFieldType::UnsignedDword}, 
    {"frontlayer_global_illumination", MemoryFieldType::UnsignedByte},
    {"unknown_illumination1", MemoryFieldType::UnsignedByte},
    {"backlayer_global_illumination", MemoryFieldType::UnsignedByte},
    {"unknown_illumination2", MemoryFieldType::UnsignedByte},
    {"unknown_int1", MemoryFieldType::UnsignedDword},
    {"unknown_int2", MemoryFieldType::UnsignedDword}
};

static const std::vector<MemoryField> gsStateSaturationVignetteFields = {
    {"red", MemoryFieldType::Float}, 
    {"green", MemoryFieldType::Float}, 
    {"blue", MemoryFieldType::Float}, 
    {"vignette_aperture", MemoryFieldType::Float}
};

static const std::vector<MemoryField> gsStateItemsFields = {
    {"__vftable", MemoryFieldType::DataPointer}, 
    {"player1", MemoryFieldType::DataPointer}, //TODO: player instead of datapointer
    {"player2", MemoryFieldType::DataPointer}, 
    {"player3", MemoryFieldType::DataPointer}, 
    {"player4", MemoryFieldType::DataPointer}, 
};


static const std::vector<MemoryField> gsLayerFields = {
    {"__vftable", MemoryFieldType::DataPointer}, 
    {"first_entity*", MemoryFieldType::DataPointer}, 
    {"b", MemoryFieldType::DataPointer}, 
    {"capacity", MemoryFieldType::Dword}, 
    {"size", MemoryFieldType::Dword}, 
};

// clang-format on

size_t spelunky2AfterBundle();
size_t spelunky2AfterBundleSize();