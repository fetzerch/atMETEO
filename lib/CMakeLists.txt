# Listed headers for IDEs even though no code is generated for the static lib
set(HEADERS
    utils.h
    demodulator.h
    bitdecoder.h
    sensor.h
)

set(SOURCES
)

set(OTHERS
    libsensors.dox
)

set(LIBSENSORS_LIBRARY libsensors)
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
    add_avr_library(libsensors ${HEADERS} ${SOURCES} ${OTHERS})
    set(LIBSENSORS_LIBRARY libsensors-${AVR_MCU})
else()
    add_library(libsensors STATIC ${HEADERS} ${SOURCES} ${OTHERS})
endif()

set_target_properties(${LIBSENSORS_LIBRARY} PROPERTIES PREFIX "")
set_target_properties(${LIBSENSORS_LIBRARY} PROPERTIES LINKER_LANGUAGE CXX)
