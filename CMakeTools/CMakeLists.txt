
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
# Deploy app and lib
#
# Usage:
#      app_deploy( my_app_name 1.0.0.1 )
#
#      lib_deploy( my_app_name 1.0.0.1
#                     Qt6Core.dll
#                     Qt6Gui.dll
#                 )

#-------------------------------------------------------------------------------
# global variables
set( deploy_dir )
set( platform_name )
set( platform_app_tail )
set( archive_type )

#-------------------------------------------------------------------------------
macro( pre_deploy i_name i_version )
    if( WIN32 )
        set( platform_name windows )
        set( platform_app_tail .exe )
        set( archive_type zip )
    elseif( UNIX )
        set( platform_name linux )
        set( platform_app_tail  )
        set( archive_type tgz )
    endif()

    # set deploy dir
    set( deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/${i_name}_${i_version}_deploy_${platform_name} )

    message( "platform: \"${platform_name}\"" )
    message( "archive: \"${archive_type}\"" )
    message( "deploy dir: \"${deploy_dir}\"" )
endmacro()


#-------------------------------------------------------------------------------
function( app_deploy in_name in_version )
    message( "app deploy" )
    pre_deploy( ${in_name} ${in_version} )

    # copy app to deploy dir
    add_custom_command( TARGET ${in_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${in_name}${platform_app_tail}
        ${deploy_dir}/${in_name}${platform_app_tail}
    )


  if( WIN32 )
      # windows deploy
      find_program( delpoy_tool windeployqt HINTS "${QT_BIN_DIR}" )
      add_custom_command( TARGET ${in_name} POST_BUILD
          COMMAND "${delpoy_tool}" ${deploy_dir}/${in_name}${platform_app_tail}
      )
  elseif( UNIX )
      FILE( WRITE ${deploy_dir}/run.sh
           "###############################################\n"
           "# Start app in local folder\n"
           "export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH\n"
           "./${in_name}\n"
      )
  endif()

  FILE( WRITE ${deploy_dir}/readme.md
       "###############################################\n"
       "# app ${in_name}_${in_version}\n"
       "windows:\n"
       "- for start run ${in_name}.exe\n"
       "linux(ubuntu):\n"
       "- for start run bush run.sh\n"
  )

  # create the archive deploy folder
  add_custom_command( TARGET ${in_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E tar cvf ${deploy_dir}.${archive_type} --format=zip .
      WORKING_DIRECTORY ${deploy_dir}
  )
endfunction( app_deploy )

#-------------------------------------------------------------------------------
function( lib_deploy in_name in_version )
  message( "lib deploy" )
  pre_deploy( ${in_name} ${in_version} )

  message( "lib list:${ARGN}" )

  # Alternatively, you can access the lists directly without parsing
  foreach( item IN LISTS ARGN )
      message("Item in List:${item}")
      # find lib
      if( UNIX )
          set( FIND_LIB /lib/x86_64-linux-gnu/${item} )
      elseif( WIN32 )
          find_file( FIND_LIB item HINTS "${QT_BIN_DIR}" )
      endif()
      message("find lib:${FIND_LIB}")
      # copy libs
      add_custom_command( TARGET ${in_name} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${FIND_LIB} ${deploy_dir}/${item}
      )
  endforeach()

endfunction( lib_deploy )

#-------------------------------------------------------------------------------

