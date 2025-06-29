/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2018 Enemy Territory Fortress Development Team

OpenJK
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

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

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (_MSC_VER)
	#if _MSC_VER >= 1600
		#include <stdint.h>
	#else
		typedef signed __int64 int64_t;
		typedef signed __int32 int32_t;
		typedef signed __int16 int16_t;
		typedef signed __int8  int8_t;
		typedef unsigned __int64 uint64_t;
		typedef unsigned __int32 uint32_t;
		typedef unsigned __int16 uint16_t;
		typedef unsigned __int8  uint8_t;
	#endif
#else // not using MSVC
	#if !defined(__STDC_LIMIT_MACROS)
		#define __STDC_LIMIT_MACROS
	#endif
	#include <stdint.h>
#endif

typedef unsigned char byte;

typedef enum { qfalse = 0, qtrue } qboolean;

// 32 bit field aliasing
typedef union byteAlias_u {
	float f;
	int32_t i;
	uint32_t ui;
	qboolean qb;
	byte b[4];
	char c[4];
} byteAlias_t;

#if defined(__cplusplus)
} // extern "C"
#endif
