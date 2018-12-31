#include "stdafx.h"
#include "SimpleHideToolApi.h"
#include "Common.h"
#include "Utils.h"

BOOL IfFileExist(const std::wstring & fullPathToFile)
{
    HANDLE hFile = ::CreateFile(fullPathToFile.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, // security attributes
        OPEN_EXISTING, // disposition
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, // flags & attributes
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    ::CloseHandle(hFile);
    return TRUE;
}

BOOL MoveDllEx(const std::wstring & dllSysDirPath, const std::wstring & dllName)
{
    BOOL res = FALSE;
    std::wstring Target = dllSysDirPath;
    Target += L'\\';
    Target += dllName;

    if (!IfFileExist(Target))
    {
        return res;
    }

    WCHAR buff[MAX_PATH];
    UINT sz = GetTempFileName(dllSysDirPath.c_str(),
        dllName.c_str(),
        0,
        buff);
    if (sz == 0)
    {
        return res;
    }

    res = DeleteFile(buff); // delete empty tmp file, because rename will be fail
    if (!res)
    {
        return res;
    }

    //rename
    res = MoveFile(Target.c_str(), buff);
    if (!res)
    {
        return res;
    }

    //delete
    res = MoveFileEx(buff, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    if (!res)
    {
        return res;
    }
    return res;
}

BOOL MoveDll(const std::wstring & dllSysDirPath, int bitness)
{
    BOOL result = FALSE;
    DWORD error;

    if (bitness == 32)
        result = MoveDllEx(dllSysDirPath, X86HookDllName);
    else
        result = MoveDllEx(dllSysDirPath, X64HookDllName);

    if (!result)
    {
        error = GetLastError();
    }
    return result;
}

BOOL UnInstallDllFromSys32(int bitness)
{
    BOOL res = FALSE;

    WCHAR buff[MAX_PATH + 1];

    UINT sz = GetSystemDirectory(buff, MAX_PATH + 1); // system32 dir
    if (sz == 0)
    {
        return res;
    }

    res = MoveDll(buff, bitness);

    return res;
}

BOOL UnInstallDllFromSysWow64()
{
    BOOL res = FALSE;

    std::wstring buff = GetSystemDirPathWow64(); // syswow64

    res = MoveDll(buff, 32);

    return res;
}

BOOL UninstallHookDlls()
{
    if (!GetSystemDirPathWow64().empty()) // if windows_x64
    {
        BOOL resSys32 = UnInstallDllFromSys32(64);
        BOOL resSysWow64 = UnInstallDllFromSysWow64();
        return (resSys32 || resSysWow64);
    }

    return  UnInstallDllFromSys32(32);
}

BOOL CopyFromTo(const std::wstring & dllSourceDirPath, const std::wstring & dllSysDirPath, int bitness)
{
    BOOL res = FALSE;

    std::wstring Source = dllSourceDirPath;
    Source += L'\\';

    if (bitness == 32)
        Source += X86HookDllName;
    else
        Source += X64HookDllName;

    std::wstring Target = dllSysDirPath;
    Target += L'\\';

    if (bitness == 32)
        Target += X86HookDllName;
    else
        Target += X64HookDllName;

    res = CopyFile(Source.c_str(), Target.c_str(), FALSE);
    DWORD err;
    if (!res)
    {
        err = GetLastError();
        return res;
    }
    return res;
}

BOOL InstallDllToSys32(const std::wstring & dllSourceDirPath, int bitness)
{
    BOOL res = FALSE;
    if (dllSourceDirPath.empty())
    {
        return res;
    }

    WCHAR buff[MAX_PATH + 1];

    UINT sz = GetSystemDirectory(buff, MAX_PATH + 1); // system32 dir
    if (sz == 0)
    {
        return res;
    }

    res = CopyFromTo(dllSourceDirPath, buff, bitness);

    return res;
}

BOOL InstallDllToSysWow64(const std::wstring & dllSourceDirPath)
{
    BOOL res = FALSE;
    if (dllSourceDirPath.empty())
    {
        return res;
    }

    res = CopyFromTo(dllSourceDirPath, GetSystemDirPathWow64(), 32);

    return res;
}

BOOL InstallHookDlls(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64)
{
    if (!GetSystemDirPathWow64().empty()) // if windows_x64
    {
        return (InstallDllToSys32(dllSourceDirPathX64, 64) &&
            InstallDllToSysWow64(dllSourceDirPathX86));
    }

    return  (InstallDllToSys32(dllSourceDirPathX86, 32));
}

BOOL ShtApi::Install(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64)
{
    BOOL result = InstallHookDlls(dllSourceDirPathX86, dllSourceDirPathX64);
    if (!result)
    {
        return result;
    }
   
    return result;
}

BOOL ShtApi::Uninstall()
{
#ifdef _M_X64
    HMODULE hModule = GetModuleHandle(X64HookDllName);
#else
    HMODULE hModule = GetModuleHandle(X86HookDllName);
#endif

    if (hModule != nullptr)
    {
        FreeLibrary(hModule);
    }

    return UninstallHookDlls();
}

BOOL ShtApi::Inject()
{
#ifdef _M_X64
    HMODULE hModule = LoadLibraryW(X64HookDllName);
#else
    HMODULE hModule = LoadLibrary(X86HookDllName);
#endif
    if (hModule == nullptr)
    {
        DWORD result = GetLastError();
        result = FALSE;
    }
    bool(WINAPI * CurStart)() = nullptr;
#ifdef _M_X64
    hModule = GetModuleHandle(X64HookDllName);
    CurStart = (bool(WINAPI *)())GetProcAddress(hModule, "?Start@@YA_NXZ");
#else
    hModule = GetModuleHandle(X86HookDllName);
    CurStart = (bool(WINAPI *)())GetProcAddress(hModule, "?Start@@YA_NXZ");
#endif
    if (hModule == nullptr)
    {
        DWORD result = GetLastError();
        return FALSE;
    }
    //
    // Inject support dll
    //
    if (CurStart != nullptr)
    {
        bool result = CurStart();
        if (!result)
        {
            return FALSE;
        }
    }

    return TRUE;
}

UINT ShtApi::InjectIntoUserSession(LPVOID pParam)
{
    DWORD dwError;

    while (true)
    {
        HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        DWORD processID = 0;
        DWORD processSessionID = 0;
        WCHAR* pBuffer = NULL;
        DWORD pBytesReturned = 0;
        std::wstring sUserName;

        // GetExplorer PID
        do
        {
            if (hSnapShot == INVALID_HANDLE_VALUE)
                break;

            PROCESSENTRY32W pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32W);

            if (Process32FirstW(hSnapShot, &pe32) == FALSE)
                break;

            do
            {
                pe32.szExeFile;

                ProcessIdToSessionId(pe32.th32ProcessID, &processSessionID);
                if (processSessionID == 0)
                    continue;

                if (pe32.th32ProcessID == 0)
                    continue;

                //	if( isUserInAdminGroup( pe32.th32ProcessID ) == FALSE )
                //		continue;

                if (0 != _wcsicmp(pe32.szExeFile, L"EXPLORER.EXE"))
                    continue;

                {

                    HANDLE h = ::OpenProcess(MAXIMUM_ALLOWED, TRUE, pe32.th32ProcessID);
                    if (h == NULL)
                        break;

                    HANDLE hProcessToken = NULL;
                    if (!::OpenProcessToken(h, TOKEN_READ, &hProcessToken) || !hProcessToken)
                        return false;

                    DWORD dwProcessTokenInfoAllocSize = 0;
                    ::GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &dwProcessTokenInfoAllocSize);

                    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>(new BYTE[dwProcessTokenInfoAllocSize]);
                        if (pUserToken != NULL)
                        {
                            if (::GetTokenInformation(hProcessToken, TokenUser, pUserToken, dwProcessTokenInfoAllocSize, &dwProcessTokenInfoAllocSize) == TRUE)
                            {
                                SID_NAME_USE snuSIDNameUse;
                                WCHAR szUser[MAX_PATH] = { 0 };
                                DWORD dwUserNameLength = MAX_PATH;
                                WCHAR szDomain[MAX_PATH] = { 0 };
                                DWORD dwDomainNameLength = MAX_PATH;

                                if (::LookupAccountSidW(NULL, pUserToken->User.Sid, szUser, &dwUserNameLength,
                                    szDomain, &dwDomainNameLength, &snuSIDNameUse))
                                {
                                    sUserName = szUser;

                                    CloseHandle(hProcessToken);
                                    delete[] pUserToken;
                                    pUserToken = NULL;
                                }
                            }

                            if (pUserToken != NULL)
                                delete[] pUserToken;
                        }
                    }
                }

                if (sUserName.compare(L"SYSTEM") == 0)
                    continue;

                processID = pe32.th32ProcessID;
                break;

            } while (Process32NextW(hSnapShot, &pe32) != FALSE);

        } while (false);

        if (hSnapShot != INVALID_HANDLE_VALUE)
            CloseHandle(hSnapShot);

        // GetActiveSession
        DWORD dwSessionId = UINT_MAX;
        {
            PWTS_SESSION_INFOW pSessionInfo = NULL;
            DWORD dwCount = 0;

            WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

            for (DWORD i = 0; i < dwCount; ++i)
            {
                WTS_SESSION_INFOW si = pSessionInfo[i];
                if (WTSActive == si.State)
                {
                    dwSessionId = si.SessionId;
                    break;
                }
            }

            WTSFreeMemory(pSessionInfo);
        }

        // CreateProcess
        BOOL isSuccess = FALSE;
        HANDLE hProcess = NULL;
        HANDLE hToken = NULL;
        HANDLE hTokenDup = NULL;
        LPVOID pEnv = NULL;
        LPWSTR lpszPath = NULL;
        LPWSTR* wszCommandLineItems = NULL;

        hProcess = ::OpenProcess(MAXIMUM_ALLOWED, TRUE, processID);
        if (hProcess == NULL)
            continue;

        isSuccess = OpenProcessToken(hProcess,
            TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ADJUST_SESSIONID,
            &hToken);
        if (isSuccess == FALSE)
            continue;

#define MAKE_ELEVATION 0

#if MAKE_ELEVATION == 0

        if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
            continue;

#else

        HANDLE linkedToken = GetLinkedToken(hToken);
        if (linkedToken == 0)
        {
            //linkedToken = hToken;
            break;
        }

        if (!DuplicateTokenEx(linkedToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
            break;

        CloseHandle(linkedToken);

#endif

        SetTokenInformation(hTokenDup, TokenSessionId, (LPVOID)&dwSessionId, sizeof(DWORD));

#if SWITCH_PARENT > 0
        STARTUPINFOEXW sie;
        ZeroMemory(&sie, sizeof(sie));
        sie.StartupInfo.cb = sizeof(sie);
        sie.StartupInfo.lpDesktop = L"winsta0\\default";

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS
            | CREATE_NEW_CONSOLE
            | EXTENDED_STARTUPINFO_PRESENT;
#else
        // CreateProcessAsUser
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(STARTUPINFOW));
        si.cb = sizeof(STARTUPINFOW);
        si.lpDesktop = (LPWSTR)L"winsta0\\default";

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW;
#endif

        if (CreateEnvironmentBlock(&pEnv, hTokenDup, TRUE))
            dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
        else
            pEnv = NULL;

        lpszPath = _wcsdup(L"SimpleHideTool.exe inject"); // fileFullPath
        if (lpszPath == NULL)
            continue;

#if SWITCH_PARENT > 0
        DWORD ParentId = processID;

        // Add temporal thread attributes
        const DWORD dwAttributeCount = 1;
        SIZE_T cbAttributeListSize = 0;
        isSuccess = InitializeProcThreadAttributeList(
            NULL,
            dwAttributeCount,
            0, //reserved and must be zero
            &cbAttributeListSize);

        if (!isSuccess)
        {
            DTRACE(DTRACE_USER_PROCESS, "cbAttributeListSize=%d\n", (int)cbAttributeListSize);
            dwError = ::GetLastError();
            if (dwError != ERROR_INSUFFICIENT_BUFFER)
            {
                DTRACE(DTRACE_USER_PROCESS, "InitializeProcThreadAttributeList(size) failed 0x%08X\n", dwError);
                return dwError;
            }
        }

        std::vector<BYTE> bufferThreadAttributeList(cbAttributeListSize);
        PPROC_THREAD_ATTRIBUTE_LIST pAttributeList =
            reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&bufferThreadAttributeList[0]);

        isSuccess = InitializeProcThreadAttributeList(
            pAttributeList,
            dwAttributeCount,
            0, //reserved and must be zero
            &cbAttributeListSize);

        if (!isSuccess)
        {
            dwError = ::GetLastError();
            DTRACE(DTRACE_USER_PROCESS, "InitializeProcThreadAttributeList(object) failed 0x%08X\n", dwError);
            return dwError;
        }

        utils::CHandleGuard hParentProcess(
            ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, ParentId)
        );

        if (NULL == hParentProcess)
        {
            dwError = ::GetLastError();
            DTRACE(DTRACE_USER_PROCESS, "OpenProcess(Parent) failed 0x%08X\n", dwError);
            return dwError;
        }

        isSuccess = UpdateProcThreadAttribute(
            pAttributeList,
            0, //reserved and must be 0
            PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
            &hParentProcess,
            sizeof(HANDLE),
            NULL,  //reserved and must be NULL
            NULL   //reserved and must be NULL
        );

        if (!isSuccess)
        {
            dwError = ::GetLastError();
            DTRACE(DTRACE_USER_PROCESS, "UpdateProcThreadAttribute failed 0x%08X\n", dwError);
            return dwError;
        }

        sie.lpAttributeList = pAttributeList;
