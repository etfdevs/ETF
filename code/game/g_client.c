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
#include "g_q3f_mapents.h"
#include "g_q3f_weapon.h"
#include "g_q3f_admin.h"
#include "g_q3f_team.h"
#include "g_q3f_flag.h"

#include "g_bot_interface.h"
#ifdef BUILD_LUA
#include "g_lua.h"
#endif
// g_client.c -- client functions that don't happen every frame

vec3_t	playerMins = {-15, -15, -24};
vec3_t	playerMaxs = {15, 15, 32};

static char	ban_reason[MAX_CVAR_VALUE_STRING];

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	qboolean	b;

	G_SpawnBoolean( "nobots", "0", &b);
	if ( b ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnBoolean( "nohumans", "0", &b );
	if ( b ) {
		ent->flags |= FL_NO_HUMANS;
	}
	G_SpawnBoolean( "warmuponly", "0", &b );
	if ( b ) {
		if (! (g_matchState.integer > MATCH_STATE_PLAYING)) {
			G_FreeEntity( ent );
			return;
		}
		ent->flags |= FL_WARMUP_ONLY;
	}
	G_SpawnFloat( "wait", "0.1", &ent->wait );	// Wait 0.1 before allowing respawns
	ent->wait *= 1000;							// Convert to milliseconds
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client ) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from, gentity_t *ent ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			if( ent && spot->mapdata )
			{
				if(	(spot->mapdata->state != Q3F_STATE_INACTIVE) ||
					(spot->mapdata->team && !(spot->mapdata->team & (1 << ent->client->sess.sessionTeam))) ||
					(spot->mapdata->classes  && !(spot->mapdata->classes & (1 << ent->client->ps.persistant[PERS_CURRCLASS]))) ||
					(spot->mapdata->holding && !G_Q3F_CheckHeld( ent, spot->mapdata->holding )) ||
					(spot->mapdata->notholding && !G_Q3F_CheckNotHeld( ent, spot->mapdata->notholding )) ||
					(spot->mapdata->clientstats && !G_Q3F_CheckClientStats( ent, spot->mapdata->clientstats, (spot->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) )) ||
					(!ent->client->sess.lives && !(spot->mapdata->flags & Q3F_FLAG_ALLOWDEAD)) )
					continue;		// Skip this, we can't respawn here
			}
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( gentity_t *ent ) {
	gentity_t	*spot;
	int			count, mode;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	for( count = mode = 0; mode < 3; mode++ )
	{
		count = 0;
		spot = NULL;

		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
		{
			if( !mode && SpotWouldTelefrag( spot ) ) {
				continue;
			}
			if( ent && spot->mapdata && !Q3F_IsSpectator( ent->client ) )
			{
				if(	(mode < 2 && spot->mapdata->state != Q3F_STATE_INACTIVE) ||
					(spot->mapdata->team && !(spot->mapdata->team & (1 << ent->client->sess.sessionTeam))) ||
					(spot->mapdata->classes  && !(spot->mapdata->classes & (1 << ent->client->ps.persistant[PERS_CURRCLASS]))) ||
					(spot->mapdata->holding && !G_Q3F_CheckHeld( ent, spot->mapdata->holding )) ||
					(spot->mapdata->notholding && !G_Q3F_CheckNotHeld( ent, spot->mapdata->notholding )) ||
					(spot->mapdata->clientstats && !G_Q3F_CheckClientStats( ent, spot->mapdata->clientstats, (spot->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) )) ||
					(!ent->client->sess.lives && !(spot->mapdata->flags & Q3F_FLAG_ALLOWDEAD)) )
					continue;		// Skip this, we can't respawn here
			}
			spots[ count ] = spot;
			count++;
		}
		if( count )
			break;		// We've found some spots
	}

	if( !count )
		return( NULL );
//		G_Error( "Unable to find a spawnpoint." );

//	if ( !count ) {	// no spots that won't telefrag
//		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
//	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, gentity_t *ent ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[64];
	gentity_t	*list_spot[64];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}

		if( ent && spot->mapdata && !Q3F_IsSpectator( ent->client ) )
		{
			if(	(spot->mapdata->state != Q3F_STATE_INACTIVE) ||
				(spot->mapdata->team && !(spot->mapdata->team & (1 << ent->client->sess.sessionTeam))) ||
				(spot->mapdata->classes  && !(spot->mapdata->classes & (1 << ent->client->ps.persistant[PERS_CURRCLASS]))) ||
				(spot->mapdata->holding && !G_Q3F_CheckHeld( ent, spot->mapdata->holding )) ||
				(spot->mapdata->notholding && !G_Q3F_CheckNotHeld( ent, spot->mapdata->notholding )) ||
				(spot->mapdata->clientstats && !G_Q3F_CheckClientStats( ent, spot->mapdata->clientstats, (spot->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) )) ||
				(!ent->client->sess.lives && !(spot->mapdata->flags & Q3F_FLAG_ALLOWDEAD)) )
				continue;		// Skip this, we can't respawn here
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );
		for (i = 0; i < numSpots; i++) {
			if ( dist > list_dist[i] ) {
				if ( numSpots >= 64 )
					numSpots = 64-1;
				for (j = numSpots; j > i; j--) {
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				if (numSpots > 64)
					numSpots = 64;
				break;
			}
		}
		if (i >= numSpots && numSpots < 64) {
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if (!numSpots) {
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			G_Error( "Couldn't find a spawn point" );
		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = ETF_random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectQ3FSpawnPoint

Finds the most appropriate spawn point to use, based on types, weightings,
and availability.
============
*/

char *weightkeystr;
gentity_t *SelectQ3FSpawnPoint(	vec3_t avoidPoint, vec3_t origin, vec3_t angles, gentity_t *ent )
{
	gentity_t *scan, *max;
	int numCandidates, weighting, weightleft, index = 0;
	char *tmp;
	q3f_keypair_t *kp;
	gentity_t *candidates[128];
	int weightings[128];
	qboolean isDynSpawn;
	qboolean foundWarmup = qfalse;

	if( !weightkeystr )
		weightkeystr = G_Q3F_GetString( "priority" );

		// Generate the weightings.
	for(	scan = &g_entities[MAX_CLIENTS], max = &g_entities[ENTITYNUM_MAX_NORMAL], numCandidates = weighting = 0;
			scan <= max; scan++ )
	{
		if( scan->mapdata && scan->mapdata->state != Q3F_STATE_INACTIVE )
			continue;
		
		isDynSpawn = Q_stricmp( scan->classname, "info_player_targetspawn" );
		if( isDynSpawn && Q_stricmp( scan->classname, "info_player_deathmatch" ))
			continue;

		if( !G_Q3F_CheckCriteria( ent, scan ) )
			continue;

		isDynSpawn = !isDynSpawn;

		if ( g_matchState.integer > MATCH_STATE_PLAYING ) {
			if (!foundWarmup ) {
				if ( scan->flags & FL_WARMUP_ONLY ) {
					foundWarmup = qtrue;
					numCandidates = 0;
					weighting = 0;
				}
			} else {
				/* Found at least one point for warmup only, the rest had better too */
				if (!( scan->flags & FL_WARMUP_ONLY ))
					continue;
			}
		}
		candidates[numCandidates] = scan;

		if( !isDynSpawn && SpotWouldTelefrag( scan ) )
			weightings[numCandidates] = 0;		// Telefragging spawn points are always skipped if possible.
		else {
			if( scan->mapdata && weightkeystr) {
				kp = G_Q3F_KeyPairArrayFind( scan->mapdata->other, weightkeystr );
				if(kp) {
					if( kp->value.type == Q3F_TYPE_STRING ) {
						kp->value.d.intdata = Q_atoi( tmp = kp->value.d.strdata );
						kp->value.type = Q3F_TYPE_INTEGER;
						G_Q3F_RemString( &tmp );
					}
					weightings[numCandidates] = kp->value.d.intdata;
				}
				else weightings[numCandidates] = 20;
			}
			else weightings[numCandidates] = 20;
		}

		if( weightings[numCandidates] < 0 )
			weightings[numCandidates] = 0;
		weighting += weightings[numCandidates];

		if( ++numCandidates >= 128 )
			break;
	}

		// Pick one of the spawnpoints.
	while( numCandidates > 0 )
	{
		if( weighting <= 0 )
			scan = candidates[rand() % numCandidates];
		else {
			weightleft = rand() % weighting;
			for( index = 0; index < numCandidates; index++ )
			{
				if( weightings[index] < 0 )
					continue;
				if( weightings[index] > weightleft )
				{
					scan = candidates[index];
					break;
				}
				weightleft -= weightings[index];
			}
			if( index >= numCandidates )
			{
				scan = candidates[rand() % numCandidates];	// Something has gone horribly wrong :)
			}
		}

			// Fill in the values for this spawnpoint.
		if( !Q_stricmp( scan->classname, "info_player_targetspawn" ) )
		{
			// This is a 'dynamic' spawn point, we attempt to find a reasonable place
			// to 'spot' the player around one or more of the targets.

			if( G_Q3F_PositionDynamicSpawn( scan, origin, angles ) )
				return( scan );

				// We couldn't use this spawn, remove it entirely.
			weighting -= weightings[index];
			memcpy( &candidates[index], &candidates[index + 1], sizeof(gentity_t *) * (numCandidates - index - 1) );
			memcpy( &weightings[index], &weightings[index + 1], sizeof(int) * (numCandidates - index - 1) );
			numCandidates--;
		}
		else {
			// This is a static spawn point.
			VectorCopy( scan->s.origin, origin );
			VectorCopy( scan->s.angles, angles );
			return( scan );
		}
	}

	return( NULL );
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, gentity_t *ent ) {
	return SelectQ3FSpawnPoint( avoidPoint, origin, angles, ent );
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, gentity_t *ent ) {
	gentity_t	*spot;

	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles, ent );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint
============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;

		//ent = G_Spawn();
		//ent->classname = "bodyqueagentdata";
		//ent->neverFree = qtrue;
		//level.bodyQueAgentData[i] = ent;
	}
}

static void BodyUnlink( gentity_t *ent ) {
	// the body ques are never actually freed, they are just unlinked
	trap_UnlinkEntity( ent );
	ent->physicsObject = qfalse;
	ent->classname = "bodyque";
	ent->s.eType = ET_GENERAL;
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
static void BodySink( gentity_t *ent ) {
	ent->physicsObject = qfalse;
	ent->nextthink = level.time + 11500;
	ent->think = BodyUnlink;
	ent->s.pos.trType = TR_LINEAR;
	ent->s.pos.trTime = level.time;
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	VectorSet( ent->s.pos.trDelta, 0, 0, -8 );
	/*if ( level.time - ent->timestamp > 11500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		ent->classname = "bodyque";
		//ent->s.eFlags = 0;
		//ent->s.eType = ET_GENERAL;
		
		// Golliwog: Remove agentdata now we're finished with it
		//trap_UnlinkEntity( level.bodyQueAgentData[ent->count] );
		return;	
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;*/
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/

void CopyToBodyQue( gentity_t *ent ) {
	// Golliwog: Heavily modified to prevent class changes on death.

	gentity_t	*body;//, *agentdata;
	int			i, contents;

	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->client->ps.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	if( !ent->client || Q3F_IsSpectator( ent->client ) )
		return;		// Golliwog: Only gib if they have a body.

	if ( ent->health <= GIB_HEALTH )
		return;

	// grab a body que and cycle to the next one
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;
	body		= level.bodyQue[ level.bodyQueIndex ];
	//agentdata	= level.bodyQueAgentData[ level.bodyQueIndex ];

	//trap_UnlinkEntity( body );
	//trap_UnlinkEntity( agentdata );

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
	body->s.eType = ET_Q3F_CORPSE;
	body->classname = "corpse";
	body->s.extFlags = 0;
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	//body->count = level.bodyQueIndex;
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	for( i=0; i<MAX_EVENTS; i++ )
		body->s.events[i] = 0;
	body->s.eventSequence = 0;


	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case ANI_DEATH_1:
	case ANI_DEAD_1:
		body->s.torsoAnim = body->s.legsAnim = ANI_DEAD_1;
		break;
	case ANI_DEATH_2:
	case ANI_DEAD_2:
	default:
		body->s.torsoAnim = body->s.legsAnim = ANI_DEAD_2;
		break;
	}

	body->r.svFlags = ent->r.svFlags & ~SVF_BOT;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->s.modelindex = ent->client->sess.sessionTeam;
	body->s.modelindex2 = ent->client->ps.persistant[PERS_CURRCLASS];

	body->nextthink = level.time + 10000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed

	// Golliwog: Put in an 'agentdata' field to prevent the body changing team/class
	// At some point I'll work out a better way of doing all this...
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
		body->r.contents	= 0;
		body->clipmask		= 0;
	} else {
		body->takedamage = qtrue;

		//body->s.eFlags |= EF_Q3F_DISGUISE;
		//agentdata->s.eType			= ET_Q3F_AGENTDATA;
		//agentdata->classname		= "agentdata";
		//agentdata->s.time			= level.time - 1;
		//agentdata->s.time2			= level.time - 1;
		//agentdata->s.torsoAnim		= ent->client->ps.persistant[PERS_CURRCLASS];
		//agentdata->s.weapon			= ent->client->sess.sessionTeam;
		//agentdata->activator		= body;
		//agentdata->s.otherEntityNum	= body->s.number;
		//agentdata->nextthink		= 0;		// No think for this ent, it gets freed by BodySink();
		//agentdata->s.modelindex2	= 1;		// Disguise
		//VectorCopy( body->s.pos.trBase, agentdata->s.pos.trBase );
		//SnapVector( ent->s.pos.trBase );
		//G_SetOrigin( agentdata, agentdata->s.pos.trBase );
		//trap_LinkEntity( agentdata );
	}
	// Golliwog.

	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	gentity_t	*tent;
	if(ent->client->killDelayTime > level.time)
		return;

	ent->client->killDelayTime = 0;		// Jules
	CopyToBodyQue (ent);
	if( ClientSpawn(ent) )
	{
		// add a teleportation effect
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;
	}
}

/*
================
TeamCount

Returns number of players on a team
================
*/
//team_t TeamCount( int ignoreClientNum, int team ) {
int TeamCount( int ignoreClientNum, int team ) {	//RR2DO2
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

/*
================
PickTeam

================
*/

typedef struct g_q3f_pickteam_s {
	int team;
	int count, score, frags;
} g_q3f_pickteam_t;

static int PickTeamSortFunc( const void *arg1, const void *arg2 )
{
	const g_q3f_pickteam_t *t1, *t2;

	t1 = arg1;
	t2 = arg2;

	if( t1->team == Q3F_TEAM_SPECTATOR )		// Spectators always come last
		return( t2->team != Q3F_TEAM_SPECTATOR );
	if( t1->count != t2->count )
		return( t1->count < t2->count ? -1 : 1 );
	if( t1->score != t2->score )
		return( t1->score < t2->score ? -1 : 1 );
	if( t1->frags != t2->frags )
		return( t1->frags < t2->frags ? -1 : 1 );
	return( 1 );
}

//team_t PickTeam( int ignoreClientNum ) {
int PickTeam( int ignoreClientNum ) {	// RR2DO2
	g_q3f_pickteam_t teams[4];
	g_q3f_teaminfo_t *ti;
	g_q3f_pickteam_t *pt;
	int i;

	for( i = Q3F_TEAM_RED; i < Q3F_TEAM_SPECTATOR; i++ )
	{
		ti = &g_q3f_teamlist[i];
		pt = &teams[i-Q3F_TEAM_RED];
		pt->count = TeamCount( ignoreClientNum, i );
		if( (pt->count >= ti->playerlimit && ti->playerlimit >= 0) ||
			!(g_q3f_allowedteams & 1 << i) )
			pt->team = Q3F_TEAM_SPECTATOR;			// Can't pick this team, it's full or disallowed
		else {
			pt->team	= i;
			pt->score	= level.teamScores[i - Q3F_TEAM_RED];
			pt->frags	= level.teamFrags[i - Q3F_TEAM_RED];
		}
	}

	qsort( teams, 4, sizeof(g_q3f_pickteam_t), PickTeamSortFunc );
	return( teams[0].team );	// Returns Q3F_TEAM_SPECTATOR if it failed to find a team
}

int PickAutoJoinTeam( int ClientNum ) {	// RR2DO2
	g_q3f_teaminfo_t *ti;
	int teamPlayerCount;

	ti = &g_q3f_teamlist[level.clients[ClientNum].sess.sessionTeam];
	teamPlayerCount = TeamCount( ClientNum, level.clients[ClientNum].sess.sessionTeam );
	if( (teamPlayerCount >= ti->playerlimit && ti->playerlimit >= 0) ||
		!(g_q3f_allowedteams & 1 << level.clients[ClientNum].sess.sessionTeam) )
		return Q3F_TEAM_SPECTATOR;			// Can't pick this team, it's full or disallowed

	return( level.clients[ClientNum].sess.sessionTeam );	// Returns Q3F_TEAM_SPECTATOR if it failed to find a team
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}*/


/*
===========
ClientCheckName
============
*/
static void ClientCleanName( int clientNum, const char *in, char *out, int outSize ) {
	int		len, colorlessLen;
	char	ch;
	char	*p;
	int		spaces;
	char	buffer[128];
	int		i, a;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while( 1 ) {
		ch = *in++;
		if( !ch ) {
			break;
		}

		// don't allow leading spaces
		if( !*p && ch == ' ' ) {
			continue;
		}

		// check colors
		if( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if( !*in ) {
				break;
			}

			// don't allow black in a name, period
/*			if( ColorIndex(*in) == 0 ) {
				in++;
				continue;
			}*/

			// make sure room in dest for both chars
			if( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if( ch == ' ' ) {
			spaces++;
			if( spaces > 3 ) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "ETF_Player", outSize );
	}

	// djbob: stop colliding nicks
	// Perhaps add a list of nicks which will be forcefully changed
	Q_strncpyz( buffer, p, outSize );
	Q_CleanStr( buffer );

	a = 0 ;
	for( i = 0; i < MAX_CLIENTS; i++ ) {
		char buf[128], buf_2[128];

		if(!g_entities[i].inuse || g_entities[i].s.number == clientNum || (g_entities[i].client->pers.connected < CON_CONNECTING)) {
			continue;
		}

		if(a) {
			Com_sprintf(buf, outSize, "%d_%s", a, buffer);
		} else {
			Q_strncpyz( buf, buffer, outSize );
		}

		Q_strncpyz( buf_2, g_entities[i].client->pers.netname, outSize );
		Q_CleanStr( buf_2 );


		if(!Q_stricmp(buf_2, buf)) {
			a++;
			i = -1;
		}
	}

	if(a) {
      //Keeger -- seems somehow Com_sprintf borks when using the destination as an argument 
      //cause the original way produced "1_1_1_1_" garbage.
      Q_strncpyz( buffer, p, outSize );  //re-make buffer to colorized string
	  Com_sprintf(p, outSize, "%d_%s", a, buffer);  //replace the p arg with buffer and bug goes away...
	}
	// djbob
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
qboolean ClientUserinfoChanged( int clientNum, const char *reason ) {
	gentity_t *ent;
	const char	*s;
	char	oldname[MAX_NETNAME];
	gclient_t	*client;
	//char	c1[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate(userinfo) ) {
		G_Printf("Client %i Userinfo: %s\n", clientNum, userinfo );
		Q_strncpyz( ban_reason, "bad userinfo", sizeof(ban_reason) );
		if ( client && client->pers.connected != CON_DISCONNECTED )
			trap_DropClient( clientNum, ban_reason, 0 );
		return qfalse;
	}

	if ( client->pers.connected == CON_DISCONNECTED ) {
		// we just checked if connecting player can join server
		// so quit now as some important data like player team is still not set
		return qtrue;
	}

	/*s = Info_ValueForKey (userinfo, "cl_anonymous");
	if ( *s && strcmp(s, "0" ) ) {
		G_Q3F_AdminTempBan( ent, "q3unban exploit attempt", Q3F_ADMIN_TEMPBAN_TIME );
		return;
	}*/

	if (!client->sess.versionOK) {
		s = Info_ValueForKey( userinfo, "cg_etfversion" );
		if (s && !strcmp(s, FORTS_VERSION))
			client->sess.versionOK = qtrue;
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
		client->sess.adminLevel = ADMIN_FULL;
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !Q_atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// see if the player has opted out
	s = Info_ValueForKey( userinfo, "cg_unlagged" );
	if ( !Q_atoi( s ) ) {
		client->pers.unlagged = 0;
	} else {
		client->pers.unlagged = Q_atoi( s );
	}

	// see if the player is nudging his shots
	s = Info_ValueForKey( userinfo, "cl_timeNudge" );
	client->pers.timeNudge = Q_atoi( s );
	s = Info_ValueForKey( userinfo, "cg_cmdTimeNudge" );
	client->pers.cmdTimeNudge = Q_atoi( s );

	// see if the player wants to debug the backward reconciliation
	s = Info_ValueForKey( userinfo, "cg_debugDelag" );
	if ( !Q_atoi( s ) ) {
		client->pers.debugDelag = qfalse;
	}
	else {
		client->pers.debugDelag = qtrue;
	}

	// see if the player is simulating incoming latency
	s = Info_ValueForKey( userinfo, "cg_latentSnaps" );
	client->pers.latentSnaps = Q_atoi( s );

	// see if the player is simulating outgoing latency
	s = Info_ValueForKey( userinfo, "cg_latentCmds" );
	client->pers.latentCmds = Q_atoi( s );

	// see if the player is simulating outgoing packet loss
	s = Info_ValueForKey( userinfo, "cg_plOut" );
	client->pers.plOut = Q_atoi( s );

	s = Info_ValueForKey( userinfo, "cg_gender" );
	client->pers.gender = Q_atoi( s );
	if(client->pers.gender < GENDER_MALE) {
		client->pers.gender = GENDER_MALE;
	} else if(client->pers.gender > GENDER_FEMALE) {
		client->pers.gender = GENDER_FEMALE;
	}

	// set name.
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	ClientCleanName( clientNum, s, client->pers.netname, sizeof(client->pers.netname) );

	//if ( client->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
	if ( Q3F_IsSpectator(ent->client)) {	// RR2DO2
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, 
				client->pers.netname) );
		}
	}

	// colors
	//strcpy(c1, Info_ValueForKey( userinfo, "color" ));

	// Golliwog: Check for automatic weapon reload
	if(ent->r.svFlags & SVF_BOT)
	{
		client->pers.autoReload = 2;
		client->ps.persistant[PERS_FLAGS] |= PF_AUTORELOAD;
	}
	else
	{
		s = Info_ValueForKey( userinfo, "cg_autoReload" );
		// Canabis, updated weapon reload settings
		client->pers.autoReload = (s ? Q_atoi( s ) : 1);
		if ( client->pers.autoReload < 0 ) 
			client->pers.autoReload = 0;
		else if ( client->pers.autoReload > 3) 
			client->pers.autoReload = 3;
	}

	// Golliwog: Check for 'init' flag if they're reinitializing
	s = Info_ValueForKey( userinfo, "init" );
	if ( !Q_atoi( s ) ) {
		client->pers.initializing = qfalse;
	} else {
		client->pers.initializing = qtrue;
	}

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	s = va("n\\%s\\t\\%i\\cls\\%i\\g\\%i\\sc\\%i", client->pers.netname, client->sess.sessionTeam, client->ps.persistant[PERS_CURRCLASS], client->pers.gender, client->sess.shoutcaster );

	trap_GetConfigstring( CS_PLAYERS+clientNum, oldname, sizeof(oldname) );
	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// not changed
	if ( !Q_stricmp( oldname, s ) ) {
		return qtrue;
	}

#ifdef BUILD_LUA
	// *LUA* API callbacks
	// This only gets called when the ClientUserinfo is changed, replicating ETPro's behaviour.
	G_LuaHook_ClientUserinfoChanged(clientNum, reason);
#endif

	G_LogPrintf( "ClientUserinfoChanged (%s): %i %s\n", reason, clientNum, s );

	return qtrue;
}

/*
===========
G_Q3F_CheckClones

Check to see if there are any 'timed out' clients from the same address.
Too bad for timed-out clients on a LAN, of course...
===========
*/

static void G_Q3F_CheckClones( int clientNum, const char *ipStr ) {
	gentity_t *ent;
	for( ent = g_entities; ent < &g_entities[MAX_CLIENTS]; ent++ ) {
		if ( ent->s.number == clientNum )
			continue;
		if( ent->client && ent->inuse && (!(ent->r.svFlags & SVF_BOT)) &&
			(ent->client->ps.ping >= 999 || !ent->client->ps.ping) &&
			ent->client->activityTime < (level.time - 3000) &&
			*ent->client->pers.ipStr &&
			ent->client->pers.connected == CON_CONNECTED &&
			!strcmp( ent->client->pers.ipStr, ipStr ) ) {
			G_Q3F_DropClient( ent, "Suspected 'ghost' client." );
		}
	}
}

/*
============
G_StripPort
============
*/
static void G_StripPort( const char *in, char *out, int destsize ) {
	if ( !*in ) {
		out[0] = '\0';
		return;
	}

	// Only IPv4 can contain dots
	if (strchr(in, '.') != NULL) {
		const char *colon = strrchr(in, ':');
		if (colon) {
			Q_strncpyz(out, in, (destsize < colon - in + 1 ? destsize : colon - in + 1));
			return;
		}
	}

	// IPv6 addresses are enclosed within [] if they have a port attached
	if (in[0] == '[' && in[1] != '\0') {
		const char* bracket = strrchr(in, ']');
		if (bracket) {
			Q_strncpyz(out, in+1, (destsize < bracket - in + 1 ? destsize : bracket - in + 1));
			return;
		}
	}

	Q_strncpyz(out, in, destsize);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
const char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
	char		*reason;
	const char	*value, *name, *ip, *guid;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	char		cleanip[MAX_IP_LENGTH];
	//char		guid_str[33] = { 0 };
#ifdef BUILD_LUA
	char		lua_reason[MAX_STRING_CHARS] = "";
#endif
	gentity_t	*ent;
	qboolean	isLocal;

	if ( clientNum >= level.maxclients ) {
 		return "Bad connection slot.";
 	}

	ent = &g_entities[ clientNum ];

	// Golliwog: If this is a reconnection, destroy any existing buildings.
	// Mystery of the day: Why does it just pick client zero again for the 'new'
	// client since it thinks there's one already there (albeit timed out).
	// I hope it's cleverer than just "same IP, overwrite client".
	//
	// RR2DO2: MrElusive says: the only problem I could find was that when a client
	// times out... and then reconnects using the same ip address within a certain
	// amount of time... then client disconnect is not called!
	if ( firstTime ) {
		// cleanup previous data manually
		// because client may silently (re)connect without ClientDisconnect in case of crash for example
		if ( level.clients[ clientNum ].pers.connected != CON_DISCONNECTED || ent->inuse ) {
			G_LogPrintf( "Forcing disconnect on active client: %i\n", clientNum );
			ClientDisconnect( clientNum );
		}
		// remove old entity from the world
		trap_UnlinkEntity( ent );
		ent->r.contents = 0;
		ent->s.eType = ET_INVISIBLE;
		ent->s.solid = 0;
		ent->s.eFlags = 0;
		ent->s.modelindex = 0;
		ent->s.clientNum = clientNum;
		ent->s.number = clientNum;
		ent->takedamage = qfalse;

		/*if (ent->inuse)
		{
			if (client->sentry)
				G_Damage(client->sentry, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CUSTOM);
			if (client->supplystation)
				G_Damage(client->supplystation, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CUSTOM);
			G_Q3F_DropAllFlags(ent, qtrue, qtrue);		// RR2DO2: Drop any flags carried.
		}*/
	}

	ent->r.svFlags &= ~SVF_BOT;
	ent->inuse = qfalse;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	ip = Info_ValueForKey( userinfo, "ip" );
	G_StripPort( ip, cleanip, sizeof(cleanip) );
	name = Info_ValueForKey( userinfo, "name" );
	guid = Info_ValueForKey( userinfo, "cl_guid" );

	if ( !strcmp( ip, "localhost" ) && !isBot )
		isLocal = qtrue;
	else
		isLocal = qfalse;

	if ( ip[0] == '\0' || cleanip[0] == '\0' ) {
		trap_SendServerCommand( -1, va( "print \"Banned player: %s^7, tried to connect.\"", name ) );
		G_LogPrintf( "ClientConnect: [Connection Refused: q3unban exploit attempt] %i [%s] \"%s^7\"\n", clientNum, ip, name );
		return "Banned: q3unban exploit attempt";
	}

	if ( isLocal && !isBot && G_FilterPacket( ip, &reason ) ) {
		trap_SendServerCommand( -1, va( "print \"Banned player: %s^7, tried to connect.\"", name ) );
		G_LogPrintf( "ClientConnect: [Connection Refused: IP Banned] %i [%s] \"%s^7\"\n", clientNum, ip, name );
		return va("Banned: %s", reason);
	}

#ifdef BUILD_LUA
	// LUA API callbacks (check with Lua scripts)
	if ( G_LuaHook_ClientConnect( clientNum, firstTime, isBot, lua_reason ) ) {
		if ( !isBot ) {
			return va("You are excluded from this server. %s\n", lua_reason);
		}
	}
#endif

	if ( !isBot && !isLocal ) {
		// check for a password
		value = Info_ValueForKey( userinfo, "password" );
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) && strcmp( g_password.string, value) != 0) {
			char privpass[MAX_CVAR_VALUE_STRING];
			trap_Cvar_VariableStringBuffer( "sv_privatepassword", privpass, sizeof(privpass) );

			if ( privpass[0] ) {
				if(strcmp( privpass, value) != 0)
					return "Invalid password";
			}
			else
				return "Invalid password";
		}
	}

	ent->client = level.clients + clientNum;
	client = ent->client;

	memset( client, 0, sizeof( *client ) );

	if ( !isBot && !isLocal )
		G_Q3F_CheckClones( clientNum, cleanip );		// Golliwog: Kick any timed-out same-IP clients UNLESS I'M A BOT

	client->ps.clientNum = clientNum;

	if ( !ClientUserinfoChanged( clientNum, "connect" ) ) {
		return ban_reason;
	}

	if ( client->pers.ipStr[0] == '\0' || Q_stricmp( client->pers.ipStr, cleanip ) != 0 ) {
		Q_strncpyz( client->pers.ipStr, cleanip, sizeof(client->pers.ipStr) );
	}

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitClientSessionData( client );
	}
	else
		G_ReadClientSessionData( client );

	if ( isBot )
		Q_strncpyz( client->pers.guidStr, "BOT", sizeof(client->pers.guidStr));
	else if ( guid[0] == '\0' || !Q_stricmp( guid, "unknown") )
		Q_strncpyz( client->pers.guidStr, "NOGUID", sizeof(client->pers.guidStr));
	else
		Q_strncpyz( client->pers.guidStr, guid, sizeof(client->pers.guidStr) );

	if (isBot) {
		ent->s.number = clientNum;
		ent->r.svFlags |= SVF_BOT;
		client->sess.versionOK = qtrue;
	}

	ent->inuse = qtrue;

	// get and distribute relevent paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
