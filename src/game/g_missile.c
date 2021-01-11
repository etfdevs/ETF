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
#include "g_q3f_weapon.h"
#include "bg_q3f_playerclass.h"
#include "g_q3f_grenades.h"
#include "g_q3f_mapents.h"

/*
================
G_BounceMissile

================
*/

void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// RR2DO2: Used for final rotation value
	VectorCopy( ent->s.pos.trDelta, ent->s.angles );

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		VectorScale( ent->s.pos.trDelta, 0.50, ent->s.pos.trDelta );
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			ent->s.time = level.time;	// Golliwog: Used for final rotation value

			// Golliwog: Standing pipes do more damage than mid-air pipes
			// Of course, there's no means to reset the damage if the support vanishes
			//if( ent->s.weapon == WP_PIPELAUNCHER )
			//	ent->damage = ent->splashDamage = 130;
			// Golliwog.

			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}

/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );
	ent->s.otherEntityNum = ENTITYNUM_WORLD;
	ent->s.otherEntityNum2 = ent->r.ownerNum;	//ent->s.number;

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		G_RadiusDamage( ent->r.currentOrigin, ent, ent->parent,
			ent->splashDamage, ent, ent->splashMethodOfDeath, 0);
	}

	ent->classname = "explosion";			// JT - Stop detpiping from exploding explosions.
	trap_LinkEntity( ent );
}

/*
================
G_MissileImpact

================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t	*other, *owner, *tent;
	int			damage_type, given;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( (!other->takedamage || !strcmp("pipe",ent->classname)) &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) 
	{
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	if (ent->s.weapon > WP_NONE && ent->s.weapon < WP_NUM_WEAPONS
		&& ent->parent && ent->parent->client ) 
		given = ent->parent->client->pers.stats.data[STATS_WP + ent->s.weapon].given;
	 else given = -1;

	// Gas gren explosion
	if(!strcmp(ent->classname,"napalm"))
	{			
		//Canabis simplified checking if we're burning gas
		//Slightly lower the splashradius so you actually have to aim a bit..
		HallucinoGenicCheckFire(ent->r.currentOrigin, 100, &g_entities[ent->r.ownerNum]);
	}

	// impact damage
	if (other->takedamage && ent->damage ) {
		// FIXME: wrong damage direction?
		vec3_t	velocity;
		owner = &g_entities[ent->r.ownerNum];
		if( !owner->client && ent->parent )
			owner = ent->parent;

		BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
		if ( VectorLength( velocity ) == 0 ) {
			velocity[2] = 1;	// stepped on a grenade
		}
		if(!strcmp(ent->classname, "flame"))
		{
			G_Q3F_Burn_Person(other, owner);
			damage_type = DAMAGE_Q3F_FIRE;
		}
		else if(!strcmp(ent->classname, "mapflame"))
		{
			//G_Q3F_Burn_Person_FromMap(other, owner);
			damage_type = DAMAGE_Q3F_FIRE;
		}
		else if(!strcmp(ent->classname,"napalm"))
		{				
			G_Q3F_Burn_Person(other, owner);
			G_Q3F_Burn_Person(other, owner);
			G_Q3F_Burn_Person(other, owner);
			damage_type = DAMAGE_Q3F_FIRE;
		}
		else if(!strcmp(ent->classname,"tranq_dart"))
		{
			G_Q3F_Tranq_Person(other, owner );
			damage_type = 0;
		}
		else if(!strcmp(ent->classname,"nail"))
		{
			damage_type = DAMAGE_Q3F_SHELL;
		}
		else if(!strcmp(ent->classname,"rail"))
		{
			damage_type = DAMAGE_Q3F_SHOCK | DAMAGE_NO_ARMOR;
		} 
		else
		{
			damage_type = DAMAGE_Q3F_EXPLOSION;
			if (other->client)  {
//				trap_SendServerCommand( -1,"print \"direct\n\"");
				VectorSubtract( other->client->ps.origin, trace->endpos, velocity );
				velocity[2] += 5;
#if 0
				trap_SendServerCommand( -1, va
					("print \"ent %.0f %.0f %.0f impact %.0f %.0f %.0f dir %.2f %.2f %.2f\n\"",
					other->client->ps.origin[0],
					other->client->ps.origin[1],
					other->client->ps.origin[2],
					trace->endpos[0],
					trace->endpos[1],
					trace->endpos[2],
					velocity[0],
					velocity[1],
					velocity[2]
				));
#endif		
			}
		}
		/* Do the direct damage to the impacted client */
		G_Damage ( other, ent, owner, velocity,
			trace->endpos, ent->damage, 
			damage_type, ent->methodOfDeath );
	}

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage )
	{
		if (!strcmp(ent->classname,"napalm"))
			G_NapalmRadiusDamage( trace->endpos, ent->parent, ent->splashDamage, 
				other, ent->splashMethodOfDeath );
		else
			G_RadiusDamage( trace->endpos, ent, ent->parent, ent->splashDamage, 
				other, ent->splashMethodOfDeath, 0);
	}

	//Record a hit
	if (given >= 0 && given < ent->parent->client->pers.stats.data[STATS_WP + ent->s.weapon].given ) 
		ent->parent->client->pers.stats.data[STATS_WP + ent->s.weapon].hits++;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	if ( other->takedamage && other->client ) {
		tent = G_TempEntity( trace->endpos, EV_MISSILE_HIT );
		tent->s.eventParm = DirToByte( trace->plane.normal );
		tent->s.otherEntityNum = other->s.number;
		tent->s.otherEntityNum2 = ent->r.ownerNum;	//ent->s.number;
	} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
		tent = G_TempEntity( trace->endpos, EV_MISSILE_MISS_METAL );
		tent->s.eventParm = DirToByte( trace->plane.normal );
		tent->s.otherEntityNum2 = ent->r.ownerNum;	//ent->s.number;
	} else {
		tent = G_TempEntity( trace->endpos, EV_MISSILE_MISS );
		tent->s.eventParm = DirToByte( trace->plane.normal );
		tent->s.otherEntityNum2 = ent->r.ownerNum;	//ent->s.number;
	}
	tent->s.weapon = ent->s.weapon;
	if (ent->s.powerups & (1 << PW_QUAD) ) 
		tent->s.weapon |= 16;

	G_FreeEntity( ent );
}


