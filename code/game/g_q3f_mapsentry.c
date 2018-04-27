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
**	g_q3f_mapsentry.c
**
**	Server-side code for map sentries
*/

/*
	Range - obvious one this. Value in world units.
	Type - choice of minigun, rocketlauncher. determines
		what model is used and what ammo is fired.
		machinegun and minigun would use same ammo, with the mapper
		determining what damage it did using the Damage flag (see below).
		rocketlauncher would fire rockets with some sort of prediction
		thing (think Tribes sentrys)
	Damage - damage each projectile does.
	RateofFire? - How many times it fires per second. Not sure how you'd work
		out a default value that worked with the various Types; we
		wouldn't want a rocketlauncher firing every 100ms :)
	Ammo - Amount of ammo the gun starts with
	Ammoregen - Speed in ms? that it regenerates ammo while inactive (stateinactive)
	Maxammo - Max amount of ammo the gun could hold
	Accuracy - not sure if we need this or how you'd regulate it. I'll leave that
		to you to have a think about :)
*/

/*
	angle			- Range * Range;
	damage			- Damage per shot.
	splashDamage	- Rate of fire.
	count			- Maximum ammo.
	health			- Current ammo.
	noise_index		- Ammo regen rate.
	soundLoop		- Projectile Speed if it shoots projectiles
	pos1			- Origin relative to parent.
	pos2			- Angle relative to parent.
	movedir			- Direction of aim, including base angles (set during target calculation).
	s.angles		- The angles of the sentry how it's positioned
	s.pos			- Trajectory/position.
	s.apos			- Trajectory/angle of barrel, before base angles are taken into account.
	s.legsAnim		- type 0 mini 1 rocket
	s.frame			- Fired
	timestamp		- Time to next poll.
	soundPos1		- Time to next shot.
	soundPos2		- Time to next ammo regen.
*/

#include "g_local.h"
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"
#include "bg_q3f_mapsentry.h"

#define	Q3F_MAPSENTRY_ROT_IDLE_SPEED	70
#define	Q3F_MAPSENTRY_ROT_SPEED			720
#define	Q3F_MAPSENTRY_ACCURACY			60
#define	Q3F_MAPSENTRY_AIM_HEIGHT		10


typedef struct g_q3f_mapsentry_s {
	float projectileSpeed;
	int defaultRange, defaultDamage, defaultRateOfFire;
	int defaultInitAmmo, defaultMaxAmmo, defaultAmmoRegen;
} g_q3f_mapsentry_t;

static g_q3f_mapsentry_t bg_q3f_mapSentryMini = {
	0,
	1000, 10, 100,
	100, 100, 300,
};

static g_q3f_mapsentry_t bg_q3f_mapSentryRocket = {
	900,
	1000, 100, 2000,
	10, 10, 1000,
};




/******************************************************************************
***** Sentry setup
****/

static void G_Q3F_Misc_MapSentryPosition( gentity_t *ent )
{
	// Attach to nearest surface, prepare nextthink.

	vec3_t surface, angles;
	gentity_t *surfaceEnt;

	if( !G_Q3F_RadiateToSurface( ent->s.origin, 64, surface, angles, &surfaceEnt, MASK_SOLID, ent->s.number ) )
	{
		G_Printf( "Failed to attach map sentry to surface.\n" );
		G_FreeEntity( ent );
		return;
	}

	if( !surfaceEnt || !surfaceEnt->inuse )
		surfaceEnt = &g_entities[ENTITYNUM_WORLD];

	ent->parent = surfaceEnt;
	VectorSubtract( surface, surfaceEnt->s.origin, ent->pos1 );
	VectorSubtract( angles, surfaceEnt->s.angles, ent->pos2 );
	VectorCopy( angles, ent->s.angles );
	VectorSet( ent->s.origin, 0, 0, 0 );

	if ( ent->target ) {
		ent->target_ent = G_Find (NULL, FOFS(targetname), ent->target);
	}
	ent->think = 0;
}