#ifdef BUILD_BOTS
	Bot_Event_ClientConnected( ent, isBot );
#endif
#ifdef DREVIL_BOT_SUPPORT
	if( isBot )
		Bot_Interface_SendGlobalEvent( GAME_ID_BOTCONNECTED, clientNum, 0, 0 );
	else
		Bot_Interface_SendGlobalEvent( GAME_ID_CLIENTCONNECTED, clientNum, 0, 0 );
#endif

	client->pers.connected = CON_CONNECTING;
	client->pers.connectTime = level.time;

	ClientUserinfoChanged( clientNum, "connect" );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap_SendServerCommand( -1, va("print \"%s^7 connected\n\"", client->pers.netname) );
	}

	client->sess.lives = -2;		// Golliwog: Magic 'set lives appropriately' flag for first connection

	if ( g_teamAutoJoin.integer ) {	// RR2DO2
		if ( !firstTime ) {
			client->sess.sessionTeam = PickAutoJoinTeam( clientNum );
		}
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	if (g_unlagged.integer) {
		trap_SendServerCommand(clientNum, "print \"This server is Unlagged: full lag compensation is ON!\n\"");
	}
	else {
		trap_SendServerCommand(clientNum, "print \"This server is Unlagged: full lag compensation is OFF!\n\"");
	}

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	gentity_t	*tent;
	int			flags;
	int i;	// RR2DO2
	int spawn_count; // Ensiform, from ET so that CG_Respawn still happens

#ifdef BUILD_LUA
	// call LUA clientBegin only once when player connects
	qboolean firsttime = qfalse;
#endif

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

#ifdef BUILD_LUA
	if ( client->pers.connected == CON_CONNECTING ) {
		firsttime = qtrue;
	}
#endif

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	if ( client->pers.connected == CON_DISCONNECTED )
		return;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position

	// Ensiform added from ET
	// DHM - Nerve :: Also save PERS_SPAWN_COUNT, so that CG_Respawn happens
	spawn_count = client->ps.persistant[PERS_SPAWN_COUNT];

	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// Ensiform added from ET
	client->ps.persistant[PERS_SPAWN_COUNT] = spawn_count;

	// RR2DO2: no ammo boxes at this moment
	for ( i = 0; i < Q3F_MAX_AMMOBOXES; i++ ) {
		client->pers.AmmoBoxes[i] = NULL;
	}

	// locate ent at a spawn point
	ClientSpawn( ent );

	// RR2DO2: no class is no play!
	//if ( client->sess.sessionTeam != Q3F_TEAM_SPECTATOR && client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_NULL && client->ps.persistant[PERS_NEXTCLASS] != Q3F_CLASS_NULL ) {
	if ( !Q3F_IsSpectator(client) ) {
		// send event
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;

		trap_SendServerCommand( -1, va("print \"%s^7 entered the game\n\"", client->pers.netname) );
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

#ifdef BUILD_LUA
	// call LUA clientBegin only once
	if (firsttime == qtrue)
	{
		// LUA API callbacks
		G_LuaHook_ClientBegin(clientNum);
	}
#endif
}

// Golliwog: Need a touch function to reveal enemy agents etc.
void player_touch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	if( ent->client && ent->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT )
	{
		if( ent->client->agentdata && (ent->s.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE)) &&
			(((ent->client->agentdata->s.modelindex2 & 1) && !(ent->client->agentdata->s.modelindex2 & 2)) ||
			((ent->client->agentdata->s.modelindex2 & 4) && !(ent->client->agentdata->s.modelindex2 & 8))) &&
			!G_Q3F_IsAllied( other, ent ) && other->health && !Q3F_IsSpectator( other->client ) &&
			((other->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT) || other->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_RECON) )
		{
			trap_SendServerCommand( ent->s.number, va( "print \"%s ^7saw through your disguise!\n\"", other->client->pers.netname ) );
			trap_SendServerCommand( other->s.number, "print \"Look out, an agent!\n\"" );
			G_Q3F_StopAgentDisguise( ent );
			G_Q3F_StopAgentInvisible( ent );
		}
	}
	else if( other->client && other->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT )
		player_touch( other, ent, trace );
}

