/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2026 Enemy Territory Fortress Development Team

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

#include "q_primitives.h"
#include <stddef.h> // Q_vsnprintf or Q_strnlen needs size_t

#if defined(__cplusplus)
extern "C" {
#endif

extern const byte locase[ 256 ];
extern const byte upcase[ 256 ];

int Q_isprint( int c );
int Q_isprintext( int c );
int Q_isgraph( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
int Q_isnumeric( int c );
int Q_isalphanumeric( int c );
int Q_isforfilename( int c );
qboolean Q_isanumber( const char *s );
qboolean Q_isintegral( float f );

// portable case insensitive compare
int Q_stricmp(const char *s1, const char *s2);
int	Q_strncmp(const char *s1, const char *s2, int n);
int	Q_stricmpn(const char *s1, const char *s2, int n);
char *Q_strlwr( char *s1 );
char *Q_strupr( char *s1 );

// relative path to a source file (src/foo/bar.c)
#define RELATIVE_FILENAME ((__FILE__) + (SOURCE_PATH_SIZE))

// buffer size safe library replacements
#define Q_strncpyz(dest, src, destsize) Q_strncpyz_fn(dest, src, destsize, __func__, RELATIVE_FILENAME, __LINE__)
#define Q_strcat(dest, size, src) Q_strcat_fn(dest, size, src, __func__, RELATIVE_FILENAME, __LINE__)
//#define Q_strnlen(str, strsz ) Q_strnlen_fn(str, strsz, __func__, RELATIVE_FILENAME, __LINE__)
void Q_strncpyz_fn( char *dest, const char *src, const int destsize, const char *func, const char *file, int line );
void Q_strcat_fn( char *dest, const int size, const char *src, const char *func, const char *file, int line );
//size_t Q_strnlen_fn(const char *str, size_t strsz, const char *func, const char *file, int line );

const char *Q_stristr( const char *s, const char *find);

qboolean Q_IsColorString( const char *p );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );

// removes color sequences from string
char *Q_CleanStr( char *string );
void Q_StripColor(char *text);
const char *Q_strchrs( const char *string, const char *search );

// strips whitespaces and bad characters
qboolean Q_isBadDirChar( const char c );
char *Q_CleanDirName( char *dirname );

//void Q_strstrip( char *string, const char *strip, const char *repl );

#if defined (_MSC_VER) && _MSC_VER < 1900
	// vsnprintf is ISO/IEC 9899:1999
	// abstracting this to make it portable
	int Q_vsnprintf( char *str, size_t size, const char *format, va_list args );
#else // not using MSVC old msvc
	#define Q_vsnprintf vsnprintf
#endif

float Q_atof(const char *str);
int Q_atoi(const char *str);

#if defined(__cplusplus)
} // extern "C"
#endif
