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
    atdemo/atdemo.dox
)

if(TARGET_CONFIGURATION STREQUAL "arduino")
    add_definitions(-DF_CPU=16000000UL)
elseif(TARGET_CONFIGURATION STREQUAL "atdemo")
    add_definitions(-DF_CPU=7372800UL)
endif()

add_subdirectory(lib)

add_avr_executable(sensors ${HEADERS} ${SOURCES} ${OTHERS})
