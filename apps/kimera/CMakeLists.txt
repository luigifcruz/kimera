file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.c)

add_executable(kimera_cli ${SOURCE_LIST})

set_target_properties(kimera_cli PROPERTIES OUTPUT_NAME kimera)

target_link_libraries(kimera_cli PRIVATE kimera)

configure_file(
  ${kimera_SOURCE_DIR}/assets/CourierPrime-Regular.ttf
  CourierPrime-Regular.ttf COPYONLY
)