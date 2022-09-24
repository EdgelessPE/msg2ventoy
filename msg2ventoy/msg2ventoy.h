#pragma once

#define INSTALL 0
#define UPDATE  1

typedef int(*PCallerCallback)(unsigned int CurrentProgress);
typedef void(*PM2VProgressCallback)(M2VProgress CurrentProgress);
typedef enum _M2VProgress
{
    STATUS_FINDV2DWND,
    STATUS_FRESH,
    STATUS_SELECTDISK,
    STATUS_START,

    WARNING_GetDlgItem
}M2VProgress;

int RunVentoy2Disk(unsigned short n, unsigned short type, PCallerCallback CallerCallback, PM2VProgressCallback M2VProgressCallback);
