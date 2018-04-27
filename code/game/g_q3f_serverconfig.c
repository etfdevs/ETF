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
**	g_q3f_serverconfig.c
**
**	Parses and processes server configuration data. This is presently dog-slow -
**	~50ms on a 750Mhz PC... just enough to put Sir Snipe-A-Lot off his shot... :)
**
*/

#include "g_local.h"
#include "g_q3f_mapdata.h"

#define CONFIGDIR		"config/"
#define	CONFIGROOT		"config/server.cfg"
#define	HISTORYFILE		"config/history.cfg"
#define	INDEXSEPERATOR	"+"

typedef struct {
	int configHandle;
	pc_token_t token;

	qboolean testMode, errorFound;

	q3f_keypairarray_t *schedules;				// Is really a double-array.
	q3f_keypairarray_t *settings, *maplists;

	char *currSetting;

	q3f_keypairarray_t *sourceMapList;
	q3f_array_t *generatedMapList, *includedLists, *excludedLists;

} g_q3f_serverConfig_t;
static g_q3f_serverConfig_t sc;


/******************************************************************************
***** Support functions
****/

static void G_Q3F_SC_Error( char *format, ... )
{
	// Stop with an error, showing file and line.

	va_list		argptr;
	char		buff[2048], filename[1024];
	int			line;

	va_start( argptr, format );
	Q_vsnprintf( buff, sizeof(buff), format, argptr );
	va_end( argptr );

	if( sc.configHandle )
	{
		trap_PC_SourceFileAndLine( sc.configHandle, filename, &line );
//		if( !sc.testMode )
//			G_Error( "Server Configuration: %s at '%s' line %d.", buff, filename, line );
		G_Printf( "Server Configuration: %s at '%s' line %d.\n", buff, filename, line );
	}
	else {
//		if( !sc.testMode )
//			G_Error( "Server Configuration: %s.", buff );
		G_Printf( "Server Configuration: %s.\n", buff );
	}
	sc.errorFound = qtrue;
}

static qboolean G_Q3F_SC_GetToken( char *requiredToken )
{
	// Get the next token

	if( !trap_PC_ReadToken( sc.configHandle, &sc.token ) )
	{
		if( !requiredToken )
			return( qfalse );
		G_Q3F_SC_Error( "Unexpected end of file" );
		return( qfalse );
	}
	if( requiredToken && *requiredToken && Q_stricmp( sc.token.string, requiredToken ) )
	{
		G_Q3F_SC_Error( "Expected '%s', found '%s'", requiredToken, sc.token.string );
		return( qfalse );
	}
	return( qtrue );
}

static q3f_keypair_t *G_Q3F_SC_FindKPEntry( q3f_keypairarray_t *array, char *keyname )
{
	// Find a keypair entry in an unsorted array.

	int index;
	q3f_keypair_t *kp;

	for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( array, &index );  )
	{
		if( !Q_stricmp( kp->key, keyname ) )
			return( kp );
	}
	return( NULL );
}

static q3f_data_t *G_Q3F_SC_FindArrayEntry( q3f_array_t *array, char *valuename )
{
	// Find a (string) array entry in an unsorted array.

	int index;
	q3f_data_t *data;

	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index );  )
	{
		if( !Q_stricmp( data->d.strdata, valuename ) )
			return( data );
	}
	return( NULL );
}

static char *G_Q3F_SC_SettingTraverse( q3f_array_t *setting, int *index )
{
	// Traverse the default and specified settings.

	int defSize, realIndex;
	q3f_keypair_t *kp;
	q3f_array_t *defSetting = NULL;
	q3f_data_t *data;

	kp = G_Q3F_SC_FindKPEntry( sc.settings, "default" );
	if(!kp) {
		defSize = -1; 
	} else {
		if((defSetting = kp->value.d.arraydata) == setting) {
			defSize = -1; 
		} else {
			if(defSetting) {
				defSize = defSetting->max - 1;
			} else {
				defSize = -1;
			}
		}
	}

	if( *index < defSize ) {
		// Parse default

		if( data = G_Q3F_ArrayTraverse( defSetting, index ) )
			return( data->d.strdata );
		*index = defSize;
	}
	// Parse specified

	realIndex = *index - defSize - 1;
	data = G_Q3F_ArrayTraverse( setting, &realIndex );
	*index = realIndex >= 0 ? (realIndex + defSize + 1) : -1;
	return( data ? data->d.strdata : NULL );
}

static const char *G_Q3F_SC_GetSetting( q3f_array_t *setting, char *key )
{
	// Find the value of the specified setting key

	int index, len;
	char *str;

	len = strlen( key );
	for( index = -1; str = G_Q3F_SC_SettingTraverse( setting, &index );  )
	{
		if( !Q_stricmpn( str, key, len ) )
		{
			if( str[len] == ' ' )
				return( str + len + 1 );
			if( !str[len] )
				return( str + len );
		}
	}
	return( NULL );
}


/******************************************************************************
***** Load functions
****/

