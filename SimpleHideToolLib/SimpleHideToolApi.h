#pragma once

namespace ShtApi
{
    // manage dlls
    BOOL Install(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64);
    BOOL Uninstall();

    // manage inject
    BOOL Inject();
    UINT InjectIntoUserSession(LPVOID pParam);

    // manage hide process list
    bool SetHideProcessList(const std::vector<std::wstring> & processList);
    bool GetHideProcessList(std::vector<std::wstring> & processList);
}

