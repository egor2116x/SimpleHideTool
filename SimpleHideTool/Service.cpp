#include "stdafx.h"
#include "Service.h"
#include "Common.h"
#include "HideToolException.h"
#include "SimpleHideToolApi.h"
#include "LogWriter.h"

SERVICE_STATUS        Service::m_ServiceStatus;
SERVICE_STATUS_HANDLE Service::m_ServiceStatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

Service Service::m_instance;

VOID WINAPI Service::ServiceMain(DWORD argc, LPTSTR * argv)
{
    try
    {
        m_ServiceStatusHandle = ::RegisterServiceCtrlHandlerEx(LOG_SERVICE_NAME, ServiceCtrlHandlerEx, NULL);
        if (m_ServiceStatusHandle == nullptr)
        {
            throw ServiceError("RegisterServiceCtrlHandlerEx");
        }

        // set status of service to "start pending"
        m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        m_ServiceStatus.dwWin32ExitCode = 0;
        m_ServiceStatus.dwServiceSpecificExitCode = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_SHUTDOWN;

        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        Service::GetInstance().Run();

        WaitForSingleObject(&Service::GetInstance().m_StopEvent, INFINITE);

        ServiceCtrlHandler(SERVICE_CONTROL_STOP);
    }
    catch (DWORD dwError)
    {
        // set status of service to "stopped"
        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        m_ServiceStatus.dwControlsAccepted = 0;
        m_ServiceStatus.dwWin32ExitCode = dwError;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
    }
}

unsigned int WINAPI Service::WorkerThread(PVOID context)
{
    Service * thisPtr = reinterpret_cast<Service*>(context);

    return 0;
}

void Service::Run()
{
    if (!ShtApi::Inject())
    {
        LOG_PRINT(L"Inject unsuccessfuly\n");
        throw ServiceError("inject unsuccessfully");
    }

    if (!ShtApi::InjectIntoUserSession(nullptr))
    {
        LOG_PRINT(L"Start user mode inject unsuccessfully\n");
        throw ServiceError("inject unsuccessfully");
    }
}

void Service::Stop()
{
    LOG_PRINT(L"Start\n");
    SetEvent(m_StopEvent);
}

VOID WINAPI Service::ServiceCtrlHandler(DWORD opcode)
{
    switch (opcode)
    {
    case SERVICE_CONTROL_STOP:
    {
        // set status of service to "stopped"
        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }

    case SERVICE_CONTROL_INTERROGATE:
    {
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }
    }
}

DWORD WINAPI Service::ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
    {
        if (m_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;
        GetInstance().Stop();

        // set status of service to "stop pending"
        m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        break;
    }

    case SERVICE_CONTROL_PRESHUTDOWN:
    {
        break;
    }

    case SERVICE_CONTROL_SHUTDOWN:
    {
        break;
    }

    case SERVICE_CONTROL_INTERROGATE:
    {
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }
    case SERVICE_CONTROL_SESSIONCHANGE:
    {
        break;
    }
    case SERVICE_CONTROL_DEVICEEVENT:
    {
        break;
    }
    }
    return NO_ERROR;
}

Service::Service()
{
    m_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_StopEvent == nullptr)
    {
        throw STATUS_INVALID_HANDLE;
    }
}
