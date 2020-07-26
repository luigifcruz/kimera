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

configure_file(
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp.in"
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp"
)

add_library(kimera SHARED)
target_include_directories(kimera PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(codec)
add_subdirectory(transport)
add_subdirectory(loopback)
add_subdirectory(client)
