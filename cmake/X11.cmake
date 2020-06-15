project(X11)

add_library(X11 INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
endif()

target_include_directories(X11 INTERFACE ${X11_INCLUDE_DIRS})
target_link_libraries(X11 INTERFACE ${X11_LDFLAGS})