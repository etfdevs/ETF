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
**	bg_q3f_weapon.h
**
**	General defines for Server-Side Q3F weapon functions
**
*/


#ifndef __G_Q3F_WEAPON_H
#define __G_Q3F_WEAPON_H


#include "bg_q3f_weapon.h"
#include "g_local.h"

#define Q3F_SNIPER_MAX_DAMAGE			300
#define Q3F_SNIPER_MAX_DELAY			1750
#define	Q3F_SNIPER_MIN_DAMAGE			50
#define	Q3F_SNIPER_ASSAULTRIFLE_DAMAGE	8

#define Q3F_FLAMES_DAMAGE_EVERY			800
#define Q3F_FLAMES_LIFESPAN				5000
//#define Q3F_MAX_FLAMES_PER_PERSON		3
#define Q3F_DAMAGE_PER_BURN				2

#define Q3F_MIN_PIPES_PER_GRENADIER		3	// If less than this, cannibalize someone else's pipes
#define Q3F_MAX_PIPES_PER_GRENADIER		6	// Grenadier can't lay more pipes than this
#define Q3F_MAX_PIPES_PER_TEAM			8	// Entire team can't lay more pipes than this

#define Q3F_TRANQ_LIFESPAN				12000

#define Q3F_DISEASE_DAMAGE_EVERY		2000
#define Q3F_DISEASE_DAMAGE				5

#define	G_Q3F_GAUNTLET_RANGE			48

#define	G_Q3F_PIPE_DET_DELAY			400 // Was: 200, and became 215

#define G_Q3F_WEAP_MEDIKIT_OVERHEAL		50

// JT -- From g_weapon.c (previously)
void MinigunPattern( vec3_t origin, vec3_t origin2, int seed, int spread, gentity_t *ent );
extern vec3_t tracebox_1_mins;
extern vec3_t tracebox_1_maxs;
extern vec3_t tracebox_2_mins;
extern vec3_t tracebox_2_maxs;

// JT -- From g_missile.c (originally)
#define	MISSILE_PRESTEP_TIME	50

typedef struct g_q3f_weapon_s {

	bg_q3f_weapon_t *s;	
	// Check we _can_ fire this.
	qboolean (*CanFire)( struct gentity_s *ent );

	// Fire it.
	void (*Fire)( struct gentity_s *ent );
//	qboolean (*FireStop)( struct gentity_s *ent ); JT: Can't see what this is for
} g_q3f_weapon_t;


extern g_q3f_weapon_t *G_Q3F_GetWeapon( int weaponnum );

// Checking functions 

qboolean Q3F_Shotgun_CanFire(struct gentity_s *ent);
qboolean Q3F_SuperShotgun_CanFire(struct gentity_s *ent);
qboolean Q3F_GrenadeLauncher_CanFire(struct gentity_s *ent);
qboolean Q3F_RocketLauncher_CanFire(struct gentity_s *ent);
qboolean Q3F_Nailgun_CanFire(struct gentity_s *ent);
qboolean Q3F_SuperNailgun_CanFire(struct gentity_s *ent);
qboolean Q3F_SniperRifle_CanFire(struct gentity_s *ent);
qboolean Q3F_Railgun_CanFire(struct gentity_s *ent);
qboolean Q3F_Flamethrower_CanFire(struct gentity_s *ent);
qboolean Q3F_Minigun_CanFire(struct gentity_s *ent);
qboolean Q3F_AssaultRifle_CanFire(struct gentity_s *ent);
qboolean Q3F_DartGun_CanFire(struct gentity_s *ent);
qboolean Q3F_PipeLauncher_CanFire(struct gentity_s *ent);
qboolean Q3F_Napalm_CanFire(struct gentity_s *ent);
qboolean Q3F_Axe_CanFire(struct gentity_s *ent);
qboolean Q3F_No_CanFire(struct gentity_s *ent);

// Weapon Firing Functions

void Weapon_Minigun_Fire(struct gentity_s *ent);
void Weapon_AssaultRifle_Fire(struct gentity_s *ent);
void Q3F_Weapon_DartGun_Fire(struct gentity_s *ent);
void Weapon_PipeLauncher_Fire(struct gentity_s *ent);
void Weapon_Napalm_Fire(struct gentity_s *ent);
void Weapon_SniperRifle_Fire(struct gentity_s *ent);
void Q3F_Railgun_Fire(struct gentity_s *ent);
void Q3F_Weapon_Axe_Fire(struct gentity_s *ent);
void Q3F_No_Fire(struct gentity_s *ent);


// Weapon Support Functions

void G_Q3F_SniperDot(struct gentity_s *ent, qboolean enable);
void G_Q3F_RunSniperDot( gentity_t *ent );
//void G_Q3F_SniperDotThink(struct gentity_s *ent);
//void G_Q3F_Burn_Person(struct gentity_s *target, struct gentity_s *attacker/*, int damage*/);
void G_Q3F_Tranq_Person(struct gentity_s *target, struct gentity_s *attacker);
void G_Q3F_Tranq_Remove(struct gentity_s *target);
void G_Q3F_DetPipe(struct gentity_s *self, int timeindex);
void G_Q3F_Pipe_Check_Quota( gentity_t *player );
void G_ExplodePipe(struct gentity_s *ent);
void G_Q3F_CheckPipesForPlayer(struct gentity_s *self);
qboolean G_Q3F_Disease_Person(struct gentity_s *target, struct gentity_s *attacker, qboolean spreadtoallies);
qboolean G_Q3F_Disease2_Person(struct gentity_s *target, struct gentity_s *attacker, qboolean spreadtoallies);
void G_Q3F_Disease_Think(struct gentity_s *ent);
void G_Q3F_Heal_Person(struct gentity_s *target, struct gentity_s *attacker);
struct gentity_s *G_Q3F_Remove_Tranqs_And_Return_Owner(struct gentity_s *victim);
struct gentity_s *G_Q3F_Remove_Flames_And_Return_Owner(struct gentity_s *victim);
void G_Q3F_Check_Maladies(struct gentity_s *ent);
void G_Q3F_Heal(struct gentity_s *target, int amount, qboolean ignore);

#endif // __G_Q3F_WEAPON_H
