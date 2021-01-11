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
**	g_q3f_team.c
**
**	Server-side _only_ definitions for team stuff. This is actually
**	everything team related except the team number enum... I'm not sure
**	what bg_q3f_team.c/h is for.
*/

#include "g_local.h"
#include "g_q3f_team.h"
#include "g_q3f_mapents.h"

g_q3f_teaminfo_t g_q3f_teamlist[Q3F_TEAM_NUM_TEAMS];	// Array of teaminfo structures
int g_q3f_allowedteams;

static char *teamdescriptions[Q3F_TEAM_NUM_TEAMS] = {
	"No Team", "^1Red Team^7", "^4Blue Team^7", "^3Yellow Team^7", "^2Green Team^7", "Spectator Team"
};

static char *teamnames[Q3F_TEAM_NUM_TEAMS] = {
	"none", "red", "blue", "yellow", "green", "spectator"
};

static vec4_t teamcolors[Q3F_TEAM_NUM_TEAMS] = {
	{0, 0, 0, 1}, {1, 0.2f, 0.2f, 1}, {0.2f, 0.2f, 1, 1}, {1, 1, 0.2f, 1}, {0.2f, 1, 0.2f, 1}, {0.7f, 0.7f, 0.7f, 1}
};

void G_Q3F_InitTeams()
{
	// Initialize the team structures.

	int index, index2;
	for( index = 0; index < Q3F_TEAM_NUM_TEAMS; index++ )
	{
		g_q3f_teamlist[index].allyteams		= (1 << index);		// Only allied with self
		g_q3f_teamlist[index].playerlimit	= 0;
		g_q3f_teamlist[index].maxlives		= 0;
		g_q3f_teamlist[index].classmenu		= NULL;
		G_Q3F_AddString( &g_q3f_teamlist[index].description, teamdescriptions[index] );
		G_Q3F_AddString( &g_q3f_teamlist[index].name, teamnames[index] );
		//G_Q3F_AddString( &g_q3f_teamlist[index].maphelp, "No help available\n" );
		g_q3f_teamlist[index].color			= &teamcolors[index];
		for( index2 = Q3F_CLASS_NULL + 1; index2 < Q3F_CLASS_CIVILIAN; index2++ )
			g_q3f_teamlist[index].classmaximums[index2] = 256;
		g_q3f_teamlist[index].classmaximums[Q3F_CLASS_NULL] = 0;
		g_q3f_teamlist[index].classmaximums[Q3F_CLASS_CIVILIAN] = 0;
	}
}

// Allies: when it has no params, only allied with self
// if only the own team is given, 
void G_Q3F_SetTeamAllies(int team, const char *s)
{
	if ( s && *s )
		if( !Q_stricmp( s, "none" ) )
			g_q3f_teamlist[team].allyteams = 0;
		else
			g_q3f_teamlist[team].allyteams = G_Q3F_ProcessTeamString(s);
	else
		g_q3f_teamlist[team].allyteams = 0;
	
	g_q3f_teamlist[team].allyteams |= 1 << team;	//Always allied with yourself
}

int G_Q3F_GetTeamNum(const char *team)
{
	int i;
	
	i = atoi( team );
	if( i )
		return( i );
	for ( i = Q3F_TEAM_FREE + 1; i < Q3F_TEAM_NUM_TEAMS; i++ )
	{
		if (!Q_stricmp(team, g_q3f_teamlist[i].name))
			return i;
	}
	return 0;	// no valid team found!
}

static char *teammenustring;
void G_Q3F_SendTeamMenu( gentity_t *player, qboolean agentmenu )
{
	// Generate (and store statically) a team-string for classnames.
	// If teamnum is non-NULL, it's a disguise menu rather than a class menu

	int index;
	char *buff;

	if( !player->client )
		return;

	if( !teammenustring )
	{
		// Need to create a new team menu string

		buff = G_Alloc( 1500 );	// If it's bigger than this it's too big to fit on a menu
		for( index = 1; index < Q3F_TEAM_SPECTATOR; index++ )
		{
			if( (g_q3f_allowedteams & (1<<index)) && g_q3f_teamlist[index].description ) {
				Q_strcat( buff, 1500, va( " \"%s\"", g_q3f_teamlist[index].description ) );
			} else {
				Q_strcat( buff, 1500, " \"\"" );
			}
		}
		G_Q3F_AddString( &teammenustring, buff );
		G_Free( buff );
	}

	// RR2DO2
	if( player->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		trap_SendServerCommand( player->s.number, va(	"menu %s%s %i",														
														"spectatorteam",
														teammenustring,
														((player->client->sess.adminLevel >= ADMIN_MATCH || player->client->sess.adminLevel >= ADMIN_FULL) ? 1 : 0)) );
	} else {
		trap_SendServerCommand( player->s.number, va(	"menu %s%s",														
														(agentmenu ? "disguiseteam" : "team"),
														teammenustring ) );
	}
}

