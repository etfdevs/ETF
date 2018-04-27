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
**	General defines for Q3F weapon functions
**
*/

#ifndef __BG_Q3F_WEAPON_H
#define __BG_Q3F_WEAPON_H

#include "q_shared.h"
// #include "g_local.h"	 - Not safe on bg_modules.


#define Q3F_MINIGUN_WARMUP_TIME			700

//#define BG_Q3F_FLAME_BLUE_LENGTH		32 // for use with flamethrowerflame
#define BG_Q3F_FLAME_BLUE_LENGTH		48 // for use with spawnflames
#define	BG_Q3F_FLAME_VELOCITY			900//600
#define	BG_Q3F_FLAME_LIFETIME			400//600
#define	BG_Q3F_FLAME_LENGTH				300
#define	BG_Q3F_FLAME_SIZE				24
#define	BG_Q3F_FLAME_SEGMENTS			8
#define BG_Q3F_FLAME_START_OFFSET		24

#define Q3F_NUM_WEAPONANIMS		3	// RR2DO2: increase this when we add more weapon anims (Sniper rifle 'aiming' ?)

typedef struct bg_q3f_weapon_s {
	// Defines the weapon properties. Probably little use for the
	// likes of the lightning gun, but we don't use it anyway :)

	// Reload times and clip sizes
	int firetime;			// Milliseconds between shots
	int reloadtime;			// Milliseconds to reload an entire clip
	int clipsize;			// Number of items in a clip (zero means no clip)
	int	ammotype;			// Type of amunition
	int	numammo;			// Number of ammo required to shoot.
	qboolean fire_on_release;				// Do we fire like a normal weapon, or like a Sniper Rifle?
	qboolean inform_on_start;				// Do we need an event generated when you start trying to fire?
	int animNumber[Q3F_NUM_WEAPONANIMS];	// Weapon animation numbers for players [IDLE][ATTACK][RELOAD]
} bg_q3f_weapon_t;

/* JT - Not needed - WP_* enum will do just fine.
typedef enum {
	Q3F_WEAP_NONE,					// No weapon

	Q3F_WEAP_AXE,					// Assorted weapons available
	Q3F_WEAP_WRENCH,
	Q3F_WEAP_BIORIFLE,
	Q3F_WEAP_KNIFE,
	Q3F_WEAP_SHOTGUN,
	Q3F_WEAP_SUPERSHOTGUN,
	Q3F_WEAP_RAILGUN,
	Q3F_WEAP_RIFLE,
	Q3F_WEAP_ASSAULTRIFLE,
	Q3F_WEAP_NAILGUN,
	Q3F_WEAP_SUPERNAILGUN,
	Q3F_WEAP_GRENADELAUNCHER,
	Q3F_WEAP_PIPEBOMBLAUNCHER,
	Q3F_WEAP_ROCKETLAUNCHER,

	Q3F_WEAP_MAX					// End-of-list marker
};
*/ 
/*typedef*/ enum {
	Q3F_WP_NONE,
	Q3F_WP_BIOAXE,
	Q3F_WP_KNIFE,
	Q3F_WP_WRENCH
};
// An array of weapon structures
extern bg_q3f_weapon_t *bg_q3f_weapons[];

	// Client and server functions
bg_q3f_weapon_t *BG_Q3F_GetWeapon( int weaponnum );	// Get a pointer to a class structure
int BG_Q3F_GetRemappedAnimFromWeaponNumAndAnim( int weapNum, int classNum, int otherWeapNum, int otherClassNum, int animNumber );
int Q3F_GetAmmoTypeForWeapon(int weaponnum);
int Q3F_GetClipSizeForWeapon(int weaponnum);
void BG_Q3F_Request_Reload(playerState_t *ps);


#endif //__BG_Q3F_WEAPON_H