/*
================
G_RunMissile

================
*/
void G_RunMissile( gentity_t *ent ) {
//	vec3_t		origin;
//	trace_t		tr;
//	trace_t		tr2;

	// Golliwog: Heavily altered to 'sit on' other objects.

	vec3_t origin, oldorigin;//, dir, angles;
	trace_t tr, tr2;
	//gentity_t *other;
//	vec3_t	velocity;
//	float	dot;
//	int		hitTime;
	int			passent;

	if( ent->s.pos.trType == TR_LINEAR )
	{
		VectorCopy(ent->r.currentOrigin, oldorigin);
			// get current position
		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

		passent = ent->r.ownerNum;

		// trace a line from the previous position to the current position,
		// ignoring interactions with the missile owner
		G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, ent->clipmask);

		VectorCopy( tr.endpos, ent->r.currentOrigin );

		if ( tr.startsolid || tr.allsolid ) {
			// make sure the tr.entityNum is set to the entity we're stuck in
			G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
			tr.fraction = 0;
		}
		else {
			VectorCopy( tr.endpos, ent->r.currentOrigin );
		}

		/* This is going to be expensive */
		/* Removal of string checks makes it slightly less expensive but still bleh */
		if(levelhasnoannoys)
		{
			if(ent->parent && ent->parent->client && ent->s.weapon != WP_FLAMETHROWER)
			//if(ent->parent && ent->parent->client && strcmp(ent->classname, "flame") && strcmp(ent->classname, "mapflame"))
			{
				vec3_t tracestart, traceend;

				VectorAdd( tr.endpos, ent->r.mins, tracestart );
				VectorAdd( tr.endpos, ent->r.maxs, traceend );

				if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->parent->client->sess.sessionTeam, Q3F_NOANNOY_PROJECTILES ) )
				{
					if(ent->s.weapon == WP_PIPELAUNCHER)
					//if(!strcmp(ent->classname, "pipe"))
					{
						ent->parent->client->pipecount --;

						// player pipe count
						ent->parent->client->ps.ammoclip[0] = ent->parent->client->pipecount;
					}
					G_FreeEntity( ent );
					return;
				}
			}
		}

		trap_LinkEntity( ent );

		if(strcmp(ent->classname,"flame") == 0)
		{
			G_Q3F_ForceFieldTrace( &tr2, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, ent->clipmask | CONTENTS_WATER);

			if(tr2.contents & CONTENTS_WATER)
			{
				G_FreeEntity(ent);
				return;
			}
			//Canabis simplified checking if we're burning gas
			HallucinoGenicCheckFire(ent->r.currentOrigin,16,&g_entities[ent->r.ownerNum]);
		}

		if ( tr.fraction != 1 ) {
			// never explode or bounce on sky
			if ( tr.surfaceFlags & SURF_NOIMPACT && strcmp(ent->classname,"grenade") && strcmp(ent->classname, "pipe")) {
				G_FreeEntity( ent );
				return;
			}

			G_MissileImpact( ent, &tr );
			if ( ent->s.eType != ET_MISSILE ) {
				return;		// exploded
			}
		}
	}
	else {
		// get current position
		if( ent->s.groundEntityNum == ENTITYNUM_NONE )
			BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		else {
			if(	(g_entities[ent->s.groundEntityNum].inuse || ent->s.groundEntityNum == ENTITYNUM_WORLD) &&
				!(g_entities[ent->s.groundEntityNum].s.eType == ET_PLAYER && g_entities[ent->s.groundEntityNum].health <= 0) )
			{
				BG_EvaluateTrajectory( &g_entities[ent->s.groundEntityNum].s.pos, level.time, origin );
				VectorAdd( origin, ent->pos1, origin );
				G_SetOrigin( ent, origin );
				origin[2]--;
			}
			else {
				// We've lost whatever we were sitting on

				VectorCopy( ent->s.pos.trBase, origin );
				origin[2]--;
				ent->s.groundEntityNum = ENTITYNUM_NONE;
				ent->s.pos.trType = TR_GRAVITY;
				ent->s.pos.trTime = level.time;
				VectorSet( ent->s.pos.trDelta, 0, 0, 0 );
			}
		}

		// trace a line from the previous position to the current position,
		// ignoring interactions with the missile owner
		if( ent->s.pos.trType == TR_STATIONARY || ent->s.pos.trType == TR_GRAVITY )
		{
			G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, ent->clipmask );

			VectorCopy( tr.endpos, ent->r.currentOrigin );

			/* This is going to be expensive */
			/* Removal of string checks makes it slightly less expensive but still bleh */
			if(levelhasnoannoys)
			{
				if(ent->parent && ent->parent->client && strcmp(ent->classname, "flame") && strcmp(ent->classname, "mapflame"))
				{
					vec3_t tracestart, traceend;

					VectorAdd( tr.endpos, ent->r.mins, tracestart );
					VectorAdd( tr.endpos, ent->r.maxs, traceend );

					if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->parent->client->sess.sessionTeam, Q3F_NOANNOY_PROJECTILES ) )
					{
						if(ent->s.weapon == WP_PIPELAUNCHER)
						//if(!strcmp(ent->classname, "pipe"))
						{
							ent->parent->client->pipecount --;

							// player pipe count
							ent->parent->client->ps.ammoclip[0] = ent->parent->client->pipecount;
						}
						G_FreeEntity( ent );
						return;
					}
				}
			}

			trap_LinkEntity( ent );

			if ( tr.startsolid ) {
				tr.fraction = 0;
			}
			if ( tr.fraction != 1 )
			{
				// never explode or bounce on sky
				if ( tr.surfaceFlags & SURF_NOIMPACT && strcmp(ent->classname,"grenade") && strcmp(ent->classname, "pipe")) {
					G_FreeEntity( ent );
					return;
				}

				if( ent->s.pos.trType != TR_STATIONARY )
				{
					G_MissileImpact( ent, &tr );
					if ( ent->s.eType != ET_MISSILE ) {
						return;		// exploded
					}
				}

				//other = &g_entities[tr.entityNum];
					// reflect the velocity on the trace plane
				if( ent->s.time == level.time &&
					ent->s.pos.trType == TR_STATIONARY &&
					!(g_entities[tr.entityNum].s.eType == ET_PLAYER &&
					g_entities[tr.entityNum].health <= 0) )
				{
					ent->s.groundEntityNum = tr.entityNum;
					BG_EvaluateTrajectory( &g_entities[tr.entityNum].s.pos, level.time, ent->pos1 );
					VectorSubtract( ent->r.currentOrigin, ent->pos1, ent->pos1 );
					ent->s.groundEntityNum = tr.entityNum;
				}
			}
			else if( ent->s.pos.trType == TR_STATIONARY )
			{
				// We're in free-fall now.

				ent->s.pos.trType = TR_GRAVITY;
				VectorCopy( origin, ent->s.pos.trBase );
				ent->s.pos.trTime = level.time;
				ent->s.groundEntityNum = ENTITYNUM_NONE;
			}
		}
		else {
			VectorCopy( origin, ent->r.currentOrigin );
		}

		trap_LinkEntity( ent );
	}

	// check think function after bouncing
	G_RunThink( ent );
}

