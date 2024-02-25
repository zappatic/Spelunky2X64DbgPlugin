#include "Data/LevelGen.h"
#include "Configuration.h"
#include "Data/State.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
//
// std::string S2Plugin::LevelGen::themeNameOfOffset(size_t offset) const
//{
//    if (offset == offsetForField("theme_dwelling.__vftable"))
//    {
//        return "DWELLING";
//    }
//    else if (offset == offsetForField("theme_jungle.__vftable"))
//    {
//        return "JUNGLE";
//    }
//    else if (offset == offsetForField("theme_volcana.__vftable"))
//    {
//        return "VOLCANA";
//    }
//    else if (offset == offsetForField("theme_olmec.__vftable"))
//    {
//        return "OLMEC";
//    }
//    else if (offset == offsetForField("theme_tidepool.__vftable"))
//    {
//        return "TIDE POOL";
//    }
//    else if (offset == offsetForField("theme_temple.__vftable"))
//    {
//        return "TEMPLE";
//    }
//    else if (offset == offsetForField("theme_icecaves.__vftable"))
//    {
//        return "ICE CAVES";
//    }
//    else if (offset == offsetForField("theme_neobabylon.__vftable"))
//    {
//        return "NEO BABYLON";
//    }
//    else if (offset == offsetForField("theme_sunkencity.__vftable"))
//    {
//        return "SUNKEN CITY";
//    }
//    else if (offset == offsetForField("theme_cosmicocean.__vftable"))
//    {
//        return "COSMIC OCEAN";
//    }
//    else if (offset == offsetForField("theme_city_of_gold.__vftable"))
//    {
//        return "CITY OF GOLD";
//    }
//    else if (offset == offsetForField("theme_duat.__vftable"))
//    {
//        return "DUAT";
//    }
//    else if (offset == offsetForField("theme_abzu.__vftable"))
//    {
//        return "ABZU";
//    }
//    else if (offset == offsetForField("theme_tiamat.__vftable"))
//    {
//        return "TIAMAT";
//    }
//    else if (offset == offsetForField("theme_eggplantworld.__vftable"))
//    {
//        return "EGGPLANT WORLD";
//    }
//    else if (offset == offsetForField("theme_hundun.__vftable"))
//    {
//        return "HUNDUN";
//    }
//    else if (offset == offsetForField("theme_basecamp.__vftable"))
//    {
//        return "BASE CAMP";
//    }
//    else if (offset == offsetForField("theme_arena.__vftable"))
//    {
//        return "ARENA";
//    }
//    return "UNKNOWN THEME";
//}
