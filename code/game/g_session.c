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

#include "g_local.h"
#include "g_q3f_playerclass.h" /* Ensiform - For Spectator Check and Q3F_CLASS_NULL */

#include "cJSON.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

// serialise a JSON object and write it to the specified file
static void Q_FSWriteJSON( void *root, fileHandle_t f ) {
	const char *serialised = NULL;

	serialised = cJSON_Serialize( (cJSON *)root, 1 );
	trap_FS_Write( serialised, strlen( serialised ), f );
	trap_FS_FCloseFile( f );

	free( (void *)serialised );
	cJSON_Delete( (cJSON *)root );
}

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
static void G_WriteClientSessionData( const gclient_t *client ) {
	const clientSession_t *sess = &client->sess;
	cJSON *root;
	fileHandle_t f;
	char fileName[MAX_QPATH] = {0};

	Com_sprintf( fileName, sizeof(fileName), "session/client%02i.json", (int)(client - level.clients) );
	Com_Printf( "Writing session file %s\n", fileName );

	root = cJSON_CreateObject();
	cJSON_AddIntegerToObject( root, "spectatorTime", sess->spectatorTime );
	cJSON_AddIntegerToObject( root, "spectatorState", sess->spectatorState );
	cJSON_AddIntegerToObject( root, "spectatorClient", sess->spectatorClient );
	cJSON_AddIntegerToObject( root, "sessionClass", sess->sessionClass );
	cJSON_AddIntegerToObject( root, "sessionTeam", (int)sess->sessionTeam );
	cJSON_AddIntegerToObject( root, "adminLevel", sess->adminLevel );
	cJSON_AddBooleanToObject( root, "muted", !!sess->muted );
	cJSON_AddBooleanToObject( root, "shoutcaster", !!sess->shoutcaster );
	cJSON_AddIntegerToObject( root, "ignoreClients0", client->sess.ignoreClients[0] );
	cJSON_AddIntegerToObject( root, "ignoreClients1", client->sess.ignoreClients[1] );
	cJSON_AddStringToObject( root, "ipStr", sess->ipStr ? sess->ipStr : "" );
	cJSON_AddStringToObject( root, "guidStr", *sess->guidStr ? sess->guidStr : "" );

	trap_FS_FOpenFile( fileName, &f, FS_WRITE );

	Q_FSWriteJSON( root, f );
}

/*
================
G_ReadClientSessionData

Called on a reconnect
================
*/
void G_ReadClientSessionData( gclient_t *client ) {
	clientSession_t *sess = &client->sess;
	cJSON *root = NULL, *object = NULL;
	char fileName[MAX_QPATH] = {0};
	char *buffer = NULL;
	fileHandle_t f = NULL_FILE;
	unsigned int len = 0;
	const char *tmp = NULL;

	Com_sprintf( fileName, sizeof(fileName), "session/client%02i.json", client - level.clients );
	len = trap_FS_FOpenFile( fileName, &f, FS_READ );

	// no file
	if ( !f || !len || len == -1 ) {
		trap_FS_FCloseFile( f );
		return;
	}

	buffer = (char *)malloc( len + 1 );
	if ( !buffer ) {
		return;
	}

	trap_FS_Read( buffer, len, f );
	trap_FS_FCloseFile( f );
	buffer[len] = '\0';

	// read buffer
	root = cJSON_Parse( buffer );
	free( buffer );

	if ( !root ) {
		Com_Printf( "G_ReadSessionData(%02i): could not parse session data\n", (int)(client - level.clients) );
		return;
	}

	if ( (object = cJSON_GetObjectItem( root, "spectatorTime" )) ) {
		sess->spectatorTime = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "spectatorState" )) ) {
		sess->spectatorState = (spectatorState_t)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "spectatorClient" )) ) {
		sess->spectatorClient = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "sessionClass" )) ) {
		sess->sessionClass = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "sessionTeam" )) ) {
		sess->sessionTeam = (q3f_team_t)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "adminLevel" )) ) {
		sess->adminLevel = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "muted" )) ) {
		sess->muted = cJSON_ToBoolean( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "shoutcaster" )) ) {
		sess->shoutcaster = cJSON_ToBoolean( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "ignoreClients0" )) ) {
		sess->ignoreClients[0] = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "ignoreClients1" )) ) {
		sess->ignoreClients[1] = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "ipStr" )) ) {
		// Golliwog: This is seriously nasty, but IP Addresses appear not to
		// be preserved over map changes, so they're stored and extracted here.
		char *ipstr;
		if ( (tmp = cJSON_ToString( object )) ) {
			G_Q3F_AddString( &ipstr, (char *)tmp );
			G_Q3F_RemString( &sess->ipStr );
			sess->ipStr = ipstr;
		}
		// Golliwog.
	}
	if ( (object = cJSON_GetObjectItem( root, "guidStr" )) ) {
		if ( (tmp = cJSON_ToString( object )) ) {
			Q_strncpyz( sess->guidStr, tmp, sizeof(sess->guidStr) );
		}
	}

	if ( !g_matchState.integer ) 
		sess->sessionTeam = Q3F_TEAM_SPECTATOR;

	if ( Q3F_IsSpectator( client ) )
	{
		/* Ensiform - Nuke the class if we're spectator */
		client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
		sess->sessionClass = Q3F_CLASS_NULL;
	}

	cJSON_Delete( root );
	root = NULL;
}

