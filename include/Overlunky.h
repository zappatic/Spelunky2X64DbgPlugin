#pragma once
#include <cstdint>
#include <map>
#include <unordered_map>

#define MAX_PLAYERS 4
struct Illumination;
struct Layer;
struct Items;
struct Color;
struct Rect;
struct Animation;
struct EntityDB;
struct Vector;
class Player;
class Entity;

using EntityCreate = Entity* (*)();
using EntityDestroy = void (*)(Entity*);
using AnimationMap = std::unordered_map<uint8_t, Animation>;

enum RepeatType : uint8_t
{
    NoRepeat,
    Linear,
    BackAndForth,
};

struct Animation
{
    int32_t texture;
    int32_t count;
    int32_t interval;
    uint8_t key;
    RepeatType repeat;
};

struct Rect
{
    int masks;
    float up_minus_down, side, up_plus_down;
    uint8_t field_10;
    uint8_t field_11;
    uint16_t field_12;
};

struct Color
{
    float r;
    float g;
    float b;
    float a;
};

struct Vector
{
  public:
    uint32_t* heap;
    uint32_t* begin;
    uint32_t size, count;
};

struct StateMemory
{
    size_t p00;
    uint32_t screen_last;
    uint32_t screen;
    uint32_t screen_next;
    uint32_t loading;
    Illumination* illumination;
    int32_t i20;
    uint32_t fadeout;
    uint32_t fadein;
    int32_t i2c;
    uint8_t ingame;
    uint8_t playing;
    uint8_t pause;
    uint8_t b33;
    int32_t i34;
    uint32_t quest_flags;
    int32_t i3c;
    int32_t i40;
    int32_t i44;
    uint32_t w;
    uint32_t h;
    int8_t kali_favor;
    int8_t kali_status;
    int8_t kali_altars_destroyed;
    uint8_t b4f;
    int32_t i50;
    int32_t i54;
    uint8_t world_start;
    uint8_t level_start;
    uint8_t theme_start;
    uint8_t b5f;
    uint32_t seed;
    uint32_t time_total;
    uint8_t world;
    uint8_t world_next;
    uint8_t level;
    uint8_t level_next;
    int32_t i6c;
    int32_t i70;
    uint8_t theme;
    uint8_t theme_next;
    uint8_t win_state; // 0 = no win 1 = tiamat win 2 = hundun win 3 = CO win; set this and next doorway leads to victory scene
    uint8_t b73;
    int32_t i74;
    uint8_t shoppie_aggro;
    uint8_t shoppie_aggro_levels;
    uint8_t merchant_aggro;
    uint8_t merchant_pad;
    uint8_t b7c;
    uint8_t b7d;
    uint8_t kills_npc;
    uint8_t level_count;
    uint8_t pad84[0x970];
    uint32_t journal_flags;
    int32_t i9f0;
    int32_t i9f4;
    uint32_t time_last_level;
    uint32_t time_level;
    int32_t ia00;
    int32_t ia04;
    int32_t hud_flags;

    char pada14[0x12b0 - 0xa14];
    Items* items;
    void* pad12b8;
    Layer* layers[2];
    char pad12d0[0x1308 - 0x12d0];
    std::unordered_map<uint32_t, Entity*> instance_id_to_pointer;
};

struct EntityDB
{
    EntityCreate create_func;
    EntityDestroy destroy_func;
    int32_t field_10;
    /* Entity id (ENT_...) */
    int32_t id;
    uint32_t search_flags;
    float width;
    float height;
    uint8_t draw_depth;
    uint8_t default_b3f;
    int16_t field_26;
    Rect rect_collision;
    int32_t field_3C;
    int32_t field_40;
    int32_t field_44;
    uint32_t default_flags;
    uint32_t default_more_flags;
    int32_t properties_flags;
    float friction;
    float elasticity;
    float weight;
    uint8_t field_60;
    float acceleration;
    float max_speed;
    float sprint_factor;
    float jump;

    /* ??? */
    float _a;
    float _b;
    float _c;
    float _d;

