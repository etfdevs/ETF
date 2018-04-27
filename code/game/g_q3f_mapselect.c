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
**	g_q3f_mapselect.c
**
**	Q3F Map query/vote/select functions
*/

#include "g_q3f_mapselect.h"
#include "g_q3f_mapents.h"

/*
**	Static variables used
*/

int mapSelectTime, mapSelectVoteCount, mapSelectNumMaps;
//q3f_array_t *mapSelectPending;
//q3f_keypairarray_t *mapSelectResults;
char **mapSelectNames;//[ETF_MAPSELECT_SELECTCOUNT];
int *mapSelectVotes;//[ETF_MAPSELECT_SELECTCOUNT];
//q3f_keypairarray_t *mapSelectVotes;

/*
**	Main time-based handler functions
*/

/*static int QDECL MSR_SortFunc( const void *a, const void *b )
{
	// Comparison function

	int diff;

	if( !((q3f_keypair_t *) a)->value.type )
		return( 1 );
	if( !((q3f_keypair_t *) b)->value.type )
		return( -1 );
	diff = (((q3f_keypair_t *) b)->value.d.intdata & 0xFF) - (((q3f_keypair_t *) a)->value.d.intdata & 0xFF);
	if( diff )
		return( diff );
	return( ((q3f_keypair_t *) b)->value.d.intdata - ((q3f_keypair_t *) a)->value.d.intdata );
}*/

/*void G_Q3F_MapSelectReady( qboolean forceready )
{
	// Check to see if we have 10 maps that at least X% of
	// the clients claim to have.

	int playermin, index, readycount;
	q3f_keypair_t *kp;
	char buff[1024];

	if( level.mapSelectState == Q3F_MAPSELECT_NONE )
		return;

	playermin = level.numVotingClients * Q3F_MAPSELECT_THRESHHOLD;	// Number of clients voting
	if( playermin < 1 )
		playermin = 1;

	for( readycount = 0, index = -1; kp = G_Q3F_KeyPairArrayTraverse( mapSelectResults, &index ); )
	{
		if(	(kp->value.d.intdata & 0xFF) >= playermin &&
			++readycount >= ETF_MAPSELECT_SELECTCOUNT )
		{
			forceready = qtrue;		// We've got ten maps, let's go
			break;
		}
	}

	if( !forceready )
		return;

	// We now want to assemble our list of 'best' maps, which will be somewhat
	// random already, assuming there were enough confirmed map responses to allow
	// randomness anyway.

	if( !mapSelectResults || !mapSelectResults->used )
		return;
	qsort( mapSelectResults->data, mapSelectResults->max, sizeof(q3f_keypair_t), &MSR_SortFunc );

	memset( mapSelectNames, 0, sizeof(mapSelectNames) );
	for(	index = -1, readycount = ETF_MAPSELECT_SELECTCOUNT; 
			readycount && (kp = G_Q3F_KeyPairArrayTraverse( mapSelectResults, &index ));
			readycount-- )
	{
		G_Q3F_AddString( &mapSelectNames[index], kp->key );
	}

	G_Q3F_KeyPairArrayDestroy( mapSelectResults );
	mapSelectResults = NULL;
	G_Q3F_ArrayDestroy( mapSelectPending );
	mapSelectPending = NULL;

	level.mapSelectState = Q3F_MAPSELECT_READY;
	mapSelectTime = level.time;
	memset( mapSelectVotes, 0, sizeof(mapSelectVotes) );

	buff[0] = 0;
	for( index = 0; index < ETF_MAPSELECT_SELECTCOUNT && mapSelectNames[index]; index++ )
		Q_strcat( buff, sizeof(buff), va( "%s ", mapSelectNames[index] ) );
	trap_SetConfigstring( CS_Q3F_MAPVOTENAMES, buff );

	buff[0] = 0;
	for( index = 0; index < ETF_MAPSELECT_SELECTCOUNT && mapSelectNames[index]; index++ )
		Q_strcat( buff, sizeof(buff), "0 " );
	trap_SetConfigstring( CS_Q3F_MAPVOTETALLY, buff );
}*/

