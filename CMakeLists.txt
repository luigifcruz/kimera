cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES C)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O3")

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

include(cmake/ffmpeg.cmake)
include(cmake/yaml.cmake)
include(cmake/sdl.cmake)
include(cmake/xcb.cmake)

include(cmake/listdirs.cmake)

add_subdirectory(src)
add_subdirectory(apps)