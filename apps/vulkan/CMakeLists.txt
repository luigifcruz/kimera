file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.cc *.c)

add_executable(vulkan ${SOURCE_LIST})

set_target_properties(vulkan PROPERTIES OUTPUT_NAME vulkan)

include(${PROJECT_SOURCE_DIR}/cmake/vulkan.cmake)

target_link_libraries(vulkan PRIVATE kimera vk)