void G_Q3F_SendSpectateMenu( gentity_t *player ) {
	// Generate (and store statically) a team-string for classnames.
	// If teamnum is non-NULL, it's a disguise menu rather than a class menu

	int index;
	char *buff;

	if( !player->client )
		return;

	if( !teammenustring )
	{
		// Need to create a new team menu string

		buff = G_Alloc( 1500 );	// If it's bigger than this it's too big to fit on a menu
		for( index = 1; index < Q3F_TEAM_SPECTATOR; index++ )
		{
			if( (g_q3f_allowedteams & (1<<index)) && g_q3f_teamlist[index].description )
				Q_strcat( buff, 1500, va( " \"%s\"", g_q3f_teamlist[index].description ) );
			else Q_strcat( buff, 1500, " \"\"" );
		}
		G_Q3F_AddString( &teammenustring, buff );
		G_Free( buff );
	}

	trap_SendServerCommand( player->s.number, va(	"menu %s%s",
													"spectatortype",
													teammenustring ) );
}

void G_Q3F_SetAllowedTeams(void)
{
	// Scan for all spawnpoints, and see if there are teamlocks... if so,
	// that's the teams we're allowed.

	gentity_t *ent;
	int teammask;
	char *classname;

	g_q3f_allowedteams = 0;
	teammask = (1<<Q3F_TEAM_RED)|(1<<Q3F_TEAM_BLUE)|(1<<Q3F_TEAM_YELLOW)|(1<<Q3F_TEAM_GREEN);
	for( ent = g_entities; ent < &g_entities[level.num_entities]; ent++ )
	{
		if( !ent->inuse || !(classname = ent->classname) )
			continue;
		if( !Q_stricmp( classname, "info_player_deathmatch" ) )
		{
			if( ent->mapdata )
				g_q3f_allowedteams |= ent->mapdata->team;
			else if( level.ctfcompat && !(ent->mapdata && ent->mapdata->team) )
				G_FreeEntity( ent );		// On CTF compat mode, kill any non-teamed spawns
		}
		else if( !Q_stricmp( classname, "team_CTF_redspawn" ) )
			g_q3f_allowedteams |= (1 << Q3F_TEAM_RED);
		else if( !Q_stricmp( classname, "team_CTF_bluespawn" ) )
			g_q3f_allowedteams |= (1 << Q3F_TEAM_BLUE);
	}

	g_q3f_allowedteams &= teammask;
	if( !g_q3f_allowedteams )
		g_q3f_allowedteams = teammask;

	trap_SetConfigstring(CS_TEAMMASK, va("%d", g_q3f_allowedteams));
}

void G_Q3F_SetAlliedTeams(void) {
	trap_SetConfigstring(CS_TEAMALLIED, va("%d %d %d %d", 
		g_q3f_teamlist[Q3F_TEAM_RED].allyteams,
		g_q3f_teamlist[Q3F_TEAM_BLUE].allyteams,
		g_q3f_teamlist[Q3F_TEAM_YELLOW].allyteams,
		g_q3f_teamlist[Q3F_TEAM_GREEN].allyteams
	));
}

// returns 0 if all teams are full or when it can't find a valid team
int G_Q3F_GetAutoTeamNum( int ignoreClientNum )
{
	int i;
	int weakestteams,weakestcount;
	int choices[Q3F_TEAM_NUM_TEAMS];
	int teamcount;
	int lowestscore, lowteamscore;

	weakestteams=0;
	weakestcount=MAX_CLIENTS;

	for (i=Q3F_TEAM_FREE + 1; i < Q3F_TEAM_SPECTATOR; i++)
	{
		// is this team enabled?
		if( !(g_q3f_allowedteams & (1 << i)) )	
			continue;
		teamcount = TeamCount( ignoreClientNum, i);
		/* Can this team take more players? */
		if (g_q3f_teamlist[i].playerlimit > 0 && teamcount >= g_q3f_teamlist[i].playerlimit)
			continue;
		/* Check for the first or an even lower team */
		if (weakestteams == 0 || teamcount < weakestcount ) {
			choices[0]=i;
			weakestteams=1;
			weakestcount=teamcount;
		/* Check if this team has the same player count */
		} else if (teamcount == weakestcount) {
			choices[weakestteams++]=i;
		}
	}
	if (!weakestteams) {
		return 0;
	}
	/* Find team with lowest score */
	lowestscore = level.teamScores[choices[0]];
	lowteamscore = choices[0];

	for(i = 1; i < weakestteams; i++) {
		if(level.teamScores[choices[i]] < lowestscore)
			lowteamscore = choices[i];
	}

	return lowteamscore;		//choices[rand() % weakestteams];
}

int G_Q3F_ClassCount( int team, int playerclass ) {
	int i, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
/*		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}*/
		if ( team >= 0 && cl->sess.sessionTeam != team ) {
			continue;
		}
		if ( cl->ps.persistant[PERS_CURRCLASS] != playerclass ) {
			continue;
		}
		num++;
	}
	return num;	
}

qboolean G_Q3F_IsAllied( gentity_t *player, gentity_t *possibleally )
{
	// See if ent2 is an ally of ent1 (not necessarily true the other way)

	if( player->client && possibleally->client &&
		g_q3f_teamlist[player->client->sess.sessionTeam].allyteams & (1 << possibleally->client->sess.sessionTeam) )
		return( qtrue );
	return( qfalse );
}
