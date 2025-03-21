
include_directories( ${Qt5Charts_INCLUDE_DIRS} )

# Instruct CMake to run moc automatically when needed
set( CMAKE_AUTOMOC ON )
# Create code from a list of Qt designer ui files
set( CMAKE_AUTOUIC ON )

set( CMAKE_AUTORCC ON )

cmake_minimum_required( VERSION 3.5 )


set( _pro_name litevna_qt )
project( ${_pro_name} LANGUAGES CXX )


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


if( WIN32 )
    set( win_deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/../../${APP_NAME}_${APP_VERSION_STRING}_deploy_win )

    find_program( WINDEPLOYQT windeployqt HINTS "${QT_BIN_DIR}" )

    find_program( ZLIB1           zlib1.dll                HINTS "${QT_BIN_DIR}" )
    find_program( LIBMD4C         libmd4c.dll              HINTS "${QT_BIN_DIR}" )
    find_program( LIBPCRE2_8      libpcre2-8-0.dll         HINTS "${QT_BIN_DIR}" )
    find_program( LIBPCRE2_16     libpcre2-16-0.dll        HINTS "${QT_BIN_DIR}" )
    find_program( LIBICONV_2      libiconv-2.dll           HINTS "${QT_BIN_DIR}" )
    find_program( LIBFFTW3        libfftw3-3.dll           HINTS "${QT_BIN_DIR}" )
    find_program( LIBZSTD         libzstd.dll              HINTS "${QT_BIN_DIR}" )
    find_program( LIBDOUBLE       libdouble-conversion.dll HINTS "${QT_BIN_DIR}" )
    find_program( LIBPNG16        libpng16-16.dll          HINTS "${QT_BIN_DIR}" )
    find_program( LIBFREETYPE     libfreetype-6.dll        HINTS "${QT_BIN_DIR}" )
    find_program( LIBBROTLICOMMON libbrotlicommon.dll      HINTS "${QT_BIN_DIR}" )
    find_program( LIBBROTLIDEC    libbrotlidec.dll         HINTS "${QT_BIN_DIR}" )
    find_program( LIBBZ2          libbz2-1.dll             HINTS "${QT_BIN_DIR}" )
    find_program( LIBGCC          libgcc_s_seh-1.dll       HINTS "${QT_BIN_DIR}" )
    find_program( LIBSTDC         libstdc++-6.dll          HINTS "${QT_BIN_DIR}" )
    find_program( LIBGLIB         libglib-2.0-0.dll        HINTS "${QT_BIN_DIR}" )
    find_program( LIBGRAPHITE     libgraphite2.dll         HINTS "${QT_BIN_DIR}" )
    find_program( LIBHARFBUZZ     libharfbuzz-0.dll        HINTS "${QT_BIN_DIR}" )
    find_program( LIBICUDT        libicudt74.dll           HINTS "${QT_BIN_DIR}" )
    find_program( LIBICUIN        libicuin74.dll           HINTS "${QT_BIN_DIR}" )
    find_program( LIBICIUC        libicuuc74.dll           HINTS "${QT_BIN_DIR}" )
    find_program( LIBINTL         libintl-8.dll            HINTS "${QT_BIN_DIR}" )


    add_custom_command(TARGET ${_pro_name} POST_BUILD
        # copy app
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${_pro_name}.exe
            ${win_deploy_dir}/${_pro_name}.exe
        # copy dll's
        COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB1}           ${win_deploy_dir}/zlib1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBMD4C}         ${win_deploy_dir}/libmd4c.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPCRE2_8}      ${win_deploy_dir}/libpcre2-8-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPCRE2_16}     ${win_deploy_dir}/libpcre2-16-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICONV_2}      ${win_deploy_dir}/libiconv-2.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBFFTW3}        ${win_deploy_dir}/libfftw3-3.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBZSTD}         ${win_deploy_dir}/libzstd.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBDOUBLE}       ${win_deploy_dir}/libdouble-conversion.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBPNG16}        ${win_deploy_dir}/libpng16-16.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBFREETYPE}     ${win_deploy_dir}/libfreetype-6.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBROTLICOMMON} ${win_deploy_dir}/libbrotlicommon.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBROTLIDEC}    ${win_deploy_dir}/libbrotlidec.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBBZ2}          ${win_deploy_dir}/libbz2-1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGCC}          ${win_deploy_dir}/libgcc_s_seh-1.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBSTDC}         ${win_deploy_dir}/libstdc++-6.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGLIB}         ${win_deploy_dir}/libglib-2.0-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGRAPHITE}     ${win_deploy_dir}/libgraphite2.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBHARFBUZZ}     ${win_deploy_dir}/libharfbuzz-0.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICUDT}        ${win_deploy_dir}/libicudt74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICUIN}        ${win_deploy_dir}/libicuin74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBICIUC}        ${win_deploy_dir}/libicuuc74.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBINTL}         ${win_deploy_dir}/libintl-8.dll

        # win deploy
        COMMAND cd ${win_deploy_dir}
        COMMAND "${WINDEPLOYQT}" ${_pro_name}.exe
    )
else()
    set( linux_deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/../../${APP_NAME}_${APP_VERSION_STRING}_deploy_linux )
    add_custom_command(TARGET ${_pro_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${_pro_name} ${linux_deploy_dir}/${_pro_name}
    )
endif()
