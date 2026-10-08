cmake_minimum_required(VERSION 3.15)
project(TelegramHarvesterBuilder VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# للإصدار النهائي، نفضل静态 linkage لتقليل الاعتماد على DLLs خارجية
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /GL /Gy")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -static -static-libgcc -static-libstdc++")
endif()

# الملف الرئيسي الذي سنبنيه في المرحلة 0 (اختبار بسيط)
add_executable(harvester_stub harvester_stub.cpp)

# في المراحل القادمة سنضيف الملفات الحقيقية
# target_sources(telegram_harvester PRIVATE
#     harvester/collectors/telegram_collector.cpp
#     ...
# )