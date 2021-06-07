#pragma once

#include "Data/GameManager.h"
#include "Data/MemoryMappedData.h"
#include "Spelunky2.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class SaveGame : public MemoryMappedData
    {
      public:
        SaveGame(Configuration* config, GameManager* gm);
        bool loadSaveGame();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;

        void reset();

      private:
        GameManager* mGameManager;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
    };
} // namespace S2Plugin
