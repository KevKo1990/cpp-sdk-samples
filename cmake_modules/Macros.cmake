# Extracting the subdirectories from a given folder
#
# Usage:
# SUBDIRLIST( SUBDIRS "path/to/base/dir" )
#
# Source: http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Search packages for host system instead of packages for target system
# in case of cross compilation thess macro should be defined by toolchain file
# adopted from OpenCV
if(NOT COMMAND find_host_package)
  macro(find_host_package)
    find_package(${ARGN})
  endmacro()
endif()
if(NOT COMMAND find_host_program)
  macro(find_host_program)
    find_program(${ARGN})
  endmacro()
endif()

macro(check_environment_variables)
  foreach(_var ${ARGN})
    if(NOT DEFINED ${_var} AND DEFINED ENV{${_var}})
      set(__value "$ENV{${_var}}")
      file(TO_CMAKE_PATH "${__value}" __value) # Assume that we receive paths
      set(${_var} "${__value}")
      message(STATUS "Update variable ${_var} from environment: ${${_var}}")
    endif()
  endforeach()
endmacro()

# Status convinience function.
# adopted from OpenCV
function(output_status msg)
  message(STATUS "${msg}")
  string(REPLACE "\\" "\\\\" msg "${msg}")
  string(REPLACE "\"" "\\\"" msg "${msg}")
endfunction()

# Status report function.
# Automatically align right column and selects text based on condition.
# Usage:
#   status(<text>)
#   status(<heading> <value1> [<value2> ...])
#   status(<heading> <condition> THEN <text for TRUE> ELSE <text for FALSE> )
# adopted from OpenCV
function(status text)
  set(status_cond)
  set(status_then)
  set(status_else)

  set(status_current_name "cond")
  foreach(arg ${ARGN})
    if(arg STREQUAL "THEN")
      set(status_current_name "then")
    elseif(arg STREQUAL "ELSE")
      set(status_current_name "else")
    else()
      list(APPEND status_${status_current_name} ${arg})
    endif()
  endforeach()

  if(DEFINED status_cond)
    set(status_placeholder_length 18)
    string(RANDOM LENGTH ${status_placeholder_length} ALPHABET " " status_placeholder)
    string(LENGTH "${text}" status_text_length)
    if(status_text_length LESS status_placeholder_length)
      string(SUBSTRING "${text}${status_placeholder}" 0 ${status_placeholder_length} status_text)
    elseif(DEFINED status_then OR DEFINED status_else)
      output_status("${text}")
      set(status_text "${status_placeholder}")
    else()
      set(status_text "${text}")
    endif()

    if(DEFINED status_then OR DEFINED status_else)
      if(${status_cond})
        string(REPLACE ";" " " status_then "${status_then}")
        string(REGEX REPLACE "^[ \t]+" "" status_then "${status_then}")
        output_status("${status_text} ${status_then}")
      else()
        string(REPLACE ";" " " status_else "${status_else}")
        string(REGEX REPLACE "^[ \t]+" "" status_else "${status_else}")
        output_status("${status_text} ${status_else}")
      endif()
    else()
      string(REPLACE ";" " " status_cond "${status_cond}")
      string(REGEX REPLACE "^[ \t]+" "" status_cond "${status_cond}")
      output_status("${status_text} ${status_cond}")
    endif()
  else()
    output_status("${text}")
  endif()
endfunction()

# Call find_package for a library and add its include dirs and library files to ${proj}_INCLUDE_DIRS and ${proj}_LIBS
#
# Usage:
# FIND_AND_ADD_LIB some_library

macro(FIND_AND_ADD_LIB proj lib)
    if(${lib}_DIR)
        find_package(${lib} REQUIRED)
        if(${lib}_FOUND)
            list(APPEND ${proj}_INCLUDE_DIRS ${${lib}_INCLUDE_DIRS} )
            list(APPEND ${proj}_LIBS ${${lib}_LIBRARIES} )
        else()
            message(SEND_ERROR "Failed to find ${lib}. Double check that \"${lib}_DIR\" to the root build directory of ${lib}.")
        endif()
    else()
        set(${lib}_DIR "" CACHE PATH "Root directory for ${lib} BUILD directory." )
        message(FATAL_ERROR "\"${lib}_DIR\" not set. Please explicitly provide the path to the root build directory of ${lib}.")
    endif()
endmacro()

# Add a library's include dir to ${proj}_INCLUDE_DIRS and its release/dbug libs to ${proj}_LIBS
# this assumes that the cmake caller (cmd line or cmake-gui) have provided values for:
# ${lib}_INCLUDE
# ${lib}_LIBRARY  (the release mode lib)
# ${lib}_LIBRARY_DEBUG (the debug mode lib)
#
# At least one of the _LIBRARY and _LIBRARY_DEBUG variables must be defined.
#
# Usage:
# ADD_LIB some_library