/*qboolean G_Q3F_MapSelectGetArenaField( char *buf, char *queryfield, char *outbuff, int outsize )
{
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	count = 0;

	while( 1 )
	{
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
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

static qboolean G_Q3F_MapSelectMapExists( const char *mapname )
{
	fileHandle_t fh;

	if( trap_FS_FOpenFile( va( "maps/%s.bsp", mapname ), &fh, FS_READ ) >= 0 ) {
		trap_FS_FCloseFile( fh );
		return qtrue;
	} else {
		return qfalse;
	}
}

static void G_Q3F_MapSelectParseMapList( void )
{
	int nummaps, len;
	fileHandle_t fh;
	char buff[8192], arenabuff[8192], *ptr, mapname[32], maptype[32];

	nummaps = trap_FS_GetFileList( "maps", ".bsp", buff, sizeof(buff) );
	for( ptr = buff; nummaps; nummaps-- )
	{
		if( strlen( ptr ) > 32 )
			continue;			// This one's too long, we'll skip it
		COM_StripExtension( ptr, mapname );
		len = trap_FS_FOpenFile( va( "scripts/%s.arena", mapname ), &fh, FS_READ );
		if( len >= 0 )
		{
			// Check we have an arena file and it is of type "q3f"
			if( len > sizeof(arenabuff)-1 )
				len = sizeof(arenabuff)-1;
			trap_FS_Read( arenabuff, len, fh );
			trap_FS_FCloseFile( fh );
			G_Q3F_MapSelectGetArenaField( arenabuff, "type", maptype, sizeof(maptype) );
			if( !Q_stricmp( maptype, "q3f" ) )
				G_Q3F_ArrayAdd( mapSelectPending, Q3F_TYPE_STRING, 0, (int) mapname );
		}
		while( *ptr++ );
	}
}

static void G_Q3F_MapSelectParseGMapVoteExcludeList( void )
{
	// Process the g_mapVoteExcludeList variable
	q3f_array_t *array;
	q3f_data_t *data;
	int nummaps, index, len;
	char buff[8192], arenabuff[8192], *ptr, mapname[32], maptype[32];
	fileHandle_t fh;
	qboolean skipmap;

	if( !(array = G_Q3F_ProcessStrings( g_mapVoteExcludeList.string )) )
		return;

	nummaps = trap_FS_GetFileList( "maps", ".bsp", buff, sizeof(buff) );
	for( ptr = buff; nummaps; nummaps-- )
	{
		if( strlen( ptr ) > 32 )
			continue;			// This one's too long, we'll skip it
		COM_StripExtension( ptr, mapname );

		// Check if we may use this map in the first place
		skipmap = qfalse;
		for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
		{
			if( !Q_stricmp( mapname, data->d.strdata ) ) {
				skipmap = qtrue;
				break;
			}
		}
		if ( skipmap ) {
			while( *ptr++ );
			continue;
		}

		len = trap_FS_FOpenFile( va( "scripts/%s.arena", mapname ), &fh, FS_READ );
		if( len >= 0 )
		{
			// Check we have an arena file and it is of type "q3f"
			if( len > sizeof(arenabuff)-1 )
				len = sizeof(arenabuff)-1;
			trap_FS_Read( arenabuff, len, fh );
			trap_FS_FCloseFile( fh );
			G_Q3F_MapSelectGetArenaField( arenabuff, "type", maptype, sizeof(maptype) );
			if( !Q_stricmp( maptype, "q3f" ) )
				G_Q3F_ArrayAdd( mapSelectPending, Q3F_TYPE_STRING, 0, (int) mapname );
		}
		while( *ptr++ );
	}
	G_Q3F_ArrayDestroy( array );
}

static void G_Q3F_MapSelectParseGMapVoteIncludeList( void )
{
	// Process the g_mapVoteIncludeList variable
	q3f_array_t *array;
	q3f_data_t *data;
	int index, len;
	char arenabuff[8192], maptype[32];
	fileHandle_t fh;

	if( !(array = G_Q3F_ProcessStrings( g_mapVoteIncludeList.string )) )
		return;

	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		if ( G_Q3F_MapSelectMapExists( data->d.strdata ) ) {
			len = trap_FS_FOpenFile( va( "scripts/%s.arena", data->d.strdata ), &fh, FS_READ );
			if( len >= 0 ) {
				// Check we have an arena file and it is of type "q3f"
				if( len > sizeof(arenabuff)-1 )
					len = sizeof(arenabuff)-1;
				trap_FS_Read( arenabuff, len, fh );
				trap_FS_FCloseFile( fh );
				G_Q3F_MapSelectGetArenaField( arenabuff, "type", maptype, sizeof(maptype) );
				if( !Q_stricmp( maptype, "q3f" ) )
					G_Q3F_ArrayAdd( mapSelectPending, Q3F_TYPE_STRING, 0, (int) data->d.strdata );
			}
		}
	}
	G_Q3F_ArrayDestroy( array );
}*/

