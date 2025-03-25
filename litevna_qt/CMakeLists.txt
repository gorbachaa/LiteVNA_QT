
include_directories( ${Qt5Charts_INCLUDE_DIRS} )

# Instruct CMake to run moc automatically when needed
set( CMAKE_AUTOMOC ON )
# Create code from a list of Qt designer ui files
set( CMAKE_AUTOUIC ON )

set( CMAKE_AUTORCC ON )

cmake_minimum_required( VERSION 3.5 )

project( ${APP_NAME} LANGUAGES CXX )

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


add_executable( ${APP_NAME} ${PROJECT_SOURCES} ${PROJECT_RESOURCES} ${PROJECT_UI} )

target_link_libraries( ${APP_NAME} Qt5::Charts ${FFTW3_LIBRARIES} xavna xavna_mock_ui )

target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libxavna/inc )

# Install destinations
install( TARGETS ${APP_NAME} RUNTIME DESTINATION ${BIN_INSTALL_DIR} )

# copy app to deploy dir
add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}${PLATFORM_APP_TAIL}
    ${DEPLOY_DIR}/${APP_NAME}${PLATFORM_APP_TAIL}
)

if( WIN32 )

    find_file( ZLIB1           zlib1.dll                HINTS "${QT_BIN_DIR}" )
    find_file( LIBMD4C         libmd4c.dll              HINTS "${QT_BIN_DIR}" )
    find_file( LIBPCRE2_8      libpcre2-8-0.dll         HINTS "${QT_BIN_DIR}" )
    find_file( LIBPCRE2_16     libpcre2-16-0.dll        HINTS "${QT_BIN_DIR}" )
    find_file( LIBICONV_2      libiconv-2.dll           HINTS "${QT_BIN_DIR}" )
    find_file( LIBFFTW3        libfftw3-3.dll           HINTS "${QT_BIN_DIR}" )
    find_file( LIBZSTD         libzstd.dll              HINTS "${QT_BIN_DIR}" )
    find_file( LIBDOUBLE       libdouble-conversion.dll HINTS "${QT_BIN_DIR}" )
    find_file( LIBPNG16        libpng16-16.dll          HINTS "${QT_BIN_DIR}" )
    find_file( LIBFREETYPE     libfreetype-6.dll        HINTS "${QT_BIN_DIR}" )
    find_file( LIBBROTLICOMMON libbrotlicommon.dll      HINTS "${QT_BIN_DIR}" )
    find_file( LIBBROTLIDEC    libbrotlidec.dll         HINTS "${QT_BIN_DIR}" )
    find_file( LIBBZ2          libbz2-1.dll             HINTS "${QT_BIN_DIR}" )
    find_file( LIBGCC          libgcc_s_seh-1.dll       HINTS "${QT_BIN_DIR}" )
    find_file( LIBSTDC         libstdc++-6.dll          HINTS "${QT_BIN_DIR}" )
    find_file( LIBGLIB         libglib-2.0-0.dll        HINTS "${QT_BIN_DIR}" )
    find_file( LIBGRAPHITE     libgraphite2.dll         HINTS "${QT_BIN_DIR}" )
    find_file( LIBHARFBUZZ     libharfbuzz-0.dll        HINTS "${QT_BIN_DIR}" )
    find_file( LIBICUDT        libicudt74.dll           HINTS "${QT_BIN_DIR}" )
    find_file( LIBICUIN        libicuin74.dll           HINTS "${QT_BIN_DIR}" )
    find_file( LIBICIUC        libicuuc74.dll           HINTS "${QT_BIN_DIR}" )
    find_file( LIBINTL         libintl-8.dll            HINTS "${QT_BIN_DIR}" )

    add_custom_command( TARGET ${APP_NAME} POST_BUILD
        # copy dll's
        COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB1}           ${DEPLOY_DIR}/zlib1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBMD4C}         ${DEPLOY_DIR}/libmd4c.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPCRE2_8}      ${DEPLOY_DIR}/libpcre2-8-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPCRE2_16}     ${DEPLOY_DIR}/libpcre2-16-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICONV_2}      ${DEPLOY_DIR}/libiconv-2.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBFFTW3}        ${DEPLOY_DIR}/libfftw3-3.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBZSTD}         ${DEPLOY_DIR}/libzstd.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBDOUBLE}       ${DEPLOY_DIR}/libdouble-conversion.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPNG16}        ${DEPLOY_DIR}/libpng16-16.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBFREETYPE}     ${DEPLOY_DIR}/libfreetype-6.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBROTLICOMMON} ${DEPLOY_DIR}/libbrotlicommon.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBROTLIDEC}    ${DEPLOY_DIR}/libbrotlidec.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBZ2}          ${DEPLOY_DIR}/libbz2-1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGCC}          ${DEPLOY_DIR}/libgcc_s_seh-1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBSTDC}         ${DEPLOY_DIR}/libstdc++-6.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGLIB}         ${DEPLOY_DIR}/libglib-2.0-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGRAPHITE}     ${DEPLOY_DIR}/libgraphite2.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBHARFBUZZ}     ${DEPLOY_DIR}/libharfbuzz-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICUDT}        ${DEPLOY_DIR}/libicudt74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICUIN}        ${DEPLOY_DIR}/libicuin74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICIUC}        ${DEPLOY_DIR}/libicuuc74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBINTL}         ${DEPLOY_DIR}/libintl-8.dll
    )


    find_program( WINDEPLOYQT windeployqt HINTS "${QT_BIN_DIR}" )
    add_custom_command( TARGET ${APP_NAME} POST_BUILD
        # win deploy
        COMMAND "${WINDEPLOYQT}" ${DEPLOY_DIR}/${APP_NAME}${PLATFORM_APP_TAIL}
    )    

else()

endif()

# create the archive deploy folder
add_custom_command( TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar cvf ${DEPLOY_DIR}.${ARCHIVE_TYPE} --format=zip .
    WORKING_DIRECTORY ${DEPLOY_DIR}
)
