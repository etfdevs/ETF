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
**	bg_q3f_flyby.c
**
**	Flyby camera system
*/

#ifdef QAGAME
#include "g_local.h"
#else
#include "../cgame/cg_local.h"
#endif

qboolean UMC_ReadLineSkipEmpty( fileHandle_t f, int *bytesread, int filelen, char *buff, int buffsize )
{
	// Read in a line, stripping off leading/trailing whitespace, and comments
	// Modified: skips empty lines

	int index = 0;
	char curr;
	qboolean prespace, forwardslash, ignore;

	if( *bytesread >= filelen )
		return( qfalse );			// We're at the end of the file

	while(1) {

		prespace = qtrue;
		forwardslash = qfalse;
		ignore = qfalse;

		for( index = 0; index < (buffsize - 1) && (*bytesread < filelen); )
		{
			trap_FS_Read( &curr, 1, f );
			(*bytesread)++;
			if( prespace && (curr == ' ' || curr == '\t') )
				continue;		// Skip leading whitespace
			prespace = qfalse;
			if( curr == 13 || curr == 10 )
				break;			// Terminate on carriage returns and newlines
			if( ignore )
				continue;		// Do no other processing for this line
			if( curr == '/' )
			{
				if( forwardslash )
				{
					ignore = qtrue;		// We've hit a comment
					index = 0;			// Set index to 0 so a new line will start
				}
				else forwardslash = qtrue;
			}
			else {
				if( forwardslash )
				{
					forwardslash = qfalse;
					buff[index++] = '/';
				}
				buff[index++] = curr;
			}
		}

		if ( index != 0 )
			break;	// We had a CRLF, and had a LF left of the last line.
	}
	if (index>0) {
		buff[index--] = 0;	// Null terminator
		while( (buff[index] == ' ' || buff[index] == '\t') && index >= 0 )
			buff[index--] = 0;	// Strip off trailing spaces
	}
	return( qtrue );	// We got something, even if it's empty
}

int BG_Q3F_GetPathIndex( char *pathname, campath_t* campaths ) {
	int pathindex;

	for ( pathindex = 0; pathindex < Q3F_MAX_PATHS; pathindex++ ) {
		if ( !strcmp( pathname, campaths[pathindex].pathname ) )
			return(pathindex);		
	}
	return(-1);
}

