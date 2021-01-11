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
**	bg_q3f_grenades.c
**
**	Shared functions for handling grenades
*/

#include "bg_q3f_grenades.h"
#include "bg_public.h"

bg_q3f_grenade_t bg_q3f_grenade_none = {
	Q3F_GFLAG_NOTHROW,		// Flags
	MOD_HANDGREN, 0,		// Damage mod and amount
	"No grenade",			// Name
	"models/ammo/grenade1.md3",	// Model
	"",						// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_normal = {
	0,						// Flags
	MOD_HANDGREN, 180,		// Damage mod and amount
	"Hand grenade",			// Name
	"models/ammo/grenade/grenade.md3",	// Model
	"",						// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_concuss = {
	0,												// Flags
	0,	0,											// Damage mod and amount
	"Stun grenade",									// Name
	"models/ammo/stungren/stungren.md3",			// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_flare = {
	Q3F_GFLAG_QUICKTHROW|Q3F_GFLAG_STICKY|Q3F_GFLAG_NOEXPLODE,	// Flags
	0,	0,				// Damage mod and amount
	"Flare",				// Name
	"models/ammo/grenade1.md3",	// Model
	"",
};

bg_q3f_grenade_t bg_q3f_grenade_flash = {
	0,												// Flags
	MOD_FLASHGREN,	60,								// Damage mod and amount
	"Flashbang grenade",							// Name
	"models/ammo/flashgren/flashgren.md3",			// Model
	""												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_nail = {
	Q3F_GFLAG_NOEXPLODE|Q3F_GFLAG_EXTENDEDEFFECT,	// Flags
	MOD_NAILGREN,	0,								// Damage mod and amount
	"Nail Bomb",									// Name
	"models/ammo/nailbomb/nailbomb.md3",			// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_cluster = {
	Q3F_GFLAG_EXTENDEDEFFECT,						// Flags
	MOD_CLUSTERGREN,	180,						// Damage mod and amount
	"Cluster bomb",									// Name
	"models/ammo/clusterbomb/clusterbomb.md3",		// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_clustersection = {
	0,												// Flags
	MOD_CLUSTERGREN,	180,						// Damage mod and amount
	"Cluster bomblet",								// Name
	"models/ammo/clusterbomb/bomblet.md3",			// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_napalm = {
	Q3F_GFLAG_NOSOUND|Q3F_GFLAG_EXTENDEDEFFECT|Q3F_GFLAG_LIESFLAT,		// Flags
	MOD_NAPALMGREN,	100,							// Damage mod and amount
	"Napalm grenade",								// Name
	"models/ammo/napalm/napalm.md3",				// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_gas = {
	Q3F_GFLAG_EXTENDEDEFFECT,						// Flags
	MOD_GASGREN,	0,								// Damage mod and amount
	"Gas grenade",									// Name
	"models/ammo/gasgren/gasgren.md3",				// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_emp = {
	Q3F_GFLAG_EXTENDEDEFFECT,						// Flags
	MOD_PULSEGREN,	0,								// Damage mod and amount
	"Pulse grenade",								// Name
	"models/ammo/pulse/pulsegren.md3",				// Model
	"",												// Skin
};

bg_q3f_grenade_t bg_q3f_grenade_charge = {
	Q3F_GFLAG_STICKY|Q3F_GFLAG_NOSPIN,	// Flags
	MOD_CHARGE,	0,				// Damage mod and amount
	"HE Charge",						// Name
	"models/objects/charge/charge.md3",	// Model
	"",								// Skin
};

bg_q3f_grenade_t *bg_q3f_grenades[Q3F_NUM_GRENADES] = {
	&bg_q3f_grenade_none,
	&bg_q3f_grenade_normal,
	&bg_q3f_grenade_concuss,
	&bg_q3f_grenade_flash,
	&bg_q3f_grenade_flare,
	&bg_q3f_grenade_nail,
	&bg_q3f_grenade_cluster,
	&bg_q3f_grenade_clustersection,
	&bg_q3f_grenade_napalm,
	&bg_q3f_grenade_gas,
	&bg_q3f_grenade_emp,
	&bg_q3f_grenade_charge,
};

bg_q3f_grenade_t *BG_Q3F_GetGrenade( int index )
{
	bg_q3f_grenade_t *gren;

	if(	index < 1 ||
		index >= Q3F_NUM_GRENADES )
		return( &bg_q3f_grenade_none );
	gren = bg_q3f_grenades[index];
	return( gren ? gren : &bg_q3f_grenade_none );
}

void BG_Q3F_ConcussionEffect( int seed, int left, vec3_t out ) {
	int i;
	float magnitude, frame;
	float concussPeriod[4];

	for( i = 0; i < 4; i++ ) {
		concussPeriod[i] = Q_random( &seed ) * 2*M_PI;
	}

	// Work out the 'angle' we're at, and the magnitude of effect
 	magnitude = ((left > 7000) ? 1 : (left / 7000.0)) * 6;
	frame = ((float)(left)) / 350;

	// Set all the offsets.
	out[0] = (sin( frame * 1.00 + concussPeriod[0] ) +
			  sin( frame * 0.57 + concussPeriod[1] )) 
			  *	magnitude;
	out[1] = (sin( frame * 0.90 + concussPeriod[2] ) +
			  sin( frame * 0.69 + concussPeriod[3] )) 
			  *	magnitude;
	out[2] = 0;
}
