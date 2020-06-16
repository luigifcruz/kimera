cmake_minimum_required(VERSION 3.13)
project(kimera VERSION 1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(UNIX AND NOT APPLE)
    message(STATUS "Assuming UNIX system...")
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

configure_file(
    "${PROJECT_SOURCE_DIR}/include/kimera/state.h.in"
    "${PROJECT_SOURCE_DIR}/include/kimera/state.h"
)

if(WINDOWS)
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
endif()

include(cmake/listdirs.cmake)

set_property(GLOBAL PROPERTY GLOBAL_CMAKE_C_FLAGS "")
set_property(GLOBAL PROPERTY GLOBAL_SOURCE_LIST "")
set_property(GLOBAL PROPERTY GLOBAL_LIB_LIST "")

add_subdirectory(src)

get_property(LIB_LIST GLOBAL PROPERTY GLOBAL_LIB_LIST)
get_property(SOURCE_LIST GLOBAL PROPERTY GLOBAL_SOURCE_LIST)
get_property(CMAKE_C_FLAGS GLOBAL PROPERTY GLOBAL_CMAKE_C_FLAGS)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT WINDOWS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-int-to-pointer-cast")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2 -g")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
    endif()
endif()

add_library(kimera ${SOURCE_LIST})
target_link_libraries(kimera ${LIB_LIST})
target_include_directories(kimera PUBLIC include)

add_subdirectory(apps)