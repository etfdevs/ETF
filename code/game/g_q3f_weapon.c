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
**	g_q3f_weapon.c
**
**	Weapon definitions and utility functions.
**
*/


#include "g_local.h"
#include "bg_public.h"
#include "g_weapon.h"
#include "g_q3f_weapon.h"
#include "g_q3f_mapents.h"
#include "bg_q3f_util.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_team.h"
#include "bg_local.h"
#include "q_shared.h"
#include "g_bot_interface.h"

extern bg_q3f_weapon_t bg_q3f_weapon_none;
extern bg_q3f_weapon_t bg_q3f_weapon_axe;
extern bg_q3f_weapon_t bg_q3f_weapon_shotgun;
extern bg_q3f_weapon_t bg_q3f_weapon_supershotgun;
extern bg_q3f_weapon_t bg_q3f_weapon_nailgun;
extern bg_q3f_weapon_t bg_q3f_weapon_supernailgun;
extern bg_q3f_weapon_t bg_q3f_weapon_grenade_launcher;
extern bg_q3f_weapon_t bg_q3f_weapon_rocket_launcher;
extern bg_q3f_weapon_t bg_q3f_weapon_sniperrifle;
extern bg_q3f_weapon_t bg_q3f_weapon_railgun;
extern bg_q3f_weapon_t bg_q3f_weapon_flamethrower;
extern bg_q3f_weapon_t bg_q3f_weapon_minigun;
extern bg_q3f_weapon_t bg_q3f_weapon_assaultrifle;
extern bg_q3f_weapon_t bg_q3f_weapon_dartgun;
extern bg_q3f_weapon_t bg_q3f_weapon_pipelauncher;
extern bg_q3f_weapon_t bg_q3f_weapon_napalm;

g_q3f_weapon_t q3f_weapon_none = {
	// No weapon

	&bg_q3f_weapon_none,
	&(Q3F_No_CanFire),		// Check firing
	&(Q3F_No_Fire),		// Fire
};

g_q3f_weapon_t q3f_weapon_axe = {
	// Axe
	&bg_q3f_weapon_none,
	&Q3F_Axe_CanFire,
	&Q3F_Weapon_Axe_Fire
};

