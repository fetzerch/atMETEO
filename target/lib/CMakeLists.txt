# Listed headers for IDEs even though no code is generated for the static lib
set(HEADERS
    adc.h
    interrupt.h
    timer.h
    uart.h
    pin.h
    dht22.h
    atomic.h
    i2c.h
    bmp180.h
    mlx90614.h
    spi.h
    ethernet.h
    wiznet.h

    ${CMAKE_SOURCE_DIR}/target/external/avr_io_cpp.h
    ${CMAKE_SOURCE_DIR}/target/external/i2cmaster/i2cmaster.h
)

set(SOURCES
    wiznet.cpp
    ${CMAKE_SOURCE_DIR}/target/external/uart/uart.c
    ${CMAKE_SOURCE_DIR}/target/external/i2cmaster/twimaster.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/socket.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/wizchip_conf.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/W5100/w5100.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/W5100/w5100.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/W5200/w5200.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/W5300/w5300.c
    ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet/W5500/w5500.c
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
target_include_directories(${LIBTARGET_LIBRARY}
    PUBLIC ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/target
    PRIVATE ${CMAKE_SOURCE_DIR}/target/external/i2cmaster
    PRIVATE ${CMAKE_SOURCE_DIR}/target/external/wiznet/Ethernet
)