#endif

#if SWITCH_IMPERSONATION > 0
        //DWORD threadId = 0;
        //CScopedImpersonation scopedImpersonation (threadId, processID);

        ::SetLastError(0);
        isSuccess = ::ImpersonateLoggedOnUser(hTokenDup);
        DTRACE(DTRACE_USER_PROCESS, "ImpersonateLoggedOnUser return %d, error 0x%08X\n", isSuccess, ::GetLastError());
#endif

#if 0
        if (EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME, TRUE) == FALSE)
        {
            DTRACE(DTRACE_USER_PROCESS, "SE_ASSIGNPRIMARYTOKEN_NAME FAILED error code : %u \n", ::GetLastError());
        }
        else
        {
            DTRACE(DTRACE_USER_PROCESS, "SE_ASSIGNPRIMARYTOKEN_NAME SUCCESS \n", ::GetLastError());
        }
#endif

        SetLastError(0);
#if SWITCH_PARENT > 0
        isSuccess = ::CreateProcessAsUserW(
            hTokenDup,
            NULL,
            lpszPath,
            NULL,
            NULL,
            FALSE,
            dwCreationFlags,
            pEnv,
            NULL,
            &sie.StartupInfo,
            &pi);
#else
        isSuccess = ::CreateProcessAsUserW(
            hTokenDup,
            NULL,
            lpszPath,
            NULL,
            NULL,
            FALSE,
            dwCreationFlags,
            pEnv,
            NULL,
            &si,
            &pi);
