cmake_minimum_required(VERSION 3.22)
project(RuaDeAcoNative VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(RuaDeAcoRuntime STATIC
    Source/RuaDeAcoRuntime/Private/RDAFighterCatalog.cpp
    Source/RuaDeAcoRuntime/Private/RDAFighter.cpp
    Source/RuaDeAcoRuntime/Private/RDACombatWorld.cpp
)
target_include_directories(RuaDeAcoRuntime PUBLIC
    Source/RuaDeAcoRuntime/Public
)

set(RAYLIB_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/include")
set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/raylib/lib")

if(ANDROID)
    include(FetchContent)
    set(PLATFORM Android CACHE STRING "raylib platform" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        URL_HASH SHA256=00e8a7c85f7a5221e56ee8e4e1c6642b773eae73d29379a91c8ac5149e803f42)
    FetchContent_MakeAvailable(raylib)
    add_library(ruadeaco SHARED Source/Prototype/PrototypeMain.cpp)
    target_compile_definitions(ruadeaco PRIVATE PLATFORM_ANDROID GRAPHICS_API_OPENGL_ES2)
    target_include_directories(ruadeaco PRIVATE "${ANDROID_NDK}/sources/android/native_app_glue")
    target_link_libraries(ruadeaco PRIVATE RuaDeAcoRuntime raylib)
    target_link_options(ruadeaco PRIVATE "-uANativeActivity_onCreate" "-Wl,-z,max-page-size=16384" "-Wl,--no-undefined")
elseif(WIN32 AND EXISTS "${RAYLIB_INC_DIR}/raylib.h" AND EXISTS "${RAYLIB_LIB_DIR}/raylibdll.lib")
    add_executable(RuaDeAcoUnreal
        Source/Prototype/PrototypeMain.cpp
    )
    target_compile_definitions(RuaDeAcoUnreal PRIVATE NOMINMAX PLATFORM_DESKTOP GRAPHICS_API_OPENGL_33 USE_LIBTYPE_SHARED)
    target_include_directories(RuaDeAcoUnreal PRIVATE "${RAYLIB_INC_DIR}")
    target_link_libraries(RuaDeAcoUnreal PRIVATE
        RuaDeAcoRuntime
        "${RAYLIB_LIB_DIR}/raylibdll.lib"
        winmm gdi32 opengl32 user32 shell32 advapi32
    )
    add_custom_command(TARGET RuaDeAcoUnreal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_LIB_DIR}/raylib.dll"
            "$<TARGET_FILE_DIR:RuaDeAcoUnreal>/raylib.dll"
    )
endif()

if(NOT ANDROID)
enable_testing()
add_executable(RuaDeAcoTests
    Tests/RDAGoldenTests.cpp
)
target_link_libraries(RuaDeAcoTests PRIVATE RuaDeAcoRuntime)
add_test(NAME RDAGoldenTests COMMAND RuaDeAcoTests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME RDACanonicalReplayParity COMMAND RuaDeAcoTests --parity WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
if(TARGET RuaDeAcoUnreal)
    add_test(NAME RDAHeadlessSmoke COMMAND RuaDeAcoUnreal --headless-smoke WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
endif()