static char *G_Q3F_itoa( int num )
	{ return( va( "%i", num ) ); }


void SP_Q3F_misc_mapsentry( gentity_t *ent )
{
	// Create a map sentry of the appropriate type.

	char *str;
	g_q3f_mapsentry_t *sentry;

		// Find the correct type to use.
	G_SpawnString( "type", "minigun", &str );
	if (!Q_stricmp( str, "minigun")) {
		sentry = &bg_q3f_mapSentryMini;
		ent->s.legsAnim = 0;
	} else if (!Q_stricmp( str, "rocketlauncher")) {
		sentry = &bg_q3f_mapSentryRocket;
		ent->s.legsAnim = 1;
	} else {
		G_Printf( "Unknown misc_mapsentry type '%s'.\n", str );
		G_FreeEntity( ent );
		return;
	}
	// Load the assorted parameters

	ent->soundLoop = sentry->projectileSpeed;
	G_SpawnFloat( "range",		G_Q3F_itoa( sentry->defaultRange		),	&ent->angle	);
	G_SpawnInt( "damage",		G_Q3F_itoa( sentry->defaultDamage		),	&ent->damage		);
	G_SpawnInt( "rateoffire",	G_Q3F_itoa( sentry->defaultRateOfFire	),	&ent->splashDamage	);
	G_SpawnInt( "initammo",		G_Q3F_itoa( sentry->defaultInitAmmo		),	&ent->health		);
	G_SpawnInt( "maxammo",		G_Q3F_itoa( sentry->defaultMaxAmmo		),	&ent->count			);
	G_SpawnInt( "ammoregen",	G_Q3F_itoa( sentry->defaultAmmoRegen	),	&ent->noise_index	);

	// Set up the ent itself.
	ent->angle*=ent->angle;
	ent->think		= G_Q3F_Misc_MapSentryPosition;
	ent->nextthink	= level.time + FRAMETIME;
	ent->s.eType	= ET_Q3F_MAPSENTRY;
	ent->soundPos2	= level.time;
}


/******************************************************************************
***** Map operation
****/

qboolean G_Q3F_MapSentrySetTargetAngle( gentity_t *ent, const vec3_t target, float speed )
{
	// Aim the sentry at an enemy.

	vec3_t angles, diff, origin;
	vec3_t cross, cross2; 
	vec3_t forward, right, up;
	vec_t canreach;
	vec_t distance;
	int i;

	// Work out the vector between us and convert to an angle
	ent->waterlevel = 0;
	AngleVectors( ent->s.angles, forward, right, up);

	VectorMA( ent->r.currentOrigin, Q3F_MAPSENTRY_AIM_HEIGHT, forward, origin );
	VectorSubtract( target, origin, diff );
	VectorNormalize( diff );
	VectorCopy( diff, ent->movedir );

	angles[PITCH] = DotProduct( diff, forward );
	if( angles[PITCH] < 0)		// Give up now if we have to go below our 'horizon'.
		return( qfalse );
	else if (angles[PITCH] > 1)
		angles[PITCH] = 1;
	angles[PITCH] = (180.0/M_PI) * acos(angles[PITCH]);
	/* Determine yaw by angle between the planes */
	CrossProduct( diff, forward, cross );
	VectorNormalize( cross );
	angles[YAW] = (180.0/M_PI) * acos(DotProduct( right, cross ));
	CrossProduct( right, cross, cross2 );
	VectorNormalize( cross2 );
	if (DotProduct(cross2, forward) < 0) 
		angles[YAW] = 360 - angles[YAW];
	angles[ROLL] = 0;

	for( i = 0; i < 3; i++ )
		diff[i] = AngleNormalize180( angles[i] - ent->s.apos.trBase[i] );

	distance = VectorLength( diff );
	if (distance < Q3F_MAPSENTRY_ACCURACY)			// Close enough to shoot?
		ent->waterlevel = 1;	
	ent->s.apos.trType = TR_INTERPOLATE;
	VectorClear( ent->s.apos.trDelta );
	ent->s.apos.trDuration = 0;
	ent->s.apos.trTime = 0;
	canreach = 0.001 * speed * (level.time - level.previousTime);
	if( distance <= canreach ) {
		// Just snap the final distance to correct rounding errors (and 'twitching')
		VectorCopy( angles, ent->s.apos.trBase );
	} else {
		canreach /= distance;
		ent->s.apos.trBase[YAW] = LerpAngle( ent->s.apos.trBase[YAW], angles[YAW], canreach );
		ent->s.apos.trBase[PITCH] = LerpAngle( ent->s.apos.trBase[PITCH], angles[PITCH], canreach );
		ent->s.apos.trBase[ROLL] = 0;
	}
	SnapVector( ent->s.apos.trBase );
	return( qtrue );
}