static void G_Q3F_SC_ProcessSchedule()
{
	// Process a schedule entry.

	char buff[1024];

		// Read the schedule name
	if( !G_Q3F_SC_GetToken( "" ) )
		return;
	Q_strncpyz( buff, sc.token.string, sizeof(buff) );
	if( !sc.schedules )
		sc.schedules = G_Q3F_KeyPairArrayCreate();
	if( !G_Q3F_SC_GetToken( "" ) )
		return;
	Q_strlwr( sc.token.string );
	G_Q3F_KeyPairArrayAdd( sc.schedules, buff, Q3F_TYPE_STRING, 0, (int) sc.token.string );

		// Check the trailing semicolon.
	G_Q3F_SC_GetToken( ";" );
}

static void G_Q3F_SC_ProcessSetting( q3f_keypairarray_t **settingroot, qboolean multitoken )
{
	// Process a setting entry.

	q3f_keypair_t *kp;
	q3f_array_t *setting;
	char cmd[10240];
//	qboolean closeQuote;

		// Read the setting name
	if( !G_Q3F_SC_GetToken( "" ) )
		return;
	Q_strlwr( sc.token.string );
	if( !*settingroot )
		*settingroot = G_Q3F_KeyPairArrayCreate();

		// Get or create the setting.
	if( !(kp = G_Q3F_SC_FindKPEntry( *settingroot, sc.token.string )) )
	{
		setting = G_Q3F_ArrayCreate();
		G_Q3F_KeyPairArrayAdd( *settingroot, sc.token.string, Q3F_TYPE_ARRAY, 0, (int) setting );
	}
	else setting = kp->value.d.arraydata;

		// Check we now have the opening brace.
	if( !G_Q3F_SC_GetToken( "{" ) )
		return;

	cmd[0] = 0;
//	closeQuote = qfalse;
	while( 1 )
	{
		if( !G_Q3F_SC_GetToken( "" ) )
			return;

		switch( sc.token.string[0] )
		{
			case '}':	// End of the setting.
//						if( closeQuote )
//							Q_strcat( cmd, sizeof(cmd), "\"" );
						if( cmd[0] )
							G_Q3F_ArrayAdd( setting, Q3F_TYPE_STRING, 0, (int) cmd );
						return;
			case ';':	// End of a config line.
						if( multitoken )
						{
//							if( closeQuote )
//								Q_strcat( cmd, sizeof(cmd), "\"" );
							if( cmd[0] )
								G_Q3F_ArrayAdd( setting, Q3F_TYPE_STRING, 0, (int) cmd );
							cmd[0] = 0;
//							closeQuote = qfalse;
						}
						break;
			default:	// Another config entry (or part of one).
						if( multitoken )
						{
							if( cmd[0] && sc.token.string[0] )
								Q_strcat( cmd, sizeof(cmd), " " );
//							{
//								Q_strcat( cmd, sizeof(cmd), closeQuote ? " " : " \"" );
//								closeQuote = qtrue;
//							}
							Q_strcat( cmd, sizeof(cmd), sc.token.string );
						}
						else {
							if( sc.token.string )
								G_Q3F_ArrayAdd( setting, Q3F_TYPE_STRING, 0, (int) sc.token.string );
							cmd[0] = 0;
						}
						break;
		}
	}
}


/******************************************************************************
***** Map list functions
****/

static int G_Q3F_SC_ParseMapIndexField( char *str )
{
	// Extract the bit numbers from the string.

	int bitfield = 0;
	while( *str )
	{
		for( ; *str && (*str < '0' || *str > '9'); str++ );
		if( *str )
		{
			bitfield |= (1 << atoi( str ));
			for( ; *str && *str >= '0' && *str <= '9'; str++ );
		}
	}
	return( bitfield );
}

static qboolean G_Q3F_SC_PartialMatch( char *pattern, char *str )
{
	// Attempt to match pattern to string.
	// Should probably do proper pattern matching later, but I doubt it's worth it.

	char *patptr, *patptr2, *strptr, *matchptr;
	char buff[128];

	for( patptr = pattern; *patptr && *patptr != '+'; patptr++ );
	if( !*patptr )
	{
		// Add +? to the end for compatibility reasons.

		Com_sprintf( buff, sizeof(buff), "%s+?", pattern );
		pattern = buff;
	}

	for( patptr = pattern, strptr = str; *patptr && *strptr; )
	{
		if( *patptr == '*' )
		{
			for( patptr2 = ++patptr; *patptr2 && *patptr2 != '*' && *patptr2 != '?'; patptr2++ );
			if( !*patptr )
				return( qtrue );		// We can match to the end of the string, no problem.
			for( strptr = str, matchptr = NULL; *strptr; strptr++ )
			{
				if( !Q_stricmpn( patptr, strptr, patptr2 - patptr ) )
					matchptr = strptr;
			}
			if( !matchptr )
				return( qfalse );		// Failed to match the text following.
			strptr = matchptr + (patptr2 - patptr);
			patptr = patptr2;
		}
		else if( *patptr == '?' )
		{
			patptr++;
			strptr++;
		}
		else if( *patptr++ != *strptr++ )
			return( qfalse );
	}

	return( !(*patptr || *strptr) );
}

