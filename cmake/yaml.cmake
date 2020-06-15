project(yaml2)

add_library(yaml INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)
endif()

if(WINDOWS)
endif()

target_include_directories(yaml INTERFACE ${YAML_INCLUDE_DIRS})
target_link_libraries(yaml INTERFACE ${YAML_LDFLAGS})