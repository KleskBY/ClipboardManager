#pragma once
#include "main.h"
#include <string>
#include <locale>
#include <codecvt>
#include <shlobj.h>

namespace Clipboard
{
    static std::string GetClipboardFormat()
    {
        std::string format = "";
        if (OpenClipboard(NULL))
        {
            UINT uFormat = EnumClipboardFormats(0); 

            while (uFormat)
            {
                if (uFormat == CF_TEXT) format = format + "CF_TEXT ";
                else if (uFormat == CF_BITMAP) format = format + "CF_BITMAP ";
                else if (uFormat == CF_METAFILEPICT) format = format + "CF_METAFILEPICT ";
                else if (uFormat == CF_SYLK) format = format + "CF_SYLK ";
                else if (uFormat == CF_DIF) format = format + "CF_DIF ";
                else if (uFormat == CF_TIFF) format = format + "CF_TIFF ";
                else if (uFormat == CF_OEMTEXT) format = format + "CF_OEMTEXT ";
                else if (uFormat == CF_DIB) format = format + "CF_DIB ";
                else if (uFormat == CF_PALETTE) format = format + "CF_PALETTE ";
                else if (uFormat == CF_PENDATA) format = format + "CF_PENDATA ";
                else if (uFormat == CF_RIFF) format = format + "CF_RIFF ";
                else if (uFormat == CF_WAVE) format = format + "CF_WAVE ";
                else if (uFormat == CF_UNICODETEXT) format = format + "CF_UNICODETEXT ";
                else if (uFormat == CF_ENHMETAFILE) format = format + "CF_ENHMETAFILE ";
                else if (uFormat == CF_HDROP) format = format + "CF_HDROP ";
                else if (uFormat == CF_LOCALE) format = format + "CF_LOCALE ";
                else if (uFormat == CF_DIBV5) format = format + "CF_DIBV5 ";
                else if (uFormat == CF_MAX) format = format + "CF_MAX ";
                uFormat = EnumClipboardFormats(uFormat);
            }
            CloseClipboard();
        }
        return format;
    }

    static std::string GetClipboardToString()
	{
        std::string clipboard = "";
        if (OpenClipboard(NULL))
        {
            HANDLE h = GetClipboardData(CF_TEXT);
            if (h && h != INVALID_HANDLE_VALUE) clipboard = (char*)h;
            CloseClipboard();
        }
        return clipboard;
	}

    static void SetClipboardToString(std::string s)
    {
        OpenClipboard(hwnd);
        EmptyClipboard();
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
        if (!hg)
        {
            CloseClipboard();
            return;
        }
        memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
        GlobalUnlock(hg);
        SetClipboardData(CF_TEXT, hg);
        CloseClipboard();
        GlobalFree(hg);
    }
    
    static void SetClipboardUnciodeText(const wchar_t* Text)
    {
        if (OpenClipboard(hwnd))
        {
            EmptyClipboard();
            wchar_t* wcBuffer = 0;
            HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE,(2 * wcslen(Text) + 2) * sizeof(wchar_t));
            if (!hglbCopy)
            {
                CloseClipboard();
                return;
            }
            wcBuffer = (wchar_t*)GlobalLock(hglbCopy);
            wcscpy(wcBuffer, Text);

            GlobalUnlock(hglbCopy);
            SetClipboardData(CF_UNICODETEXT, hglbCopy);
            CloseClipboard();
            GlobalFree(hglbCopy);
        }
    }

    static void PasteText(std::string s)
    {
        std::string backup = GetClipboardToString();

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(s);         //std::string narrow = converter.to_bytes(wide_utf16_source_string);
        SetClipboardUnciodeText(wide.c_str());

        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = 0;
        SendInput(1, &ip, sizeof(INPUT));
        ip.ki.wVk = 0x56;
        ip.ki.dwFlags = 0;
        SendInput(1, &ip, sizeof(INPUT));

        Sleep(50);

        ip.ki.wVk = 0x56;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        SetClipboardToString(backup);
    }
}

namespace Monitor
{
    static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
    {
        static unsigned long long _previousTotalTicks = 0;
        static unsigned long long _previousIdleTicks = 0;

        unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
        unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

        float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

        _previousTotalTicks = totalTicks;
        _previousIdleTicks = idleTicks;
        return ret;
    }

    static unsigned long long FileTimeToInt64(const FILETIME& ft) { return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime); }
    float GetCPULoad()
    {
        FILETIME idleTime, kernelTime, userTime;
        return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
    }

    static DWORD GetRAM()
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        return statex.dwMemoryLoad;
    }

}

bool CreateLnk(const char* pathToProgarm, const char* Directory, const char* Description, const wchar_t* path)
{
    CoInitialize(NULL);
    IShellLink* pShellLink = NULL;
    HRESULT hres;
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLink, (void**)&pShellLink);

    if (SUCCEEDED(hres))
    {
        pShellLink->SetPath(pathToProgarm);  // Path to the object we are referring to
        pShellLink->SetDescription(Description);
        pShellLink->SetIconLocation(pathToProgarm, 0);
        pShellLink->SetRelativePath(Directory,0);

        IPersistFile* pPersistFile;
        hres = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

        if (SUCCEEDED(hres))
        {
            hres = pPersistFile->Save(path, TRUE);
            pPersistFile->Release();
            return true;
        }
        else return false;
        pShellLink->Release();
    }
    return false;
}

