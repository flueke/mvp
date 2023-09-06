#ifndef __MESYTEC_MVLC_BITS_H__
#define __MESYTEC_MVLC_BITS_H__

#include <cassert>
#include <x86intrin.h>

#include "int_types.h"
//#include "mesytec-mvlc/util/int_types.h"

namespace mesytec::mvlc::util
{

/* bit_gather() and bit_scatter() are from the FXT library by Joerg Arndt.
 * See http://www.jjj.de/fxt/ for details on the FXT library.
 *
 * I added calls to intrinsics where available.
 */

inline u32 bit_gather(u32 w, u32 m)
// Return  word with bits of w collected as indicated by m:
// Example:
//  w = 00A0BC00
//  m = 00101100
//  ==> 00000ABC
// This is the inverse of bit_scatter()
{
#ifdef __BMI2__
    return _pext_u32(w, m);
#else
    u32 z = 0;
    u32 b = 1;
    while ( m )
    {
        u32 i = m & -m;  // lowest bit
        m ^= i;  // clear lowest bit in m
        z += (i&w ? b : 0);
        b <<= 1;
    }
    return  z;
#endif
}
// -------------------------

inline u32 bit_scatter(u32 w, u32 m)
// Return  word with bits of w distributed as indicated by m:
// Example:
//  w = 00000ABC
//  m = 00101100
//  ==> 00A0BC00
// This is the inverse of bit_gather()
{
#ifdef __BMI2__
//#warning "use pdep intrinsic here"
#endif
    u32 z = 0;
    u32 b = 1;
    while ( m )
    {
        u32 i = m & -m;  // lowest bit
        m ^= i;
        z += (b&w ? i : 0);
        b <<= 1;
    }
    return  z;
}
// -------------------------

// Source: http://stackoverflow.com/a/109025 (SWAR)
inline int number_of_set_bits(u32 i)
{
#ifdef __SSE4_2__
    return __builtin_popcountl(i);
#else
    // Java: use >>> instead of >>
    // C or C++: use uint32_t
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif
}

// Source: http://stackoverflow.com/a/757266
inline int trailing_zeroes(uint32_t v)
{
    int r;           // result goes here
    static const int MultiplyDeBruijnBitPosition[32] =
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    r = MultiplyDeBruijnBitPosition[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
    return r;
}

/* Source: https://stackoverflow.com/a/4840428 by Jonathan Leffler */
/* Align upwards - bit mask mode (hence _b) */
inline uint8_t *align_upwards_b(uint8_t *stack, uintptr_t align)
{
    assert(align > 0 && (align & (align - 1)) == 0); /* Power of 2 */
    assert(stack != 0);

    uintptr_t addr  = (uintptr_t)stack;
    addr = (addr + (align - 1)) & -align;   // Round up to align-byte boundary
    assert(addr >= (uintptr_t)stack);
    return (uint8_t *)addr;
}


// Bit rotation functions from: https://stackoverflow.com/a/776523 by Peter Cordes

inline uint32_t rotl32 (uint32_t n, unsigned int c)
{
  const unsigned int mask = (8*sizeof(n) - 1);  // assumes width is a power of 2.

  // assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;
  return (n<<c) | (n>>( (-c)&mask ));
}

inline uint32_t rotr32 (uint32_t n, unsigned int c)
{
  const unsigned int mask = (8*sizeof(n) - 1);

  // assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;
  return (n>>c) | (n<<( (-c)&mask ));
}


}

#endif /* __MESYTEC_MVLC_BITS_H__ */