static qboolean G_Q3F_MapSentryLock( gentity_t *sentry, gentity_t *target, vec3_t origin, vec3_t lockpoint, qboolean ignoreenvironment )
{
	// Attempt to lock onto the specified entity, returning the point there was
	// a successful trace. Aims for feet before head, just to be nasty and 
	// increase the odds of splash damage :)
	// It can't see into/out of water, so it works out the 'type' the player's 
	// feet are in, then uses the 'waterlevel' variable to work out.

	trace_t tr;
//	trajectory_t pos;
	vec3_t intersectPos, playerDir, playerOrigin, velocity;//, rocketDir;
	float lockz, invTargetSpeed, intersectAngle, intersectLen, intersectMinTime, intersectMaxTime;// intersectTime, ;
	int mask, attno, targno, index;

//	G_Printf("Locking from %f %f %f\n",origin[0],origin[1],origin[2]);

	if( !target->client )
		return( qfalse );

	if( sentry->soundLoop )
	{
		// Try to predict the client position on intersection with the projectile.
		// Attempt one failed miserably due to lack of sleep... 
		// Attempt two creates a 'time triangle' based on _times_ rather than distances or
		// actual physical vectors (since players move at different speeds from the projectile).
		// The intersection point is the point along the player's movement where |SI| == |PI|.
		// We first calculate the length of each side based on time, then work out the angle SPI
		// using the Law of Cosines. Since the intersection point forms an isosceles triangle
		// (by virtue of two lines being identical) with the line SP being the third, we then
		// take the old cos(x) = adj/hyp, and work out the hyp line, which will be the time to
		// intersection. The adj line is half of the line SP (since that's where the
		// perpendicular line is formed on an isosceles triangle).
		// And in case anyone has doubts, it appears to work... I don't even want to think about
		// trying to work out the math for a player influenced by gravity. :)

		VectorCopy( target->client->ps.velocity, velocity );
		VectorCopy( target->client->ps.origin, playerOrigin );
		if( target->client->ps.groundEntityNum == ENTITYNUM_NONE )
		{
			if( trap_PointContents( playerOrigin, target->s.number ) & MASK_WATER )
			{
				// They're in the water, calculate their trajectory until they hit a wall or the edge of the water.
				// Need to test that a trace out of water hits the interface.

				VectorMA( playerOrigin, 2 * sentry->angle, velocity, intersectPos );
				G_Q3F_ForceFieldTrace( &tr, playerOrigin, target->r.mins, target->r.maxs, intersectPos, target->s.number, (MASK_SOLID|MASK_WATER) );
				VectorCopy( tr.endpos, intersectPos );
			}
			else {
				// They're in mid-air, treat it as a ground-trace if they're within 64 units.

				VectorCopy( playerOrigin, intersectPos );
				intersectPos[2] -= 64;
				G_Q3F_ForceFieldTrace( &tr, playerOrigin, target->r.mins, target->r.maxs, intersectPos, target->s.number, (MASK_SOLID|MASK_WATER) );
				if( tr.fraction < 1 )
				{
					float backoff;
					int i;

					VectorCopy( tr.endpos, playerOrigin );

					// Adjust to follow the line of the surface
					backoff = DotProduct ( velocity, tr.plane.normal );

					// Default overbounce ( OVERCLIP )
					if ( backoff < 0 ) {
						backoff *= 1.001f;
					} else {
						backoff /= 1.001f;
					}

					for ( i=0 ; i<3 ; i++ ) {
						velocity[i] -= tr.plane.normal[i] * backoff;
					}

					//velocity[2] = 0;	// Should be adjusted to follow the line of the surface.
				}
				VectorMA( playerOrigin, 2 * sentry->angle, velocity, intersectPos );
				G_Q3F_ForceFieldTrace( &tr, playerOrigin, target->r.mins, target->r.maxs, intersectPos, target->s.number, (MASK_SOLID|MASK_WATER) );
				VectorCopy( tr.endpos, intersectPos );
			}
		}
		else {
				// Ground trace
			VectorMA( playerOrigin, 2 * sentry->angle, velocity, intersectPos );
			G_Q3F_ForceFieldTrace( &tr, playerOrigin, target->r.mins, target->r.maxs, intersectPos, target->s.number, (MASK_SOLID|MASK_WATER) );
			VectorCopy( tr.endpos, intersectPos );
		}

				// c^2 = a^2 + b^2 - 2*a*b*cos(C)
				// a^2 + b^2 - c^2 = 2*a*b*cos(C)
				// (a^2 + b^2 - c^2) / 2*a*b = cos(C)
			invTargetSpeed = SQRTFAST( velocity[0]*velocity[0] + velocity[1]*velocity[1] + velocity[2]*velocity[2] );
			intersectMinTime	= Distance( origin, playerOrigin ) / sentry->soundLoop;
			intersectMaxTime	= Distance( origin, intersectPos ) / sentry->soundLoop;
			intersectLen		= Distance( playerOrigin, intersectPos ) * invTargetSpeed;
			if( intersectLen >= intersectMaxTime )
			{
				intersectAngle =	(intersectMinTime * intersectMinTime + intersectLen * intersectLen - intersectMaxTime * intersectMaxTime) /
									(2 * intersectMinTime * intersectLen);
				intersectLen = 0.5 * intersectMinTime * intersectAngle;
				VectorNormalize2( velocity, playerDir );
				VectorMA(	playerOrigin,
							intersectLen / invTargetSpeed,
							playerDir, lockpoint );
			}
			else VectorCopy( intersectPos, lockpoint );	// We can't reach it in time, but shoot anyway.
	}
	else {
		VectorCopy( target->client->ps.origin, lockpoint );
	}
	lockz = lockpoint[2];
	targno = target->s.number;
	attno = sentry->s.number;
//	mask = MASK_OPAQUE;// & ~trap_PointContents( origin, sentry->s.number );
	mask = trap_PointContents( origin, attno ) & MASK_WATER;

		// Work out water type of player feet
	if( !ignoreenvironment && mask && (target->watertype & MASK_WATER) && (target->watertype & MASK_WATER) != mask )
		return( qfalse );		// They're in a different enviroment from the player

		// Search for a lock bottom-to-top or top-to-bottom based on the splashDamage flag.
	for( index = sentry->soundLoop ? 0 : 2; sentry->soundLoop ? index < 3 : index >= 0; index += sentry->soundLoop ? 1 : -1 )
	{
			// Lock onto bottom
		if( index == 0 &&
			(ignoreenvironment || (mask && target->waterlevel >= 1) || (!mask && target->waterlevel < 1)) )
		{
			lockpoint[2] = lockz + target->r.mins[2] * 0.75;
			G_Q3F_ForceFieldExtTrace( &tr, origin, NULL, NULL, lockpoint, attno, ENTITYNUM_NONE,  MASK_SHOT );
			if( tr.fraction == 1 || tr.entityNum == targno )
				return( qtrue );
		}

			// Lock onto center
		if( index == 1 &&
			(ignoreenvironment || (mask && target->waterlevel >= 2) || (!mask && target->waterlevel < 2)) )
		{
			G_Q3F_ForceFieldExtTrace( &tr, origin, NULL, NULL, lockpoint, attno, ENTITYNUM_NONE, MASK_SHOT );
			if( tr.fraction == 1 || tr.entityNum == targno )
				return( qtrue );
		}

			// Lock onto top
		if( index == 2 &&
			(ignoreenvironment || (mask && target->waterlevel >= 3) || (!mask && target->waterlevel < 3)) )
		{
			lockpoint[2] = lockz + target->r.maxs[2] * 0.75;
			G_Q3F_ForceFieldExtTrace( &tr, origin, NULL, NULL, lockpoint, attno, ENTITYNUM_NONE, MASK_SHOT );
			if( tr.fraction == 1 || tr.entityNum == targno )
				return( qtrue );
		}
	}

	return( qfalse );
}


