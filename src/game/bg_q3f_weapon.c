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
**	bg_q3f_weapon.c
**
**	Weapon definitions and utility functions.
**
*/

#include "bg_q3f_weapon.h"
#include "bg_q3f_playerclass.h"
#include "bg_public.h"
#include "bg_q3f_util.h"

bg_q3f_weapon_t bg_q3f_weapon_none = {
	// No weapon
	
	400,			// 0.4 seconds between shots
	0,			// 3.2 seconds for a full clup
	0,			// ... assuming 8 shells in a clip.
	AMMO_NAILS,// Ammo type
	0,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ 0, 0, 0 }

};

bg_q3f_weapon_t bg_q3f_weapon_axe = {
	// Axe
	
	500,			// 0.5 seconds between shots
	0,			// 3.2 seconds for a full clup
	0,			// ... assuming 8 shells in a clip.
	AMMO_NONE,// Ammo type
	0,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_AXE_IDLE, ANI_WEAPON_AXE_ATTACK, ANI_WEAPON_AXE_IDLE }

};


bg_q3f_weapon_t bg_q3f_weapon_shotgun = {
	// The single-barelled shotgun.
	
	500,			
	2000,			// 2 seconds for a full clup
	8,			// ... assuming 8 shells in a clip.
	AMMO_SHELLS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_SHOTGUN_IDLE, ANI_WEAPON_SHOTGUN_ATTACK, ANI_WEAPON_SHOTGUN_RELOAD }
};

bg_q3f_weapon_t bg_q3f_weapon_supershotgun = {
	// The double-barelled shotgun.
	
	700,			
	3000,			// 3.0 seconds for a full clup
	16,			// ... assuming 16 shells in a clip.
	AMMO_SHELLS,
	2,			// # of ammo to fire.
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_SSHOTGUN_IDLE, ANI_WEAPON_SSHOTGUN_ATTACK, ANI_WEAPON_SSHOTGUN_RELOAD }
};


bg_q3f_weapon_t bg_q3f_weapon_grenade_launcher = {
	// The Grenade Launcher
	600,			// Time between shots.
	4000,			// 4.0 seconds for a full clip
	6,			// ... assuming 6 grens in a clip.
	AMMO_ROCKETS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_GLAUNCHER_IDLE, ANI_WEAPON_GLAUNCHER_ATTACK, ANI_WEAPON_GLAUNCHER_RELOAD }
};

bg_q3f_weapon_t bg_q3f_weapon_rocket_launcher = {
	// The Rocket Launcher
	800,			// Time between shots.
	4000,			// 5.0 seconds for a full clip  // RR2DO2: too slow for the public, make it 4
	4,			// ... assuming 4 rockets in a clip.
	AMMO_ROCKETS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_RLAUNCHER_IDLE, ANI_WEAPON_RLAUNCHER_ATTACK, ANI_WEAPON_RLAUNCHER_RELOAD }
};


bg_q3f_weapon_t bg_q3f_weapon_nailgun = {
	// The Nailgun
	//100,			// Time between shots.
	105,			// time between shots
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_NAILS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_NAILGUN_IDLE, ANI_WEAPON_NAILGUN_ATTACK, ANI_WEAPON_NAILGUN_IDLE }
};


bg_q3f_weapon_t bg_q3f_weapon_supernailgun = {
	// The Supernailgun
	//100,			// Time between shots.		// JT: Should be 200
	//47,			// Time between shots // according to russellm (tf2.5)
	94,			// Time between shots // beta1d
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_NAILS,
	2,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_SNAILGUN_IDLE, ANI_WEAPON_SNAILGUN_ATTACK, ANI_WEAPON_SNAILGUN_IDLE }
};


bg_q3f_weapon_t bg_q3f_weapon_sniperrifle = {
	// The Sniper Rifle
	1500,			// Time between shots.
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_SHELLS,
	1,
	qtrue,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_SRIFLE_IDLE, ANI_WEAPON_SRIFLE_ATTACK, ANI_WEAPON_SRIFLE_IDLE }
};

bg_q3f_weapon_t bg_q3f_weapon_railgun = {
	// The Railgun
	500,			// Time between shots.
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_NAILS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_RAILGUN_IDLE, ANI_WEAPON_RAILGUN_ATTACK, ANI_WEAPON_RAILGUN_IDLE }

};

