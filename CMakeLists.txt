cmake_minimum_required(VERSION 3.24)

project(InkBlade LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(INKBLADE_USE_SFML "Build the SFML renderer instead of the dependency-free Win32 renderer" OFF)

if (INKBLADE_USE_SFML)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
    )
    FetchContent_MakeAvailable(SFML)

    add_executable(InkBlade src/main.cpp)
    target_compile_definitions(InkBlade PRIVATE SFML_STATIC)
    target_link_libraries(InkBlade PRIVATE sfml-graphics sfml-window sfml-system)
else()
    add_executable(InkBlade WIN32 src/main_win32.cpp)
    target_link_libraries(InkBlade PRIVATE gdi32 user32 windowscodecs ole32 msimg32)
endif()

if (MSVC)
    target_compile_options(InkBlade PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(InkBlade PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET InkBlade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:InkBlade>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:InkBlade>/assets
)
