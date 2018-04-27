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

//
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"
#include "g_q3f_weapon.h"
#include "bg_q3f_playerclass.h"
#include "g_q3f_mapents.h"
#include "g_bot_interface.h"

//Some useful variables
vec3_t tracebox_1_mins = { -1, -1, -1 };
vec3_t tracebox_1_maxs = {  1,  1,  1 };
vec3_t tracebox_2_mins = { -2, -2, -2 };
vec3_t tracebox_2_maxs = {  2,  2,  2 };

/*
================
G_BounceProjectile
================
*/

void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}

/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE 5

void ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent, int damage, int mod ) {
	trace_t		tr;
	vec3_t		forward;
	gentity_t	*traceEnt;

	G_Q3F_ForceFieldTrace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

	if( g_debugBullets.integer & 1 ) {
		G_Q3F_DebugTrace( start, &tr );
	}

	traceEnt = &g_entities[ tr.entityNum ];		
    
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}
	if ( traceEnt->takedamage) {
		VectorSubtract( end, start, forward );
		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_Q3F_SHELL , mod);
	}
	return;
}

// this should match CG_ShotgunPattern
void ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	G_DoTimeShiftFor( ent );
	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		ShotgunPellet( origin, end, ent, DEFAULT_SHOTGUN_DAMAGE, MOD_SHOTGUN );
	}

	if( g_debugBullets.integer & 2 )
		G_Q3F_MuzzleTraceBox( ent, origin, forward );

	G_UndoTimeShiftFor( ent );
}

void MinigunPattern( vec3_t origin, vec3_t origin2, int seed, int spread, gentity_t *ent ) {
	int			i;
	float		r, u, factor;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	factor = DEFAULT_MINIGUN_PELLET_SPREAD + (EXTRA_MINIGUN_PELLET_SPREAD * spread) / 15;

	G_DoTimeShiftFor( ent );
	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_MINIGUN_PELLET_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * factor * 16;
		u = Q_crandom( &seed ) * factor * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		ShotgunPellet( origin, end, ent, 5, MOD_MINIGUN );
	}

	if( g_debugBullets.integer & 2 )
		G_Q3F_MuzzleTraceBox( ent, origin, forward );

	G_UndoTimeShiftFor( ent );
}

// this should match CG_ShotgunPattern
void SingleShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	G_DoTimeShiftFor( ent );
	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SINGLE_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SINGLE_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SINGLE_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		
		ShotgunPellet( origin, end, ent, DEFAULT_SHOTGUN_DAMAGE, MOD_SINGLESHOTGUN );
	}

	if( g_debugBullets.integer & 2 )
		G_Q3F_MuzzleTraceBox( ent, origin, forward );

	G_UndoTimeShiftFor( ent );
}

void weapon_supershotgun_fire (gentity_t *ent) {
	gentity_t		*tent;
	int given;
	vec3_t			muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward);
	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.otherEntityNum = ent->s.number;
	tent->s.eventParm = ent->client->attackTime % 256;

	ent->client->pers.stats.data[STATS_WP + WP_SUPERSHOTGUN].shots++;
	given = ent->client->pers.stats.data[STATS_WP + WP_SUPERSHOTGUN].given;
	ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
	if (given < ent->client->pers.stats.data[STATS_WP + WP_SUPERSHOTGUN].given )
		ent->client->pers.stats.data[STATS_WP + WP_SUPERSHOTGUN].hits++;
}

void weapon_shotgun_fire (gentity_t *ent) {
	gentity_t		*tent;
	int given;
	vec3_t			muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward);
	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SINGLESHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.otherEntityNum = ent->s.number;
	tent->s.eventParm = ent->client->attackTime % 256;

	ent->client->pers.stats.data[STATS_WP + WP_SHOTGUN].shots++;
	given = ent->client->pers.stats.data[STATS_WP + WP_SHOTGUN].given;
	SingleShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
	if (given < ent->client->pers.stats.data[STATS_WP + WP_SHOTGUN].given )
		ent->client->pers.stats.data[STATS_WP + WP_SHOTGUN].hits++;
}

void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;
	vec3_t			muzzle, forward;

	TraceMuzzlePoint( ent, muzzle, forward, tracebox_2_mins, tracebox_2_maxs );
	// extra vertical velocity
	forward[2] += 0.2f;			// JT - less of that! :)

	m = fire_grenade (ent, muzzle, forward);
	m->s.powerups = ent->s.powerups & (1 << PW_QUAD);

	ent->client->pers.stats.data[STATS_WP + WP_GRENADE_LAUNCHER].shots++;
}

void weapon_rocketlauncher_fire (gentity_t *ent) {
	gentity_t	*m;
	vec3_t			muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward);
	m = fire_rocket (ent, muzzle, forward);
	m->s.powerups = ent->s.powerups & (1 << PW_QUAD);

	ent->client->pers.stats.data[STATS_WP + WP_ROCKET_LAUNCHER].shots++;
}

