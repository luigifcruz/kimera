if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(YAML REQUIRED IMPORTED_TARGET yaml-cpp)
endif()

if(WINDOWS)
endif()

target_include_directories(kimera PUBLIC ${YAML_INCLUDE_DIRS})
target_link_libraries(kimera PUBLIC ${YAML_LDFLAGS})

if(LINUX OR MACOS OR WINDOWS)
    file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
    target_sources(kimera PRIVATE ${SOURCE_LIST})
endif()