int BG_Q3F_LoadCamPaths( char *mapname, campath_t* campaths ) {
	int len, splineindex, pathindex, bytesread;
	fileHandle_t fh;
	spdheader_t spdheader;
	char spdname[128];
	char linebuff[MAX_STRING_CHARS];
	
	COM_StripExtension( mapname, spdname, sizeof(spdname) );
	Q_strcat( spdname, sizeof(spdname), ".spd" );
	// Com_sprintf( spdname, sizeof(spdname), "%s.spd", spdname );  Slothy - src & dest same bug

	len = trap_FS_FOpenFile( spdname, &fh, FS_READ );

	if ( len > 0 ) {
		bytesread = 0;
		memset( &spdheader, 0, sizeof(spdheader));

		//trap_FS_Read( &spdheader, sizeof(spdheader_t), fh );
		if( !UMC_ReadLineSkipEmpty( fh, &bytesread, len, linebuff, sizeof(linebuff) ) ) {
			trap_FS_FCloseFile( fh );
			Com_Printf( "^3Warning: SPD File Corrupt^7\n");
			return(0);
		}

		sscanf( linebuff, "%i %i %i", &spdheader.version, &spdheader.numCamPaths, &spdheader.numSplines );

		if( spdheader.version != SPD_IDENT ) {
			trap_FS_FCloseFile( fh );
			Com_Printf( "^3Warning: SPD File Version Mismatch^7\n");
			return(0);
		}

		for ( pathindex = 0; pathindex < spdheader.numCamPaths; pathindex++ ) {
			if( !UMC_ReadLineSkipEmpty( fh, &bytesread, len, linebuff, sizeof(linebuff) ) ) {
				trap_FS_FCloseFile( fh );
				Com_Printf( "^3Warning: SPD File Corrupt^7\n");
				return(0);
			}
			strcpy( spdheader.pathname[pathindex], linebuff );
		}

		for ( pathindex = 0; pathindex < Q3F_MAX_PATHS; pathindex++ ) {
			campaths[pathindex].numsplines = 0;
			strcpy( campaths[pathindex].pathname, spdheader.pathname[pathindex]);
		}

		for ( splineindex = 0; splineindex < spdheader.numSplines; splineindex++ ) {
			vec3_t origin, dir, lookat;
			int speed, roll;
			/*trap_FS_Read( &pathindex, sizeof(int), fh );
			trap_FS_Read( &campaths[pathindex].camsplines[splineindex].origin, sizeof(vec3_t), fh );
			trap_FS_Read( &campaths[pathindex].camsplines[splineindex].dir, sizeof(vec3_t), fh );
			trap_FS_Read( &campaths[pathindex].camsplines[splineindex].speed, sizeof(int), fh );
			trap_FS_Read( &campaths[pathindex].camsplines[splineindex].roll, sizeof(int), fh );*/
			if( !UMC_ReadLineSkipEmpty( fh, &bytesread, len, linebuff, sizeof(linebuff) ) ) {
				trap_FS_FCloseFile( fh );
				Com_Printf( "^3Warning: SPD File Corrupt^7\n");
				return(0);
			}
			sscanf( linebuff, "%i %f %f %f %f %f %f %i %i %f %f %f", &pathindex, 
															&origin[0], &origin[1], &origin[2],
															&dir[0], &dir[1], &dir[2],
															&speed,
															&roll,
															&lookat[0], &lookat[1], &lookat[2]);
			VectorCopy( origin, campaths[pathindex].camsplines[campaths[pathindex].numsplines].origin );
			VectorCopy( dir, campaths[pathindex].camsplines[campaths[pathindex].numsplines].dir );
			campaths[pathindex].camsplines[campaths[pathindex].numsplines].speed = speed;
			campaths[pathindex].camsplines[campaths[pathindex].numsplines].roll = roll;
			VectorCopy( lookat, campaths[pathindex].camsplines[campaths[pathindex].numsplines].lookat );

			campaths[pathindex].numsplines++;
		}

		trap_FS_FCloseFile( fh );

		for ( pathindex = 0; pathindex < spdheader.numCamPaths; pathindex++ ) {
			for ( splineindex = 0; splineindex < campaths[pathindex].numsplines; splineindex++ ) {
				if ( splineindex == campaths[pathindex].numsplines - 1 )
					campaths[pathindex].camsplines[splineindex].next = &campaths[pathindex].camsplines[0];
				else
					campaths[pathindex].camsplines[splineindex].next = &campaths[pathindex].camsplines[splineindex+1];

				VectorCopy(campaths[pathindex].camsplines[splineindex].origin, campaths[pathindex].splines[splineindex].ControlPoint[0]);
				VectorMA(campaths[pathindex].camsplines[splineindex].origin, campaths[pathindex].camsplines[splineindex].speed, campaths[pathindex].camsplines[splineindex].dir, campaths[pathindex].splines[splineindex].ControlPoint[1]);
				VectorMA(campaths[pathindex].camsplines[splineindex].next->origin, campaths[pathindex].camsplines[splineindex].next->speed, campaths[pathindex].camsplines[splineindex].next->dir, campaths[pathindex].splines[splineindex].ControlPoint[2]);
				MirrorVtxInVtx(campaths[pathindex].splines[splineindex].ControlPoint[2], campaths[pathindex].splines[splineindex].ControlPoint[2], campaths[pathindex].camsplines[splineindex].next->origin );
				VectorCopy(campaths[pathindex].camsplines[splineindex].next->origin, campaths[pathindex].splines[splineindex].ControlPoint[3]);
				campaths[pathindex].splines[splineindex].nSegments = 16;
				BG_Q3F_CubicSpline_ComputeSegments( &campaths[pathindex].splines[splineindex] );
			}
		}
	} else {
		return(0);
	}

	return(spdheader.numCamPaths);
}

int BG_Q3F_LocateFlybyPath( int numPaths, campath_t* campaths ) {
	int pathindex;

	for ( pathindex = 0; pathindex < numPaths; pathindex++ ) {
		if ( !strcmp( campaths[pathindex].pathname, "flyby" ) ) {
			return( pathindex );
			break;
		}
	}

	return( -1 );
}
