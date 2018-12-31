// SimpleHideTool.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "HideToolException.h"
#include "Common.h"
#include "Service.h"
#include "SimpleHideToolApi.h"
#include "Utils.h"
#include "LogWriter.h"

int wmain(int argc, TCHAR* argv[])
{
    std::wstring params;
    params = (argc == 2 ? argv[1] : L"");

    if (_wcsicmp(L"install", params.c_str()) == 0) 
    {
        // dll install
        std::wstring x86_DLL_directory = GetFullCurrentProcessPathToFolder();
        x86_DLL_directory += X86hookDllPath;
        std::wstring x64_DLL_directory = GetFullCurrentProcessPathToFolder();
        x64_DLL_directory += X64hookDllPath;

        if (!ShtApi::Install(x86_DLL_directory, x64_DLL_directory))
        {
            LOG_PRINT(L"Install dll unsuccessfully\n");
            return 1;
        }
    }
    else if (_wcsicmp(L"uninstall", params.c_str()) == 0)
    {
        if (!ShtApi::Uninstall())
        {
            LOG_PRINT(L"Uninstall dll unsuccessfully\n");
            return 1;
        }
    }
    else if (_wcsicmp(L"inject", params.c_str()) == 0)
    {
        if (!ShtApi::Inject())
        {
            LOG_PRINT(L"Inject dll unsuccessfully\n");
            return 1;
        }
        system("pause");
    }

    if (argc == 2)
    {
        return 0;
    }

    SERVICE_TABLE_ENTRY dispatchTable[] = { { LOG_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)Service::ServiceMain },{ NULL, NULL } };
    try
    {
        if (!::StartServiceCtrlDispatcher(dispatchTable))
        {
            std::wstring mes(L"StartServiceCtrlDispatcher error : ");
            mes += std::to_wstring(static_cast<unsigned long>(GetLastError()));
            mes += L'\n';
            LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
            DWORD error = ::GetLastError();
        }
    }
    catch (const ServiceError & e)
    {
        UNREFERENCED_PARAMETER(e);
        DWORD error = ::GetLastError();
        std::wstring mes(L"ServiceError caught error : ");
        mes += std::to_wstring(static_cast<unsigned long>(e.GetError()));
        mes += ' ';
        mes += str2wstr(e.what());
        mes += L'\n';
        LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    }
    catch (const WriterLoggerError & e)
    {
        UNREFERENCED_PARAMETER(e);
        DWORD error = ::GetLastError();
        std::wstring mes(L"WriterLoggerError caught error : ");
        mes += std::to_wstring(static_cast<unsigned long>(e.GetError()));
        mes += ' ';
        mes += str2wstr(e.what());
        mes += L'\n';
        LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    }
    return 0;
}


