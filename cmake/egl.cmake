project(egl)

add_library(egl INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(egl REQUIRED IMPORTED_TARGET egl)
endif()

if(WINDOWS)
    message(SEND_ERROR "EGL isn't supported on Windows yet.")
endif()

if(MACOS)
    file(GLOB egl_LDFLAGS "${CMAKE_SOURCE_DIR}/deps/angle/*.dylib")
endif()

target_include_directories(egl INTERFACE ${egl_INCLUDE_DIRS})
target_link_libraries(egl INTERFACE ${egl_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST egl)