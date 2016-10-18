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
