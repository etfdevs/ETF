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
**	cg_q3f_mapselect.c
**
**	The client side functions for selecting a map after the end of the game proper
*/

#include "cg_q3f_mapselect.h"

#define	MEDIUMCHAR_WIDTH	12		// 1.5 * SMALLCHAR_WIDTH

char mapSelectBuff[10240];
int mapSelectBuffMark;
char *mapSelectNames[ETF_MAPSELECT_SELECTCOUNT];
char *mapSelectInfos[ETF_MAPSELECT_SELECTCOUNT];
qhandle_t mapSelectLevelShots[ETF_MAPSELECT_SELECTCOUNT];
qboolean mapSelectPresent[ETF_MAPSELECT_SELECTCOUNT];
int mapSelectNum, mapSelectNextNum, mapSelectVoteNum;
int mapSelectStartTime, mapSelectEndTime, mapSelectInfoLines;
int mapSelectTally[ETF_MAPSELECT_SELECTCOUNT];

/*
**	Support functions
*/

static char *CG_Q3F_MapSelectCloneString( char *str )
{
	// Clone the string in the main buffer

	int highmark;
	char *newstr;

	highmark = sizeof(mapSelectBuff) - 1;
	newstr = &mapSelectBuff[mapSelectBuffMark];
	while( *str && mapSelectBuffMark < highmark )
		mapSelectBuff[mapSelectBuffMark++] = *str++;

	mapSelectBuff[mapSelectBuffMark++] = 0;
	if( mapSelectBuffMark > highmark )
		mapSelectBuffMark = highmark;

	return( newstr );
}


/*
**	Functions called from CG_ConfigStringModified
*/

/*void CG_Q3F_MapSelectTally( const char *cs )
{
	// Update the tally of votes

	char *ptr;
	int i, curr;

	ptr = (char *) cs;
	memset( mapSelectTally, 0, sizeof(mapSelectTally) );
	for( i = 1; i <= ETF_MAPSELECT_SELECTCOUNT && mapSelectNames[i]; i++ )
	{
		for( curr = 0; *ptr && *ptr != ' '; ptr++ )
		{
			if( *ptr >= '0' && *ptr <= '9' )
				curr = curr * 10 + *ptr - '0';
		}
		while( *ptr == ' ' )
			ptr++;
		mapSelectTally[i-1] = curr;
	}
}*/

/*void CG_Q3F_MapSelectInit( const char *cs )
{
	// We've just recieved a configstring from the server, split it up and process it

	char buff[MAX_ARENAS_TEXT], mapname[32], longname[32];
	char *ptr;//, *buffptr;//, curr;
	int len, i;
	qhandle_t levelshot;
	qboolean havemap;
	cg_q3f_mapinfo_t mapInfo[3];

	mapSelectBuffMark = 0;
	memset( mapSelectNames, 0, sizeof(mapSelectNames) );

	ptr = (char *) cs;
	for( i = 1; i <= ETF_MAPSELECT_SELECTCOUNT && *ptr; i++ )
	{
		for( len = 0; *ptr && *ptr != ' ' && len < (sizeof(mapname) - 1); len++ )
			mapname[len] = *ptr++;
		while( *ptr == ' ' )
			ptr++;
		mapname[len] = 0;

		mapInfo[0].key			= "longname";
		mapInfo[0].value		= longname;
		mapInfo[0].valueSize	= sizeof(longname);
		mapInfo[1].key			= "mapinfo";
		mapInfo[1].value		= buff;
		mapInfo[1].valueSize	= sizeof(buff);
		mapInfo[2].key			= "mapname";
		mapInfo[2].value		= mapname;
		mapInfo[2].valueSize	= sizeof(mapname);
		CG_Q3F_GetMapInfo( mapname, mapInfo, 3, 0 );

		if( !*longname )
			Q_strncpyz( longname, mapname, sizeof(longname) );
		havemap = *buff;

		// Get the levelshot, if we're not running low on memory
		levelshot = 0;
		if( trap_MemoryRemaining() > 4 * 1024 * 1024 )
			levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", mapname ) );
		if( !levelshot )
			levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap" );

			// And now we've got all the data, store it for later use
		mapSelectNames[i-1]			= CG_Q3F_MapSelectCloneString( longname );
		mapSelectInfos[i-1]			= buff ? CG_Q3F_MapSelectCloneString( buff ) : "No information on this map is available.";
		mapSelectLevelShots[i-1]	= levelshot;
		mapSelectPresent[i-1]		= havemap;
	}

	cg.mapSelectState = Q3F_MAPSELECT_SLIDEIN;
	mapSelectStartTime = cg.time;
	mapSelectEndTime = cg.time + Q3F_MAPSELECT_SLIDEINTIME;
	mapSelectNum = mapSelectNextNum = mapSelectVoteNum = -1;
}*/