static void G_Q3F_SC_ProcessMapPattern( char *mapstr, qboolean include )
{
	// Process a single entry in or out of the array. Very slow :)

	q3f_keypair_t *kp;
	q3f_data_t *data;
	q3f_array_t **sublist;
	int index;

	if( sc.maplists && (kp = G_Q3F_SC_FindKPEntry( sc.maplists, mapstr )) )
	{
		// It's a list definition, process it too.

		sublist = include ? &sc.includedLists : &sc.excludedLists;
		if( !*sublist || !G_Q3F_SC_FindArrayEntry( *sublist, mapstr ) )
		{
			if( !*sublist )
				*sublist = G_Q3F_ArrayCreate();
			G_Q3F_ArrayAdd( *sublist, Q3F_TYPE_STRING, 0, (int) mapstr );
			for( index = -1; data = G_Q3F_ArrayTraverse( kp->value.d.arraydata, &index ); )
				G_Q3F_SC_ProcessMapPattern( data->d.strdata, include );
		}
	}
	else {
		// Process this entry.

		for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( sc.sourceMapList, &index ); )
		{
			if( include )
			{
				if( G_Q3F_SC_PartialMatch( mapstr, kp->key ) &&
					!G_Q3F_SC_FindArrayEntry( sc.generatedMapList, kp->key ) )
					G_Q3F_ArrayAdd( sc.generatedMapList, Q3F_TYPE_STRING, 0, (int) kp->key );
			}
			else {
				if( G_Q3F_SC_PartialMatch( mapstr, kp->key ) &&
					(data = G_Q3F_SC_FindArrayEntry( sc.generatedMapList, kp->key )) )
					G_Q3F_ArrayDel( sc.generatedMapList, data - sc.generatedMapList->data );
			}
		}
	}
}

static void G_Q3F_SC_ProcessMapString( char *mapstr, qboolean include )
{
	// Add or remove all specified entries from the map list.

	char buff[128];
	char *ptr, *buffptr;

	while( mapstr && *mapstr )
	{
		for( ptr = mapstr, buffptr = buff; *ptr && *ptr != ' '; ptr++ )
		{
			if( buffptr < (buff + sizeof(buff) - 1) )
				*buffptr++ = *ptr;
		}
		*buffptr = 0;
		G_Q3F_SC_ProcessMapPattern( buff, include );
		mapstr = *ptr ? (ptr + 1) : ptr;
	}
}

#define MAPMAX( x )	((x >> 8) & 0xFF)
#define	MAPMIN( x )	(x & 0xFF)
static void G_Q3F_SC_FilterByPlayers( int playerCount, char *playerLower, char *playerUpper )
{
	// Attempt to strip out all maps that won't be suitable for the current number of players.

	int pLow, pHigh;
	int index, mapCount;
	q3f_keypair_t *kp;
	q3f_data_t *data;
	q3f_array_t *newmaplist;

		// Work out our acceptable boundaries.
	pLow	= playerLower ? atoi( playerLower ) : 6;
	pHigh	= playerUpper ? atoi( playerUpper ) : 6;
	if( pHigh <= 0 || pHigh > 6 )
		pHigh = 6;
	if( pLow <= 6 || pLow > 6 );//|| pLow > pHigh )
		pLow = 6;
	pHigh	+= playerCount;
	pLow	= playerCount - pLow;
	if( pLow < 4 )
		pLow = 4;	// Don't let it go lower than this.

		// See how many maps would be left over after this.
	for( index = -1, mapCount = 0; data = G_Q3F_ArrayTraverse( sc.generatedMapList, &index ); )
	{
		if( (kp = G_Q3F_KeyPairArrayFind( sc.sourceMapList, data->d.strdata )) &&
			MAPMAX( kp->value.d.intdata )	>= pHigh &&
			MAPMIN( kp->value.d.intdata )	<= pLow )
			mapCount++;
	}

		// Don't strip out any maps if it would strip them _all_ out.
	if( !mapCount || mapCount == sc.generatedMapList->used )
		return;

		// Strip out some maps, since we know we'll leave at least one.
	newmaplist = G_Q3F_ArrayCreate();
	for( index = -1; data = G_Q3F_ArrayTraverse( sc.generatedMapList, &index ); )
	{
		if( (kp = G_Q3F_KeyPairArrayFind( sc.sourceMapList, data->d.strdata )) &&
			MAPMAX( kp->value.d.intdata )	>= pHigh &&
			MAPMIN( kp->value.d.intdata )	<= pLow )
			G_Q3F_ArrayAdd( newmaplist, Q3F_TYPE_STRING, 0, (int) kp->key );
	}
	G_Q3F_ArrayDestroy( sc.generatedMapList );
	sc.generatedMapList = newmaplist;
}