g_q3f_weapon_t q3f_weapon_shotgun = {
	// The single-barelled shotgun.

	&bg_q3f_weapon_shotgun,

	&Q3F_Shotgun_CanFire,	// Check firing
	&weapon_shotgun_fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_supershotgun = {
	// The double-barrelled shotgun

	&bg_q3f_weapon_supershotgun,
	&Q3F_SuperShotgun_CanFire,			// Check firing
	&weapon_supershotgun_fire			// Fire
};

g_q3f_weapon_t q3f_weapon_grenade_launcher = {
	// The Grenade Launcher
	&bg_q3f_weapon_grenade_launcher,
	&Q3F_GrenadeLauncher_CanFire,		// Check firing
	&weapon_grenadelauncher_fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_rocket_launcher = {
	// The Rocket Launcher
	&bg_q3f_weapon_rocket_launcher,
	&Q3F_RocketLauncher_CanFire,		// Check firing
	&weapon_rocketlauncher_fire,		// Fire
};


g_q3f_weapon_t q3f_weapon_nailgun = {
	// The Nailgun
	&bg_q3f_weapon_nailgun,
	&Q3F_Nailgun_CanFire,		// Check firing
	&weapon_nailgun_fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_supernailgun = {
	// The Supernailgun
	&bg_q3f_weapon_supernailgun,
	&Q3F_SuperNailgun_CanFire,		// Check firing
	&weapon_supernailgun_fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_sniperrifle = {
	// The Sniperrifle
	&bg_q3f_weapon_sniperrifle,
	&Q3F_SniperRifle_CanFire,		// Check firing
	&Weapon_SniperRifle_Fire,		// Fire			// JT - Hack!
};

g_q3f_weapon_t q3f_weapon_railgun = {
	// The Railgun
	&bg_q3f_weapon_railgun,
	&Q3F_Railgun_CanFire,		// Check firing
	&Q3F_Railgun_Fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_flamethrower = {
	// The Flamethrower
	&bg_q3f_weapon_flamethrower,
	&Q3F_Flamethrower_CanFire,		// Check firing
	&Weapon_Flamethrower_Fire,		// Fire
};

g_q3f_weapon_t q3f_weapon_minigun = {
	// The Minigun
	&bg_q3f_weapon_minigun,
	&Q3F_Minigun_CanFire,		// Check firing
	&Weapon_Minigun_Fire		// Fire
};

g_q3f_weapon_t q3f_weapon_assaultrifle = {
	// The 'assault' mode rifle
	&bg_q3f_weapon_assaultrifle,
	&Q3F_AssaultRifle_CanFire,		// Check firing
	&Weapon_AssaultRifle_Fire		// Fire
};

g_q3f_weapon_t q3f_weapon_dartgun = {
	// The Agent 'dart gun'
	&bg_q3f_weapon_dartgun,
	&Q3F_DartGun_CanFire,		// Check firing
	&Q3F_Weapon_DartGun_Fire		// Fire
};

g_q3f_weapon_t q3f_weapon_pipelauncher = {
	// The Pipe launcher
	&bg_q3f_weapon_pipelauncher,
	&Q3F_PipeLauncher_CanFire,		// Check firing
	&Weapon_PipeLauncher_Fire		// Fire
};

g_q3f_weapon_t q3f_weapon_napalm = {
	// The Napalm cannon
	&bg_q3f_weapon_napalm,
	&Q3F_Napalm_CanFire,		// Check firing
	&Weapon_Napalm_Fire			// Fire
};

/*In the following order

	WP_NONE,

	WP_AXE,
	WP_SHOTGUN,
	WP_SUPERSHOTGUN,
	WP_NAILGUN,
	WP_SUPERNAILGUN,
	WP_GRENADE_LAUNCHER,
	WP_ROCKET_LAUNCHER,
	WP_SNIPER_RIFLE,
	WP_RAILGUN,
	WP_FLAMETHROWER,
	WP_MINIGUN,
	WP_ASSAULTRIFLE,
	WP_DARTGUN,
	WP_PIPELAUNCHER,
	WP_NAPALMCANNON,*/

	// An array of weapon structures

#define Q3F_WEAP_NONE 0

g_q3f_weapon_t *g_q3f_weapons[] = {
	&q3f_weapon_none,
	&q3f_weapon_axe,
	&q3f_weapon_shotgun,
	&q3f_weapon_supershotgun,
	&q3f_weapon_nailgun,
	&q3f_weapon_supernailgun,
	&q3f_weapon_grenade_launcher,
	&q3f_weapon_rocket_launcher,
	&q3f_weapon_sniperrifle,
	&q3f_weapon_railgun,
	&q3f_weapon_flamethrower,
	&q3f_weapon_minigun,
	&q3f_weapon_assaultrifle,
	&q3f_weapon_dartgun,
	&q3f_weapon_pipelauncher,
	&q3f_weapon_napalm,
};

void G_Q3F_DebugLine( const vec3_t start, const vec3_t end, const vec4_t color) {
	gentity_t *tent;

	tent = G_TempEntity( end, EV_DEBUG_DATA );
	/* Prevent a snapped origin */
	G_SetOrigin( tent, end );
	tent->s.constantLight =
		( (int)(color[0] * 255)  << 0  ) |
		( (int)(color[1] * 255)  << 8  ) |
		( (int)(color[2] * 255)  << 16 ) |
		( (int)(color[3] * 255)  << 24 );
	VectorCopy( start , tent->s.pos.trDelta );
}

void G_Q3F_DebugTrace( const vec3_t start, const trace_t * tr ) {
	gentity_t *traceEnt;
	float *color;

	traceEnt = &g_entities[ tr->entityNum ];

	if ( tr->surfaceFlags & SURF_NOIMPACT || tr->entityNum == ENTITYNUM_WORLD ) {
		color = colorLtGrey;
	} else if (tr->entityNum < MAX_CLIENTS ) {
		color = colorRed;
	} else if ( traceEnt->s.eType == ET_MOVER ) {
		color = colorGreen;
	} else color = colorMdCyan;
	G_Q3F_DebugLine( start, tr->endpos, color );
}


void G_Q3F_DebugBox( const vec3_t origin, const vec3_t mins, const vec3_t maxs, const vec4_t color ) {
	gentity_t *tent;

	tent = G_TempEntity( origin, EV_DEBUG_DATA );
	tent->s.eventParm = 1;
	VectorCopy( origin, tent->s.pos.trBase );
	VectorCopy( mins, tent->s.apos.trBase );
	VectorCopy( maxs, tent->s.apos.trDelta );

	tent->s.constantLight =
		( (int)(color[0] * 255)  << 0  ) |
		( (int)(color[1] * 255)  << 8  ) |
		( (int)(color[2] * 255)  << 16 ) |
		( (int)(color[3] * 255)  << 24 );
};


void G_Q3F_MuzzleTraceBox( gentity_t *ent, const vec3_t muzzle, const vec3_t forward ) {
	trace_t tr;
	gentity_t *traceEnt;
	vec3_t end;

	VectorMA( muzzle, 8192, forward, end );
	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

	traceEnt = &g_entities[ tr.entityNum ];
	if (traceEnt->client) {
		G_Q3F_DebugBox( traceEnt->r.currentOrigin, traceEnt->r.mins, traceEnt->r.maxs, colorCyan );
	}
}

// Client and server-side functions
g_q3f_weapon_t *G_Q3F_GetWeapon( int weaponnum )
{
	if( weaponnum < 0 || weaponnum >= WP_NUM_WEAPONS )
	{
		return( g_q3f_weapons[Q3F_WEAP_NONE] );
	}
	
	return( g_q3f_weapons[weaponnum] ? g_q3f_weapons[weaponnum] : g_q3f_weapons[Q3F_WEAP_NONE] );
}

qboolean Q3F_Axe_CanFire(struct gentity_s *ent)
{
	return qtrue;
}


qboolean Q3F_Shotgun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_SuperShotgun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}


qboolean Q3F_GrenadeLauncher_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_RocketLauncher_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_Nailgun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_SuperNailgun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_SniperRifle_CanFire(struct gentity_s *ent)
{
	return( qtrue );
}

qboolean Q3F_Railgun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_Flamethrower_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_Minigun_CanFire(struct gentity_s *ent)
{
	// JT: Don't allow you to fire if you're starting up the gun.
	// JT: Don't allow you to fire if you're moving.

	if( ent->client->ps.weaponstate == WEAPON_STARTING ||
		pm->cmd.forwardmove || pm->cmd.rightmove ||
		pm->ps->groundEntityNum == ENTITYNUM_NONE )
	{
		return( qfalse );
	}

	return( qtrue );
}


qboolean Q3F_AssaultRifle_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_DartGun_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_PipeLauncher_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_Napalm_CanFire(struct gentity_s *ent)
{
	return qtrue;
}

qboolean Q3F_No_CanFire(struct gentity_s *ent)
{
	return qfalse;
}

// Weapon Fire Functions (others contained in g_weapons.c)

void Q3F_No_Fire(struct gentity_s *ent)
{
	return;
}

void Q3F_Railgun_Fire(struct gentity_s *ent){
	gentity_t	*bolt;
	vec3_t		muzzle, forward;

	TraceMuzzlePoint( ent, muzzle, forward, tracebox_1_mins, tracebox_1_maxs );
	VectorNormalize ( forward );

	bolt = G_Spawn();
	bolt->classname = "rail";
	bolt->nextthink = level.time + 60000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_RAILGUN;
	bolt->r.ownerNum = ent->s.number;
	bolt->parent = ent;
	bolt->damage = 15;			// Golliwog: Reduced from 25.
	bolt->splashDamage = 0;
	bolt->methodOfDeath = MOD_RAILGUN;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;

	bolt->r.mins[0] = -1;
	bolt->r.mins[1] = -1;
	bolt->r.mins[2] = -1;

	bolt->r.maxs[0] = 1;
	bolt->r.maxs[1] = 1;
	bolt->r.maxs[2] = 1;

	bolt->s.pos.trTime = level.time;

	VectorCopy( muzzle, bolt->s.pos.trBase );
//	VectorScale( forward, 2000, bolt->s.pos.trDelta );
	VectorScale( forward, 2500, bolt->s.pos.trDelta ); // djbob: New for 2.2
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (muzzle, bolt->r.currentOrigin);

	ent->client->pers.stats.data[STATS_WP + WP_RAILGUN].shots++;
}

void Weapon_Minigun_Fire(struct gentity_s *ent)
{
	gentity_t	*tent;
	int			heat, cold, given;
	vec3_t		muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward );

	// send minigun blast
	tent = G_TempEntity( muzzle, EV_MINIGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	heat = (15.0 / 10000.0) * (float) (level.time - ent->client->minigunFireTime);
	if( heat > 15 )
		heat = 15;
	if( (level.time - ent->client->minigunLastFireTime) > 500 )
	{
		// They stopped firing a while back.

		cold = (15.0 / 5000.0) * (float) (level.time - ent->client->minigunLastFireTime);
		heat -= cold;
		if( heat < 0 )
			heat = 0;
		ent->client->minigunFireTime = level.time - heat * (5000.0 / 15.0);
	}
	ent->client->minigunLastFireTime = level.time;

	tent->s.eventParm = ent->client->attackTime % 256;
	tent->s.frame = heat;
	
	tent->s.otherEntityNum = ent->s.number;

	ent->client->pers.stats.data[STATS_WP + WP_MINIGUN].shots++;
	given = ent->client->pers.stats.data[STATS_WP + WP_MINIGUN].given;
	MinigunPattern( muzzle, tent->s.origin2, tent->s.eventParm, heat, ent );
	if (given < ent->client->pers.stats.data[STATS_WP + WP_MINIGUN].given ) {
		ent->client->pers.stats.data[STATS_WP + WP_MINIGUN].hits++;
	};
	return;
}

void Weapon_PipeLauncher_Fire(struct gentity_s *ent) {
	gentity_t	*m;
	vec3_t		muzzle, forward;

	TraceMuzzlePoint( ent, muzzle, forward, tracebox_2_mins, tracebox_2_maxs );
	// extra vertical velocity
	forward[2] += 0.2f;			// JT - less of that! :)

	m = fire_pipe (ent, muzzle, forward);
	m->s.powerups |= ent->s.powerups & (1 << PW_QUAD);

	G_Q3F_Pipe_Check_Quota( ent );			// JT: Check if that's going to bust the quotas.
	ent->client->pers.stats.data[STATS_WP + WP_PIPELAUNCHER].shots++;
}

void Weapon_Napalm_Fire(struct gentity_s *ent) {
	gentity_t	*bolt;
	vec3_t		muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward );

	VectorNormalize ( forward );

	bolt = G_Spawn();
	bolt->classname = "napalm";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_NAPALMCANNON;
	bolt->r.ownerNum = ent->s.number;
	//Unlagged client check
	bolt->s.otherEntityNum = ent->s.number;
	bolt->parent = ent;
	//bolt->damage = 10+Q_flrand(0.0f, 1.0f)*20; // RR2DO2: 1e
	//bolt->damage = 12+Q_flrand(0.0f, 1.0f)*20; // RR2DO2: 1f
	//bolt->damage = 25+Q_flrand(0.0f, 1.0f)*10; // RR2DO2: 1h
	bolt->damage = 40+Q_flrand(0.0f, 1.0f)*10; // djbob: 2.2
	//bolt->splashDamage = 20; // RR2DO2: 1e
//	bolt->splashDamage = 22; // RR2DO2: 1f
	bolt->splashDamage = 37; // djbob: 2.2
	bolt->methodOfDeath = MOD_FLAME;
	bolt->splashMethodOfDeath = MOD_FLAME;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( forward, 750, bolt->s.pos.trDelta );
//	VectorScale( forward, g_napalmRocketVel.integer, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy ( muzzle , bolt->r.currentOrigin);

	if(ent->s.powerups & (1 << PW_QUAD))
	{
		bolt->s.powerups = (1 << PW_QUAD);
	}

	ent->client->pers.stats.data[STATS_WP + WP_NAPALMCANNON].shots++;
}

void Weapon_AssaultRifle_Fire(struct gentity_s *ent) {
	int			damage, given;
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	vec3_t		muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, 4096, forward, end);

	G_DoTimeShiftFor( ent );
	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

	if( g_debugBullets.integer & 1 ) 
		G_Q3F_DebugTrace( muzzle, &tr );

	if( g_debugBullets.integer & 2 )
		G_Q3F_MuzzleTraceBox( ent, muzzle, forward );

	G_UndoTimeShiftFor( ent );

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	damage	= Q3F_SNIPER_ASSAULTRIFLE_DAMAGE * (tr.startsolid ? 1 : (1 - 0.5 * tr.fraction));

	traceEnt = &g_entities[ tr.entityNum ];

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact

	// JT - We hit.
	if ( traceEnt->takedamage && traceEnt->client ) {
		// RR2DO2: flametrooper has bullet resistance
		if( traceEnt->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_FLAMETROOPER )
			damage *= 0.85;

		tent = G_TempEntity( tr.endpos, EV_SNIPER_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
	} else {
		tent = G_TempEntity( tr.endpos, EV_SNIPER_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}
	// we need the client number to determine whether or not to
	// suppress this event
	tent->s.otherEntityNum = ent->s.number;

	if ( traceEnt->takedamage) {
		given = ent->client->pers.stats.data[STATS_WP + WP_ASSAULTRIFLE].given;
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, DAMAGE_Q3F_NAIL, MOD_RIFLE_ASSAULT );
		if (given < ent->client->pers.stats.data[STATS_WP + WP_ASSAULTRIFLE].given ) {
			ent->client->pers.stats.data[STATS_WP + WP_ASSAULTRIFLE].hits++;
		}
	}
	ent->client->pers.stats.data[STATS_WP + WP_ASSAULTRIFLE].shots++;
}

void Weapon_SniperRifle_Fire(struct gentity_s *ent) {
	int			duration, damage, given, i;
	int			mod = MOD_SNIPER_RIFLE;
	int			dflags = 0;
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	vec3_t		muzzle, forward;
	vec3_t		t_origin, t_mins, t_maxs;

	CalcMuzzlePoint( ent, muzzle, forward );

	duration = (ent->client->attackTime - ent->client->aimtime);
	damage = Q3F_SNIPER_MIN_DAMAGE + 
		(duration * (Q3F_SNIPER_MAX_DAMAGE - Q3F_SNIPER_MIN_DAMAGE)) / Q3F_SNIPER_MAX_DELAY;
	if( damage > Q3F_SNIPER_MAX_DAMAGE )
		damage = Q3F_SNIPER_MAX_DAMAGE;
	else if( damage < Q3F_SNIPER_MIN_DAMAGE )
		damage = Q3F_SNIPER_MIN_DAMAGE;

	//Always disable the sniper dot
	G_Q3F_SniperDot(ent, qfalse);
	if(pm->ps->groundEntityNum == ENTITYNUM_NONE)
	{
		//trap_SendServerCommand( ent->s.number, va("print \"You cannot fire without both feet on the ground!\n"));
		return;
	}
	
	ent->client->pers.stats.data[STATS_WP + WP_SNIPER_RIFLE].shots++;	
	//Com_Printf("Sniper Rifle Fired!: Duration = %d - Damage = %d\n",duration, damage);	// JT - Debug
	VectorMA (muzzle, 8192, forward, end);

	G_DoTimeShiftFor( ent );
	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

	if( g_debugBullets.integer & 1 ) 
		G_Q3F_DebugTrace( muzzle, (trace_t *)&tr );

	traceEnt = &g_entities[ tr.entityNum ];
	VectorCopy( traceEnt->r.currentOrigin, t_origin );
	VectorCopy( traceEnt->r.mins, t_mins);
	VectorCopy( traceEnt->r.maxs, t_maxs);

	G_UndoTimeShiftFor( ent );

	if ( tr.surfaceFlags & SURF_NOIMPACT )
		return;

	// send bullet impact

	// JT - We hit.
	if( !tr.startsolid && traceEnt->client ) {
		vec3_t	start;
		vec3_t	impact;
		vec3_t	head_container;
		qboolean head_hit;

		// RR2DO2: flametrooper has bullet resistance
		if( traceEnt->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_FLAMETROOPER )
			damage *= 0.815f;

		VectorSubtract( tr.endpos, t_origin , start );

		VectorSet( head_container, t_maxs[2]-8, t_maxs[2], 5 );
		head_hit = CylinderTraceImpact( start, forward, head_container, impact );

		/* Didn't hit head, find closest point to center on way through box */
		if (!head_hit ) {
			float maxlen = 9999;
			float bestlen;
			float centerlen;

			/* Calculate how long it takes to leave the hitbox */

			for (i = 0; i < 3; i++ ) {
				if ( forward[i] < 0 ) {
					float test = ( start[i] - t_mins[i] ) / -forward[i];
					if (test < maxlen ) 
						maxlen = test;
				} else if (forward[i] > 0 ) {
					float test = (t_maxs[i] - start[i] ) / forward[i];
					if (test < maxlen ) 
						maxlen = test;
				}
			}

			if (!forward[0]) {
				bestlen = (0 - start[1]) / forward[1];
			} else if (!forward[1]) {
				bestlen = (0 - start[0]) / forward[0];
			} else {
				/* Calculate smallest distance to center in 2d plane of x,y */
				bestlen = ( -forward[1]/forward[0] * start[0] + start[1] ) /
					(-forward[0]/forward[1] - forward[1]/forward[0] );
				bestlen = (bestlen - start[0]) / forward[0];
			}
			if (bestlen < 0 && bestlen > maxlen)
				bestlen = 0;

			VectorMA( start, bestlen, forward, impact );
			centerlen = sqrt(impact[0]*impact[0] + impact[1]*impact[1]);

			/* Determine where we hit someone */
			if ( impact[2] < 0.45*(t_mins[2]+t_maxs[2])) {
				damage *= 0.5f;
				mod = MOD_SNIPER_RIFLE_FEET;
				if ( centerlen > 10 ) {
					damage *= 1 - (centerlen - 10) / 20;
					trap_SendServerCommand( ent->s.number, va("print \"Leg Shot! But just a flesh wound!\n"));
				} else {
					trap_SendServerCommand( traceEnt->s.number, va("print \"You have been shot in the legs!\n"));
					trap_SendServerCommand( ent->s.number, va("print \"Leg Shot! That'll slow him down!\n"));
					if ( !G_Q3F_IsAllied( ent, traceEnt ) ||
					   ( ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_FULL ) ||
					   ( ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_HALF ) ) {
						if(traceEnt->client->legwounds < 6)			// JT - Add some legwounds to slow the player down.
							traceEnt->client->legwounds++;
					}
#if 0
					if ( G_Q3F_IsAllied( ent, traceEnt ) && (
					   ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_FULL 
					|| ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_HALF ) ) {
						if(traceEnt->client->legwounds < 7)			// JT - Add some legwounds to slow the player down.
							traceEnt->client->legwounds++;
					}
#endif
				}
			} else {
			// Use distance from center to figure out hit region
				if ( centerlen > 10 ) {
					trap_SendServerCommand( ent->s.number, va("print \"Glancing Body Shot!\n"));
					damage *= 0.5;
				} else
					trap_SendServerCommand( ent->s.number, va("print \"Body Shot!\n"));
				mod = MOD_SNIPER_RIFLE;
			}
		} else {
			/* Headshot */
			damage *= 2;
			mod = MOD_SNIPER_RIFLE_HEAD;
			trap_SendServerCommand( ent->s.number, va("print \"Head Shot!\n"));
		}

		if( g_debugBullets.integer & 2 ) {
			G_Q3F_DebugBox( t_origin, t_mins, t_maxs, colorCyan );
			VectorAdd( impact, t_origin, impact );
			if ( head_hit ) {
				G_Q3F_DebugLine( tr.endpos, impact, colorMdRed );
			} else {
				G_Q3F_DebugLine( tr.endpos, impact, colorMdYellow );
			}
		}

		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards( tr.endpos, muzzle );

		tent = G_TempEntity( tr.endpos, EV_SNIPER_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
	} else {
		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards( tr.endpos, muzzle );

		tent = G_TempEntity( tr.endpos, EV_SNIPER_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
		mod = MOD_SNIPER_RIFLE; // Don't know exactly what it is, but we're sure it's a hit with a sniper rifle
	}

	// we need the client number to determine whether or not to
	// suppress this event
	tent->s.otherEntityNum = ent->s.number;

	if ( traceEnt->takedamage) {
		given = ent->client->pers.stats.data[STATS_WP + WP_SNIPER_RIFLE].given;
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, dflags, mod);
		if (given < ent->client->pers.stats.data[STATS_WP + WP_SNIPER_RIFLE].given ) {
			ent->client->pers.stats.data[STATS_WP + WP_SNIPER_RIFLE].hits++;
		}
	}
	G_AddEvent( ent, EV_MUZZLEFLASH, 0 );
}

#define sd ent->client->sniperdot

void G_Q3F_SniperDot(struct gentity_s *ent, qboolean enable) {
	vec3_t endpoint;
	trace_t tr;
	//bg_q3f_playerclass_t	*cls;
	vec3_t	muzzle, forward;

	//cls = BG_Q3F_GetClass(&(ent->client->ps));

	if(!enable) {
		if(sd) {
			G_FreeEntity(sd);
			sd = NULL;
		}
		return;
	}

	if( !sd )
		sd = G_Spawn();			// Make a new entity

	sd->touch = 0;	// No touch function
	sd->activator = ent;
	sd->classname = "sniperdot";
	sd->physicsObject	= qfalse;
//	sd->s.eFlags = EF_Q3F_UNPREDICTABLE;				// Try to disable client-side prediction.
	sd->r.ownerNum = ent->s.number;
	sd->s.otherEntityNum = ent->s.number;

	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA( muzzle, 10000, forward, endpoint );
	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, endpoint, ent->s.number, MASK_SHOT );
	G_SetOrigin(sd, tr.endpos);
	vectoangles(tr.plane.normal,sd->s.angles);
	vectoangles(tr.plane.normal,sd->r.currentAngles);
	VectorCopy( tr.endpos, sd->r.currentOrigin );
	SnapVector( sd->r.currentOrigin );
	VectorCopy( sd->r.currentOrigin, sd->pos1 );
	VectorCopy( sd->r.currentOrigin, sd->pos2 );
	sd->s.torsoAnim = (tr.entityNum == ENTITYNUM_WORLD) ? qfalse : qtrue;
	sd->s.eType = ET_SNIPER_DOT;
	sd->timestamp = level.time;
	G_Q3F_RunSniperDot( sd );
}

void G_Q3F_RunSniperDot( gentity_t *ent ) {
	// Called every frame to update the sniper dot.
	// pos1 is current position, pos2 is old position,
	// The entity state is only updated if necessary.

	vec3_t oldorigin, muzzle, forward, endpoint;
	trace_t tr;

	if( !ent->activator || !ent->activator->client || !ent->activator->inuse )
		return;

	VectorCopy( ent->pos1, ent->pos2 );
	CalcMuzzlePoint( ent->activator, muzzle, forward );
	VectorMA( muzzle, 10000, forward, endpoint );
	G_Q3F_ForceFieldTrace( &tr, muzzle, NULL, NULL, endpoint, ent->activator->s.number, MASK_SHOT );
	VectorCopy( tr.endpos, ent->pos1 );

	if( tr.fraction == 1 || (tr.surfaceFlags & SURF_SKY) )
		ent->s.eFlags |= EF_NODRAW;
	else {
		ent->s.eFlags &= ~EF_NODRAW;
		ent->s.torsoAnim = (tr.entityNum == ENTITYNUM_WORLD) ? qfalse : qtrue;
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		VectorCopy( ent->pos1, ent->r.currentOrigin );
		VectorSubtract( ent->pos1, ent->pos2, oldorigin );
		VectorScale( oldorigin, ((float)(level.time - ent->s.pos.trTime)) / 1000, ent->s.pos.trDelta );
		vectoangles( tr.plane.normal, ent->s.angles );
		ent->s.pos.trTime = level.time - FRAMETIME;
	}

	trap_LinkEntity( ent );
}

void G_Q3F_Flame_Think(struct gentity_s *ent)
{
	vec3_t temp_vec;

	if( !ent->inuse ) {
		// Ensiform: Fixes bug causing 'think' to happen when already free'd
		return;
	}

	if(level.time>ent->timestamp)		// Am I too old and dead?
	{
		if( ent->target_ent->client )
			ent->target_ent->client->flames --;
		ent->nextthink = level.time;
		ent->think = G_FreeEntity;
		//G_FreeEntity(ent);
		return;
	}

	if( !ent->target_ent || !ent->target_ent->inuse ||
		ent->target_ent->health <= 0 ||
		(ent->client && ent->client->ps.powerups[PW_Q3F_INVULN]) )
	{
		ent->nextthink = level.time;
		ent->think = G_FreeEntity;
		//G_FreeEntity( ent );
		return;
	}

	if(ent->target_ent->waterlevel == 3)		// Is the person I'm burning underwater now?
	{
		ent->target_ent->client->flames--;			// TODO: Make a 'psssssh' sound ;)
		ent->nextthink = level.time;
		ent->think = G_FreeEntity;
		//G_FreeEntity(ent);
		return;
	}
	VectorCopy(ent->target_ent->s.pos.trBase, temp_vec);
	temp_vec[0] += (Q_flrand(0.0f, 1.0f) * 10) - 5;
	temp_vec[1] += (Q_flrand(0.0f, 1.0f) * 10) - 5;
	temp_vec[2] += (Q_flrand(0.0f, 1.0f) * 10) - 5;
	SnapVector( temp_vec );
	G_SetOrigin(ent,temp_vec);	// No longer needed.

	if(ent->count < level.time)
	{
		ent->count += Q3F_FLAMES_DAMAGE_EVERY;
		G_Damage(ent->target_ent,ent,ent->activator, NULL, NULL,
				ent->damage, DAMAGE_Q3F_FIRE, MOD_FLAME);
	}
	ent->nextthink = level.time+FRAMETIME;

	trap_LinkEntity(ent);
}

void G_Q3F_Burn_Person(struct gentity_s *target, struct gentity_s *attacker/*, int damage*/)
{
	gentity_t *flame;

	if( target->client )
	{
		if(target->client->ps.pm_type == PM_DEAD)
			return;						// Don't set light to dead people.

		if(target->client->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_FIRE)
			return;						// Don't set light to asbestos people

		if(G_Q3F_IsAllied( attacker, target ) && level.friendlyFire == FF_Q3F_NONE )		// Friends don't let friends catch fire *grin*
			return;

		if( target->client->ps.powerups[PW_Q3F_INVULN] )
			return;						// Can't burn invulnerable people

		if(target->client->flames > Q3F_MAX_FLAMES_PER_PERSON)	// If they're already burning merrily, don't make it worse.
			return;

		target->client->flames++;
		trap_SendServerCommand(target->s.clientNum,va("print \"You are on Fire!\n\""));
	}
	else {
		if( target->s.eType != ET_Q3F_SENTRY )
			return;
		if( target->sound2to1 )		// Flag used for 'already burning'
			return;
		if( target->waterlevel >= 2 )
			return;
	}

	flame = G_Spawn();			// Make a new entity
	flame->s.eType = ET_FLAME;
	flame->r.svFlags = SVF_NOCLIENT;
	flame->touch = 0;	// No touch function
	flame->target_ent = target;
	//flame->target = target->s.number;
	// flame->owner=attacker;
	flame->classname = "flame";
	flame->physicsObject	= qfalse;
	//flame->s.eFlags = EF_Q3F_UNPREDICTABLE;				// Try to disable client-side prediction.
	flame->r.ownerNum = attacker->s.number;
	flame->activator = attacker;
	flame->think = G_Q3F_Flame_Think;
	flame->nextthink = level.time+FRAMETIME;
	flame->count = level.time + Q3F_FLAMES_DAMAGE_EVERY;			// Burns every 800ms.
	flame->timestamp = level.time+Q3F_FLAMES_LIFESPAN;				// Lasts for 5 seconds.
	flame->damage = Q3F_DAMAGE_PER_BURN;
	VectorCopy( target->r.currentOrigin, flame->s.pos.trBase );	
	SnapVector( flame->s.pos.trBase );
	G_SetOrigin( flame, flame->s.pos.trBase );
	// Ensiform: This is a wrong origin.
	//G_SetOrigin(flame,target->s.origin);
	//SnapVector(target->s.origin);
	trap_LinkEntity(flame);									// JT: SNAP: Snap vector
}


void G_Q3F_Burn_Person_FromMap(struct gentity_s *target, struct gentity_s *attacker/*, int damage*/)
{
	gentity_t *flame;

	if ( !attacker || !attacker->inuse || attacker->s.eType != ET_Q3F_FLAMER || !attacker->mapdata || attacker->mapdata->state == Q3F_STATE_INVISIBLE || attacker->mapdata->state == Q3F_STATE_DISABLED )
		return;

	if( target->client )
	{
		if(target->client->ps.pm_type == PM_DEAD)
			return;						// Don't set light to dead people.

		if(target->client->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_FIRE)
			return;						// Don't set light to asbestos people

		if ( !G_Q3F_CheckCriteria( target, attacker ) )
			return;

		if( target->client->ps.powerups[PW_Q3F_INVULN] )
			return;						// Can't burn invulnerable people

		if(target->client->flames > Q3F_MAX_FLAMES_PER_PERSON)	// If they're already burning merrily, don't make it worse.
			return;

		target->client->flames++;
		trap_SendServerCommand(target->s.clientNum,va("print \"You are on Fire!\n\""));
	}
	else {
		if( target->s.eType != ET_Q3F_SENTRY )
			return;

		if( target->parent && target->parent->client && !G_Q3F_CheckCriteria( target->parent, attacker ) )
			return;

		if( target->sound2to1 )		// Flag used for 'already burning'
			return;
		if( target->waterlevel >= 2 )
			return;
	}

	flame = G_Spawn();			// Make a new entity
	flame->s.eType = ET_FLAME;
	flame->r.svFlags = SVF_NOCLIENT;
	flame->touch = 0;	// No touch function
	flame->target_ent = target;
	//flame->target = target->s.number;
	// flame->owner=attacker;
	flame->classname = "flame";
	flame->physicsObject	= qfalse;
	//flame->s.eFlags = EF_Q3F_UNPREDICTABLE;				// Try to disable client-side prediction.
	flame->r.ownerNum = attacker->s.number;
	flame->activator = attacker;
	flame->think = G_Q3F_Flame_Think;
	flame->nextthink = level.time+FRAMETIME;
	flame->count = level.time + Q3F_FLAMES_DAMAGE_EVERY;			// Burns every 800ms.
	flame->timestamp = level.time+Q3F_FLAMES_LIFESPAN;				// Lasts for 5 seconds.
	flame->damage = Q3F_DAMAGE_PER_BURN;
	VectorCopy( target->r.currentOrigin, flame->s.pos.trBase );	
	SnapVector( flame->s.pos.trBase );
	G_SetOrigin( flame, flame->s.pos.trBase );
	// Ensiform: This is a wrong origin.
	//G_SetOrigin(flame,target->s.origin);
	//SnapVector(target->s.origin);
	trap_LinkEntity(flame);									// JT: SNAP: Snap vector
}


/*
================
G_ExplodePipe

Explode a missile without an impact
================
*/
void G_ExplodePipe( gentity_t *ent ) {
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
		G_RadiusDamage( ent->r.currentOrigin, ent, ent->parent, ent->splashDamage, NULL, ent->splashMethodOfDeath, 0 );
	}
	ent->classname = "explosion";			// JT - Stop detpiping from exploding explosions.
	if(ent->parent)
	{
		ent->parent->client->pipecount --;

		// player pipe count
		ent->parent->client->ps.ammoclip[0] = ent->parent->client->pipecount;
	}
	trap_LinkEntity( ent );
}

void G_Q3F_DetPipe(struct gentity_s *self, int timeindex)
{
	struct gentity_s *ent;

	ent= NULL;

//	G_TimeShiftAllClients( timeindex, self );

	while ((ent = G_Find (ent, FOFS(classname), "pipe")) != NULL)
	{
		if(	ent->parent == self &&
			((self->health <= 0) || (ent->timestamp + G_Q3F_PIPE_DET_DELAY < timeindex)) )
			G_ExplodePipe(ent);
	}
//	G_UnTimeShiftAllClients( self );
}

void G_Q3F_CheckPipesForPlayer(struct gentity_s *self)
{
	struct gentity_s *ent, *oldest;
	int numpipes = 0;

	ent= oldest = NULL;

	while ((ent = G_Find (ent, FOFS(classname), "pipe")) != NULL)
	{
		if(ent->parent == self)
		{
			if(!oldest || ent->timestamp < oldest->timestamp)
				oldest = ent;

			numpipes++;
		}
	}

	if(numpipes > Q3F_MAX_PIPES_PER_GRENADIER)
		G_ExplodePipe(oldest);
}

void G_Q3F_Pipe_Check_Quota( gentity_t *player )
{
	// Check to see if we've exceeded our player / team quota, and intelligently (?)
	// pick an old one to destroy

	struct gentity_s *ent;
	int teampipes, team, playerpipes, playerteam;
	qboolean finished;
	struct gentity_s *team_oldest, *player_oldest;

	finished=qfalse;

	while(!finished)
	{
		player_oldest = team_oldest = NULL;
		playerpipes = teampipes = 0;
		playerteam = player->client->sess.sessionTeam;
		ent = NULL;

		while ((ent = G_Find (ent, FOFS(classname), "pipe")) != NULL)
		{
			if(!ent->parent)
				G_Error("Non-owned pipe found!");			// Safety.
			team = ent->parent->client->sess.sessionTeam;
			if( team == playerteam )
			{
				if( !team_oldest || ent->timestamp < team_oldest->timestamp )
					team_oldest = ent;
				teampipes++;
			}
			if( ent->parent == player )
			{
				if( !player_oldest || ent->timestamp < player_oldest->timestamp )
					player_oldest = ent;
				playerpipes++;
			}
		}
		finished = qtrue;

		if( playerpipes > Q3F_MAX_PIPES_PER_GRENADIER && player_oldest )
		{
			// Player has too many, just det one of his own
			G_ExplodePipe( player_oldest );
			finished = qfalse;
			continue;
		}
		if( teampipes > Q3F_MAX_PIPES_PER_TEAM && team_oldest )
		{
			if( playerpipes > Q3F_MIN_PIPES_PER_GRENADIER && player_oldest ) {
				G_ExplodePipe( player_oldest );
			} else {
				G_ExplodePipe( team_oldest );
			}
			finished = qfalse;
			continue;
		}
	}
}

void G_Q3F_Tranq_Person(struct gentity_s *target, struct gentity_s *attacker)
{
	if( !target->client || target->health <= 0 )
		return;

	if( !attacker->client || !G_Q3F_IsAllied( attacker, target ) )
	{
		if( !target->client->tranqTime )
			trap_SendServerCommand( target->s.number, "print \"You feel tired...\n\"" );
		target->client->tranqTime = level.time + Q3F_TRANQ_LIFESPAN;
		target->client->tranqEnt	= attacker;
	}
}

struct gentity_s *G_Q3F_Remove_Tranqs_And_Return_Owner(struct gentity_s *victim)
{
	if( !victim->client || !victim->client->tranqTime )
		return( NULL );
	victim->client->tranqTime = 0;
	return( victim->client->tranqEnt );
}

void Q3F_Weapon_DartGun_Fire(struct gentity_s *ent)
{
	gentity_t	*bolt;
	vec3_t		muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward );

	bolt = G_Spawn();
	bolt->classname = "tranq_dart";
	bolt->nextthink = level.time + 60000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_DARTGUN;
	bolt->r.ownerNum = ent->s.number;
	bolt->parent = ent;
	bolt->damage = 20;
	bolt->splashDamage = 0;
	bolt->methodOfDeath = MOD_DARTGUN;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR;

	// HACKHACKHACK?:fast projectiles appear way to far ahead cause of client prediction. Bad for nails
	//				 so, we need to pull it back a bit (or, in other words, let the client think that it
	//				 has been spawned some frames ago).
	// REMARK: if you set s.pos.tr.Time to a time before level.time (eg level.time - 50) it does the same
	//		   as if you set it to just level.time
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( forward, 1500, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (muzzle, bolt->r.currentOrigin);

	bolt->s.powerups = ent->s.powerups & (1 << PW_QUAD);

	ent->client->pers.stats.data[STATS_WP + WP_DARTGUN].shots++;
}


void G_Q3F_Weapon_Knife_Fire(struct gentity_s *ent)
{
	// JT - Mostly liberated from CheckGauntletAttack

	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	vec3_t		muzzle, forward;

	ent->client->pers.stats.data[STATS_WP + WP_AXE].shots++;
	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, G_Q3F_GAUNTLET_RANGE, forward, end);
	G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.otherEntityNum2 = ent->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}

	if ( !traceEnt->takedamage ) {
		if( tr.entityNum != ENTITYNUM_NONE && tr.fraction < 1 )
		{
			tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.otherEntityNum2 = ent->s.number;
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
		}
		return;
	}

	// JT - Work out if it's a backstab or not. TF2.5 source does this by working out the cross product between
	// the victim ent's _RIGHT_ unit-vectory, and the attackers _FORWARD_ unit-vector.
	// After much consultation of Rich's Maths notes, it sort of makes sense, in a brain-melting sort of way.
	// However... on closer inspection, brain-damage set in. So we'll just assume people are standing up (should be reasonable)
	// Actually. It's probably _MORE_ correct. At least... that's my argument, and I'm sticking to it.

/*
	AngleVectors(traceEnt->client->ps.viewangles, NULL, tright, NULL);
	AngleVectors(ent->client->ps.viewangles, tforward, NULL, NULL);
	CrossProduct(tright, tforward, xproduct);
*/

	if(traceEnt->client)
	{
		if(abs(AngleSubtract(ent->client->ps.viewangles[1],traceEnt->client->ps.viewangles[1])) > 90)
		{
			// They're facing towards me.
			damage = 40;
		}
		else
		{
			// They're facing away from me.
			damage = 120;
		}
	}
	else
		damage = 40;				// It's _NOT_ a player.
	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, DAMAGE_NO_GIB|DAMAGE_NO_ARMOR, MOD_KNIFE );

	return;


}

