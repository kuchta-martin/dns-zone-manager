cmake_minimum_required(VERSION 3.16)
project(dns-zone-manager VERSION 1.0 LANGUAGES CXX)

# ── Language standard ────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # -std=c++20, not -std=gnu++20

# ── Compiler warnings ────────────────────────────────────────────────────────
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)

# ── Main executable ──────────────────────────────────────────────────────────
add_executable(dns-manager
    src/main.cpp
    src/zone.cpp
)

target_include_directories(dns-manager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
