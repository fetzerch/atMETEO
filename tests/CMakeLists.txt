find_package(GMock 1.7.0 REQUIRED)

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
    add_test(NAME ${_NAME} COMMAND ${_NAME})
    set_property(TEST ${_NAME}
        PROPERTY ENVIRONMENT "GTEST_OUTPUT=xml:${_NAME}.xml")
endfunction()
