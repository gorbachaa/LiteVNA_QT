
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



