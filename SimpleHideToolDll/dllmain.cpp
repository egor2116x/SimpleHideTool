// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SimpleHideToolDll.h"

HMODULE hCurrentModule = nullptr;
HMODULE hNtDll = nullptr;
HMODULE hKernel32 = nullptr;
HMODULE userenv = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if ((hNtDll = LoadLibraryA("ntdll.dll")) == NULL)
        {
            return FALSE;
        }

        if ((hKernel32 = LoadLibraryA("Kernel32.dll")) == NULL)
        {
            return FALSE;
        }

        if ((userenv = LoadLibraryA("Userenv.dll")) == NULL)
        {
            return FALSE;
        }

        hCurrentModule = hModule;
        if (!InstallHooks())
        {
            // something wrong
        }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


