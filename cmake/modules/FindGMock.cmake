# - Find Google Mock
# Find Google Mock (and Google Test).
#
# This module includes and builds Google Mock as external project since it is
# not recommended to use any pre-compiled version of the library (see
# https://code.google.com/p/googletest/wiki/FAQ).
#
# The directory containing the Google Mock sources can be specified with
# GMOCK_SOURCE_DIR. If this variable is omitted or the specified directory
# does not include a version of Google Mock, the sources will download.
#
# The directory where Google Mock is built can be specified with
# GMOCK_BINARY_DIR.
#
# This module defines the following variables:
#  GMOCK_FOUND          - If Google Mock is available
#  GMOCK_INCLUDE_DIRS   - Include directories for <gmock/gmock.h>
#  GMOCK_LIBRARY        - Google Mock Library
#  GMOCK_MAIN_LIBRARY   - Google Mock Main Library
#  GMOCK_BOTH_LIBRARIES - Google Mock Libraries
#  GTEST_INCLUDE_DIRS   - Include directories for <gtest/gtest.h>
#  GTEST_LIBRARY        - Google Test Library
#  GTEST_MAIN_LIBRARY   - Google Test Main Library
#  GTEST_BOTH_LIBRARIES - Google Test Libraries
#

include(ExternalProject)
include(CheckCXXCompilerFlag)
find_package(Threads REQUIRED)

# Set default directories.
if(NOT GMOCK_SOURCE_DIR)
    set(GMOCK_SOURCE_DIR ${CMAKE_BINARY_DIR}/gmock-src)
endif()
if(NOT GMOCK_BINARY_DIR)
    set(GMOCK_BINARY_DIR ${CMAKE_BINARY_DIR}/gmock-bin)
endif()

# Compiler settings have to be passed on to externalproject_add in order to
# support cross compilation.
# Disable 'unused-local-typedefs' warnings for GMock < 1.7.0.
if(GMock_FIND_VERSION AND GMock_FIND_VERSION VERSION_LESS "1.7.0")
    check_cxx_compiler_flag(
        -Wno-unused-local-typedefs HAS_NO_UNUSED_LOCAL_TYPEDEFS)
    if(HAS_NO_UNUSED_LOCAL_TYPEDEFS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()
endif()
set(CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})

# Determine if GMOCK_SOURCE_DIR contains GMock sources.
find_file(GMOCK_LOCAL_SOURCE
    NAMES gmock.h
    PATHS ${GMOCK_SOURCE_DIR}/include/gmock
    NO_DEFAULT_PATH)

# Build GMock from local sources.
if(GMOCK_LOCAL_SOURCE)

    message(STATUS "Using local GMock version")

    externalproject_add(GMock
        PREFIX gmock
        CMAKE_ARGS ${CMAKE_ARGS}
        SOURCE_DIR ${GMOCK_SOURCE_DIR}
        BINARY_DIR ${GMOCK_BINARY_DIR}
        INSTALL_COMMAND "")

# Download GMock.
else()

    # Latest version available on the googlecode.com mirror.
    if(NOT GMock_FIND_VERSION)
        set(GMock_FIND_VERSION 1.7.0)
    endif()

    message(STATUS "Downloading GMock ${GMock_FIND_VERSION}")
    set(GMOCK_URL "http://googlemock.googlecode.com/files/")
    set(GMOCK_URL "${GMOCK_URL}/gmock-${GMock_FIND_VERSION}.zip")

    externalproject_add(GMock
        PREFIX gmock
        URL "${GMOCK_URL}"
        CMAKE_ARGS ${CMAKE_ARGS}
        SOURCE_DIR ${GMOCK_SOURCE_DIR}
        BINARY_DIR ${GMOCK_BINARY_DIR}
        INSTALL_COMMAND ""
        UPDATE_COMMAND "")
endif()

# Set the result variables.
set(GMOCK_INCLUDE_DIRS
    ${GMOCK_SOURCE_DIR}/include
    ${GMOCK_SOURCE_DIR}/gtest/include)

set(GTEST_INCLUDE_DIRS
    ${GMOCK_SOURCE_DIR}/gtest/include)

# Add dependency to the external target so that CMake is able
# to resolve the dependencies to GMock libs and builds GMock first.
macro(add_gmock_library _name _lib)
    string(TOUPPER "${_name}_LIBRARY" _LIB_VAR)
    set(${_LIB_VAR} ${_name} ${CMAKE_THREAD_LIBS_INIT})
    add_library(${_name} STATIC IMPORTED)
    set_target_properties(${_name} PROPERTIES IMPORTED_LOCATION ${_lib})
    add_dependencies(${_name} GMock)
endmacro()

add_gmock_library(gmock ${GMOCK_BINARY_DIR}/libgmock.a)
add_gmock_library(gmock_main ${GMOCK_BINARY_DIR}/libgmock_main.a)
add_gmock_library(gtest ${GMOCK_BINARY_DIR}/gtest/libgtest.a)
add_gmock_library(gtest_main ${GMOCK_BINARY_DIR}/gtest/libgtest_main.a)

# For GMock < 1.7.0 GTest symbols are not included in the library
# and have to be added separately.
if(GMock_FIND_VERSION AND GMock_FIND_VERSION VERSION_LESS "1.7.0")
    list(APPEND GMOCK_LIBRARY ${GTEST_LIBRARY})
endif()

set(GMOCK_BOTH_LIBRARIES ${GMOCK_MAIN_LIBRARY} ${GMOCK_LIBRARY})
set(GTEST_BOTH_LIBRARIES ${GTEST_MAIN_LIBRARY} ${GTEST_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG GMOCK_SOURCE_DIR)
