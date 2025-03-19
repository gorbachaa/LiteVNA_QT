
set( INC_DIR    inc )
set( SRC_DIR    src )

# Source files:
set( PROJECT_SOURCES )
glob_src( _list_inc_ ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
glob_src( _list_src_ ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND PROJECT_SOURCES ${_list_inc_} )
list( APPEND PROJECT_SOURCES ${_list_src_} )



add_library( xavna SHARED ${PROJECT_SOURCES} )

set_target_properties( xavna PROPERTIES VERSION ${xaVNA_VERSION_STRING} SOVERSION ${xaVNA_VERSION_MAJOR} )

target_link_libraries( xavna ${xa_LIBRARIES} )

# Declare destinations
install( TARGETS xavna LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )

if( WIN32 )
    add_custom_command(TARGET xavna POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copy libxavna.dll for deploy"
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/libxavna.dll
            ${CMAKE_CURRENT_BINARY_DIR}/../../${PROJECT_NAME}_deploy/libxavna.dll
    )
endif()
