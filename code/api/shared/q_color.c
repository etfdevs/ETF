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

#include "q_color.h"

#include <string.h>

vec4_t		colorBlack		=	{0, 0, 0, 1};
vec4_t		colorRed		=	{1, 0, 0, 1};
vec4_t		colorGreen		=	{0, 1, 0, 1};
vec4_t		colorBlue		=	{0, 0, 1, 1};
vec4_t		colorYellow		=	{1, 1, 0, 1};
vec4_t		colorOrange		=	{1, 0.5, 0, 1};
vec4_t		colorMagenta	=	{1, 0, 1, 1};
vec4_t		colorCyan		=	{0, 1, 1, 1};
vec4_t		colorWhite		=	{1, 1, 1, 1};
vec4_t		colorLtGrey		=	{0.75, 0.75, 0.75, 1};
vec4_t		colorMdGrey		=	{0.5, 0.5, 0.5, 1};
vec4_t		colorDkGrey		=	{0.25, 0.25, 0.25, 1};
vec4_t		colorMdRed		=	{0.5, 0, 0, 1};
vec4_t		colorMdGreen	=	{0, 0.5, 0, 1};
vec4_t		colorDkGreen	=	{0, 0.20, 0, 1};
vec4_t		colorMdCyan		=	{0, 0.5, 0.5, 1};
vec4_t		colorMdYellow	=	{0.5, 0.5, 0, 1};
vec4_t		colorMdOrange	=	{0.5, 0.25, 0, 1};
vec4_t		colorMdBlue		=	{0, 0, 0.5, 1};
vec4_t		colorLtBlue	= {0.367f, 0.261f, 0.722f, 1};
vec4_t		colorDkBlue	= {0.199f, 0.0f,   0.398f, 1};

vec4_t g_color_table[Q_COLOR_BITS+1] = {
		{ 0.0,	0.0,	0.0,	1.0 },	// 0 - black		0
		{ 1.0,	0.0,	0.0,	1.0 },	// 1 - red			1
		{ 0.0,	1.0,	0.0,	1.0 },	// 2 - green		2
		{ 1.0,	1.0,	0.0,	1.0 },	// 3 - yellow		3
		{ 0.0,	0.0,	1.0,	1.0 },	// 4 - blue			4
		{ 0.0,	1.0,	1.0,	1.0 },	// 5 - cyan			5
		{ 1.0,	0.0,	1.0,	1.0 },	// 6 - purple		6
		{ 1.0,	1.0,	1.0,	1.0 },	// 7 - white		7
		{ 1.0,	0.5,	0.0,	1.0 },	// 8 - orange		8
		{ 0.5,	0.5,	0.5,	1.0 },	// 9 - md.grey		9
		{ 0.75,	0.75,	0.75,	1.0 },	// : - lt.grey		10		// lt grey for names
		{ 0.75, 0.75,	0.75,	1.0 },	// ; - lt.grey		11
		{ 0.0,	0.5,	0.0,	1.0 },	// < - md.green		12
		{ 0.5,	0.5,	0.0,	1.0 },	// = - md.yellow	13
		{ 0.0,	0.0,	0.5,	1.0 },	// > - md.blue		14
		{ 0.5,	0.0,	0.0,	1.0 },	// ? - md.red		15
		{ 0.5,	0.25,	0.0,	1.0 },	// @ - md.orange	16
		{ 1.0,	0.6f,	0.1f,	1.0 },	// A - lt.orange	17
		{ 0.0,	0.5,	0.5,	1.0 },	// B - md.cyan		18
		{ 0.5,	0.0,	0.5,	1.0 },	// C - md.purple	19
		{ 0.0,	0.5,	1.0,	1.0 },	// D				20
		{ 0.5,	0.0,	1.0,	1.0 },	// E				21
		{ 0.2f,	0.6f,	0.8f,	1.0 },	// F				22
		{ 0.8f,	1.0,	0.8f,	1.0 },	// G				23
		{ 0.0,	0.4,	0.2f,	1.0 },	// H				24
		{ 1.0,	0.0,	0.2f,	1.0 },	// I				25
		{ 0.7f,	0.1f,	0.1f,	1.0 },	// J				26
		{ 0.6f,	0.2f,	0.0,	1.0 },	// K				27
		{ 0.8f,	0.6f,	0.2f,	1.0 },	// L				28
		{ 0.6f,	0.6f,	0.2f,	1.0 },	// M				29
		{ 1.0,	1.0,	0.75,	1.0 },	// N				30
		{ 1.0,	1.0,	0.5,	1.0 },	// O				31
};

qboolean GetColourFromHex( const char *string, vec4_t colour ) {
	char hexvalue[2], *ptr;
	char *digits = "0123456789ABCDEF\0";
	int i;
	char *a, *b;

	VectorSet4( colour, 1, 1, 1, 1 );

	if( strlen(string) != 8 )
		return( qfalse );	// bad string
	else if( *string == '0' && *(string+1) == 'x' ) {
		for( i = 0, ptr = (char*)(string + 2); *ptr && *(ptr + 1) && i <= 3; ptr+=2, i++ ) {
			hexvalue[0] = *ptr;
			hexvalue[1] = *(ptr + 1);

			a = strchr(digits,toupper(hexvalue[0]));
			b = strchr(digits,toupper(hexvalue[1]));

			if( a && b ) {
				colour[i] = ((a - digits) * 16) + (b - digits);
			}
		}
	} else {
		return( qfalse );
	}
	return( qtrue );
}

qboolean GetColourFromString( const char *string, vec4_t colour ) {
	if( !Q_stricmp( string, "white" ) ) {
		VectorCopy4( colorWhite, colour );
	} else if( !Q_stricmp( string, "black" ) ) {
		VectorCopy4( colorBlack, colour );
	} else if( !Q_stricmp( string, "red" ) ) {
		VectorCopy4( colorRed, colour );
	} else if( !Q_stricmp( string, "green" ) ) {
		VectorCopy4( colorGreen, colour );
	} else if( !Q_stricmp( string, "blue" ) ) {
		VectorCopy4( colorBlue, colour );
	} else if( !Q_stricmp( string, "yellow" ) ) {
		VectorCopy4( colorYellow, colour );
	} else if( !Q_stricmp( string, "magenta" ) ) {
		VectorCopy4( colorMagenta, colour );
	} else if( !Q_stricmp( string, "cyan" ) ) {
		VectorCopy4( colorCyan, colour );
	} else if( !Q_stricmp( string, "gray" ) ) {
		VectorCopy4( colorMdGrey, colour );
	} else {
		VectorSet4( colour, 1, 1, 1, 1 );
		return( qfalse );
	}
	return( qtrue );
}

unsigned ColorBytes3 (float r, float g, float b) {
	unsigned i;

	( (byte *)&i )[0] = (byte)(r * 255);
	( (byte *)&i )[1] = (byte)(g * 255);
	( (byte *)&i )[2] = (byte)(b * 255);

	return i;
}

unsigned ColorBytes4 (float r, float g, float b, float a) {
	unsigned i;

	( (byte *)&i )[0] = (byte)(r * 255);
	( (byte *)&i )[1] = (byte)(g * 255);
	( (byte *)&i )[2] = (byte)(b * 255);
	( (byte *)&i )[3] = (byte)(a * 255);

	return i;
}

float NormalizeColor( const vec3_t in, vec3_t out ) {
	float	max;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}
