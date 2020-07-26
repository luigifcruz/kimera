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

set_property(GLOBAL PROPERTY GLOBAL_CMAKE_CXX_FLAGS "")
set_property(GLOBAL PROPERTY GLOBAL_DEPENDENCIES "")
set_property(GLOBAL PROPERTY GLOBAL_SOURCE_LIST "")
set_property(GLOBAL PROPERTY GLOBAL_LIB_LIST "")

SUBDIRLIST(SUBDIRS "${PROJECT_SOURCE_DIR}/src")

foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
endforeach()

get_property(LIB_LIST GLOBAL PROPERTY GLOBAL_LIB_LIST)
get_property(SOURCE_LIST GLOBAL PROPERTY GLOBAL_SOURCE_LIST)
get_property(DEPENDENCIES GLOBAL PROPERTY GLOBAL_DEPENDENCIES)
get_property(CMAKE_CXX_FLAGS GLOBAL PROPERTY GLOBAL_CMAKE_CXX_FLAGS)

add_library(kimera SHARED ${SOURCE_LIST})
target_link_libraries(kimera ${LIB_LIST})
target_include_directories(kimera PUBLIC ../include)

if(DEPENDENCIES)
add_dependencies(kimera ${DEPENDENCIES})
endif()
