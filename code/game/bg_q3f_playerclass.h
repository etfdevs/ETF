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
**	q3f_playerclass.h
**
**	Definitions for player classes
**
*/

#ifndef __BG_Q3F_PLAYERCLASS_H
#define __BG_Q3F_PLAYERCLASS_H

#include "q_shared.h"



#define Q3F_NUM_WEAPONSLOTS 10
#define Q3F_NUM_WEAPONMAPSLOTS	4
typedef struct bg_q3f_playerclass_s {
	// Text fields - title, description, 'command' (i.e. "become this class") strings.
	char *title, *description, *commandstring;
	char *sound[2], *commandsynonyms;

	// Class maximums
	int maxhealth, maxspeed, maxarmour;

	// Weapons
	int weaponslot[Q3F_NUM_WEAPONSLOTS];

	int defweapon;
	// Grenades
	int gren1type, gren2type;
	int gren1init, gren2init;
	int gren1max, gren2max;

	// Max Ammo
	int maxammo_shells, maxammo_nails, maxammo_rockets, maxammo_cells, maxammo_medikit, maxammo_charge;

	// Initial Ammo
	int initammo_shells, initammo_nails, initammo_rockets, initammo_cells, initammo_medikit, initammo_charge; //RR2DO2: ugh... LCC

	// Initial armour quantity
	int initarmour;

	// Initial/maximum armour type and class
	int maxarmourtype, initarmourtype, maxarmourclass, initarmourclass;

	// Mass (for knockback)
	int mass;

	// Agent 'masquerade' weapons, for trang/ssg/nailgun
	//int agentweap1, agentweap2, agentweap3;

	// Remap for agentdata
	int weaponmap[Q3F_NUM_WEAPONMAPSLOTS];

	vec3_t	mins;
	vec3_t	maxs;

} bg_q3f_playerclass_t;

enum {
	Q3F_CLASS_NULL = 0,			// This is a dummy ID (so we know if they have no class)

	Q3F_CLASS_RECON,			// Our list of classes
	Q3F_CLASS_SNIPER,
	Q3F_CLASS_SOLDIER,
	Q3F_CLASS_GRENADIER,
	Q3F_CLASS_PARAMEDIC,
	Q3F_CLASS_MINIGUNNER,
	Q3F_CLASS_FLAMETROOPER,
	Q3F_CLASS_AGENT,
	Q3F_CLASS_ENGINEER,
	Q3F_CLASS_CIVILIAN,

	Q3F_CLASS_MAX				// A placeholder for the number of classes
};

extern bg_q3f_playerclass_t *bg_q3f_classlist[];			// Array of pointers to class structures

bg_q3f_playerclass_t *BG_Q3F_GetClass( const playerState_t *ps );	// Get a pointer to a class structure
int CG_Q3F_GetClassNum(const char *classname);
int BG_Q3F_GetWeaponSlotFromWeaponNum( const playerState_t *ps, int num); // Reverse Weaponslot lookup
int BG_Q3F_GetRemappedWeaponFromWeaponNum( int classNum, int otherClassNum, int weapNum );	// Weapon map lookup

#endif	// __BG_Q3F_PLAYERCLASS_H
