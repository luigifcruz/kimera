project(yaml2)

add_library(yaml INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-0.1)
endif()

if(WINDOWS)
    message(SEND_ERROR "YAML isn't supported on Windows yet.")
endif()

target_include_directories(yaml INTERFACE ${YAML_INCLUDE_DIRS})
target_link_libraries(yaml INTERFACE ${YAML_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST yaml)