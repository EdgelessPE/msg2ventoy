#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>


#define INSTALL 0
#define UPDATE  1

HWND hCombox;
HMODULE hDll;

#define IDC_COMBO1                      1001
#define IDC_BUTTON3                     1004
#define IDC_BUTTON4                     1005
#define IDC_COMMAND1                    1024

typedef int(WINAPI*PMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef int(*PCallerCallback)(int CurrentProgress);
typedef BOOL(*PSetV2dHook)(DWORD V2dThreadId);
typedef BOOL(*PDropV2dHook)();
typedef int(*PGetCurrentProgress)(DWORD V2dThreadId, PCallerCallback Callback);

PSetV2dHook SetV2dHook;
PDropV2dHook DropV2dHook;
PGetCurrentProgress GetCurrentProgress;
PCallerCallback g_CallerCallback;

int DllGetCurrentProgress(DWORD V2dThreadId, PCallerCallback Callback)
{
    //TODO: SetV2dHook
    //TODO: 命名管道通信
    //TODO: 得到结果传给Callback
    //（dll顺便hook messagebox）
}

int ExeGetCurrentProgress(DWORD V2dThreadId, PCallerCallback Callback)
{
    //TODO: 直接循环给v2d发消息获得进度
}

BOOL CALLBACK EnumWindowsProc(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    UINT n = LOWORD(lParam);
    UINT type = HIWORD(lParam);

    WCHAR buffer[31] = { 0 };
    GetWindowTextW(hwnd, buffer, 30);
    if (wcsstr(buffer, L"Ventoy2Disk") != NULL)
    {
        puts("find Ventoy2Disk window");
        hCombox = GetDlgItem(hwnd, IDC_COMBO1);
        if (hCombox == NULL)
        {
            puts("GetDlgItem: 无效的对话框句柄或不存在的控件");
            return 1;
        }
        //ShowWindow(hwnd, SW_MINIMIZE);
        
        //刷新
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMMAND1, BN_CLICKED), 0);

        //选择磁盘
        SendMessageW(hCombox, CB_SETCURSEL, n, 0);
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO1, CBN_SELCHANGE), (LPARAM)hCombox);

        puts("Select disk ok");

        //按下制作/更新按钮
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(type ? IDC_BUTTON3 : IDC_BUTTON4, BN_CLICKED), 0);

        //反馈进度
        //hV2DHook = SetWindowsHookExW(WH_CALLWNDPROC, V2DHookProc, LoadLibraryA("msg2ventoy.dll"), GetWindowThreadProcessId(hwnd, NULL));
        //DWORD test = GetLastError();
        //printf("%ud", test);
        GetCurrentProgress(GetWindowThreadProcessId(hwnd, NULL), g_CallerCallback);

        return 0;
    }
    return 1;
}

__declspec(dllexport) int RunVentoy2Disk(unsigned short n, unsigned short type, PCallerCallback CallerCallback)
{
    g_CallerCallback = CallerCallback;

    if ((hDll = LoadLibraryW(L"hook.dll")) != NULL)
    {
        SetV2dHook = (PSetV2dHook)GetProcAddress(hDll, "SetV2dHook");
        DropV2dHook = (PDropV2dHook)GetProcAddress(hDll, "DropV2dHook");

        GetCurrentProgress = DllGetCurrentProgress;
    }
    else
    {
        GetCurrentProgress = ExeGetCurrentProgress;
    }

    EnumWindows(EnumWindowsProc, MAKELPARAM(n, type));


    return 0;
}

__declspec(dllexport) void FinishVentoy2Disk()
{
    //UnhookWindowsHookEx(hV2DHook);
    FreeLibrary(hDll);
}

int DefaultCallback(int CurrentProgress)
{
    printf("%d\n", CurrentProgress);
}

int main()
{
    INT16 n;
    INT16 type;

    puts("请输入盘符排序第n个（从0开始）");
    scanf("%hd", &n);
    puts("请输入进行的操作（0 = install，1 = update）");
    scanf("%hd", &type);

    int s = RunVentoy2Disk(n, type, DefaultCallback);
    //FinishVentoy2Disk();
    return s;
}
