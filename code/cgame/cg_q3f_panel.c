/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2018 Enemy Territory Fortress Development Team

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

/*
**	cg_q3f_panel.c
**
**	Flat panel rendering.
*/

#include "cg_q3f_panel.h"

typedef struct {
	int id;
	panelRenderFunc_t *renderFunc;
} panelEntType_t;

panelData_t panel;


/******************************************************************************
***** Shared rendering code.
****/

static vec3_t panelTeamRGBs[] = {
	{ 0.5f,	0.5f, 0.5f },		// Q3F_TEAM_FREE
	{ 1.0f,	0.3f, 0.3f },		// Q3F_TEAM_RED
	{ 0.3f,	0.3f, 1.0f },		// Q3F_TEAM_BLUE
	{ 1.0f,	1.0f, 0.3f },		// Q3F_TEAM_YELLOW
	{ 0.3f,	1.0f, 0.3f },		// Q3F_TEAM_GREEN
	{ 0.5f,	0.5f, 0.5f }		// Q3F_TEAM_SPECTATOR
};
static char *panelTeamNames[] = {
	"Free",			// Q3F_TEAM_FREE
	"Red",			// Q3F_TEAM_RED
	"Blue",			// Q3F_TEAM_BLUE
	"Yellow",		// Q3F_TEAM_YELLOW
	"Green",		// Q3F_TEAM_GREEN
	"Spectator"		// Q3F_TEAM_SPECTATOR
};
static char panelTeamCodes[] = {
	COLOR_WHITE, COLOR_RED, COLOR_BLUE, COLOR_YELLOW, COLOR_GREEN, COLOR_WHITE
};
void CG_Q3F_PanelCalculateTeamColour( int team, vec4_t colour )
{
	// Multiply the specified colour with that of the specified team. Set all colour fields to 1.0 for no conversion.

	float *teamColour = panelTeamRGBs[team];

	colour[0] = teamColour[0] * colour[0];
	colour[1] = teamColour[1] * colour[1];
	colour[2] = teamColour[2] * colour[2];
	colour[3] = 1.0f * colour[3];
	//colour[3] = teamColour[3] * colour[3];
}

enum {
	TOPLEFT = 0,
	TOPRIGHT,
	BOTTOMRIGHT,
	BOTTOMLEFT
} corners_t;
void CG_Q3F_PanelPrepareCoords( float offset )
{
	// Prepare a set of mapping coordinates at the specified offset from the surface.

	vec3_t viewoff;
	float scale;

	if( offset == panel.surfaceoffset )
		return;

	panel.surfaceoffset = offset;
	VectorSubtract( cg.refdef.vieworg, panel.origin, viewoff );
	scale = VectorNormalize( viewoff );
	scale = 1.0f - (offset / scale);

	VectorMA( panel.origin,		offset,					viewoff,		panel.corners[0]	);
	VectorMA( panel.corners[0],	-0.5 * panel.width * scale,		panel.right,	panel.corners[0]	);
	VectorMA( panel.corners[0],	0.5 * panel.height * scale,		panel.up,		panel.corners[0]	);	// Top-left
	VectorMA( panel.corners[0],	panel.width * scale,			panel.right,	panel.corners[1]	);	// Top-right
	VectorMA( panel.corners[1],	-panel.height * scale,			panel.up,		panel.corners[2]	);	// Bottom-right
	VectorMA( panel.corners[2],	-panel.width * scale,			panel.right,	panel.corners[3]	);	// Bottom-left

	VectorScale( panel.right,	scale * panel.width / 640.0f,	panel.xscalecoord );
	VectorScale( panel.up,		scale * panel.height / -480.0f,	panel.yscalecoord );
}

static void CG_Q3F_PanelCoordMap( float x, float y, vec3_t out )
{
	// Map the specifed point in panel 'space' to a 3d coordinate.

	VectorMA( panel.corners[0],	x, panel.xscalecoord, out );		// Shift right.
	VectorMA( out,				y, panel.yscalecoord, out );		// Shift down.
}

void CG_Q3F_PanelDrawPoly(	float x, float y, float w, float h,
							float sx, float ty, float sw, float th,
							vec4_t rgba, qhandle_t shader, qboolean clip )
{
	// Draw a panel-mapped polygon.

	float w2, h2;
	polyVert_t verts[5];
	unsigned char crgba[4];

	if( clip )
	{
		// Check the rectangle fits into the clipping area.

		w2 = w;
		h2 = h;

		if( x < panel.clipxmin )
		{
			sx = (panel.clipxmin - x) / w;
			w -= panel.clipxmin - x;
			x = panel.clipxmin;
		}
		if( y < panel.clipymin )
		{
			ty = (panel.clipymin - y) / h;
			h -= panel.clipymin - y;
			y = panel.clipymin;
		}
		if( x + w > panel.clipxmax )
			w = panel.clipxmax - x;
		if( y + h > panel.clipymax )
			h = panel.clipymax - y;

		sw = sw * w / w2;
		th = th * h / h2;
	}

	CG_Q3F_PanelCoordMap(	x,		y + h,	verts[0].xyz );
	CG_Q3F_PanelCoordMap(	x,		y,		verts[1].xyz );
	CG_Q3F_PanelCoordMap(	x + w,	y,		verts[2].xyz );
	CG_Q3F_PanelCoordMap(	x + w,	y + h,	verts[3].xyz );

	verts[0].st[0] = sx;
	verts[0].st[1] = ty + th;
	verts[1].st[0] = sx;
	verts[1].st[1] = ty;
	verts[2].st[0] = sx + sw;
	verts[2].st[1] = ty;
	verts[3].st[0] = sx + sw;
	verts[3].st[1] = ty + th;

	if( rgba )
	{
		crgba[0] = (unsigned char) (255.0f * rgba[0]);
		crgba[1] = (unsigned char) (255.0f * rgba[1]);
		crgba[2] = (unsigned char) (255.0f * rgba[2]);
		crgba[3] = (unsigned char) (255.0f * rgba[3]);
	}
	else crgba[0] = crgba[1] = crgba[2] = crgba[3] = 255;

	*((int *) verts[0].modulate) = *(int *) crgba;
	*((int *) verts[1].modulate) = *(int *) crgba;
	*((int *) verts[2].modulate) = *(int *) crgba;
	*((int *) verts[3].modulate) = *(int *) crgba;

	verts[4] = verts[0];

	panel.polyCount += 2;	// Two triangles in a polygon
	trap_R_AddPolyToScene( shader, 5, verts );
}