void G_Q3F_MapSentryPoll( gentity_t *ent )
{
	// Locate the nearest recognizably enemy entity, and aim sentry at it

	gentity_t *scan, *closest;
	float range, closelen;
	vec3_t forward, target, muzzle;

	AngleVectors( ent->s.angles, forward, NULL, NULL );
	VectorMA( ent->r.currentOrigin, Q3F_MAPSENTRY_AIM_HEIGHT, forward, muzzle );

	if( ent->enemy )
	{
		// We're already tracking - check we still can.

		if(	ent->enemy->inuse && ent->enemy->health > 0 && ent->enemy->client &&
			!ent->enemy->client->noclip &&
			DistanceSquared( muzzle, ent->enemy->r.currentOrigin ) < ent->angle &&
			!Q3F_IsSpectator( ent->enemy->client ) &&
			G_Q3F_CheckCriteria( ent->enemy, ent ) &&
			!(ent->enemy->s.eFlags & EF_Q3F_INVISIBLE) &&
			!(ent->enemy->s.powerups & (1<<PW_INVIS)) )
		{
			// Check we have line-of-sight and range on the enemy
			if(	G_Q3F_MapSentryLock( ent, ent->enemy, muzzle, target, qtrue ) &&
				G_Q3F_MapSentrySetTargetAngle( ent, target, Q3F_MAPSENTRY_ROT_SPEED ) )
				return;		// Same enemy.
		}
		ent->enemy = NULL;									// No enemy
	}

	closelen	= ent->angle;	// Maxmimum range.
	closest		= NULL;
	for( scan = level.gentities; scan < &level.gentities[MAX_CLIENTS]; scan++ )
	{
		// Test each entity for 'targetability'.
		if(	scan->inuse && scan->health > 0 && scan->client &&
			!scan->client->noclip &&
			!Q3F_IsSpectator( scan->client ) &&
			G_Q3F_CheckCriteria( scan, ent ) &&
			!(scan->s.eFlags & EF_Q3F_INVISIBLE) &&
			!(scan->s.powerups & (1<<PW_INVIS)) )
		{
			range = DistanceSquared( muzzle, scan->r.currentOrigin );
			if( range > closelen )
				continue;
			if( !G_Q3F_MapSentryLock( ent, scan, muzzle, target, qfalse ) ||
				!G_Q3F_MapSentrySetTargetAngle( ent, target, Q3F_MAPSENTRY_ROT_SPEED ) )
				continue;		// Outside PVS or line of fire, keep searching
			closest = scan;
			closelen = range;
		}
	}

		// Check our shortlist for an attackable enemy
	if( closest )
	{
		ent->enemy	= closest;
		return;
	}

	/* No enemy found, continue to finding our base angles */
	if (!ent->target_ent)  {
		VectorMA( muzzle, 10000, forward, target );
        G_Q3F_MapSentrySetTargetAngle( ent, target, Q3F_MAPSENTRY_ROT_IDLE_SPEED );
	} else 
		G_Q3F_MapSentrySetTargetAngle( ent, ent->target_ent->r.currentOrigin, Q3F_MAPSENTRY_ROT_IDLE_SPEED );

}

