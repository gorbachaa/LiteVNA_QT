# Minimum CMake required
cmake_minimum_required( VERSION 3.5 )
# Set C++11 as standard for the whole project
set( CMAKE_CXX_STANDARD 11 )

if( WIN32 )
  if( ${CMAKE_VERSION} VERSION_LESS "3.8" )
    message( WARNING "Your current cmake version is ${CMAKE_VERSION} which does not support setting the toolset architecture to x64. This may cause \"compiler out of heap space\" errors when building. Consider upgrading your cmake to > 3.8 and using the flag -Thost=x64 when running cmake.")
  else()
    if( NOT CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE OR NOT "${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}" STREQUAL "x64" )
      message( WARNING "Your current cmake generator is set to use 32 bit toolset architecture. This may cause \"compiler out of heap space\" errors when building. Consider using the flag -Thost=x64 when running cmake.")
    endif()
  endif()
endif()

# Project
set( REPO_NAME LiteVNA_QT )
set( APP_NAME litevna_qt )

set( xaVNA_VERSION_MAJOR 0 )
set( xaVNA_VERSION_MINOR 3 )
set( APP_VERSION_STRING 1.0.${xaVNA_VERSION_MAJOR}.${xaVNA_VERSION_MINOR} )

if( WIN32 )
  set( PLATFORM_NAME windows )
  set( PLATFORM_APP_TAIL .exe )
  set( PLATFORM_LIB_TAIL .dll )
  set( ARCHIVE_TYPE zip )
elseif( UNIX )
  set( PLATFORM_NAME linux )
  set( PLATFORM_APP_TAIL  )
  set( PLATFORM_LIB_TAIL .so )
  set( ARCHIVE_TYPE tgz )
endif()

set( DEPLOY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${REPO_NAME}_${APP_VERSION_STRING}_deploy_${PLATFORM_NAME} )

project( ${REPO_NAME} C CXX )

if( UNIX )
  set( CMAKE_CXX_FLAGS "-O2 ${CMAKE_CXX_FLAGS}" )
  set( CMAKE_CXX_FLAGS_DEBUG "-O2 -g ${CMAKE_CXX_FLAGS}" )
endif()

if( NOT LIB_INSTALL_DIR )
    set( LIB_INSTALL_DIR lib )
endif()

if( NOT BIN_INSTALL_DIR )
    set( BIN_INSTALL_DIR bin )
endif()

# Set version
add_definitions( -DVERSION=\"${APP_VERSION_STRING}\" )
add_definitions( -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT )

# CMake policies
cmake_policy( SET CMP0022 NEW )

# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )

# Position independent code
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# Local Include
include_directories( include )

# ==============================================================================
# Eigen3
# ==============================================================================
find_package( Eigen3 REQUIRED )
if( EIGEN3_FOUND )
  message( STATUS "  eigen3 version: ${EIGEN3_VERSION}" )
  message( STATUS "  eigen3 includes: ${EIGEN3_INCLUDE_DIR}" )
  include_directories( ${EIGEN3_INCLUDE_DIR} )
endif()

# ==============================================================================
# FFTW3
# ==============================================================================
find_package( FFTW3 REQUIRED )
if( FFTW3_FOUND )
  message( STATUS "Found FFTW3 (${FFTW3_VERSION})" )
  message( STATUS "  fftw3 includes: ${FFTW3_INCLUDE_DIRS}" )
  message( STATUS "  fftw3 libraries: ${FFTW3_LIBRARIES}" )
  include_directories( ${FFTW3_INCLUDE_DIR} )
endif()

# ==============================================================================
# QT5 Charts (and QT5 subdependences)
# ==============================================================================
find_package( Qt5 COMPONENTS Core Gui Widgets Charts REQUIRED )
if( Qt5Charts_FOUND )
  message( STATUS "Found QT5 version: ${Qt5_VERSION}" )
  message( STATUS "  qt5 charts dir: ${Qt5Charts_DIR}" )
endif()

# ==============================================================================
# xaVNA subprojects
# ==============================================================================
add_subdirectory( CMakeTools )

add_subdirectory( libxavna )
add_subdirectory( libxavna_mock_ui )
add_subdirectory( litevna_qt )


