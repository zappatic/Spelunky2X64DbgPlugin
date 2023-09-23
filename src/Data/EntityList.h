#pragma once

#include "Data/IDNameList.h"
#include "Spelunky2.h"
#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class EntityList : public IDNameList
    {
      public:
        explicit EntityList(Spelunky2* spel2);
    };
} // namespace S2Plugin