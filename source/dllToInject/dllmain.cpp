// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include <iostream>
#include <wchar.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable:4996)
#define JMP(from, to) (int)(((int)to - (int)from) - 5);

void Jump(unsigned long ulAddress, void* Function, unsigned long ulNops) {
    try {
        DWORD OldProtection;
        VirtualProtect((LPVOID)ulAddress, 0x4, PAGE_EXECUTE_READWRITE, &OldProtection);

        *(unsigned char*)ulAddress = 0xE9;
        *(unsigned long*)(ulAddress + 1) = JMP(ulAddress, Function);
        memset((void*)(ulAddress + 5), 0x90, ulNops);

        VirtualProtect((LPVOID)ulAddress, 0x4, OldProtection, &OldProtection);
    }
    catch (...) {}
}

void killProcessByName(const wchar_t* filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    if (!hSnapShot) return;

    PROCESSENTRY32W pEntry;
    pEntry.dwSize = sizeof(pEntry);

    BOOL hRes = Process32FirstW(hSnapShot, &pEntry);
    while (hRes)
    {
        if (StrStrIW(pEntry.szExeFile, filename))
        {
          
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, pEntry.th32ProcessID);
            if (hProcess)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32NextW(hSnapShot, &pEntry);
    }

    CloseHandle(hSnapShot);
}


// char __cdecl ProcessHelper::checkProcessRunning(LPCWSTR lpString2)
size_t __cdecl hook_checkProcessRunning(LPCWSTR lpString2) {

    if (!lstrcmpiW(L"gxxapphelper.exe", lpString2)) {
        // check lol agent?
        killProcessByName(L"league"); // league of lengend blablabla
        Sleep(300);
    }


    HANDLE Toolhelp32Snapshot; // esi
    char v3; // [esp+Bh] [ebp-231h]
    PROCESSENTRY32W pe; // [esp+Ch] [ebp-230h] BYREF

    Toolhelp32Snapshot = CreateToolhelp32Snapshot(2u, 0);
    if (Toolhelp32Snapshot == (HANDLE)-1)
        return 0;
    memset(&pe, 0, sizeof(pe));
    pe.dwSize = 556;
    if (!Process32FirstW(Toolhelp32Snapshot, &pe))
    {
        CloseHandle(Toolhelp32Snapshot);
        return 0;
    }
    v3 = 0;
    while (lstrcmpiW(pe.szExeFile, lpString2))
    {
        if (!Process32NextW(Toolhelp32Snapshot, &pe))
            goto LABEL_10;
    }
    v3 = 1;
LABEL_10:
    CloseHandle(Toolhelp32Snapshot);
    return v3;
}



DWORD WINAPI callback(LPVOID lpParameter) {

    size_t targetFunc = 0;
    while (!targetFunc) {
        targetFunc = (size_t)GetProcAddress(LoadLibraryA("commutil.dll"), "?checkProcessRunning@ProcessHelper@@YA_NPB_W@Z");
        Sleep(300);
    }
    orginalPtr_checkProcessRunning = targetFunc + 9;
    Jump(targetFunc, hook_checkProcessRunning, 0);
    Beep(500, 500);

    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0xFF, callback, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

