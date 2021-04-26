#include "Spelunky2.h"
#include "pluginmain.h"
#include <QMessageBox>

static size_t gSpelunky2CodeSectionStart = 0;
static size_t gSpelunky2CodeSectionSize = 0;
static size_t gSpelunky2AfterBundle = 0;
static size_t gSpelunky2AfterBundleSize = 0;

void displayError(const char* fmt, ...)
{
    char buffer[1024] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(buffer);
    msgBox.setWindowTitle("Spelunky2");
    msgBox.exec();
    _plugin_logprintf("[Spelunky2] %s\n", buffer);
}

void findSpelunky2InMemory()
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
    gSpelunky2AfterBundle = Script::Pattern::FindMem(gSpelunky2CodeSectionStart + gSpelunky2CodeSectionSize - (7 * 1024 * 1024), sevenMegs, "48 81 EC E8 00 00 00");
    if (gSpelunky2AfterBundle == 0)
    {
        displayError("Could not locate the 'after_bundle' location");
        return;
    }
    gSpelunky2AfterBundleSize = gSpelunky2CodeSectionStart + gSpelunky2CodeSectionSize - gSpelunky2AfterBundle;
}

size_t spelunky2AfterBundle()
{
    if (gSpelunky2AfterBundle == 0)
    {
        findSpelunky2InMemory();
    }
    return gSpelunky2AfterBundle;
}

size_t spelunky2AfterBundleSize()
{
    if (gSpelunky2AfterBundle == 0)
    {
        findSpelunky2InMemory();
    }
    return gSpelunky2AfterBundleSize;
}
