option(YAML_BUILD_SHARED_LIBS "" ON)
option(BUILD_STATIC_WITH_PIC "" ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/yaml-cpp build)
target_link_libraries(kimera PRIVATE yaml-cpp)

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
target_sources(kimera PRIVATE ${SOURCE_LIST})
