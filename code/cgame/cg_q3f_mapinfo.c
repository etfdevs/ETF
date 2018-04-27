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
**	$Id: cg_q3f_mapinfo.c,v 1.4 2002/04/17 23:06:53 rr2do2 Exp $
**
**	Parses and processes map information.
**
*/

#include "cg_local.h"
#include "cg_q3f_scriptlib.h"

typedef struct {
	int infoHandle;
	pc_token_t token;
	qboolean errorFound;
} cg_q3f_mapinternal_t;
static cg_q3f_mapinternal_t mi;



/******************************************************************************
*****	Support functions
****/

static void CG_Q3F_MI_Error( char *format, ... )
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
		CG_Printf( BOX_PRINT_MODE_CHAT, "Map Info: %s at '%s' line %d.\n", buff, filename, line );
	}
	else CG_Printf( BOX_PRINT_MODE_CHAT, "Map Info: %s.\n", buff );
	mi.errorFound = qtrue;
}


static qboolean CG_Q3F_MI_GetToken( char *requiredToken )
{
	// Get the next token

	if( !trap_PC_ReadToken( mi.infoHandle, &mi.token ) )
	{
		if( !requiredToken )
			return( qfalse );
		CG_Q3F_MI_Error( "Unexpected end of file" );
		return( qfalse );
	}
	if( requiredToken && *requiredToken && Q_stricmp( mi.token.string, requiredToken ) )
	{
		CG_Q3F_MI_Error( "Expected '%s', found '%s'", requiredToken, mi.token.string );
		return( qfalse );
	}
	return( qtrue );
}

static qboolean CG_Q3F_MI_IsSpace( char c )
{
	return( c == ' ' || c == '\n' || c == '\t' || c == '\r' );
}

static char *matchStrings[] = {
	"-", "and", "to", NULL
};


static qboolean CG_Q3F_MI_StoreMapInfoItem( char *key, char *value, cg_q3f_mapinfo_t *mapInfo, int numItems, qboolean override )
{
	// Store the specified entry in the array.

	int index;
	char c;

	for( index = 0; index < numItems; index++ )
	{
		if( mapInfo[index].value && !Q_stricmp( mapInfo[index].key, key ) )
		{
			if( (c = *mapInfo[index].value) && !override )
				return( qfalse );
			Q_strncpyz( mapInfo[index].value, value, mapInfo[index].valueSize );
			return( c ? qfalse : qtrue );
		}
	}
	return( qfalse );
}

static qboolean CG_Q3F_MI_MapSelectGetArenaField( char *mapname, char *queryfield, char *outbuff, int outsize )
{
	// Pull the longname field out of the .arena file, if present.

	char	*token, *buffptr;
	int		count, len;
	char	buff[8192];
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];
	qhandle_t arenaHandle;

	count = 0;

	if( (len = trap_FS_FOpenFile( va( "scripts/%s.arena", mapname ), &arenaHandle, FS_READ )) < 0 )
		return( qfalse );
	if( len > sizeof(buff) - 1 )
		len = sizeof(buff) - 1;
	trap_FS_Read( buff, len, arenaHandle );
	buff[len] = 0;
	buffptr = buff;

	while( 1 )
	{
		token = COM_Parse( &buffptr );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			COM_ParseError( "Missing { in info file" );
			//Com_Printf( "Missing { in info file\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( &buffptr, qtrue );
			if ( !token[0] ) {
				COM_ParseError( "Unexpected end of info file" );
				//Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buffptr, qfalse );
			if ( !token[0] ) {
				strcpy( token, "<NULL>" );
			}

			if( !Q_stricmp( key, queryfield ) )
			{
				Q_strncpyz( outbuff, token, outsize );
				return( qtrue );
			}
		}
	}
	return( qfalse );
}


/******************************************************************************
*****	Main functions
****/

