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
 * \defgroup libtarget_interrupt Interrupt handling
 * \ingroup libtarget
 *
 * \brief Utilities for interrupt service routines.
 *
 * \sa http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
 */

/*!
 * \file
 * \ingroup libtarget_interrupt
 * \copydoc libtarget_interrupt
 */

/*!
 * \addtogroup libtarget_interrupt
 * \{
 */

/*!
 * \brief Creates an interrupt service routine as static member function
 *        of a class.
 *
 * Interrupt service routines are typically introduced as free functions by
 * using the ISR macro. When programming in C++, it might be desirable to
 * create the interrupt service routine as part of a class.
 *
 * Usage:
 * \code
 * class Timer0Interrupt {
 *     CLASS_IRQ(serviceRoutine, TIMER0_COMPA_vect);
 * };
 * \endcode
 *
 * \param name Name of the new static member function.
 * \param vector MCU specific interrupt vector name.
 *
 * \sa http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
 * \sa http://lists.gnu.org/archive/html/avr-chat/2008-02/msg00001.html
 *
 * \hideinitializer
 */
#define CLASS_IRQ(name, vector) \
    static void name(void) asm(__STRINGIFY(vector)) \
    __attribute__ ((signal, __INTR_ATTRS))

/*! \} */  // \addtogroup interrupt
