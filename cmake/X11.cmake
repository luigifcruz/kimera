if(UNIX AND NOT APPLE)
    project(X11)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

    add_library(X11 INTERFACE IMPORTED GLOBAL)
    target_include_directories(X11 INTERFACE ${X11_INCLUDE_DIRS})
    target_link_libraries(X11 INTERFACE ${X11_LDFLAGS})
endif()