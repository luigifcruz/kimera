# Compile YAML-CPP
option(YAML_CPP_BUILD_TOOLS "" OFF)
option(CMAKE_POSITION_INDEPENDENT_CODE "" ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies/yaml-cpp build)
target_link_libraries(kimera PUBLIC yaml-cpp)

# Add module source list
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
target_sources(kimera PRIVATE ${SOURCE_LIST})

