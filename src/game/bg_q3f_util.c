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
===============
Clip Handling Functions			-- JT
===============
*/

#include "g_local.h"
#include "q_shared.h"
#include "bg_public.h"
#include "bg_q3f_util.h"

/* Weapons are:
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
	WP_NUM_WEAPONS
*/ 

static int Q3F_ClipOffsets[] =
{
	-1,	//	WP_NONE,

	-1,	//	WP_AXE,
	0,	//	WP_SHOTGUN,
	1,	//	WP_SUPERSHOTGUN,
	-1,	//	WP_NAILGUN,
	-1,	//	WP_SUPERNAILGUN,
	2,	//	WP_GRENADE_LAUNCHER,
	3,	//	WP_ROCKET_LAUNCHER,
	-1,	//	WP_SNIPER_RIFLE,
	-1,	//	WP_RAILGUN,
	-1,	//	WP_FLAMETHROWER,
	-1,	//	WP_MINIGUN,
	-1,	//	WP_ASSAULTRIFLE,
	-1,	//	WP_DARTGUN,
	2,	//	WP_PIPELAUNCHER (apparently _NOT_ the same as the gren launcher) // JT: Apparently it _IS_. One to me ;)
	-1,	//	WP_NAPALMCANNON,
};


// JT: Mapping from ammunition type to the possible weapons that can have clips containing them.
// Note that this is _CLIPS_, not all weapons.
static int Q3F_AmmoClips[4][3] = 
{
	{ WP_SHOTGUN, WP_SUPERSHOTGUN, -1},		//	AMMO_SHELLS
	{ -1, -1, -1 },				//  AMMO_NAILS
	{ WP_ROCKET_LAUNCHER, WP_GRENADE_LAUNCHER, -1 },			//	AMMO_ROCKETS
	{ -1, -1, -1 },				// AMMO_CELLS
};


void Q3F_CapClips(playerState_t *playstate)
{
	int i,j;
	for(i=AMMO_SHELLS; i < AMMO_CELLS; i++)
	{
		j=0;
		while(Q3F_AmmoClips[i][j] !=-1)
		{
			if(Q3F_GetClipValue(Q3F_AmmoClips[i][j], playstate) > playstate->ammo[i])
				Q3F_SetClipValue(Q3F_AmmoClips[i][j], playstate->ammo[i], playstate);
			j++;
		}
	}
}

int Q3F_GetAmmoNumInClip(int ammotype, playerState_t *playstate)
{
	int i, temp;
	int num=0;

	if(ammotype < AMMO_SHELLS || ammotype>AMMO_CELLS)		// Safety
	{
		return 0;
	}
	
	i=0;	
	while(Q3F_AmmoClips[ammotype][i] != -1)
	{
		temp = Q3F_GetClipValue(Q3F_AmmoClips[ammotype][i], playstate);
		if(temp > num)
			num = temp;
		i++;
	}
	return num;
}

void Q3F_SetClipValue(int weapon, int value, playerState_t *playstate)
{
	int woffset;
	int offset;

	if(weapon > WP_NUM_WEAPONS || weapon <0)		// Safety.
		return;

	woffset = Q3F_ClipOffsets[weapon];
	offset = AMMO_CLIP1+(woffset>>1);
	if(woffset == -1)
		return;										// Trying to set the value of a non-clipped weapon. So abort.


	if(woffset &1)
	{
		playstate->ammo[offset] = (playstate->ammo[offset] & 0xff00) + (value & 0xff);
	}
	else
	{
		playstate->ammo[offset] = (playstate->ammo[offset] & 0xff) + ((value << 8)& 0x7f00);
	}
}

int Q3F_GetClipValue(int weapon, playerState_t *playstate)
{
	int woffset;
	int offset;

	if(weapon > WP_NUM_WEAPONS || weapon <0)		// Safety.
		return -1;


	woffset = Q3F_ClipOffsets[weapon];
	if(woffset == -1)								// Tried to get the value for a non-clipped weapon.
		return -1;
	offset = AMMO_CLIP1 + (woffset>>1);
	if(woffset & 1)
	{
		return(playstate->ammo[offset] & 0x7f);
	}
	else
	{
		return((playstate->ammo[offset] & 0x7f00)>>8);
	}
}

