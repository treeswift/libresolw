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

#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

#define _BIG_ENDIAN     4321
#define _LITTLE_ENDIAN  1234
#define _PDP_ENDIAN     3412

/* `libresolw` targets Windows and Windows is typically LE; default to LE */
#ifndef _BYTE_ORDER
#define _BYTE_ORDER _LITTLE_ENDIAN
#endif /* _BYTE_ORDER */

#ifdef _RESOLW_BYTE_ORDER_NOUNDERDEF
#define BIG_ENDIAN _BIG_ENDIAN
#define LITTLE_ENDIAN _LITTLE_ENDIAN
#define PDP_ENDIAN _PDP_ENDIAN
#define BYTE_ORDER _BYTE_ORDER
#endif /* no-underscore defines */

#endif /* _SYS_ENDIAN_H_ */