macro(ADD_LIB proj lib)
    if(${lib}_INCLUDE)
        list( APPEND ${proj}_INCLUDE_DIRS ${${lib}_INCLUDE} )
    else()
        set( ${lib}_INCLUDE "" CACHE PATH "${lib} include dir" )
        message(FATAL_ERROR "\"${lib}_INCLUDE\" not set. Please explicitly provide the path to the ${lib} include dir.")
    endif()

    if(${lib}_LIBRARY AND ${lib}_LIBRARY_DEBUG AND MSVC)
        list( APPEND ${proj}_LIBS optimized ${${lib}_LIBRARY} debug ${${lib}_LIBRARY_DEBUG})
    else()
        set( ${lib}_LIBRARY "" CACHE FILEPATH "${lib} release mode library" )
        set( ${lib}_LIBRARY_DEBUG "" CACHE FILEPATH "${lib} debug mode library" )
        if(${lib}_LIBRARY)
            list( APPEND ${proj}_LIBS ${${lib}_LIBRARY})
        elseif(${lib}_LIBRARY_DEBUG)
            list( APPEND ${proj}_LIBS ${${lib}_LIBRARY_DEBUG})
        else()
            message(FATAL_ERROR "\"${lib}_LIBRARY\" and/or \"${lib}_LIBRARY_DEBUG\" not set. Please explicitly provide one or both of these values.")
        endif()
    endif()
endmacro()

# Setup compiler for C++11 compatibility
macro(CONFIG_COMPILER_11)
    status("Setting up compiler config")

    cmake_minimum_required (VERSION 3.1)  #CMAKE_CXX_STANDARD support was introduced in 3.1
    set(CMAKE_CXX_STANDARD 11)

    if(MSVC)
        # Force to always compile with W4
        if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
        endif()
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        set(CXX_COMPILER_WARNINGS "-Wreturn-type" CACHE STRING "Compiler warnings to use")
        set(CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS} ${CXX_COMPILER_WARNINGS}")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
            status("Updating compiler to make use of C++11")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -fno-vectorize")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -fno-vectorize")
            set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} ${CXX_COMPILER_WARNINGS}")
        else()
            status("Updating compiler to make use of C++11")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
            set(CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS} ${CXX_COMPILER_WARNINGS}")

        endif()
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    endif()

	status("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

    # Setup "Profile" build type
    set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg")
    set(CMAKE_C_FLAGS_PROFILE "-O3 -pg")
endmacro()

macro(CONFIG_LINKER)
    # Setup "Profile" build type
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg")
    set(CMAKE_MODULE_LINKER_FLAGS_PROFILE "-pg")
endmacro()

# Boost package
# ----------------------------------------------------------------------------
# find Boost components.  Usage: ADD_BOOST proj component1 component2 component3...
# Note: BOOST_ROOT must be set.
# Sets:
#     Boost_FOUND, Boost_INCLUDE_DIRS, Boost_LIBRARY_DIRS, Boost_LIBRARIES, Boost_VERSION
# Appends to:
#     ${proj}_INCLUDE_DIRS, ${proj}_LIBS
macro(ADD_BOOST proj)
    set(BOOST_COMPONENTS "${ARGN}")
    set(Boost_USE_STATIC_LIBS ON)
    set(BOOST_MIN_VERSION "1.63.0" CACHE STRING "Minimum version of boost you would like to link against (e.g. C:/BOOST_1_63_0 is 1.63.0" )
    status("")
    find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS} )
    if(NOT Boost_FOUND)
        if(NOT DEFINED BOOST_ROOT)
            set(BOOST_ROOT "" CACHE PATH "Root directory for Boost.")
        endif(NOT DEFINED BOOST_ROOT)
        message(FATAL_ERROR "Failed to find Boost (or missing components). Double check that \"BOOST_ROOT\" is properly set")
    endif( NOT Boost_FOUND )
    list( APPEND ${proj}_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})

    list( APPEND ${proj}_LIBS ${Boost_LIBRARIES})
endmacro()

# Setup install locations
macro(SETUP_INSTALL_DIRS)
    if( NOT RUNTIME_INSTALL_DIRECTORY )
        set( RUNTIME_INSTALL_DIRECTORY "bin" CACHE STRING "Install sub-directory of CMAKE_INSTALL_PREFIX for RUNTIME targets (binaries, and *.dll on windows)." )
    endif( NOT RUNTIME_INSTALL_DIRECTORY )

    if( NOT LIBRARY_INSTALL_DIRECTORY )
        set( DEFAULT_LIBRARY_INSTALL_DIRECTORY "lib")
        set( LIBRARY_INSTALL_DIRECTORY ${DEFAULT_LIBRARY_INSTALL_DIRECTORY} CACHE STRING
            "Install sub-directory of CMAKE_INSTALL_PREFIX for LIBRARY targets (shared libs)" )
    endif( NOT LIBRARY_INSTALL_DIRECTORY )

    if( NOT ARCHIVE_INSTALL_DIRECTORY )
        set( DEFAULT_ARCHIVE_INSTALL_DIRECTORY "lib")
        set( ARCHIVE_INSTALL_DIRECTORY ${DEFAULT_ARCHIVE_INSTALL_DIRECTORY} CACHE STRING
            "Install sub-directory of CMAKE_INSTALL_PREFIX for ARCHIVE targets (static libs and *.def on windows)" )
    endif( NOT ARCHIVE_INSTALL_DIRECTORY )

    set( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Output directory for libraries" )
    set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Output directory for applications" )
endmacro()
