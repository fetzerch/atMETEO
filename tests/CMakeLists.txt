add_subdirectory(catch2)
include(catch2/contrib/Catch.cmake)

# Code coverage target for all tests
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    find_package(PythonInterp REQUIRED)
    find_program(GCOVR_EXECUTABLE NAMES gcovr)

    add_custom_target(gcovr_to_cobertura
        COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            -e ${CMAKE_SOURCE_DIR}/tests
            -x -o coverage.xml -s
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting gcov coverage data to cobertura format with gcovr")

    add_custom_target(gcovr_to_html
        COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            -e ${CMAKE_SOURCE_DIR}/tests
            --html --html-details -o coverage.html -s
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting gcov coverage data to html format with gcovr")
endif()

set(HEADERS
    utils.h
)

set(SOURCES
    test_utils.cpp
    test_demodulator.cpp
    test_bitdecoder.cpp
    test_hidekisensor.cpp
    test_hidekidevice.cpp
    test_tgs2600.cpp
)

set(OTHERS
    test.dox
)

set(CATCH_MAIN_FILE ${CMAKE_BINARY_DIR}/catch-main-test.cpp)
file(WRITE ${CATCH_MAIN_FILE} "#include \"catch.hpp\"\n")
set_source_files_properties(${CATCH_MAIN_FILE} PROPERTIES
    COMPILE_DEFINITIONS CATCH_CONFIG_MAIN
    GENERATED TRUE
)

add_executable(tests ${HEADERS} ${SOURCES} ${OTHERS} ${CATCH_MAIN_FILE})
target_link_libraries(tests PRIVATE libsensors Catch2::Catch)
catch_discover_tests(tests)