bg_q3f_weapon_t bg_q3f_weapon_flamethrower = {
	// The Flamethrower
	100,//150,			// Time between shots.
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_CELLS,
	1,
	qfalse,		// Fire on release?
	qtrue,		// Inform on start?
	{ ANI_WEAPON_FTHROWER_IDLE, ANI_WEAPON_FTHROWER_ATTACK, ANI_WEAPON_FTHROWER_IDLE }
};

bg_q3f_weapon_t bg_q3f_weapon_minigun = {
	// The Minigun
//	100,		// Time between shots.
	105,		// 2.01c test TEMP djbob
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_SHELLS,
	1,			// Is that right - is now.
	qfalse,		// Fire on release?
	qtrue,		// Inform on start?
	{ ANI_WEAPON_MINIGUN_IDLE, ANI_WEAPON_MINIGUN_ATTACK, ANI_WEAPON_MINIGUN_IDLE }
};

bg_q3f_weapon_t bg_q3f_weapon_assaultrifle = {
	// The 'assault' mode rifle
	100,			// Time between shots.
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_SHELLS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_ARIFLE_IDLE, ANI_WEAPON_ARIFLE_ATTACK, ANI_WEAPON_ARIFLE_IDLE }
};

bg_q3f_weapon_t bg_q3f_weapon_dartgun = {
	// The Dart gun
	1500,			// Time between shots.
	0,			// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_NAILS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_DARTGUN_IDLE, ANI_WEAPON_DARTGUN_ATTACK, ANI_WEAPON_DARTGUN_IDLE }
};

bg_q3f_weapon_t bg_q3f_weapon_pipelauncher = {
	// The Pipe Launcher
	600,			// Time between shots.
	4000,		// 2.0 seconds for a full clip
	6,			// ... assuming 6 grens in a clip.
	AMMO_ROCKETS,
	1,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_PLAUNCHER_IDLE, ANI_WEAPON_PLAUNCHER_ATTACK, ANI_WEAPON_PLAUNCHER_RELOAD }
};

bg_q3f_weapon_t bg_q3f_weapon_napalm = {
	// The Napalm Cannon
	1200,			// Time between shots.
	0,		// 2.0 seconds for a full clip
	0,			// ... assuming 6 grens in a clip.
	AMMO_ROCKETS,
	3,
	qfalse,		// Fire on release?
	qfalse,		// Inform on start?
	{ ANI_WEAPON_NAPCANNON_IDLE, ANI_WEAPON_NAPCANNON_ATTACK, ANI_WEAPON_NAPCANNON_IDLE }
};



/* In the following order

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
	WP_NAPALMCANNON,
	
*/

	// An array of weapon structures

#define Q3F_WEAP_NONE 0

bg_q3f_weapon_t *bg_q3f_weapons[] = {
	&bg_q3f_weapon_none,
	&bg_q3f_weapon_axe,
	&bg_q3f_weapon_shotgun,
	&bg_q3f_weapon_supershotgun,
	&bg_q3f_weapon_nailgun,
	&bg_q3f_weapon_supernailgun,
	&bg_q3f_weapon_grenade_launcher,
	&bg_q3f_weapon_rocket_launcher,
	&bg_q3f_weapon_sniperrifle,
	&bg_q3f_weapon_railgun,
	&bg_q3f_weapon_flamethrower,
	&bg_q3f_weapon_minigun,
	&bg_q3f_weapon_assaultrifle,
	&bg_q3f_weapon_dartgun,
	&bg_q3f_weapon_pipelauncher,
	&bg_q3f_weapon_napalm,
};


	// Client and server-side functions
bg_q3f_weapon_t *BG_Q3F_GetWeapon( int weaponnum )
{
	
	if( weaponnum < 0 || weaponnum >= WP_NUM_WEAPONS )
	{
		return( bg_q3f_weapons[Q3F_WEAP_NONE] );
	}
	
	return( bg_q3f_weapons[weaponnum] ? bg_q3f_weapons[weaponnum] : bg_q3f_weapons[Q3F_WEAP_NONE] );
}

int Q3F_GetAmmoTypeForWeapon(int weaponnum)
{
	return(BG_Q3F_GetWeapon(weaponnum)->ammotype);
}

