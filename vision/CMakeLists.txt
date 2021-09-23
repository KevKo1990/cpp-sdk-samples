cmake_minimum_required(VERSION 3.0)

set(rootProject vision-samples)
project(${rootProject})

# load some useful macros
include(../cmake_modules/Macros.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules")

# --------------------
# CONFIG
# --------------------

CONFIG_COMPILER_11()
CONFIG_LINKER()
SETUP_INSTALL_DIRS()

# --------------------
# DEPENDENCIES
# --------------------

# OpenCV
set( OPENCV_COMPONENTS highgui core )
if( DEFINED OpenCV_DIR )    # Force the user to tell us which OpenCV they want (otherwise find_package can find the wrong one, cache it and changes to OpenCV_DIR are ignored)
    find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
    if( NOT OpenCV_FOUND)
        message(SEND_ERROR "Failed to find OpenCV. Double check that \"OpenCV_DIR\" to the root build directory of OpenCV.")
    endif(NOT OpenCV_FOUND)
else( DEFINED OpenCV_DIR )
    set( OpenCV_DIR "" CACHE PATH "Root directory for opencv BUILD directory." )
    message(FATAL_ERROR "\"OpenCV_DIR\" not set. Please explicitly provide the path to the root build directory of OpenCV.")
endif( DEFINED OpenCV_DIR )

# Boost
ADD_BOOST(${rootProject} system filesystem program_options)

# Affectiva SDK
if(NOT DEFINED AFFECTIVA_SDK_DIR )
    set(AFFECTIVA_SDK_DIR "" CACHE PATH "Install directory for the Affectiva SDK" )
    message(FATAL_ERROR "Please set AFFECTIVA_SDK_DIR to the directory where the Affectiva SDK is installed")
endif()

set(AffectivaVision_DIR ${AFFECTIVA_SDK_DIR})
FIND_AND_ADD_LIB(${rootProject} AffectivaVision)

# --------------------
# SUBPROJECTS
# --------------------

add_subdirectory(frame-detector-webcam-demo)
add_subdirectory(frame-detector-video-demo)

# --------------------
# SUMMARY
# --------------------

status("${CMAKE_INCLUDE_DIRECTORIES}")
status( "------- SUMMARY ------- " )
status( "Boost version found   = ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} (${Boost_VERSION})" )
foreach( comp ${BOOST_COMPONENTS} )
    string( TOUPPER ${comp} COMP )
    status( "  - ${comp}" 1 THEN "${Boost_${COMP}_LIBRARY}" )
endforeach( comp )

status("")
status("Affectiva SDK")
status( "${AffectivaVision_LIBRARY}")

status("")
status( "OpenCV version found   = ${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH} (${OpenCV_VERSION})" )
status( "OpenCV_LIB_DIR         = ${OpenCV_DIR}/lib" )
foreach( lib ${OpenCV_LIBRARIES} )
    foreach( comp ${OPENCV_COMPONENTS} )
        if( ${lib} MATCHES ${comp} )
            status( "  - ${comp}" 1 THEN "${lib}" )
        endif( ${lib} MATCHES ${comp} )
    endforeach( comp )
endforeach( lib )

status("")
status( "Apps identified for building:" )
foreach( app ${${rootProject}_APPS} )
    status( "   - ${app}" )
endforeach( app ${${rootProject}_APPS} )
