#pragma once

#include <Windows.h>

typedef enum _M2VProgress
{
    STATUS_FINDV2DWND,
    STATUS_FRESH,
    STATUS_SELECTDISK,
    STATUS_SECUREBOOT,
    STATUS_PARTSTYLE,
    STATUS_START,

    WARNING_GetDlgItem,

    ERROR_SizeOfParamsStruct
}M2VProgress;
typedef int(*PCallerCallback)(unsigned int CurrentProgress);
typedef void(*PM2VProgressCallback)(M2VProgress CurrentProgress);

typedef enum _M2V_PARAMS_ENUM
{
    MAKE_INSTALL = 0b0000'0000,
    MAKE_UPDATE = 0b0000'0001,

    STYLE_MBR = 0b0001'0000,
    STYLE_GPT = 0b0001'0001
}M2V_PARAMS_ENUM;
#define INSTALL 0
#define UPDATE  1

typedef struct _M2V_PARAMS
{
    size_t SizeOfM2V_PARAMS;

    USHORT WhichDevice;
    BOOL isSecureBoot;
    M2V_PARAMS_ENUM MakeType;
    M2V_PARAMS_ENUM PartitionStyle;
}M2V_PARAMS, * PM2V_PARAMS;

//int RunVentoy2Disk(PM2V_PARAMS params, PCallerCallback CallerCallback, PM2VProgressCallback M2VProgressCallback);