//=============================================================================

/*
=================
fire_flame

=================
*/

gentity_t *fire_flame (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt, *lastflame;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "flame";
	bolt->nextthink = level.time + BG_Q3F_FLAME_LIFETIME;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	//bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;//| SVF_NOCLIENT;
	bolt->r.svFlags = SVF_NOCLIENT; // RR2DO2 - flamethrower rendering is independant of actual flames. Don't send em to the client.
	//Canabis, change this so when flame are visible the parent also is visible
	bolt->r.svFlags |= SVF_VISDUMMY; 
	bolt->s.otherEntityNum = self->s.number;
	bolt->s.weapon = WP_FLAMETHROWER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	//bolt->damage = 15; // RR2DO2: 1e
	//bolt->damage = 16; // RR2DO2: 1f
	bolt->damage = 10; // RR2DO2: b2
	bolt->splashDamage = 10;
	bolt->methodOfDeath = MOD_FLAME;
	bolt->splashMethodOfDeath = MOD_FLAME_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	if( self->client ) {
		if( self->client->lastflame && (lastflame = &g_entities[self->client->lastflame])->inuse )
			lastflame->s.otherEntityNum2 = bolt->s.number;
		self->client->lastflame = bolt->s.number;
	} else {
		if( self->soundLoop && (lastflame = &g_entities[self->soundLoop])->inuse )
			lastflame->s.otherEntityNum2 = bolt->s.number;
		self->soundLoop = bolt->s.number;
	}

	bolt->r.mins[0] = -1;
	bolt->r.mins[1] = -1;
	bolt->r.mins[2] = -1;

	bolt->r.maxs[0] = 1;
	bolt->r.maxs[1] = 1;
	bolt->r.maxs[2] = 1;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, BG_Q3F_FLAME_VELOCITY, bolt->s.pos.trDelta );		// Slow it down.

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

