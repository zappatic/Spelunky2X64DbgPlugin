#include "Spelunky2.h"
#include "Data/EntityDB.h"
#include "pluginmain.h"
#include <QIcon>
#include <QMessageBox>

static size_t gSpelunky2CodeSectionStart = 0;
static size_t gSpelunky2CodeSectionSize = 0;
static size_t gSpelunky2AfterBundle = 0;
static size_t gSpelunky2AfterBundleSize = 0;

void S2Plugin::Spelunky2::displayError(const char* fmt, ...)
{
    if (mInitErrorShown)
    {
        return;
    }
    char buffer[1024] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
    msgBox.setText(buffer);
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
    _plugin_logprintf("[Spelunky2] %s\n", buffer);
    mInitErrorShown = true;
}

void S2Plugin::Spelunky2::reset()
{
    gSpelunky2CodeSectionStart = 0;
    gSpelunky2CodeSectionSize = 0;
    gSpelunky2AfterBundle = 0;
    gSpelunky2AfterBundleSize = 0;
}

void S2Plugin::Spelunky2::findSpelunky2InMemory()
{
    if (gSpelunky2CodeSectionStart != 0)
    {
        return;
    }

    // see if we can get the main module
    Script::Module::ModuleInfo moduleInfo;
    if (!Script::Module::GetMainModuleInfo(&moduleInfo))
    {
        displayError("GetMainModuleInfo failed; Make sure spel2.exe is loaded");
        return;
    }

    // see if the main module is Spelunky 2
    std::string mainModuleName = "spel2.exe";
    if (std::string{"spel2.exe"}.compare(moduleInfo.name) != 0)
    {
        displayError("Main module is not spel2.exe");
        return;
    }

    // retrieve the memory map and loop every entry, until we find the .text section of spel2.exe
    MEMMAP memoryMap = {0};
    DbgMemMap(&memoryMap);
    for (auto i = 0; i < memoryMap.count; ++i)
    {
        MEMORY_BASIC_INFORMATION mbi = (memoryMap.page)[i].mbi;
        auto info = std::string((memoryMap.page)[i].info);
        uint64_t baseAddress = (uint64_t)mbi.BaseAddress;

        char name[MAX_MODULE_SIZE] = {0};
        Script::Module::NameFromAddr(baseAddress, name);
        if (std::string{"spel2.exe"}.compare(name) != 0 || info.find(".text") == std::string::npos)
        {
            continue;
        }
        gSpelunky2CodeSectionStart = baseAddress;
        gSpelunky2CodeSectionSize = mbi.RegionSize;
        break;
    }

    if (gSpelunky2CodeSectionStart == 0)
    {
        displayError("Could not locate the .text section in the loaded spel2.exe image");
    }

    // find the 'after_bundle' location, where the actual code is
    // only search in the last 7 megabytes
    auto sevenMegs = 7 * 1024 * 1024;
    gSpelunky2AfterBundle = Script::Pattern::FindMem(gSpelunky2CodeSectionStart + gSpelunky2CodeSectionSize - sevenMegs, sevenMegs, "48 81 EC E8 00 00 00");
    if (gSpelunky2AfterBundle == 0)
    {
        displayError("Could not locate the 'after_bundle' location");
        return;
    }
    gSpelunky2AfterBundleSize = gSpelunky2CodeSectionStart + gSpelunky2CodeSectionSize - gSpelunky2AfterBundle;
}

size_t S2Plugin::Spelunky2::spelunky2AfterBundle()
{
    if (gSpelunky2AfterBundle == 0)
    {
        findSpelunky2InMemory();
    }
    return gSpelunky2AfterBundle;
}

size_t S2Plugin::Spelunky2::spelunky2AfterBundleSize()
{
    if (gSpelunky2AfterBundle == 0)
    {
        findSpelunky2InMemory();
    }
    return gSpelunky2AfterBundleSize;
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

std::string S2Plugin::Spelunky2::getEntityName(size_t offset, EntityDB* entityDB)
{
    std::string entityName = "";
    if (offset == 0)
    {
        return entityName;
    }

    auto entityID = getEntityTypeID(offset);

    if (entityID > 0 && entityID <= entityDB->entityList()->highestID())
    {
        entityName = entityDB->entityList()->nameForID(entityID);
    }
    else
    {
        entityName = "UNKNOWN/DEAD ENTITY";
    }
    return entityName;
}

uint32_t S2Plugin::Spelunky2::getEntityTypeID(size_t offset)
{
    if (offset == 0)
    {
        return 0;
    }
    uint32_t entityDBPtr = Script::Memory::ReadQword(offset + 8);
    if (entityDBPtr == 0)
    {
        return 0;
    }
    return Script::Memory::ReadDword(entityDBPtr + 20);
}
