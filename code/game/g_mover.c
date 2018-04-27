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
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"
#include "g_bot_interface.h"

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

#define DOOR_STARTOPEN	0x1
#define DOOR_CRUSHER	0x4
#define DOOR_TOGGLE		0x8

void MatchTeam( gentity_t *teamLeader, int moverState, int time );

#define PUSH_STACK_DEPTH 3
int pushedStackDepth = 0;

typedef struct {
	gentity_t	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_GENTITIES*PUSH_STACK_DEPTH], *pushed_p;		// Arnout *PUSH_STACK_DEPTH to prevent overflows

/*
============
G_TestEntityPosition

============
*/
gentity_t	*G_TestEntityPosition( gentity_t *ent ) {
	// Golliwog: Modified to stop entities going through solid objects at high speed.

	trace_t	tr;
	int		mask;

	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID;
	}
	if ( ent->client ) {
		trap_Trace( &tr, ent->client->ps.origin, ent->r.mins, ent->r.maxs, ent->client->ps.origin, ent->s.number, mask );
	} else {
		trap_Trace( &tr, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, ent->s.pos.trBase, ent->s.number, mask );
	}
	
	if( tr.startsolid )
		return &g_entities[ tr.entityNum ];
		
	return NULL;
}

qboolean G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove ) {
	vec3_t		org, org2, move2;
	gentity_t	*block;
	vec3_t		matrix[3], transpose[3];
	float		x, fx, y, fy, z, fz;
#define	JITTER_INC	4
#define	JITTER_MAX	(check->r.maxs[0]/2.0)

	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if ( ( pusher->s.eFlags & EF_MOVER_STOP ) && 
		check->s.groundEntityNum != pusher->s.number ) {
		return qfalse;
	}

	// save off the old position
	if (pushed_p > &pushed[MAX_GENTITIES]) {
		G_Error( "pushed_p > &pushed[MAX_GENTITIES]" );
	}
	pushed_p->ent = check;
	VectorCopy (check->s.pos.trBase, pushed_p->origin);
	VectorCopy (check->s.apos.trBase, pushed_p->angles);
	if ( check->client ) {
		pushed_p->deltayaw = check->client->ps.delta_angles[YAW];
		VectorCopy (check->client->ps.origin, pushed_p->origin);
	}
	pushed_p++;

	// try moving the contacted entity 
	VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);
	if (check->client) {
		// make sure the client's view rotates when on a rotating mover
		check->client->ps.delta_angles[YAW] += ANGLE2SHORT(amove[YAW]);
	}

	// figure movement due to the pusher's amove
	CreateRotationMatrix( amove, transpose );
	TransposeMatrix( transpose, matrix );	
	if ( check->client ) {
		VectorSubtract (check->client->ps.origin, pusher->r.currentOrigin, org);
	} else {
		VectorSubtract (check->s.pos.trBase, pusher->r.currentOrigin, org);
	}
	VectorRotate( org, matrix, org2);
	VectorSubtract (org2, org, move2);
	VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);
	if ( check->client ) {
		VectorAdd (check->client->ps.origin, move, check->client->ps.origin);
		VectorAdd (check->client->ps.origin, move2, check->client->ps.origin);
	}

	// may have pushed them off an edge
	if ( check->s.groundEntityNum != pusher->s.number ) {
		check->s.groundEntityNum = -1;
	}

	block = G_TestEntityPosition( check );
	if (!block) {
		// pushed ok
		if ( check->client ) {
			VectorCopy( check->client->ps.origin, check->r.currentOrigin );
		} else {
			VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
		}
		return qtrue;
	}

	// Arnout, if blocking entity is a player, try to move this player first.

	if( block->client ) {
		pushedStackDepth++;
		if( pushedStackDepth < PUSH_STACK_DEPTH && G_TryPushingEntity( block, pusher, move, amove ) ) {
			// pushed ok
			if ( check->client ) {
				VectorCopy( check->client->ps.origin, check->r.currentOrigin );
			} else {
				VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
			}
			return qtrue;
		}
		pushedStackDepth--;
	}

	// RF, if still not valid, move them around to see if we can find a good spot
	if (JITTER_MAX > JITTER_INC) {
		if ( check->client )
			VectorCopy (check->client->ps.origin, org);
		else 
			VectorCopy( check->s.pos.trBase, org );
		for (z=0; z<JITTER_MAX; z+=JITTER_INC)
		for (fz=-z; fz<=z; fz+=2*z) {
			for (x=JITTER_INC; x<JITTER_MAX; x+=JITTER_INC)
			for (fx=-x; fx<=x; fx+=2*x) {
				for (y=JITTER_INC; y<JITTER_MAX; y+=JITTER_INC)
				for (fy=-y; fy<=y; fy+=2*y) {
					VectorSet( move2, fx, fy, fz );
					VectorAdd( org, move2, org2 );
					VectorCopy( org2, check->s.pos.trBase );
					if ( check->client ) {
						VectorCopy (org2, check->client->ps.origin);
					}

					//
					// do the test
					block = G_TestEntityPosition( check );
					if (!block) {
						// pushed ok
						if ( check->client ) {
							VectorCopy( check->client->ps.origin, check->r.currentOrigin );
						} else {
							VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
						}
						return qtrue;
					}
				}
			}
			if (!fz) break;
		}
		// didnt work, so set the position back
		VectorCopy( org, check->s.pos.trBase );
		if ( check->client ) {
			VectorCopy (org, check->client->ps.origin);
		}
	}

	// if it is ok to leave in the old position, do it
	// this is only relevent for riding entities, not pushed
	// Sliding trapdoors can cause this.
	if ( check->client )
		VectorCopy( (pushed_p-1)->origin, check->client->ps.origin);
	else 
		VectorCopy( (pushed_p-1)->origin, check->s.pos.trBase);
	VectorCopy( (pushed_p-1)->angles, check->s.apos.trBase );
	block = G_TestEntityPosition( check );
	if ( !block ) {
		check->s.groundEntityNum = -1;
		pushed_p--;
		return qtrue;
	}

	// blocked
 	return qfalse;
}

