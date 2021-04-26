#include "QtPlugin.h"
#include "Data/EntityDB.h"
#include "ViewToolbar.h"
#include "pluginmain.h"
#include <QFile>
#include <QMainWindow>
#include <QMdiArea>
#include <QWidget>

static ViewToolbar* gsViewToolbar;
static QMainWindow* gsSpelunky2MainWindow;
static QMdiArea* gsMDIArea;
static EntityDB* gsEntityDB;

static HANDLE hSetupEvent;
static HANDLE hStopEvent;

static QByteArray getResourceBytes(const char* path)
{
    QByteArray b;
    QFile s(path);
    if (s.open(QFile::ReadOnly))
        b = s.readAll();
    return b;
}

static QWidget* getParent()
{
    return QWidget::find((WId)S2Plugin::hwndDlg);
}

void QtPlugin::Init()
{
    hSetupEvent = CreateEventW(nullptr, true, false, nullptr);
    hStopEvent = CreateEventW(nullptr, true, false, nullptr);
}

// enum
// {
//     MENU_DISASM_DECOMPILE_SELECTION,
//     MENU_DISASM_DECOMPILE_FUNCTION,
//     MENU_GRAPH_DECOMPILE,
// };

void QtPlugin::Setup()
{
    QWidget* parent = getParent();

    gsSpelunky2MainWindow = new QMainWindow();
    gsSpelunky2MainWindow->setWindowIcon(QIcon(":/icons/caveman.png"));
    gsMDIArea = new QMdiArea();
    gsSpelunky2MainWindow->setCentralWidget(gsMDIArea);
    gsSpelunky2MainWindow->setWindowTitle("Spelunky 2");
    gsEntityDB = new EntityDB();

    gsViewToolbar = new ViewToolbar(gsEntityDB, gsMDIArea, parent);
    gsSpelunky2MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, gsViewToolbar);

    GuiAddQWidgetTab(gsSpelunky2MainWindow);
    SetEvent(hSetupEvent);
}

void QtPlugin::WaitForSetup()
{
    WaitForSingleObject(hSetupEvent, INFINITE);
}

void QtPlugin::Stop()
{
    GuiCloseQWidgetTab(gsSpelunky2MainWindow);
    gsSpelunky2MainWindow->close();
    delete gsSpelunky2MainWindow;
    SetEvent(hStopEvent);
}

void QtPlugin::WaitForStop()
{
    WaitForSingleObject(hStopEvent, INFINITE);
}

void QtPlugin::ShowTab()
{
    GuiShowQWidgetTab(gsSpelunky2MainWindow);
}

void QtPlugin::MenuPrepare(int hMenu)
{
    // if(hMenu == GUI_DISASM_MENU)
    // {
    //     SELECTIONDATA sel = { 0, 0 };
    //     GuiSelectionGet(GUI_DISASSEMBLY, &sel);
    //     auto selectionInFunction = DbgFunctionGet(sel.start, nullptr, nullptr);
    //     _plugin_menuentrysetvisible(Plugin::handle, MENU_DISASM_DECOMPILE_FUNCTION, selectionInFunction);
    // }
}

void QtPlugin::MenuEntry(int hEntry)
{
    // switch(hEntry)
    // {
    // case MENU_DISASM_DECOMPILE_SELECTION:
    // {
    //     if(!DbgIsDebugging())
    //         return;

    //     SELECTIONDATA sel = { 0, 0 };
    //     GuiSelectionGet(GUI_DISASSEMBLY, &sel);

    //     ShowTab();
    //     SnowmanRange range{ sel.start, sel.end + 1 };
    //     snowman->decompileAt(&range, 1);
    // }
    // break;

    // case MENU_DISASM_DECOMPILE_FUNCTION:
    // {
    //     if(!DbgIsDebugging())
    //         return;

    //     SELECTIONDATA sel = { 0, 0 };
    //     GuiSelectionGet(GUI_DISASSEMBLY, &sel);
    //     duint addr = sel.start;
    //     duint start;
    //     duint end;
    //     if(DbgFunctionGet(addr, &start, &end))
    //     {
    //         BASIC_INSTRUCTION_INFO info;
    //         DbgDisasmFastAt(end, &info);
    //         end += info.size - 1;

    //         ShowTab();
    //         SnowmanRange range{ start,end };
    //         snowman->decompileAt(&range, 1);
    //     }
    // }
    // break;

    // case MENU_GRAPH_DECOMPILE:
    // {
    //     if(!DbgIsDebugging())
    //         return;

    //     BridgeCFGraphList graphList;
    //     GuiGetCurrentGraph(&graphList);
    //     BridgeCFGraph currentGraph(&graphList, true);
    //     if(currentGraph.nodes.empty())
    //         return;

    //     std::vector<SnowmanRange> ranges;
    //     ranges.reserve(currentGraph.nodes.size());
    //     for(const auto & nodeIt : currentGraph.nodes)
    //     {
    //         SnowmanRange r;
    //         const BridgeCFNode & node = nodeIt.second;
    //         r.start = node.instrs.empty() ? node.start : node.instrs[0].addr;
    //         r.end = node.instrs.empty() ? node.end : node.instrs[node.instrs.size() - 1].addr;
    //         BASIC_INSTRUCTION_INFO info;
    //         DbgDisasmFastAt(r.end, &info);
    //         r.end += info.size - 1;
    //         ranges.push_back(r);
    //     }
    //     std::sort(ranges.begin(), ranges.end(), [](const SnowmanRange & a, const SnowmanRange & b)
    //     {
    //         return a.start > b.start;
    //     });

    //     ShowTab();
    //     snowman->decompileAt(ranges.data(), ranges.size());
    // }
    // break;
    // }
}