void CG_Q3F_PanelFill( int shader, float offset )
{
	// Fill the specified panel layer with the specified panel.

	CG_Q3F_PanelPrepareCoords( offset );
	CG_Q3F_PanelDrawPoly( 0, 0, 640, 480, 0, 0, 1, 1, NULL, shader, qfalse );
}

void CG_Q3F_PanelDrawShader( float x, float y, float width, float height, vec4_t rgba, qhandle_t shader, qboolean clip )
{
	// Draw a shader to the panel.

	CG_Q3F_PanelDrawPoly( x, y, width, height, 0, 0, 1, 1, rgba, shader, clip );
}

void CG_Q3F_PanelDrawChar( float x, float y, float width, float height, int ch, vec4_t rgba, qboolean clip )
{
	// Draw a letter to the panel.

	int row, col;
	float frow, fcol;
	float size;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	CG_Q3F_PanelDrawPoly( x, y, width, height, fcol, frow, size, size, rgba, cgs.media.charsetShader, clip );
}

void CG_Q3F_PanelDrawString( char *str, float x, float y, float size, float maxx, float maxy, int flags, vec4_t rgba )
{
	// Draw a string to the panel.

	float currx, curry, maxwidth, maxheight, lastspacewidth;
	int index, lastspace, numLines;
	char *wordendptr;
	float width;
	vec4_t realrgba;
	vec_t *colour;
	qboolean broken;

	if( maxx <= x )
		maxx = 640;
	if( maxy <= y )
		maxy = 480;
	maxwidth	= maxx - x;
	maxheight	= maxy - y;

	panel.clipxmin = x;
	panel.clipxmax = maxx;
	panel.clipymin = y;
	panel.clipymax = maxy;

	curry = y;
	VectorCopy4( rgba, realrgba );
	numLines = 1;
	while( *str )
	{
			// Start by getting a line of text.
		for(	index = 0, lastspace = -1, width = 0, wordendptr = NULL, broken = qfalse;
				*str && index < sizeof(panel.buff) - 1; index++ )
		{
			panel.buff[index] = *str++;
			if( !(panel.buff[index] == Q_COLOR_ESCAPE ||
				(index && panel.buff[index - 1] == Q_COLOR_ESCAPE)) )
			{
				width += size;
				if( panel.buff[index] == ' ' )
				{
					lastspace = index;
					wordendptr = str;
					lastspacewidth = width;
				}
			}
			if( width > maxwidth )
			{
				width -= size;
				str--;
				broken = qtrue;
				break;
			}
		}
		panel.buff[index] = 0;

			// Break on the last space, if available.
		if( broken && (flags & PANEL_STR_SPACEBREAK) && lastspace >= 0 )
		{
			if( wordendptr )
				str = wordendptr;
			while( *str == ' ' ) str++;
			width = lastspacewidth > maxwidth ? maxwidth : lastspacewidth;
			while( panel.buff[lastspace] == ' ' )
			{
				width -= size;
				panel.buff[lastspace--] = 0;
			}
//			panel.buff[lastnonspace + 1] = 0;
		}

			// Position the text
		switch( flags & PANEL_STR_JUSTIFY )
		{
			default:
			case PANEL_STR_LEFT:	currx = x;								break;
			case PANEL_STR_CENTER:	currx = x + 0.5 * (maxwidth - width);	break;
			case PANEL_STR_RIGHT:	currx = maxx - width;					break;
		}

			// Render the string.
		for( index = 0; curry < maxy && panel.buff[index]; index++ )
		{
			if( panel.buff[index] == Q_COLOR_ESCAPE && (!index || panel.buff[index - 1] != Q_COLOR_ESCAPE) )
				continue;
			else if( index && panel.buff[index - 1] == Q_COLOR_ESCAPE )
			{
				if( !(flags & PANEL_STR_COLOUR) )
					continue;
				colour = g_color_table[ColorIndex( panel.buff[index] )];
				VectorCopy( colour, realrgba );
			}
			else {
				CG_Q3F_PanelDrawChar( currx, curry, size, size, panel.buff[index], realrgba, (curry + size > maxy) || (currx + size > maxx) );
				currx += size;
			}
		}

		if( !(flags & PANEL_STR_WRAP) )		// Prevent wrapping if the line was too long.
			break;
		curry += size * panel.aspect;
		numLines++;
	}
}

