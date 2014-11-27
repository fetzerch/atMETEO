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
 * \defgroup libtarget_timer Timer module
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing built-in timers facilities.
 *
 * - Common timer utilities: Avr::TimerUtils
 * - Input capture: Avr::TimerInputCapture
 */

/*!
 * \file
 * \ingroup libtarget_timer
 * \copydoc libtarget_timer
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/utils.h"

#include "interrupt.h"

// Ensure compatibility with older ATmega processors
#ifndef TIMSK1
    /// Timer/Counter Interrupt Mask Register
    #define TIMSK1 TIMSK
#endif
#ifndef TICIE1
    /// Timer 1 Input Capture Interrupt Enable
    #define TICIE1 ICIE1
#endif

#ifndef F_CPU
#error "F_CPU not defined for timer.h"
#endif

namespace Avr
{

/*!
 * \addtogroup libtarget_timer
 * \{
 */

/*!
 * \brief Compile time calculation of clock select and conversion of time
 *        (in us) to system clock ticks given the CPU frequency (F_CPU) and
 *        the \p prescaler.
 *
 * \tparam prescaler The prescaler for the timer configuration.
 */
template <uint16_t prescaler>
struct TimerUtils
{
    /*!
     * \brief Calculates the clock select value for the given \p prescaler.
     *
     * The clock select value is needed to configure the timer with the given
     * \p prescaler. Typically it has to be set in the Timer/Counter Control
     * Register (TCCR).
     *
     * Usage:
     * \code
     * TCCR1B |= Avr::TimerUtils<8>::clockSelect();
     * \endcode
     *
     * \return The clock select value needed to configure the timer.
     */
    static constexpr uint8_t clockSelect()
    {
        static_assert(prescaler == 1 || prescaler == 8 || prescaler == 64 ||
                      prescaler == 256 || prescaler == 1024,
                      "Invalid prescaler (1, 8, 64, 256, 1024)");
        return prescaler ==    1 ? (1 << CS10) :
               prescaler ==    8 ? (1 << CS11) :
               prescaler ==   64 ? (1 << CS11) | (1 << CS10) :
               prescaler ==  256 ? (1 << CS12) :
               prescaler == 1024 ? (1 << CS12) | (1 << CS10) : 0;
    }

    /*!
     * \brief Converts us to ticks for the given CPU frequency and
     *        \p prescaler.
     *
     * While time values are typically needed when writing an application,
     * the Timer/Counter Register (TCNT) counts (prescaled) system clock ticks.
     *
     * Usage:
     * \code
     * uint16_t ticks = Avr::TimerUtils<8>::usToTicks<100>()
     * \endcode
     *
     * \tparam us Time value (in us) to be converted into system clock ticks.
     * \return System clock ticks representing the time value \p us.
     */
    template <uint16_t us>
    static constexpr uint16_t usToTicks()
    {
        return ((uint64_t)F_CPU * us) / (1000000UL * prescaler);
    }
};

namespace internal
{

/*!
 * \brief Internal base class for Avr::TimerInputCapture.
 *
 * The construct with the base class is necessary because there is currently no
 * known way to introduce an interrupt service routine in a class template.
 * Avr::TimerInputCapture needs to implement an interrupt service routine for
 * the input capture interrupt in order to be functional.
 *
 * \note This class cannot be used directly.
 */
class TimerInputCaptureBase
{
    CLASS_IRQ(InputCaptureInterrupt, TIMER1_CAPT_vect);
    static TimerInputCaptureBase* s_baseInstance;

    virtual void _pulseWidthReceived(uint16_t) = 0;
};
TimerInputCaptureBase *TimerInputCaptureBase::s_baseInstance = nullptr;

void TimerInputCaptureBase::InputCaptureInterrupt()
{
    s_baseInstance->_pulseWidthReceived(TCNT1);
    TCNT1 = 0;

    // Flip edge detection
    Sensors::bitFlip(TCCR1B, ICES1);
}

}  // namespace internal

/*!
 * \brief C++ wrapper for accessing the built-in input capture facility for
 *        measuring the width of external pulses.
 *
 * Usage:
 * \code
 * struct MyTimerObserver {
 *     static void pulseWidthReceived(uint16_t pulseWidth) {
 *         // Input capture event received with measured pulse width
 *     }
 * };
 *
 * int main() {
 *     // ...
 *     Avr::TimerInputCapture<8, MyTimerObserver>::instance();
 *     sei();
 *     // ...
 * }
 * \endcode
 *
 * \tparam prescaler The prescaler for the timer configuration.
 * \tparam TObserver The observer that is notified of input capture events
 *         using its `void pulseWidthReceived(uint16_t pulseWidth)` function.
 */
template <uint16_t prescaler, class TObserver>
class TimerInputCapture
        : private internal::TimerInputCaptureBase, private TObserver
{
public:
    /*!
     * \brief Returns the Avr::TimerInputCapture instance.
     *
     * \return The Avr::TimerInputCapture instance.
     */
    static TimerInputCapture& instance()
    {
        return s_instance;
    }

private:
    static TimerInputCapture s_instance;

    TimerInputCapture()
    {
        TCCR1B |= TimerUtils<prescaler>::clockSelect();

        // Input capture edge select: start with rising edge
        Sensors::bitSet(TCCR1B, ICES1);

        // Enable input capture
        Sensors::bitSet(TIMSK1, TICIE1);

        s_baseInstance = this;
    }

    void _pulseWidthReceived(uint16_t pulseWidth)
    {
        s_instance.pulseWidthReceived(pulseWidth);
    }
};

template <uint16_t prescaler, class TObserver>
TimerInputCapture<prescaler, TObserver>
TimerInputCapture<prescaler, TObserver>::s_instance = TimerInputCapture();

/*! \} */  // \addtogroup libtarget_timer

}  // namespace Avr
