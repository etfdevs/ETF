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