void CG_Q3F_PanelFitString( int *numlines, float *size, char *str, int flags, float w, float h )
{
	// Work out what size a string would have to be to fit into the specified width and height.

	qboolean wrap, spaceBreak, proportional;
	float wordWidths[64];
	int wordWidthIndex, wordCount, index, len, linesUsed;
	char *ptr, *wordstartptr;
	float width, height, lines, invaspect;

	wrap			= flags & PANEL_STR_WRAP;
	spaceBreak		= qtrue;	//flags & PANEL_STR_SPACEBREAK;		// Break on spaces wherever possible anyway.
	proportional	= qfalse;	// flags & PANEL_STR_PROPORTIONAL;	// Don't know how to handle this... yet.

	len = strlen( str );
	for( wordWidthIndex = 0, wordstartptr = ptr = str, width = 0, wordCount = index = 0; index < len; )
	{
		if( *ptr == Q_COLOR_ESCAPE )
		{
			index++;
			ptr++;
		}
		else if( *ptr == ' ' )
		{
			if( width )
			{
				wordWidths[wordWidthIndex++] = width;
				if( wordWidthIndex >= (sizeof(wordWidths)/sizeof(float)) )
					break;
				width = 0;
			}
		}
		else width += proportional ? 1.0f : 1.0f;
		index++;
		ptr++;
	}
	if( width && wordWidthIndex < (sizeof(wordWidths)/sizeof(float)) )
		wordWidths[wordWidthIndex++] = width;
	wordCount = wordWidthIndex;

	// Now we have the word widths, attempt to find the best fit.
	// Attempts to find the biggest size where (square) letters will all fit.
	// This could be somewhat cleverer (i.e. try to find the optimal break points, 
	// and start with more than one line), but it does the job.
	if( wrap )
	{
		for( invaspect = 1 / panel.aspect, lines = 1, height = h; lines < 64; lines++ )
		{
			height = h / lines;
			width = 0;
			linesUsed = 1;

			for(	wordWidthIndex = 0;
					wordWidthIndex < wordCount && linesUsed < lines;
					wordWidthIndex++ )
			{
					// Add to width, along with a 'space' width if it's not the first word on the line.
				width += ((width ? 1.0f : 0) + wordWidths[wordWidthIndex] * height) * invaspect;
				if( width > w )
				{
					linesUsed++;
					width = 0;
					wordWidthIndex--;
				}
			}
			if( linesUsed < lines )
				break;
		}

		*numlines = linesUsed;
		*size = height * invaspect;
	}
	else {
		// A 'no wrap' version - fit the whole lot onto a single line.

		*numlines = 1;
		for( width = wordWidthIndex = 0; wordWidthIndex < wordCount; wordWidthIndex++ )
			width += (width ? 1.0f : 0) + wordWidths[wordWidthIndex];
		*size = width > 0 ? (w / width) : w;
	}
}


/*
**	The message parsing function. When given a string, activator, and queryer,
**	it will fill in all the expected fields approprately, e.g. %H %L.
*/

static void ms_itoa( int value, char *buffptr )
{
	// Convert integer to ascii (assumes buff is big enough for an int)

	char *startptr;
	int index;
	char temp;

	if( value < 0 )
	{
		*buffptr++ = '-';
		value = -value;
	}
	startptr = buffptr;
	if( !value )
		*buffptr++ = '0';		// Force a zero

	while( value )
	{
		*buffptr++ = '0' + (value % 10);
		value /= 10;
	}
	*buffptr = 0;
	for( index = 0; index < ((buffptr - startptr)>>1); index++ )
	{
		// Swap all the digits round
		temp = *(startptr + index);
		*(startptr  + index) = *(buffptr - index - 1);
		*(buffptr - index - 1) = temp;
	}
}

