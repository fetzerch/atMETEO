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
find_package(Threads REQUIRED)

# Set default directories.
if(NOT GMOCK_SOURCE_DIR)
    set(GMOCK_SOURCE_DIR ${CMAKE_BINARY_DIR}/gmock-src)
endif()
if(NOT GMOCK_BINARY_DIR)
    set(GMOCK_BINARY_DIR ${CMAKE_BINARY_DIR}/gmock-bin)
endif()

set(CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})

# Determine if GMOCK_SOURCE_DIR contains GMock sources.
find_file(GMOCK_LOCAL_SOURCE
    NAMES gmock.h
    PATHS ${GMOCK_SOURCE_DIR}/googlemock/include/gmock
    NO_DEFAULT_PATH)

# Build GMock from local sources.
if(GMOCK_LOCAL_SOURCE)

    message(STATUS "Using local GMock version")

    externalproject_add(GMock
        PREFIX gmock
        CMAKE_ARGS ${CMAKE_ARGS}
        SOURCE_DIR ${GMOCK_SOURCE_DIR}
        BINARY_DIR ${GMOCK_BINARY_DIR}
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/libgmock.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/libgmock_main.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest_main.a)

# Download GMock.
else()
    if(NOT GMock_FIND_VERSION)
        set(GMock_FIND_VERSION 1.8.0)
    endif()

    message(STATUS "Downloading GMock ${GMock_FIND_VERSION}")

    externalproject_add(GMock
        PREFIX gmock
        GIT_REPOSITORY git@github.com:google/googletest.git
        GIT_TAG release-${GMock_FIND_VERSION}
        CMAKE_ARGS ${CMAKE_ARGS}
        SOURCE_DIR ${GMOCK_SOURCE_DIR}
        BINARY_DIR ${GMOCK_BINARY_DIR}
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/libgmock.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/libgmock_main.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest.a
        BUILD_BYPRODUCTS ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest_main.a)
endif()

# Set the result variables.
set(GMOCK_INCLUDE_DIRS
    ${GMOCK_SOURCE_DIR}/googlemock/include
    ${GMOCK_SOURCE_DIR}/googletest/include)

set(GTEST_INCLUDE_DIRS
    ${GMOCK_SOURCE_DIR}/googletest/include)

# Add dependency to the external target so that CMake is able
# to resolve the dependencies to GMock libs and builds GMock first.
macro(add_gmock_library _name _lib)
    string(TOUPPER "${_name}_LIBRARY" _LIB_VAR)
    set(${_LIB_VAR} ${_name} ${CMAKE_THREAD_LIBS_INIT})
    add_library(${_name} STATIC IMPORTED)
    set_target_properties(${_name} PROPERTIES IMPORTED_LOCATION ${_lib})
    add_dependencies(${_name} GMock)
endmacro()

add_gmock_library(gmock ${GMOCK_BINARY_DIR}/googlemock/libgmock.a)
add_gmock_library(gmock_main ${GMOCK_BINARY_DIR}/googlemock/libgmock_main.a)
add_gmock_library(gtest ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest.a)
add_gmock_library(gtest_main ${GMOCK_BINARY_DIR}/googlemock/gtest/libgtest_main.a)

set(GMOCK_BOTH_LIBRARIES ${GMOCK_MAIN_LIBRARY} ${GMOCK_LIBRARY})
set(GTEST_BOTH_LIBRARIES ${GTEST_MAIN_LIBRARY} ${GTEST_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG GMOCK_SOURCE_DIR)
