#include "Data/EntityList.h"
#include <regex>

static const std::regex regexEntityLine("^([0-9]+): ENT_TYPE_(.*?)$", std::regex_constants::ECMAScript);

S2Plugin::EntityList::EntityList() : IDNameList("plugins/Spelunky2Entities.txt", regexEntityLine) {}