void G_Q3F_SC_FilterByHistory( char *historyLimit )
{
	// Filter out maps that are above the history limit

	q3f_keypairarray_t *history;
	int index, mapCount, limit;
	q3f_data_t *data;
	q3f_keypair_t *kp;
	q3f_array_t *newmaplist;

	limit = historyLimit ? atoi( historyLimit ) : 5;
	if( limit <= 0 || limit >= 100 )
		limit = 5;

		// Load the history.
	if( !(history = G_Q3F_LoadMapHistory()) )
		return;

		// See how many maps would be left over after this.
	for( index = -1, mapCount = 0; data = G_Q3F_ArrayTraverse( sc.generatedMapList, &index ); )
	{
		if( !(kp = G_Q3F_SC_FindKPEntry( history, data->d.strdata )) ||
			kp->value.d.intdata < limit )
			mapCount++;
	}

		// Don't strip out any maps if it would strip them _all_ out.
	if( !mapCount || mapCount == sc.generatedMapList->used )
		return;

		// Strip out some maps, since we know we'll leave at least one.
	newmaplist = G_Q3F_ArrayCreate();
	for( index = -1; data = G_Q3F_ArrayTraverse( sc.generatedMapList, &index ); )
	{
		if( !(kp = G_Q3F_SC_FindKPEntry( history, data->d.strdata )) ||
			kp->value.d.intdata < limit )
			G_Q3F_ArrayAdd( newmaplist, Q3F_TYPE_STRING, 0, (int) data->d.strdata );
	}
	G_Q3F_ArrayDestroy( sc.generatedMapList );
	sc.generatedMapList = newmaplist;
}


static void G_Q3F_SC_CompileMapList( q3f_array_t *setting, int playerCount, qboolean stripRecent )
{
	// Returns a list of maps recommended for play.

	int index, numMaps, gameIndex, gameIndices, minPlayers, maxPlayers, cumMin, cumMax;
//	q3f_data_t *data;
	q3f_keypairarray_t *mapinfo;
//	q3f_keypair_t*kp;
	char rawmaplistbuff[10240], mapname[1024];
	char *ptr, *str, *filters, *historyLimit, *playerLower, *playerUpper;
//	q3f_array_t *defaultSetting;

		// These better have been deallocated by the previous caller.
	sc.includedLists	= NULL;
	sc.excludedLists	= NULL;
	sc.generatedMapList	= NULL;

	// Build a list of source maps to check against.
	if( !sc.sourceMapList )
	{
		numMaps = trap_FS_GetFileList( "maps", ".bsp", rawmaplistbuff, sizeof(rawmaplistbuff) );
		sc.sourceMapList = G_Q3F_KeyPairArrayCreate();
		cumMin = 128;
		cumMax = 0;
		for( index = 0, ptr = rawmaplistbuff; index < numMaps; index++ )
		{
			COM_StripExtension( COM_SkipPath( ptr ), mapname, sizeof(mapname) );

				// Add each mode of the map into the source list.
			if( mapinfo = G_Q3F_LoadMapInfo( ptr ) )
			{
				gameIndices = G_Q3F_SC_ParseMapIndexField( G_Q3F_GetMapInfoEntry( mapinfo, "gameindices", 0, "1" ) );
				for( gameIndex = 0; (1 << gameIndex) <= gameIndices; gameIndex++ )
				{
					if( gameIndices & (1 << gameIndex) )
					{
						str = G_Q3F_GetMapInfoEntry( mapinfo, "minplayers", gameIndex, "0" );
						minPlayers = str ? atoi( str ) : 0;
						str = G_Q3F_GetMapInfoEntry( mapinfo, "maxplayers", gameIndex, "32" );
						maxPlayers = str ? atoi( str ) : 32;
						str = G_Q3F_GetMapInfoEntry( mapinfo, "novote", gameIndex, "0");
						if(atoi(str) == 1)
							continue;

						G_Q3F_KeyPairArrayAdd(	sc.sourceMapList, va( "%s+%d", mapname, gameIndex ),
												Q3F_TYPE_INTEGER, 0,
												(minPlayers & 0xFF) + ((maxPlayers & 0xFF) << 8) );

						if( minPlayers && minPlayers < cumMin )
							cumMin = minPlayers;
						if( maxPlayers && maxPlayers > cumMax )
							cumMax = maxPlayers;
					}
				}

				G_Q3F_KeyPairArrayDestroy( mapinfo );
			}

			while( *ptr ) ptr++;
			ptr++;

		}
		G_Q3F_KeyPairArraySort( sc.sourceMapList );
	}

		// Get the default setting, since it's also used.
//	defaultSetting = (kp = G_Q3F_SC_FindKPEntry( sc.settings, "default" )) ? kp->value.d.arraydata : NULL;

		// Go through the setting including and excluding maps, and getting other useful settings
	sc.generatedMapList = G_Q3F_ArrayCreate();
	filters = playerLower = playerUpper = historyLimit = NULL;
	if( setting == NULL )
	{
		// No server config, just build everything to a default specification.

		G_Q3F_SC_ProcessMapString( "etf_*", qtrue );
		filters = "player, history";
	}
	else {
		for( index = -1; str = G_Q3F_SC_SettingTraverse( setting, &index ); )
		{
			if( !Q_stricmpn( str, "map_include ", 12 ) )
				G_Q3F_SC_ProcessMapString( str + 12, qtrue );
			else if( !Q_stricmpn( str, "map_exclude ", 12 ) )
				G_Q3F_SC_ProcessMapString( str + 12, qfalse );
			else if( !Q_stricmpn( str, "map_filter ", 11 ) )
				filters = str + 11;
			else if( !Q_stricmpn( str, "map_playerlower ", 16 ) )
				playerLower = str + 16;
			else if( !Q_stricmpn( str, "map_playerupper ", 16 ) )
				playerUpper = str + 16;
			else if( !Q_stricmpn( str, "map_historylimit ", 17 ) )
				historyLimit = str + 17;
		}
	}

	//Perform cull on the map based on each specified type.
	if (filters) while( *filters )
	{
		for( ; *filters && !Q_isalpha( *filters ); filters++ );
		for( ptr = filters; *ptr && Q_isalpha( *ptr ); ptr++ );

		if( playerCount >= 0 && !Q_stricmpn( filters, "player", ptr - filters ) )
			G_Q3F_SC_FilterByPlayers( playerCount, playerLower, playerUpper );
		else if( stripRecent && !Q_stricmpn( filters, "history", ptr - filters ) )
			G_Q3F_SC_FilterByHistory( historyLimit );

		filters = *ptr ? (ptr + 1) : ptr;
	}
}


