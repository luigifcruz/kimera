file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.c)

add_executable(benchmark ${SOURCE_LIST})

set_target_properties(benchmark PROPERTIES OUTPUT_NAME benchmark)

target_link_libraries(benchmark PRIVATE kimera)