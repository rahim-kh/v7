// harvester/system/system_info.h
// واجهة لجمع معلومات النظام والبرامج المثبتة
#pragma once

#include <string>
#include <vector>

// بنية لتخزين معلومات النظام
struct SystemInfo {
    std::string ipAddress;           // عنوان IP العام
    std::string computerName;        // اسم الكمبيوتر
    std::string userName;            // اسم المستخدم
    std::string osVersion;           // نظام التشغيل (مثل Windows 11 Pro)
    std::string architecture;        // x86, x64, ARM
    std::string ram;                 // حجم الذاكرة (GB)
    std::string cpu;                 // اسم المعالج
    std::string gpu;                 // اسم بطاقة الرسومات (اختياري)
    std::string language;            // اللغة (مثل en-US)
    std::string timezone;            // المنطقة الزمنية
    std::vector<std::string> installedSoftware; // قائمة البرامج المثبتة
};

// يجمع كل معلومات النظام (بما في ذلك IP العام)
SystemInfo CollectSystemInfo();

// يخزن معلومات النظام في ملف نصي بتنسيق readable (system_info.txt)
void SaveSystemInfoToFile(const SystemInfo& info, const std::string& outputDir);

// يعيد قائمة البرامج المثبتة من الريجستري
std::vector<std::string> GetInstalledSoftware();

// يحصل على عنوان IP العام (عبر api.ipify.org أو icanhazip.com)
std::string GetPublicIP();