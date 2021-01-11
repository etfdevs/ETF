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
**	cg_grenades.h
**
**	Client side functions for grenade handling.
*/

#ifndef	__CG_GRENADES_H
#define	__CG_GRENADES_H

#include "../game/bg_q3f_grenades.h"

typedef struct cg_q3f_grenade_s {
	bg_q3f_grenade_t *g;

	qboolean (*RenderGren)( struct centity_s *cent, refEntity_t *ent );	// Custom render

	qhandle_t hModel;			// Not set statically, a dynamic fudge thing :)
	qhandle_t hSkin;

} cg_q3f_grenade_t;

extern cg_q3f_grenade_t *cg_q3f_grenades[Q3F_NUM_GRENADES];

cg_q3f_grenade_t *CG_Q3F_GetGrenade( int index );
void CG_Q3F_RegisterGrenade( int grenadeNum );

void CG_Q3F_Grenade( centity_t *cent );		// Run function for grenades

void CG_Q3F_GrenOnePrime( void );		// Prime and throw grenade
void CG_Q3F_GrenTwoPrime( void );
void CG_Q3F_GrenThrow( void );
void CG_Q3F_GrenOnePlusPrime( void );
void CG_Q3F_GrenOneThrow( void );
void CG_Q3F_GrenTwoPlusPrime( void );
void CG_Q3F_GrenTwoThrow( void );
void CG_Q3F_GrenOneToggle( void );
void CG_Q3F_GrenTwoToggle( void );

#endif//__CG_GRENADES_H
