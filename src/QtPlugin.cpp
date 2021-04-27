#include "QtPlugin.h"
#include "Data/EntityDB.h"
#include "Data/State.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QFile>
#include <QMainWindow>
#include <QMdiArea>
#include <QWidget>

static ViewToolbar* gsViewToolbar;
static QMainWindow* gsSpelunky2MainWindow;
static QMdiArea* gsMDIArea;
static EntityDB* gsEntityDB;
static State* gsState;

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

void QtPlugin::Setup()
{
    QWidget* parent = getParent();

    gsSpelunky2MainWindow = new QMainWindow();
    gsSpelunky2MainWindow->setWindowIcon(QIcon(":/icons/caveman.png"));
    gsMDIArea = new QMdiArea();
    gsSpelunky2MainWindow->setCentralWidget(gsMDIArea);
    gsSpelunky2MainWindow->setWindowTitle("Spelunky 2");

    gsEntityDB = new EntityDB();
    gsState = new State();

    gsViewToolbar = new ViewToolbar(gsEntityDB, gsState, gsMDIArea, parent);
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
