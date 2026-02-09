/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2026 Enemy Territory Fortress Development Team

This file is part of Enemy Territory Fortress (ETF).

ETF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ETF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ETF. If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolfenstein: Enemy Territory GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the ETF Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __Q_ENDIAN_H__
#define __Q_ENDIAN_H__

#include "q_primitives.h"

#if defined(__cplusplus)
extern "C" {
#endif

//endianness
void CopyShortSwap(void *dest, void *src);
void CopyLongSwap(void *dest, void *src);

#ifdef _MSC_VER

#include <stdlib.h>
#define ShortSwap(x) _byteswap_ushort(x)
#define LongSwap(x) _byteswap_ulong(x)
#define LongLongSwap(x) _byteswap_uint64(x)

#elif defined(__APPLE__) || defined(__APPLE_CC__)

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define ShortSwap(x) OSSwapInt16(x)
#define LongSwap(x) OSSwapInt32(x)
#define LongLongSwap(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define ShortSwap(x) BSWAP_16(x)
#define LongSwap(x) BSWAP_32(x)
#define LongLongSwap(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define ShortSwap(x) bswap16(x)
#define LongSwap(x) bswap32(x)
#define LongLongSwap(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define ShortSwap(x) swap16(x)
#define LongSwap(x) swap32(x)
#define LongLongSwap(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define ShortSwap(x) bswap16(x)
#define LongSwap(x) bswap32(x)
#define LongLongSwap(x) bswap64(x)
#endif

#elif defined(__MINGW32__)

#define ShortSwap(x) __builtin_bswap16(x)
#define LongSwap(x) __builtin_bswap32(x)
#define LongLongSwap(x) __builtin_bswap64(x)

#else

#include <byteswap.h>
#define ShortSwap(x) bswap_16(x)
#define LongSwap(x) bswap_32(x)
#define LongLongSwap(x) bswap_64(x)

#endif

float FloatSwap(float f);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