void CG_Q3F_MessageString( char *srcptr, clientInfo_t *activator, clientInfo_t *queryent, int colour, char *buff, int buffsize )
{
	// A cut-down version of the qagame function adapted to run on cgame.

	char *buffptr, *buffendptr;//, *loc;
	char curr;
	clientInfo_t *current;
	char minibuff[64];
//	bg_q3f_playerclass_t *cls;
	int colourstack[32], colourstacksize;

	if( colour < 0 || colour > 31 )
	{
		colour = ColorIndex( colour );
		if( colour < 0 || colour > 31 )
			colour = 7;
	}

	colourstack[0] = colour;
	colourstacksize = 0;

	for( buffptr = buff, buffendptr = buff + buffsize - 1; *srcptr && buffptr < buffendptr; )
	{
		if( *srcptr == '%' || *srcptr == '$' )
		{
			curr = *(srcptr+1) | 32;
			current = (*(srcptr+1) & 32) ? queryent : activator;	// If it's upper or lower case
			srcptr += 2;
			switch( curr )	// Get lowercase letter;
			{
				case 'h':	// Health
							if( current )
							{
								ms_itoa( current->health, minibuff );
								Q_strncpyz( buffptr, minibuff, buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
							}
							break;
				case 'a':	// Armour
							if( current )
							{
								ms_itoa( current->armor, minibuff );
								Q_strncpyz( buffptr, minibuff, buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
							}
							break;
/*				case 'l':	// Location
							if( current )
							{
								loc = CG_Q3F_GetLocation( current->location, qtrue );
								Q_strncpyz( buffptr, loc ? loc : "Unknown Location", buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
							}
							break;*/
				case 'n':	// Name
							if( current )
							{
								Q_strncpyz( buffptr, current->name, buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
							}
							break;
				case 't':	// Team
							if( current )
							{
								Q_strncpyz( buffptr, panelTeamNames[current->team], buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
							}
				case 'c':	// Team colour
							if( current )
							{
								*buffptr += '^';
								*buffptr += panelTeamCodes[current->team];
							}
				default:	// Another letter - leave it as-is, so it can be processed with va()
							*buffptr++ = *(srcptr - 2);
							*buffptr++ = *(srcptr - 1);
			}
		}
		else {
			*buffptr++ = *srcptr++;
			if( Q_IsColorStringPtr( srcptr - 1 ) )
			{
				if( *srcptr < '0' || *srcptr > 'O' )
				{
					// A cancel, take the last one from the stack
					if( colourstacksize > 0 )
						colourstacksize--;
					colour = colourstack[colourstacksize];
					*buffptr++ = '0' + colour;
					srcptr++;

				}
				else {
					// A new colour, add it to the stack
					colour = ColorIndex( *srcptr );
					*buffptr++ = *srcptr++;
					if( colourstacksize < 7 )
						colourstack[++colourstacksize] = colour;
				}
			}
		}
	}
	*buffptr = 0;	// Terminate buffer
}



/******************************************************************************
***** Transitions (between on and off).
****/

typedef enum {
	PTT_OFFPRE = 0,
	PTT_OFFPOST,
	PTT_ONPRE,
	PTT_ONPOST
} panelTransitionType_t;

static int CG_Q3F_TransitionFuncNone( int mode, int msec )
{
	// Dummy function, performs no transitions.

	panel.transrgba[0] = panel.transrgba[1] = panel.transrgba[2] = panel.transrgba[3] = 1.0f;
	return( mode == PTT_ONPRE ? 1 : 0 );
}

static int CG_Q3F_TransitionFuncFade( int mode, int msec )
{
	// Basic fade in-out function, simply sets RBGA value.

	panel.transrgba[0] = panel.transrgba[1] = panel.transrgba[2] = 1.0f;
	if( mode == PTT_OFFPRE )
		panel.transrgba[3] = 1.0f - msec / 1000.0f;
	else if( mode == PTT_ONPRE )
		panel.transrgba[3] = msec / 1000.0;
	else panel.transrgba[3] = 1.0f;

	return( 1 );
}

static panelTransition_t panelTransitions[] = {
	{ Q3F_PANELTRANS_NONE, 500,		500,	&CG_Q3F_TransitionFuncNone },
	{ Q3F_PANELTRANS_FADE, 1000,	1000,	&CG_Q3F_TransitionFuncFade },
};

//#if (sizeof(panelTransitions) / sizeof(panelTransition_t)) != Q3F_NUM_PANELTRANSITIONS
//	#error Panel transitions table is incorrect.
//#endif

/******************************************************************************
***** Per-panel rendering.
****/

static int CG_Q3F_PanelFuncName()
{
	// Draw the client's name.

	int y;

	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );
	y = ((cg.time / 10) % 800) - 400;
	if( y < 0 )
		y = -y;
	CG_Q3F_PanelDrawString(	cgs.clientinfo[cg.clientNum].name, 0, y, 80, 0, 0,
							PANEL_STR_COLOUR|PANEL_STR_RIGHT|PANEL_STR_WRAP|PANEL_STR_SPACEBREAK, panel.transrgba );
//	CG_Q3F_PanelDrawString(	"^1Peter ^2Piper ^3picked ^4a ^5peck ^6of ^7pickled ^1peppers, ^2so ^3where's ^4the ^5peck ^6of ^7pickled ^1peppers ^2Peter ^3Piper ^4picked?",
//							0, y, 40, 0, 0, PANEL_STR_COLOUR|PANEL_STR_CENTER|PANEL_STR_WRAP|PANEL_STR_SPACEBREAK, panel.transrgba );
	return( 0 );
}

static int CG_Q3F_PanelFuncScoreSummary()
{
	// Draw a score summary.

	int index, numteams,min, max, score;
	float axis, y, left, right, range, height, unitsize;
	vec4_t colour;

	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );
	CG_Q3F_PanelDrawString( "Scores", 64, 64, 64, 576, 128, PANEL_STR_CENTER, panel.transrgba );

		// Calculate team numbers, and min/max values
	for( index = Q3F_TEAM_RED, numteams = min = max = 0; index < Q3F_TEAM_NUM_TEAMS; index++ )
	{
		if( !(cgs.teams & (1 << index)) )
			continue;
		score = cg.teamScores[index - Q3F_TEAM_RED];
		if( score < min )
			min = score;
		if( score > max )
			max = score;
		numteams++;
	}
	if( !numteams )
		return( 0 );

	// Some geometry values
	range = max - min;
	if( range )
	{
		unitsize = range ? (512.0f / (float) range) : 10;
		axis = 64 - unitsize * min;
	}
	else {
		unitsize = 10;
		axis = 302;
	}
	height = 256.0f / (float) numteams;

	// Draw the score bars.
	for( index = Q3F_TEAM_RED, y = 160; index < Q3F_TEAM_NUM_TEAMS; index++ )
	{
		if( !(cgs.teams & (1 << index)) )
			continue;
		score = cg.teamScores[index - Q3F_TEAM_RED];
		if( score < 0 )
		{
			left = axis + unitsize * (float) score;
			right = axis;
		}
		else if( score > 0 )
		{
			left	= axis;
			right	= axis + unitsize * (float) score;
		}
		else {
			left	= axis - 2;
			right	= axis + 2;
		}
		VectorCopy4( panel.transrgba, colour );
		CG_Q3F_PanelCalculateTeamColour( index, colour );
		CG_Q3F_PanelDrawShader( left, y, right - left, height, colour, cgs.media.whiteShader, qfalse );

		y += height;
	}

		// Draw interesting stats. We have a minimum of 64 units in which to draw text.
	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN2 );
	for( index = Q3F_TEAM_RED, y = 160; index < Q3F_TEAM_NUM_TEAMS; index++ )
	{
		if( !(cgs.teams & (1 << index)) )
			continue;
		score = cg.teamScores[index - Q3F_TEAM_RED];
		CG_Q3F_PanelDrawString( va( "%d", score ), 64, y + (height - 64) * 0.5, 48, 512, y + height, PANEL_STR_CENTER, panel.transrgba );
		y += height;
	}

	return( 1 );
}

static int CG_Q3F_PanelFuncLocation()
{
	// Render a panel holding the current location.

//	CG_Q3F_PanelFill( trap_R_RegisterShader( "panel/blackback" ), PANEL_LAYER_MAIN );
//	return( 1 );

	centity_t *cent = (centity_t *) panel.data;
	char *locStr;
	vec3_t locOrigin;
	int numLines;
	float size, yoff;
	vec4_t colour;

	locStr = (char *) cent->muzzleFlashTime;
	numLines = cent->teleportFlag;
	size = cent->beamEnd[0];

	if( !locStr || cent->miscTime < cg.time )
	{
		VectorMA( panel.origin, 1, panel.forward, locOrigin );
		if( !(locStr = CG_Q3F_GetLocation( locOrigin, qtrue )) )
			locStr = "This location waiting for a competent mapper.";
		cent->miscTime = cg.time + 500 + Q_flrand(0.0f, 1.0f) * 100;	// Wait a while before updating again.
		CG_Q3F_PanelFitString(	&numLines, &size,
								locStr, PANEL_STR_WRAP,
								640 - 2*cent->currentState.angles2[0], 480 - 2*cent->currentState.angles2[1] );
		cent->teleportFlag = numLines;
		cent->beamEnd[0] = size;
		cent->muzzleFlashTime = (int) locStr;
	}
	yoff = 480 - 2*cent->currentState.angles2[1];
	yoff = cent->currentState.angles2[1] + 0.5 * (yoff - numLines * size * panel.aspect);
	colour[0] = panel.transrgba[0] * ((float) ((cent->currentState.time2 >> 16) & 0xFF)) / 255.0f;
	colour[1] = panel.transrgba[1] * ((float) ((cent->currentState.time2 >> 8) & 0xFF)) / 255.0f;
	colour[2] = panel.transrgba[2] * ((float) (cent->currentState.time2 & 0xFF)) / 255.0f;
	colour[3] = panel.transrgba[3];

	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );

	CG_Q3F_PanelDrawString( locStr, cent->currentState.angles2[0], yoff, size,
							640 - cent->currentState.angles2[0], 480 - cent->currentState.angles2[1],
							PANEL_STR_CENTER|PANEL_STR_WRAP|PANEL_STR_COLOUR|PANEL_STR_SPACEBREAK,
							colour );

	return( 1 );
}

