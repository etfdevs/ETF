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
**	$Id: g_q3f_mapinfo.c,v 1.8 2002/04/17 23:05:57 rr2do2 Exp $
**
**	Parses and processes map information.
**
*/

#include "g_local.h"
#include "g_q3f_mapdata.h"

typedef struct {
	int infoHandle;
	pc_token_t token;
	qboolean errorFound;
} g_q3f_mapInfo_t;
static g_q3f_mapInfo_t mi;


/******************************************************************************
*****	Support functions
****/

static void G_Q3F_MI_Error( char *format, ... )
{
	// print an error, showing file and line.

	va_list		argptr;
	char		buff[2048], filename[1024];
	int			line;

	va_start( argptr, format );
	Q_vsnprintf( buff, sizeof(buff), format, argptr );
	va_end( argptr );

	if( mi.infoHandle )
	{
		trap_PC_SourceFileAndLine( mi.infoHandle, filename, &line );
		G_Printf( "Map Info: %s at '%s' line %d.\n", buff, filename, line );
	}
	else G_Printf( "Map Info: %s.\n", buff );
	mi.errorFound = qtrue;
}


static qboolean G_Q3F_MI_GetToken( char *requiredToken )
{
	// Get the next token

	if( !trap_PC_ReadToken( mi.infoHandle, &mi.token ) )
	{
		if( !requiredToken )
			return( qfalse );
		G_Q3F_MI_Error( "Unexpected end of file" );
		return( qfalse );
	}
	if( requiredToken && *requiredToken && Q_stricmp( mi.token.string, requiredToken ) )
	{
		G_Q3F_MI_Error( "Expected '%s', found '%s'", requiredToken, mi.token.string );
		return( qfalse );
	}
	return( qtrue );
}

static qboolean G_Q3F_MI_IsSpace( char c )
{
	return( c == ' ' || c == '\n' || c == '\t' || c == '\r' );
}

static char *matchStrings[] = {
	"-", "and", "to", NULL
};


/******************************************************************************
*****	Main functions
****/

char *G_Q3F_GetMapInfoEntry( q3f_keypairarray_t *mpi, char *key, int gameindex, char *defstr )
{
	// Find the specified mapinfo key.

	q3f_keypair_t *kp;
	char *realkey;

	if( gameindex &&
		(realkey = G_Q3F_GetString( va( "%s+%d", key, gameindex ) )) &&
		(kp = G_Q3F_KeyPairArrayFind( mpi, realkey )) )
		return( kp->value.d.strdata );

	if(	(realkey = G_Q3F_GetString( key )) &&
		(kp = G_Q3F_KeyPairArrayFind( mpi, realkey )) )
		return( kp->value.d.strdata );

	return( defstr );
}

