cmake_minimum_required(VERSION 3.16)
project(nspireutils C CXX ASM)

file(GLOB_RECURSE NSPIREUTILS_SOURCES
    src/*.cpp
    src/*.c
    src/*.S
)

add_library(nspireutils STATIC ${NSPIREUTILS_SOURCES})

# nspire-utils requires Ndless SDK headers
target_include_directories(nspireutils
    PUBLIC include
    PRIVATE ${Ndless_INCLUDE_DIRS}
)

target_compile_definitions(nspireutils PRIVATE _TINSPIRE)

target_compile_options(nspireutils PRIVATE
    -Wall -Wextra -Wpedantic -Werror
    -marm
    -march=armv5te
    -mtune=arm926ej-s
    -mfpu=auto
    -Os
    -ffunction-sections
    -fdata-sections
    -mno-unaligned-access
)

set_target_properties(nspireutils PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