static int panelTimerCalcTime, panelTimerRTC;
static int CG_Q3F_PanelFuncTimer()
{
	// Draw a clock or timer.

	centity_t *cent = (centity_t *) panel.data;
	char *str, *seperator;
	int msec;
	float size;
	qtime_t qtime;
	vec4_t colour;

	if( panelTimerCalcTime <= cg.time )
	{
		// Get the real-time clock again.

		panelTimerCalcTime = cg.time + 1000;
		trap_RealTime( &qtime );
		panelTimerRTC = 10000 * qtime.tm_hour +
						100 * qtime.tm_min +
						qtime.tm_sec;
	}

	switch( cent->currentState.otherEntityNum )
	{
		case 1:	// Game time elapsed.
				msec = cg.time - cgs.levelStartTime;
				str = va( "%02d%s%02d", msec / 60000, (msec % 1000) >= 500 ? ":" : " ", (msec / 1000) % 60 );
				size = 5;
				break;
		case 2:	// Game time elapsed, with milliseconds.
				msec = cg.time - cgs.levelStartTime;
				str = va( "%02d:%02d.%03d", msec / 60000, (msec / 1000) % 60, msec % 1000 );
				size = 9;
				break;
		case 3:	// Game time remaining.
				if( (msec = cgs.levelStartTime + cgs.timelimit * 60000 - cg.time) < 0 )
					msec = 0;
				str = (cgs.timelimit || (panelTimerCalcTime - cg.time) > 500)
						? va( "%02d%s%02d", msec / 60000, (msec % 1000) >= 500 ? ":" : " ", (msec / 1000) % 60 )
						: "  :  ";
				size = 5;
				break;
		case 4:	// Game time remaining, with milliseconds.
				if( (msec = cgs.levelStartTime + cgs.timelimit * 60000 - cg.time) < 0 )
					msec = 0;
				str = (cgs.timelimit || (panelTimerCalcTime - cg.time) > 500)
						? va(	"%02d:%02d.%03d", msec / 60000, (msec / 1000) % 60, msec % 1000 )
						: "  :  .   ";
				size = 9;
				break;
		case 5:	// Realtime clock, 12-hour.
				seperator = (panelTimerCalcTime - cg.time) > 500 ? ":" : " ";
				str = va(	"%02d%s%02d%s%02d", (msec = ((int) (panelTimerRTC * 0.0001)) % 12) ? msec : 12,
							seperator, ((int)(panelTimerRTC * 0.01)) % 100, seperator, panelTimerRTC % 100 );
				size = 8;
				break;
		case 6:	// Realtime clock, 24-hour.
				seperator = (panelTimerCalcTime - cg.time) > 500 ? ":" : " ";
				str = va(	"%02d%s%02d%s%02d", (int) (panelTimerRTC * 0.0001),
							seperator, ((int)(panelTimerRTC * 0.01)) % 100, seperator, panelTimerRTC % 100 );
				size = 8;
				break;
		case 7:	// Realtime clock, 12-hour AM/PM.
				seperator = (panelTimerCalcTime - cg.time) > 500 ? ":" : " ";
				str = va(	"%02d%s%02d%s%02d %s", (msec = ((int) (panelTimerRTC * 0.0001)) % 12) ? msec : 12,
							seperator, ((int)(panelTimerRTC * 0.01)) % 100, seperator, panelTimerRTC % 100,
							panelTimerRTC * 0.0001 <= 11 ? "AM" : "PM" );
				size = 11;
				break;
		default:	// No timer type.
				str = "";
				size = 1;
				break;
	}

	colour[0] = panel.transrgba[0] * ((float) ((cent->currentState.time2 >> 16) & 0xFF)) / 255.0f;
	colour[1] = panel.transrgba[1] * ((float) ((cent->currentState.time2 >> 8) & 0xFF)) / 255.0f;
	colour[2] = panel.transrgba[2] * ((float) (cent->currentState.time2 & 0xFF)) / 255.0f;
	colour[3] = panel.transrgba[3];

	size = 640.0f / size;
	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );
	CG_Q3F_PanelDrawString( str, 0, 0.5 * (480 - size * panel.aspect), size, 640, 480, PANEL_STR_CENTER, colour );

	return( 1 );
}


