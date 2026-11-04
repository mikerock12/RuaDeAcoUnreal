cmake_minimum_required(VERSION 3.22)
project(RuaDeAcoNative VERSION 0.4.0 LANGUAGES C CXX)

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
        URL_HASH SHA256=aea98ecf5bc5c5e0b789a76de0083a21a70457050ea4cc2aec7566935f5e258e)
    FetchContent_MakeAvailable(raylib)
    include(cmake/AndroidSurface.cmake)
    slt_android_buffer_geometry("${raylib_SOURCE_DIR}")
    add_library(ruadeaco SHARED Source/Prototype/PrototypeMain.cpp)
    target_compile_definitions(ruadeaco PRIVATE PLATFORM_ANDROID GRAPHICS_API_OPENGL_ES2)
    target_include_directories(ruadeaco PRIVATE "${ANDROID_NDK}/sources/android/native_app_glue")
    target_link_libraries(ruadeaco PRIVATE RuaDeAcoRuntime raylib)
    target_link_options(ruadeaco PRIVATE "-uANativeActivity_onCreate" "-Wl,-z,max-page-size=16384" "-Wl,--no-undefined")
elseif(WIN32 AND EXISTS "${RAYLIB_INC_DIR}/raylib.h" AND EXISTS "${RAYLIB_LIB_DIR}/raylibdll.lib")
    add_executable(RuaDeAcoUnreal WIN32
        Source/Prototype/PrototypeMain.cpp
    )
    include(FetchContent)
    FetchContent_Declare(webview2sdk URL https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/1.0.4314.35/microsoft.web.webview2.1.0.4314.35.nupkg
        URL_HASH SHA256=90778deb1b77e36292828b4b5833c9255c27533f40e0ddd55891ae718944edc4 DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(webview2sdk)
    target_sources(RuaDeAcoUnreal PRIVATE Platform/Windows/OnlineWindow.cpp)
    target_include_directories(RuaDeAcoUnreal PRIVATE "${webview2sdk_SOURCE_DIR}/build/native/include")
    target_link_libraries(RuaDeAcoUnreal PRIVATE "${webview2sdk_SOURCE_DIR}/build/native/x64/WebView2LoaderStatic.lib" ole32 oleaut32 shlwapi version)
    if(MSVC)
        target_link_options(RuaDeAcoUnreal PRIVATE /ENTRY:mainCRTStartup)
    endif()
    enable_language(RC)
    set(SLT_BRANDING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Platform/Windows/Branding")
    set(SLT_PRODUCT_NAME "Rua de Aço")
    set(SLT_VERSION "${PROJECT_VERSION}")
    string(REPLACE "." "," SLT_VERSION_COMMAS "${PROJECT_VERSION}.0")
    configure_file(Platform/Windows/GameResources.rc.in "${CMAKE_CURRENT_BINARY_DIR}/GameResources.rc" @ONLY)
    target_sources(RuaDeAcoUnreal PRIVATE Platform/Windows/WindowsBranding.cpp "${CMAKE_CURRENT_BINARY_DIR}/GameResources.rc")
    target_include_directories(RuaDeAcoUnreal PRIVATE Platform/Windows)
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
if(WIN32)
    add_executable(RuaDeAcoPreferencesTests Tests/PreferencesTests.cpp)
    target_include_directories(RuaDeAcoPreferencesTests PRIVATE Source/Prototype "${RAYLIB_INC_DIR}")
    target_link_libraries(RuaDeAcoPreferencesTests PRIVATE "${RAYLIB_LIB_DIR}/raylibdll.lib")
    add_test(NAME RuaDeAcoPreferencesTests COMMAND RuaDeAcoPreferencesTests)
endif()
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
