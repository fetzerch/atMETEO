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
option(BUILD_DOCUMENTATION "Build documentation" ON)

# Documentation
if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(dox ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# Print configuration ---------------------------------------------------------
include(Utils)
message(STATUS)
message(STATUS "${CMAKE_PROJECT_NAME} - Configuration:")
status_message("Build type" CMAKE_BUILD_TYPE)
status_message("Documentation" BUILD_DOCUMENTATION)
message(STATUS)
