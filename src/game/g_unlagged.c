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
#include "bg_local.h"
#include "g_q3f_mapents.h"

qboolean Q3F_IsSpectator(struct gclient_s *client);

/*
============
G_ResetHistory

Clear out the given client's history (should be called when the teleport bit is flipped)
============
*/
void G_ResetHistory( gentity_t *ent ) {
	int		i, time;

	// fill up the history with data (assume the current position)
	ent->client->historyHead = NUM_CLIENT_HISTORY - 1;
	for ( i = ent->client->historyHead, time = level.time; i >= 0; i--, time -= 50 ) {
		VectorCopy( ent->r.mins, ent->client->history[i].mins );
		VectorCopy( ent->r.maxs, ent->client->history[i].maxs );
		VectorCopy( ent->r.currentOrigin, ent->client->history[i].currentOrigin );
		ent->client->history[i].leveltime = time;
	}
}


/*
============
G_StoreHistory

Keep track of where the client's been
============
*/
void G_StoreHistory( gentity_t *ent ) {
	int		head/*, frametime*/;

	//frametime = level.time - level.previousTime;

	ent->client->historyHead++;
	if ( ent->client->historyHead >= NUM_CLIENT_HISTORY ) {
		ent->client->historyHead = 0;
	}

	head = ent->client->historyHead;

	// store all the collision-detection info and the time
	VectorCopy( ent->r.mins, ent->client->history[head].mins );
	VectorCopy( ent->r.maxs, ent->client->history[head].maxs );
	VectorCopy( ent->s.pos.trBase, ent->client->history[head].currentOrigin );
	SnapVector( ent->client->history[head].currentOrigin );
	ent->client->history[head].leveltime = level.time;
}


/*
=============
TimeShiftLerp

Used below to interpolate between two previous vectors
Returns a vector "frac" times the distance between "start" and "end"
=============
*/
static void TimeShiftLerp( float frac, vec3_t start, vec3_t end, vec3_t result ) {
// From CG_InterpolateEntityPosition in cg_ents.c:
/*
	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );
*/
// Making these exactly the same should avoid floating-point error

	result[0] = start[0] + frac * ( end[0] - start[0] );
	result[1] = start[1] + frac * ( end[1] - start[1] );
	result[2] = start[2] + frac * ( end[2] - start[2] );
}


/*
=================
G_TimeShiftClient

Move a client back to where he was at the specified "time"
=================
*/
void G_TimeShiftClient( gentity_t *ent, int time, qboolean debug, gentity_t *debugger ) {
	int		j, k;
	char msg[2048];

	// this will dump out the head index, and the time for all the stored positions
/*
	if ( debug ) {
		char	str[MAX_STRING_CHARS];

		Com_sprintf(str, sizeof(str), "print \"head: %d, %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\"",
			ent->client->historyHead,
			ent->client->history[0].leveltime,
			ent->client->history[1].leveltime,
			ent->client->history[2].leveltime,
			ent->client->history[3].leveltime,
			ent->client->history[4].leveltime,
			ent->client->history[5].leveltime,
			ent->client->history[6].leveltime,
			ent->client->history[7].leveltime,
			ent->client->history[8].leveltime,
			ent->client->history[9].leveltime,
			ent->client->history[10].leveltime,
			ent->client->history[11].leveltime,
			ent->client->history[12].leveltime,
			ent->client->history[13].leveltime,
			ent->client->history[14].leveltime,
			ent->client->history[15].leveltime,
			ent->client->history[16].leveltime);

		trap_SendServerCommand( debugger - g_entities, str );
	}
*/

	// find two entries in the history whose times sandwich "time"
	// assumes no two adjacent records have the same timestamp
	j = k = ent->client->historyHead;
	do {
		if ( ent->client->history[j].leveltime <= time )
			break;

		k = j;
		j--;
		if ( j < 0 ) {
			j = NUM_CLIENT_HISTORY - 1;
		}
	}
	while ( j != ent->client->historyHead );

	// if we got past the first iteration above, we've sandwiched (or wrapped)
	if ( j != k ) {
		// make sure it doesn't get re-saved
		if ( ent->client->saved.leveltime != level.time ) {
			// save the current origin and bounding box
			VectorCopy( ent->r.mins, ent->client->saved.mins );
			VectorCopy( ent->r.maxs, ent->client->saved.maxs );
			VectorCopy( ent->r.currentOrigin, ent->client->saved.currentOrigin );
			ent->client->saved.leveltime = level.time;
		}

		// if we haven't wrapped back to the head, we've sandwiched, so
		// we shift the client's position back to where he was at "time"
		if ( j != ent->client->historyHead ) {
			float	frac = (float)(time - ent->client->history[j].leveltime) /
				(float)(ent->client->history[k].leveltime - ent->client->history[j].leveltime);

			// interpolate between the two origins to give position at time index "time"
			TimeShiftLerp( frac,
				ent->client->history[j].currentOrigin, ent->client->history[k].currentOrigin,
				ent->r.currentOrigin );

			// lerp these too, just for fun (and ducking)
			TimeShiftLerp( frac,
				ent->client->history[j].mins, ent->client->history[k].mins,
				ent->r.mins );

			TimeShiftLerp( frac,
				ent->client->history[j].maxs, ent->client->history[k].maxs,
				ent->r.maxs );

			if ( debug && debugger != NULL ) {
				// print some debugging stuff exactly like what the client does

				// it starts with "Rec:" to let you know it backward-reconciled
				Com_sprintf( msg, sizeof(msg),
					"print \"^1Rec: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n"
					"^2frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n"
					"^7level.time: %d, est time: %d, level.time delta: %d, est real ping: %d\n\"",
					time, ent->client->history[j].leveltime, ent->client->history[k].leveltime,
					ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
					frac,
					ent->client->history[j].currentOrigin[0],
					ent->client->history[j].currentOrigin[1],
					ent->client->history[j].currentOrigin[2], 
					ent->client->history[k].currentOrigin[0],
					ent->client->history[k].currentOrigin[1],
					ent->client->history[k].currentOrigin[2],
					level.time, level.time + debugger->client->frameOffset,
					level.time - time, level.time + debugger->client->frameOffset - time);

				trap_SendServerCommand( debugger - g_entities, msg );
			}

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		} else {
			// we wrapped, so grab the earliest
			VectorCopy( ent->client->history[k].currentOrigin, ent->r.currentOrigin );
			VectorCopy( ent->client->history[k].mins, ent->r.mins );
			VectorCopy( ent->client->history[k].maxs, ent->r.maxs );

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		}
	}
	else {
		// this only happens when the client is using a negative timenudge, because that
		// number is added to the command time

		// print some debugging stuff exactly like what the client does

		// it starts with "No rec:" to let you know it didn't backward-reconcile
		if ( debug && debugger != NULL ) {
			Com_sprintf( msg, sizeof(msg),
				"print \"^1No rec: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n"
				"^2frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n"
				"^7level.time: %d, est time: %d, level.time delta: %d, est real ping: %d\n\"",
				time, level.time, level.time,
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
				0.0f,
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], 
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
				level.time, level.time + debugger->client->frameOffset,
				level.time - time, level.time + debugger->client->frameOffset - time);

			trap_SendServerCommand( debugger - g_entities, msg );
		}
	}
}


