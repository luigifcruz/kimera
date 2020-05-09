cmake_minimum_required(VERSION 3.10)
project(yaml2)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)

add_library(yaml INTERFACE IMPORTED GLOBAL)
target_include_directories(yaml INTERFACE ${YAML_INCLUDE_DIRS})
target_link_libraries(yaml INTERFACE ${YAML_LDFLAGS})