namespace Regedit
{
    std::string RegRead(HKEY hKey, LPCTSTR subkey, LPCTSTR name, DWORD type)
    {
        HKEY key;
        TCHAR value[255];
        DWORD value_length = 255;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_READ | KEY_WOW64_64KEY, &key);
        if (RegQueryValueEx(key, name, NULL, &type, (LPBYTE)&value, &value_length) != ERROR_SUCCESS) return "ERROR";
        RegCloseKey(key);
        return value;
    }
    DWORD RegReadInt(HKEY hKey, LPCTSTR subkey, LPCTSTR name, DWORD type)
    {
        HKEY key;
        DWORDLONG value;
        unsigned long size = 1024;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_READ | KEY_WOW64_64KEY, &key);
        RegQueryValueEx(key, name, NULL, &type, (LPBYTE)&value, &size);
        RegCloseKey(key);
        return value;
    }
    void RegWrite(HKEY hKey, LPCTSTR subkey, LPCTSTR name, DWORD type, const char* value)
    {
        HKEY key;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_WRITE | KEY_WOW64_64KEY, &key);
        RegSetValueEx(key, name, 0, type, (LPBYTE)value, strlen(value) * sizeof(char));
        RegCloseKey(key);
    }
    void RegWriteInt(HKEY hKey, LPCTSTR subkey, LPCTSTR name, DWORD type, DWORD value)
    {
        HKEY key;
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, NULL, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &key);
        RegSetValueEx(key, name, 0, type, (const BYTE*)&value, sizeof(value));
        RegCloseKey(key);
    }
    void RegWriteQword(HKEY hKey, LPCTSTR subkey, LPCTSTR name, DWORD type, const BYTE* value)
    {
        HKEY key;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &key);
        RegSetValueEx(key, name, 0, type, value, sizeof(value));
        RegCloseKey(key);
    }
    static bool RegExists(HKEY hKey, LPCTSTR subkey, LPCTSTR name)
    {
        bool ret = false;
        HKEY key;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_WRITE | KEY_WOW64_64KEY, &key);
        if (RegQueryValueEx(key, name, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            ret = true;
            RegCloseKey(key);
        }
        return ret;
    }
    static void RegDeleteSubkey(HKEY hKey, LPCTSTR subkey, LPCTSTR name)
    {
        HKEY key;
        RegOpenKeyEx(hKey, subkey, NULL, KEY_WRITE | KEY_WOW64_64KEY, &key);
        RegDeleteValueA(key, name);
        RegCloseKey(key);
    }

}

namespace FileOperations
{
    bool CopyTo(const char* dist, const char* path)
    {
        if (GetFileAttributes(dist) != INVALID_FILE_ATTRIBUTES) return true;
        else if (CopyFile(path, dist, FALSE)) return true;
        return false;
    }
}

static void EnableAutostart(const char* pathToExecutable = "", const char* ExecutableNane = "")
{
    std::string RegeditExePath = pathToExecutable;
    std::string ExeName = ExecutableNane;
    if (!RegeditExePath.size() || !ExeName.size())
    {
        char CurrentPathBuffer[MAX_PATH];
        GetModuleFileName(NULL, CurrentPathBuffer, MAX_PATH);
        std::string ExePath = std::string(CurrentPathBuffer);
        std::string::size_type pos = ExePath.find_last_of("\\/") + 1;
        ExeName = std::string(CurrentPathBuffer).substr(pos, strlen(CurrentPathBuffer));
        RegeditExePath = ExePath;
        //RegeditExePath = "\"" + ExePath + "\"";
    }

    Regedit::RegWrite(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", ExeName.c_str(), REG_SZ, RegeditExePath.c_str());
    Regedit::RegWrite(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", ExeName.c_str(), REG_SZ, RegeditExePath.c_str());
    Regedit::RegWrite(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", ExeName.c_str(), REG_SZ, RegeditExePath.c_str());
    Regedit::RegWrite(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", ExeName.c_str(), REG_SZ, RegeditExePath.c_str());

    
}

static void DisableAutostart(const char* ExecutableNane = "")
{
    std::string ExeName = ExecutableNane;
    if (!ExeName.size())
    {
        char CurrentPathBuffer[MAX_PATH];
        GetModuleFileName(NULL, CurrentPathBuffer, MAX_PATH);
        std::string ExePath = std::string(CurrentPathBuffer);
        std::string::size_type pos = ExePath.find_last_of("\\/") + 1;
        ExeName = std::string(CurrentPathBuffer).substr(pos, strlen(CurrentPathBuffer));
    }

    Regedit::RegDeleteSubkey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", ExeName.c_str());
    Regedit::RegDeleteSubkey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", ExeName.c_str());
    Regedit::RegDeleteSubkey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", ExeName.c_str());
    Regedit::RegDeleteSubkey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", ExeName.c_str());
}

namespace Encryption
{
    char* Xor(char* Array, char key)
    {
        char* ret;
        for (int i = 0; i < strlen(Array); i++) ret[i] = Array[i] ^ key;
        return ret;
    }
}