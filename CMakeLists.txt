cmake_minimum_required(VERSION 3.22)
project(RuaDeAcoNative VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

if(WIN32 AND EXISTS "${RAYLIB_INC_DIR}/raylib.h" AND EXISTS "${RAYLIB_LIB_DIR}/raylib.lib")
    add_executable(RuaDeAcoUnreal
        Source/Prototype/PrototypeMain.cpp
    )
    target_compile_definitions(RuaDeAcoUnreal PRIVATE NOMINMAX PLATFORM_DESKTOP GRAPHICS_API_OPENGL_33)
    target_include_directories(RuaDeAcoUnreal PRIVATE "${RAYLIB_INC_DIR}")
    target_link_libraries(RuaDeAcoUnreal PRIVATE
        RuaDeAcoRuntime
        "${RAYLIB_LIB_DIR}/raylib.lib"
        winmm gdi32 opengl32 user32 shell32 advapi32
    )
    add_custom_command(TARGET RuaDeAcoUnreal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_LIB_DIR}/raylib.dll"
            "$<TARGET_FILE_DIR:RuaDeAcoUnreal>/raylib.dll"
    )
endif()

enable_testing()
add_executable(RuaDeAcoTests
    Tests/RDAGoldenTests.cpp
)
target_link_libraries(RuaDeAcoTests PRIVATE RuaDeAcoRuntime)
add_test(NAME RDAGoldenTests COMMAND RuaDeAcoTests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
if(TARGET RuaDeAcoUnreal)
    add_test(NAME RDAHeadlessSmoke COMMAND RuaDeAcoUnreal --headless-smoke WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
