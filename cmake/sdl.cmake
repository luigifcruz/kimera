project(sdl)

add_library(sdl INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2     REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
endif()

if(WINDOWS)
    message(SEND_ERROR "SDL isn't supported on Windows yet.")
endif()

target_include_directories(sdl INTERFACE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(sdl INTERFACE
    ${SDL2_LDFLAGS}
    ${SDL2_TTF_LDFLAGS}
)

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST sdl)