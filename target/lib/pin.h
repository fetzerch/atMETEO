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
 * \defgroup libtarget_pin Pin module
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing digital I/O ports.
 *
 * This module provides convenient access to digital I/O ports using the
 * following classes: Avr::InputPin, Avr::OutputPin, Avr::OutputPinInverted,
 * Avr::InputOutputPin.
 *
 * If needed, the set of predefined input / output pins can be extended using
 * the template configuration classes: Avr::OutputConfiguration,
 * Avr::OutputConfigurationInverted, Avr::OutputConfigurationDisabled,
 * Avr::InputConfiguration, Avr::InputConfigurationDisabled.
 *
 * In addition this module provides access to common digital I/O registers
 * for the usage with the classes mentioned above: Avr::DigitalIoA,
 * Avr::DigitalIoB, Avr::DigitalIoC, Avr::DigitalIoD.
 *
 * Usage:
 * \code
 * using namespace Avr;
 * InputPin<DigitalIoD, PD1> pd1;
 * OutputPin<DigitalIoD, PD2> pd2;
 * while (true) {
 *     pd2.set(pd1.isSet());
 *     _delay_ms(1000);
 * }
 * \endcode
 */

/*!
 * \file
 * \ingroup libtarget_pin
 * \copydoc libtarget_pin
 */

#include <external/avr_io_cpp.h>

#include "lib/utils.h"

using Sensors::bitSet;
using Sensors::bitClear;
using Sensors::bitFlip;
using Sensors::bitRead;
using Sensors::bitWrite;

namespace Avr
{

/*!
 * \addtogroup libtarget_pin
 * \{
 */

/*!
 * \brief Digital I/O register configuration.
 * \tparam DDR The Data Direction Register.
 * \tparam PORT The Data Register.
 * \tparam PIN The Input pins Register.
 */
template <typename DDR, typename PORT, typename PIN>
struct DigitalIo
{
protected:
    static DDR  m_DDR;
    static PORT m_PORT;
    static PIN  m_PIN;
};

#if defined(DDRA) || defined (DOXYGEN)
/*!
 * \brief Digital I/O Port A register configuration.
 */
using DigitalIoA = DigitalIo<
    AVR_IOR_PARAM(DDRA), AVR_IOR_PARAM(PORTA), AVR_IOR_PARAM(PINA)>;
#endif
#if defined(DDRB) || defined (DOXYGEN)
/*!
 * \brief Digital I/O Port B register configuration.
 */
using DigitalIoB = DigitalIo<
    AVR_IOR_PARAM(DDRB), AVR_IOR_PARAM(PORTB), AVR_IOR_PARAM(PINB)>;
#endif
#if defined(DDRC) || defined (DOXYGEN)
/*!
 * \brief Digital I/O Port C register configuration.
 */
using DigitalIoC = DigitalIo<
    AVR_IOR_PARAM(DDRC), AVR_IOR_PARAM(PORTC), AVR_IOR_PARAM(PINC)>;
#endif
#if defined(DDRD) || defined (DOXYGEN)
/*!
 * \brief Digital I/O Port D register configuration.
 */
using DigitalIoD = DigitalIo<
    AVR_IOR_PARAM(DDRD), AVR_IOR_PARAM(PORTD), AVR_IOR_PARAM(PIND)>;
#endif

/*!
 * \brief Output configuration parameter for pin (normal mode).
 * \tparam TDigitalIo Digital I/O register configuration.
 * \tparam pinNumber Output pin.
 */
template <class TDigitalIo, uint8_t pinNumber>
struct OutputConfiguration : public TDigitalIo
{
    /*!
     * \brief Turns output pin on.
     */
    void on() { bitSet(this->m_PORT, pinNumber); }

    /*!
     * \brief Turns output pin off.
     */
    void off() { bitClear(this->m_PORT, pinNumber); }