/*
=====================
G_TimeShiftAllClients

Move ALL clients back to where they were at the specified "time",
except for "skip"
=====================
*/
void G_TimeShiftAllClients( int time, gentity_t *skip ) {
	int			i;
	gentity_t	*ent;

	qboolean debug = ( skip != NULL && skip->client && 
			skip->client->pers.debugDelag );

	// for every client
	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++ ) {
		if ( ent->client && ent->inuse && !Q3F_IsSpectator(ent->client) && ent != skip ) {
			G_TimeShiftClient( ent, time, debug, skip );
		}
	}
}


/*
================
G_DoTimeShiftFor

Decide what time to shift everyone back to, and do it
================
*/
void G_DoTimeShiftFor( gentity_t *ent ) {	
	int time;

	// don't time shift for mistakes or bots
	if ( !ent->inuse || !ent->client || (ent->r.svFlags & SVF_BOT) ) {
		return;
	}

	// if it's enabled server-side and the client wants it or wants it for this weapon
	if ( g_unlagged.integer && ent->client->pers.unlagged ) {
		// do the full lag compensation, except what the client nudges
		time = ent->client->attackTime + ent->client->pers.cmdTimeNudge;
	}
	else {
		// do just 50ms
		time = level.previousTime + ent->client->frameOffset;
	}

	G_TimeShiftAllClients( time, ent );
}


/*
===================
G_UnTimeShiftClient

Move a client back to where he was before the time shift
===================
*/
void G_UnTimeShiftClient( gentity_t *ent ) {
	// if it was saved
	if ( ent->client->saved.leveltime == level.time ) {
		// move it back
		VectorCopy( ent->client->saved.mins, ent->r.mins );
		VectorCopy( ent->client->saved.maxs, ent->r.maxs );
		VectorCopy( ent->client->saved.currentOrigin, ent->r.currentOrigin );
		ent->client->saved.leveltime = 0;

		// this will recalculate absmin and absmax
		trap_LinkEntity( ent );
	}
}


/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients( gentity_t *skip ) {
	int			i;
	gentity_t	*ent;

	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++) {
		if ( ent->client && ent->inuse && !Q3F_IsSpectator(ent->client) && ent != skip ) {
			G_UnTimeShiftClient( ent );
		}
	}
}


/*
==================
G_UndoTimeShiftFor

Put everyone except for this client back where they were
==================
*/
void G_UndoTimeShiftFor( gentity_t *ent ) {

	// don't un-time shift for mistakes or bots
	if ( !ent->inuse || !ent->client || (ent->r.svFlags & SVF_BOT) ) {
		return;
	}

	G_UnTimeShiftAllClients( ent );
}

void G_UnlaggedTrace( gentity_t *ent, trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	G_DoTimeShiftFor( ent );
	G_Q3F_ForceFieldTrace( results, start, mins, maxs, end, passEntityNum, contentmask );
	G_UndoTimeShiftFor( ent );
}

