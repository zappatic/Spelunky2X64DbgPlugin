#include "Data/ParticleEmittersList.h"
#include "Spelunky2.h"
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): PARTICLEEMITTER_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::ParticleEmittersList::ParticleEmittersList() : IDNameList("plugins/Spelunky2ParticleEmitters.txt", regexEntityLine) {}
