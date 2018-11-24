#include "stdafx.h"
#include "Utils.h"

std::wstring GetSystemDirPathWow64()
{

    WCHAR SystemWow64Directory[MAX_PATH + 1];

    UINT(WINAPI * CurGetSystemWow64Directory)(__out LPTSTR lpBuffer, __in UINT uSize) = NULL;
    CurGetSystemWow64Directory =
        (UINT(WINAPI *)(LPTSTR, UINT))GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemWow64DirectoryW");
    if (!CurGetSystemWow64Directory)
        return L"";

    UINT sz = CurGetSystemWow64Directory(SystemWow64Directory, MAX_PATH + 1);
    if (sz == 0 || GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
    {
        return L"";
    }

    return std::wstring(SystemWow64Directory, sz);
}

std::wstring toLowerW(const std::wstring & str)
{
    std::wstring tmp = str;
    for (auto & ch : tmp)
    {
        ch = std::towlower(ch);
    }
    return tmp;
}

std::string toLowerA(const std::string & str)
{
    std::string tmp = str;
    for (auto & ch : tmp)
    {
        ch = std::tolower(ch);
    }
    return tmp;
}

std::wstring GetFullCurrentProcessPath()
{
    DWORD dwResult = NO_ERROR;
    std::wstring fullPath;
    fullPath.resize(MAX_PATH);

    for (;;)
    {
        dwResult = GetModuleFileName(NULL, &fullPath[0], static_cast<DWORD>(fullPath.size()));
        if (dwResult == 0)
        {
            return std::wstring();
        }
        else if (dwResult == fullPath.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            fullPath.resize(fullPath.size() * 2);
            continue;
        }
        break;
    }

    //fullPath.resize(dwResult);

    return fullPath;
}

std::wstring GetFullCurrentProcessPathToFolder()
{
    std::wstring fullPath = GetFullCurrentProcessPath();

    std::wstring::size_type pos = fullPath.rfind(L'\\');
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }

    return fullPath.substr(0, pos);
}


std::wstring GetCurrentProcessName()
{
    std::wstring fullPath = GetFullCurrentProcessPath();

    std::wstring::size_type pos = fullPath.rfind(L'\\');
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }
    fullPath = fullPath.substr(pos + 1, fullPath.size() - pos);

    return fullPath;
}

std::string wstr2str(const std::wstring & str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(str);
}
std::wstring str2wstr(const std::string & str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str.c_str());
}

bool LoadMultiString(const WCHAR * valueName, std::vector<std::wstring> & processExceptionList)
{
    LSTATUS status = 0;
    HKEY  hKey;

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        CONFIG_REGISTRY_USER_MODE_SUBKEY,
        0,
        KEY_READ | KEY_QUERY_VALUE,
        &hKey);

    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    DWORD dwSize = 0;
    DWORD dwType = 0;

    status = RegQueryValueExW(hKey,
        valueName,
        NULL,
        &dwType,
        NULL,
        &dwSize);

    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    if (dwType != REG_MULTI_SZ)
    {
        return false;
    }

    if (dwSize < sizeof(WCHAR))
    {
        return false;
    }

    std::wstring buff;
    buff.resize(dwSize / sizeof(wchar_t));

    status = RegQueryValueExW(hKey,
        valueName,
        NULL,
        &dwType,
        reinterpret_cast<PBYTE>(&buff[0]),
        &dwSize);

    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    while (!buff.empty() && buff.back() == L'\0')
    {
        buff.resize(buff.size() - 1);
    }

    for (size_t i = 0; i<buff.size(); ++i)
    {
        if (buff[i] == L'\0')
        {
            buff[i] = CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER;
        }
    }
    buff += CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER;

    std::wstring::size_type pos = buff.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER, 0);

    if (pos == std::string::npos && !buff.empty())
    {
        processExceptionList.push_back(buff);
    }

    while (pos != std::string::npos)
    {
        const std::wstring str = buff.substr(0, pos);
        processExceptionList.push_back(str);
        if (pos + 1 > buff.size() - pos)
        {
            break;
        }
        buff = buff.substr(pos + 1, buff.size() - pos);
        pos = buff.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER, 0);

        if (pos == std::string::npos && !buff.empty())
        {
            processExceptionList.push_back(buff);
        }
    }

    return true;
}

std::wstring GetFullProcessPath()
{
    DWORD dwResult = NO_ERROR;
    std::vector<WCHAR> pathBuffer;
    pathBuffer.resize(MAX_PATH);
    std::wstring fullPath;

    for (;;)
    {
        dwResult = GetModuleFileName(NULL, &pathBuffer.at(0), static_cast<DWORD>(pathBuffer.size()));
        if (dwResult == 0)
        {
            return std::wstring();
        }
        else if (dwResult == pathBuffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pathBuffer.resize(pathBuffer.size() * 2);
            continue;
        }
        break;
    }

    fullPath.assign(&pathBuffer.at(0), dwResult);

    std::wstring::size_type pos = fullPath.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }

    return fullPath.substr(0, pos);
}