/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If qfalse is returned, *obstacle will be the blocking entity
============
*/
qboolean G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle ) {
	int			i, e;
	gentity_t	*check;
	vec3_t		mins, maxs;
	pushed_t	*p;
	pushed_t	*work;
	int			entityList[MAX_GENTITIES];
	int			moveList[MAX_GENTITIES];
	int			listedEntities, moveEntities;
	vec3_t		totalMins, totalMaxs;

	*obstacle = NULL;

	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( pusher->r.currentAngles[0] || pusher->r.currentAngles[1] || pusher->r.currentAngles[2]
		|| amove[0] || amove[1] || amove[2] ) {
		float		radius;

		radius = RadiusFromBounds( pusher->r.mins, pusher->r.maxs );
		for ( i = 0; i < 3; i++ ) {
			mins[i] = pusher->r.currentOrigin[i] - radius + move[i];
			maxs[i] = pusher->r.currentOrigin[i] + radius + move[i];
			totalMins[i] = pusher->r.currentOrigin[i] - radius;
			totalMaxs[i] = pusher->r.currentOrigin[i] + radius;
		}
	} else {
		for ( i = 0; i < 3; i++ ) {
			mins[i] = pusher->r.absmin[i] + move[i];
			maxs[i] = pusher->r.absmax[i] + move[i];
		}

		VectorCopy( pusher->r.absmin, totalMins );
		VectorCopy( pusher->r.absmax, totalMaxs );
	}
	for ( i = 0; i < 3; i++ ) {
		if ( move[i] > 0 ) {
			totalMaxs[i] += move[i];
		} else {
			totalMins[i] += move[i];
		}
	}

	// unlink the pusher so we don't get it in the entityList
	trap_UnlinkEntity( pusher );

	listedEntities = trap_EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES );

	// move the pusher to it's final position
	VectorAdd( pusher->r.currentOrigin, move, pusher->r.currentOrigin );
	VectorAdd( pusher->r.currentAngles, amove, pusher->r.currentAngles );
	trap_LinkEntity( pusher );

	moveEntities = 0;
	// see if any solid entities are inside the final position
	for ( e = 0 ; e < listedEntities ; e++ ) {
		check = &g_entities[ entityList[ e ] ];

		// only push items and players
		if ( check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && !check->physicsObject ) {
			continue;
		}

		if ( check->client && check->client->noclip ) {
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->s.groundEntityNum != pusher->s.number ) {
			// see if the ent needs to be tested
			if ( check->r.absmin[0] >= maxs[0]
			|| check->r.absmin[1] >= maxs[1]
			|| check->r.absmin[2] >= maxs[2]
			|| check->r.absmax[0] <= mins[0]
			|| check->r.absmax[1] <= mins[1]
			|| check->r.absmax[2] <= mins[2] ) {
				continue;
			}
			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if (G_TestEntityPosition( check ) != pusher) {
				continue;
			}
		}

		moveList[moveEntities++] = entityList[e];
	}

	// unlink all to be moved entities so they cannot get stuck in each other
	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		trap_UnlinkEntity( check );
	}

	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		// the entity needs to be pushed
		pushedStackDepth = 0;	// Arnout: new push, reset stack depth
		if ( G_TryPushingEntity( check, pusher, move, amove ) ) {
			// link it in now so nothing else tries to clip into us
			trap_LinkEntity( check );
			continue;
		}

		// the move was blocked an entity

		// bobbing entities are instant-kill and never get blocked
		if ( pusher->s.pos.trType == TR_SINE || pusher->s.apos.trType == TR_SINE ) {
			/* Ensiform - Nuke any stuck items so they don't go all over the place */
			if ( check->s.eType == ET_ITEM && (check->flags & FL_DROPPED_ITEM) )
				G_FreeEntity( check );
			else
				G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH );
			continue;
		}

		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		// rain - changed the loop slightly to avoid checking an invalid
		// pointer (do the -1 inside the loop, > instead of >=)
		for ( p=pushed_p ; p>pushed ; p-- ) {
			work = p - 1;

			VectorCopy (work->origin, work->ent->s.pos.trBase);
			VectorCopy (work->angles, work->ent->s.apos.trBase);
			if ( work->ent->client ) {
				work->ent->client->ps.delta_angles[YAW] = work->deltayaw;
				VectorCopy (work->origin, work->ent->client->ps.origin);
			}
 		}

		// link all entities at their original position
		for ( e = 0; e < moveEntities; e++ ) {
			check = &g_entities[ moveList[e] ];

			trap_LinkEntity( check );
		}
		// movement failed
		return qfalse;
	}
	// link all entities at their final position
	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		trap_LinkEntity( check );
	}
	// movement was successfull
	return qtrue;
}


/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( gentity_t *ent ) {
	vec3_t		move, amove;
	gentity_t	*part, *obstacle;
	vec3_t		origin, angles;

	obstacle = NULL;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain) {
		// get current position
		BG_EvaluateTrajectory( &part->s.pos, level.time, origin );
		BG_EvaluateTrajectory( &part->s.apos, level.time, angles );
		VectorSubtract( origin, part->r.currentOrigin, move );
		VectorSubtract( angles, part->r.currentAngles, amove );
		if ( !G_MoverPush( part, move, amove, &obstacle ) ) {
			break;	// move was blocked
		}
	}

	if (part) {
		// go back to the previous position
		for ( part = ent ; part ; part = part->teamchain ) {
			part->s.pos.trTime += level.time - level.previousTime;
			part->s.apos.trTime += level.time - level.previousTime;
			BG_EvaluateTrajectory( &part->s.pos, level.time, part->r.currentOrigin );
			BG_EvaluateTrajectory( &part->s.apos, level.time, part->r.currentAngles );
			trap_LinkEntity( part );
		}

		// if the pusher has a "blocked" function, call it
		if (ent->blocked) {
			ent->blocked( ent, obstacle );
		}
		return;
	}

	// the move succeeded
	for ( part = ent ; part ; part = part->teamchain ) {
		// call the reached function if time is at or past end point
		if ( part->s.pos.trType == TR_LINEAR_STOP ) {
			if ( level.time >= part->s.pos.trTime + part->s.pos.trDuration ) {
				if ( part->reached ) {
					part->reached( part );
				}
			}
		}
		// Golliwog: Allow rotating doors to do the same thing.
		else if ( part->s.apos.trType == TR_LINEAR_STOP ) {
			if ( level.time >= part->s.apos.trTime + part->s.apos.trDuration ) {
				if ( part->reached ) {
					part->reached( part );
				}
			}
		}
		// Golliwog.
	}
}

/*
================
G_RunMover

================
*/
void G_RunMover( gentity_t *ent ) {
	// if not a team captain, don't do anything, because
	// the captain will handle everything
	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	// if stationary at one of the positions, don't move anything
	if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) {
		G_MoverTeam( ent );
	}

	// check think function
	G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
===============
SetMoverState
===============
*/
void SetMoverState( gentity_t *ent, moverState_t moverState, int time ) {
	vec3_t			delta;
	float			f;
	trajectory_t	*trajectory;

	ent->moverState = moverState;
	trajectory = ent->watertype ? &ent->s.apos : &ent->s.pos;

	trajectory->trTime = time;
	switch( moverState ) {
	case MOVER_POS1:
		VectorCopy( ent->pos1, trajectory->trBase );
		trajectory->trType = TR_STATIONARY;
		break;
	case MOVER_POS2:
		VectorCopy( ent->pos2, trajectory->trBase );
		trajectory->trType = TR_STATIONARY;
		break;
	case MOVER_1TO2:
		VectorCopy( ent->pos1, trajectory->trBase );
		VectorSubtract( ent->pos2, ent->pos1, delta );
		f = 1000.0 / trajectory->trDuration;
		VectorScale( delta, f, trajectory->trDelta );
		trajectory->trType = TR_LINEAR_STOP;
		break;
	case MOVER_2TO1:
		VectorCopy( ent->pos2, trajectory->trBase );
		VectorSubtract( ent->pos1, ent->pos2, delta );
		f = 1000.0 / trajectory->trDuration;
		VectorScale( delta, f, trajectory->trDelta );
		trajectory->trType = TR_LINEAR_STOP;
		break;
	}

	BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );
	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
	trap_LinkEntity( ent );
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( gentity_t *teamLeader, int moverState, int time ) {
	gentity_t		*slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {
		SetMoverState( slave, moverState, time );
	}
}