void ExitLevel (void);
void G_Q3F_MapSelectInit()
{
	// Prepare the list of maps to query

	q3f_array_t *mapList;
	q3f_data_t *data;
	int index, nameIndex;
	char **allNames, *str;
	char buff[1024];

	G_Q3F_LoadServerConfiguration( qfalse );
	if( !(mapList = G_Q3F_GetAvailableMaps()) )
	{
		// No maps available, or something went belly-up - switch to whatever
		// is held in "nextmap" (usually, the same map as the current one).

		level.mapSelectState = Q3F_MAPSELECT_CHANGEMAP;
		ExitLevel();
	}
	G_Q3F_UnloadServerConfiguration();

/*	if( mapSelectPending )
		G_Q3F_ArrayDestroy( mapSelectPending );
	mapSelectPending = G_Q3F_ArrayCreate();
	if( mapSelectResults )
		G_Q3F_KeyPairArrayDestroy( mapSelectResults );
	mapSelectResults = G_Q3F_KeyPairArrayCreate();

	if ( *g_mapVoteExcludeList.string )
		G_Q3F_MapSelectParseGMapVoteExcludeList();
	else if( *g_mapVoteIncludeList.string )
		G_Q3F_MapSelectParseGMapVoteIncludeList();
	else
		G_Q3F_MapSelectParseMapList();
*/
		// Make the maplist into an array so we can hold the votes too.
/*	mapSelectVotes = G_Q3F_KeyPairArrayCreate();
	for( index = -1; data = G_Q3F_ArrayTraverse( mapList, &index ); )
		G_Q3F_KeyPairArrayAdd( mapSelectVotes, data->d.strdata, Q3F_TYPE_INTEGER, 0, 0 );
	G_Q3F_ArrayDestroy( mapList );*/

		// Pick a set of entries at random.
	allNames = G_Alloc( sizeof(char *) * mapList->used );
	for( index = -1, nameIndex = 0; data = G_Q3F_ArrayTraverse( mapList, &index ); nameIndex++ )
		allNames[nameIndex] = data->d.strdata;
	for( nameIndex = 0; nameIndex < mapList->used; nameIndex++ )
	{
		index				= rand() % mapList->used;
		str					= allNames[nameIndex];
		allNames[nameIndex] = allNames[index];
		allNames[index]		= str;
	}
	mapSelectNumMaps = ETF_MAPSELECT_SELECTCOUNT < mapList->used ? ETF_MAPSELECT_SELECTCOUNT : mapList->used;
	mapSelectNames	= G_Alloc( sizeof(char *)	* mapSelectNumMaps );
	mapSelectVotes	= G_Alloc( sizeof(int)		* mapSelectNumMaps );
	memcpy( mapSelectNames, allNames, sizeof(char *) * mapSelectNumMaps );
	G_Free( allNames );

	level.mapSelectState	= Q3F_MAPSELECT_READY;//Q3F_MAPSELECT_QUERY;
	mapSelectTime	= level.time;

	buff[0] = 0;
	for( index = 0; index < mapSelectNumMaps; index++ )
	{
		if( index > 0 )
			Q_strcat( buff, sizeof(buff), " " );
		Q_strcat( buff, sizeof(buff), mapSelectNames[index] );
	}
	trap_SetConfigstring( CS_FORTS_MAPVOTENAMES, buff );
	buff[0] = 0;
	for( index = 0; index < mapSelectNumMaps; index++ )
		Q_strcat( buff, sizeof(buff), "0 " );
	trap_SetConfigstring( CS_FORTS_MAPVOTETALLY, buff );
}