static qboolean GG_Q3F_GetRadarBlip( int blipIndex, entityState_t *es, float *angle, float *range, float *intensity, int *team, int *height )
{
	// Extract a 'radar blip' from the entity structure.
	// each blip contains: angle (8 bits) range (8 bits), intensity (3 bits), colour (3 bits), height (2 bits)
	// The blips _have_ to match on the game side, for obvious reasons.
	// constantLight/32, generic1/32, powerups/32, modelindex/8, modelindex2/8, frame/8

	unsigned char encoded[3];

	switch( blipIndex )
	{
		case 0:		encoded[0] = ((unsigned char *) &es->constantLight)[0];
					encoded[1] = ((unsigned char *) &es->constantLight)[1];
					encoded[2] = ((unsigned char *) &es->constantLight)[2];
					break;
		case 1:		encoded[0] = ((unsigned char *) &es->constantLight)[3];
					encoded[1] = ((unsigned char *) &es->generic1)[0];
					encoded[2] = ((unsigned char *) &es->generic1)[1];
					break;
		case 2:		encoded[0] = ((unsigned char *) &es->generic1)[2];
					encoded[1] = ((unsigned char *) &es->generic1)[3];
					encoded[2] = ((unsigned char *) &es->powerups)[0];
					break;
		case 3:		encoded[0] = ((unsigned char *) &es->powerups)[1];
					encoded[1] = ((unsigned char *) &es->powerups)[2];
					encoded[2] = ((unsigned char *) &es->powerups)[3];
					break;
		case 4:		encoded[0] = ((unsigned char *) &es->modelindex)[0];
					encoded[1] = ((unsigned char *) &es->modelindex2)[0];
					encoded[2] = ((unsigned char *) &es->frame)[0];
					break;
		default:	return( qfalse );
	}

	*angle		= ((float) encoded[0]) / 255.0;
	*range		= ((float) encoded[1]) / 255.0;
	*intensity	= ((float)(encoded[2] & 7)) / 7.0;
	*team		= (encoded[2] >> 3) & 3;
	*height		= (encoded[2] >> 6) & 3;

	return( qtrue );
}
extern localEntity_t	cg_activeLocalEntities;		// double linked list
static int CG_Q3F_PanelFuncRadar()
{
	// Display 'blips' on the panel.

	centity_t *cent = (centity_t *) panel.data;
	localEntity_t *le;
	float sweepFront, range, angle, blipAngle, blipRange, blipIntensity;
	int index, blipTeam, blipHeight;
	qboolean sweepCalculated;
//	vec3_t blipOrg;

	if( cent->currentState.time && cent->currentState.time != cent->miscTime )
	{
		cent->miscTime = cent->currentState.time;

		for( index = 0, sweepCalculated = qfalse; qtrue; index++ )
		{
			if( !GG_Q3F_GetRadarBlip(	index, &cent->currentState, &blipAngle, &blipRange,
										&blipIntensity, &blipTeam, &blipHeight ) )
				break;
			if( !blipIntensity )
				continue;
			if( !sweepCalculated )
			{
				sweepFront	= ((float) cg.time) / cent->currentState.angles2[1];
				sweepFront	= (sweepFront - floor(sweepFront)) * 2*M_PI;
				range		= cent->currentState.angles2[0];
				angle		= ((float) 100) / cent->currentState.angles2[1];
				angle		= (angle - floor(angle)) * 2*M_PI;
				sweepCalculated = qtrue;
			}
			blipAngle = sweepFront - angle * blipAngle;

			le = CG_AllocLocalEntity( 0 );
			le->leType = LE_Q3F_PANELRADARBLIP;
			le->pos.trBase[0] = 320 + 312 * blipRange * sin( blipAngle );	// We cut 8 from each to stop dots right at the edge.
			le->pos.trBase[1] = 240 + 232 * blipRange * cos( blipAngle );
			le->pos.trBase[2] = blipHeight;
			if( cent->currentState.time2 )
			{
				// Colour override.

				le->color[0] = cent->currentState.time2 & 0xFF;
				le->color[1] = (cent->currentState.time2 >> 8) & 0xFF;
				le->color[2] = (cent->currentState.time2 >> 16) & 0xFF;
				le->color[3] = 1.0f;
			}
			else {
				// Use team colour.

				le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0f;
				CG_Q3F_PanelCalculateTeamColour( blipTeam, le->color );
			}
			le->bounceFactor = cent->currentState.number;
			le->endTime = cg.time + blipIntensity * cent->currentState.angles2[1];
			le->startTime = le->endTime - cent->currentState.angles2[1];
			le->lifeRate = 1.0f / cent->currentState.angles2[1];
		}
	}

	// Draw all blips currently on-panel.
	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );
	panel.clipxmin = panel.clipymin = 0;
	panel.clipxmax = 640;
	panel.clipymax = 480;
	for( le = cg_activeLocalEntities.next; le != &cg_activeLocalEntities; le = le->next )
	{
		if( le->leType == LE_Q3F_PANELRADARBLIP && le->bounceFactor == cent->currentState.number )
		{
			// This is an appropriate blip, let's render.

			blipIntensity = 1 - (le->endTime - cg.time) * le->lifeRate;
			le->color[3] = 1 - blipIntensity * blipIntensity;
			CG_Q3F_PanelDrawShader( le->pos.trBase[0] - 4, le->pos.trBase[1] - 4, 8, 8, le->color, cgs.media.whiteShader, qtrue );
		}
	}

	return( 1 );
}

