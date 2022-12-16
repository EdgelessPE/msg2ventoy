#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>
#include <CommCtrl.h>

#include "msg2ventoy.h"

HWND hCombox;
HMODULE hDll;

#define IDC_COMBO1                      1001    //下拉选项
#define IDC_BUTTON3                     1004    //更新
#define IDC_BUTTON4                     1005    //安装
#define IDC_COMMAND1                    1024    //刷新
#define IDC_PROGRESS1                   1006    //进度条
#define ID_PARTSTYLE_MBR                40012
#define ID_PARTSTYLE_GPT                40013

typedef int(WINAPI*PMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
typedef BOOL(*PSetV2dHook)(DWORD V2dThreadId, LPTHREAD_START_ROUTINE ThreadProc);
typedef BOOL(*PDropV2dHook)();
typedef int(*PGetCurrentProgress)(HWND V2dhWnd, PCallerCallback Callback);



PSetV2dHook SetV2dHook;
PDropV2dHook DropV2dHook;
PGetCurrentProgress GetCurrentProgress;
PCallerCallback g_CallerCallback;
PM2VProgressCallback g_M2VProgressCallback;

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

        Sleep(50);
    }

    return 0;
}

BOOL CALLBACK EnumWindowsProc(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    PM2V_PARAMS params = (PM2V_PARAMS)lParam;

    WCHAR buffer[31] = { 0 };
    GetWindowTextW(hwnd, buffer, 30);
    if (wcsstr(buffer, L"Ventoy2Disk") != NULL)
    {
        g_M2VProgressCallback(STATUS_FINDV2DWND);
        hCombox = GetDlgItem(hwnd, IDC_COMBO1);
        if (hCombox == NULL)
        {
            g_M2VProgressCallback(WARNING_GetDlgItem);
            return 1;
        }
        //ShowWindow(hwnd, SW_MINIMIZE);
        
        //刷新
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMMAND1, BN_CLICKED), 0);
        g_M2VProgressCallback(STATUS_FRESH);

        Sleep(100);

        //选择磁盘
        SendMessageW(hCombox, CB_SETCURSEL, params->WhichDevice, 0);
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_COMBO1, CBN_SELCHANGE), (LPARAM)hCombox);
        g_M2VProgressCallback(STATUS_SELECTDISK);

        //安全启动
        if (!params->isSecureBoot)
        {
            SendMessageW(hwnd, WM_COMMAND, 0, 0);
            g_M2VProgressCallback(STATUS_SECUREBOOT);
        }

        //分区类型（选择gpt时有未知bug）
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(((params->PartitionStyle & 1) ? ID_PARTSTYLE_GPT : ID_PARTSTYLE_MBR), 0), 0);
        g_M2VProgressCallback(STATUS_PARTSTYLE);

        Sleep(100);

        //按下制作/更新按钮
        SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM((params->MakeType & 1) ? IDC_BUTTON3 : IDC_BUTTON4, BN_CLICKED), 0);
        g_M2VProgressCallback(STATUS_START);

        //反馈进度
        //hV2DHook = SetWindowsHookExW(WH_CALLWNDPROC, V2DHookProc, LoadLibraryA("msg2ventoy.dll"), GetWindowThreadProcessId(hwnd, NULL));
        //DWORD test = GetLastError();
        //printf("%ud", test);
        GetCurrentProgress(hwnd, g_CallerCallback);

        return 0;
    }
    return 1;
}

__declspec(dllexport) int RunVentoy2Disk(PM2V_PARAMS params, PCallerCallback CallerCallback, PM2VProgressCallback M2VProgressCallback)
{
    if (params->SizeOfM2V_PARAMS != sizeof(M2V_PARAMS))
    {
        M2VProgressCallback(ERROR_SizeOfParamsStruct);
        return 1;
    }

    g_CallerCallback = CallerCallback;
    g_M2VProgressCallback = M2VProgressCallback;

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

    EnumWindows(EnumWindowsProc, (LPARAM)params);


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
    return 0;
}

void DefaultM2VProgress(M2VProgress CurrentProgress)
{
    printf("m2v: %d\n", CurrentProgress);
}

int main()
{
    INT16 n;
    INT16 type;
    INT16 isSecureBoot = TRUE;
    INT16 ps = 0;

    puts("请输入盘符排序第n个（从0开始）");
    scanf("%hd", &n);
    puts("请输入进行的操作（0 = install，1 = update）");
    scanf("%hd", &type);
    puts("请输入是否启用安全启动（0 = 否，1 = 是）");
    scanf("%hd", &isSecureBoot);
    puts("请输入磁盘分区类型（0 = MBR，1 = GPT）");
    scanf("%hd", &ps);

    PM2V_PARAMS p = (PM2V_PARAMS)malloc(sizeof(M2V_PARAMS));
    p->SizeOfM2V_PARAMS = sizeof(M2V_PARAMS);
    p->WhichDevice = n;
    p->MakeType = type;
    p->isSecureBoot = isSecureBoot;
    p->PartitionStyle = ps | 0x10;

    int s = RunVentoy2Disk(p, DefaultCallback, DefaultM2VProgress);
    
    free(p);

    return s;
}
