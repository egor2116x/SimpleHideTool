#pragma once

#define LOG_SERVICE_NAME L"SimpleHideToolSrv"

#define X86HookDllName   L"SimpleHideToolDll_X86.dll"
#define X64HookDllName   L"SimpleHideToolDll_X64.dll"

#define CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER L';'
#define CONFIG_REGISTRY_USER_MODE_SUBKEY L"Software\\SimpleHideTool"
#define CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER        L';'
#define CONFIG_REGISTRY_PROCESS_HIDE L"ProcessHideList"

#define LOG_KEY_FILE_NAME L"SimpleHideToolLog"
#define LOG_FILE_NAME L"SimpleHideTool.txt"
#define LOG_SERVICE_NAME L"SimpleHideToolSrv"
#define LOG_SERVICE_BIN_NAME L"SimpleHideToolSrv.exe"
#define LOG_KEY_FLAGS L"LogFlags"

// flags for messages filtering
#define LOG_FATAL   0x00000001
#define LOG_ERROR   0x00000002
#define LOG_WARNING 0x00000004
#define LOG_INFO    0x00000008
#define LOG_FAILED  0x00000010

#ifdef _DEBUG
#define IF_DEBUG 1
#else
#define IF_DEBUG 0
#endif
