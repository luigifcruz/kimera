if(UNIX AND NOT APPLE)
    project(dl)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(dl REQUIRED IMPORTED_TARGET dl)

    add_library(dl INTERFACE IMPORTED GLOBAL)
    target_include_directories(dl INTERFACE ${dl_INCLUDE_DIRS})
    target_link_libraries(dl INTERFACE ${dl_LDFLAGS})
endif()