void G_RunTeleportTransition( gentity_t *ent )
{
	// Keep the ent following it's parent.

	if( !ent->parent->inuse )
		G_FreeEntity( ent );
	else {
		VectorCopy( ent->parent->r.currentOrigin, ent->s.origin );
		SnapVector( ent->s.origin );
		VectorCopy( ent->s.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
		G_RunThink( ent );
	}
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
qboolean ClientSpawn(gentity_t *ent) {
	int					index;
	vec3_t				spawn_origin, spawn_angles;
	gclient_t			*client;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	q3f_array_t			*savedchatchans;		// JT
	int					persistant[MAX_PERSISTANT];
	gentity_t			*spawnPoint, *chargeEntity, *sentry, *supplystation, *teleportTransitionEnt;
	int					flags, savedPing, i;
	g_q3f_playerclass_t	*cls;
	qboolean			beginclass = qfalse;
	g_q3f_location_t	*deathloc, *gren1loc, *gren2loc;
	int					eventSequence;
	char				userinfo[MAX_INFO_STRING];

	index	= ent - g_entities;
	client	= ent->client;
	cls		= G_Q3F_GetClass( &client->ps );

	client->lastSupplystationTouchTime = 0;

	// Golliwog: If class has changed, perform class term/init functions
	if( client->ps.persistant[PERS_CURRCLASS] != client->sess.sessionClass )
	{
		// RR2DO2: check for classlimits
		if( g_q3f_teamlist[ent->client->sess.sessionTeam].classmaximums[client->sess.sessionClass] == -1 ||
			g_q3f_teamlist[ent->client->sess.sessionTeam].classmaximums[client->sess.sessionClass] > G_Q3F_ClassCount(ent->client->sess.sessionTeam, client->sess.sessionClass) ||
			client->sess.sessionClass == Q3F_CLASS_MAX ) {
			beginclass = qtrue;
			if( client->sess.sessionClass == Q3F_CLASS_MAX )
				client->ps.persistant[PERS_CURRCLASS] = G_Q3F_SelectRandomClass( client->sess.sessionTeam, ent );
			else client->ps.persistant[PERS_CURRCLASS] = client->sess.sessionClass;
			ClientUserinfoChanged( client - level.clients, "spawn with new class" );
		}
	}
	else beginclass = qfalse;
	// Golliwog.

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if( client->sess.sessionTeam == Q3F_TEAM_SPECTATOR ||
		client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
			spawn_origin, spawn_angles);
	} else {
		spawnPoint = NULL;
		if ( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL || client->sess.sessionClass == Q3F_CLASS_NULL )
		{
			// the first spawn should be at a good looking spot
			spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles, ent );
		}
		else if(	(g_q3f_allowedteams & (1 << client->sess.sessionTeam)) &&
					(g_q3f_teamlist[client->sess.sessionTeam].classmaximums[client->ps.persistant[PERS_CURRCLASS]]) )
		{
			spawnPoint = SelectSpawnPoint(	client->ps.origin,
												spawn_origin, spawn_angles,
												ent );
		}
		if( !spawnPoint )
		{
			// Golliwog: Delay respawn if there's no free points

			client->respawnTime = level.time + 1000;
			client->respawnForce = qtrue;
			ent->client->ps.eFlags |= EF_Q3F_NOSPAWN;
			BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
			return( qfalse );
		}
	}

	// Golliwog: If class has changed, perform class term/init functions
	if( beginclass )
	{
		if( cls->EndClass )
			cls->EndClass( ent );		// End current class
		ent->s.otherEntityNum2 = client->ps.persistant[PERS_CURRCLASS];						// JT - Set so that the cgame can know.
		cls = G_Q3F_GetClass( &client->ps );	// Get new class pointer
	}
	// Golliwog

	client->pers.teamState.state = TEAM_ACTIVE;

	// Golliwog: Send a menu command
	if( client->sess.sessionTeam == Q3F_TEAM_SPECTATOR )
		G_Q3F_SendTeamMenu( ent, qfalse );
	else if( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL )
		G_Q3F_SendClassMenu( ent, 0 );

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// Reset historical data
	G_ResetHistory( ent );
	// and this is as good a time as any to clear the saved state
	ent->client->saved.leveltime = 0;

	// clear everything but the persistant data

	saved = client->pers;
	savedchatchans = client->chatchannels;		// JT
	savedSess = client->sess;
	savedPing = client->ps.ping;
	deathloc = client->deathLoc;
	gren1loc = client->gren1Loc;
	gren2loc = client->gren2Loc;
	chargeEntity = client->chargeEntity;
	sentry = client->sentry;
	supplystation = client->supplystation;
	teleportTransitionEnt = client->teleportTransitionEnt;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}

	eventSequence = client->ps.eventSequence;

	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->chatchannels = savedchatchans;		// JT
	client->sess = savedSess;	
	client->ps.ping = savedPing;
	client->deathLoc = deathloc;
	client->gren1Loc = gren1loc;
	client->gren2Loc = gren2loc;
	client->chargeEntity = chargeEntity;
	client->sentry = sentry;
	client->supplystation = supplystation;
	client->teleportTransitionEnt = teleportTransitionEnt;
	client->lastkilled_client = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}

	client->ps.eventSequence = eventSequence;

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + HOLDBREATHTIME;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );

	// clear entity values
	client->ps.eFlags = flags;

	client->ps.groundEntityNum = ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->touch = player_touch;		// Golliwog: Reveal enemy agents if recon/agent
	ent->pain = 0;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	client->ps.clientNum = index;

	client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_AXE );

	Q3F_SetupClass(ent);

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