/*
================
ReturnToPos1
================
*/
void ReturnToPos1( gentity_t *ent ) {
//	trace_t tr;

	MatchTeam( ent, MOVER_2TO1, level.time );

	// looping sound
	ent->s.loopSound = ent->soundLoop;

	// starting sound
	if ( ent->sound2to1 ) {
		if(ent->timestamp)
			G_AddEvent(ent, ent->timestamp, ent->sound2to1);
		else
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
	}
}


/*
================
Reached_BinaryMover
================
*/
void Reached_BinaryMover( gentity_t *ent ) {
	trace_t tr;

	// stop the looping sound
	ent->s.loopSound = ent->soundLoop;

	if ( ent->moverState == MOVER_1TO2 ) {
		// reached pos2
		SetMoverState( ent, MOVER_POS2, level.time );

		// play sound
		if ( ent->soundPos2 ) {
			if(ent->timestamp)
				G_AddEvent(ent, ent->timestamp, ent->soundPos2);
			else
				G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos2 );
		}

		// return to pos1 after a delay
		if( ent->wait < 0 || (ent->teammaster && ent->teammaster != ent) ) {
			ent->nextthink = 0;
		} else {
			ent->think = ReturnToPos1;
			ent->nextthink = level.time + ent->wait;
		}

		// fire targets
		if ( !ent->activator ) {
			ent->activator = ent;
		}
		G_UseTargets( ent, ent->activator );
	} else if ( ent->moverState == MOVER_2TO1 ) {
		// reached pos1
		SetMoverState( ent, MOVER_POS1, level.time );

		// play sound
		if ( ent->soundPos1 ) {
			if(ent->timestamp)
				G_AddEvent(ent, ent->timestamp, ent->soundPos1);
			else
				G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos1 );
		}

		// close areaportals
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qfalse );
		
		// Golliwog: Inactivate any active triggers
		if( ent->target_ent && ent->target_ent->mapdata && ent->target_ent->mapdata->state == Q3F_STATE_ACTIVE )
		{
			// Ensiform : This ensures the trigger still returns to inactive during ceasefire
			// Accompanies the other bugfix with doors/plats still triggering
			qboolean ceasefirestate = level.ceaseFire;
			if( g_mapentDebug.integer && ceasefirestate )
				G_Printf(">>>> Ignoring Ceasefire State on %s\n", ent->target_ent->classname);
			level.ceaseFire = qfalse;
			G_Q3F_TriggerEntity( ent->target_ent, ent->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, &tr, qtrue );
			level.ceaseFire = ceasefirestate;
			if( g_mapentDebug.integer && ceasefirestate )
				G_Printf("<<<< End Ignore Ceasefire State on %s\n", ent->target_ent->classname);
			ent->target_ent->mapdata->waittime = ent->target_ent->nextthink;
		}
		else if( ent->mapdata && ent->mapdata->state == Q3F_STATE_ACTIVE )
		{
			// Ensiform : This ensures the trigger still returns to inactive during ceasefire
			// Accompanies the other bugfix with doors/plats still triggering
			qboolean ceasefirestate = level.ceaseFire;
			if( g_mapentDebug.integer && ceasefirestate )
				G_Printf(">>>> Ignoring Ceasefire State on %s\n", ent->classname);
			level.ceaseFire = qfalse;
			G_Q3F_TriggerEntity( ent, ent->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, &tr, qtrue );
			level.ceaseFire = ceasefirestate;
			if( g_mapentDebug.integer && ceasefirestate )
				G_Printf("<<<< End Ignore Ceasefire State on %s\n", ent->classname);
			ent->mapdata->waittime = ent->nextthink;
		}
		// Golliwog
		}
	} else {
		G_Error( "Reached_BinaryMover: bad moverState" );
	}
}


/*
================
Use_BinaryMover
================
*/
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	int		total;
	int		partial;

	// only the master should be used
	if ( ent->flags & FL_TEAMSLAVE ) {
		Use_BinaryMover( ent->teammaster, other, activator);
		return;
	}

#ifdef BUILD_BOTS
	if (ent->target)
		Bot_Util_SendTrigger(ent, NULL, va("%s activated", ent->target), "pushed");
#endif

	ent->activator = activator;

	if( ent->target_ent && ent->target_ent->mapdata )
		ent->target_ent->mapdata->waittime = -1;	// Prevent other ent skipping to inactive
	else if( ent->mapdata )
		ent->mapdata->waittime = -1;	// Prevent this ent skipping to inactive

	if ( ent->moverState == MOVER_POS1 ) {
		// start moving 50 msec later, becase if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeam( ent, MOVER_1TO2, level.time + 50 );

		// starting sound
		if ( ent->sound1to2 ) {
			//G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
			G_AddEvent( ent, ent->timestamp, ent->sound1to2 );
		}

		// looping sound
		ent->s.loopSound = ent->soundLoop;

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qtrue );
		}

		return;
	}

	// if all the way up, just delay before coming down
	if ( (ent->moverState == MOVER_POS2) && ent->nextthink ) {
		ent->nextthink = level.time + ent->wait;
	}

	// only partway down before reversing
	if ( ent->moverState == MOVER_2TO1 ) {
		total = ent->watertype ? ent->s.apos.trDuration : ent->s.pos.trDuration;
		partial = level.time - (ent->watertype ? ent->s.apos.trTime : ent->s.pos.trTime);
		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_1TO2, level.time - ( total - partial ) );

		if ( ent->sound1to2 ) {
			//G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
			G_AddEvent( ent, ent->timestamp, ent->sound1to2 );
		}

		return;
	}

	// only partway up before reversing
	if ( ent->moverState == MOVER_1TO2 ) {
		total = ent->watertype ? ent->s.apos.trDuration : ent->s.pos.trDuration;
		partial = level.time - ent->s.pos.trTime;
		if ( partial > total ) {
			partial = total;
		}

		MatchTeam( ent, MOVER_2TO1, level.time - ( total - partial ) );

		if ( ent->sound2to1 ) {
			//G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
			G_AddEvent( ent, ent->timestamp, ent->sound2to1 );
		}

/*		if( ent->target_ent && ent->target_ent->mapdata && ent->target_ent->mapdata->state == Q3F_STATE_ACTIVE )
			G_Q3F_TriggerEntity( ent->target_ent, NULL, Q3F_STATE_INACTIVE, &tr, qfalse );
		else if( ent->mapdata && ent->mapdata->state == Q3F_STATE_ACTIVE )
			G_Q3F_TriggerEntity( ent, lastTriggerer, Q3F_STATE_INACTIVE, &tr, qfalse );*/

		return;
	}
}