    /*!
     * \brief Sets output pin to given state.
     * \param enable Value to write into output pin.
     */
    void set(bool enable) { bitWrite(this->m_PORT, pinNumber, enable); }

    /*!
     * \brief Flips output pin.
     */
    void flip() { bitFlip(this->m_PORT, pinNumber); }
};

/*!
 * \brief Output configuration parameter for pin (for inverted output pins /
 *        active low configuration).
 * \tparam TDigitalIo Digital I/O register configuration.
 * \tparam pinNumber Output pin.
 */
template <class TDigitalIo, uint8_t pinNumber>
struct OutputConfigurationInverted : public TDigitalIo
{
    /*!
     * \brief Turns output pin on.
     */
    void on() { bitClear(this->m_PORT, pinNumber); }

    /*!
     * \brief Turns output pin off.
     */
    void off() { bitSet(this->m_PORT, pinNumber); }

    /*!
     * \brief Sets output pin to given state.
     * \param enable Value to write into output pin.
     */
    void set(bool enable) { bitWrite(this->m_PORT, pinNumber, !enable); }

    /*!
     * \brief Flips output pin.
     */
    void flip() { bitFlip(this->m_PORT, pinNumber); }
};

/*!
 * \brief Output configuration parameter for pin (disabled).
 */
template <class TDigitalIo, uint8_t pinNumber>
struct OutputConfigurationDisabled
{
};

/*!
 * \brief Input configuration parameter for pin (normal mode).
 * \tparam TDigitalIo Digital I/O register configuration.
 * \tparam pinNumber Input pin.
 */
template <class TDigitalIo, uint8_t pinNumber>
struct InputConfiguration : public TDigitalIo
{
    /*!
     * \brief Determines if the input pin is set.
     * \return `true` if the input pin is set, `false` otherwise.
     */
    bool isSet() const { return bitRead(this->m_PIN, pinNumber); }
};

/*!
 * \brief Input configuration parameter for pin (disabled).
 */
template <class TDigitalIo, uint8_t pinNumber>
struct InputConfigurationDisabled
{
};

/*!
 * \brief Output pin (normal mode).
 */
template<class TDigitalIo, uint8_t pinNumber>
struct OutputPin :
    public OutputConfiguration<TDigitalIo, pinNumber>,
    public InputConfigurationDisabled<TDigitalIo, pinNumber>
{
    OutputPin() { bitSet(this->m_DDR, pinNumber); }
};

/*!
 * \brief Inverted output pin (active low configuration).
 */
template <class TDigitalIo, uint8_t pinNumber>
struct OutputPinInverted :
    public OutputConfigurationInverted<TDigitalIo, pinNumber>,
    public InputConfigurationDisabled<TDigitalIo, pinNumber>
{
    OutputPinInverted() { bitSet(this->m_DDR, pinNumber); }
};

/*!
 * \brief Input pin.
 */
template <class TDigitalIo, uint8_t pinNumber>
struct InputPin :
    public OutputConfigurationDisabled<TDigitalIo, pinNumber>,
    public InputConfiguration<TDigitalIo, pinNumber>
{
    InputPin() { bitClear(this->m_DDR, pinNumber); }
};

/*!
 * \brief Input / Output pin (initially configured as output pin).
 */
template <class TDigitalIo, uint8_t pinNumber>
struct InputOutputPin :
    public OutputConfiguration<TDigitalIo, pinNumber>,
    public InputConfiguration<TDigitalIo, pinNumber>
{
    InputOutputPin() { setOutput(); }

    /*!
     * \brief Use pin as output pin.
     */
    void setOutput() {
        bitSet(OutputConfiguration<TDigitalIo, pinNumber>::m_DDR, pinNumber);
    }

    /*!
     * \brief Use pin as input pin.
     */
    void setInput() {
        bitClear(InputConfiguration<TDigitalIo, pinNumber>::m_DDR, pinNumber);
    }
};

/*! \} */  // \addtogroup libtarget_pin

}  // namespace Avr
