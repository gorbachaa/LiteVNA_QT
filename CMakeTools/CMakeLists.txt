
cmake_minimum_required( VERSION 3.5 )


#-------------------------------------------------------------------------------
# Find source files
#
# Usage:
#      set( PROJECT_SOURCES )
#      glob_src( _list_inc_ ${PROJECT_SOURCE_DIR}/inc )
#      glob_src( _list_src_ ${PROJECT_SOURCE_DIR}/src )
#      list( APPEND PROJECT_SOURCES ${_list_inc_} )
#      list( APPEND PROJECT_SOURCES ${_list_src_}
function( glob_src VARNAME src_path )
    file( GLOB_RECURSE _glob_src
            ${src_path}/*.cpp
            ${src_path}/*.cc
            ${src_path}/*.c
            ${src_path}/*.s
            ${src_path}/*.S
            ${src_path}/*.h
            ${src_path}/*.hpp
        )
    set( ${VARNAME} ${_glob_src} PARENT_SCOPE )
endfunction( glob_src )
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Find resources files
#
# Usage:
#      set( PROJECT_RESOURCES )
#      glob_resrc( _list_resrc_  ${PROJECT_SOURCE_DIR}/res )
#      list( APPEND PROJECT_RESOURCES ${_list_resrc_} )
function( glob_resrc VARNAME resources_path )
    file( GLOB_RECURSE _glob_resrc
            ${resources_path}/*.qrc
            ${resources_path}/*.rc
        )
    set( ${VARNAME} ${_glob_resrc} PARENT_SCOPE )
endfunction( glob_resrc )
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Find forms files in folders
#
# Usage:
#     set( PROJECT_UI )
#     glob_ui( _list_ui_  ${PROJECT_SOURCE_DIR}/ui )
#     list( APPEND PROJECT_UI ${_list_ui_} )
function( glob_ui VARNAME ui_path )
    file( GLOB_RECURSE _glob_ui
            ${ui_path}/*.ui
        )
    set( ${VARNAME} ${_glob_ui} PARENT_SCOPE )
endfunction( glob_ui )
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Given a variable containing a file list,
# this macro will remove all the files wich
# basename match the specified pattern
#
# Usage:
#    file(GLOB_RECURSE config_files RELATIVE "${PROJECT_SOURCE_DIR}" *.in)
#    reject_file_pattern("^a" config_files)
macro(reject_file_pattern pattern input_list)
  foreach(src_file ${${input_list}})
    get_filename_component(base_name ${src_file} NAME)
    if(${base_name} MATCHES ${pattern})
      list(REMOVE_ITEM ${input_list} "${src_file}")
    endif()
  endforeach()
endmacro()
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Deploy app
#
# Usage:
#      app_deploy( my_app 1.0.0.1 )
function( app_deploy _name _version )
  if( WIN32 )
    set( platform_name windows )
    set( platform_app_tail .exe )
    set( archive_type zip )
  elseif( UNIX )
    set( platform_name linux )
    set( platform_app_tail  )
    set( archive_type tgz )
  endif()

  set( deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/${_name}_${_version}_deploy_${platform_name} )

  # copy app to deploy dir
  add_custom_command( TARGET ${_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}${platform_app_tail}
      ${deploy_dir}/${_name}${platform_app_tail}
  )


  if( WIN32 )
      # windows deploy
      find_program( delpoy_tool windeployqt HINTS "${QT_BIN_DIR}" )
      add_custom_command( TARGET ${_name} POST_BUILD
          COMMAND "${delpoy_tool}" ${deploy_dir}/${_name}${platform_app_tail}
      )

  elseif( UNIX )

  endif()

  # create the archive deploy folder
  add_custom_command( TARGET ${_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E tar cvf ${deploy_dir}.${archive_type} --format=zip .
      WORKING_DIRECTORY ${deploy_dir}
  )
endfunction( app_deploy )
#-------------------------------------------------------------------------------

