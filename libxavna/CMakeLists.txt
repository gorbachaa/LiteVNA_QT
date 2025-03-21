
set( INC_DIR    inc )
set( SRC_DIR    src )

# Source files:
set( PROJECT_SOURCES )
glob_src( _list_inc_ ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
glob_src( _list_src_ ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND PROJECT_SOURCES ${_list_inc_} )
list( APPEND PROJECT_SOURCES ${_list_src_} )

if(WIN32)
    reject_file_pattern( platform_abstraction.cpp PROJECT_SOURCES )
else()
    reject_file_pattern( platform_abstraction_windows.cpp PROJECT_SOURCES )
endif()

add_library( xavna SHARED ${PROJECT_SOURCES} )

set_target_properties( xavna PROPERTIES VERSION ${APP_VERSION_STRING} SOVERSION ${xaVNA_VERSION_MAJOR} )

target_link_libraries( xavna ${xa_LIBRARIES} )

# Declare destinations
install( TARGETS xavna LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )

if( WIN32 )
    set( win_deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/../../${APP_NAME}_${APP_VERSION_STRING}_deploy_win )
    add_custom_command(TARGET xavna POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libxavna.dll ${win_deploy_dir}/libxavna.dll
    )
else()
    set( linux_deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/../../${APP_NAME}_${APP_VERSION_STRING}_deploy_linux )
    add_custom_command(TARGET xavna POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libxavna.so ${linux_deploy_dir}/libxavna.so
    )
endif()
