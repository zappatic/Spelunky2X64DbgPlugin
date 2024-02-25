#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    class Entity
    {
      public:
        Entity(uintptr_t offset) : mEntityPtr(offset){};

        std::string entityClassName() const;
        uint32_t entityTypeID() const;
        std::string entityTypeName() const;
        static std::vector<std::string> classHierarchy(std::string validClassName);
        std::vector<std::string> classHierarchy() const
        {
            return classHierarchy(entityClassName());
        }

        uint8_t cameraLayer() const;
        uint32_t uid() const;
        uintptr_t ptr() const
        {
            return mEntityPtr;
        }
        std::pair<float, float> position() const;
        std::pair<int, int> position_floor() const
        {
            return static_cast<std::pair<int, int>>(position());
        }

      private:
        uintptr_t mEntityPtr;

        enum ENTITY_OFFSETS
        {
            UID = 0x38,
            LAYER = 0xA0,
            TYPE_PTR = 0x8,
            DB_TYPE_ID = 0x14,
            POS = 0x40,
        };
    };
} // namespace S2Plugin