/******************************************************************************
***** Schedule functions
****/

static char *dayNames[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static void G_Q3F_SC_ParseDate( char *str, int *day, int *time )
{
	char buff[32];
	int index, len;
	char *ptr;
	qboolean foundTime;

	for( index = 0, ptr = str; index < sizeof(buff) - 1 && Q_isalpha( *ptr ); index++ )
		buff[index] = *ptr++;
	buff[len = index] = 0;

	if( len )
	{
		for( index = 0; index < 7 && (len < 2 || Q_stricmpn( buff, dayNames[index], len )); index++ );
		if( index == 7 )
			G_Q3F_SC_Error( "Unknown day '%s' in schedule '%s'", buff, str );
		*day = index;
	}

	for( len = 0, foundTime = qfalse; *ptr; ptr++ )
	{
		if( *ptr >= '0' && *ptr <= '9' )
		{
			len = 10 * len + *ptr - '0';
			foundTime = qtrue;
		}
	}
	if( foundTime )
	{
		if( (len % 100) > 59 || (len / 100) > 23 )
			G_Q3F_SC_Error( "Unknown time in schedule '%s'", str );
		*time = len;
	}
}

enum {
	STARTDAY = 0,
	STARTTIME,
	ENDDAY,
	ENDTIME,
};
static void G_Q3F_SC_ParseDateRange( char *str, int dates[4], int day )
{
	// Parse a date range string into two dates.

	char *ptr, *buffptr, *maxptr;
	char buff[32];

	maxptr = buff + sizeof(buff) - 2;
	for( buffptr = buff, ptr = str; *ptr && buffptr < maxptr && *ptr != '-'; )
		*buffptr++ = *ptr++;
	*buffptr = 0;

	dates[STARTDAY] = dates[STARTTIME]  = -1;
	G_Q3F_SC_ParseDate( buff, &dates[STARTDAY], &dates[STARTTIME] );
	dates[ENDDAY] = dates[ENDTIME]  = -1;
	G_Q3F_SC_ParseDate( (*ptr == '-') ? (ptr + 1) : buff, &dates[ENDDAY], &dates[ENDTIME] );

		// Fill in unspecified values.
	if( dates[ENDDAY] < 0 )		dates[ENDDAY] = dates[STARTDAY] < 0 ? day : dates[STARTDAY];
	if( dates[STARTDAY] < 0 )	dates[STARTDAY] = day;
	if( dates[ENDTIME] < 0 )	dates[ENDTIME] = dates[STARTTIME] < 0 ? 2359 : dates[STARTTIME];
	if( dates[STARTTIME] < 0 )	dates[STARTTIME] = 0;
}

static int G_Q3F_CompareDate( int d1, int t1, int d2, int t2 )
{
	return( (d1 * 10000 + t1) - (d2 * 10000 + t2) );
}

static qboolean G_Q3F_SC_InDateRange( int day, int time, int dates[4] )
{
	// See if the specified day and time are in (inclusive) range.

	int startCmp, endCmp, boundCmp;

	startCmp = G_Q3F_CompareDate(	day, time,
									dates[STARTDAY], dates[STARTTIME] );
	endCmp = G_Q3F_CompareDate(		day, time,
									dates[ENDDAY], dates[ENDTIME] );
	boundCmp = G_Q3F_CompareDate(	dates[STARTDAY], dates[STARTTIME],
									dates[ENDDAY], dates[ENDTIME] );
	return( (boundCmp <= 0 && startCmp >= 0 && endCmp <= 0) ||
			(boundCmp > 0 && startCmp >= 0 || endCmp <= 0) );
}

static void G_Q3F_DetermineSetting()
{
	qtime_t qtime;
	int index;
	q3f_keypair_t *kp;
	int dates[4];

		// Get the time (using a cached version if it's available, to prevent
		// flipover between the time it's parsed before map change and after)
	if( *g_serverConfigTime.string )
		sscanf( g_serverConfigTime.string, "%d %d %d %d %d %d %d",
				&qtime.tm_sec,	&qtime.tm_min,	&qtime.tm_hour,	&qtime.tm_mday,
				&qtime.tm_mon,	&qtime.tm_year,	&qtime.tm_wday );
	else {
		trap_RealTime( &qtime );
		trap_Cvar_Set(	"g_serverConfigTime", va( "%d %d %d %d %d %d %d",
						qtime.tm_sec,	qtime.tm_min,	qtime.tm_hour,	qtime.tm_mday,
						qtime.tm_mon,	qtime.tm_year,	qtime.tm_wday ) );
	}

		// Go through each schedule looking for something that looks like a match
	for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( sc.schedules, &index ); )
	{
		if( !Q_stricmp( kp->key, "all" ) )
		{
			sc.currSetting = kp->value.d.strdata;
			G_Printf( "Using server configuration setting '%s'.\n", sc.currSetting );
			return;
		}

		G_Q3F_SC_ParseDateRange( kp->key, dates, qtime.tm_wday );

		if( G_Q3F_SC_InDateRange( qtime.tm_wday, qtime.tm_hour * 100 + qtime.tm_min, dates ) )
		{
			// Decide if we're in this range.
			sc.currSetting = kp->value.d.strdata;
			G_Printf( "Using server configuration setting '%s'.\n", sc.currSetting );
			return;
		}
	}

	G_Q3F_SC_Error( "No appropriate schedule for %s %02d:%02d", dayNames[qtime.tm_wday], qtime.tm_hour, qtime.tm_min );
}