/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMover( gentity_t *ent ) {
	vec3_t		move;
	float		distance;
	float		light;
	vec3_t		color;
	qboolean	lightSet, colorSet;
	char		*sound;
	trajectory_t *trajectory;

	trajectory = ent->watertype ? &ent->s.apos : &ent->s.pos;

	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2 ) {
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
	}

	// if the "loopsound" key is set, use a constant looping sound when moving
	if ( G_SpawnString( "noise", "100", &sound ) ) {
		ent->s.loopSound = G_SoundIndex( sound );
	}

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnColor( "1 1 1", color );
	if ( lightSet || colorSet ) {
		int		r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}


	ent->use = Use_BinaryMover;
	ent->reached = Reached_BinaryMover;

	ent->moverState = MOVER_POS1;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_MOVER;
	if( ent->watertype ) {
		VectorCopy( ent->pos1, ent->r.currentAngles );
	} else {
		VectorCopy( ent->pos1, ent->r.currentOrigin );
	}
	trap_LinkEntity (ent);

	trajectory->trType = TR_STATIONARY;
	VectorCopy( ent->pos1, trajectory->trBase );

	// calculate time to reach second position from speed
	VectorSubtract( ent->pos2, ent->pos1, move );
	distance = VectorLength( move );
	if ( ! ent->speed ) {
		ent->speed = 100;
	}
	VectorScale( move, ent->speed, trajectory->trDelta );
	trajectory->trDuration = distance * 1000 / ent->speed;
	if ( trajectory->trDuration <= 0 ) {
		trajectory->trDuration = 1;
	}
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
void Blocked_Door( gentity_t *ent, gentity_t *other ) {
	// remove anything other than a client (Golliwog: or charge... :)
	gentity_t *slave;
	if ( other ) {
		if ( other->client ) {
			if (ent->health <= 0 )
				G_Damage( other, ent, ent, NULL, NULL, 10000, 0, MOD_CRUSH );
			if (ent->damage )
				G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH );
		} else if (other->s.eType == ET_Q3F_SUPPLYSTATION || other->s.eType == ET_Q3F_SENTRY ) {
			G_Damage( other, ent, ent, NULL, NULL, 10000, 0, MOD_CRUSH );
		} else if( other->s.eType == ET_Q3F_GOAL ) {
			if (ent->spawnflags & DOOR_CRUSHER ) {
				G_Q3F_TriggerEntity( other, other->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, NULL, qfalse );
			}
		} else {
			G_TempEntity( other->s.origin, EV_ITEM_POP );
			G_FreeEntity( other );
		}
	}

	if ( ent->spawnflags & DOOR_CRUSHER ) {
		return;		// crushers don't reverse
	}

	// reverse direction
	ent->timestamp = EV_DOOR;
//	Use_BinaryMover( ent, other, ent->activator);
	for ( slave = ent ; slave ; slave = slave->teamchain ) 
	{	
		int time;
		if( slave->watertype ) {
			/* Rotating door */
			time = level.time - (slave->s.apos.trDuration - (level.time - slave->s.apos.trTime));
		} else {
			/* normal door */
			time = level.time - (slave->s.pos.trDuration - (level.time - slave->s.pos.trTime));
		}
		if (slave->moverState == MOVER_1TO2) {
			SetMoverState( slave, MOVER_2TO1, time);
		}
		else {
			SetMoverState( slave, MOVER_1TO2, time);
		}
		trap_LinkEntity(slave);
	}
}

void Use_Door( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	// Golliwog: Use a door (i.e. when triggered)

	if( !activator->client || Q3F_IsSpectator( activator->client )) 
		return;

	// Golliwog: Prevent it reversing while moving at all (except when blocked)
	if( !ent->parent && ent->moverState != MOVER_1TO2 && ent->moverState != MOVER_2TO1 )
	{
		ent->timestamp = EV_DOOR;
		Use_BinaryMover( ent, ent, activator);
	} else if( ent->parent && ent->parent->moverState != MOVER_1TO2 && ent->parent->moverState != MOVER_2TO1 )
	{
		ent->parent->timestamp = EV_DOOR;
		Use_BinaryMover( ent->parent, ent, activator);
	}
}

/*
================
Touch_DoorTrigger
================
*/
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || Q3F_IsSpectator( other->client ) )
		return;

	// Ensiform : Added so that you cannot touch door triggers during ceasefire toggle
	if( level.ceaseFire )
		return;			// Nothing is triggered during ceasefires

	// Golliwog: Prevent it reversing while moving at all (except when blocked)
	if( !ent->parent && ent->moverState != MOVER_1TO2 && ent->moverState != MOVER_2TO1 ) {
		ent->timestamp = EV_DOOR;
		Use_BinaryMover( ent, ent, other);
	} else if( ent->parent && ent->parent->moverState != MOVER_1TO2 && ent->parent->moverState != MOVER_2TO1 ) {
		ent->parent->timestamp = EV_DOOR;
		Use_BinaryMover( ent->parent, ent, other);
	}
	// Golliwog.
}

/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTrigger( gentity_t *ent ) {
	gentity_t		*other;
	vec3_t		mins, maxs, pos;
	int			i, best;

	// set all of the slaves as shootable
	for ( other = ent ; other ; other = other->teamchain ) {
		// Only make them shootable if they're marked so.
		if( other->mapdata && (other->mapdata->flags & Q3F_FLAG_SHOOTABLE) )
			other->takedamage = qtrue;
	}

	// find the bounds of everything on the team
	VectorCopy (ent->r.absmin, mins);
	VectorCopy (ent->r.absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain) {
		AddPointToBounds (other->r.absmin, mins, maxs);
		AddPointToBounds (other->r.absmax, mins, maxs);
		if( other->watertype )
		{
			// It's a rotating door, add it's (horizontally) "rotated" bounds as well,

			RotatePointAroundVector( pos, ent->r.mins, ent->r.currentOrigin, ent->pos2[YAW] );
			AddPointToBounds( pos, mins, maxs );
			RotatePointAroundVector( pos, ent->r.maxs, ent->r.currentOrigin, ent->pos2[YAW] );
			AddPointToBounds( pos, mins, maxs );
		}
	}

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for ( i = 1 ; i < 3 ; i++ ) {
		if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
			best = i;
		}
	}
	maxs[best] += 120;
	mins[best] -= 120;

	// create a trigger with this size
	other = G_Spawn ();
	other->classname = "door_trigger";
	VectorCopy (mins, other->r.mins);
	VectorCopy (maxs, other->r.maxs);
	other->parent = ent;
	other->r.contents = CONTENTS_TRIGGER;
	other->touch = Touch_DoorTrigger;
	// remember the thinnest axis
	other->count = best;
	G_Q3F_CopyMapData( ent, other );
	G_Q3F_AddEntityToTargetArray( other );
	trap_LinkEntity (other);

	if( other->mapdata )
		other->mapdata->flags |= Q3F_FLAG_RETOUCH;	// Allow retouches without triggering

	ent->think = NULL;	// just to be safe

	MatchTeam( ent, ent->moverState, level.time );

	// set all of the slaves as controlled by this trigger
	for( ; ent; ent = ent->teamchain )
		ent->target_ent = other;
}