int CG_Q3F_GetMapInfo( const char *mapname, cg_q3f_mapinfo_t mapInfo[], int numItems, int gameIndex )
{
	// Parse the mapinfo file, filling in any appropriate entry in the supplied mapInfo struct.

	int index, currGameIndex, foundItems, c1, c2;
	char rawmapname[1024], buff[1024], fallbackbuff[128];
	char *infoname, *ptr, *ptr2, *ptr3, *ptr4;

		// Strip out the raw map name without path, extensions, or gameindex
	for( ptr = ptr2 = (char *) mapname, ptr3 = ptr4 = NULL; *ptr; ptr++ )
	{
		if( *ptr == '/' )
			ptr2 = ptr + 1;
		if( *ptr == '.' )
			ptr3 = ptr;
		if( *ptr == '+' )
			ptr4 = ptr;
	}
	ptr3 = ptr3 ? ptr3 : (ptr4 ? ptr4 : (ptr - 1));
	for( infoname = rawmapname, ptr = rawmapname + sizeof(rawmapname) - 1; ptr2 < ptr3 && *ptr2 && infoname < ptr; )
		*infoname++ = *ptr2++;
	*infoname = 0;
		// Extract the gameIndex from the mapname if not supplied explicitely.
	if( ptr4 && !gameIndex )
		gameIndex = atoi( ptr4 + 1 );

		// Reset our array.
	for( index = foundItems = 0; index < numItems; index++ )
	{
		if( mapInfo[index].value )
			*mapInfo[index].value = 0;
	}

		// Save out our mapname if desired.
	if( CG_Q3F_MI_StoreMapInfoItem( "mapname", rawmapname, mapInfo, numItems, qfalse ) )
		foundItems++;

	infoname = va( "%s/%s%s", MAPINFODIR, rawmapname, MAPINFOEXT );

	if( (mi.infoHandle = trap_PC_LoadSource( infoname )) )
	{
		index = 0;
		currGameIndex = 0;

		if( CG_Q3F_MI_GetToken( "mapinfo" ) &&
			CG_Q3F_MI_GetToken( "{"/*}*/ ) )
		{
			while( CG_Q3F_MI_GetToken( NULL ) )
			{
				if( !Q_stricmp( mi.token.string, "gameIndexDef" ) )
				{
					// It's the start of an index definition.
					if( index )
					{
						CG_Q3F_MI_Error( "Nested indexDef" );
						break;
					}
					if( !CG_Q3F_MI_GetToken( "" ) )
						break;
					if( !(currGameIndex = mi.token.intvalue) )
					{
						CG_Q3F_MI_Error( "Invalid indexDef" );
						break;
					}
					if( !CG_Q3F_MI_GetToken( "{"/*}*/ ) )
						break;
				}
				else if( !Q_stricmp( mi.token.string, /*{*/"}" ) )
				{
					// It's the end of an index definition.
					if( !currGameIndex )
						break;
					currGameIndex = 0;
				}
				else 
				{
					Q_strncpyz( buff, mi.token.string, sizeof(buff) );
					if( !CG_Q3F_MI_GetToken( "" ) )
						break;

					if( CG_Q3F_MI_StoreMapInfoItem( buff, mi.token.string, mapInfo, numItems, currGameIndex == gameIndex ) )
						foundItems++;
					if( !CG_Q3F_MI_GetToken( ";" ) )
						break;
				}
			}
		}

		trap_PC_FreeSource( mi.infoHandle );
		if( !mi.errorFound )
			return( foundItems );
	}



		// That was a washout, we'll try the fallback mode.

		// Reset our array.
	for( index = foundItems = 0; index < numItems; index++ )
	{
		if( mapInfo[index].value )
			*mapInfo[index].value = 0;
	}
		// Save out our mapname if desired.
	if( CG_Q3F_MI_StoreMapInfoItem( "mapname", rawmapname, mapInfo, numItems, qfalse ) )
		foundItems++;

		// Attempt to load in the old mapinfo file, and parse out player limits.
	infoname = va( "%s/%s%s", MAPINFODIR, rawmapname, OLDMAPINFOEXT );
	if( (index = trap_FS_FOpenFile( infoname, &mi.infoHandle, FS_READ )) <= 0 )
	{
		if( index == 0 )
			trap_FS_FCloseFile( mi.infoHandle );
		return( 0 );
	}
	if( index >= sizeof(buff) )
		index = sizeof(buff) - 1;
	trap_FS_Read( buff, index, mi.infoHandle );
	trap_FS_FCloseFile( mi.infoHandle );
	buff[index] = 0;
	/* Ensiform - Add this so we can check what files cause the Unexpected end of info file error */
	COM_BeginParseSession(infoname);

		// Save out our mapname if desired.
	if( CG_Q3F_MI_StoreMapInfoItem( "mapinfo", buff, mapInfo, numItems, qfalse ) )
		foundItems++;

	for( ptr = buff; ptr && *ptr; ptr++ )
	{
		if( c1 = atoi( ptr ) )
		{
			// It's a number, let's see if we can find a marker and a second number.

			for( ptr2 = ptr; *ptr2 && ((*ptr2 >= '0' && *ptr2 <= '9') || CG_Q3F_MI_IsSpace( *ptr2 )); ptr2++ );
			for( ptr3 = ptr2; *ptr3 && (*ptr3 <'0' || *ptr3 > '9') && !CG_Q3F_MI_IsSpace( *ptr3 ); ptr3++ );
			for( ptr4 = ptr3; *ptr4 && CG_Q3F_MI_IsSpace( *ptr4 ); ptr4++ );

			if( (c2 = atoi( ptr4 )) && c2 > c1 )
			{
				// Check the marker.

				for( ptr4 = fallbackbuff; ptr2 < ptr3 && ptr4 < fallbackbuff + sizeof(fallbackbuff) - 1; )
					*ptr4++ = *ptr2++;
				*ptr4 = 0;
				for( index = 0; matchStrings[index]; index++ )
				{
					if( !Q_stricmp( matchStrings[index], fallbackbuff ) )
					{
						// We've got a parsable pair of numbers, stick them in the array if possible.

						Com_sprintf( buff, sizeof(buff), "%d", c1 );
						if( CG_Q3F_MI_StoreMapInfoItem( "minplayers", buff, mapInfo, numItems, qfalse ) )
							foundItems++;
						Com_sprintf( buff, sizeof(buff), "%d", c2 );
						if( CG_Q3F_MI_StoreMapInfoItem( "maxplayers", buff, mapInfo, numItems, qfalse ) )
							foundItems++;
						ptr = (char *) -1;
						break;
					}
				}
			}
		}
	}

		// Look for the longname field in the .arena file if necessary.
	for( index = 0; index < numItems; index++ )
	{
		if( mapInfo[index].value && !Q_stricmp( mapInfo[index].key, "longname" ) )
		{
			if( CG_Q3F_MI_MapSelectGetArenaField( rawmapname, "longname", mapInfo[index].value, mapInfo[index].valueSize ) )
				foundItems++;
			break;
		}
	}

	return( foundItems );
}
