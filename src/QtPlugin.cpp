#include "QtPlugin.h"
#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/State.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QFile>
#include <QMainWindow>
#include <QMdiArea>
#include <QWidget>

static QMainWindow* gsSpelunky2MainWindow;
static QMdiArea* gsMDIArea;
static S2Plugin::Configuration* gsConfiguration;
static S2Plugin::ViewToolbar* gsViewToolbar;
static S2Plugin::EntityDB* gsEntityDB;
static S2Plugin::State* gsState;

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
    gsConfiguration = new S2Plugin::Configuration();
    if (!gsConfiguration->isValid())
    {
        dprintf("Configuration error: %s\n", gsConfiguration->lastError().c_str());
    }
    else
    {
        QWidget* parent = getParent();

        gsSpelunky2MainWindow = new QMainWindow();
        gsSpelunky2MainWindow->setWindowIcon(QIcon(":/icons/caveman.png"));
        gsMDIArea = new QMdiArea();
        gsSpelunky2MainWindow->setCentralWidget(gsMDIArea);
        gsSpelunky2MainWindow->setWindowTitle("Spelunky 2");

        gsEntityDB = new S2Plugin::EntityDB(gsConfiguration);
        gsState = new S2Plugin::State(gsConfiguration);

        gsViewToolbar = new S2Plugin::ViewToolbar(gsEntityDB, gsState, gsConfiguration, gsMDIArea, parent);
        gsSpelunky2MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, gsViewToolbar);

        GuiAddQWidgetTab(gsSpelunky2MainWindow);
    }
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
