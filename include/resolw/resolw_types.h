/**
 * This file has no copyright assigned and is placed in the public domain
 * according to the terms of the Unlicense License: https://unlicense.org
 * The entirety of it or any part of it may be used by anyone and for any
 * purpose, commercial or noncommercial, with or without attribution.
 *
 * This file is part of the libresolw compatibility library:
 *   https://github.com/treeswift/libresolw
 * 
 * The complete libresolw library reuses a substantial amount of code from
 * the OpenBSD project and is therefore distributed under the 3-clause BSD
 * license. Refer to the LICENSE file in the project root.
 */

#ifndef _RESOLW_RESOLW_TYPES_H_
#define _RESOLW_RESOLW_TYPES_H_
#include <sys/types.h>
#include <_bsd_types.h>
#include <stdint.h>

/* Upstream OpenBSD is already getting rid of this type naming convention; we are simply following suit */
#define u_int8_t uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t

/* idiomatic */
#if defined(__cplusplus)
#define __PORTABLE_CAST(_k,_t,_v) (_k<_t>(_v))
#else
#define __PORTABLE_CAST(_k,_t,_v) ((_t) (_v))
#endif

#define __LIBC_HIDDEN__ __attribute__((visibility("hidden")))

#endif /* _RESOLW_RESOLW_TYPES_H_ */
