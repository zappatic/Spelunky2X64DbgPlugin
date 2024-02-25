#include "Spelunky2.h"

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "pluginmain.h"
#include <QIcon>
#include <QMessageBox>

S2Plugin::Spelunky2* S2Plugin::Spelunky2::ptr = nullptr;

S2Plugin::Spelunky2* S2Plugin::Spelunky2::get()
{
    if (ptr == nullptr)
    {
        // see if we can get the main module
        Script::Module::ModuleInfo moduleInfo;
        if (!Script::Module::GetMainModuleInfo(&moduleInfo))
        {
            displayError("GetMainModuleInfo failed; Make sure spel2.exe is loaded");
            return nullptr;
        }
        // see if the main module is Spelunky 2
        if (std::string{"spel2.exe"}.compare(moduleInfo.name) != 0)
        {
            displayError("Main module is not spel2.exe");
            return nullptr;
        }

        // retrieve the memory map and loop every entry, until we find the .text section of spel2.exe
        uintptr_t Spelunky2CodeSectionStart{0};
        size_t Spelunky2CodeSectionSize{0};
        uintptr_t Spelunky2AfterBundle{0};

        MEMMAP memoryMap = {0};
        DbgMemMap(&memoryMap);
        for (auto i = 0; i < memoryMap.count; ++i)
        {
            MEMORY_BASIC_INFORMATION mbi = (memoryMap.page)[i].mbi;
            auto info = std::string((memoryMap.page)[i].info);
            uintptr_t baseAddress = (uintptr_t)mbi.BaseAddress;

            char name[MAX_MODULE_SIZE + 1] = {0};
            Script::Module::NameFromAddr(baseAddress, name);
            if (std::string{"spel2.exe"}.compare(name) != 0 || info.find(".text") == std::string::npos)
                continue;

            Spelunky2CodeSectionStart = baseAddress;
            Spelunky2CodeSectionSize = mbi.RegionSize;
            break;
        }

        if (Spelunky2CodeSectionStart == 0 && Spelunky2CodeSectionSize == 0)
        {
            displayError("Could not locate the .text section in the loaded spel2.exe image");
            return false;
        }

        Spelunky2AfterBundle = getAfterBundle(Spelunky2CodeSectionStart, Spelunky2CodeSectionSize);
        if (Spelunky2AfterBundle == 0)
            return false;

        THREADLIST threadList;
        uintptr_t heapBase{0};
        DbgGetThreadList(&threadList);
        for (auto x = 0; x < threadList.count; ++x)
        {
            auto threadAllInfo = threadList.list[x];
            if (threadAllInfo.BasicInfo.ThreadNumber == 0) // main thread
            {
                auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
                auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(uintptr_t)));
                auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
                heapBase = Script::Memory::ReadQword(tebAddress11Value + TEB_offset);
                break;
            }
        }
        if (heapBase == 0)
        {
            displayError("Could not retrieve heap base of the main thread!");
            return false;
        }

        auto addr = new Spelunky2{};
        addr->codeSectionStart = Spelunky2CodeSectionStart;
        addr->codeSectionSize = Spelunky2CodeSectionSize;
        addr->afterBundle = Spelunky2AfterBundle;
        addr->afterBundleSize = Spelunky2CodeSectionStart + Spelunky2CodeSectionSize - Spelunky2AfterBundle;
        addr->heapBaseAddr = heapBase;
        ptr = addr;
    }
    return ptr;
}

void S2Plugin::Spelunky2::reset()
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

// size_t S2Plugin::Spelunky2::findEntityListMapOffset()
// {
//     auto offset = Script::Pattern::FindMem(spelunky2AfterBundle(), spelunky2AfterBundleSize(), "29 5C 8F 3D");
//     offset = Script::Pattern::FindMem(offset, spelunky2AfterBundleSize(), "48 8D 8B");
//     auto entitiesOffset = Script::Memory::ReadDword(offset + 3);

//     offset = Script::Pattern::FindMem(spelunky2AfterBundle(), spelunky2AfterBundleSize(), "48 B8 02 55 A7 74 52 9D 51 43") - 7;
//     auto entitiesPtr = Script::Memory::ReadDword(offset + 3) + offset + 7;

//     auto mapOffset = Script::Memory::ReadQword(entitiesPtr) + entitiesOffset;

//     auto mapPtr = reinterpret_cast<std::unordered_map<std::string, uint16_t>*>(mapOffset);
//     dprintf("mapPtr size = %d\n", mapPtr->size());

//     return offset;
// }

uintptr_t S2Plugin::Spelunky2::find(const char* pattern, uintptr_t start, size_t size) const
{
    if (start == 0)
        start = afterBundle;

    if (size == 0)
        size = afterBundleSize - (start - afterBundle);

    return Script::Pattern::FindMem(start, size, pattern);
}

uintptr_t S2Plugin::Spelunky2::find_between(const char* pattern, uintptr_t start, uintptr_t end) const
{
    if (start == 0)
        start = afterBundle;

    size_t size;
    if (end == 0)
        size = afterBundleSize - (start - afterBundle);
    else
        size = end - start;

    return Script::Pattern::FindMem(start, size, pattern);
}

uintptr_t S2Plugin::Spelunky2::get_SaveDataPtr()
{
    auto gm = get_GameManagerPtr();
    if (gm == 0)
        return 0;
    auto heapOffsetSaveGame = Script::Memory::ReadQword(Script::Memory::ReadQword(gm + 8));
    if (heapOffsetSaveGame == 0)
        return 0;
    return heapBaseAddr + heapOffsetSaveGame;
}

std::string S2Plugin::Spelunky2::themeNameOfOffset(uintptr_t offset) const // TODO use QString?
{
    auto config = Configuration::get();
    uintptr_t firstThemeOffset = config->offsetForField(MemoryFieldType::LevelGen, "theme_dwelling", get_LevelGenPtr());

    const static auto themeNames = {
        "DWELLING",     "JUNGLE",       "VOLCANA", "OLMEC", "TIDE POOL", "TEMPLE",         "ICE CAVES", "NEO BABYLON", "SUNKEN CITY",
        "COSMIC OCEAN", "CITY OF GOLD", "DUAT",    "ABZU",  "TIAMAT",    "EGGPLANT WORLD", "HUNDUN",    "BASE CAMP",   "ARENA",
    };
    for (uint8_t idx = 0; idx < themeNames.size(); ++idx)
    {
        uintptr_t testPtr = Script::Memory::ReadQword(firstThemeOffset + idx * 0x8u);
        if (testPtr == offset)
            return *(themeNames.begin() + idx);
    }

    return "UNKNOWN THEME";
}
