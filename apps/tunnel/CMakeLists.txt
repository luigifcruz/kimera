file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.c)

add_executable(tunnel ${SOURCE_LIST})

set_target_properties(tunnel PROPERTIES OUTPUT_NAME tunnel)

target_link_libraries(tunnel PRIVATE kimera)

configure_file(
    ${kimera_SOURCE_DIR}/assets/CourierPrime-Regular.ttf
    CourierPrime-Regular.ttf COPYONLY
)