void G_Q3F_SC_CheckScheduleCoverage()
{
	// Check that the whole week is covered by schedules.

	int index, day, time;
	q3f_keypair_t *kp;
	int dates[4];

	day = time = 0;
	while( 1 )
	{
		for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( sc.schedules, &index ); )
		{
			if( !Q_stricmp( kp->key, "all" ) )
				return;

			G_Q3F_SC_ParseDateRange( kp->key, dates, day );

			if( G_Q3F_SC_InDateRange( day, time, dates ) )
				break;
		}
		if( !kp )
		{
			G_Q3F_SC_Error( "Gap in schedule at %s %02d:%02d", dayNames[day], time / 100, time % 100 );
			return;
		}

		time = (dates[ENDTIME] + (((dates[ENDTIME] % 100) == 59) ? 41 : 1)) % 2400;
		day = dates[ENDDAY] + (time ? 0 : 1);
		if( day >= 7 )
			return;			// We've cycled the full week.
	}
}


/******************************************************************************
***** Load the actual configuration into memory.
****/

void G_Q3F_LoadServerConfiguration( qboolean testMode )
{
	int index, index2;
	q3f_keypair_t *kp, *kp2;

	memset( &sc, 0, sizeof(sc) );
	sc.testMode = testMode;

	if( !(sc.configHandle = trap_PC_LoadSource( CONFIGROOT )) )
	{
		G_Q3F_SC_Error( "Unable to load server configuration '" CONFIGROOT "'" );
		return;
	}

	while( G_Q3F_SC_GetToken( NULL ) )
	{
		if( !Q_stricmp( sc.token.string,		"schedule" ) )
			G_Q3F_SC_ProcessSchedule();
		else if( !Q_stricmp( sc.token.string,	"setting" ) )
			G_Q3F_SC_ProcessSetting( &sc.settings, qtrue );
		else if( !Q_stricmp( sc.token.string,	"maplist" ) )
			G_Q3F_SC_ProcessSetting( &sc.maplists, qfalse );
		else {
			G_Q3F_SC_Error( "Unexpected token '%s'", sc.token.string );
			break;
		}
	}

	trap_PC_FreeSource( sc.configHandle );
	sc.configHandle = 0;

	if( sc.testMode && !sc.errorFound )
	{
		// Check our configuration now.

		if( !sc.schedules )
			G_Q3F_SC_Error( "No schedules defined" );
		if( !sc.settings )
			G_Q3F_SC_Error( "No settings defined" );
		if( sc.settings && !G_Q3F_SC_FindKPEntry( sc.settings, "default" ) )
			G_Q3F_SC_Error( "No \"default\" setting defined (an empty setting is fine)" );

		for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( sc.schedules, &index ); )
		{
			if( !G_Q3F_SC_FindKPEntry( sc.settings, kp->value.d.strdata ) )
				G_Q3F_SC_Error( "Setting '%s' not found for schedule '%s'", kp->key, kp->value.d.strdata );
		}

		G_Q3F_SC_CheckScheduleCoverage();	// Make sure there's a schedule for all occasions.

		for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( sc.settings, &index ); )
		{
			for( index2 = -1; kp2 = G_Q3F_KeyPairArrayTraverse( sc.schedules, &index2 ); )
			{
				if( !Q_stricmp( kp2->value.d.strdata, kp->key ) )
					break;
			}
			if( index2 == -1 )	// This setting isn't referenced.
				continue;

			G_Q3F_SC_CompileMapList( kp->value.d.arraydata, -1, qfalse );
			if( !sc.generatedMapList || !sc.generatedMapList->used )
				G_Q3F_SC_Error( "No maps available in setting '%s'", kp->key );
			if( sc.includedLists	)	G_Q3F_ArrayDestroy( sc.includedLists );
			if( sc.excludedLists	)	G_Q3F_ArrayDestroy( sc.excludedLists );
			if( sc.generatedMapList	)	G_Q3F_ArrayDestroy( sc.generatedMapList );
		}
		if( sc.sourceMapList	)	G_Q3F_KeyPairArrayDestroy( sc.sourceMapList );

		if( !sc.errorFound )
			G_Printf( "Server configuration OK.\n" );

			// We'll have memory all over the place after this :)
		G_DefragmentMemory();
	}
}