void G_Q3F_Heal_Armour(struct gentity_s *traceEnt, struct gentity_s *ent)
{
	int maxarmour;
	int armour;

	int healam = 40;

	if( !traceEnt->client || traceEnt->health <= 0 )
		return;						// Safety.

	if((armour = traceEnt->client->ps.stats[STAT_ARMOR])  == 0)
		return;						// Only repair armour if they've got some left.

	maxarmour = BG_Q3F_GetClass(&(traceEnt->client->ps))->maxarmour;

	if(maxarmour-armour <healam)
		healam = (int)((maxarmour-armour));

	if(ent->client->ps.ammo[AMMO_CELLS] < (healam/4))
		return;

	if (healam > 0)
	{
		armour += healam;
		if (armour > maxarmour)
			armour = maxarmour;
		#ifdef BUILD_BOTS
		Bot_Event_GaveSpannerArmor( ent, traceEnt, traceEnt->client->ps.stats[STAT_ARMOR], armour );
		Bot_Event_GotSpannerArmor( traceEnt, ent, traceEnt->client->ps.stats[STAT_ARMOR], armour );
		#endif
		ent->client->ps.ammo[AMMO_CELLS] -= (healam/4);			// Deduct ammo
		traceEnt->client->ps.stats[STAT_ARMOR] = armour;	// Add armour
		G_Sound(traceEnt, CHAN_AUTO, G_SoundIndex("sound/misc/ar2_pkup.wav"));
	}
}

