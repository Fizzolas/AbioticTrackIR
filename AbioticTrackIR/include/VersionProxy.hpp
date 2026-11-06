#pragma once
#include <windows.h>

class VersionProxy
{
public:
    static bool Initialize();
    static void Shutdown();

    // Exported function pointers
    static FARPROC pGetFileVersionInfoA;
    static FARPROC pGetFileVersionInfoByHandle;
    static FARPROC pGetFileVersionInfoExA;
    static FARPROC pGetFileVersionInfoExW;
    static FARPROC pGetFileVersionInfoSizeA;
    static FARPROC pGetFileVersionInfoSizeExA;
    static FARPROC pGetFileVersionInfoSizeExW;
    static FARPROC pGetFileVersionInfoSizeW;
    static FARPROC pGetFileVersionInfoW;
    static FARPROC pVerFindFileA;
    static FARPROC pVerFindFileW;
    static FARPROC pVerInstallFileA;
    static FARPROC pVerInstallFileW;
    static FARPROC pVerLanguageNameA;
    static FARPROC pVerLanguageNameW;
    static FARPROC pVerQueryValueA;
    static FARPROC pVerQueryValueW;

private:
    static HMODULE s_hOriginalDll;
};
