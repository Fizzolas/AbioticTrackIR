#include "VersionProxy.hpp"
#include <string>

HMODULE VersionProxy::s_hOriginalDll = NULL;
FARPROC VersionProxy::pGetFileVersionInfoA = NULL;
FARPROC VersionProxy::pGetFileVersionInfoByHandle = NULL;
FARPROC VersionProxy::pGetFileVersionInfoExA = NULL;
FARPROC VersionProxy::pGetFileVersionInfoExW = NULL;
FARPROC VersionProxy::pGetFileVersionInfoSizeA = NULL;
FARPROC VersionProxy::pGetFileVersionInfoSizeExA = NULL;
FARPROC VersionProxy::pGetFileVersionInfoSizeExW = NULL;
FARPROC VersionProxy::pGetFileVersionInfoSizeW = NULL;
FARPROC VersionProxy::pGetFileVersionInfoW = NULL;
FARPROC VersionProxy::pVerFindFileA = NULL;
FARPROC VersionProxy::pVerFindFileW = NULL;
FARPROC VersionProxy::pVerInstallFileA = NULL;
FARPROC VersionProxy::pVerInstallFileW = NULL;
FARPROC VersionProxy::pVerLanguageNameA = NULL;
FARPROC VersionProxy::pVerLanguageNameW = NULL;
FARPROC VersionProxy::pVerQueryValueA = NULL;
FARPROC VersionProxy::pVerQueryValueW = NULL;

bool VersionProxy::Initialize()
{
    if (s_hOriginalDll) return true;

    char systemDir[MAX_PATH];
    GetSystemDirectoryA(systemDir, MAX_PATH);
    std::string realDllPath = std::string(systemDir) + "\\version.dll";

    s_hOriginalDll = LoadLibraryA(realDllPath.c_str());
    if (!s_hOriginalDll) return false;

    pGetFileVersionInfoA         = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoA");
    pGetFileVersionInfoByHandle = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoByHandle");
    pGetFileVersionInfoExA       = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoExA");
    pGetFileVersionInfoExW       = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoExW");
    pGetFileVersionInfoSizeA     = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoSizeA");
    pGetFileVersionInfoSizeExA   = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoSizeExA");
    pGetFileVersionInfoSizeExW   = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoSizeExW");
    pGetFileVersionInfoSizeW     = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoSizeW");
    pGetFileVersionInfoW         = GetProcAddress(s_hOriginalDll, "GetFileVersionInfoW");
    pVerFindFileA                = GetProcAddress(s_hOriginalDll, "VerFindFileA");
    pVerFindFileW                = GetProcAddress(s_hOriginalDll, "VerFindFileW");
    pVerInstallFileA             = GetProcAddress(s_hOriginalDll, "VerInstallFileA");
    pVerInstallFileW             = GetProcAddress(s_hOriginalDll, "VerInstallFileW");
    pVerLanguageNameA            = GetProcAddress(s_hOriginalDll, "VerLanguageNameA");
    pVerLanguageNameW            = GetProcAddress(s_hOriginalDll, "VerLanguageNameW");
    pVerQueryValueA              = GetProcAddress(s_hOriginalDll, "VerQueryValueA");
    pVerQueryValueW              = GetProcAddress(s_hOriginalDll, "VerQueryValueW");

    return true;
}

void VersionProxy::Shutdown()
{
    if (s_hOriginalDll)
    {
        FreeLibrary(s_hOriginalDll);
        s_hOriginalDll = NULL;
    }
}