void G_Q3F_Weapon_Wrench_Fire(struct gentity_s *ent)
{
	// JT - Mostly liberated from CheckGauntletAttack

	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage, given;
	vec3_t		muzzle, forward;

	ent->client->pers.stats.data[STATS_WP + WP_AXE].shots++;
	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, G_Q3F_GAUNTLET_RANGE, forward, end);
	G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if(G_Q3F_IsAllied(ent, traceEnt))
	{
		G_Q3F_Heal_Armour(traceEnt, ent);
	}
	else if( traceEnt->s.eType == ET_Q3F_SENTRY && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) )
	{
		G_Q3F_SentryUpgrade( ent, traceEnt->s.number );
		G_Q3F_SentryRepair( ent, traceEnt->s.number );
		G_Q3F_SentryRefill( ent, traceEnt->s.number );
	}
	else if( traceEnt->s.eType == ET_Q3F_SUPPLYSTATION && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) )
	{
		// Only upgrade if an upgrade occurs, so as to avoid giving ammo when you mean to upgrade
		// An upgrade instantly repairs to new max health
		int oldlevel = traceEnt->s.legsAnim;
		G_Q3F_SupplyStationUpgrade( ent, traceEnt->s.number );
		if( traceEnt->s.legsAnim != oldlevel )
			return;
		G_Q3F_SupplyStationRepair( ent, traceEnt->s.number );
		G_Q3F_SupplyStationRefill( ent, traceEnt->s.number );
	}
	else
	{
		// send blood impact
		if ( traceEnt->takedamage && traceEnt->client ) {
			tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.otherEntityNum2 = ent->s.number;
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
		}

		if ( !traceEnt->takedamage) {
			if( tr.entityNum != ENTITYNUM_NONE && tr.fraction < 1 )
			{
				tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
				tent->s.otherEntityNum = traceEnt->s.number;
				tent->s.otherEntityNum2 = ent->s.number;
				tent->s.eventParm = DirToByte( tr.plane.normal );
				tent->s.weapon = ent->s.weapon;
			}
			return;
		}
		given = ent->client->pers.stats.data[STATS_WP + WP_AXE].given;
		damage = 20;
		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_WRENCH );
		if (given < ent->client->pers.stats.data[STATS_WP + WP_AXE].given ) {
			ent->client->pers.stats.data[STATS_WP + WP_AXE].hits++;
		}
	}
}

