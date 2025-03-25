
add_definitions( -DLIBXAVNA_MOCK_UI_LIBRARY )

include_directories( ${Qt5Charts_INCLUDE_DIRS} )

# Instruct CMake to run moc automatically when needed
set( CMAKE_AUTOMOC ON )
# Create code from a list of Qt designer ui files
set( CMAKE_AUTOUIC ON )

# Source & Ui dirs:
set( INC_DIR    inc )
set( SRC_DIR    src )
set( UI_DIR     ui )

# Source files:
set( PROJECT_SOURCES )
glob_src( _list_inc_ ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
glob_src( _list_src_ ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND PROJECT_SOURCES ${_list_inc_} )
list( APPEND PROJECT_SOURCES ${_list_src_} )

# Ui files:
set( PROJECT_UI )
glob_ui( _list_ui_  ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR} )
list( APPEND PROJECT_UI ${_list_ui_} )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR}
)

list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR} )

add_library( xavna_mock_ui SHARED ${PROJECT_SOURCES} ${PROJECT_UI} )

set_target_properties( xavna_mock_ui PROPERTIES VERSION ${APP_VERSION_STRING} SOVERSION ${xaVNA_VERSION_MAJOR} )

target_link_libraries( xavna_mock_ui Qt5::Core Qt5::Widgets Qt5::Gui xavna )

# Declare destinations
install( TARGETS xavna_mock_ui LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )

# copy lib in deploy dir
add_custom_command( TARGET xavna_mock_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/libxavna_mock_ui${PLATFORM_LIB_TAIL}
    ${DEPLOY_DIR}/libxavna_mock_ui${PLATFORM_LIB_TAIL}
)

