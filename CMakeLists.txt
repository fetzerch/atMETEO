# Project setup ---------------------------------------------------------------
project(atMETEO)
cmake_minimum_required(VERSION 2.8.11)

# Show additional project files in IDEs (e.g. QtCreator)
set(PROJECT_FILES README.md LICENSE)
add_custom_target(_dummy SOURCES ${PROJECT_FILES})

# Additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake/modules)

# Print configuration ---------------------------------------------------------
include(Utils)
message(STATUS)
message(STATUS "${CMAKE_PROJECT_NAME} - Configuration:")
status_message("Build type" CMAKE_BUILD_TYPE)
message(STATUS)
