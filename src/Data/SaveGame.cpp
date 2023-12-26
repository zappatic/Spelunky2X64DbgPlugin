#include "Data/SaveGame.h"
#include "Configuration.h"
#include "Data/GameManager.h"
#include "Spelunky2.h"
#include "pluginmain.h"

S2Plugin::SaveGame::SaveGame(GameManager* gm) : mGameManager(gm) {}

bool S2Plugin::SaveGame::loadSaveGame()
{
    refreshOffsets();
    return true;
}

std::unordered_map<std::string, size_t>& S2Plugin::SaveGame::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::SaveGame::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mGameManager->saveGameOffset();
    auto config = Configuration::get();
    for (const auto& field : config->typeFields(MemoryFieldType::SaveGame))
    {
        offset = config->setOffsetForField(field, "SaveGame." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::SaveGame::offsetForField(const std::string& fieldName) const
{
    auto full = "SaveGame." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::SaveGame::reset()
{
    mMemoryOffsets.clear();
}