static int CG_Q3F_PanelFuncMessage()
{
	// Attempts to find the state message, fills in parameters, and centers on the panel.

	centity_t *cent = (centity_t *) panel.data;
	size_t index;
	int numLines;
	float size, yoff;
	char buff[2048];
	char **statemessages;

	for( index = 0; index < cgs.numEntityData; index++ )
	{
		if( cgs.entityIndex[index] == cent->currentState.powerups )
			break;
	}
	if( index >= cgs.numEntityData )
		return( 0 );
	statemessages = cgs.entityData[index];
	if( !statemessages || !statemessages[cent->currentState.time] )
		return( 0 );

	CG_Q3F_MessageString(	statemessages[cent->currentState.time],
							(cent->currentState.otherEntityNum < MAX_CLIENTS && (cgs.clientinfo[cent->currentState.otherEntityNum].infoValid)
								? &cgs.clientinfo[cent->currentState.otherEntityNum] : NULL),
							&cgs.clientinfo[cg.clientNum], COLOR_WHITE, buff, 2000 );
	CG_Q3F_PanelFitString(	&numLines, &size,
							buff, PANEL_STR_WRAP,
							640, 480 );
	yoff = 0.5 * (480 - numLines * size * panel.aspect);
	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );
	CG_Q3F_PanelDrawString( buff, 0, yoff, size, 640, 480,
							PANEL_STR_CENTER|PANEL_STR_WRAP|PANEL_STR_COLOUR|PANEL_STR_SPACEBREAK,
							panel.transrgba );

	return( 1 );
}


/******************************************************************************
***** The main entry point(s).
****/

// trailTime		- Time panel was last active.
// dustTrailTime	- Time panel was last inactive.

