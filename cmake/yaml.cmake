project(yaml)

add_library(yaml INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-cpp)
endif()

if(WINDOWS)
    file(GLOB YAML_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/*yaml*/include")
    file(GLOB YAML_LDFLAGS "${CMAKE_SOURCE_DIR}/deps/*yaml*/out/build/x64-Release/*.lib")
endif()

target_include_directories(yaml INTERFACE ${YAML_INCLUDE_DIRS})
target_link_libraries(yaml INTERFACE ${YAML_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST yaml)
