# Generate dummy.cpp so that it's possible to build the library
file(WRITE ${CMAKE_BINARY_DIR}/dummy.cpp
    "int main() {}")

set(HEADERS
)

set(SOURCES
    ${CMAKE_BINARY_DIR}/dummy.cpp
)

set(OTHERS
    target.dox
    arduino/arduino.dox
)

if(TARGET_CONFIGURATION STREQUAL "arduino")
    add_definitions(-DF_CPU=16000000UL)
endif()

add_subdirectory(lib)

add_avr_executable(sensors ${HEADERS} ${SOURCES} ${OTHERS})