#if 0
// Assumes self is a misc_flamethrower, eventually can do away with start and dir parms once that ent is finished
gentity_t *fire_mapflame (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt, *lastflame;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "mapflame";
	bolt->nextthink = level.time + BG_Q3F_FLAME_LIFETIME;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	//bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;//| SVF_NOCLIENT;
	bolt->r.svFlags = SVF_NOCLIENT; // RR2DO2 - flamethrower rendering is independant of actual flames. Don't send em to the client.
	//Canabis, change this so when flame are visible the parent also is visible
	bolt->r.svFlags |= SVF_VISDUMMY; 
	bolt->s.otherEntityNum = self->s.number;
	bolt->s.weapon = WP_FLAMETHROWER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	//bolt->damage = 15; // RR2DO2: 1e
	//bolt->damage = 16; // RR2DO2: 1f
	bolt->damage = 10; // RR2DO2: b2
	bolt->splashDamage = 10;
	bolt->methodOfDeath = MOD_FLAME;
	bolt->splashMethodOfDeath = MOD_FLAME_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	if( self->soundLoop && (lastflame = &g_entities[self->soundLoop])->inuse )
		lastflame->s.otherEntityNum2 = bolt->s.number;
	self->soundLoop = bolt->s.number;

	bolt->r.mins[0] = -1;
	bolt->r.mins[1] = -1;
	bolt->r.mins[2] = -1;

	bolt->r.maxs[0] = 1;
	bolt->r.maxs[1] = 1;
	bolt->r.maxs[2] = 1;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, BG_Q3F_FLAME_VELOCITY, bolt->s.pos.trDelta );		// Slow it down.

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
#endif

