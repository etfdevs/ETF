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
**	g_q3f_flag.h
**
**	Server-side stuff for Q3F flags entities (which will be a large
**	part of the whole lot)
*/

#ifndef __G_Q3F_FLAG_H
#define	__G_Q3F_FLAG_H


#include "g_local.h"
#include "g_q3f_mapdata.h"


void G_Q3F_FlagInfo( gentity_t *queryent );		// Query flag status.

void SP_Q3F_func_flag( gentity_t *ent );		// Spawn a func_flag

void Q3F_func_flag_touch( gentity_t *self, gentity_t *other, trace_t *trace );
void Q3F_func_flag_think( gentity_t *self );
void Q3F_func_flag_use( gentity_t *self, gentity_t *other, gentity_t *activator );

void G_Q3F_FlagUseHeld( gentity_t *player );	// 'use' specified flag.

float G_Q3F_CalculateGoalItemSpeedScale( gentity_t *player );	// Calculate speed scale.
void G_Q3F_DropFlag( gentity_t *ent );			// Drop the flag
void G_Q3F_DropAllFlags( gentity_t *player, qboolean ignorenodrop, qboolean ignorekeepondeath );	// Drop all a player's flags
void G_Q3F_ReturnFlag( gentity_t *ent );		// Return the flag

	// See if holder has all the named entities
qboolean G_Q3F_CheckHeld( gentity_t *holder, q3f_array_t *array );
qboolean G_Q3F_CheckNotHeld( gentity_t *holder, q3f_array_t *array );


#endif//__G_Q3F_FLAG_H
