project(xcb)

add_library(xcb INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)
endif()

target_include_directories(xcb INTERFACE ${XCB_INCLUDE_DIRS})
target_link_libraries(xcb INTERFACE ${XCB_LDFLAGS})