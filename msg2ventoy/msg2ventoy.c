#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>

#include "detours.h"
#pragma comment(lib, "detours.lib")


#define INSTALL 0
#define UPDATE  1

HWND hCombox;

#define IDC_COMBO1                      1001
#define IDC_BUTTON3                     1004
#define IDC_BUTTON4                     1005
#define IDC_COMMAND1                    1024

typedef int (WINAPI* PMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef void (* PCallerCallback)(int CurrentProgress);

#pragma data_seg("SHARED") 
PCallerCallback g_CallerCallback = NULL;
HHOOK hV2DHook = NULL;
#pragma data_seg()         
#pragma comment(linker, "/section:SHARED,RWS")

LRESULT CALLBACK V2DHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    //TODO: 获取并输出进度
    //TODO: 拦截MessageBoxW
    //DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, PartDialogProc); 输yes的确认框

    return CallNextHookEx(NULL, code, wParam, lParam);
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
        hV2DHook = SetWindowsHookExW(WH_CALLWNDPROC, V2DHookProc, GetModuleHandle(NULL), GetWindowThreadProcessId(hwnd, NULL));

        return 0;
    }
    return 1;
}

__declspec(dllexport) int RunVentoy2Disk(unsigned short n, unsigned short type, PCallerCallback CallerCallback)
{
    g_CallerCallback = CallerCallback;
    EnumWindows(EnumWindowsProc, MAKELPARAM(n, type));

    return 0;
}

__declspec(dllexport) void FinishVentoy2Disk()
{
    UnhookWindowsHookEx(hV2DHook);
}

void DefaultCallback(int CurrentProgress)
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

    HMODULE hmod = LoadLibraryA("msg2ventoy.exe");
    int (*r)(unsigned short, unsigned short, PCallerCallback) = (int(*)(unsigned short, unsigned short, PCallerCallback))GetProcAddress(hmod, "RunVentoy2Disk");
    
    return r(n, type, DefaultCallback);
}