void G_Q3F_Weapon_BioAxe_Fire(struct gentity_s *ent)
{
	// JT - Mostly liberated from CheckGauntletAttack

	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	vec3_t		muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, G_Q3F_GAUNTLET_RANGE * 1.5, forward, end);
	G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];


	if(G_Q3F_IsAllied(ent, traceEnt))
	{
		// RR2DO2: Add possibility for "operation failed"
		traceEnt->health -= 1;
		if ( traceEnt->client ) {
			traceEnt->client->ps.stats[STAT_HEALTH] = traceEnt->health;
		}		
		if ( traceEnt->health <= 0 ) {
			traceEnt->health = -1;
			traceEnt->enemy = ent;
			traceEnt->die (traceEnt, ent, ent, 1, MOD_FAILED_OPERATION);
		}
		G_Q3F_Heal_Person(traceEnt, ent);
	}
	else
	{
			// send blood impact
		if ( traceEnt->takedamage && traceEnt->client ) {
			tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.otherEntityNum2 = ent->s.number;
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
		}

		if ( !traceEnt->takedamage ) {
			if( tr.entityNum != ENTITYNUM_NONE && tr.fraction < 1 )
			{
				tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
				tent->s.otherEntityNum = traceEnt->s.number;
				tent->s.otherEntityNum2 = ent->s.number;
				tent->s.eventParm = DirToByte( tr.plane.normal );
				tent->s.weapon = ent->s.weapon;
			}
			return;
		}


		damage = 10;
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, DAMAGE_NO_ARMOR, MOD_NEEDLE_PRICK ); // Ensiform: just hitting is just a prick, not the disease itself
		G_Q3F_Disease_Person(traceEnt, ent, qfalse);
		return;

	}

}