void G_Q3F_MapSelectQuery()
{
	// Called every frame during intermission.
	// The query/response phase has been removed, on the assumption that most
	// players will have all the same maps, what with pure issues, and all.

	int i, j, k;
	char buff[1024], map[128], *ptr;
//	q3f_data_t *data;
//	q3f_keypair_t *kp;
//	char *votedMap;

	if( level.mapSelectState == Q3F_MAPSELECT_READY )
	{
		// We're ready to vote, check to see if we have enough votes, or
		// we've hit the timelimit.

		if( mapSelectVoteCount >= level.numVotingClients ||
			(mapSelectVoteCount >= level.numVotingClients * Q3F_MAPSELECT_THRESHHOLD &&
			mapSelectTime + Q3F_MAPSELECT_VOTEALLTIME <= level.time) ||
			mapSelectTime + Q3F_MAPSELECT_VOTETIME <= level.time )
		{
			// We've got enough votes, or else hit the timelimit - pick a map

				// Total up the number of winners.
			for( i = j = 0, k = -1; i < mapSelectNumMaps; i++ )
			{
				if( mapSelectVotes[i] > k )
				{
					j = 1;
					k = mapSelectVotes[i];
				}
				else if( mapSelectVotes[i] == k ) {
					j++;
				}
			}
				// Pick one of the winners at random.
			j = rand() % j;
			for( i = 0; j >= 0; i++ )
			{
				if( mapSelectVotes[i] == k )
				{
					if( !j-- )
					{
						// Seperate the map from the gameindex, and set both cvars.
						Q_strncpyz( map, mapSelectNames[i], sizeof(map) );
						for( ptr = map; *ptr && *ptr != '+'; ptr++ );
						if( *ptr == '+' ) {
							trap_Cvar_Set( "g_gameindex", ptr + 1 );
							trap_Cvar_Update( &g_gameindex );
						}
						*ptr = 0;
						trap_Cvar_Set( "nextmap", va( "map %s", map ) );
						level.mapSelectState = Q3F_MAPSELECT_CHANGEMAP;
						ExitLevel();
						return;
					}
				}
			}

				// Pick the same map as a fallback.
			// Ensiform sv_mapname is bogus cvar
			//trap_Cvar_VariableStringBuffer( "sv_mapname", buff, 32 );
			trap_Cvar_VariableStringBuffer( "mapname", buff, 32 );
			ptr = COM_SkipPath( buff );
			COM_StripExtension( ptr, buff + 32, sizeof(buff) - 32 );
			trap_Cvar_Set( "nextmap", va( "map %s", buff + 32 ) );
			level.mapSelectState = Q3F_MAPSELECT_CHANGEMAP;
			ExitLevel();	// In case something went badly wrong, switch level anyway
		}
		return;
	}

/*	if( level.mapSelectState == Q3F_MAPSELECT_RESPONSE )
	{
		if( mapSelectTime <= level.time )
			G_Q3F_MapSelectReady( qtrue );				// We've timed out, start 
		return;
	}*/

	if( level.mapSelectState == Q3F_MAPSELECT_NONE )
		G_Q3F_MapSelectInit();

/*	// Randomly fire off a set of maps of interest

	if( mapSelectTime > level.time )
		return;
	Q_strncpyz( buff, "mapquery ", sizeof(buff) );
	for( i = 0; i < Q3F_MAPSELECT_MAXQUERY && mapSelectPending && mapSelectPending->used; i++ )
	{
		for( j = rand() % mapSelectPending->used, k = -1; (data = G_Q3F_ArrayTraverse( mapSelectPending, &k )) && j; j-- );
		G_Q3F_KeyPairArrayAdd( mapSelectResults, data->d.strdata, Q3F_TYPE_INTEGER, 0, (rand() & 0xFF00) );	// Random seed in upper 8 bits
		Q_strcat( buff, sizeof(buff), va( "%s ", data->d.strdata ) );
		G_Q3F_ArrayDel( mapSelectPending, k );
	}
	G_Q3F_KeyPairArraySort( mapSelectResults );		// This is required for searches later

	trap_SendServerCommand( -1, buff );
	mapSelectTime = level.time + Q3F_MAPSELECT_QUERYDELAY;

	if( !mapSelectPending->used )
	{
		// We've run out of maps to send, let's wait a little while for responses

		G_Q3F_ArrayDestroy( mapSelectPending );
		mapSelectPending = NULL;
		level.mapSelectState = Q3F_MAPSELECT_RESPONSE;
		mapSelectTime = level.time + Q3F_MAPSELECT_REPONSETIME;
	}*/
}