int Q3F_GetClipSizeForWeapon(int weaponnum)
{
	return(BG_Q3F_GetWeapon(weaponnum)->clipsize);
}

static int bg_q3f_weaponEx_animMap[4][Q3F_NUM_WEAPONANIMS] = {
	{ ANI_WEAPON_AXE_IDLE, ANI_WEAPON_AXE_ATTACK, ANI_WEAPON_AXE_IDLE },
	{ ANI_WEAPON_SYRINGE_IDLE, ANI_WEAPON_SYRINGE_ATTACK, ANI_WEAPON_SYRINGE_IDLE },
	{ ANI_WEAPON_KNIFE_IDLE, ANI_WEAPON_KNIFE_ATTACK, ANI_WEAPON_KNIFE_IDLE },
	{ ANI_WEAPON_WRENCH_IDLE, ANI_WEAPON_WRENCH_ATTACK, ANI_WEAPON_WRENCH_IDLE }
};

static int BG_Q3F_GetRemappedAnimFromWeaponNumExAndAnim( int weapNumEx, int otherWeapNumEx, int animNumber ) {
	int i;

	if( weapNumEx == otherWeapNumEx )
		return animNumber;

	for( i = 0; i < Q3F_NUM_WEAPONANIMS; i++ ) {
		if( bg_q3f_weaponEx_animMap[weapNumEx][i] == animNumber ) {
			return( bg_q3f_weaponEx_animMap[otherWeapNumEx][i] );
		}
	}

	// failsafe, we should never get here
	return animNumber;
}

int BG_Q3F_GetRemappedAnimFromWeaponNumAndAnim( int weapNum, int classNum, int otherWeapNum, int otherClassNum, int animNumber ) {
	int i;

	// Axe is special
	if( weapNum == WP_AXE ) {
		int weapNumEx, otherWeapNumEx;
		switch( classNum ) {
		case Q3F_CLASS_PARAMEDIC:	weapNumEx = Q3F_WP_BIOAXE;	break;
		case Q3F_CLASS_AGENT:		weapNumEx = Q3F_WP_KNIFE;	break;
		case Q3F_CLASS_ENGINEER:	weapNumEx = Q3F_WP_WRENCH;	break;
		default:					weapNumEx = 0;				break;	// not none, but normal axe
		}
		switch( otherClassNum ) {
		case Q3F_CLASS_PARAMEDIC:	otherWeapNumEx = Q3F_WP_BIOAXE;	break;
		case Q3F_CLASS_AGENT:		otherWeapNumEx = Q3F_WP_KNIFE;	break;
		case Q3F_CLASS_ENGINEER:	otherWeapNumEx = Q3F_WP_WRENCH;	break;
		default:					otherWeapNumEx = 0;				break;	// not none, but normal axe
		}
		return BG_Q3F_GetRemappedAnimFromWeaponNumExAndAnim( weapNumEx, otherWeapNumEx, animNumber );
	} else {
		if( weapNum == otherWeapNum )
			return animNumber;

		for( i = 0; i < Q3F_NUM_WEAPONANIMS; i++ ) {
			if( bg_q3f_weapons[weapNum]->animNumber[i] == animNumber ) {
				return( bg_q3f_weapons[otherWeapNum]->animNumber[i] );
			}
		}
	}

	// failsafe, we should never get here
	return animNumber;
}

void BG_Q3F_Request_Reload(playerState_t *ps)
{
	bg_q3f_weapon_t *wp;
	int clipsize;
	wp = BG_Q3F_GetWeapon(ps->weapon);

	// if we are reloading already, return
	if( ps->weaponstate == WEAPON_RDROPPING )
		return;
	if( (clipsize = wp->clipsize) == 0 )
		return;
	if( Q3F_GetClipValue(ps->weapon,ps) == clipsize )
		return;
	// RR2DO2 : if we don't have enough ammo to reload, return
	if(Q3F_GetClipValue( ps->weapon,ps ) >= ps->ammo[ Q3F_GetAmmoTypeForWeapon(ps->weapon)] )
		return;

	ps->stats[STAT_Q3F_FLAGS] |= 1<< Q3F_WEAPON_RELOAD;
}
