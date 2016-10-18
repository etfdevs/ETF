#include "g_local.h"
#include "g_q3f_playerclass.h"

qboolean CheckEveryoneReady( void ) {
	int i;
	int	numReady = 0;

	for( i = 0; i < MAX_CLIENTS; i++ ) {

		// only connected clients ready up
		if ( level.clients[i].pers.connected < CON_CONNECTED ) {
			continue;
		}

		// only non-spectators ready up
		if ( Q3F_IsSpectator( &level.clients[i] ) ) {
			continue;
		}

		// bots are always ready
		if( !level.clients[i].pers.isReady /*&&
			!(g_entities[i].r.svFlags & SVF_BOT)*/ )
			return( qfalse );
		else
			numReady++;
	}

	if( numReady )
		return( qtrue );
	else
		return( qfalse );	// don't start the match if nobody is playing
}

void NotReadyNames( char *names, size_t size ) {
	int i;
	int numNotReady = 0;

	memset( names, 0, size );

	for( i = 0; i < MAX_CLIENTS; i++ ) {

		// only connected clients ready up
		if ( level.clients[i].pers.connected < CON_CONNECTED ) {
			continue;
		}

		// only non-spectators ready up
		if ( Q3F_IsSpectator( &level.clients[i] ) ) {
			continue;
		}

		// bots are always ready
		/*if( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}*/

		if( !level.clients[i].pers.isReady ) {
			if( numNotReady )
				Q_strcat( names, size, ", " );

			Q_strcat( names, size, level.clients[i].pers.netname );
			Q_strcat( names, size, "^7" );
			numNotReady++;
		}
	}
}

void WarnNotReadyClients( void ) {
	int i;

	for( i = 0; i < MAX_CLIENTS; i++ ) {

		// only connected clients ready up
		if ( level.clients[i].pers.connected < CON_CONNECTED ) {
			continue;
		}

		// only non-spectators ready up
		if ( Q3F_IsSpectator( &level.clients[i] ) ) {
			continue;
		}

		// bots are always ready
		/*if( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}*/

		if( !level.clients[i].pers.isReady ) {
			trap_SendServerCommand( i, "menu ready" );
			trap_SendServerCommand( i, va( "cp \"%s^7, please ready up...\n\"", level.clients[i].pers.netname ) );
		}
	}
}

void Cmd_Ready_f( gentity_t *ent ) {
	if ( Q3F_IsSpectator( ent->client ) ) {
		return;
	}

	if ( !g_matchState.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not in prematch, can't ready up.\n\"" );
		return;	
	}

	if( ent->client->pers.isReady ) {
		trap_SendServerCommand( ent-g_entities, "print \"You are already ready.\n\"" );
		return;
	}

	ent->client->pers.isReady = qtrue;
	trap_SendServerCommand( ent-g_entities, "print \"You are ready.\n\"" );
}

void Cmd_UnReady_f( gentity_t *ent ) {
	if ( Q3F_IsSpectator( ent->client ) ) {
		return;
	}

	if ( !g_matchState.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not in prematch, can't ready up.\n\"" );
		return;	
	}

	if( !ent->client->pers.isReady ) {
		trap_SendServerCommand( ent-g_entities, "print \"You are already not ready.\n\"" );
		return;
	}

	ent->client->pers.isReady = qfalse;
	trap_SendServerCommand( ent-g_entities, "print \"You are no longer ready.\n\"" );
}