void G_Q3F_UnloadServerConfiguration()
{
	// Clean up the map data.

	if( sc.schedules )	G_Q3F_KeyPairArrayDestroy(	sc.schedules	);
	if( sc.settings )	G_Q3F_KeyPairArrayDestroy(	sc.settings		);
	if( sc.maplists )	G_Q3F_KeyPairArrayDestroy(	sc.maplists		);
}

void G_Q3F_ExecuteSetting( char *mapexec, int gameindex )
{
	// Locate the 'current' map settings, and execute them to the console.

	q3f_keypair_t *kp;
//	q3f_data_t *data;
	int index;
	const char *directory;
	char *configname, *str;
	char mapname[1024];
	fileHandle_t fhandle;

	if( sc.errorFound )
		return;

	if( !sc.settings )
		return;
	if( !sc.currSetting )
		G_Q3F_DetermineSetting();

	if( Q_stricmp( sc.currSetting, "default" ) &&
		(kp = G_Q3F_SC_FindKPEntry( sc.settings, sc.currSetting )) )
	{
		// Parse current settings.

		for( index = -1; str = G_Q3F_SC_SettingTraverse( kp->value.d.arraydata, &index ); )
		{
			if( Q_stricmpn( str, "map_", 4 ) )
			{
				trap_SendConsoleCommand( EXEC_APPEND, str );
				trap_SendConsoleCommand( EXEC_APPEND, "\n" );
			}
		}
	}

	if( mapexec )
	{
		// Find a per-map configuration file and execute that.

		COM_StripExtension( COM_SkipPath( mapexec ), mapname, sizeof(mapname) );

		if( Q_stricmp( sc.currSetting, "default" ) &&
			(kp = G_Q3F_SC_FindKPEntry( sc.settings, sc.currSetting )) &&
			(directory = G_Q3F_SC_GetSetting( kp->value.d.arraydata, "map_config" )) )
		{
			if( gameindex &&
				(configname = va( "%s/%s%c%d.cfg", directory, mapname, INDEXSEPERATOR, gameindex )) &&
				trap_FS_FOpenFile( configname, &fhandle, FS_READ ) > 0 )
			{
				trap_FS_FCloseFile( fhandle );
				trap_SendConsoleCommand( EXEC_APPEND, va("exec %s\n", configname ));
				mapexec = NULL;
			}
			else if(	(configname = va( "%s/%s.cfg", directory, mapname )) &&
						trap_FS_FOpenFile( configname, &fhandle, FS_READ ) > 0 )
			{
				trap_FS_FCloseFile( fhandle );
				trap_SendConsoleCommand( EXEC_APPEND, va("exec %s\n", configname ));
				mapexec = NULL;
			}
		}

		if( mapexec &&
			(kp = G_Q3F_SC_FindKPEntry( sc.settings, "default" )) &&
			(directory = G_Q3F_SC_GetSetting( kp->value.d.arraydata, "map_config" )) )
		{
			if( gameindex &&
				(configname = va( "%s/%s%c%d.cfg", directory, mapname, INDEXSEPERATOR, gameindex )) &&
				trap_FS_FOpenFile( configname, &fhandle, FS_READ ) > 0 )
			{
				trap_FS_FCloseFile( fhandle );
				trap_SendConsoleCommand( EXEC_APPEND, va( "exec %s\n", configname ));
				mapexec = NULL;
			}
			else if(	(configname = va( "%s/%s.cfg", directory, mapname )) &&
						trap_FS_FOpenFile( configname, &fhandle, FS_READ ) > 0 )
			{
				trap_FS_FCloseFile( fhandle );
				trap_SendConsoleCommand( EXEC_APPEND, va( "exec %s\n", configname ) );
				mapexec = NULL;
			}
		}
	}
}

