// zip_utils.h
#pragma once
#include <string>

// يضغط مجلد بأكمله إلى ملف ZIP
// folderPath: المجلد المراد ضغطه (مثل "C:\\temp\\collected_data")
// zipFilePath: المسار الكامل لملف ZIP الناتج (مثل "C:\\temp\\output.zip")
bool ZipFolder(const std::string& folderPath, const std::string& zipFilePath);