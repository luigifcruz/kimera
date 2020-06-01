file(GLOB APP_SOURCES CONFIGURE_DEPENDS "*.c")

foreach(source ${APP_SOURCES})
    get_filename_component(app ${source} NAME_WE)
    get_filename_component(source ${source} NAME)
    
    add_executable(${app}_app ${source})
    set_target_properties(${app}_app PROPERTIES OUTPUT_NAME ${app})

    target_link_libraries(${app}_app PRIVATE kimera)
endforeach()

configure_file(
  ${kimera_SOURCE_DIR}/assets/CourierPrime-Regular.ttf
  ${kimera_BINARY_DIR}/apps/CourierPrime-Regular.ttf
  COPYONLY
)