void G_Q3F_MapSentryCalcMuzzlePoint( gentity_t *ent, vec3_t muzzle, vec3_t forward )
{
	// Calculate the 'muzzle point' of the sentry
	AngleVectors( ent->s.angles, forward, NULL, NULL );
	VectorMA( ent->r.currentOrigin, Q3F_MAPSENTRY_AIM_HEIGHT, forward, muzzle );
	VectorCopy( ent->movedir, forward );
}

void G_Q3F_MapSentryBulletFire( gentity_t *ent )
{
	// Fire a bullet from the sentry towards the enemy entity.

	gentity_t		*tent, *traceEnt;
	trace_t		tr;
	vec3_t		muzzle, forward, end;

	G_Q3F_MapSentryCalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, 4096, forward, end);

	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
		ent->last_move_time = level.time;
	} else {
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}
	tent->s.otherEntityNum = ent->s.number;

	if( traceEnt->takedamage )
		G_Damage( traceEnt, ent, ent->parent, forward, tr.endpos, ent->damage, DAMAGE_Q3F_SHELL, MOD_MAPSENTRY_BULLET );
}

#define	MISSILE_PRESTEP_TIME	50
void G_ExplodeMissile( gentity_t *ent );
void G_Q3F_MapSentryRocketFire( gentity_t *ent )
{
	gentity_t	*bolt;
	vec3_t muzzle, forward;

	G_Q3F_MapSentryCalcMuzzlePoint( ent, muzzle, forward );

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = ent->s.number;			// Owned by sentry, will hit sentry owner
	bolt->parent = ent->parent;
	bolt->damage = 92+Q_flrand(0.0f, 1.0f)*20;
	bolt->splashDamage = 92;
	bolt->methodOfDeath = MOD_MAPSENTRY_ROCKET;
	bolt->splashMethodOfDeath = MOD_MAPSENTRY_ROCKET;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( forward, ent->soundLoop, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( muzzle, bolt->r.currentOrigin );

	trap_LinkEntity( bolt );
	return;
}

void G_Q3F_RunMapSentry( gentity_t *ent )
{
	// Position the sentry, poll for enemies, fire if necessary.

		// Set the position (should check for parents that rotate as well)
	if( ent->think ) {
		G_RunThink( ent );
		return;
	}
	if(ent->parent && ent->parent->s.eType == ET_MOVER )
	{
		ent->s.pos = ent->parent->s.pos;
		VectorAdd( ent->s.pos.trBase, ent->pos1, ent->s.pos.trBase );
	}
	else {
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
	}

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
	BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );
	trap_LinkEntity( ent );

		// See if we're active or not.
	if( ent->enemy ) {
		G_Q3F_MapSentryPoll( ent );		// Update target
		if (!ent->enemy)
			ent->timestamp = level.time + 500;
	}
	if( !ent->enemy ) {
		if ( level.time> ent->timestamp ) {
			G_Q3F_MapSentryPoll( ent );
		}

		if( ent->soundPos2 <= level.time )
		{
			if( ent->health < ent->count )
				ent->health++;
			ent->soundPos2 = level.time + ent->noise_index;
		}
		return;
	}
	/* We have an enemy, determine if it's time to FIRE! */
	if( (ent->count && ent->health <= 0) || level.time < ent->soundPos1 )
		return;

	// Shoot the little bastard. Watch those gibs fly, yeah baby!
	// Mental note: Pizza after long periods without food causes odd side-effects
	// in programmer.
	ent->soundPos1 = level.time + ent->splashDamage;
	if( ent->count )
		ent->health--;
	ent->s.frame++;
	if (!ent->s.legsAnim ) {
		G_Q3F_MapSentryBulletFire( ent );
	} else {
		G_Q3F_MapSentryRocketFire( ent );
	}
	G_RunThink( ent );
}
