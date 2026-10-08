// telegram_client.h
#pragma once
#include <string>

// إرسال ملف إلى بوت Telegram عبر واجهة sendDocument
// ترجع true إذا نجح الإرسال
bool SendToTelegram(const std::string& filePath, const std::string& botToken, const std::string& chatId);