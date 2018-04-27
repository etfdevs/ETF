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
**	g_q3f_grenades.c
**
**	Server-side functions for handling grenades
*/

#include "g_local.h"
#include "g_q3f_grenades.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_weapon.h"
#include "g_q3f_team.h"
#include "bg_q3f_util.h"
#include "g_q3f_mapents.h"

/*
**	server-side grenade definitions
*/

extern bg_q3f_grenade_t bg_q3f_grenade_none;
extern bg_q3f_grenade_t bg_q3f_grenade_normal;
extern bg_q3f_grenade_t bg_q3f_grenade_concuss;
extern bg_q3f_grenade_t bg_q3f_grenade_flash;
extern bg_q3f_grenade_t bg_q3f_grenade_flare;
extern bg_q3f_grenade_t bg_q3f_grenade_nail;
extern bg_q3f_grenade_t bg_q3f_grenade_cluster;
extern bg_q3f_grenade_t bg_q3f_grenade_clustersection;
extern bg_q3f_grenade_t bg_q3f_grenade_napalm;
extern bg_q3f_grenade_t bg_q3f_grenade_gas;
extern bg_q3f_grenade_t bg_q3f_grenade_emp;
extern bg_q3f_grenade_t bg_q3f_grenade_charge;

/*
**	Constants for the size of the effects
*/

#define GREN_GAS_RADIUS 150
#define GREN_GAS_PARAMEDIC_RESIST		0.66f
#define GREN_STUN_MINIGUNNER_RESIST		0.66f
#define GREN_STUN_PARAMEDIC_RESIST		0.66f
#define GREN_STUN_MAX_TIME				26000


/*
**	Throw and explosion effects
*/

qboolean ConcussExplode( gentity_t *ent )
{
	//	We've exploded, tell all affected clients the good news

	gentity_t *player;
	gentity_t *temp;
	int distance, effect;
	vec3_t distancevec, bouncevec;
	trace_t trace;
	bg_q3f_playerclass_t *cls;

	for( player = level.gentities; player < &level.gentities[MAX_CLIENTS]; player++ )
	{
		if( !(player->inuse && player->client && player->health > 0 && !Q3F_IsSpectator( player->client ) && !player->client->noclip) )
			continue;
		VectorSubtract( player->s.pos.trBase, ent->s.pos.trBase, distancevec );
		distance = VectorLength( distancevec );
		if ( distance >= 270 )		// Replace this with a define?
			continue;
		G_Q3F_ForceFieldExtTrace( &trace, ent->r.currentOrigin, NULL, NULL, player->r.currentOrigin, (ent - level.gentities), ent->r.ownerNum, MASK_SOLID|CONTENTS_FORCEFIELD );
		if( trace.contents & CONTENTS_FORCEFIELD )
			continue;	// Let's not concuss them through a forcefield.
		effect = 17000 * (1.0 - distance*distance / ((player == ent->activator) ? 145800.0 : 97200.0) );
		if( trace.fraction != 1.0 && trace.entityNum != (player - level.gentities) ) {
			effect *= 0.5;		// Reduce effect through walls
		}
		switch( player->client->ps.persistant[PERS_CURRCLASS] ) {
		case Q3F_CLASS_PARAMEDIC:
			effect *= GREN_STUN_PARAMEDIC_RESIST;
			if( player->client->ps.powerups[PW_Q3F_CONCUSS] > level.time )
				effect += (player->client->ps.powerups[PW_Q3F_CONCUSS] - level.time);
			if (effect > ( GREN_STUN_PARAMEDIC_RESIST * GREN_STUN_MAX_TIME ))
				effect = GREN_STUN_PARAMEDIC_RESIST * GREN_STUN_MAX_TIME;
			break;
		case Q3F_CLASS_MINIGUNNER:
			effect *= GREN_STUN_MINIGUNNER_RESIST;
			if( player->client->ps.powerups[PW_Q3F_CONCUSS] > level.time )
				effect += (player->client->ps.powerups[PW_Q3F_CONCUSS] - level.time);
			if (effect > ( GREN_STUN_MINIGUNNER_RESIST * GREN_STUN_MAX_TIME ))
				effect = GREN_STUN_MINIGUNNER_RESIST * GREN_STUN_MAX_TIME;
			break;
		default:
			if( player->client->ps.powerups[PW_Q3F_CONCUSS] > level.time )
				effect += (player->client->ps.powerups[PW_Q3F_CONCUSS] - level.time);
			if (effect > GREN_STUN_MAX_TIME )
				effect = GREN_STUN_MAX_TIME;
			break;
		}
		player->client->ps.powerups[PW_Q3F_CONCUSS] = level.time + effect;
		player->client->ps.generic1 = rand();
		if( !(player->flags & FL_NO_KNOCKBACK) )
		{
			// And send them flying
			cls = BG_Q3F_GetClass( &player->client->ps );
	#if 0
			// Sensible conc blast
			BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );
//		}
			VectorNormalize( distancevec );
			if( trace.fraction != 1.0 && trace.entityNum != level.gentities-player )
				distance *= 2;		// Reduce effect through walls
			if( cls->mass != 200 )	// 200 is default mass
				distance *= 200 / cls->mass;
			if( ent->count && player == ent->activator )
				VectorScale( distancevec, (1.0 - distance / 400.0) * 230 /*g_grenadeConcBlast.integer*/ / 4, bouncevec );
			else VectorScale( distancevec, (1.0 - distance / 400.0) * 230 /*g_grenadeConcBlast.integer*/, bouncevec );
	#else
			// TF-style conc-blast (Oh, the horror!)
			effect = distance * ((230 /*g_grenadeConcBlast.integer*/ - 0.5 * distance) / 20);
			if( cls->maxammo_nails != 200 )
				effect *= 200 / cls->mass;
			VectorNormalize2( distancevec, bouncevec );
			VectorScale( bouncevec, effect, bouncevec );
			// RR2DO2: Little hack(?) to limit horizontal conc blast
			/*bouncevec[0] *= g_grenadeConcHBlast.value * 0.01f;
			bouncevec[1] *= g_grenadeConcHBlast.value * 0.01f;
			bouncevec[2] *= g_grenadeConcVBlast.value * 0.01f;*/
			bouncevec[0] *= 0.8f;
			bouncevec[1] *= 0.8f;
			bouncevec[2] *= 0.95f;
	#endif
			VectorAdd( player->client->ps.velocity, bouncevec, player->client->ps.velocity );
		}
	}

	temp = G_TempEntity( ent->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	temp->s.eventParm = ETF_GRENDADE_EXPLOSION_CONCUSSION;
	temp->s.angles[1] = bg_q3f_grenade_concuss.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}

#define	FLASHRADIUS 200.f
qboolean FlashExplode( gentity_t *ent )
{
	//	We've exploded, tell all affected clients the good news

	gentity_t *player;
	gentity_t *temp;
	int distance, effect;
	vec3_t distancevec;
	trace_t trace;
	int neweffect;


	for( player = level.gentities; player < &level.gentities[MAX_CLIENTS]; player++ )
	{
		if( !(player->inuse && player->client && !Q3F_IsSpectator( player->client ) && !player->client->noclip) )
			continue;
		VectorSubtract( player->s.pos.trBase, ent->s.pos.trBase, distancevec );
		distance = VectorLength( distancevec );
		if ( distance >= FLASHRADIUS )
			continue;

//		if( player == ent->activator || g_friendlyFire.integer == 1 || !G_Q3F_IsAllied( ent->activator, player ) )
//		{
			// Leave this one effective through forcefields :)
			trap_Trace( &trace, ent->r.currentOrigin, NULL, NULL, player->r.currentOrigin, (level.gentities-ent), MASK_OPAQUE );
			effect = 9000 * ( 1.0 - ((distance * distance ) / (FLASHRADIUS * FLASHRADIUS)) );
			if( trace.fraction != 1.0 && trace.entityNum != level.gentities-player )
				effect *= 0.5;		// Reduce effect through walls
			if( player->client->ps.powerups[PW_Q3F_FLASH] > level.time )
				effect += (player->client->ps.powerups[PW_Q3F_FLASH] - level.time) * (player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC ? 1.75 : 1);
			if( effect > 3000 )
				effect = 3000;
			if( player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC )
				effect *= 0.66f;
			neweffect = level.time + effect;
			if(neweffect > player->client->ps.powerups[PW_Q3F_FLASH])
				player->client->ps.powerups[PW_Q3F_FLASH] = neweffect;
			BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );
//		}
	}

	temp = G_TempEntity( ent->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	temp->s.eventParm = ETF_GRENDADE_EXPLOSION_FLASH;
	temp->s.angles[1] = bg_q3f_grenade_flash.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}

