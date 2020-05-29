cmake_minimum_required(VERSION 3.10)
project(xcb)

find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)

add_library(xcb INTERFACE IMPORTED GLOBAL)
target_include_directories(xcb INTERFACE
    ${XCB_INCLUDE_DIRS}
)

target_link_libraries(xcb INTERFACE
    ${XCB_LDFLAGS}
)