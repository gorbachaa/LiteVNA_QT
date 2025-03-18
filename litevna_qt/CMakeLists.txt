
include_directories( ${Qt5Charts_INCLUDE_DIRS} )

# Instruct CMake to run moc automatically when needed
set( CMAKE_AUTOMOC ON )
# Create code from a list of Qt designer ui files
set( CMAKE_AUTOUIC ON )

set( CMAKE_AUTORCC ON )

cmake_minimum_required( VERSION 3.5 )





set( _pro_name litevna_qt )
project( ${_pro_name} VERSION 0.1 LANGUAGES CXX )

set( INC_DIR    inc )
set( SRC_DIR    src )
set( UI_DIR     ui )
set( RESRC_DIR  resources )



# Source files:
set( PROJECT_SOURCES )
glob_src( _list_inc_ ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
glob_src( _list_src_ ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND PROJECT_SOURCES ${_list_inc_} )
list( APPEND PROJECT_SOURCES ${_list_src_} )

# Resources files:
set( PROJECT_RESOURCES )
glob_resrc( _list_resrc_  ${CMAKE_CURRENT_SOURCE_DIR}/${RESRC_DIR} )
list( APPEND PROJECT_RESOURCES ${_list_resrc_} )

# Ui files:
set( PROJECT_UI )
glob_ui( _list_ui_  ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR} )
list( APPEND PROJECT_UI ${_list_ui_} )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${RESRC_DIR}
)

list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR} )
list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} )
list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${RESRC_DIR} )
list( APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${UI_DIR} )


add_executable( ${_pro_name} ${PROJECT_SOURCES} ${PROJECT_RESOURCES} ${PROJECT_UI} )

target_link_libraries( ${_pro_name} Qt5::Charts ${FFTW3_LIBRARIES} xavna xavna_mock_ui )

target_include_directories( ${_pro_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libxavna/inc )

# Install destinations
install( TARGETS ${_pro_name} RUNTIME DESTINATION ${BIN_INSTALL_DIR} )
