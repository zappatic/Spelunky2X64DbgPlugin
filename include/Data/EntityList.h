#pragma once

#include "Data/IDNameList.h"

namespace S2Plugin
{
    struct Spelunky2;

    class EntityList : public IDNameList
    {
      public:
        explicit EntityList(Spelunky2* spel2);
    };
} // namespace S2Plugin