q3f_keypairarray_t *G_Q3F_LoadMapInfo( char *mapname )
{
	// Parse the mapinfo file and generate a set of data.

	char rawmapname[1024], keyname[1024];
	char *infoname, *rawmpi, *ptr, *ptr2, *ptr3, *ptr4;
	int index, c1, c2;//, gameIndices;
	q3f_keypairarray_t *kpa;

	COM_StripExtension( COM_SkipPath( mapname ), rawmapname, sizeof(rawmapname) );
	infoname = va( "%s/%s%s", MAPINFODIR, rawmapname, MAPINFOEXT );

	memset( &mi, 0, sizeof(mi) );
	kpa = NULL;
	if( (mi.infoHandle = trap_PC_LoadSource( infoname )) )
	{
		if( G_Q3F_MI_GetToken( "mapinfo" ) &&
			G_Q3F_MI_GetToken( "{" ) )
		{
			index = 0;
			while( G_Q3F_MI_GetToken( NULL ) )
			{
				if( !Q_stricmp( mi.token.string, "gameIndexDef" ) )
				{
					// It's the start of an index definition.
					if( index )
					{
						G_Q3F_MI_Error( "Nested indexDef" );
						break;
					}
					if( !G_Q3F_MI_GetToken( "" ) )
						break;
					if( !(index = mi.token.intvalue) )
					{
						G_Q3F_MI_Error( "Invalid indexDef" );
						break;
					}
					if( !G_Q3F_MI_GetToken( "{"/*}*/ ) )
						break;
				}
				else if( !Q_stricmp( mi.token.string, /*{*/"}" ) )
				{
					// It's the end of an index definition.
					if( !index )
						break;
					index = 0;
				}
				else {
					if( index )
						Com_sprintf( keyname, sizeof(keyname), "%s+%d", mi.token.string, index );
					else Q_strncpyz( keyname, mi.token.string, sizeof(keyname) );
					Q_strlwr( keyname );
					if( !G_Q3F_MI_GetToken( "" ) )
						break;
					if( !kpa )
						kpa = G_Q3F_KeyPairArrayCreate();
					G_Q3F_KeyPairArrayAdd( kpa, keyname, Q3F_TYPE_STRING, 0, (int) mi.token.string );
					if( !G_Q3F_MI_GetToken( ";" ) )
						break;
				}
			}
		}

		trap_PC_FreeSource( mi.infoHandle );
		if( kpa && !mi.errorFound )
		{
			G_Q3F_KeyPairArraySort( kpa );
			return( kpa );
		}

		if( kpa )
			G_Q3F_KeyPairArrayDestroy( kpa );
	}

		// Attempt to load in the old mapinfo file, and parse out player limits.
	infoname = va( "%s/%s%s", MAPINFODIR, rawmapname, OLDMAPINFOEXT );
	if( (index = trap_FS_FOpenFile( infoname, &mi.infoHandle, FS_READ )) <= 0 )
	{
		if( index == 0 )
			trap_FS_FCloseFile( mi.infoHandle );
		return( NULL );
	}
	rawmpi = G_Alloc( index + 1 );
	trap_FS_Read( rawmpi, index, mi.infoHandle );
	trap_FS_FCloseFile( mi.infoHandle );

	kpa = G_Q3F_KeyPairArrayCreate();
	G_Q3F_KeyPairArrayAdd( kpa, "mapinfo", Q3F_TYPE_STRING, 0, (int) rawmpi );
	for( ptr = rawmpi; ptr && *ptr; ptr++ )
	{
		if( c1 = atoi( ptr ) )
		{
			// It's a number, let's see if we can find a marker and a second number.

			for( ptr2 = ptr; *ptr2 && ((*ptr2 >= '0' && *ptr2 <= '9') || G_Q3F_MI_IsSpace( *ptr2 )); ptr2++ );
			for( ptr3 = ptr2; *ptr3 && (*ptr3 <'0' || *ptr3 > '9') && !G_Q3F_MI_IsSpace( *ptr3 ); ptr3++ );
			for( ptr4 = ptr3; *ptr4 && G_Q3F_MI_IsSpace( *ptr4 ); ptr4++ );

			if( (c2 = atoi( ptr4 )) && c2 > c1 )
			{
				// Check the marker.

				for( ptr4 = keyname; ptr2 < ptr3 && ptr4 < keyname + sizeof(keyname) - 1; )
					*ptr4++ = *ptr2++;
				*ptr4 = 0;
				for( index = 0; matchStrings[index]; index++ )
				{
					if( !Q_stricmp( matchStrings[index], keyname ) )
					{
						Com_sprintf( keyname, sizeof(keyname), "%d", c1 );
						G_Q3F_KeyPairArrayAdd( kpa, "minplayers", Q3F_TYPE_STRING, 0, (int) keyname );
						Com_sprintf( keyname, sizeof(keyname), "%d", c2 );
						G_Q3F_KeyPairArrayAdd( kpa, "maxplayers", Q3F_TYPE_STRING, 0, (int) keyname );
						ptr = (char *) -1;
						break;
					}
				}
			}
		}
	}
	G_Free( rawmpi );
	G_Q3F_KeyPairArraySort( kpa );
	return( kpa );
}

void G_Q3F_CheckGameIndex()
{
	// Ensure the gameindex is valid, and set to 1 otherwise.

	int index, indices;
	char *ptr;

	if( level.mapInfo )
	{
		ptr = G_Q3F_GetMapInfoEntry( level.mapInfo, "gameindices", 0, "1" );
		for( indices = 0; *ptr; )
		{
			index = 0;
			while( *ptr >= '0' && *ptr <= '9' )
				index = 10 * index + *ptr++ - '0';
			if( index == g_gameindex.integer )
				return;
			while( *ptr && (*ptr < '0' || *ptr > '9') ) ptr++;
		}
	}
	trap_Cvar_Set( "g_gameindex", "1" );
	trap_Cvar_Update( &g_gameindex );
}