void G_Q3F_Weapon_Axe_Fire(struct gentity_s *ent)
{
	// JT - Mostly liberated from CheckGauntletAttack

	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage, given;
	vec3_t		muzzle, forward;


	ent->client->pers.stats.data[STATS_WP + WP_AXE].shots++;
	CalcMuzzlePoint( ent, muzzle, forward );
	VectorMA (muzzle, G_Q3F_GAUNTLET_RANGE, forward, end);
	G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.otherEntityNum2 = ent->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}

	if ( !traceEnt->takedamage) {
		if( tr.entityNum != ENTITYNUM_NONE && tr.fraction < 1 )
		{
			tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
			tent->s.otherEntityNum = traceEnt->s.number;
			tent->s.otherEntityNum2 = ent->s.number;
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
		}
	} else {
		given = ent->client->pers.stats.data[STATS_WP + WP_AXE].given;
		damage = 20;
		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_AXE );
		if (given < ent->client->pers.stats.data[STATS_WP + WP_AXE].given ) {
			ent->client->pers.stats.data[STATS_WP + WP_AXE].hits++;
		}
	}
}

void G_Q3F_Heal_Person(struct gentity_s *target, struct gentity_s *attacker)
{
	/* JT - Do the following:
		Remove Hallucinations
		Remove Concussion
		Remove Tranquilisation
		Remove Blindness
		Remove Disease
		Extinguish Fire
		Health people up
		Increase Max Health (by 5)
		(Healing function sorts out leg-wounds, too)
		In addition, a frag is granted for removing effects, if the effect was caused by the enemy.
	*/

	gentity_t *original_attacker, *tent;
	int maxhealth;

	if( !target->client || target->health <= 0 )
		return;

//	tent = G_TempEntity( target->r.currentOrigin, EV_GENERAL_SOUND );
//	tent->s.eventParm = G_SoundIndex( "/sound/items/n_health.wav" );

	// Remove Halluciongens
	if(target->client->ps.powerups[PW_Q3F_GAS] )
	{
		// They're concussed
		target->client->ps.powerups[PW_Q3F_GAS] = 0;				// Remove the gas effect.
		// Should only give frags for gassing caused by baddies, but there's no way to know that currently.
		AddScore(attacker, attacker->r.currentOrigin, 1);											// So we'll do it regardless
		trap_SendServerCommand( target->s.number, va("print \"%s " S_COLOR_WHITE "has cured you hallucinations.\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have cured %s" S_COLOR_WHITE "'s hallucination.\n\"",
			target->client->pers.netname) );
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}


	// Remove Concussion
	if( target->client->ps.powerups[PW_Q3F_CONCUSS] )
	{
		// They're concussed
		target->client->ps.powerups[PW_Q3F_CONCUSS] = 0;				// Remove the concussion effect.
		// Should only give frags for consussion caused by baddies, but there's no way to know that currently.
		AddScore(attacker, attacker->r.currentOrigin, 1);											// So we'll do it regardless
		trap_SendServerCommand( target->s.number, va("print \"%s " S_COLOR_WHITE "has cured you of your concussion.\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have cured %s" S_COLOR_WHITE "'s concussion.\n\"",
			target->client->pers.netname) );
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}

	// Remove Tranquilisation
	if( target->client->tranqTime )
	{
		// They're tranquilised
		// Only give a frag if the tranq's caused by baddies.
		original_attacker = G_Q3F_Remove_Tranqs_And_Return_Owner(target);
		if(!G_Q3F_IsAllied(attacker, original_attacker))
			AddScore(attacker, attacker->r.currentOrigin, 1);

		trap_SendServerCommand( target->s.number, va("print \"%s" S_COLOR_WHITE " has cured you of your lethargy.\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have cured %s" S_COLOR_WHITE "'s lethargy.\n\"",
			target->client->pers.netname) );
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}

	// Remove Blindness
	if( target->client->ps.powerups[PW_Q3F_FLASH] )
	{
		// They're blinded
		target->client->ps.powerups[PW_Q3F_FLASH] = 0;				// Remove the blinded effect
		// Should only give frags for flash caused by baddies, but there's no way to know that currently.
		AddScore(attacker, attacker->r.currentOrigin, 1);											// So we'll do it regardless
		trap_SendServerCommand( target->s.number, va("print \"%s" S_COLOR_WHITE " has cured you of your blindness.\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have cured %s" S_COLOR_WHITE "'s blindness.\n\"",
			target->client->pers.netname) );
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}

	// Remove Diseases
	if( target->client->diseaseTime )
	{
		// They're diseased
		// Only give a frag if the disease is caused by baddies.
		/* Ensiform - (MAPDISEASE) And if they were a client to begin with */
		original_attacker = target->client->diseaseEnt;
		if( original_attacker && original_attacker->client && !G_Q3F_IsAllied(attacker, original_attacker) )
			AddScore(attacker, attacker->r.currentOrigin, 1);

		trap_SendServerCommand( target->s.number, va("print \"%s" S_COLOR_WHITE " has cured your infection!\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have cured %s" S_COLOR_WHITE "'s infection.\n\"",
			target->client->pers.netname) );
		// Curing people costs them half their remaining health, too.
		G_Damage(target, NULL, attacker, NULL, NULL, target->health/2, DAMAGE_NO_ARMOR, MOD_FAILED_OPERATION);
		target->client->diseaseTime = 0;								// Remove the disease.
		target->client->diseaseEnt = 0;
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}

	// Remove Flames
	if(target->client->flames)
	{
		// They're alight
		// Only give a frag if the flames's caused by baddies.
		original_attacker = G_Q3F_Remove_Flames_And_Return_Owner(target);
		if( original_attacker && !G_Q3F_IsAllied(attacker, original_attacker) )
			AddScore(attacker, attacker->r.currentOrigin, 1);

		trap_SendServerCommand( target->s.number, va("print \"%s" S_COLOR_WHITE " has doused the flames!\n\"",
			attacker->client->pers.netname) );
		trap_SendServerCommand( attacker->s.number, va("print \"You have extinguished %s" S_COLOR_WHITE ".\n\"",
			target->client->pers.netname) );
		#ifdef BUILD_BOTS
		Bot_Event_Cured( target, attacker );
		#endif
		return;
	}
	if(attacker->client->ps.ammo[AMMO_MEDIKIT] < 1)
		return;													// Are we out of ammo?

	// Now... think about repairing people.
	maxhealth = BG_Q3F_GetClass(&(target->client->ps))->maxhealth - 1; // -1 cause we allways do 1 damage !!

	if(target->client->ps.stats[STAT_HEALTH] < maxhealth)
	{
		int before = target->client->ps.stats[STAT_HEALTH];
		G_Q3F_Heal(target,10000, qfalse);
		#ifdef BUILD_BOTS
		Bot_Event_GaveMedicHealth( attacker, target, before, target->client->ps.stats[STAT_HEALTH] );
		Bot_Event_GotMedicHealth( target, attacker, before, target->client->ps.stats[STAT_HEALTH] );
		#endif
	}
	else
	{
		if(target->client->ps.stats[STAT_HEALTH] < maxhealth + G_Q3F_WEAP_MEDIKIT_OVERHEAL)
		{
			int before = target->client->ps.stats[STAT_HEALTH];
			G_Q3F_Heal(target, 5, qtrue);
			#ifdef BUILD_BOTS
			Bot_Event_GaveMedicHealth( attacker, target, before, target->client->ps.stats[STAT_HEALTH] );
			Bot_Event_GotMedicHealth( target, attacker, before, target->client->ps.stats[STAT_HEALTH] );
			#endif
		}
	}
	tent = G_TempEntity( attacker->r.currentOrigin, EV_HEAL_PERSON);
	tent->s.eventParm = attacker->client->ps.persistant[PERS_TEAM];
}

