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
)

add_avr_executable(sensors ${HEADERS} ${SOURCES} ${OTHERS})
