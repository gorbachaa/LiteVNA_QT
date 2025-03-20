
cmake_minimum_required( VERSION 3.5 )


#-------------------------------------------------------------------------------
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

function( glob_resrc VARNAME resources_path )
    file( GLOB_RECURSE _glob_resrc
            ${resources_path}/*.qrc
            ${resources_path}/*.rc
        )
    set( ${VARNAME} ${_glob_resrc} PARENT_SCOPE )
endfunction( glob_resrc )
#-------------------------------------------------------------------------------

function( glob_ui VARNAME ui_path )
    file( GLOB_RECURSE _glob_ui
            ${ui_path}/*.ui
        )
    set( ${VARNAME} ${_glob_ui} PARENT_SCOPE )
endfunction( glob_ui )
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

