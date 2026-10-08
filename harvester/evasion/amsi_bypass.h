// amsi_bypass.h
#pragma once
#include <windows.h>

// يعطل AMSI في العملية الحالية عن طريق تصحيح AmsiScanBuffer
bool BypassAMSI();

// استعادة AMSI (اختياري)
bool RestoreAMSI();