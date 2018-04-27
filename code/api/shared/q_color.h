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

#include <string.h>
#include <ctype.h>
#include "q_string.h"
#include "q_math.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

// Hex Color string support
#define gethex(ch) ((ch) > '9' ? ((ch) >= 'a' ? ((ch) - 'a' + 10) : ((ch) - '7')): ((ch) - '0'))
#define ishex(ch)  ((ch) && (((ch) >= '0' && (ch) <= '9' ) || ((ch) >= 'A' && (ch) <= 'F' ) || ((ch) >= 'a' && (ch) <= 'f' )))
// check if it's format rrggbb r,g,b e {0..9} U {A...F}
#define Q_IsHexColorString(p) (ishex(*(p)) && ishex(*((p)+1)) && ishex(*((p)+2)) && ishex(*((p)+3)) && ishex(*((p)+4)) && ishex(*((p)+5)))
#define Q_HexColorStringHasAlpha(p) (ishex(*((p)+6)) && ishex(*((p)+7)))


#define Q_COLOR_ESCAPE	'^'
#define Q_COLOR_BITS 0x1F

#define Q_IsColorStringPtr(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )
#define Q_IsColorString(p)	( *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define COLOR_ORANGE	'8'
#define COLOR_MDGREY	'9'
#define COLOR_LTGREY	':'
//#define COLOR_LTGREY	';'
#define COLOR_MDGREEN	'<'
#define COLOR_MDYELLOW	'='
#define COLOR_MDBLUE	'>'
#define COLOR_MDRED		'?'
#define COLOR_LTORANGE	'A'
#define COLOR_MDCYAN	'B'
#define COLOR_MDPURPLE	'C'
#define COLOR_NULL		'*'
#define ColorIndex(c)	( ( (c) - '0' ) & Q_COLOR_BITS )


#define S_COLOR_BLACK		"^0"
#define S_COLOR_RED			"^1"
#define S_COLOR_GREEN		"^2"
#define S_COLOR_YELLOW		"^3"
#define S_COLOR_BLUE		"^4"
#define S_COLOR_CYAN		"^5"
#define S_COLOR_MAGENTA		"^6"
#define S_COLOR_WHITE		"^7"
#define S_COLOR_ORANGE		"^8"
#define S_COLOR_MDGREY		"^9"
#define S_COLOR_LTGREY		"^:"
//#define S_COLOR_LTGREY		"^;"
#define S_COLOR_MDGREEN		"^<"
#define S_COLOR_MDYELLOW	"^="
#define S_COLOR_MDBLUE		"^>"
#define S_COLOR_MDRED		"^?"
#define S_COLOR_LTORANGE	"^A"
#define S_COLOR_MDCYAN		"^B"
#define S_COLOR_MDPURPLE	"^C"
#define S_COLOR_NULL		"^*"


extern vec4_t colorBlack;
extern vec4_t colorRed;
extern vec4_t colorGreen;
extern vec4_t colorBlue;
extern vec4_t colorYellow;
extern vec4_t colorOrange;
extern vec4_t colorMagenta;
extern vec4_t colorCyan;
extern vec4_t colorWhite;
extern vec4_t colorLtGrey;
extern vec4_t colorMdGrey;
extern vec4_t colorDkGrey;
extern vec4_t colorMdRed;
extern vec4_t colorMdGreen;
extern vec4_t colorDkGreen;
extern vec4_t colorMdCyan;
extern vec4_t colorMdYellow;
extern vec4_t colorMdOrange;
extern vec4_t colorMdBlue;

extern vec4_t clrBrown;
extern vec4_t clrBrownDk;
extern vec4_t clrBrownLine;
extern vec4_t clrBrownText;
extern vec4_t clrBrownTextDk;
extern vec4_t clrBrownTextDk2;
extern vec4_t clrBrownTextLt;
extern vec4_t clrBrownTextLt2;
extern vec4_t clrBrownLineFull;

extern vec4_t g_color_table[Q_COLOR_BITS+1];

qboolean GetColourFromHex( const char *string, vec4_t colour );
qboolean GetColourFromString( const char *string, vec4_t colour );
unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);
float NormalizeColor( const vec3_t in, vec3_t out );

#if defined(__cplusplus)
} // extern "C"
#endif
