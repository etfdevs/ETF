// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_q3f_playerclass.h" /* Ensiform - For Spectator Check and Q3F_CLASS_NULL */


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i %i %i %i :%s", 
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.sessionClass,
		client->sess.sessionTeam,
		client->sess.adminLevel,
		client->sess.muted,
		client->sess.shoutcaster,
		client->sess.ignoreClients[0],
		client->sess.ignoreClients[1],
		(client->sess.ipStr ? client->sess.ipStr : "")
		);

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	char *var, *ipstr;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i", 
		&client->sess.spectatorTime,
		(int *)&client->sess.spectatorState,
		&client->sess.spectatorClient,
		&client->sess.sessionClass,
		(int *)&client->sess.sessionTeam,
		&client->sess.adminLevel,
		(int *)&client->sess.muted,
		(int *)&client->sess.shoutcaster,
		&client->sess.ignoreClients[0],
		&client->sess.ignoreClients[1]
		);

	if ( !g_matchState.integer ) 
		client->sess.sessionTeam = Q3F_TEAM_SPECTATOR;

	if ( Q3F_IsSpectator( client ) )
	{
		/* Ensiform - Nuke the class if we're spectator */
		client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
		client->sess.sessionClass = Q3F_CLASS_NULL;
	}

	// Golliwog: This is seriously nasty, but IP Addresses appear not to
	// be preserved over map changes, so they're stored and extracted here.
	for( var = s; *var && *var != ':'; var++ );
	if( *var++ == ':' && *var )
	{
		G_Q3F_AddString( &ipstr, var );
		G_Q3F_RemString( &client->sess.ipStr );
		client->sess.ipStr = ipstr;
	}
	// Golliwog.
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
//	const char		*value;

	sess = &client->sess;

	// initial team determination
	if ( g_teamAutoJoin.integer ) {
		sess->sessionTeam = PickTeam( -1 );
		BroadcastTeamChange( client, -1 );
	} else {
		// always spawn as spectator in team games unless bot
		sess->sessionTeam = Q3F_TEAM_SPECTATOR;
	}

	if ( Q3F_IsSpectator( client ) || sess->sessionTeam == Q3F_TEAM_SPECTATOR )
	{
		/* Ensiform - Nuke the class if we're spectator */
		client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
		client->sess.sessionClass = Q3F_CLASS_NULL;
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	memset( sess->ignoreClients, 0, sizeof( sess->ignoreClients ) );
	sess->muted = qfalse;

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
