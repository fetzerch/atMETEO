/*
 * atMETEO - An ATmega based weather station
 * Copyright (C) 2014-2015 Christian Fetzer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

/*!
 * \defgroup libtarget_adc Analog to Digital Converter (ADC)
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing built-in analog to digital conversion
 *        facilities.
 */

/*!
 * \file
 * \ingroup libtarget_adc
 * \copydoc libtarget_adc
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include <avr/io.h>

#ifndef F_CPU
#error "F_CPU not defined for adc.h"
#endif

#ifndef VCC
#error "VCC not defined for adc.h"
#endif

namespace Avr
{

/*!
 * \addtogroup libtarget_adc
 * \{
 */

/*!
 * \brief A C++ wrapper for accessing the built-in 10 bit Analog to Digital
 *        Conversion (ADC) facilities.
 *
 * \note Avr::Adc is currently limited to ADC mode Single Conversion.
 * \note The prescaler is automatically configured to the lowest possible
 *       value targeting a sample rate between 50 and 200 kHz for a good
 *       compromise between performance and accuracy.
 */
class Adc
{
public:
    /*!
     * \brief Returns the Avr::Adc instance.
     *
     * \return The Avr::Adc instance.
     */
    static Adc& instance()
    {
        return s_instance;
    }

    /*!
     * \brief Reads the (average) raw ADC value (in Single Conversion mode)
     *        for the given \p channel.
     * \param channel The ADC channel to read from.
     * \param nsamples Number of samples taken to build an average from.
     * \return the raw ADC value.
     */
    uint16_t read(uint8_t channel, uint8_t nsamples = 1)
    {
        uint32_t sum = 0;
        for (uint8_t n = 0; n < nsamples; n++) {
            sum += readAdc(channel);
        }
        return (uint16_t)(sum / nsamples);
    }

    /*!
     * \brief Reads the (average) ADC voltage (in Single Conversion mode)
     *        for the given \p channel.
     * \param channel The ADC channel to read from.
     * \param nsamples Number of samples taken to build an average from.
     * \return ADC value in millivolts.
     */
    uint16_t readMilliVolts(uint8_t channel, uint8_t nsamples = 1)
    {
        return toMillivolts(read(channel, nsamples));
    }

    /*!
     * \brief Reads the (average) ADC voltage (in Single Conversion mode)
     *        for the given \p channel.
     * \param channel The ADC channel to read from.
     * \param nsamples Number of samples taken to build an average from.
     * \return ADC value in volts.
     */
    float readVolts(uint8_t channel, uint8_t nsamples = 1)
    {
        return toVolts(read(channel, nsamples));
    }

private:
    static Adc s_instance;

    Adc()
    {
        // Use Vcc as reference voltage
        ADMUX = (1 << REFS0);

        // Set ADC prescaler
        ADCSRA |= prescaler();

        // Enable ADC (single conversion)
        ADCSRA |= (1 << ADEN);

        // Dummy readout
        read(0);
    }

    uint16_t readAdc(uint8_t channel)
    {
        ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        return ADCW;
    }

    static uint16_t toMillivolts(uint16_t adc)
    {
        uint16_t result = (static_cast<uint32_t>(VCC) * adc) / 1024;
        return Sensors::min(result, VCC);
    }

    static float toVolts(uint16_t adc)
    {
        float result = (static_cast<uint32_t>(VCC) * adc) / 1024000.0F;
        return Sensors::min(result, static_cast<float>(VCC));
    }

    template <uint16_t prescaler>
    static constexpr uint8_t adcPrescaler()
    {
        static_assert(prescaler == 2 || prescaler == 4 || prescaler == 8 ||
                      prescaler == 16 || prescaler == 32 || prescaler == 64 ||
                      prescaler == 128,
                      "Invalid prescaler (2, 4, 8, 16, 32, 64, 128)");
        return prescaler ==   2 ? (1 << ADPS0) :
               prescaler ==   4 ? (1 << ADPS1) :
               prescaler ==   8 ? (1 << ADPS1) | (1 << ADPS0) :
               prescaler ==  16 ? (1 << ADPS2) :
               prescaler ==  32 ? (1 << ADPS2) | (1 << ADPS0) :
               prescaler ==  64 ? (1 << ADPS2) | (1 << ADPS1) :
               prescaler == 128 ? (1 << ADPS2) | (1 << ADPS1) |
                                  (1 << ADPS0) : 0;
    }

    template <uint32_t min, uint32_t max, uint8_t prescaler>
    static constexpr bool checkPrescaler()
    {
        return (F_CPU / prescaler) <= max &&
               (F_CPU / prescaler) >= min ? true : false;
    }

    template <uint32_t min, uint32_t max>
    static constexpr uint8_t calcPrescaler()
    {
        return checkPrescaler<min, max,   2>() ?   2 :
               checkPrescaler<min, max,   4>() ?   4 :
               checkPrescaler<min, max,   8>() ?   8 :
               checkPrescaler<min, max,  16>() ?  16 :
               checkPrescaler<min, max,  32>() ?  32 :
               checkPrescaler<min, max,  64>() ?  64 :
               checkPrescaler<min, max, 128>() ? 128 : 0;
    }

    static constexpr uint8_t prescaler()
    {
        return adcPrescaler<calcPrescaler<50000, 200000>()>();
    }
};

Adc Adc::s_instance = Adc();

/*! \} */  // \addtogroup libtarget_adc

}  // namespace Avr