//=============================================================================
 

/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	//Unlagged client check
	bolt->s.otherEntityNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 120;
	bolt->splashDamage = 120;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->s.groundEntityNum = ENTITYNUM_NONE;	// Golliwog: It's not touching anything at start

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

	VectorSet( bolt->r.mins, -2.f, -2.f, -2.f );
	VectorSet( bolt->r.maxs, 2.f, 2.f, 2.f );

	VectorCopy( start, bolt->s.pos.trBase );
//	VectorScale( dir, 700, bolt->s.pos.trDelta );				//ETF 0.0
	VectorScale( dir, 650, bolt->s.pos.trDelta );
	//VectorAdd(self->client->ps.velocity, bolt->s.pos.trDelta, bolt->s.pos.trDelta);		// Nobody likes that ;)

	//bolt->s.pos.trDelta[2] = 200;				// Erk. But that's what it says in the source!

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_pipe
=================
*/
gentity_t *fire_pipe (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;
	trace_t		tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "pipe";
	bolt->nextthink = level.time + 120000;		// 120 seconds.
	bolt->think = G_ExplodePipe;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PIPELAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;

	//Unlagged client check
	bolt->s.otherEntityNum = self->s.number;

	bolt->parent = self;
	bolt->damage = 120;
	bolt->splashDamage = 120;
	bolt->methodOfDeath = MOD_PIPE;
	bolt->splashMethodOfDeath = MOD_PIPE;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->timestamp = level.time;				// Save my creation time for later use
	bolt->s.groundEntityNum = ENTITYNUM_NONE;	// Golliwog: It's not touching anything at start

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

	VectorSet( bolt->r.mins, -2.f, -2.f, -2.f );
	VectorSet( bolt->r.maxs, 2.f, 2.f, 2.f );

	VectorCopy( start, bolt->s.pos.trBase );
//	VectorScale( dir, 700, bolt->s.pos.trDelta );				//ETF 0.0
	VectorScale( dir, 650, bolt->s.pos.trDelta );
//	VectorScale( dir, g_pipeLauncherVel.value, bolt->s.pos.trDelta );		

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	
	G_Q3F_ForceFieldTrace( &tr, start, bolt->r.mins, bolt->r.maxs, start, self->s.number, bolt->clipmask );
	if (tr.startsolid) {
		//Com_Printf("WTF SOLID NOOOO\n");
	}

	VectorCopy (start, bolt->r.currentOrigin);
	self->client->pipecount++;

	// player pipe count
	self->client->ps.ammoclip[0] = self->client->pipecount;

	return bolt;
}

/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	//Unlagged client check
	bolt->s.otherEntityNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 92+Q_flrand(0.0f, 1.0f)*20;
	bolt->splashDamage = 92;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 900, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

// RR2DO2: nailgun fire init function
/*
=================
fire_nail
=================
*/
gentity_t *fire_nail (gentity_t *self, vec3_t start, vec3_t dir, int damage, int mod) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "nail";
	bolt->nextthink = level.time + 60000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAILGUN;
	bolt->r.ownerNum = self->s.number;
	//Unlagged client check
	bolt->s.otherEntityNum = self->s.number;

	bolt->parent = self;
	bolt->damage = damage;
	bolt->splashDamage = 0;
	bolt->methodOfDeath = mod;	//MOD_NAILGUN;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;

	bolt->r.mins[0] = -1;
	bolt->r.mins[1] = -1;
	bolt->r.mins[2] = -1;

	bolt->r.maxs[0] = 1;
	bolt->r.maxs[1] = 1;
	bolt->r.maxs[2] = 1;

	bolt->s.pos.trTime = level.time;
	//VectorMA( muzzle, -50, dir, bolt->s.pos.trBase );		// Draw a frame previous.
	VectorMA( start, -15, dir, bolt->s.pos.trBase );		// Draw a frame previous.
	bolt->s.pos.trBase[2] -= 6;
//	VectorScale( dir, 1100, bolt->s.pos.trDelta );
	VectorScale( dir, 1250, bolt->s.pos.trDelta ); // djbob: 2.2 value
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}
// RR2DO2

