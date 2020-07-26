cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

if(UNIX AND NOT APPLE)
    message(STATUS "Assuming Linux system...")
    set(LINUX TRUE)
    set(KIMERA_LINUX TRUE)
endif()

if(WIN32)
    message(STATUS "Assuming Windows system...")
    set(WINDOWS TRUE)
    set(KIMERA_WINDOWS TRUE)
endif()

if(APPLE)
    message(STATUS "Assuming Apple system...")
    set(MACOS TRUE)
    set(KIMERA_MACOS TRUE)
endif()

add_subdirectory(src)
add_subdirectory(python)
add_subdirectory(apps)