#ifdef DREVIL_BOT_SUPPORT
	Bot_Interface_SendEvent(MESSAGE_SPAWN, ent->s.clientNum, 0, 0, 0);
#endif

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

	//if ( ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR || client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL || client->ps.persistant[PERS_NEXTCLASS] == Q3F_CLASS_NULL) {
	if (Q3F_IsSpectator(ent->client)) {	// RR2DO2
	} else {
		G_KillBox( ent );
		trap_LinkEntity (ent);

		// force the base weapon up
		client->ps.weapon = BG_Q3F_GetClass(&(ent->client->ps))->defweapon;
		client->ps.weaponstate = WEAPON_READY;
	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;

	// set default animations
	client->ps.torsoAnim = PM_GetIdleAnim( client->ps.weapon, client->ps.persistant[PERS_CURRCLASS] );
	client->ps.legsAnim = ANI_MOVE_IDLESTAND;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	}
	// Golliwog: Trigger spawn point if required.
	else if( spawnPoint && !Q3F_IsSpectator( client ) )
		G_Q3F_TriggerEntity( spawnPoint, ent, Q3F_STATE_ACTIVE, NULL, qfalse );

#ifdef BUILD_LUA
	// *LUA* API callbacks
	// FIXME should only call if real class maybe?
	// IE: Skip the team-but-no-class initial spawn
	// And what about no room to spawn cases?
	G_LuaHook_ClientSpawn(ent - g_entities);