// Exported forwarders
extern "C"
{
    __declspec(dllexport) BOOL WINAPI GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
    {
        typedef BOOL (WINAPI* Fn)(LPCSTR, DWORD, DWORD, LPVOID);
        if (VersionProxy::pGetFileVersionInfoA) return ((Fn)VersionProxy::pGetFileVersionInfoA)(lptstrFilename, dwHandle, dwLen, lpData);
        return FALSE;
    }

    __declspec(dllexport) BOOL WINAPI GetFileVersionInfoByHandle(DWORD dwFlags, HANDLE hFile, LPVOID lpData, DWORD* pdwLen)
    {
        typedef BOOL (WINAPI* Fn)(DWORD, HANDLE, LPVOID, DWORD*);
        if (VersionProxy::pGetFileVersionInfoByHandle) return ((Fn)VersionProxy::pGetFileVersionInfoByHandle)(dwFlags, hFile, lpData, pdwLen);
        return FALSE;
    }

    __declspec(dllexport) BOOL WINAPI GetFileVersionInfoExA(DWORD dwFlags, LPCSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
    {
        typedef BOOL (WINAPI* Fn)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
        if (VersionProxy::pGetFileVersionInfoExA) return ((Fn)VersionProxy::pGetFileVersionInfoExA)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
        return FALSE;
    }

    __declspec(dllexport) BOOL WINAPI GetFileVersionInfoExW(DWORD dwFlags, LPCWSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
    {
        typedef BOOL (WINAPI* Fn)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
        if (VersionProxy::pGetFileVersionInfoExW) return ((Fn)VersionProxy::pGetFileVersionInfoExW)(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
        return FALSE;
    }

    __declspec(dllexport) DWORD WINAPI GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle)
    {
        typedef DWORD (WINAPI* Fn)(LPCSTR, LPDWORD);
        if (VersionProxy::pGetFileVersionInfoSizeA) return ((Fn)VersionProxy::pGetFileVersionInfoSizeA)(lptstrFilename, lpdwHandle);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI GetFileVersionInfoSizeExA(DWORD dwFlags, LPCSTR lpwstrFilename, LPDWORD lpdwHandle)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPCSTR, LPDWORD);
        if (VersionProxy::pGetFileVersionInfoSizeExA) return ((Fn)VersionProxy::pGetFileVersionInfoSizeExA)(dwFlags, lpwstrFilename, lpdwHandle);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI GetFileVersionInfoSizeExW(DWORD dwFlags, LPCWSTR lpwstrFilename, LPDWORD lpdwHandle)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPCWSTR, LPDWORD);
        if (VersionProxy::pGetFileVersionInfoSizeExW) return ((Fn)VersionProxy::pGetFileVersionInfoSizeExW)(dwFlags, lpwstrFilename, lpdwHandle);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR lptstrFilename, LPDWORD lpdwHandle)
    {
        typedef DWORD (WINAPI* Fn)(LPCWSTR, LPDWORD);
        if (VersionProxy::pGetFileVersionInfoSizeW) return ((Fn)VersionProxy::pGetFileVersionInfoSizeW)(lptstrFilename, lpdwHandle);
        return 0;
    }

    __declspec(dllexport) BOOL WINAPI GetFileVersionInfoW(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
    {
        typedef BOOL (WINAPI* Fn)(LPCWSTR, DWORD, DWORD, LPVOID);
        if (VersionProxy::pGetFileVersionInfoW) return ((Fn)VersionProxy::pGetFileVersionInfoW)(lptstrFilename, dwHandle, dwLen, lpData);
        return FALSE;
    }

    __declspec(dllexport) DWORD WINAPI VerFindFileA(DWORD uFlags, LPSTR szFileName, LPSTR szWinDir, LPSTR szAppDir, LPSTR szCurDir, PUINT lpuCurDirLen, LPSTR szDestDir, PUINT lpuDestDirLen)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPSTR, LPSTR, LPSTR, LPSTR, PUINT, LPSTR, PUINT);
        if (VersionProxy::pVerFindFileA) return ((Fn)VersionProxy::pVerFindFileA)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, lpuCurDirLen, szDestDir, lpuDestDirLen);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI VerFindFileW(DWORD uFlags, LPWSTR szFileName, LPWSTR szWinDir, LPWSTR szAppDir, LPWSTR szCurDir, PUINT lpuCurDirLen, LPWSTR szDestDir, PUINT lpuDestDirLen)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPWSTR, LPWSTR, LPWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
        if (VersionProxy::pVerFindFileW) return ((Fn)VersionProxy::pVerFindFileW)(uFlags, szFileName, szWinDir, szAppDir, szCurDir, lpuCurDirLen, szDestDir, lpuDestDirLen);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI VerInstallFileA(DWORD uFlags, LPSTR szSrcFileName, LPSTR szDestFileName, LPSTR szSrcDir, LPSTR szDestDir, LPSTR szCurDir, LPSTR szTmpFile, PUINT lpuTmpFileLen)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPSTR, LPSTR, LPSTR, LPSTR, LPSTR, LPSTR, PUINT);
        if (VersionProxy::pVerInstallFileA) return ((Fn)VersionProxy::pVerInstallFileA)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, lpuTmpFileLen);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI VerInstallFileW(DWORD uFlags, LPWSTR szSrcFileName, LPWSTR szDestFileName, LPWSTR szSrcDir, LPWSTR szDestDir, LPWSTR szCurDir, LPWSTR szTmpFile, PUINT lpuTmpFileLen)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPWSTR, LPWSTR, LPWSTR, LPWSTR, LPWSTR, LPWSTR, PUINT);
        if (VersionProxy::pVerInstallFileW) return ((Fn)VersionProxy::pVerInstallFileW)(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, lpuTmpFileLen);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI VerLanguageNameA(DWORD wLang, LPSTR szLang, DWORD nSize)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPSTR, DWORD);
        if (VersionProxy::pVerLanguageNameA) return ((Fn)VersionProxy::pVerLanguageNameA)(wLang, szLang, nSize);
        return 0;
    }

    __declspec(dllexport) DWORD WINAPI VerLanguageNameW(DWORD wLang, LPWSTR szLang, DWORD nSize)
    {
        typedef DWORD (WINAPI* Fn)(DWORD, LPWSTR, DWORD);
        if (VersionProxy::pVerLanguageNameW) return ((Fn)VersionProxy::pVerLanguageNameW)(wLang, szLang, nSize);
        return 0;
    }

    __declspec(dllexport) BOOL WINAPI VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen)
    {
        typedef BOOL (WINAPI* Fn)(LPCVOID, LPCSTR, LPVOID*, PUINT);
        if (VersionProxy::pVerQueryValueA) return ((Fn)VersionProxy::pVerQueryValueA)(pBlock, lpSubBlock, lplpBuffer, puLen);
        return FALSE;
    }

    __declspec(dllexport) BOOL WINAPI VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen)
    {
        typedef BOOL (WINAPI* Fn)(LPCVOID, LPCWSTR, LPVOID*, PUINT);
        if (VersionProxy::pVerQueryValueW) return ((Fn)VersionProxy::pVerQueryValueW)(pBlock, lpSubBlock, lplpBuffer, puLen);
        return FALSE;
    }
}
