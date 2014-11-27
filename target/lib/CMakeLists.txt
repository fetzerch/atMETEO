# Listed headers for IDEs even though no code is generated for the static lib
set(HEADERS
    interrupt.h
    timer.h
    uart.h
)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/target/external/uart/uart.c
)

set(OTHERS
    libtarget.dox
)

set(LIBTARGET_LIBRARY libtarget)
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
    add_avr_library(libtarget ${HEADERS} ${SOURCES} ${OTHERS})
    set(LIBTARGET_LIBRARY libtarget-${AVR_MCU})
else()
    message(FATAL_ERROR "Crosscompiling required for libtarget")
endif()

set_target_properties(${LIBTARGET_LIBRARY} PROPERTIES PREFIX "")
set_target_properties(${LIBTARGET_LIBRARY} PROPERTIES LINKER_LANGUAGE CXX)
