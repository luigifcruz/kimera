cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES C)

if(UNIX AND NOT APPLE)
    message(STATUS "Assuming UNIX system...")
    set(LINUX TRUE)
endif()

if(WIN32)
    message(STATUS "Assuming Windows system...")
    set(WINDOWS TRUE)
endif()

if(APPLE)
    message(STATUS "Assuming Apple system...")
    set(MACOS TRUE)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    if(CMAKE_COMPILER_IS_GNUCXX)
        message(STATUS "Compiler is GCC, adding compile flags...")
        set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
        set(CMAKE_C_FLAGS_DEBUG "-g")
        set(CMAKE_C_FLAGS_RELEASE "-O3")
    endif(CMAKE_COMPILER_IS_GNUCXX)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

include(cmake/ffmpeg.cmake)
include(cmake/yaml.cmake)
include(cmake/sdl.cmake)
include(cmake/xcb.cmake)

include(cmake/listdirs.cmake)

add_subdirectory(src)
add_subdirectory(apps)