#endif
        DWORD err = (isSuccess == FALSE ? GetLastError() : ERROR_SUCCESS);
        if (isSuccess != FALSE)
        {
            //WaitForSingleObject(pi.hProcess, INFINITE);

            if (pi.hProcess != NULL)
                CloseHandle(pi.hProcess);
            if (pi.hThread != NULL)
                CloseHandle(pi.hThread);
            break;
        }

#if SWITCH_PARENT > 0
        if (pAttributeList != NULL)
        {
            DeleteProcThreadAttributeList(pAttributeList);
        }
#endif

#if SWITCH_IMPERSONATION > 0
        ::SetLastError(0);
        isSuccess = ::RevertToSelf();
        DTRACE(DTRACE_USER_PROCESS, "RevertToSelf return %d, error 0x%08X\n", isSuccess, ::GetLastError());
#endif

        Sleep(2000);
    }

    return 1;
}



bool ShtApi::SetHideProcessList(const std::vector<std::wstring>& processList)
{
    if (processList.empty())
    {
        return false;
    }

    LSTATUS status = 0;
    HKEY  hKey;

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        CONFIG_REGISTRY_USER_MODE_SUBKEY,
        0,
        KEY_READ | KEY_WRITE | KEY_QUERY_VALUE,
        &hKey);

    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    std::wstring buff;
    for (const auto & name : processList)
    {
        buff += name;
        buff += L'\0';
    }
    buff += L'\0';

    status = RegSetValueExW(hKey, CONFIG_REGISTRY_PROCESS_HIDE, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(&buff[0]), buff.size() / sizeof(wchar_t));
    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool ShtApi::GetHideProcessList(std::vector<std::wstring>& processList)
{
    return LoadMultiString(CONFIG_REGISTRY_PROCESS_HIDE, processList);
}