void FlareThrow( gentity_t *flare )
{
	// Throw a flare... much faster than ordinary grenades

	VectorScale( flare->s.pos.trDelta, 2, flare->s.pos.trDelta );
}

qboolean FlareExplode( gentity_t *flare )
{
	// Do nothing, just hang around for the next 32 seconds or so...

	flare->nextthink = level.time + 32000;
	return( qtrue );
}

static void G_Q3F_GrenadeThink( gentity_t *ent );
qboolean ClusterExplode( gentity_t *cluster )
{
	// Spawn half a dozen more grenades.

	gentity_t *segment;
	gentity_t *temp;
	g_q3f_grenade_t *gren;
	int index;

	gren = G_Q3F_GetGrenade( Q3F_GREN_CLUSTERSECTION );

	for( index = 0; index < 6; index++ )
	{
		segment = G_Spawn();

		segment->s.eType = ET_Q3F_GRENADE;
		segment->classname = "handgrenade"; /* Ensiform - This was "grenade", should've been "handgrenade" */
		//VectorSet( segment->r.mins, -4, -4, -4 );	// Fix this?
		//VectorSet( segment->r.maxs, 4, 4, 4 );
		VectorSet( segment->r.mins, -2.f, -2.f, -2.f );	// Fix this?
		VectorSet( segment->r.maxs, 2.f, 2.f, 2.f );
		segment->touch = cluster->touch;
		segment->s.weapon	= Q3F_GREN_CLUSTERSECTION;					// The actual grenade type
		segment->r.ownerNum	= cluster->r.ownerNum;
		segment->s.time		= level.time + 2000 + (rand() % 1000);		// Time it explodes
		segment->nextthink	= segment->s.time;
		segment->think		= G_Q3F_GrenadeThink;
		segment->activator	= cluster->activator;
		segment->s.eFlags |= EF_BOUNCE;
		segment->s.groundEntityNum = ENTITYNUM_NONE;

		G_SetOrigin( segment, cluster->r.currentOrigin );

		// set aiming directions
//		segment->s.pos.trDelta[0]	= Q_flrand(-1.0f, 1.0f) * 230;
//		segment->s.pos.trDelta[1]	= Q_flrand(-1.0f, 1.0f) * 230;
//		segment->s.pos.trDelta[2]	= 230 + Q_flrand(0.0f, 1.0f) * 115;
		segment->s.pos.trDelta[0]	= Q_flrand(-1.0f, 1.0f) * 100;
		segment->s.pos.trDelta[1]	= Q_flrand(-1.0f, 1.0f) * 100;
		segment->s.pos.trDelta[2]	= 350 + Q_flrand(0.0f, 1.0f) * 100;

		segment->s.pos.trType		= TR_GRAVITY;
		segment->s.pos.trTime		= level.time;

		// Set angles
		segment->s.apos.trDelta[YAW]	= 5;
		segment->s.apos.trDelta[PITCH]	= 360;
		segment->s.apos.trDelta[ROLL]	= 0;
		segment->s.apos.trTime			= level.time;
		segment->s.apos.trType			= TR_LINEAR;

		if( gren->ThrowGren )
			gren->ThrowGren( segment );				// Custom throw processing.

		if( cluster->s.powerups & (1 << PW_QUAD) )
		{
			segment->s.powerups = 1 << PW_QUAD;
			segment->s.otherEntityNum = cluster->activator->s.number;
		}
		//Give cluster section same water status
		segment->hasbeeninwater = cluster->hasbeeninwater;

		trap_LinkEntity( segment );
	}

	temp = G_TempEntity( cluster->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	temp->s.weapon = Q3F_GREN_CLUSTER;
	if(cluster->s.powerups & (1 << PW_QUAD))
	{
		temp->s.powerups = 1 << PW_QUAD;
		temp->s.otherEntityNum = cluster->activator->s.number;
	}
	temp->s.angles[1] = bg_q3f_grenade_cluster.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}

#define	EMPRADIUS 250
static void G_Q3F_EmpItemExplode( gentity_t *item )
{
	// 'Blow up' an item. Uses the item->damage field for the damage

	gentity_t *tent;

	Touch_Item( item, NULL, NULL );		// Reset it ready for the next event
	tent = G_TempEntity( item->r.currentOrigin, EV_MISSILE_MISS );
	tent->s.weapon = WP_ROCKET_LAUNCHER;
	if (item->splashDamage) 
		tent->s.weapon |= 16;
	tent->s.otherEntityNum = ENTITYNUM_WORLD;
	tent->s.otherEntityNum2 = item->activator->s.number;
	G_RadiusDamage( item->r.currentOrigin, item, item->activator, item->damage,  NULL, MOD_PULSEGREN, 0 );
	item->activator = NULL;
}

qboolean EmpExplode( gentity_t *emp )
{
	// Do damage to all susceptible entities. Full damage is done to all ents in range.

	gentity_t *ent;
	gentity_t *temp;
	vec3_t vec;
	float damage;
	int avail_ammo, max_clip;
	int given;


	if (emp->activator && emp->activator->client) 
		given = emp->activator->client->pers.stats.data[STATS_GREN + Q3F_GREN_EMP].given;
	else given = -1;

	for( ent = g_entities; ent < &g_entities[MAX_GENTITIES]; ent++ )
	{
		if( !ent->inuse )
			continue;
		VectorCopy( emp->r.currentOrigin, vec );
		VectorSubtract( vec, ent->r.currentOrigin, vec );
		if( VectorLength( vec ) > EMPRADIUS )
			continue;
		if( ent->client )//&& (ent == emp->activator || g_friendlyFire.integer ||
//			!G_Q3F_IsAllied( emp->activator, ent )) )
		{
			// It's a player.

			//damage =	(int) (0.75 * ent->client->ps.ammo[AMMO_SHELLS]) +
			//			1.5 * (int) (0.75 * ent->client->ps.ammo[AMMO_ROCKETS]);
			damage =	(int) (0.85 * ent->client->ps.ammo[AMMO_SHELLS]) +
						1.75 * (int) (0.75 * ent->client->ps.ammo[AMMO_ROCKETS]);

			if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
				damage += 0.75 * ent->client->ps.ammo[AMMO_CELLS];
			if( damage > 0 )
			{
				vec3_t emporg, dir;

				max_clip = 0;
				max_clip = Q_max( Q3F_GetClipValue( WP_SHOTGUN, &ent->client->ps ), Q3F_GetClipValue( WP_SUPERSHOTGUN, &ent->client->ps ) );
				avail_ammo = ent->client->ps.ammo[AMMO_SHELLS] - max_clip;
				ent->client->ps.ammo[AMMO_SHELLS] = avail_ammo * 0.25 + max_clip;
				max_clip = 0;
				max_clip = Q_max( Q3F_GetClipValue( WP_ROCKET_LAUNCHER, &ent->client->ps ), Q3F_GetClipValue( WP_GRENADE_LAUNCHER, &ent->client->ps ) );
				max_clip = Q_max( max_clip, Q3F_GetClipValue( WP_PIPELAUNCHER, &ent->client->ps ) );
				avail_ammo = ent->client->ps.ammo[AMMO_ROCKETS] - max_clip;
				ent->client->ps.ammo[AMMO_ROCKETS] = avail_ammo * 0.25 + max_clip;
				/*ent->client->ps.ammo[AMMO_SHELLS] *= 0.25;
				ent->client->ps.ammo[AMMO_ROCKETS] *= 0.25;*/
				if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
					ent->client->ps.ammo[AMMO_CELLS] *= 0.25;

				// Knockback direction
				VectorCopy( emp->r.currentOrigin, emporg );
				emporg[2]-=16;
				VectorSubtract( ent->r.currentOrigin, emporg, dir );

				G_Damage( ent, emp, emp->activator, dir, NULL, damage, DAMAGE_Q3F_SHOCK, MOD_PULSEGREN );
				Q3F_CapClips(&(ent->client->ps));
			}
		}
		else if(	(ent->s.eType == ET_Q3F_SENTRY || ent->s.eType == ET_Q3F_SUPPLYSTATION) )
//					(g_friendlyFire.integer || !G_Q3F_IsAllied( emp->activator, ent )) )
		{
			// It's an autosentry/supply station

			G_Damage( ent, emp, emp->activator, NULL, NULL, 150, DAMAGE_Q3F_SHOCK, MOD_PULSEGREN );
		}
		else if( ent->s.eType == ET_Q3F_GRENADE )
		{
			// It's another grenade (check for recursive pulse detonation first :)

			if( ent->s.time > (level.time + 512) )
			{
				if( !Q_stricmp( ent->classname, "charge" ) ) {
					ent->count	= 2 + (rand() % 3);		// Delayed detonation
					ent->nextthink	= level.time;
					ent->s.time = level.time + ent->count * 1000;
				} else {
					ent->nextthink	= level.time + (rand() & 511);
					ent->s.time		= ent->nextthink;
				}
			}
		}
		else if(	!Q_stricmp( ent->classname, "pipe" ) ||
					!Q_stricmp( ent->classname, "grenade" ) ||
					!Q_stricmp( ent->classname, "rocket" ) )
		{
			// It's an explosive missile
			ent->nextthink = level.time + (rand() & 511);
		}
		else if( ent->s.eType == ET_ITEM && !(ent->s.eFlags & EF_NODRAW) )
		{
			// It's an item, 'use' it and fake an explosion.

			if( ent->item->giType == IT_AMMO )
				damage = ent->item->quantity * 2;
			else if( ent->item->giType == IT_POWERUP )
				damage = ent->item->quantity * 3;
			else if( ent->item->giType == IT_Q3F_AMMOBOX || ent->item->giType == IT_Q3F_BACKPACK )
			{
				damage =	0.75 * ent->s.time2 +
							2.25 * ent->s.torsoAnim +
							0.75 * ent->s.weapon;
			}
			else damage = 0;

			if( damage )
			{
				ent->r.contents	= 0;		// Prevent trigger
				ent->think		= G_Q3F_EmpItemExplode;
				ent->damage		= damage;		// Not used by items
				ent->activator	= emp->activator;
				if(emp->s.powerups & (1 << PW_QUAD))
					ent->splashDamage = 1;
				else
					ent->splashDamage = 0;
				ent->nextthink	= level.time + (rand() & 511);
			}
		}
		else if( !Q_stricmp( ent->classname, "func_damage" ) )
		{
			// Ensiform : Should we maybe come up with an algo to 
			// allow emp's to still cause damage to func_damage ents.
		}
	}

	if (given >= 0 && given < emp->activator->client->pers.stats.data[STATS_GREN + Q3F_GREN_EMP].given) {
		given = emp->activator->client->pers.stats.data[STATS_GREN + Q3F_GREN_EMP].hits++;
	}

	temp = G_TempEntity( emp->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	temp->s.angles[1] = bg_q3f_grenade_emp.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}

#define	NAPALMRADIUS 170
extern void HallucinogenicExplodeThink( gentity_t *ent );
extern void HallucinogenicBurnThink( gentity_t * gasgren );

#if 0
void GetEntityCenter( gentity_t *ent, vec3_t _pos )
{
	VectorSubtract(ent->r.absmin, ent->r.mins, _pos);
}
#endif

void NapalmExplodeThink( gentity_t *napalm )
{
	// Explode for twenty seconds or so, setting fire to people as we go.

	gentity_t *ent;
	vec3_t diff, midpoint;
	float fraction;

	if( (napalm->s.time + 10000) < level.time || (trap_PointContents( napalm->r.currentOrigin, ENTITYNUM_NONE ) & (CONTENTS_WATER|CONTENTS_SLIME)) )
		G_FreeEntity( napalm );
	else {
		for( ent = g_entities; ent < &g_entities[level.num_entities]; ent++ )
		{
			vec3_t dir;

			if( !ent->inuse || !(ent->s.eType == ET_PLAYER || ent->s.eType == ET_Q3F_SENTRY || ent->s.eType == ET_Q3F_SUPPLYSTATION || ( ent->s.eType == ET_GENERAL && !Q_stricmp( ent->classname, "func_damage" ) ) ) )
				continue;
			VectorCopy( napalm->r.currentOrigin, diff );
			if( !Q_stricmp( ent->classname, "func_damage" ) )
			{
				VectorAdd( ent->r.absmin, ent->r.absmax, midpoint );
				VectorScale( midpoint, 0.5, midpoint );
				VectorSubtract( diff, midpoint, diff );
			}
			else
			{
				VectorSubtract( diff, ent->r.currentOrigin, diff );
			}
			fraction = VectorLength( diff ) / NAPALMRADIUS;
			if( fraction > 1 || !CanDamage( ent, napalm->r.currentOrigin, &g_entities[ent->r.ownerNum] ) )
				continue;
			VectorNegate( diff, dir );
			G_Damage( ent, napalm, napalm->activator, dir, napalm->r.currentOrigin, 35 * (1 - fraction * fraction), DAMAGE_Q3F_FIRE , MOD_NAPALMGREN );
			G_Q3F_Burn_Person( ent, napalm->activator );
		}

		//Canabis simplified checking if we're burning gas
		HallucinoGenicCheckFire(napalm->r.currentOrigin,16,&g_entities[napalm->r.ownerNum]); // Ensiform: was 16, never did anything

		napalm->think		= NapalmExplodeThink;
		napalm->nextthink	= level.time + 800;
	}
}

qboolean NapalmExplode( gentity_t *napalm )
{
	gentity_t *temp;

	napalm->s.pos.trTime = level.time;
	napalm->s.pos.trDelta[0] = Q_flrand(-1.0f, 1.0f) * 10;
	napalm->s.pos.trDelta[1] = Q_flrand(-1.0f, 1.0f) * 10;
	napalm->s.pos.trType = TR_GRAVITY;
	napalm->soundLoop = 1;				//Tag the grenade as having exploded
	NapalmExplodeThink( napalm );

	temp = G_TempEntity( napalm->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	if(napalm->s.powerups & (1 << PW_QUAD))
	{
		temp->s.powerups = 1 << PW_QUAD;
		temp->s.otherEntityNum = napalm->activator->s.number;
	}
	temp->s.angles[1] = bg_q3f_grenade_flash.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}

extern void HallucinogenicExplodeThink( gentity_t *ent );
extern void HallucinogenicBurnThink( gentity_t * gasgren );

void HallucinoGenicCheckFire(vec3_t flamepos,float flameradius,gentity_t * owner) {
	gentity_t *gasgren;

	for( gasgren = g_entities; gasgren < &g_entities[MAX_GENTITIES]; gasgren++ ) {
		vec3_t vec;

		if( !gasgren->inuse )
			continue;
		//CaNaBiS, Check if we are passing through a gas cloud
		VectorCopy( flamepos, vec );
		VectorSubtract( vec, gasgren->r.currentOrigin, vec );
		if(( VectorLength( vec )-flameradius) > GREN_GAS_RADIUS )
			continue;
		if( !Q_stricmp( gasgren->classname, "handgrenade" ) &&
			gasgren->s.weapon == Q3F_GREN_GAS && 
			!gasgren->freeAfterEvent &&
			gasgren->think == HallucinogenicExplodeThink )
		{
			gasgren->enemy=owner;
			HallucinogenicBurnThink( gasgren );
			G_AddEvent( gasgren, EV_ETF_GASEXPLOSION, 0 );
			gasgren->freeAfterEvent = qtrue;
		}
	}
}

void HallucinogenicBurnThink( gentity_t * gasgren ) 
{
	// with a nice explosion of fire...
	gentity_t *exp_target;
	vec3_t diff, dir;
	float fraction;

	//Canabis, maybe make the burn effect take longer?
	if (gasgren->think == HallucinogenicBurnThink) {
		G_FreeEntity(gasgren);
		return;
	}

	for( exp_target = g_entities; exp_target < &g_entities[level.num_entities]; exp_target++ ) {
		if( !exp_target->inuse || !(exp_target->s.eType == ET_PLAYER || exp_target->s.eType == ET_Q3F_SENTRY || exp_target->s.eType == ET_Q3F_SUPPLYSTATION) )
			continue;
		VectorCopy( gasgren->r.currentOrigin, diff );
		VectorSubtract( diff, exp_target->r.currentOrigin, diff );
		fraction = VectorLength( diff ) / GREN_GAS_RADIUS;
		if( fraction > 1 || !CanDamage( exp_target, gasgren->r.currentOrigin, gasgren->enemy ) )
			continue;
		VectorNegate( diff, dir );
		//Only do explosion damage on first bang
		if (gasgren->think != HallucinogenicBurnThink)
			G_Damage( exp_target, gasgren->enemy, gasgren->enemy, dir, gasgren->r.currentOrigin, 140 * (1 - fraction * fraction), DAMAGE_Q3F_FIRE, MOD_GASEXPLOSION );
		G_Q3F_Burn_Person( exp_target, gasgren->enemy );
	}
	gasgren->think		= HallucinogenicBurnThink;
	gasgren->nextthink	= level.time + 300;
}


void HallucinogenicExplodeThink( gentity_t *ent )
{
	//	Let's gas

	gentity_t *player;
	int distance, time;
	vec3_t distancevec;

	if( (ent->s.time + 20000) < level.time )
	{
		G_FreeEntity( ent );
		return;
	}
	ent->think		= HallucinogenicExplodeThink;
	ent->nextthink	= level.time + 500;
	for( player = level.gentities; player < &level.gentities[MAX_CLIENTS]; player++ )
	{
		if( !(player->inuse && player->client && player->health > 0 && !Q3F_IsSpectator( player->client ) && !player->client->noclip) )
			continue;
		VectorSubtract( player->s.pos.trBase, ent->s.pos.trBase, distancevec );
		distance = VectorLength( distancevec );
		if ( distance >= GREN_GAS_RADIUS )
			continue;

		if( player == ent->activator || level.friendlyFire == FF_Q3F_FULL || level.friendlyFire == FF_Q3F_HALF || !G_Q3F_IsAllied( ent->activator, player ) )
		{
			// RR2DO2: flametroopers can't be gassed
			if( player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_FLAMETROOPER )
				continue;

			if( !CanDamage( player, ent->r.currentOrigin, &g_entities[ent->r.ownerNum] ) )
				continue;
			// canabis, check if the player was already recently gassed
			if (player->client->lastgasTime+500>level.time)
				continue;
			player->client->lastgasTime=level.time;

			G_Damage( player, ent, ent->activator, NULL, NULL, 5, DAMAGE_NO_ARMOR, MOD_GASGREN );
				// Gas them

			time = player->client->ps.powerups[PW_Q3F_GAS] - level.time;
			if( player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC )
				time /= 0.66;
			time += 5000;
			if( time < 12000 )
				time = 12000;
			if( time > 22000 )
				time = 22000;
			if( player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC )
				time *= 0.66;
			player->client->ps.powerups[PW_Q3F_GAS] = level.time + time;
			BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );
		}
	}
}
qboolean HallucinogenicExplode( gentity_t *ent )
{

//	ent->soundLoop = 1;				//Tag the grenade as having exploded
	HallucinogenicExplodeThink( ent );

//	temp = G_TempEntity( ent->s.pos.trBase, EV_Q3F_GRENADE_EXPLOSION );
//	temp->s.angles[1] = bg_q3f_grenade_flash.damage;
//	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}


/*
**	Nail grenade code (enough to merit it's own comment :)
*/

struct gnail {
	float xvel, yvel, angle, xpos, ypos;
	int starttime;
};
static struct gnail *NailArrayCreate( gentity_t *grenade )
{
	// Create the array, and populate with coordinates & vectors

	struct gnail *array;
	struct gnail *ptr;
	int index, starttime;
	vec3_t angles, direction;
	char *bitptr;

		// Allocate a set of gnails for each nail, plus enough for a bitfield
	array = G_Alloc( sizeof(struct gnail) * Q3F_NUM_NAILGRENNAILS + (Q3F_NUM_NAILGRENNAILS>>3) );

	angles[YAW] = angles[PITCH] = angles[ROLL] = 0;
	ptr = (struct gnail *) array;
	starttime = grenade->s.time + 500;
	for( index = 0; index < Q3F_NUM_NAILGRENNAILS; )
	{
		AngleVectors( angles, direction, 0, 0 );

		ptr->xpos		= grenade->r.currentOrigin[0];
		ptr->ypos		= grenade->r.currentOrigin[1];
		ptr->xvel		= direction[0];
		ptr->yvel		= direction[1];
		ptr->angle		= angles[YAW];
		ptr->starttime	= starttime;
		index++;
		ptr++;

		ptr->xpos		= grenade->r.currentOrigin[0];
		ptr->ypos		= grenade->r.currentOrigin[1];
		ptr->xvel		= -direction[0];
		ptr->yvel		= -direction[1];
		ptr->starttime	= starttime;
		ptr->angle		= angles[YAW];
		index++;
		ptr++;
		angles[YAW] += Q3F_NAILGRENANGLE;
		starttime += Q3F_NAILGRENINTERVAL;
	}

	bitptr = (char *) (array + Q3F_NUM_NAILGRENNAILS);
	memset( bitptr, 0xFF, (Q3F_NUM_NAILGRENNAILS >> 3) );
	return( array );
}

#define	NBITCOPY(w,x,y,z) if(z<Q3F_NUM_NAILGRENNAILS){*(y+(char *)x) = *w++; z+=8;}
static void PackNailBits( gentity_t *grenade, char *bitptr )
{
	// Pack the nail bits into the entitystate (macro is to prevent overflows)
	int size, temp;

	size = 0;
	NBITCOPY( bitptr, &grenade->s.time2,			0, size )
	NBITCOPY( bitptr, &grenade->s.time2,			1, size )
	NBITCOPY( bitptr, &grenade->s.time2,			2, size )
	NBITCOPY( bitptr, &grenade->s.time2,			3, size )

	temp = 0;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.origin2[0] = temp;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.origin2[1] = temp;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.origin2[2] = temp;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.angles2[0] = temp;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.angles2[1] = temp;
	NBITCOPY( bitptr, &temp,						0, size );
	NBITCOPY( bitptr, &temp,						1, size );
	grenade->s.angles2[2] = temp;

	NBITCOPY( bitptr, &grenade->s.constantLight,	0, size )
	NBITCOPY( bitptr, &grenade->s.constantLight,	1, size )
//	NBITCOPY( bitptr, &grenade->s.otherEntityNum,	0, size )
	NBITCOPY( bitptr, &grenade->s.otherEntityNum2,	0, size )
	NBITCOPY( bitptr, &grenade->s.modelindex2,		0, size )
	NBITCOPY( bitptr, &grenade->s.legsAnim,			0, size )
	NBITCOPY( bitptr, &grenade->s.torsoAnim,		0, size )

	NBITCOPY( bitptr, &grenade->s.generic1,			0, size )
	NBITCOPY( bitptr, &grenade->s.generic1,			1, size )
//	NBITCOPY( bitptr, &grenade->s.generic1,			2, size )
//	NBITCOPY( bitptr, &grenade->s.generic1,			3, size )
}

static void NailThink( gentity_t *grenade )
{
	// Allocate space for a set of point 'nails', their vectors, and their
	// start time. Once that's done, the NailThink function can work out
	// if they hit anything and remove their bit from the array.

	int index, damage;
	trace_t trace;
	struct gnail *array, *ptr;
	char *bitptr;
	vec3_t start, end;
	//qboolean nailhit;
	gentity_t *hitent;
	vec3_t mins, maxs;

	//nailhit = qfalse;
	if( (grenade->s.time + Q3F_NAILGRENADETIME) <= level.time )
	{
		// We're finished altogether, just clean up and go.
		G_Free( (void *) grenade->health );
		G_FreeEntity( grenade );
		return;
	}
	if( !grenade->damage && (grenade->s.time + Q3F_NAILGRENINTERVAL * Q3F_NUM_NAILGRENNAILS / 2 + 500) <= level.time )
	{
		gentity_t *temp;

		// Do a trailing explosion

		damage = 120;
		if( grenade->s.powerups & (1 << PW_QUAD) )
			G_AddEvent( grenade, EV_POWERUP_QUAD, 0 );
		G_RadiusDamage(	grenade->r.currentOrigin, grenade, &level.gentities[grenade->r.ownerNum],
						damage, NULL, MOD_NAILGREN, 0 );
		grenade->damage = 1;
		grenade->r.contents = 0;	// no longer solid

		// BOOM! :D
		temp = G_TempEntity( grenade->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
		// Fudge the nail bomb explosion origin up by 30 units... Mmmm fudge... :D
		temp->s.pos.trBase[2] += 30;
		SnapVector(temp->s.pos.trBase);	// save network bandwidth
		G_SetOrigin(temp, temp->s.pos.trBase);
		temp->s.weapon = Q3F_GREN_NAIL;
		if(grenade->s.powerups & (1 << PW_QUAD))
		{
			temp->s.powerups |= 1 << PW_QUAD;
			temp->s.otherEntityNum = grenade->activator->s.number;
		}
		temp->s.angles[1] = bg_q3f_grenade_flash.damage;
		temp->r.svFlags = SVF_BROADCAST;	// send to everyone
	}

	array = (struct gnail *) grenade->health;
	bitptr = (char *) (array + Q3F_NUM_NAILGRENNAILS);

	mins[0] = mins[1] = mins[2] = -4;
	maxs[0] = maxs[1] = maxs[2] = 4;
	//nailhit = qfalse;
	for( index = 0; index < Q3F_NUM_NAILGRENNAILS; index++ )
	{
		ptr = array + index;
		if( !ptr->starttime || ptr->starttime > level.time )
			continue;
		start[0]	= ptr->xpos;
		start[1]	= ptr->ypos;
		start[2]	= grenade->r.currentOrigin[2];
		end[0]		= grenade->r.currentOrigin[0] + (level.time - ptr->starttime) * ptr->xvel * Q3F_NAILSPEED / 1000;
		end[1]		= grenade->r.currentOrigin[1] + (level.time - ptr->starttime) * ptr->yvel * Q3F_NAILSPEED / 1000;
		end[2]		= grenade->r.currentOrigin[2];
		//trap_Trace( &trace, start, mins, maxs, end, grenade->s.number, MASK_SHOT );
		G_Q3F_ForceFieldExtTrace( &trace, start, mins, maxs, end, grenade->s.number, grenade->r.ownerNum, MASK_SHOT );
		if( trace.startsolid || trace.fraction < 1 )
		{
			// We hit something. What a shame.

			if( trace.entityNum < ENTITYNUM_MAX_NORMAL )
			{
				vec3_t dir;

				// We hit something the client may not know about, so flag it to them.
				// We let the client calculate 'world' and 'forcefield' collisions.

				if( !(trace.contents & CONTENTS_FORCEFIELD) )
					*(bitptr + (index>>3)) = *(bitptr + (index>>3)) & ~(1<<(index&7));
				//nailhit = qtrue;
				hitent = &level.gentities[trace.entityNum];
				VectorSubtract( end, start, dir );
				G_Damage( hitent, grenade, grenade->activator, dir, end, 30, DAMAGE_Q3F_SHELL, MOD_NAILGREN );
			}
			else if( trace.startsolid )
			{
				vec3_t dir;

				// Someone's standing _in_ the grenade - check if it's the player.
				// Shouldn't have to check this, but if startsolid is true, it's 
				// always ENTITYNUM_WORLD for some reason.

				VectorCopy( grenade->activator->r.currentOrigin, end );
				if(	end[0] >= (start[0] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->mins[0]) &&
					end[0] <= (start[0] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->maxs[0]) &&
					end[1] >= (start[1] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->mins[1]) &&
					end[1] <= (start[1] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->maxs[1]) &&
					end[2] >= (start[2] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->mins[2]) &&
					end[2] <= (start[2] + bg_q3f_classlist[Q3F_CLASS_SOLDIER]->maxs[2]) )
				{
					*(bitptr + (index>>3)) = *(bitptr + (index>>3)) & ~(1<<(index&7));
					//nailhit = qtrue;
					VectorSubtract( end, start, dir );
					G_Damage( grenade->activator, grenade, grenade->activator, dir, end, 30, DAMAGE_Q3F_SHELL, MOD_NAILGREN );
				}
			}
			ptr->starttime = 0;		// Mark this nail as expired.
		}
		else {
			ptr->xpos = end[0];
			ptr->ypos = end[1];
		}
	}
		// We need to pack the bits into the entitystate for transmission to the client
	if( grenade->pain_debounce_time <= level.time )
	{
		grenade->pain_debounce_time = level.time + Q3F_NAILGREN_PACKINTERVAL/*g_nailGrenUpdateInterval.integer*/;
		PackNailBits( grenade, bitptr );
	}
	
	grenade->nextthink = grenade->nextthink + FRAMETIME;

	G_AddEvent(grenade, EV_VISUAL_NAILFIRE, 0);
}

#define NAILBOMB_SOLID

qboolean NailExplode( gentity_t *grenade )
{
	// Rise 30 units and then start spewing out nails.

	vec3_t vec;
	trace_t tr;

	// Do a trace and see if we can rise the full 30 units.
	VectorCopy( grenade->r.currentOrigin, vec );
	vec[2] += 30;
//	trap_Trace( &tr, grenade->r.currentOrigin, grenade->r.mins, grenade->r.maxs, vec, grenade->activator->s.number, MASK_SOLID );
	G_Q3F_ForceFieldTrace( &tr, grenade->r.currentOrigin, grenade->r.mins, grenade->r.maxs, vec, grenade->activator->s.number, MASK_SOLID );
	if( tr.startsolid )
		tr.fraction = 0;

	G_SetOrigin( grenade, grenade->r.currentOrigin );

	VectorSet( grenade->s.pos.trDelta, 0, 0, grenade->count ? 60 : 120 );	// 1/4 second, so 4x per-second movement
#ifdef NAILBOMB_SOLID
	grenade->r.ownerNum = ENTITYNUM_NONE;	// Nobody 'owns' this as far as physics are concerned
	grenade->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	grenade->r.contents = CONTENTS_PLAYERCLIP;	// Block player motion.
#else
	grenade->r.contents = CONTENTS_PLAYERCLIP;	// Block player motion.
#endif
	grenade->s.pos.trDuration = 250 * tr.fraction;		// May not be able to rise all the way.
	grenade->s.pos.trType = TR_LINEAR_STOP;
	grenade->s.pos.trTime = level.time;
	grenade->s.groundEntityNum = ENTITYNUM_NONE;
//	grenade->s.time = level.time + 500;		// Prevent nail trigger on client side.
	grenade->nextthink = level.time + 500;
	BG_EvaluateTrajectory( &grenade->s.apos, level.time, vec );
	grenade->s.apos.trBase[0] = AngleMod( vec[0] );
	grenade->s.apos.trBase[1] = AngleMod( vec[1] );
	grenade->s.apos.trBase[2] = AngleMod( vec[2] );
	grenade->s.apos.trDelta[0]	= -4 * grenade->s.apos.trBase[0];	// Spin to zero in quarter of a second
	grenade->s.apos.trDelta[1]	= -4 * grenade->s.apos.trBase[1];
	grenade->s.apos.trDelta[2]	= -4 * grenade->s.apos.trBase[2];
	grenade->s.apos.trType = TR_LINEAR_STOP;
	grenade->s.apos.trDuration = 250;
	grenade->s.apos.trTime = level.time;
	grenade->think = NailThink;
	grenade->health = (int) NailArrayCreate( grenade );	// Generate the array
	grenade->pain_debounce_time = level.time + Q3F_NAILGREN_PACKINTERVAL;
	PackNailBits( grenade, (char *)(((struct gnail *) grenade->health) + Q3F_NUM_NAILGRENNAILS) );
	trap_LinkEntity( grenade );
	return( qtrue );
}

qboolean NormalExplode( gentity_t *grenade )
{
	gentity_t *temp;

	temp = G_TempEntity( grenade->s.pos.trBase, EV_ETF_GRENADE_EXPLOSION );
	temp->s.weapon = (grenade->s.weapon == Q3F_GREN_CLUSTERSECTION) ? Q3F_GREN_CLUSTERSECTION : Q3F_GREN_NORMAL;
	if(grenade->s.powerups & (1 << PW_QUAD))
	{
		temp->s.powerups |= 1 << PW_QUAD;
		temp->s.otherEntityNum = grenade->activator->s.number;
	}
	temp->s.angles[1] = bg_q3f_grenade_flash.damage;
	temp->r.svFlags = SVF_BROADCAST;	// send to everyone

	return( qtrue );
}


g_q3f_grenade_t g_q3f_grenade_none = {
	&bg_q3f_grenade_none,
	0,
	0,
};

g_q3f_grenade_t g_q3f_grenade_normal = {
	&bg_q3f_grenade_normal,
	0,
	&NormalExplode//0,
};

g_q3f_grenade_t g_q3f_grenade_concuss = {
	&bg_q3f_grenade_concuss,
	0,
	&ConcussExplode,
};

g_q3f_grenade_t g_q3f_grenade_flash = {
	&bg_q3f_grenade_flash,
	0,
	&FlashExplode,
};

g_q3f_grenade_t g_q3f_grenade_flare = {
	&bg_q3f_grenade_flare,
	&FlareThrow,
	&FlareExplode,
};

g_q3f_grenade_t g_q3f_grenade_nail = {
	&bg_q3f_grenade_nail,
	0,
	&NailExplode,
};

g_q3f_grenade_t g_q3f_grenade_cluster = {
	&bg_q3f_grenade_clustersection,
	0,
	&ClusterExplode,
};

g_q3f_grenade_t g_q3f_grenade_clustersection = {
	&bg_q3f_grenade_clustersection,
	0,
	&NormalExplode, //djbob: missing explosion????
};

g_q3f_grenade_t g_q3f_grenade_napalm = {
	&bg_q3f_grenade_napalm,
	0,
	&NapalmExplode,
};

g_q3f_grenade_t g_q3f_grenade_gas = {
	&bg_q3f_grenade_gas,
	0,
	&HallucinogenicExplode,
};

g_q3f_grenade_t g_q3f_grenade_emp = {
	&bg_q3f_grenade_emp,
	0,
	&EmpExplode,
};

g_q3f_grenade_t g_q3f_grenade_charge = {
	&bg_q3f_grenade_charge,
	0,
	0,		// NOT a grenade, not to be thrown like one :)
};

g_q3f_grenade_t *g_q3f_grenades[Q3F_NUM_GRENADES] = {
	&g_q3f_grenade_none,
	&g_q3f_grenade_normal,
	&g_q3f_grenade_concuss,
	&g_q3f_grenade_flash,
	&g_q3f_grenade_flare,
	&g_q3f_grenade_nail,
	&g_q3f_grenade_cluster,
	&g_q3f_grenade_clustersection,
	&g_q3f_grenade_napalm,
	&g_q3f_grenade_gas,
	&g_q3f_grenade_emp,
	&g_q3f_grenade_charge,
};

g_q3f_grenade_t *G_Q3F_GetGrenade( int index )
{
	g_q3f_grenade_t *gren;

	if(	index < 1 ||
		index >= Q3F_NUM_GRENADES )
		return( &g_q3f_grenade_none );
	gren = g_q3f_grenades[index];
	return( gren ? gren : &g_q3f_grenade_none );
}

static void G_Q3F_GrenadeTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// Grenade touches something.

}

static void G_Q3F_GrenadeRemove( gentity_t *ent )
{
	// Remove this grenade

	G_FreeEntity( ent );

}
static void G_Q3F_GrenadeThink( gentity_t *ent )
{
	// Grenade hits timer limit

	g_q3f_grenade_t *gren;
	int damage, given, statnum;
	vec3_t origin;
	gclient_t *client;


	gren = G_Q3F_GetGrenade( ent->s.weapon );
	statnum = G_StatsModIndex( gren->g->mod );

	client = (ent->activator && ent->activator->client) ? ent->activator->client : NULL;
	client->pers.stats.data[statnum].shots++;

	if( gren->g->damage )
	{
		damage = gren->g->damage;
		if( ent->s.powerups & (1 << PW_QUAD) )
			G_AddEvent( ent, EV_POWERUP_QUAD, 0 );

		if (client) 
			given = client->pers.stats.data[statnum].given;

		G_RadiusDamage(	ent->r.currentOrigin, ent, ent->activator, damage, NULL, gren->g->mod, 0);
		
		if (client && given < client->pers.stats.data[statnum].given) 
			client->pers.stats.data[statnum].hits++;

		if( ent->count && ent->activator->health > 0 )
		{
			int r = (rand() % 3);
			// They forgot to throw the grenade :)

			switch(r) {
				case 0: trap_SendServerCommand(	-1, va( "print \"No, %s%s, you're supposed to THROW the grenade!\n\"",
										ent->activator->client->pers.netname, S_COLOR_WHITE ) );
					break;
				case 1: trap_SendServerCommand(	-1, va( "print \"%s%s, perhaps you should throw that grenade.\n\"",
						   ent->activator->client->pers.netname, S_COLOR_WHITE ) );
					break;
				default : trap_SendServerCommand(	-1, va( "print \"Oh snap, %s%s, you forgot to throw your grenade!\n\"",
							ent->activator->client->pers.netname, S_COLOR_WHITE ) );
					break;
			}
		}
	}

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	G_SetOrigin( ent, origin );

	ent->think			= G_Q3F_GrenadeRemove;
	ent->nextthink		= level.time + 2000;		// Allow client effects before vanishing

	if( gren->ExplodeGren )
		gren->ExplodeGren( ent );
}

qboolean G_Q3F_GrenadeCommand( gentity_t *ent )
{
	// We've recieved a grenade command, work out if it's valid and act on it.
	// This is invoked from ClientCommand, so we use trap_Argv to get the parameters.

	int grentype, primetime, throwtime, limittime;
	bg_q3f_playerclass_t *cls;
	g_q3f_grenade_t *gren;
	gentity_t *grenent;
	vec3_t velocity, up, side;
	char strbuff[16];
	trace_t tr;
	int contents;

	if( !ent->client )
		return( qfalse );			// Some kind of problem?

	if( level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )
		return( qfalse );			// Can't throw during ceasefires

	trap_Argv( 1, strbuff, 16 );
	grentype = atoi( strbuff );
	trap_Argv( 2, strbuff, 16 );
	primetime = atoi( strbuff );
	trap_Argv( 3, strbuff, 16 );
	throwtime = atoi( strbuff );

	limittime = level.time + FRAMETIME / 2;
	if( primetime >= throwtime || primetime > limittime || throwtime > limittime )
	{
		G_Printf(	"Invalid grenade time (%d / %d / %d) from %s.\n",
					primetime, throwtime, level.time,
					ent->client->pers.netname );
		return( qfalse );			// Absolutely not valid
	}
	if( (primetime < ent->client->respawnTime && throwtime >= ent->client->respawnTime) ||
		(throwtime < ent->client->respawnTime && ent->client->ps.stats[STAT_HEALTH] > 0) )
	{
		G_Printf( "Grenade from %s lost: Primed before last respawn.\n", ent->client->pers.netname ); 
		return(	qfalse );			// We _don't_ want cross-death throws
	}
	if( primetime < ent->client->lastgrenTime )
	{
		G_Printf( "Grenade from %s lost: Primed before previous grenade thrown.\n", ent->client->pers.netname ); 
		return( qfalse );			// They can't have primed before the last throw
	}
	if( primetime < (level.time - 8000) )
	{
		G_Printf( "Grenade from %s lost: Primed over 8 seconds ago.\n", ent->client->pers.netname ); 
		return( qfalse );			// Possibly valid, but it's a dud, right? :)
	}
	if( level.intermissiontime )
		return( qfalse );			// No grenades at intermission, I think :)

	cls = BG_Q3F_GetClass( &ent->client->ps );

	if( !((cls->gren1type == grentype && (ent->client->ps.ammo[AMMO_GRENADES] & 0x00FF)) ||
		(cls->gren2type == grentype && (ent->client->ps.ammo[AMMO_GRENADES] >> 8))) )
		return( qfalse );			// No grenades left of this time

	gren = G_Q3F_GetGrenade( grentype );
	if( gren->g->flags & Q3F_GFLAG_NOTHROW )
		return( qfalse );			// Can't fire this grenade

	ent->client->pers.stats.data[STATS_GREN + grentype].shots++;

	ent->client->ps.ammo[AMMO_GRENADES] =	ent->client->ps.ammo[AMMO_GRENADES] -
											((grentype == cls->gren1type) ? 1 : 0x100);

	ent->client->lastgrenTime = throwtime;

	// Looks like we're rolling, spawn the grenade (and any special commands for it),
	// then fire it off.

	grenent = G_Spawn();
	grenent->s.eType = ET_Q3F_GRENADE;
//	grenent->s.modelindex = 0;
	grenent->classname = "handgrenade";
	//VectorSet( grenent->r.mins, -4, -4, -4 );	// Fix this?
	//VectorSet( grenent->r.maxs, 4, 4, 4 );
	// RR2DO2: Ghetto wanted smaller grenades
	VectorSet( grenent->r.mins, -2.f, -2.f, -2.f );	// Fix this?
	VectorSet( grenent->r.maxs, 2.f, 2.f, 2.f );
	grenent->touch = G_Q3F_GrenadeTouch;

	grenent->s.time		= primetime + Q3F_GRENADE_PRIME_TIME;		// Time it explodes
	grenent->s.weapon	= grentype;				// The actual grenade type
	grenent->r.ownerNum	= ent->s.number;
	grenent->s.otherEntityNum = ent->s.number;

	grenent->activator	= ent;
	grenent->s.groundEntityNum = ENTITYNUM_NONE;

	VectorCopy( ent->client->ps.origin, grenent->r.currentOrigin );
	//if( grenent->s.time - g_grenadePJdelay.value > level.time )
	if( grenent->s.time - 60 > level.time && ent->client->ps.stats[STAT_HEALTH] > 0)
	{
		// Toss da bomb.

//		grenent->r.currentOrigin[2] += g_grenadeZorg.value;
		grenent->r.currentOrigin[2] += 16;
		G_SetOrigin( grenent, grenent->r.currentOrigin );
		grenent->s.pos.trType = TR_GRAVITY;
		grenent->s.pos.trTime = throwtime;			// Could be before we get the message
		grenent->count = 0;

		// set aiming directions
		AngleVectors( ent->client->ps.viewangles, velocity, side, up );
//		VectorScale( velocity, g_grenadeThrow.value, velocity );		// Forward
//		VectorScale( velocity, 700, velocity );		// Forward			// ETF 0.0
		VectorScale( velocity, 650, velocity );		// Forward
//		VectorMA( velocity, g_grenadeZvel.value, up, velocity );	// Up (by a third)
		VectorMA( velocity, 150, up, velocity );	// Up (by a third)
		VectorMA( velocity, Q_flrand(-1.0f, 1.0f) * 5, side, grenent->s.pos.trDelta );	// Left/right
		if( ent->health <= 0 )
			VectorScale( velocity, 0.1, velocity );	// Only throw a little if dead

		// Set angles
		grenent->s.apos.trBase[YAW]		= ent->client->ps.viewangles[YAW];
		grenent->s.apos.trBase[PITCH]	= 0;
		grenent->s.apos.trBase[ROLL]	= 0;
		grenent->s.apos.trDelta[YAW]	= Q_flrand(-1.0f, 1.0f) * 5;
		grenent->s.apos.trDelta[PITCH]	= 360 + Q_flrand(-1.0f, 1.0f) * 10;
		grenent->s.apos.trDelta[ROLL]	= 0;
		grenent->s.apos.trTime			= level.time;
		grenent->s.apos.trType			= TR_LINEAR;

		// Evaluate it's position, to avoid odd (i.e. through walls) initial position.
		// This method is wrong, but produces A: mostly reliable and B: fairly cheap
		// positioning. On the other hand, if you're facing a wall you can expect the
		// grenade to go off in your face.
		BG_EvaluateTrajectory( &grenent->s.pos, level.time, velocity );
//		trap_Trace(	&tr, grenent->s.pos.trBase, grenent->r.mins, grenent->r.maxs,
//					velocity, ent->s.number, MASK_PLAYERSOLID );
		G_Q3F_ForceFieldTrace( &tr, grenent->s.pos.trBase, grenent->r.mins, grenent->r.maxs, velocity, ent->s.number, MASK_PLAYERSOLID );
		while( !tr.startsolid && tr.fraction < 1 && ((level.time - grenent->s.pos.trTime) > 100))
		{
			grenent->s.pos.trTime += (level.time - grenent->s.pos.trTime) / 2;
			BG_EvaluateTrajectory( &grenent->s.pos, level.time, velocity );
				// Do a distance check to prevent a nasty infinite loop in the engine
			if( Distance( grenent->s.pos.trBase, velocity ) > 5 )
			{
//				trap_Trace(	&tr, grenent->s.pos.trBase, grenent->r.mins, grenent->r.maxs,
//							velocity, ent->s.number, MASK_PLAYERSOLID );
				G_Q3F_ForceFieldTrace(	&tr, grenent->s.pos.trBase, grenent->r.mins, grenent->r.maxs, velocity, ent->s.number, MASK_PLAYERSOLID );
			}
			else {
				// Abort the loop
				tr.startsolid = qtrue;
				break;
			}
		}
		VectorCopy( velocity, grenent->s.pos.trBase );	// This is where it would have been at the current time
		grenent->s.pos.trTime = level.time;				// And we start calculating from this point.

		if( ent->client->ps.powerups[PW_QUAD] ) {
			grenent->s.powerups |= 1 << PW_QUAD;
		}
	}
	else {
		// Sat on their grenade, as they say.
//		VectorMA( grenent->r.currentOrigin, -(g_grenadeDropDelay.value * 0.001), ent->client->ps.velocity, grenent->r.currentOrigin );
		VectorMA( grenent->r.currentOrigin, -(20 * 0.001), ent->client->ps.velocity, grenent->r.currentOrigin );
//		grenent->r.currentOrigin[2] += g_grenadeDropZ.value;
		G_SetOrigin( grenent, grenent->r.currentOrigin );
		//Let grenades fall to the ground when your dead
		grenent->s.pos.trType = ent->client->ps.stats[STAT_HEALTH] > 0 ? TR_STATIONARY : TR_GRAVITY;
		grenent->s.pos.trTime = level.time + 1;		// Ensure time is in the future
		grenent->count = ( ent->client->ps.stats[STAT_HEALTH] > 0 ? 1 : 0 );
		grenent->s.pos.trDelta[0] = 0;
		grenent->s.pos.trDelta[1] = 0;
		grenent->s.pos.trDelta[2] = 0;
		//grenent->s.pos.trDelta[2] = g_grenadeDropZVel.value * -0.1;//-1;

		if( ent->client->ps.powerups[PW_QUAD] ) {
			grenent->s.powerups |= 1 << PW_QUAD;
		}
	}

	grenent->s.eFlags |= EF_BOUNCE;

	grenent->think				= G_Q3F_GrenadeThink;
	grenent->nextthink			= grenent->s.time;		// Three seconds. Boom!
	grenent->activator			= ent;
	grenent->r.ownerNum			= ent - level.gentities;
	grenent->s.otherEntityNum	= ent - level.gentities;
	grenent->physicsObject = qtrue;

	contents = trap_PointContents( ent->s.pos.trBase, -1 );
	if ( contents & CONTENTS_WATER ) {
		ent->hasbeeninwater = qtrue;
	}
	

	if( gren->ThrowGren )
		gren->ThrowGren( grenent );				// Custom throw processing.

	contents = trap_PointContents( grenent->r.currentOrigin, -1 );

	if ( contents & CONTENTS_WATER ) {
		grenent->hasbeeninwater = qtrue;
	}

	trap_LinkEntity( grenent );

	// throw animation
	if( !(ent->client->ps.extFlags & EXTF_ANI_THROWING) ) {
		ent->client->ps.extFlags |= EXTF_ANI_THROWING;
		ent->client->torsoanimEndTime = level.time + Q3F_THROW_ANIM_DURATION;
	}

	return( qtrue );
}


/*
** Grenade 'run' function - Works largely like an item, except with the
** option of 'sticky grenades'.
*/

void G_Q3F_RunGrenade( gentity_t *ent )
{
	vec3_t origin, dir, angles;
	trace_t tr;
	//gentity_t *other;
	bg_q3f_grenade_t *gren;
	vec3_t	velocity;
	float	dot;
	int		hitTime;
	int		contents;

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
		G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, MASK_SHOT );

		VectorCopy( tr.endpos, ent->r.currentOrigin );

		/* This is going to be expensive */
		if(levelhasnoannoys)
		{
			if(ent->activator && ent->activator->client && ent->s.weapon != Q3F_GREN_CHARGE && ent->s.weapon != Q3F_GREN_CONCUSS)
			{
				vec3_t tracestart, traceend;
				int bitcheck = Q3F_NOANNOY_GRENS;

				VectorAdd( tr.endpos, ent->r.mins, tracestart );
				VectorAdd( tr.endpos, ent->r.maxs, traceend );

				switch(ent->s.weapon)
				{
				case Q3F_GREN_GAS:
					bitcheck = Q3F_NOANNOY_GASGREN;
					break;
				case Q3F_GREN_NAPALM:
					bitcheck = Q3F_NOANNOY_NAPALMGREN;
					break;
				case Q3F_GREN_NAIL:
					bitcheck = Q3F_NOANNOY_NAILGREN;
					break;
				default:
					bitcheck = Q3F_NOANNOY_GRENS;
				}

				if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->activator->client->sess.sessionTeam, bitcheck ) )
				{
					G_FreeEntity( ent );
					// Bye!
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
			if( ent->s.time > level.time && ent->s.pos.trType != TR_STATIONARY )
				G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );

			BG_EvaluateTrajectory( &ent->s.apos, level.time, dir );

			gren = BG_Q3F_GetGrenade( ent->s.weapon );
			if( (gren->flags & Q3F_GFLAG_STICKY) && tr.entityNum >= MAX_CLIENTS )
			{
				// Stop it moving at all.
				ent->s.pos.trType = TR_STATIONARY;
				VectorCopy( tr.endpos, ent->s.pos.trBase );
				VectorCopy( dir, ent->s.apos.trBase );
				ent->s.apos.trType = TR_STATIONARY;
				ent->s.groundEntityNum = tr.entityNum;
				BG_EvaluateTrajectory( &g_entities[tr.entityNum].s.pos, level.time, ent->pos1 ) ;
				VectorSubtract( ent->r.currentOrigin, ent->pos1, ent->pos1 );
			}
			else {
				//other = &g_entities[tr.entityNum];
					// reflect the velocity on the trace plane
				hitTime = level.previousTime + ( level.time - level.previousTime ) * tr.fraction;
				BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
				dot = DotProduct( velocity, tr.plane.normal );
				VectorMA( velocity, -2*dot, tr.plane.normal, ent->s.pos.trDelta );
				VectorScale( ent->s.pos.trDelta, 0.45f/*g_grenadeBounce.value*/, ent->s.pos.trDelta );
					// check for stop
				if( tr.plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 80/*g_grenadeStick.value*/ && tr.entityNum >= MAX_CLIENTS )
				{
					G_SetOrigin( ent, tr.endpos );
					VectorCopy( dir, ent->s.apos.trBase );
					ent->s.apos.trType = TR_STATIONARY;
					// JT - Make it sit flat, if it's supposed to.
					if(gren->flags & Q3F_GFLAG_LIESFLAT)
					{
						ent->s.apos.trBase[YAW]		= 0;
						ent->s.apos.trBase[PITCH]	= 0;
						ent->s.apos.trBase[ROLL]	= 0;
						ent->s.apos.trDelta[YAW]	= 0;
						ent->s.apos.trDelta[PITCH]	= 0;
						ent->s.apos.trDelta[ROLL]	= 0;
						VectorCopy(ent->s.apos.trDelta,ent->s.angles);
					}

					if( !(g_entities[tr.entityNum].s.eType == ET_PLAYER && g_entities[ent->s.groundEntityNum].health <= 0) )
					{
						ent->s.groundEntityNum = tr.entityNum;
						BG_EvaluateTrajectory( &g_entities[tr.entityNum].s.pos, level.time, ent->pos1 ) ;
						VectorSubtract( ent->r.currentOrigin, ent->pos1, ent->pos1 );
					}
				}
				else {
					VectorAdd( ent->r.currentOrigin, tr.plane.normal, ent->r.currentOrigin);
					VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
					ent->s.pos.trTime = level.time;
					if( !(gren->flags & Q3F_GFLAG_NOSPIN) )
					{
						VectorCopy( dir, ent->s.apos.trBase );
						ent->s.apos.trTime = level.time;
						vectoangles( tr.plane.normal, angles );
						ent->s.apos.trDelta[0] -= 2 * (ent->s.apos.trDelta[0] - angles[0]);
						ent->s.apos.trDelta[1] -= 2 * (ent->s.apos.trDelta[1] - angles[1]);
						ent->s.apos.trDelta[2] -= 2 * (ent->s.apos.trDelta[2] - angles[2]);
					}
					ent->s.groundEntityNum = ENTITYNUM_NONE;
				}
			}
		}
		else if( ent->s.pos.trType == TR_STATIONARY )
		{
			ent->s.pos.trType = TR_GRAVITY;
			VectorCopy( origin, ent->s.pos.trBase );
			ent->s.pos.trTime = level.time;
			ent->s.groundEntityNum = ENTITYNUM_NONE;

		}
	}
	else {
		VectorCopy( origin, ent->r.currentOrigin );

		/* This is going to be expensive */
		if(levelhasnoannoys)
		{
			if(ent->activator && ent->activator->client && ent->s.weapon != Q3F_GREN_CHARGE && ent->s.weapon != Q3F_GREN_CONCUSS)
			{
				vec3_t tracestart, traceend;
				int bitcheck = Q3F_NOANNOY_GRENS;

				VectorAdd( origin, ent->r.mins, tracestart );
				VectorAdd( origin, ent->r.maxs, traceend );

				switch(ent->s.weapon)
				{
				case Q3F_GREN_GAS:
					bitcheck = Q3F_NOANNOY_GASGREN;
					break;
				case Q3F_GREN_NAPALM:
					bitcheck = Q3F_NOANNOY_NAPALMGREN;
					break;
				case Q3F_GREN_NAIL:
					bitcheck = Q3F_NOANNOY_NAILGREN;
					break;
				default:
					bitcheck = Q3F_NOANNOY_GRENS;
				}

				if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->activator->client->sess.sessionTeam, bitcheck ) )
				{
					G_FreeEntity( ent );
					// Bye!
					return;
				}
			}
		}
	}

	trap_LinkEntity( ent );

	if( !ent->nextthink )
	{
		G_Printf( "Warning: Grenade with no nextthink removed.\n" );
		G_FreeEntity( ent );
		return;
	}

	// check think function after bouncing
	G_RunThink( ent );

	// ugh - but whatever
	if( ent->think && ent->think != G_Q3F_GrenadeRemove && !ent->soundLoop) {
		// RR2DO2: check for watersplash
		contents = trap_PointContents( ent->r.currentOrigin, -1 );
		if( !ent->hasbeeninwater ) {
			if ( contents & CONTENTS_WATER ) {
				G_AddEvent( ent, EV_ETF_WATERSPLASH, 0 );
				ent->hasbeeninwater = qtrue;
			}
		} else {
			if ( !(contents & CONTENTS_WATER) ) {
				ent->hasbeeninwater = qfalse;
			}
		}
	}
}