void Think_MatchTeam( gentity_t *ent ) {
	MatchTeam( ent, ent->moverState, level.time );
}
// RR2DO2

static void G_Q3F_SetMoverSound( gentity_t *ent )
{
	// Golliwog: Use custom sounds if if specified

	q3f_keypair_t *kp;

	if( ent->mapdata && ent->mapdata->other )
	{
		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "reachstart_sound" ) );
		if( kp )
			ent->soundPos1 = *kp->value.d.strdata ? G_SoundIndex( kp->value.d.strdata ) : 0;
		else ent->soundPos1 = G_SoundIndex("sound/movers/doors/dr1_end.wav");

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "reachend_sound" ) );
		if( kp )
			ent->soundPos2 = *kp->value.d.strdata ? G_SoundIndex( kp->value.d.strdata ) : 0;
		else ent->soundPos2 = G_SoundIndex("sound/movers/doors/dr1_end.wav");

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "leavestart_sound" ) );
		if( kp )
			ent->sound1to2 = *kp->value.d.strdata ? G_SoundIndex( kp->value.d.strdata ) : 0;
		else ent->sound1to2 = G_SoundIndex("sound/movers/doors/dr1_strt.wav");

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "leaveend_sound" ) );
		if( kp )
			ent->sound2to1 = *kp->value.d.strdata ? G_SoundIndex( kp->value.d.strdata ) : 0;
		else ent->sound2to1 = G_SoundIndex("sound/movers/doors/dr1_strt.wav");
	}
	else {
		ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/doors/dr1_strt.wav");
		ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/doors/dr1_end.wav");
	}
}

/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
*/
void SP_func_door (gentity_t *ent) {
	vec3_t	abs_movedir;
	float	distance;
	vec3_t	size;
	float	lip;

	G_Q3F_SetMoverSound( ent );

	ent->blocked = Blocked_Door;

	// default speed of 400
	if (!ent->speed)
		ent->speed = 400;

	// default wait of 2 seconds
	if (!ent->wait)
		ent->wait = 2;
	ent->wait *= 1000;

	// default lip of 8 units
	G_SpawnFloat( "lip", "8", &lip );

	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage );

	// first position at start
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	trap_SetBrushModel( ent, ent->model );
	G_SetMovedir (ent->s.angles, ent->movedir);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = DotProduct( abs_movedir, size ) - lip;
	VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

	// if "start_open", reverse position 1 and 2
	if ( ent->spawnflags & DOOR_STARTOPEN ) {
		vec3_t	temp;

		VectorCopy( ent->pos2, temp );
		VectorCopy( ent->s.origin, ent->pos2 );
		VectorCopy( temp, ent->pos1 );
	}

	InitMover( ent );

	ent->nextthink = level.time + FRAMETIME;
	if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
		int health;

		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
// XreaL BEGIN
#ifdef _ETXREAL
		// Doom 3 mapping convention: every entity has a name
		if ( (ent->targetname && !ent->targetnameAutogenerated) || health ) {
#else
		if ( ent->targetname || health ) {
#endif
// XreaL END
			// non touch/shoot doors
			ent->think = Think_MatchTeam;
		} else {
			ent->think = Think_SpawnNewDoorTrigger;
		}
		ent->use = Use_Door;	// Not touchable, but triggerable
	}
}

void SP_func_door_rotating (gentity_t *ent) {
//	vec3_t	abs_movedir;
//	float	distance;
//	vec3_t	size;
//	float	lip;

	G_Q3F_SetMoverSound( ent );

	ent->blocked = Blocked_Door;
	ent->watertype = 1;				// Mark it as a rotating door.

	// default speed of 400
	if (!ent->speed)
		ent->speed = 45;

	// default wait of 2 seconds
	if (!ent->wait)
		ent->wait = 2;
	ent->wait *= 1000;

	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage );

	trap_SetBrushModel( ent, ent->model );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

	trap_LinkEntity( ent );

	// calculate second position
	VectorSet( ent->pos2, 0, 0, 0 );
	G_SpawnVector( "angles", "0 90 0", ent->pos2 );

	// if "start_open", reverse position 1 and 2
	if ( ent->spawnflags & DOOR_STARTOPEN ) {
		vec3_t	temp;

		VectorCopy( ent->pos2, temp );
		VectorCopy( ent->s.origin, ent->pos2 );
		VectorCopy( temp, ent->pos1 );
	}

	InitMover( ent );

	ent->nextthink = level.time + FRAMETIME;
	if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
		int health;

		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
// XreaL BEGIN
#ifdef _ETXREAL
		// Doom 3 mapping convention: every entity has a name
		if ( (ent->targetname && !ent->targetnameAutogenerated) || health ) {
#else
		if ( ent->targetname || health ) {
#endif
// XreaL END
			// non touch/shoot doors
			ent->think = Think_MatchTeam;
		} else {
			ent->think = Think_SpawnNewDoorTrigger;
		}
		ent->use = Use_Door;	// Not touchable, but triggerable
	}
}

/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
void Touch_Plat( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || other->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// delay return-to-pos1 by one second
	if ( ent->moverState == MOVER_POS2 ) {
		ent->nextthink = level.time + 1000;
	}
}

