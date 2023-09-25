#include "Data/ParticleEmittersList.h"
#include "Spelunky2.h"
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): PARTICLEEMITTER_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::ParticleEmittersList::ParticleEmittersList(Spelunky2* spel2) : IDNameList(spel2, "plugins/Spelunky2ParticleEmitters.txt", regexEntityLine) {}