/*
**	User commands
*/

qboolean CG_Q3F_MapSelectChoice( int choice )
{
	// User has pressed the key, wants to highlight this map.

	int temptime;

	if( cg.mapSelectState == Q3F_MAPSELECT_NONE )
		return( qfalse );		// Didn't process

	if( !mapSelectNames[choice-1] )
		return( qtrue );		// No map to select

	if(	(mapSelectNextNum >= 0 && mapSelectNextNum == (choice - 1)) ||
		(mapSelectNextNum < 0 && mapSelectNum == (choice - 1)) )
		return( qtrue );		// Already selected

	if( cg.mapSelectState == Q3F_MAPSELECT_READY )
	{
		// Fade out, then back in.

		mapSelectStartTime	= cg.time;
		mapSelectEndTime	= cg.time + Q3F_MAPSELECT_FADEOUTTIME;
		cg.mapSelectState		= Q3F_MAPSELECT_FADEOUT;
		mapSelectNextNum	= choice - 1;
	}
	else if( cg.mapSelectState == Q3F_MAPSELECT_FADEIN )
	{
		// Switch to fadeout without any 'jump' in the current fadein

		temptime = mapSelectStartTime;
		mapSelectStartTime	= cg.time - (mapSelectEndTime - cg.time);
		mapSelectEndTime	= cg.time - (temptime - cg.time);
		cg.mapSelectState	= Q3F_MAPSELECT_FADEOUT;
		mapSelectNextNum	= choice - 1;
	}
	else if( cg.mapSelectState == Q3F_MAPSELECT_FADEOUT )
	{
		// Just switch the map, we're already on the way out

		mapSelectNextNum	= choice - 1;
	}

	return( qtrue );
}

/*qboolean CG_Q3F_MapSelectVote()
{
	// User has decided to vote for their preferred map.

	int cmdnum;
	usercmd_t latestCmd;

	if( mapSelectNextNum < 0 && mapSelectNum < 0 || cg.mapSelectState == Q3F_MAPSELECT_NONE )
		return( qfalse );

	cmdnum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdnum, &latestCmd );
	if( !(latestCmd.buttons & BUTTON_ATTACK) )
		return( qfalse );

	mapSelectVoteNum = mapSelectNextNum >= 0 ? mapSelectNextNum : mapSelectNum;
	trap_SendClientCommand( va( "mapvote %d", mapSelectVoteNum ) );
	return( qtrue );
}*/


/*
**	Process map query from server
*/

