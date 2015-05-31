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
 * \defgroup libtarget_atomic Atomic execution
 * \ingroup libtarget
 *
 * \brief Utilities for ensuring atomic / non atomic execution of code blocks.
 *
 * This module contains RAII-style utilities that ensure atomic / non atomic
 * execution of scoped blocks by controlling the global interrupt state
 * (in the SREG register).
 *
 * Usage:
 * \code
 * int main() {
 *     // ...
 *     {
 *         Avr::AtomicGuard<Avr::AtomicRestoreState> atomicGuard;
 *         // Interrupts are guaranteed to be disabled
 *     }
 *     // ...
 * }
 * \endcode
 *
 * \sa http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html
 */

/*!
 * \file
 * \ingroup libtarget_atomic
 * \copydoc libtarget_atomic
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include <avr/io.h>

namespace Avr
{

/*!
 * \addtogroup libtarget_atomic
 * \{
 */

/*!
 * \brief RAII-style wrapper to disable interrupts for the duration of a
 *        scoped block.
 *
 * \tparam TAtomicMode Controls if interrupts should be globally enabled
 *         (AtomicForceOn) or if the global interrupt state should
 *         be restored (AtomicRestoreState) when leaving the scoped block.
 */
template<class TAtomicMode>
class AtomicGuard : private TAtomicMode
{
};

/*!
 * \brief Configuration parameter for AtomicGuard that unconditionally enables
 *        interrupts on exit.
 *
 * An AtomicGuard with AtomicForceOn avoids saving the SREG status and hence
 * saves flash space. A safer alternative is AtomicRestoreState which will save
 * and restore the interrupt state.
 */
class AtomicForceOn
{
protected:
    AtomicForceOn() { cli(); }
    ~AtomicForceOn() { sei(); }
};

/*!
 * \brief Configuration parameter for AtomicGuard that saves the global
 *        interrupt state and restores it on exit.
 */
class AtomicRestoreState
{
protected:
    AtomicRestoreState() : m_SREG(SREG) { cli(); }
    ~AtomicRestoreState() { SREG = m_SREG; }

private:
    const uint8_t m_SREG;
};

/*!
 * \brief RAII-style wrapper to enable interrupts for the duration of a
 *        scoped block.
 *
 * NonAtomicGuard is especially useful within an block where interrupts are
 * globally disabled using AtomicGuard.
 *
 * \tparam TNonAtomicMode Controls if interrupts should be globally
 *         disabled (NonAtomicForceOff) or if the global interrupt state should
 *         be restored (NonAtomicRestoreState) when leaving the scoped block.
 */
template<class TNonAtomicMode>
class NonAtomicGuard : private TNonAtomicMode
{
};

/*!
 * \brief Configuration parameter for NonAtomicGuard that unconditionally
 *        disables interrupts on exit.
 *
 * A NonAtomicGuard with NonAtomicForceOff avoids saving the SREG status and
 * hence saves flash space. A safer alternative is NonAtomicRestoreState which
 * will save and restore the interrupt state.
 */
class NonAtomicForceOff
{
protected:
    NonAtomicForceOff() { sei(); }
    ~NonAtomicForceOff() { cli(); }
};

/*!
 * \brief Configuration parameter for NonAtomicGuard that saves the global
 *        interrupt state and restores it on exit.
 */
class NonAtomicRestoreState
{
protected:
    NonAtomicRestoreState() : m_SREG(SREG) { sei(); }
    ~NonAtomicRestoreState() { SREG = m_SREG; }

private:
    const uint8_t m_SREG;
};

/*! \} */  // \addtogroup libtarget_atomic

}  // namespace Avr