void G_Q3F_Heal(struct gentity_s *target, int amount, qboolean ignore)
{
	int desthealth;
	int maxhealth;
	
	if(!target->client)
		G_Error("PANIC: G_Q3F_Heal called on non-client object.");		// Safety.

	maxhealth = BG_Q3F_GetClass(&(target->client->ps))->maxhealth;
	desthealth = target->client->ps.stats[STAT_HEALTH];
	desthealth += amount;
	if(!ignore && desthealth > maxhealth)
		desthealth = maxhealth;

	// JT: From TF2.5 Source. Seems a little odd to me.

	if (target->client->legwounds)
	{
		if (desthealth > 95)
			target->client->legwounds  = 0;
		else
			target->client->legwounds = target->client->legwounds - (desthealth/20);

		if (target->client->legwounds < 1)
			target->client->legwounds = 0;
	}

	target->health = desthealth;
	target->client->ps.stats[STAT_HEALTH] = desthealth;
}

qboolean G_Q3F_Disease_Person(struct gentity_s *target, struct gentity_s *attacker, qboolean spreadtoallies)
{
	if( !target->client )
		return qfalse;						// Safety

	if( target->health <= 0 )
		return qfalse;						// Don't disease dead people

	if( target->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC ) 
		return qfalse;						// Don't disease paramedics

	if( target->client->ps.powerups[PW_Q3F_INVULN] ) 
		return( qfalse );					// Don't disease while invulnerable

	if( target->client->diseaseTime )
		return qfalse;						// Don't disease people more than once.

	if( G_Q3F_IsAllied(attacker, target) && !spreadtoallies )
		return qfalse;						// Don't disease the owner's team unless you're supposed to.

	target->client->diseaseTime = level.time + Q3F_DISEASE_DAMAGE_EVERY;
	target->client->diseaseEnt = attacker;
	#ifdef BUILD_BOTS
	Bot_Event_Infected( target, attacker );
	#endif
	return qtrue;
}

