#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

#include "detours.h"
#pragma comment(lib, "detours.lib")

typedef int (WINAPI* PMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef int (*PCallerCallback)(int CurrentProgress);

#pragma data_seg("SHARED") 
PCallerCallback g_CallerCallback = NULL;
HHOOK hV2DHook = NULL;
#pragma data_seg()         
#pragma comment(linker, "/section:SHARED,RWS")