/*
**	Responses from user
*/

/*void G_Q3F_MapSelectResponse( gentity_t *ent )
{
	// Player says they've got the map. There's no checking to ensure they're
	// not flooding responses to bias the maps that actually get picked for
	// the vote, but it's not actually stuffing the ballot-box, so to speak.

	char buff[32];
	int numargs, index;
	q3f_keypair_t *kp;

	if( level.mapSelectState == Q3F_MAPSELECT_NONE )
		return;

	numargs = trap_Argc();
	for( index = 1; index < numargs; index++ )
	{
		trap_Argv( index, buff, sizeof(buff) );
		kp = G_Q3F_KeyPairArrayFind( mapSelectResults, G_Q3F_GetString( buff ) );
		if( kp )
			kp->value.d.intdata++;		// Will never overflow 0xFF limit, not enough clients
	}

	G_Q3F_MapSelectReady( qfalse );
}*/

void G_Q3F_MapSelectVote( gentity_t *ent )
{
	// Update the tally and return

	gclient_t *client;
	char buff[1024];
	int index, num;

	if( level.mapSelectState == Q3F_MAPSELECT_NONE )
		return;

	client = ent->client;
	if( !client )
		return;

	trap_Argv( 1, buff, sizeof(buff) );
	num = atoi( buff );
	if( num < 0 || num >= mapSelectNumMaps )
		return;

	client->mapVote = num + 1;

	memset( mapSelectVotes, 0, sizeof(int) * mapSelectNumMaps);
	mapSelectVoteCount = 0;
	for( index = 0, ent = g_entities; index < MAX_CLIENTS; index++, ent++ )
	{
		if( ent->inuse && ent->client && ent->client->mapVote )
		{
			mapSelectVotes[ent->client->mapVote - 1]++;
			mapSelectVoteCount++;
		}
	}

	buff[0] = 0;
	for( index = 0; index < mapSelectNumMaps; index++ )
		Q_strcat( buff, sizeof(buff), va( "%d ", mapSelectVotes[index] ) );
	trap_SetConfigstring( CS_FORTS_MAPVOTETALLY, buff );
}
