#include <stdio.h>
#include <tchar.h>
#include <exception>
#include <stdexcept>
#include <Windows.h>
#include <future>
#include <vector>
#include <string>
#include "SimpleHideToolApi.h"
#include "Common.h"
#include "Utils.h"
#include "simplehidetooltestqt_ui.h"

SimpleHideToolTestQT_UI::SimpleHideToolTestQT_UI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QObject::connect(ui.pushButton, &QPushButton::clicked, this, &SimpleHideToolTestQT_UI::InstallLibs);
    QObject::connect(ui.pushButton_2, &QPushButton::clicked, this, &SimpleHideToolTestQT_UI::UninstallLibs);
    QObject::connect(ui.pushButton_3, &QPushButton::clicked, this, &SimpleHideToolTestQT_UI::InjectSlot);
    QObject::connect(ui.pushButton_4, &QPushButton::clicked, this, &SimpleHideToolTestQT_UI::InjectIntoUserSessionSlot);
}

SimpleHideToolTestQT_UI::~SimpleHideToolTestQT_UI()
{

}

void SimpleHideToolTestQT_UI::InjectSlot()
{

}

void SimpleHideToolTestQT_UI::InjectIntoUserSessionSlot()
{

}

void SimpleHideToolTestQT_UI::InstallLibs()
{
    // dll install
    /*std::wstring x86_DLL_directory = GetFullCurrentProcessPathToFolder();
    x86_DLL_directory += X86hookDllPath;
    std::wstring x64_DLL_directory = GetFullCurrentProcessPathToFolder();
    x64_DLL_directory += X64hookDllPath;*/

    /*if (!ShtApi::Install(x86_DLL_directory, x64_DLL_directory))
    {
        ui.statusBar->showMessage("Dll installation was unsuccessful");
    }*/
    ui.statusBar->showMessage("Dll install failed");
}

void SimpleHideToolTestQT_UI::UninstallLibs()
{
    /*if (!ShtApi::Uninstall())
    {
        ui.statusBar->showMessage("Dll uninstall was unsuccessful");
        return;
    }*/

    ui.statusBar->showMessage("Dll uninstall failed");
}

void SimpleHideToolTestQT_UI::SetProcessList()
{

}

void SimpleHideToolTestQT_UI::GetProcessList()
{

}