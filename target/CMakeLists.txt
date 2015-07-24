# Disable language features that cannot be easily fulfilled on AVR
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")

# Enable floating point format in printf
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -uvfprintf -lprintf_flt")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(HEADERS
)

set(SOURCES
    main.cpp
)

set(OTHERS
    target.dox
    arduino/arduino.dox
    atdemo/atdemo.dox
)

if(TARGET_CONFIGURATION STREQUAL "arduino")
    add_definitions(-DF_CPU=16000000UL)
    add_definitions(-DVCC=5000U)
elseif(TARGET_CONFIGURATION STREQUAL "atdemo")
    add_definitions(-DF_CPU=7372800UL)
    add_definitions(-DVCC=5000U)
endif()

# WIZnet Ethernet configuration (W5100)
add_definitions(-D_WIZCHIP_=5100)

add_subdirectory(lib)

add_avr_executable(sensors ${HEADERS} ${SOURCES} ${OTHERS})
target_link_libraries(sensors-${AVR_MCU}.elf libsensors-${AVR_MCU}
    libtarget-${AVR_MCU})
