find_package(GMock 1.7.0 REQUIRED)

# Code coverage target for all tests
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    find_package(PythonInterp REQUIRED)
    find_program(GCOVR_EXECUTABLE NAMES gcovr)

    add_custom_target(gcovr_to_cobertura
        COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            -e ${CMAKE_SOURCE_DIR}/tests
            -e ".*gmock.*"
            -x -o coverage.xml -v -s
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting gcov coverage data to cobertura format with gcovr")
endif()

set(HEADERS
    utils.h
)

set(OTHERS
    test.dox
)

function(add_test_gmock _NAME)
    include_directories(${CMAKE_SOURCE_DIR} ${GMOCK_INCLUDE_DIRS})
    add_executable(${_NAME} ${ARGN} ${HEADERS} ${OTHERS})
    target_link_libraries(${_NAME} libsensors ${GMOCK_BOTH_LIBRARIES})
    add_test(
        NAME ${_NAME}
        COMMAND ${_NAME} --gtest_output=xml:${_NAME}.xml)
endfunction()

add_test_gmock(test_utils test_utils.cpp)
add_test_gmock(test_demodulator test_demodulator.cpp)
add_test_gmock(test_bitdecoder test_bitdecoder.cpp)
add_test_gmock(test_hidekisensor test_hidekisensor.cpp)
add_test_gmock(test_hidekidevice test_hidekidevice.cpp)
