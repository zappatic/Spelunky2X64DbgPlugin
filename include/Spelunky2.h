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

        // TODO: those should be private
        size_t codeSectionStart{0};
        size_t codeSectionSize{0};
        size_t afterBundle{0};
        size_t afterBundleSize{0};

        static Spelunky2* ptr;

      private:
        Spelunky2() = default;
        ~Spelunky2(){};
        Spelunky2(const Spelunky2&) = delete;
        Spelunky2& operator=(const Spelunky2&) = delete;
    };
} // namespace S2Plugin
