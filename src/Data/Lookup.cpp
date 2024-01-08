#pragma once

#include "Data/CharacterDB.h"
#include "Spelunky2.h"
#include "pluginmain.h"

uintptr_t S2Plugin::Spelunky2::get_GameManager()
{
    if (gameManager != 0)
        return gameManager;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "C6 80 39 01 00 00 00 48");
    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 10);
    auto offsetPtr = instructionOffset + pcOffset + 14;
    gameManager = Script::Memory::ReadQword(offsetPtr);
    return gameManager;
}
