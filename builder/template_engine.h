// builder/template_engine.h
#pragma once
#include <string>
#include <unordered_map>

namespace TemplateEngine {
    // يستبدل كل {{KEY}} بالقيمة المقابلة في الخريطة
    std::string ReplaceVariables(const std::string& templateContent,
                                 const std::unordered_map<std::string, std::string>& variables);
    
    // يحول BuildSettings إلى خريطة مناسبة لقالب harvester
    std::unordered_map<std::string, std::string> SettingsToMap(const BuilderConfig::BuildSettings& settings);
}