    int32_t texture;
    int32_t technique;
    int32_t tile_x;
    int32_t tile_y;
    uint8_t damage;
    uint8_t life;
    uint8_t field_96;
    uint8_t field_97;
    uint8_t field_98;
    int32_t description;
    int32_t field_a0;
    int32_t field_a4;
    float field_a8;
    int32_t field_AC;
    AnimationMap animations;
    float attachOffsetX;
    float attachOffsetY;
    uint8_t init;
};

struct Items
{
    void* __vftable;
    Player* players[MAX_PLAYERS];
    Player* player(size_t index);
};

struct SaturationVignette
{
    float red;
    float green;
    float blue;
    float vignette_aperture;
};

struct Illumination
{
    SaturationVignette saturation_vignette;
    SaturationVignette saturation_vignette_other[3]; // there's three more, no idea why (multiplayer doesn't change these)
    float brightness1;
    float brightness2;
    float something_min;
    float something_max;
    size_t unknown_empty;
    float unknown_float;
    float unknown_nan;
    uint32_t unknown_timer;
    uint8_t frontlayer_global_illumination; // 0 = on; 1 = off; 4 = white; ... higher starts to flicker
    uint8_t unknown_illumination1;
    uint8_t backlayer_global_illumination; // 0 = off ; 1 = on but turns front layer off
    uint8_t unknown_illumination2;         // influences backlayer_global_illumination
    uint32_t unknown_int1;                 // crash when changed
    uint32_t unknown_int2;
};

class Entity
{
  public:
    EntityDB* type;
    Entity* overlay;
    Vector items;
    uint32_t flags;
    uint32_t more_flags;
    int32_t uid;
    uint8_t animation_frame;
    uint8_t b3d;
    uint8_t draw_depth;
    uint8_t b3f;
    float x;
    float y;
    float w;
    float h;
    float f50;
    float f54;
    Color color;
    float offsetx;
    float offsety;
    float hitboxx;
    float hitboxy;
    uint32_t duckmask;
    float angle;
    size_t p80;
    size_t texture;
    float tilew;
    float tileh;
    uint8_t camera_layer;
    uint8_t b99;
    uint8_t b9a;
    uint8_t b9b;
    uint32_t i9c;
};

class Movable : public Entity
{
  public:
    std::map<int64_t, int64_t> pa0;
    std::map<int, int> pb0;
    size_t anim_func;
    int32_t ic8;
    int32_t icc;
    float movex;
    float movey;
    uint32_t buttons;
    uint32_t stand_counter;
    float fe0;
    int32_t price;
    int32_t owner_uid;
    int32_t last_owner_uid;
    size_t animation_func;
    uint32_t idle_counter;
    int32_t standing_on_uid;
    float velocityx;
    float velocityy;
    int32_t holding_uid;
    uint8_t state;
    uint8_t last_state;
    uint8_t move_state;
    uint8_t health;
    uint16_t stun_timer;
    uint16_t stun_state;
    uint32_t some_state;
    int16_t poison_tick_timer;
    int16_t unknown_timer;
    int32_t i11c;
    int32_t i120;
    uint8_t b124;
    uint8_t airtime;
    uint8_t b126;
    uint8_t b127;
};

class Monster : public Movable
{
  public:
    std::map<int64_t, int64_t> inside;
};

struct Inventory
{
    uint32_t money;
    uint8_t bombs;
    uint8_t ropes;
    uint8_t b06;
    uint8_t b07;
    uint8_t pad08[0x141c]; // specific treasure and killed monsters here, boring
    uint32_t kills_level;
    uint32_t kills_total;
};

class Player : public Monster
{
  public:
    Inventory* inventory_ptr;
    size_t p140;
    int32_t i148;
    int32_t i14c;
    size_t ai_func;
    size_t input_ptr;
    size_t p160;
    int32_t i168;
    int32_t i16c;
    uint8_t jump_timer;
    uint8_t unknown1;
    uint8_t unknown2;
    uint8_t unknown3;
    uint8_t some_timer;
    uint8_t can_use;
    uint8_t b176;
    uint8_t b177;
};
