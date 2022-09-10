// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

LRESULT CALLBACK V2DHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    //TODO: 获取并输出进度
    //TODO: 拦截MessageBoxW
    //DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, PartDialogProc); 输yes的确认框
    

    return CallNextHookEx(NULL, code, wParam, lParam);
}

HMODULE WINAPI ModuleFromAddress(PVOID pv)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
    {
        return (HMODULE)mbi.AllocationBase;
    }
    else
    {
        return NULL;
    }
}

__declspec(dllexport) BOOL SetV2dHook(DWORD V2dThreadId)
{
    //TODO: 命名管道通信
    //TODO: （dll顺便hook messagebox）
    hV2DHook = SetWindowsHookExW(WH_CALLWNDPROC, V2DHookProc, ModuleFromAddress(V2DHookProc), V2dThreadId);
    return TRUE;
}

_declspec(dllexport) BOOL DropV2dHook()
{
    return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

