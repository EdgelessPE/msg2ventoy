# msg2ventoy

让 ventoy2disk 支持从命令行调用。

原理是 SendMessage 与 inline hook (detours) 。

___目前 msg2ventoy.exe 是可用的，hook.dll 不可用，因此运行 msg2ventoy.exe 时务必不要让 hook.dll 出现在 dll 加载路径 或 msg2ventoy.exe 同目录下！___

## msg2ventoy.exe
两种使用方法：
1. 直接运行按照提示输入参数
2. 链接到 msg2ventoy.lib，通过 msg2ventoy.h 将 msg2ventoy.exe 作为动态链接库（DLL）使用


参数中修改分区格式为 GPT 这一功能有未知原因的 bug，建议使用时直接通过修改 Ventoy2Disk.ini 来切换分区格式。