/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitClientSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess = &client->sess;

	// initial team determination
	if ( g_teamAutoJoin.integer ) {
		sess->sessionTeam = PickTeam( -1 );
		BroadcastTeamChange( client, -1 );
	} else {
		// always spawn as spectator in team games
		sess->sessionTeam = Q3F_TEAM_SPECTATOR;
	}

	if ( Q3F_IsSpectator( client ) || sess->sessionTeam == Q3F_TEAM_SPECTATOR )
	{
		/* Ensiform - Nuke the class if we're spectator */
		client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
		sess->sessionClass = Q3F_CLASS_NULL;
		sess->spectatorState = SPECTATOR_FREE;
	}
	else
		sess->spectatorState = SPECTATOR_NOT;

	sess->spectatorTime = level.time;

	memset( sess->ignoreClients, 0, sizeof( sess->ignoreClients ) );
	sess->muted = qfalse;

	G_WriteClientSessionData( client );
}

static const char *metaFileName = "session/meta.json";

/*
==================
G_InitWorldSession

==================
*/
void G_ReadSessionData( void ) {
	char *buffer = NULL;
	fileHandle_t f = NULL_FILE;
	unsigned int len = 0u;
	cJSON *root;

	Com_Printf( "G_ReadSessionData: reading %s...", metaFileName );
	len = trap_FS_FOpenFile( metaFileName, &f, FS_READ );

	// no file
	if ( !f || !len || len == -1 ) {
		Com_Printf( "failed to open file, clearing session data...\n" );
		level.newSession = qtrue;
		if(f)
			trap_FS_FCloseFile( f );
		return;
	}

	buffer = (char *)malloc( len + 1 );
	if ( !buffer ) {
		Com_Printf( "failed to allocate buffer, clearing session data...\n" );
		level.newSession = qtrue;
		return;
	}

	trap_FS_Read( buffer, len, f );
	trap_FS_FCloseFile( f );
	buffer[len] = '\0';

	// read buffer
	root = cJSON_Parse( buffer );

	// if the gametype changed since the last session, don't use any client sessions
	if ( g_gametype.integer != cJSON_ToInteger( cJSON_GetObjectItem( root, "gametype" ) ) ) {
		level.newSession = qtrue;
		Com_Printf( "gametype changed, clearing session data..." );
	}

	free( buffer );
	cJSON_Delete( root );
	root = NULL;
	Com_Printf( "done\n" );
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int i;
	fileHandle_t f = NULL_FILE;
	const gclient_t *client = NULL;
	cJSON *root = cJSON_CreateObject();

	cJSON_AddIntegerToObject( root, "gametype", g_gametype.integer );

	Com_Printf( "G_WriteSessionData: writing %s...", metaFileName );
	trap_FS_FOpenFile( metaFileName, &f, FS_WRITE );

	Q_FSWriteJSON( root, f );
	// the above function closes the file and cleans up mem for us

	for ( i = 0, client = level.clients; i < level.maxclients; i++, client++ ) {
		if ( client->pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( client );
		}
	}

	Com_Printf( "done\n" );
}
