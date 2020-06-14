project(egl)

add_library(egl INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(egl REQUIRED IMPORTED_TARGET egl)
endif()

if(WINDOWS)
endif()

if(MACOS)
endif()

target_include_directories(egl INTERFACE ${egl_INCLUDE_DIRS})
target_link_libraries(egl INTERFACE ${egl_LDFLAGS})