void Use_Plat( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	if ( !activator->client ) {
		return;
	}

	if( ent->parent && ent->parent->moverState == MOVER_POS1 ) {
		ent->parent->timestamp = EV_LIFT;
		Use_BinaryMover( ent->parent, other, activator  );
	} else if( !ent->parent && ent->moverState == MOVER_POS1 ) {
		ent->timestamp = EV_LIFT;
		Use_BinaryMover( ent, other, activator  );
	}
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_PlatCenterTrigger(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || Q3F_IsSpectator( other->client ) ) {
		return;
	}

	// Ensiform : Added so that you cannot touch door triggers during ceasefire toggle
	if( level.ceaseFire )
		return;			// Nothing is triggered during ceasefires

	ent->parent->timestamp = EV_LIFT;
	if( ent->parent && ent->parent->moverState == MOVER_POS1 )
		Use_BinaryMover( ent->parent, ent, other );
}


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger( gentity_t *ent ) {
	gentity_t	*trigger;
	vec3_t	tmin, tmax;

	// the middle trigger will be a thin trigger just
	// above the starting position
	trigger = G_Spawn();
	trigger->classname = "plat_trigger";
	trigger->touch = Touch_PlatCenterTrigger;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->parent = ent;
	ent->target_ent = trigger;

	tmin[0] = ent->pos1[0] + ent->r.mins[0] + 33;
	tmin[1] = ent->pos1[1] + ent->r.mins[1] + 33;
	tmin[2] = ent->pos1[2] + ent->r.mins[2];

	tmax[0] = ent->pos1[0] + ent->r.maxs[0] - 33;
	tmax[1] = ent->pos1[1] + ent->r.maxs[1] - 33;
	tmax[2] = ent->pos1[2] + ent->r.maxs[2] + 8;

	if ( tmax[0] <= tmin[0] ) {
		tmin[0] = ent->pos1[0] + (ent->r.mins[0] + ent->r.maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if ( tmax[1] <= tmin[1] ) {
		tmin[1] = ent->pos1[1] + (ent->r.mins[1] + ent->r.maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy (tmin, trigger->r.mins);
	VectorCopy (tmax, trigger->r.maxs);

	G_Q3F_CopyMapData( ent, trigger );
	G_Q3F_AddEntityToTargetArray( ent );
	if( trigger->mapdata )
		trigger->mapdata->flags |= Q3F_FLAG_RETOUCH;	// Flag can be retouched without triggering

	trap_LinkEntity (trigger);
}


/*QUAKED func_plat (0 .5 .8) ?
Plats are always drawn in the extended position so they will light correctly.

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_plat (gentity_t *ent) {
	float		lip, height;

	G_Q3F_SetMoverSound( ent );
	ent->timestamp = EV_LIFT;

	VectorClear (ent->s.angles);

	G_SpawnFloat( "speed", "200", &ent->speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "wait", "1", &ent->wait );
	G_SpawnFloat( "lip", "8", &lip );

	/* Ensiform - Lets print a note to mappers/server about their custom waits on func_plats that don't actually work */
	/* Doing this because we don't want to fuck up any timings of existing maps */
	if ( ent->wait != 1.f ) {
		G_Printf ("func_plat with custom wait set to %.2f being ignored (forced to 1 second)!\n", ent->wait);
	}

	ent->wait = 1000;

	// create second position
	trap_SetBrushModel( ent, ent->model );

	if ( !G_SpawnFloat( "height", "0", &height ) ) {
		height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;
	}

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy( ent->s.origin, ent->pos2 );
	VectorCopy( ent->pos2, ent->pos1 );
	ent->pos1[2] -= height;

	InitMover( ent );

	// touch function keeps the plat from returning while
	// a live player is standing on it
//	ent->touch = Touch_Plat;

	ent->blocked = Blocked_Door;

	ent->parent = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
// XreaL BEGIN
#ifdef _ETXREAL
	if ( !ent->targetname || ent->targetnameAutogenerated )
#else
	if ( !ent->targetname )
#endif
// XreaL END
		SpawnPlatTrigger(ent);
	ent->use = Use_Plat;
}


/*
===============================================================================

LIFT

===============================================================================
*/

/*
==============
Touch_Lift

Don't allow decent if a living player is on it
===============
*/
void Touch_Lift( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || other->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// delay return-to-pos1 by one second
	if ( ent->moverState == MOVER_POS2 ) {
		ent->nextthink = level.time + 1000;
	}
}

void Use_Lift( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	if ( !activator->client ) {
		return;
	}

	if( ent->parent && ent->parent->moverState == MOVER_POS1 ) {
		ent->parent->timestamp = EV_LIFT;
		Use_BinaryMover( ent->parent, other, activator  );
	} else if( !ent->parent && ent->moverState == MOVER_POS1 ) {
		ent->timestamp = EV_LIFT;
		Use_BinaryMover( ent, other, activator  );
	}
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_LiftCenterTrigger(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || Q3F_IsSpectator( other->client ) ) {
		return;
	}

	// Ensiform : Added so that you cannot touch door triggers during ceasefire toggle
	if( level.ceaseFire )
		return;			// Nothing is triggered during ceasefires

	ent->parent->timestamp = EV_LIFT;
	if( ent->parent && ent->parent->moverState == MOVER_POS1 )
		Use_BinaryMover( ent->parent, ent, other );
}


/*
================
SpawnLiftTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnLiftTrigger( gentity_t *ent ) {
	gentity_t	*trigger;
	vec3_t	tmin, tmax;

	// the middle trigger will be a thin trigger just
	// above the starting position
	trigger = G_Spawn();
	trigger->classname = "lift_trigger";
	trigger->touch = Touch_PlatCenterTrigger;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->parent = ent;
	ent->target_ent = trigger;

	tmin[0] = ent->pos1[0] + ent->r.mins[0] + 33;
	tmin[1] = ent->pos1[1] + ent->r.mins[1] + 33;
	tmin[2] = ent->pos1[2] + ent->r.mins[2];

	tmax[0] = ent->pos1[0] + ent->r.maxs[0] - 33;
	tmax[1] = ent->pos1[1] + ent->r.maxs[1] - 33;
	tmax[2] = ent->pos1[2] + ent->r.maxs[2] + 8;

	if ( tmax[0] <= tmin[0] ) {
		tmin[0] = ent->pos1[0] + (ent->r.mins[0] + ent->r.maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if ( tmax[1] <= tmin[1] ) {
		tmin[1] = ent->pos1[1] + (ent->r.mins[1] + ent->r.maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy (tmin, trigger->r.mins);
	VectorCopy (tmax, trigger->r.maxs);

	G_Q3F_CopyMapData( ent, trigger );
	G_Q3F_AddEntityToTargetArray( ent );
	if( trigger->mapdata )
		trigger->mapdata->flags |= Q3F_FLAG_RETOUCH;	// Flag can be retouched without triggering

	trap_LinkEntity (trigger);
}


/*QUAKED func_lift (0 .5 .8) ?
Lifts are always drawn in the extended position so they will light correctly.
Identical to func_plat except that the wait key actually is useful

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_lift (gentity_t *ent) {
	float		lip, height;

	G_Q3F_SetMoverSound( ent );
	ent->timestamp = EV_LIFT;

	VectorClear (ent->s.angles);

	G_SpawnFloat( "speed", "200", &ent->speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "wait", "1", &ent->wait );
	G_SpawnFloat( "lip", "8", &lip );

	ent->wait *= 1000;

	// create second position
	trap_SetBrushModel( ent, ent->model );

	if ( !G_SpawnFloat( "height", "0", &height ) ) {
		height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;
	}

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy( ent->s.origin, ent->pos2 );
	VectorCopy( ent->pos2, ent->pos1 );
	ent->pos1[2] -= height;

	InitMover( ent );

	// touch function keeps the plat from returning while
	// a live player is standing on it
//	ent->touch = Touch_Plat;

	ent->blocked = Blocked_Door;

	ent->parent = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
// XreaL BEGIN
#ifdef _ETXREAL
	if ( !ent->targetname || ent->targetnameAutogenerated )
#else
	if ( !ent->targetname )
#endif
// XreaL END
		SpawnLiftTrigger(ent);
	ent->use = Use_Lift;
}


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace ) {
	if ( !other->client || Q3F_IsSpectator( other->client ) ) {
		return;
	}

	// Ensiform : Added so that you cannot touch door triggers during ceasefire toggle
	if( level.ceaseFire )
		return;			// Nothing is triggered during ceasefires

	ent->timestamp = EV_VISUAL_TRIGGER;
	if ( ent->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent, other, other  );
	}
}

void Use_Button( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	// Golliwog: Use a button (i.e. when shot)

	if( !activator->client || Q3F_IsSpectator( activator->client )) 
		return;

	// Ensiform : Added so that you cannot touch door triggers during ceasefire toggle
	if( level.ceaseFire )
		return;			// Nothing is triggered during ceasefires

	ent->timestamp = EV_VISUAL_TRIGGER;
	if ( ent->moverState == MOVER_POS1 ) {
		Use_BinaryMover( ent, ent, activator );
	}
}


/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_button( gentity_t *ent ) {
	vec3_t		abs_movedir;
	float		distance;
	vec3_t		size;
	float		lip;

	ent->sound1to2 = G_SoundIndex("sound/movers/switches/butn2.wav");
	ent->timestamp = EV_VISUAL_TRIGGER;	/* Ensiform - Fix the missing event spam with buttons */
	
	if ( !ent->speed ) {
		ent->speed = 40;
	}

	if ( !ent->wait ) {
		ent->wait = 1;
	}
	ent->wait *= 1000;

	// first position
	VectorCopy( ent->s.origin, ent->pos1 );

	// calculate second position
	trap_SetBrushModel( ent, ent->model );

	G_SpawnFloat( "lip", "4", &lip );

	G_SetMovedir( ent->s.angles, ent->movedir );
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
	VectorMA (ent->pos1, distance, ent->movedir, ent->pos2);

	if (ent->health) {
		// shootable button
		ent->takedamage = qtrue;

		ent->use = Use_Button; /* Ensiform - Testing if this fixes shootable buttons with mapdata & waittime breaking */
	} else {
		// touchable button
		ent->touch = Touch_Button;
	}

	InitMover( ent );
}



/*
===============================================================================

TRAIN

===============================================================================
*/
/*
===============
Reached_Train
===============
*/
#define MAX_TRAIN_TARGETS	128
static void G_Q3F_LocateNextTrainTarget( gentity_t *train )
{
	// Find the next non-disabled path_corner ent to link to.

	gentity_t *ent, *test;
	gentity_t *targets[MAX_TRAIN_TARGETS];
	int index, arrayindex;
	q3f_keypair_t *targetkp;
	q3f_array_t *targetarray;
	q3f_data_t *target;

	if( train->mapdata && (train->mapdata->state == Q3F_STATE_DISABLED || train->mapdata->state == Q3F_STATE_INVISIBLE) )
		return;					// Train is disabled, just hold position

	ent = train->nextTrain ? train->nextTrain : train;		// Get the path_corner, or itself
	if( !ent->target )
		return;					// No more targets.

	targetkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ent->target );
	if( !targetkp )
		return;					// No targets by this name.
	targetarray = targetkp->value.d.arraydata;
	for( index = -1, arrayindex = 0; ((target = G_Q3F_ArrayTraverse( targetarray, &index )) != NULL && arrayindex < MAX_TRAIN_TARGETS); )
	{
		test = target->d.entitydata;
		if( !test->mapdata || (test->mapdata->state != Q3F_STATE_DISABLED && test->mapdata->state != Q3F_STATE_INVISIBLE) )
			targets[arrayindex++] = test;
	}
	if( !arrayindex )
		return;					// No selectable targets.

	index = rand() % arrayindex;
	train->nextTrain = targets[index];
}

static void G_Q3F_TrainStart( gentity_t *train, gentity_t *curr, gentity_t *next )
{
	// set the new trajectory

	float speed, length, f;
	vec3_t move;

	train->nextTrain = next;
	VectorCopy( curr->s.origin, train->pos1 );
	VectorCopy( next->s.origin, train->pos2 );

	// if the path_corner has a speed, use that
	if ( curr->speed ) {
		speed = curr->speed;
	} else {
		// otherwise use the train's speed
		speed = train->speed;
	}
	if ( speed < 1 ) {
		speed = 1;
	}

	// calculate duration
	VectorSubtract( train->pos2, train->pos1, move );
	length = VectorLength( move );

	train->s.pos.trDuration = length * 1000 / speed;

	// Manually calculate angles (this might 'a little, but nothing major
	VectorCopy( curr->s.angles, train->s.apos.trBase );
	VectorSubtract( next->s.angles, curr->s.angles, train->s.apos.trDelta );
	train->s.apos.trDuration = length * 1000 / speed;
	f = 1000.0 / train->s.apos.trDuration;
	VectorScale( train->s.apos.trDelta, f, train->s.apos.trDelta );
	train->s.apos.trTime = level.time;
	train->s.apos.trType = TR_LINEAR_STOP;

	// looping sound
	train->s.loopSound = curr->soundLoop;

	// start it going
	SetMoverState( train, MOVER_1TO2, level.time );
}

void Reached_Train( gentity_t *ent );

void Think_BeginMoving( gentity_t *ent ) {
	// The wait time at a corner has completed, so start moving again

	gentity_t *curr, *next;

	curr = ent->nextTrain;
	G_Q3F_LocateNextTrainTarget( ent );
	next = ent->nextTrain;

	if( !next || curr == next) {
		// RR2DO2: maybe we have a valid target next frame
		ent->nextTrain = curr;
		ent->nextthink = level.time + FRAMETIME;
		return;		// just stop till then
	}
	ent->reached = Reached_Train;

	G_Q3F_TrainStart( ent, curr, next );
}

void Reached_Train( gentity_t *ent ) {
	gentity_t		*curr, *next;
	trace_t			tr;

	// copy the apropriate values
	curr = ent->nextTrain;
	if( !curr )
	{
		// First call, jump to curr
		G_Q3F_LocateNextTrainTarget( ent );
		curr = ent->nextTrain;
		if( !curr )
		{
			// RR2DO2: maybe we have a valid target next frame
			ent->nextthink = level.time + FRAMETIME;
			return;		// Just stop, there's no ent to start with (?)
		}
	}

	// fire all other targets
	memset( &tr, 0, sizeof(tr) );
	G_Q3F_TriggerEntity( curr, NULL, Q3F_STATE_ACTIVE, &tr, qfalse );

	// if there is a "wait" value on the target, don't start moving yet
	if( curr->wait ) {
		//ent->nextTrain = curr;		
		ent->reached = NULL;
		ent->think = Think_BeginMoving;
		ent->nextthink = level.time + curr->wait * 1000;
		return;
	}

	G_Q3F_LocateNextTrainTarget( ent );
	next = ent->nextTrain;
	if( !next || curr == next ) {
		// RR2DO2: maybe we have a valid target next frame
		ent->nextTrain = curr;
		ent->reached = NULL;
		ent->think = Think_BeginMoving;
		ent->nextthink = level.time + FRAMETIME;
		return;		// just stop
	}

	G_Q3F_TrainStart( ent, curr, next );
}

/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
/*void Think_SetupTrainTargets( gentity_t *ent ) {
	gentity_t		*path, *next, *start;

	ent->nextTrain = G_Find( NULL, FOFS(targetname), ent->target );
	if ( !ent->nextTrain ) {
		G_Printf( "func_train at %s with an unfound target\n",
			vtos(ent->r.absmin) );
		return;
	}

	start = NULL;
	for ( path = ent->nextTrain ; path != start ; path = next ) {
		if ( !start ) {
			start = path;
		}

		if ( !path->target ) {
			G_Printf( "Train corner at %s without a target\n",
				vtos(path->s.origin) );
			return;
		}

		// find a path_corner among the targets
		// there may also be other targets that get fired when the corner
		// is reached
		next = NULL;
		do {
			next = G_Find( next, FOFS(targetname), path->target );
			if ( !next ) {
				G_Printf( "Train corner at %s without a target path_corner\n",
					vtos(path->s.origin) );
				return;
			}
		} while ( strcmp( next->classname, "path_corner" ) );

		path->nextTrain = next;
	}

	// start the train moving from the first corner
	Reached_Train( ent );
}*/


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner( gentity_t *self ) {
// XreaL BEGIN
#ifdef _ETXREAL
	if ( (!self->targetname || self->targetnameAutogenerated) && !(self->mapdata || self->mapdata->groupname) ) {
#else
	if ( !self->targetname && !(self->mapdata || self->mapdata->groupname) ) {
#endif
// XreaL END
		G_Printf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}
	// path corners don't need to be linked in
}

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"	.md3 model to also draw
"speed"		default 100
"dmg"		default	2
"noise"		looping sound to play when the train is in motion
"target"	next path corner
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_train (gentity_t *self) {
	VectorClear (self->s.angles);

	if (!self->damage) {
		self->damage = 2;
	}

	if ( !self->speed ) {
		self->speed = 100;
	}

	if ( !self->target ) {
		G_Printf ("func_train without a target at %s\n", vtos(self->r.absmin));
		G_FreeEntity( self );
		return;
	}

	G_SpawnInt( "dmg", "0", &self->damage );

	trap_SetBrushModel( self, self->model );
	InitMover( self );

	self->reached = Reached_Train;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	self->nextthink = level.time + FRAMETIME;
	self->think = Reached_Train;
//	self->think = G_Q3F_LocateNextTrainTarget; //Think_SetupTrainTargets;

	self->blocked = Blocked_Door;
	self->spawnflags |= DOOR_CRUSHER;		//Always CRUSH!!!!
}

/*
===============================================================================

STATIC

===============================================================================
*/


/*QUAKED func_static (0 .5 .8) ?
A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_static( gentity_t *ent ) {
	trap_SetBrushModel( ent, ent->model );
	InitMover( ent );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
}


/*
===============================================================================

ROTATING

===============================================================================
*/

static int G_Q3F_FuncRotatingStateThink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace ) {
	if( targetstate == Q3F_STATE_DISABLED || targetstate == Q3F_STATE_INVISIBLE ) {
		if ( level.time == 0 )
			ent->s.apos.trTime = 1; // prevent div by 0 in traj calculations
		else
			ent->s.apos.trTime = level.time + ent->s.apos.trTime;
		trap_UnlinkEntity( ent );
	} else {
		trap_LinkEntity( ent );
		ent->s.apos.trTime = ent->s.apos.trTime - level.time;
	}

	return targetstate;
}


/*QUAKED func_rotating (0 .5 .8) ? - - X_AXIS Y_AXIS Z_AXIS
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"model2"	.md3 model to also draw
"speed"		determines how fast it moves; default value is 100.
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_rotating (gentity_t *ent) {
	qboolean	anyaxis = qfalse;

	G_SpawnFloat( "speed", "100", &ent->speed );
	G_SpawnInt( "dmg", "2", &ent->damage );

	// set the axis of rotation
	ent->s.apos.trType = TR_ROTATING;
	if ( ent->spawnflags & 4 ) {
		ent->s.apos.trDelta[2] = ent->speed;
		anyaxis = qtrue;
	}
	if ( ent->spawnflags & 8 ) {
		ent->s.apos.trDelta[0] = ent->speed;
		anyaxis = qtrue;
	}
	if ( ( ent->spawnflags & 16 ) || !anyaxis ) {
		ent->s.apos.trDelta[1] = ent->speed;
	}

	trap_SetBrushModel( ent, ent->model );

	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

	if ( ent->mapdata && ( ent->mapdata->state == Q3F_STATE_DISABLED || ent->mapdata->state == Q3F_STATE_INVISIBLE) ) {
		if ( level.time == 0 )
			ent->s.apos.trTime = 1; // prevent div by 0 in traj calculations
		else
			ent->s.apos.trTime = level.time + ent->s.apos.trTime;
		trap_UnlinkEntity( ent );
	}
	else {
		trap_LinkEntity( ent );
	}

	if( ent->mapdata )
		ent->mapdata->statethink = G_Q3F_FuncRotatingStateThink;
}


/*
===============================================================================

BOBBING

===============================================================================
*/


/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS Z_AXIS
Normally bobs on the Z axis
"model2"	.md3 model to also draw
"height"	amplitude of bob (32 default)
"speed"		seconds to complete a bob cycle (4 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_bobbing (gentity_t *ent) {
	qboolean	anyaxis = qfalse;
	float		height;
	float		phase;

	G_SpawnFloat( "speed", "4", &ent->speed );
	G_SpawnFloat( "height", "32", &height );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent, ent->model );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.pos.trDuration = ent->speed * 1000;
	ent->s.pos.trTime = ent->s.pos.trDuration * phase;
	ent->s.pos.trType = TR_SINE;

	// set the axis of bobbing
	if ( ent->spawnflags & 1 ) {
		ent->s.pos.trDelta[0] = height;
		anyaxis = qtrue;
	}
	if ( ent->spawnflags & 2 ) {
		ent->s.pos.trDelta[1] = height;
		anyaxis = qtrue;
	}
	if ( ( ent->spawnflags & 4 ) || !anyaxis ) {
		ent->s.pos.trDelta[2] = height;
	}
}

/*
===============================================================================

PENDULUM

===============================================================================
*/


/*QUAKED func_pendulum (0 .5 .8) ?
You need to have an origin brush as part of this entity.
Pendulums always swing north / south on unrotated models.  Add an angles field to the model to allow rotation in other directions.
Pendulum frequency is a physical constant based on the length of the beam and gravity.
"model2"	.md3 model to also draw
"speed"		the number of degrees each way the pendulum swings, (30 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_pendulum(gentity_t *ent) {
	float		freq;
	float		length;
	float		phase;
	float		speed;

	G_SpawnFloat( "speed", "30", &speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent, ent->model );

	// find pendulum length
	length = fabs( ent->r.mins[2] );
	if ( length < 8 ) {
		length = 8;
	}

	freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity.value / ( 3 * length ) );

	ent->s.pos.trDuration = ( 1000 / freq );

	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->s.apos.trDuration = 1000 / freq;
	ent->s.apos.trTime = ent->s.apos.trDuration * phase;
	ent->s.apos.trType = TR_SINE;
	ent->s.apos.trDelta[2] = speed;
}
