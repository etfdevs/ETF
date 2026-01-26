/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2026 Enemy Territory Fortress Development Team

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

#ifndef	__G_Q3F_EVENTLOG_H
#define	__G_Q3F_EVENTLOG_H

#include "g_local.h"

void G_EventLog_Init( void );
void G_EventLog_Shutdown( void );
void G_EventLog_PlayerStart( gentity_t *player );
void G_EventLog_GameStart( void );
void G_EventLog_GameEnd( void );
void G_EventLog_RoundStart( void );
void G_EventLog_RoundEnd( void );
void G_EventLog_ChangeClass( gentity_t *player, int prevclass, int nextclass, int timeplayed );
void G_EventLog_Damage( gentity_t *attacker, gentity_t *target, gentity_t *inflictor, meansOfDeath_t mod, int damage );
void G_EventLog_Death( gentity_t *attacker, gentity_t *target, gentity_t *inflictor, meansOfDeath_t mod );
void G_EventLog_Attack( gentity_t *player, int weaponid );
void G_EventLog_Goal( gentity_t *player );
void G_EventLog_GoalPickup( gentity_t *player );
void G_EventLog_GoalFumble( gentity_t *player, int timecarried );
void G_EventLog_TeamScores( void );

#endif