qhandle_t portal;
qboolean CG_Q3F_RenderPanel(	qhandle_t backshader, qhandle_t foreshader, int transitionid,
								vec3_t origin, vec3_t angles, float width, float height, float maxdist,
								panelRenderFunc_t *renderFunc, void *renderData,
								int activetime, int inactivetime, qboolean clientsided, qboolean twosided )
{
	// Render the panel (duh :)
	// Takes:
	//		backshader		-	The background shader to render first.
	//		foreshader		-	The foreground shader to render last.
	//		transitionid	-	A transition (active <-> inactive) effect ID.
	//		origin			-	The center of the panel.
	//		angles			-	The normal (PITCH/YAW) and rotation (ROLL) of the panel.
	//		width			-	The width of the panel.
	//		height			-	The height of the panel.
	//		maxdist			-	The distance after which the panel goes inactive (with trans).
	//		renderFunc		-	A callback to the actual panel rendering function.
	//		activetime		-	The time the panel was last active.
	//		inactivetime	-	The time the panel was last inactive.
	//		clientsided		-	Panel was drawn from a complete clientsided process (ie supplystation).
	//		twosided		-	Mirror panel normal if the client is at the back of the panel.
	// Returns:
	//		Whether the panel was considered active or not (so the caller knows to update their
	//		activetime/inactivetime values). Rendering may will still have been performed,
	//		for back/foreground shaders & transition effect, even if false is returned.

	vec3_t eyepos, tmp;
	float fovproduct, fovtemp;
	panelTransition_t *ptrans;
	qboolean inrange;
//	refEntity_t ref;

	//if( cg.rendering2ndRefDef || cg.renderingSkyPortal )
	//	return( qtrue );

	if( panel.frametime != cg.time )
	{
		panel.frametime = cg.time;
		if( cg_debugPanel.integer )
			CG_Printf( BOX_PRINT_MODE_CHAT, "PanelPolys: %d\n", panel.polyCount );
		panel.polyCount = 0;
		AngleVectors( cg.refdefViewAngles, panel.viewforward, NULL, NULL );
	}
		// Prepare the panel
	VectorCopy( angles, panel.angles );
	VectorCopy( origin, panel.origin );
	AngleVectors( angles, panel.forward, panel.right, panel.up );

		// Don't bother rendering if it's facing the wrong way.
	VectorCopy( cg.currentrefdef->vieworg, eyepos );

	if( twosided )
	{
		// If the panel is visible at all, regardless of orientation.
		// If eyepos->panelorigin . panelnormal > 0 then the player
		// is 'behind' the panel and it must therefore have it's normal reversed.

		if( !trap_R_inPVS( eyepos, origin ) )
			return( Distance( origin, eyepos ) <= maxdist );	// This is just 'out of view', so no distance transition will be required when it comes into view.
		VectorSubtract( origin, eyepos, tmp );

		fovproduct = sin( (M_PI / 360) * (cg.refdef.fov_x > cg.refdef.fov_y ? cg.refdef.fov_x : cg.refdef.fov_y) );

		if( DotProduct( tmp, panel.forward ) > 0 )
		{
			VectorNegate( panel.forward, panel.forward );
			VectorNegate( panel.right, panel.right );
		}
	}
	else {
		// If the panel is visible from a single direction. The fovproduct stuff is
		// my trying to be clever by working out if the panel origin is in the
		// 'view cone' - it will potentially fail to work if the origin is out of it
		// but part of the panel is still in. If it proves a problem, we can just change
		// it to the same check used for the two-sided panels (i.e. is the player behind
		// the plane of the panel).

		// slothy
		fovproduct = sin( (M_PI / 360) * (cg.refdef.fov_x > cg.refdef.fov_y ? cg.refdef.fov_x : cg.refdef.fov_y) );

		VectorMA( origin, 4, panel.forward, tmp );
		fovtemp = DotProduct( panel.viewforward, panel.forward );
		fovproduct = sin( (M_PI / 360) * (cg.refdef.fov_x > cg.refdef.fov_y ? cg.refdef.fov_x : cg.refdef.fov_y) );
		if( DotProduct( panel.viewforward, panel.forward ) > fovproduct )
		{
			if( !trap_R_inPVS( eyepos, tmp ) )
				return( Distance( origin, eyepos ) <= maxdist );	// This is just 'out of view', so no distance transition will be required when it comes into view.
		}
	}

	panel.width		= width;
	panel.height	= height;
	panel.aspect	= (width / height) * (480.0f/640.0f);
	panel.data = renderData;
	VectorScale( panel.right, -1, panel.right );		// The panel's right is our left, etc.
	panel.surfaceoffset = -1;
	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_BACK );
	panel.trans = ptrans = &panelTransitions[transitionid];
	panel.transrgba[0] = panel.transrgba[1] = panel.transrgba[2] = panel.transrgba[3] = 1.0f;

		// Draw background shader
	if( backshader ) {
		if( clientsided )
			CG_Q3F_PanelFill( backshader, PANEL_LAYER_BACK );
		else
			CG_Q3F_PanelFill( cgs.gameShaders[backshader], PANEL_LAYER_BACK );
	}

	if( Distance( origin, eyepos ) > maxdist )
	{
		// We're out of range - render shutdown effect if required.

		if( activetime && (cg.time - activetime) < ptrans->offTime )
		{
			// Time for the shutdown effect.

			if( ptrans->renderFunc( PTT_OFFPRE, cg.time - activetime ) )
				renderFunc();
			ptrans->renderFunc( PTT_OFFPOST, 0 );
		}
		inrange = qfalse;
	}
	else {
		// We're in range - render startup effect if required.

		if( inactivetime && (cg.time - inactivetime) < ptrans->onTime )
		{
			// Time for the startup effect.

			if( ptrans->renderFunc( PTT_ONPRE, cg.time - inactivetime ) )
				renderFunc();
			ptrans->renderFunc( PTT_ONPOST, 0 );
		}
		else {
			// Just render normally.
			renderFunc();
		}
		inrange = qtrue;
	}

		// Draw foreground shader
	if( foreshader ) {
		if( clientsided )
			CG_Q3F_PanelFill( foreshader, PANEL_LAYER_FORE );
		else
			CG_Q3F_PanelFill( cgs.gameShaders[foreshader], PANEL_LAYER_FORE );
	}

	return( inrange );
}


panelEntType_t panelEntTypes[] = {
	{ Q3F_PANELTYPE_NAME,			&CG_Q3F_PanelFuncName				},
	{ Q3F_PANELTYPE_SCORESUMMARY,	&CG_Q3F_PanelFuncScoreSummary		},
	{ Q3F_PANELTYPE_LOCATION,		&CG_Q3F_PanelFuncLocation			},
	{ Q3F_PANELTYPE_TIMER,			&CG_Q3F_PanelFuncTimer				},
	{ Q3F_PANELTYPE_RADAR,			&CG_Q3F_PanelFuncRadar				},
	{ Q3F_PANELTYPE_MESSAGE,		&CG_Q3F_PanelFuncMessage			},
};
#define PANELENTTYPETABLESIZE (sizeof(panelEntTypes) / sizeof(panelEntType_t))

void CG_Q3F_Panel( centity_t *cent )
{
	// Renders the entity based on the ET_PANEL subtype.

	panelEntType_t *ptype;
	int index;
	float distance;

	if( cg.rendering2ndRefDef || cg.renderingSkyPortal || !cg_drawPanel.integer ) {
		return;
	}

	for( index = 0, ptype = panelEntTypes; index < PANELENTTYPETABLESIZE; index++, ptype++ )
	{
		if( ptype->id == cent->currentState.groundEntityNum )
			break;
	}

	if( index >= PANELENTTYPETABLESIZE )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Unknown panel entity type %d.\n", cent->currentState.groundEntityNum );
		return;
	}

	//distance = cg_drawPanel.integer ? cent->currentState.origin2[2] : 0;
	distance = cent->currentState.origin2[2];

	if( CG_Q3F_RenderPanel(	cent->currentState.legsAnim,	// Back shader
							cent->currentState.torsoAnim,	// Fore shader
							cent->currentState.weapon,		// Transition ID
							cent->currentState.origin,		// Origin
							cent->currentState.angles,		// Normal && roll
							cent->currentState.origin2[0],	// Width
							cent->currentState.origin2[1],	// Height
							distance,						// Max distance
							ptype->renderFunc,				// Render func
							cent,							// Data passed to the render func
							cent->trailTime,				// Last active time.
							cent->dustTrailTime,			// Last inactive time.
							qfalse,
							qfalse ) )
		cent->trailTime = cg.time;
	else cent->dustTrailTime = cg.time;
}
