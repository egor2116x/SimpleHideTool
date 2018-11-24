#pragma once
#include "stdafx.h"

class Service
{
public:
    static Service & GetInstance() { return m_instance; }
    void Run();
    void Stop();
    static VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
    static VOID WINAPI ServiceCtrlHandler(DWORD opcode);
    static DWORD WINAPI ServiceCtrlHandlerEx(
        DWORD dwControl,
        DWORD dwEventType,
        LPVOID lpEventData,
        LPVOID lpContext
    );
private:
    Service();
    Service(const Service &) = delete;
    Service(const Service &&) = delete;
    Service & operator=(const Service &) = delete;
    Service & operator=(const Service &&) = delete;
    static unsigned int WINAPI WorkerThread(PVOID context);
private:
    static Service m_instance;
    static SERVICE_STATUS m_ServiceStatus;
    static SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
    std::future<unsigned int> m_workerThreadFuture;
    HANDLE m_StopEvent;
};


