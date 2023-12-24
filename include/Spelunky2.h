#pragma once

#include "pluginmain.h"
#include <QMetaEnum>
#include <cstdint>
#include <map>
#include <qnamespace.h>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    constexpr uint32_t TEB_offset = 0x120;

    enum class GAME_OFFSET : size_t
    {
        UNKNOWN1 = 0x8,            // - ?
        MALLOC = 0x20,             // - malloc base
        UNKNOWN2 = 0x3B4,          // - ?
        ILLUMINATION_SYNC = 0x3D0, // - illumination sync timer
        PRNG = 0x3F0,              // - PRNG
        STATE = 0x4A0,             // - State Memory
        LEVEL_GEN = 0xD7B30,       // - level gen
        LIQUID_ENGINE = 0xD8650,   // - liquid physics
        UNKNOWN3 = 0x108420,       // - some vector?
    };

    class EntityDB;

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

    struct Spelunky2
    {
        static Spelunky2* get();
        static void reset();
        static bool is_loaded()
        {
            return get() != nullptr;
        };

        std::string getEntityName(size_t offset, EntityDB* entityDB) const;
        uint32_t getEntityTypeID(size_t offset) const;

        size_t find(const char* pattern, size_t start = 0, size_t size = 0) const;
        size_t find_between(const char* pattern, size_t start = 0, size_t end = 0) const;

        // TODO: those should be private
        size_t codeSectionStart{0};
        size_t codeSectionSize{0};
        size_t afterBundle{0};
        size_t afterBundleSize{0};

        static Spelunky2* ptr;

      private:
        size_t heapBaseAddr{0};

        Spelunky2() = default;
        ~Spelunky2(){};
        Spelunky2(const Spelunky2&) = delete;
        Spelunky2& operator=(const Spelunky2&) = delete;
    };
} // namespace S2Plugin
