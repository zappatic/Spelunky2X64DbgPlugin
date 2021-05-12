#include "Data/EntityList.h"
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList(Spelunky2* spel2) : IDNameList(spel2, "plugins/Spelunky2Entities.txt", regexEntityLine) {}
