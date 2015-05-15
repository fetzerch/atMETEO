/* Copyright (C) 2010 Andreas Ferber <af@chaos-agency.de>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/* $Id: avr_io_cpp.h $ */

/** \file
 * \brief AVR IO in a more C++ way
 *
 * This header file provides the same IO definitions as the C-centric
 * standard AVR headers, however it does so in a more C++ way,
 * especially making them usable as template arguments for more generic,
 * parameterizable library code.
 *
 * It does so by redefining some macros used by the standard AVR header
 * files. This way, it automatically provides all IO registers defined
 * by <avr/io.h>, without having to list them all explicitly. The
 * drawback is that it uses the undocumented fact that all IO
 * definition macros in those headers eventually lead back to the
 * _MMIO_BYTE macro (or _MMIO_WORD/_MMIO_DWORD), so this might break in
 * the future when the avr-libc headers change in this respect.
 */

#ifndef AVR_IO_CPP_H
#define AVR_IO_CPP_H 1

#include <avr/io.h>

/**
 * \brief IO register.
 *
 * This class template represents one particular IO register, as defined
 * by its type and address.
 *
 * All instances of one particular template instance represent the same
 * actual IO register. It can be used just like the original C macros
 * are used, it is however not possible to take the address of a
 * register.
 *
 * This class template should never be used directly, instead it should
 * only be used through the AVR IO macro definitions like PORTA.
 */
template<typename T, unsigned int Addr>
struct avr_ior {
	avr_ior() { }
	avr_ior(const avr_ior& v) { }

	avr_ior& operator=(T value) volatile {
		*(volatile T *)(Addr) = value;
		return *(avr_ior *)this;
	}

	operator volatile T&() const volatile { return *(volatile T *)(Addr); }

	volatile T* operator&() volatile {
		return (volatile T *)(Addr);
	}
};

/**
 * \internal
 * \brief Internal template for automatic IO register class instantiation.
 */
template<typename T, unsigned int Addr>
struct avr_ior_provider {
	static volatile avr_ior<T, Addr> ior;
};

/** \internal */
#undef _MMIO_BYTE
#define _MMIO_BYTE(mem_addr) (avr_ior_provider<uint8_t, (mem_addr)>::ior)
/** \internal */
#undef _MMIO_WORD
#define _MMIO_WORD(mem_addr) (avr_ior_provider<uint16_t, (mem_addr)>::ior)
/** \internal */
#undef _MMIO_DWORD
#define _MMIO_DWORD(mem_addr) (avr_ior_provider<uint32_t, (mem_addr)>::ior)

/**
 * \brief Macro for use of IO register as template argument.
 *
 * You can use IO registers as template arguments as shown in the
 * following example:
 *
 * \code
 * template <typename R>
 * class C {
 *         static R m_reg;
 *         void do_something() { m_reg = 0; }
 * };
 *
 * C<AVR_IOR_PARAM(PORTA)> v;
 * \endcode
 *
 * Note that the IO register is passed to the template in the form of a
 * type parameter, so a variable of this type has to be defined in order
 * to access the IO register.
 */
#define AVR_IOR_PARAM(ior) __typeof__(ior)

#endif /* !defined(AVR_IO_CPP_H) */

// vim:ft=cpp
