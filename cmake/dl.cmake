project(dl)

add_library(dl INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(dl REQUIRED IMPORTED_TARGET dl)
endif()

target_include_directories(dl INTERFACE ${dl_INCLUDE_DIRS})
target_link_libraries(dl INTERFACE ${dl_LDFLAGS})