#endif

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

	// Golliwog: Perform class init
	if( beginclass && cls->BeginClass )
		cls->BeginClass( ent );
	// Golliwog.

	// positively link the client, even if the command times are weird
	//if ( ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR && client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_NULL ) {
	if ( !Q3F_IsSpectator(ent->client) ) {	// RR2DO2
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

	// Golliwog: Cancel any inappropriate menus that may be up (well, ANY menus).
	if( !Q3F_IsSpectator( ent->client ) )
		trap_SendServerCommand( ent->s.number, "menu cancel" );

	// Set speed from any goalitems carried over after death
	ent->client->speedscale = G_Q3F_CalculateGoalItemSpeedScale( ent );

	// Golliwog: Set engineer building data once more
	G_Q3F_UpdateEngineerStats( ent );

	// Canabis: Slightly delay first time you can shoot a weapon
	ent->client->ps.weaponTime = 500;

	return( qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;
	g_q3f_playerclass_t *cls;

	ent = g_entities + clientNum;

	if ( !ent->client || ent->client->pers.connected == CON_DISCONNECTED ) {
		return;
	}

#ifdef BUILD_LUA
	// LUA API callbacks
	G_LuaHook_ClientDisconnect(clientNum);
#endif

#ifdef BUILD_BOTS
	Bot_Event_ClientDisConnected( ent );
#endif

	if(ent->client->pers.netname[0]) {
		trap_SendServerCommand( -1, va("print \"%s ^1has disconnected.\n\"", ent->client->pers.netname ) );
	}

	// Slothy - used to zero out kill stats
	tent = G_TempEntity( ent->client->ps.origin, EV_DISCONNECT );
	tent->s.clientNum = ent->s.clientNum;

	// stop any following clients
	// Ensiform: try to follow next client
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == Q3F_TEAM_SPECTATOR
			&& ( level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW || level.clients[i].sess.spectatorState == SPECTATOR_CHASE )
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			Cmd_FollowCycle_f( &g_entities[i], 1, level.clients[i].sess.spectatorState );
			//StopFollowing( &g_entities[i] );
		}
	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

#ifdef DREVIL_BOT_SUPPORT
	Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTDISCONNECTED, clientNum, 0,0);