void CG_Q3F_MapSelectRespond()
{
	// Process map query from server

	fileHandle_t fh;
	int i, numargs;
	char buff[1024], mapname[32];

	buff[0] = 0;
	for(	i = 1, numargs = trap_Argc();
			i < numargs; i++ )
	{
		trap_Argv( i, mapname, sizeof(mapname) );
		if( trap_FS_FOpenFile( va( "maps/%s.bsp", mapname ), &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			if( !*buff )
				Q_strncpyz( buff, "mapresponse ", sizeof(buff) );
			Q_strcat( buff, sizeof(buff), va( "%s ", mapname ) );
		}
	}
	if( *buff )
		trap_SendClientCommand( buff );
}


/*
**	Rendering functions
*/

// Layout has:
// 8 pixel horizontal bar at 64-71
// 112 pixel grey background (7*SMALLCHAR_HEIGHT) for map names
// 8 pixel horizontal bar at 184-191
// 8 pixel vertical bar at 120-127 (going from 192 to 479)
// 384 pixel grey background for levelshot / map info. Vertically 288, for 4:3 ratio
// 8 pixel vertical bar at 512-519 (going from 192 to 479)

/*qboolean CG_Q3F_MapSelectDraw()
{
	// Draw the map select overlay, if active

	float frac, x, y;
	int i, highlighted;
	vec4_t colour1, colour2;
	char *lineptr, *lineendptr;
	qboolean lastline;

	if( cg.mapSelectState == Q3F_MAPSELECT_NONE )
		return( qfalse );

	if( cg.mapSelectState == Q3F_MAPSELECT_SLIDEIN )
	{
		// Initial 'slide-in'

		if( mapSelectEndTime <= cg.time )
		{
			cg.mapSelectState = Q3F_MAPSELECT_FADEIN;
			mapSelectStartTime = mapSelectEndTime;
			mapSelectEndTime += Q3F_MAPSELECT_FADEINTIME;
		}
		else {
			frac = 1 - ((float) (mapSelectEndTime - cg.time)) / (float) (mapSelectEndTime - mapSelectStartTime);

				// 0 - 0.3 is the border bars sliding in
			VectorSet( colour1, 1, 1, 0.3f );
			VectorSet( colour2, 1, 1, 0.6f );
			x = (frac > 0.3) ? 1 : (frac / 0.3f);
			colour1[3] = colour2[3] = x * x;
			x = frac > 0.3 ? 640 : (640 * frac / 0.3f);
			CG_FillRect( 0, 64, x, 2, colour1 );		// Top horizontal
			CG_FillRect( 0, 67, x, 2, colour2 );
			CG_FillRect( 0, 70, x, 2, colour1 );

			CG_FillRect( 640 - x, 184, x, 2, colour1 );	// Middle horizontal
			CG_FillRect( 640 - x, 187, x, 2, colour2 );
			CG_FillRect( 640 - x, 190, x, 2, colour1 );

			x = (frac > 0.3) ? 288 : (288 * (frac / 0.3f));
			CG_FillRect( 120, 480 - x, 2, x, colour1 );	// Left vertical
			CG_FillRect( 123, 480 - x, 2, x, colour2 );
			CG_FillRect( 126, 480 - x, 2, x, colour1 );

			CG_FillRect( 512, 480 - x, 2, x, colour1 );	// Right vertical
			CG_FillRect( 515, 480 - x, 2, x, colour2 );
			CG_FillRect( 518, 480 - x, 2, x, colour1 );

			if( frac > 0.3 )
			{
				VectorSet( colour1, 0, 0, 0 );
				colour1[3] = ((frac > 0.6f) ? 0.3f : (frac - 0.3f)) / 0.5f;		// Should result in 0 to 0.6 opacity

				CG_FillRect( 0, 72, 640, 112, colour1 );
				CG_FillRect( 128, 192, 384, 288, colour1 );
			}

			if( frac > 0.6 )
			{
				VectorSet( colour1, 1, 1, 1 );
				VectorSet( colour1, 1, 1, 0 );
				for( i = 0; i < 5; i++ )
				{
					x = (frac - 0.6f - 0.03f * i) / 0.3f;
					if( x <= 0 )
						continue;
					if( x > 1 )
						x = 1;
					colour1[3] = x * (mapSelectPresent[i] ? 1 : 0.5f);
					colour2[3] = x;
					if( mapSelectNames[i] && i < ETF_MAPSELECT_SELECTCOUNT )
					{
						CG_DrawStringExt(	320 - SMALLCHAR_WIDTH * (4 + CG_DrawStrlen( mapSelectNames[i] )),
											88 + SMALLCHAR_HEIGHT * i, mapSelectNames[i],
											colour1, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
						CG_DrawStringExt(	320 - SMALLCHAR_WIDTH * 3,
											88 + SMALLCHAR_HEIGHT * i, va( ".%i", i + 1 ),
											colour2, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
					}
					colour1[3] = x * (mapSelectPresent[9-i] ? 1 : 0.5f);
					if( mapSelectNames[9-i] && (9-i) < ETF_MAPSELECT_SELECTCOUNT )
					{
						CG_DrawStringExt(	320 + SMALLCHAR_WIDTH * 4,
											8 + SMALLCHAR_HEIGHT * (9-i), mapSelectNames[9-i],
											colour1, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
						CG_DrawStringExt(	320 + SMALLCHAR_WIDTH,
											8 + SMALLCHAR_HEIGHT * (9-i), va( "%i.", (10 - i) % 10 ),
											colour2, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
					}
				}
			}

			lineptr = "Pick a map with 1-0, and press fire to vote.";
			x = CG_DrawStrlen( lineptr );
			colour1[0] = colour1[1] = colour1[2] = 1;
			colour1[3] = frac;
			CG_DrawStringExt(	320 - x * MEDIUMCHAR_WIDTH / 2, (64 - SMALLCHAR_HEIGHT) / 2,
								lineptr, colour1, qfalse, qtrue,
								MEDIUMCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );

			return( qtrue );		// No more rendering during this section
		}
	}

		// All the other states share the following common rendering -
	VectorSet( colour1, 1, 1, 0.3f );
	VectorSet( colour2, 1, 1, 0.6f );
	colour1[3] = colour2[3] = 1;
	CG_FillRect( 0, 64, 640, 2, colour1 );		// Top horizontal
	CG_FillRect( 0, 67, 640, 2, colour2 );
	CG_FillRect( 0, 70, 640, 2, colour1 );
	CG_FillRect( 0, 184, 640, 2, colour1 );	// Middle horizontal
	CG_FillRect( 0, 187, 640, 2, colour2 );
	CG_FillRect( 0, 190, 640, 2, colour1 );
	CG_FillRect( 120, 192, 2, 344, colour1 );	// Left vertical
	CG_FillRect( 123, 192, 2, 344, colour2 );
	CG_FillRect( 126, 192, 2, 344, colour1 );
	CG_FillRect( 512, 192, 2, 344, colour1 );	// Right vertical
	CG_FillRect( 515, 192, 2, 344, colour2 );
	CG_FillRect( 518, 192, 2, 344, colour1 );
	VectorSet( colour1, 0, 0, 0 );
	colour1[3] = 0.6f;
	CG_FillRect( 0, 72, 640, 112, colour1 );
	CG_FillRect( 128, 192, 384, 288, colour1 );

	if( cg.mapSelectState == Q3F_MAPSELECT_FADEOUT )
	{
		// An existing selection is being faded out

		if( mapSelectEndTime <= cg.time )
		{
			cg.mapSelectState = Q3F_MAPSELECT_FADEIN;
			mapSelectStartTime = mapSelectEndTime;
			mapSelectEndTime += Q3F_MAPSELECT_FADEINTIME;
			mapSelectNum = mapSelectNextNum;
			mapSelectNextNum = -1;
			cgs.mapinfowrapped[0] = 0;
		}
		else {
			frac = ((float) (mapSelectEndTime - cg.time)) / (float) (mapSelectEndTime - mapSelectStartTime);
			highlighted = mapSelectNum;
		}
	}

	if( cg.mapSelectState == Q3F_MAPSELECT_FADEIN )
	{
		// An new selection is being faded in

		if( mapSelectEndTime <= cg.time )
		{
			cg.mapSelectState = Q3F_MAPSELECT_READY;
		}
		else {
			frac = 1 - ((float) (mapSelectEndTime - cg.time)) / (float) (mapSelectEndTime - mapSelectStartTime);
			highlighted = mapSelectNum;

			if( highlighted >= 0 && !cgs.mapinfowrapped[0] )
			{
				// Wrap the mapinfo to fit
				mapSelectInfoLines = CG_Q3F_WrapText(	cgs.mapinfowrapped,
														mapSelectInfos[highlighted],
														cgs.mapinfowrapwidth = 46,
														sizeof(cgs.mapinfowrapped) );
			}
		}
	}

	if( cg.mapSelectState == Q3F_MAPSELECT_READY )
	{
		frac = 1;
		highlighted = mapSelectNum;
	}

		// Draw map names
	for( i = 0; i < 5; i++ )
	{
		VectorSet( colour1, 1, 1, 1 );
		VectorSet( colour1, 1, 1, 0 );
		colour1[3] = 1;
		colour2[3] = 1;

		if( mapSelectNames[i] && i < ETF_MAPSELECT_SELECTCOUNT )
		{
			colour1[0] = (i == highlighted) ? 1 - frac : 1;
			colour1[3] = mapSelectPresent[i] ? 1 : 0.5;
//			if( i != mapSelectVoteNum || !(cg.time & 512) )
				CG_DrawStringExt(	320 - SMALLCHAR_WIDTH * (4 + CG_DrawStrlen( mapSelectNames[i] )),
									88 + SMALLCHAR_HEIGHT * i, mapSelectNames[i],
									colour1, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			CG_DrawStringExt(	320 - SMALLCHAR_WIDTH * 3,
								88 + SMALLCHAR_HEIGHT * i, va( ".%i", i + 1 ),
								colour2, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			if( mapSelectTally[i]) 
			{
				lineptr = va( "%i", mapSelectTally[i] );
				CG_DrawStringExt(	SMALLCHAR_WIDTH, 88 + SMALLCHAR_HEIGHT * i,
									lineptr, colour1, qtrue, qtrue,
									SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			}
			if( mapSelectVoteNum == i )
			{
				CG_DrawStringExt(	SMALLCHAR_WIDTH * 4, 88 + SMALLCHAR_HEIGHT * i,
									"Vote", colour2, qtrue, qtrue,
									SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			}
		}
		if( mapSelectNames[9-i] && (9-i) < ETF_MAPSELECT_SELECTCOUNT )
		{
			colour1[0] = (9-i == highlighted) ? 1 - frac : 1;
			colour1[3] = mapSelectPresent[9-i] ? 1 : 0.5;
//			if( (9-i) != mapSelectVoteNum || !(cg.time & 512) )
				CG_DrawStringExt(	320 + SMALLCHAR_WIDTH * 4,
									8 + SMALLCHAR_HEIGHT * (9-i), mapSelectNames[9-i],
									colour1, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			CG_DrawStringExt(	320 + SMALLCHAR_WIDTH,
								8 + SMALLCHAR_HEIGHT * (9-i), va( "%i.", (10 - i) % 10 ),
								colour2, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			if( mapSelectTally[9-i] ) 
			{
				lineptr = va( "%i", mapSelectTally[9-i] );
				CG_DrawStringExt(	640 - SMALLCHAR_WIDTH * (1 + CG_DrawStrlen( lineptr )),
									8 + SMALLCHAR_HEIGHT * (9-i),
									lineptr, colour1, qtrue, qtrue,
									SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			}
			if( mapSelectVoteNum == 9 - i )
			{
				CG_DrawStringExt(	640 - SMALLCHAR_WIDTH * 8, 8 + SMALLCHAR_HEIGHT * (9-i),
									"Vote", colour2, qtrue, qtrue,
									SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
			}
		}
	}

		// Draw levelshot
	x = sin( 0.5 * M_PI * frac );
	if( x > 0 && highlighted >= 0 )
	{
		CG_DrawPic( 128, 480 - 288*x, 384, 288, mapSelectLevelShots[highlighted] );
		VectorSet( colour1, 0, 0, 0 );
		colour1[3] = 0.3f;
		CG_FillRect( 128, 480-288*x, 384, 288, colour1 );	// Darken image by 30%
	}

		// Draw map info
	lineptr = cgs.mapinfowrapped;
	colour1[0] = colour1[1] = colour1[2] = 1.0;
	colour1[3] = frac;
	y = 288 - mapSelectInfoLines * (float) SMALLCHAR_HEIGHT  / 3;
	while( *lineptr )
	{
		lineendptr = lineptr;
		while( *lineendptr && *lineendptr != '\n' )
			lineendptr++;
		lastline = *lineendptr != '\n';

		x = 320 - ((SMALLCHAR_WIDTH * (lineendptr - lineptr)) >> 1);	// Find where to draw from
		if( x < SMALLCHAR_WIDTH )
			x = SMALLCHAR_WIDTH;
		*lineendptr = 0;		// Insert terminator for CG_DrawStringExt();
		CG_DrawStringExt( x, y, lineptr, colour1, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, (640/SMALLCHAR_WIDTH) );
		if( lastline )
			break;
		*lineendptr = '\n';		// Restore for next time we draw the string
		lineptr = lineendptr + 1;
		y += SMALLCHAR_HEIGHT;
	}

	lineptr = "Pick a map with 1-0, and press fire to vote.";
	x = CG_DrawStrlen( lineptr );
	colour1[0] = colour1[1] = colour1[2] = colour1[3] = 1;
	CG_DrawStringExt(	320 - x * MEDIUMCHAR_WIDTH / 2, (64 - SMALLCHAR_HEIGHT) / 2,
						lineptr, colour1, qfalse, qtrue,
						MEDIUMCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );

	return( qtrue );
}*/
