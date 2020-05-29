cmake_minimum_required(VERSION 3.13)
project(sdl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2     REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

add_library(sdl INTERFACE IMPORTED GLOBAL)
target_include_directories(sdl INTERFACE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(sdl INTERFACE
    ${SDL2_LDFLAGS}
    ${SDL2_TTF_LDFLAGS}
)