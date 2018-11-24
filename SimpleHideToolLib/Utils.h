#pragma once
#include "stdafx.h"
#include "Common.h"

std::wstring GetFullProcessPath();
bool LoadMultiString(const WCHAR * valueName, std::vector<std::wstring> & processExceptionList);
std::wstring GetFullCurrentProcessPath();
std::wstring GetFullCurrentProcessPathToFolder();
std::wstring GetCurrentProcessName();
std::wstring GetSystemDirPathWow64();
std::wstring toLowerW(const std::wstring & str);
std::string toLowerA(const std::string & str);

std::string wstr2str(const std::wstring & str);
std::wstring str2wstr(const std::string & str);

template<bool condition = true, typename Output, typename Arg, typename ... Args>
inline void SendToConsoleIf(Output & out, Arg && arg, Args &&... args)
{
    if (condition)
    {
        out << std::forward<Arg>(arg);
        using expander = int[];
        (void)expander {
            0, (void(out << std::forward<Args>(args)), 0)...
        };
        out << std::endl;
    }
}