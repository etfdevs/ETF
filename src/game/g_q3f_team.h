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
**	g_q3f_team.h
**
**	Server-side _only_ definitions for team stuff. This is actually
**	everything team related except the team number enum... I'm not sure
**	what bg_q3f_team.c/h is for. (Those are not usefull anymore, deleted)
*/

#ifndef	__G_Q3F_TEAM_H
#define	__G_Q3F_TEAM_H

#include "g_local.h"
#include "bg_q3f_playerclass.h"

typedef struct g_q3f_teaminfo_s {
	//char *name, *description, *classmenu, *maphelp;
	char *name, *description, *classmenu;
	vec4_t *color;
	int	allyteams;
	int playerlimit;
	int maxlives;
	int classmaximums[Q3F_CLASS_MAX];
	char *classnames[Q3F_CLASS_MAX];
} g_q3f_teaminfo_t;

extern g_q3f_teaminfo_t g_q3f_teamlist[Q3F_TEAM_NUM_TEAMS];	// Array of teaminfo structures
extern int g_q3f_allowedteams;

void G_Q3F_InitTeams(void);
void G_Q3F_SetTeamAllies(int team, const char *s);
int G_Q3F_GetTeamNum(const char *team);
void G_Q3F_SendTeamMenu( gentity_t *player, qboolean agentmenu );
void G_Q3F_SendSpectateMenu( gentity_t *player );
void G_Q3F_SetAllowedTeams(void);
void G_Q3F_SetAlliedTeams(void);
int G_Q3F_GetAutoTeamNum( int ignoreClientNum );
int G_Q3F_ClassCount( int team, int playerclass );
qboolean G_Q3F_IsAllied( gentity_t *player, gentity_t *possibleally );

#endif// __G_Q3F_TEAM_H
