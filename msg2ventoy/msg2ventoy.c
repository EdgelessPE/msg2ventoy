#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>


#define INSTALL 0
#define UPDATE  1

HWND hCombox;

#define IDC_COMBO1                      1001
#define IDC_BUTTON3                     1004
#define IDC_BUTTON4                     1005
#define IDC_COMMAND1                    1024

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
        ShowWindow(hwnd, SW_MINIMIZE);
        //system("pause");
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMMAND1, BN_CLICKED), 0);

        SendMessageW(hCombox, CB_SETCURSEL, n, 0);
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO1, CBN_SELCHANGE), (LPARAM)hCombox);

        puts("Select disk ok");

        //system("pause");

        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(type ? IDC_BUTTON3 : IDC_BUTTON4, BN_CLICKED), 0);
        return 0;
    }
    return 1;
}

__declspec(dllexport) int RunVentoy2Disk(unsigned short n, unsigned short type)
{
    EnumWindows(EnumWindowsProc, MAKELPARAM(n, type));

    return 0;
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
    int (*r)(unsigned short, unsigned short) = (int(*)(unsigned short, unsigned short))GetProcAddress(hmod, "RunVentoy2Disk");
    
    return r(n, type);
}