void G_Q3F_TestServerConfiguration()
{
	// Test the map configuration

	G_Q3F_LoadServerConfiguration( qtrue );
	G_Q3F_UnloadServerConfiguration();
}

q3f_array_t *G_Q3F_GetAvailableMaps()
{
	// Get a list of available maps at the end of a round, based on 
	// number of players and map history.
	// Assumes that the server config is already loaded.

	sc.includedLists = sc.excludedLists = NULL;
	sc.sourceMapList = NULL;
	if( sc.errorFound )
		G_Q3F_SC_CompileMapList( NULL, level.numPlayingClients, qtrue );
	else {
		q3f_keypair_t* kp;
		if( !sc.currSetting )
			G_Q3F_DetermineSetting();

		kp = G_Q3F_SC_FindKPEntry( sc.settings, sc.currSetting );
		G_Q3F_SC_CompileMapList( kp ? kp->value.d.arraydata : NULL, level.numPlayingClients, qtrue );
	}
	if( sc.includedLists	)	G_Q3F_ArrayDestroy( sc.includedLists );
	if( sc.excludedLists	)	G_Q3F_ArrayDestroy( sc.excludedLists );
	if( sc.sourceMapList	)	G_Q3F_KeyPairArrayDestroy( sc.sourceMapList );
	return( sc.generatedMapList );
}

q3f_keypairarray_t *G_Q3F_LoadMapHistory()
{
	// Load in the map history.
	// Assumes that there isn't a file handle already open.

	q3f_keypairarray_t *history;
	char map[32];

	if( sc.errorFound )
		return( NULL );
	if( sc.configHandle )
		G_Error( "G_Q3F_LoadMapHistory: filehandle already in use." );

		// Load the history

	if( !(sc.configHandle = trap_PC_LoadSource( HISTORYFILE )) )
		return( NULL );
	history = NULL;
	while( G_Q3F_SC_GetToken( NULL ) )
	{
		Q_strncpyz( map, sc.token.string, sizeof(map) );
		if( G_Q3F_SC_GetToken( "" ) )
		{
			if( sc.token.type == TT_NUMBER )
			{
				if( !history )
					history = G_Q3F_KeyPairArrayCreate();
				if( sc.token.intvalue > 0 )
					G_Q3F_KeyPairArrayAdd( history, map, Q3F_TYPE_INTEGER, 0, sc.token.intvalue );

				if( !G_Q3F_SC_GetToken( "" ) )
					break;
			}
			else break;
		}
	}

	trap_PC_FreeSource( sc.configHandle );
	sc.configHandle = 0;
	return( history );
}

void G_Q3F_UpdateMapHistory( char *mapname )
{
	// Update the map history with the specified map being bumped. Assumes the map
	// is a qualified map+gameindex string. Also assumes the current setting has been
	// loaded, in order to find the appropriate bump value.

	q3f_keypairarray_t *history;
	int index, bumpValue;
	q3f_keypair_t *kp;
	qhandle_t fileHandle;
	qboolean foundMap;
	char *str;

	if( sc.errorFound )
		return;

		// Find the current 'bump' value.
	if( !sc.currSetting )
		G_Q3F_DetermineSetting();
	str = (char *) G_Q3F_SC_GetSetting(	G_Q3F_SC_FindKPEntry( sc.settings, sc.currSetting )->value.d.arraydata,
										"map_historyUnit" );
	bumpValue = str ? atoi( str ) : 0;
	if( bumpValue < 1 || bumpValue > 50 )
		bumpValue = 10;

		// Get the present history
	if( !(history = G_Q3F_LoadMapHistory()) )
		history = G_Q3F_KeyPairArrayCreate();

		// Update it for this round.
	for( foundMap = qfalse, index = -1; kp = G_Q3F_KeyPairArrayTraverse( history, &index ); )
	{
		if( !Q_stricmp( kp->key, mapname ) )
		{
			kp->value.d.intdata += bumpValue;
			foundMap = qtrue;
		}
		else kp->value.d.intdata--;
	}
	if( !foundMap )
		G_Q3F_KeyPairArrayAdd( history, mapname, Q3F_TYPE_INTEGER, 0, bumpValue );

		// Write it back to disk.
	if( trap_FS_FOpenFile( HISTORYFILE, &fileHandle, FS_WRITE ) >= 0 )
	{
		str = "//\r\n// This file is automatically written by ETF, please don't delete it.\r\n//\r\n\r\n";
		trap_FS_Write( str, strlen( str ), fileHandle );
		for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( history, &index ); )
		{
			if( kp->value.d.intdata > 0 )
			{
				str = va( "\"%s\" %d;\r\n", kp->key, kp->value.d.intdata );
				trap_FS_Write( str, strlen( str ), fileHandle );
			}
		}
		trap_FS_FCloseFile( fileHandle );
	}
}