/* Ensiform - (MAPDISEASE) This is same as above, except it can be spread to medics */
qboolean G_Q3F_Disease2_Person(struct gentity_s *target, struct gentity_s *attacker, qboolean spreadtoallies)
{
	if( !target->client )
		return qfalse;						// Safety

	if( target->health <= 0 )
		return qfalse;						// Don't disease dead people

	if( target->client->ps.powerups[PW_Q3F_INVULN] ) 
		return( qfalse );					// Don't disease while invulnerable

	if( target->client->diseaseTime )
		return qfalse;						// Don't disease people more than once.

	target->client->diseaseTime = level.time + Q3F_DISEASE_DAMAGE_EVERY;
	target->client->diseaseEnt = attacker;
	#ifdef BUILD_BOTS
	Bot_Event_Infected(target, attacker); // might crash with world/other ents
	#endif
	return qtrue;
}


void G_Q3F_Paramedic_Regen_Think(struct gentity_s *ent)
{
	bg_q3f_playerclass_t *cls;

	if( level.time < ent->client->paramedicregenTime )
		return;

	if( ent->health <= 0 )
		return;

	if(ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_PARAMEDIC)
		return;						// Don't heal non-paramedics

	ent->client->paramedicregenTime = level.time + G_Q3F_PARAMEDIC_HEAL_INTERVAL;

	cls = BG_Q3F_GetClass(&(ent->client->ps));
	if( ent->client->ps.stats[STAT_HEALTH] >= cls->maxhealth )
		return;						// Don't regen people on full health.
	if(ent->client->ps.ammo[AMMO_MEDIKIT] > G_Q3F_PARAMEDIC_HEAL_AMOUNT)
	{
		G_Q3F_Heal(ent, G_Q3F_PARAMEDIC_HEAL_AMOUNT, qfalse);
		ent->client->ps.ammo[AMMO_MEDIKIT] -= G_Q3F_PARAMEDIC_HEAL_AMOUNT;
	}
}

void G_Q3F_Check_Maladies(struct gentity_s *ent)
{
	G_Q3F_Disease_Think(ent);
	// JT: FIXME: Add Tranqs to this.
	G_Q3F_Paramedic_Regen_Think(ent);
}

void G_Q3F_Disease_Think(struct gentity_s *ent)
{
	int i,e, numListedEntities, dist;
	int entityList[MAX_GENTITIES];
	vec3_t v, mins, maxs;
	gentity_t *tent;
	int radius;

	if(ent->client->ps.stats[STAT_HEALTH] <=0)	// Don't do it if he's already dead.
	{
		ent->client->diseaseTime = 0;
	}

	if( !ent->client->diseaseTime || ent->client->ps.powerups[PW_Q3F_INVULN] )
	{
		ent->client->ps.eFlags &= ~(EF_Q3F_DISEASED);			// Reset the disease flag...
//		trap_LinkEntity(ent);
		return;											// JT - Do nothing if we're not infected.
	}
	ent->client->ps.eFlags |= EF_Q3F_DISEASED;					// Set the diseased flag.
//	trap_LinkEntity(ent);
	
	// First off... do some damage, if it's time.
	if( ent->client->diseaseTime <= level.time)
	{
		ent->client->diseaseTime = level.time + Q3F_DISEASE_DAMAGE_EVERY;
		G_Damage(ent, NULL,ent->client->diseaseEnt, NULL, NULL,
				Q3F_DISEASE_DAMAGE, DAMAGE_NO_ARMOR, MOD_DISEASE);
		G_AddEvent(ent, EV_DISEASE, 0);
	}

	// Now... see if I can find another victim to infect.

	// JT - Robbed from G_RadiusDamage

	radius = 16;							// JT: Number of units between items to be deemed 'close enough'.

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = ent->client->ps.origin[i] - radius;
		maxs[i] = ent->client->ps.origin[i] + radius;
	}



	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ )
	{
		tent = &g_entities[entityList[ e ]];

		if(!tent->client)
			continue;
		if (tent == ent)
			continue;
		//if( !G_Q3F_IsAllied( ent, tent ) ) // RR2DO2: disabled for qwtf-alike things
		//	continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( ent->client->ps.origin[i] < tent->r.absmin[i] ) {
				v[i] = tent->r.absmin[i] - ent->client->ps.origin[i];
			} else if ( ent->client->ps.origin[i] > ent->r.absmax[i] ) {
				v[i] = ent->client->ps.origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		if( rand() & 15 )		// Reduce likelyhood of infection
			continue;

		// Ok. That's a person, we believe.
		// Give the credit for this disease to the original attacker.
		// We don't bother doing this if we were diseased by a map ent
		if(ent->client->diseaseEnt->client)
		{
			if(G_Q3F_Disease_Person(tent, ent->client->diseaseEnt, ( ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_FULL ||
																	 ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_HALF /*||
																	 ( level.friendlyFire & FF_Q3F_MASK ) & FF_Q3F_ARMOUR */)))
			{
				trap_SendServerCommand( ent->s.number, va("print \"You have infected %s^7!\n\"",
					tent->client->pers.netname) );
				trap_SendServerCommand( tent->s.number, va("print \"%s^7 has infected You!\n\"",
					ent->client->pers.netname) );
				if(ent->activator && ent->activator->client)
				{
					trap_SendServerCommand( ent->activator->s.number, va("print \"%s^7 has infected %s^7!\n\"",
						ent->client->pers.netname, tent->client->pers.netname) ); 
				}
			}
		}
	}
}


struct gentity_s *G_Q3F_Remove_Flames_And_Return_Owner(struct gentity_s *victim)
{
	gentity_t *ent = NULL;
	gentity_t *owner = NULL;

	while ((ent = G_Find (ent, FOFS(classname), "flame")) != NULL)
	{
		if(ent->target_ent == victim)
		{
			owner = ent->activator;
			ent->nextthink = level.time;
			ent->think = G_FreeEntity;
			//ent->nextthink = 0;
			//ent->think = NULL;
			//G_FreeEntity(ent);
		}
	}
	victim->client->flames = 0;
	return owner;
}


void Q3F_Weapon_Axe_Fire(struct gentity_s *ent)
{
	switch(ent->client->ps.persistant[PERS_CURRCLASS])
	{
		case Q3F_CLASS_AGENT:
			G_Q3F_Weapon_Knife_Fire(ent);
			break;
		case Q3F_CLASS_PARAMEDIC:
			G_Q3F_Weapon_BioAxe_Fire(ent);
			break;
		case Q3F_CLASS_ENGINEER:
			G_Q3F_Weapon_Wrench_Fire(ent);
			break;
		default:
			G_Q3F_Weapon_Axe_Fire(ent);
			break;
	}

}

