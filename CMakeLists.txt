# Project setup ---------------------------------------------------------------
project(atMETEO)
cmake_minimum_required(VERSION 2.8.11)

# Show additional project files in IDEs (e.g. QtCreator)
set(PROJECT_FILES README.md LICENSE)
add_custom_target(_dummy SOURCES ${PROJECT_FILES})

# Additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake/modules)

# Options and configuration ---------------------------------------------------

# Target specific configuration for cross compiling with cmake-avr
# (cmake-avr https://github.com/mkleemann/cmake-avr)
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
    set(TARGET_CONFIGURATION dummy CACHE STRING "Target configuration")
    set(TARGET_CONFIGURATIONS dummy)
    list(FIND TARGET_CONFIGURATIONS ${TARGET_CONFIGURATION} _INDEX)
    if(_INDEX LESS 0)
        message(FATAL_ERROR "Target configuration '${TARGET_CONFIGURATION}'"
                            "does not exist. Valid target configurations: "
                            ${VALID_TARGET_CONFIGURATIONS})
    endif()
endif()

# Options
include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTING "Build unit tests" ON
    "NOT CMAKE_CROSSCOMPILING" OFF)
option(BUILD_DOCUMENTATION "Build documentation" ON)

# Build targets ---------------------------------------------------------------

# Default compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

# libsensors: Hardware independent utilities and algorithms
add_subdirectory(lib)

# libsensors: Host based unit tests
if(BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
endif()

# Documentation
if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(dox ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# Target specific code
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
    add_subdirectory(target)
endif()

# Print configuration ---------------------------------------------------------
include(Utils)
message(STATUS)
message(STATUS "${CMAKE_PROJECT_NAME} - Configuration:")
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
    status_message("Cross compiling" TARGET_CONFIGURATION)
    status_message("AVR processor" AVR_MCU)
    status_message("AVR upload tool" AVR_UPLOADTOOL)
    status_message("AVR upload tool port" AVR_UPLOADTOOL_PORT)
    status_message("AVR programmer" AVR_PROGRAMMER)
else()
    status_message("Compiling for local system")
endif()
status_message("Build type" CMAKE_BUILD_TYPE)
status_message("Unit tests" BUILD_TESTING)
status_message("Documentation" BUILD_DOCUMENTATION)
message(STATUS)
