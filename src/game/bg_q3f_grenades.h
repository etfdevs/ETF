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
**	bg_q3f_grenades.h
**
**	Common functions for grenade handling.
*/

#ifndef	__BG_GRENADES_H
#define	__BG_GRENADES_H

#include "q_shared.h"

enum {
	// Types of grenades

	Q3F_GREN_NONE,

	Q3F_GREN_NORMAL,
	Q3F_GREN_CONCUSS,
	Q3F_GREN_FLASH,
	Q3F_GREN_FLARE,
	Q3F_GREN_NAIL,
	Q3F_GREN_CLUSTER,
	Q3F_GREN_CLUSTERSECTION,
	Q3F_GREN_NAPALM,
	Q3F_GREN_GAS,
	Q3F_GREN_EMP,
	Q3F_GREN_CHARGE,

	Q3F_NUM_GRENADES
};

#define	Q3F_GFLAG_QUICKTHROW		0x01
#define	Q3F_GFLAG_STICKY			0x02
#define	Q3F_GFLAG_NOEXPLODE			0x04
#define	Q3F_GFLAG_NOTHROW			0x08
#define	Q3F_GFLAG_NOSPIN			0x10
#define	Q3F_GFLAG_NOSOUND			0x20
#define	Q3F_GFLAG_EXTENDEDEFFECT	0x40
#define Q3F_GFLAG_LIESFLAT			0x80

typedef struct bg_q3f_grenade_s {
	int flags;
	int mod, damage;			// Initial detonation values
	char *name, *model, *skin;
//	float light;
//	vec3_t lightColor;
} bg_q3f_grenade_t;

extern bg_q3f_grenade_t *bg_q3f_grenades[Q3F_NUM_GRENADES];

bg_q3f_grenade_t *BG_Q3F_GetGrenade( int index );
void BG_Q3F_ConcussionEffect( int seed, int left, vec3_t out );

#endif //__BG_GRENADES_H
