reg add hklm\software\SimpleHideTool
reg add hklm\software\SimpleHideTool /v LogFlags /t reg_dword /d 0x1f
reg add hklm\software\SimpleHideTool /v SimpleHideToolLog /t reg_sz /d SimpleHideTool.txt
reg add hklm\software\SimpleHideTool /v ProcessHideList /t reg_multi_sz /d SimpleHideTool.exe\0notepad.exe\0calc.exe