void weapon_nailgun_fire (gentity_t *ent) {
	gentity_t	*m;
	vec3_t			muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward);
	m = fire_nail (ent, muzzle, forward, 9, MOD_NAILGUN );		// JT: Used to be 9
	m->s.powerups = ent->s.powerups & (1 << PW_QUAD);

	ent->client->pers.stats.data[STATS_WP + WP_NAILGUN].shots++;
}


void weapon_supernailgun_fire (gentity_t *ent) {
	gentity_t	*m;
	vec3_t			muzzle, forward;

	CalcMuzzlePoint( ent, muzzle, forward);
	//m = fire_nail (ent, muzzle, forward, 13 );		// JT: Used to be 13
	//m = fire_nail (ent, muzzle, forward, 6 );		// RR2DO2: russellm says 6 (qwtf1.5)
	//m = fire_nail (ent, muzzle, forward, 14 );		// RR2DO2: beta1d
	//m = fire_nail (ent, muzzle, forward, 13 );		// RR2DO2: beta1f
	m = fire_nail (ent, muzzle, forward, 14, MOD_SUPERNAILGUN );		// Golliwog: beta2
	m->s.powerups = ent->s.powerups & (1 << PW_QUAD);

	ent->client->pers.stats.data[STATS_WP + WP_SUPERNAILGUN].shots++;
}

void Weapon_Flamethrower_Fire (gentity_t *ent) {
	gentity_t	*m;
	vec3_t		muzzle, forward;

	TraceMuzzlePoint( ent, muzzle, forward, tracebox_1_mins, tracebox_1_maxs );

	ent->client->pers.stats.data[STATS_WP + WP_FLAMETHROWER].shots++;
	m = fire_flame (ent, muzzle, forward);
	m->s.powerups = ent->s.powerups & (1 << PW_QUAD);
}

static void G_Q3F_ConcussionEffect( const gentity_t* ent, vec3_t out )
{
	vec3_t angleoffset;

	VectorCopy( ent->client->ps.viewangles, out );

	if( ent->client->ps.powerups[PW_Q3F_CONCUSS] < level.time )
		return;		// Not concussed
	if( ent->health <= 0 || ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR || ent->client->sess.sessionClass == Q3F_CLASS_NULL )
		return;		// Don't concuss dead players/spectators/players without a class

	BG_Q3F_ConcussionEffect( ent->client->ps.generic1, ent->client->ps.powerups[PW_Q3F_CONCUSS] - level.time, angleoffset ); 

	// And update the viewangles
	out[0] += angleoffset[0];
	out[1] += angleoffset[1];
}

/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( const gentity_t *ent, vec3_t muzzle, vec3_t forward ) {
	vec3_t angles;

	G_Q3F_ConcussionEffect( ent, angles );
	AngleVectors( angles, forward, NULL, NULL );

	VectorCopy( ent->s.pos.trBase, muzzle );
	muzzle[2] += ent->client->ps.viewheight;

	VectorMA( muzzle, 14, forward, muzzle);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzle );
}

void TraceMuzzlePoint ( const  gentity_t *ent, vec3_t muzzle, vec3_t forward, const vec3_t mins, const vec3_t maxs ) {
	vec3_t angles, end;
	trace_t	tr;

	G_Q3F_ConcussionEffect( ent, angles );
	AngleVectors( angles, forward, NULL, NULL );

	VectorCopy( ent->s.pos.trBase, muzzle );
	muzzle[2] += ent->client->ps.viewheight;

	VectorMA( muzzle, 14, forward, end );
	G_Q3F_ForceFieldTrace( &tr, muzzle, mins, maxs, end, ent->s.clientNum, MASK_SHOT );

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVectorTowards( tr.endpos, muzzle );
	VectorCopy( tr.endpos, muzzle );
}


/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
	g_q3f_weapon_t *wp;


	if( level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )		// Golliwog: No fighting during ceasefire
		return;

	wp = G_Q3F_GetWeapon(ent->s.weapon);

	// JT - Should be dealt with differently.

	if(wp->CanFire(ent))
	{
		G_Q3F_StopAgentDisguise( ent );		// This breaks our disguise
		G_Q3F_StopAgentInvisible( ent );	// This breaks our invisibility, too :)
		if ( ent->client->ps.powerups[PW_QUAD] ) 
			G_AddEvent( ent, EV_POWERUP_QUAD, 0 );
		wp->Fire(ent);
	}

	// Omni-bot - Send a fire event.
	#ifdef BUILD_BOTS
	Bot_Event_FireWeapon(ent, Bot_WeaponGameToBot(ent->s.weapon, ent->client->ps.persistant[PERS_CURRCLASS]), NULL); //fixme use pFiredShot instead of NULL
	#endif
}
