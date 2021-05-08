#include "pluginmain.h"
#include "QtPlugin.h"

int S2Plugin::handle;
HWND S2Plugin::hwndDlg;
int S2Plugin::hMenu;
int S2Plugin::hMenuDisasm;
int S2Plugin::hMenuDump;
int S2Plugin::hMenuStack;

PLUG_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, PLUGIN_NAME, _TRUNCATE);
    S2Plugin::handle = initStruct->pluginHandle;
    QtPlugin::Init();
    return true;
}

PLUG_EXPORT bool plugstop()
{
    GuiExecuteOnGuiThread(QtPlugin::Stop);
    QtPlugin::WaitForStop();
    return true;
}

PLUG_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    S2Plugin::hwndDlg = setupStruct->hwndDlg;
    S2Plugin::hMenu = setupStruct->hMenu;
    S2Plugin::hMenuDisasm = setupStruct->hMenuDisasm;
    S2Plugin::hMenuDump = setupStruct->hMenuDump;
    S2Plugin::hMenuStack = setupStruct->hMenuStack;
    GuiExecuteOnGuiThread(QtPlugin::Setup);
    QtPlugin::WaitForSetup();
}

PLUG_EXPORT void CBDETACH(CBTYPE cbType, PLUG_CB_DETACH* info)
{
    GuiExecuteOnGuiThread(QtPlugin::Detach);
}

PLUG_EXPORT void CBEXITPROCESS(CBTYPE cbType, PLUG_CB_EXITPROCESS* info)
{
    GuiExecuteOnGuiThread(QtPlugin::Detach);
}