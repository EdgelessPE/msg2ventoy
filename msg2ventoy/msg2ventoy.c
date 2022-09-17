#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>
#include <CommCtrl.h>


#define INSTALL 0
#define UPDATE  1

HWND hCombox;
HMODULE hDll;

#define IDC_COMBO1                      1001    //下拉选项
#define IDC_BUTTON3                     1004    //更新
#define IDC_BUTTON4                     1005    //安装
#define IDC_COMMAND1                    1024    //刷新
#define IDC_PROGRESS1                   1006    //进度条

typedef int(WINAPI*PMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef int(*PCallerCallback)(unsigned int CurrentProgress);
typedef BOOL(*PSetV2dHook)(DWORD V2dThreadId, LPTHREAD_START_ROUTINE ThreadProc);
typedef BOOL(*PDropV2dHook)();
typedef int(*PGetCurrentProgress)(HWND V2dhWnd, PCallerCallback Callback);

PSetV2dHook SetV2dHook;
PDropV2dHook DropV2dHook;
PGetCurrentProgress GetCurrentProgress;
PCallerCallback g_CallerCallback;

/*
typedef enum PROGRESS_POINT
{
    PT_START = 0,
    PT_LOCK_FOR_CLEAN = 8,
    PT_DEL_ALL_PART,
    PT_LOCK_FOR_WRITE,
    PT_FORMAT_PART1,
    PT_LOCK_VOLUME = PT_FORMAT_PART1,
    PT_FORMAT_PART2,

    PT_WRITE_VENTOY_START,
    PT_WRITE_VENTOY_FINISH = PT_WRITE_VENTOY_START + 32,

    PT_WRITE_STG1_IMG,
    PT_WRITE_PART_TABLE,
    PT_MOUNT_VOLUME,

    PT_FINISH
}PROGRESS_POINT;
*/

DWORD WINAPI ExeThreadProc(
    _In_ LPVOID lpParameter
)
{
    g_CallerCallback((UINT)lpParameter);
    return 1;
}

int DllGetCurrentProgress(HWND V2dhWnd, PCallerCallback Callback)
{
    //TODO: SetV2dHook
    //（dll顺便hook messagebox）
    //CreateRemoteThread通信

    SetV2dHook(GetWindowThreadProcessId(V2dhWnd, NULL), (LPTHREAD_START_ROUTINE)ExeThreadProc);

    return 0;
}

int ExeGetCurrentProgress(HWND V2dhWnd, PCallerCallback Callback)
{
    HWND hProgress = GetDlgItem(V2dhWnd, IDC_PROGRESS1);
    UINT CurrentProgress = 0;
    UINT tmp = 0;

    Callback(0); //PT_START
    while (1)
    {
        CurrentProgress = (UINT)SendMessageA(hProgress, PBM_GETPOS, 0, 0);
        if (CurrentProgress > tmp)
        {
            tmp = CurrentProgress;
            Callback(CurrentProgress);

            if (CurrentProgress == 49)  //PT_FINISH
            {
                break;
            }
        }

        Sleep(1);
    }

    return 0;
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
        GetCurrentProgress(hwnd, g_CallerCallback);

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

int DefaultCallback(unsigned int CurrentProgress)
{
    printf("%u\n", CurrentProgress);
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
