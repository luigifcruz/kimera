cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES CXX)

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

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(python)
add_subdirectory(apps)