#endif
	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED
		&& !Q3F_IsSpectator(ent->client) ) {	// RR2DO2

		if( ent->health > 0 )		// Kill them first :)
			G_Damage( ent, NULL, ent, NULL, NULL, 10000, DAMAGE_NO_PROTECTION, MOD_DISCONNECT );

		// RR2DO2: we dont like this (well, "we" is a factor that can be discussed about ;)
		//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		//tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		ent->client->pers.connected = CON_DISCONNECTED;	// Golliwog: Stop infinite loops with target_cycle
		G_Q3F_DropAllFlags( ent, qtrue, qtrue );		// Golliwog: Drop any flags carried.

		// Golliwog: Do any per-class cleanup
		cls = G_Q3F_GetClass( &ent->client->ps );
		if( cls->DeathCleanup )
			cls->DeathCleanup( ent );
		if( cls->EndClass )
			cls->EndClass( ent );
		// Golliwog;
	}

	trap_UnlinkEntity (ent);
	ent->r.contents = 0;
 	ent->s.eType = ET_INVISIBLE;
 	ent->s.solid = 0;
 	ent->s.eFlags = 0;
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = Q3F_TEAM_FREE;
	ent->client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
	ent->client->sess.sessionClass = Q3F_CLASS_NULL;
	ent->client->sess.sessionTeam = Q3F_TEAM_FREE;
	ent->takedamage = qfalse;

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	// Golliwog: Remove allocated memory for chat
	G_Q3F_ArrayDestroy( ent->client->chatchannels );
	ent->client->chatchannels = NULL;
	// Golliwog.
}
