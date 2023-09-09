#pragma once

#include "Data/IDNameList.h"

namespace S2Plugin
{
    struct Spelunky2;

    class ParticleEmittersList : public IDNameList
    {
      public:
        explicit ParticleEmittersList(Spelunky2* spel2);
    };
} // namespace S2Plugin
