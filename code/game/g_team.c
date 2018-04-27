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

#include "g_local.h"
#include "g_q3f_team.h"

void Team_InitGame(void)
{
}

const char *TeamName(int team)  {
	if (team==Q3F_TEAM_RED)
		return "RED";
	else if (team==Q3F_TEAM_BLUE)
		return "BLUE";
	else if (team==Q3F_TEAM_YELLOW)
		return "YELLOW";
	else if (team==Q3F_TEAM_GREEN)
		return "GREEN";
	else if (team==Q3F_TEAM_SPECTATOR)
		return "SPECTATOR";
	return "FREE";
}

const char *TeamColorString(int team) {
	if (team==Q3F_TEAM_RED)
		return S_COLOR_RED;
	else if (team==Q3F_TEAM_BLUE)
		return S_COLOR_BLUE;
	else if (team==Q3F_TEAM_YELLOW)
		return S_COLOR_YELLOW;
	else if (team==Q3F_TEAM_GREEN)
		return S_COLOR_GREEN;
	else if (team==Q3F_TEAM_SPECTATOR)
		return S_COLOR_YELLOW;
	return S_COLOR_WHITE;
}

// NULL for everyone
void QDECL PrintMsg( gentity_t *ent, const char *fmt, ... ) {
	char		msg[1024];
	va_list		argptr;
	char		*p;
	
	va_start (argptr,fmt);
	if (Q_vsnprintf (msg, sizeof(msg), fmt, argptr) > (int)sizeof(msg)) {
		G_Error ( "PrintMsg overrun" );
	}
	va_end (argptr);

	// double quotes are bad
	while ((p = strchr(msg, '"')) != NULL)
		*p = '\'';

	trap_SendServerCommand ( ( (ent == NULL) ? -1 : ent-g_entities ), va("print \"%s\"", msg ));
}

/*
==============
AddTeamScore

 used for gametype > GT_TEAM
 for gametype GT_TEAM the level.teamScores is updated in AddScore in g_combat.c
==============
*/
void AddTeamScore(vec3_t origin, int team, int score) {
	gentity_t	*te;

	te = G_TempEntity(origin, EV_GLOBAL_TEAM_SOUND );
	te->r.svFlags |= SVF_BROADCAST;

	level.teamScores[ team ] += score;
}

/*
==============
OnSameTeam
==============
*/
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 ) {
	if ( !ent1 || !ent2 || !ent1->client || !ent2->client ) {
		return qfalse;
	}

	if ( ent1->client->sess.sessionTeam == ent2->client->sess.sessionTeam ) {
		return qtrue;
	}

	return qfalse;
}

/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/

g_q3f_location_t *Team_GetLocationFromPos( vec3_t pos )
{
	// Golliwog: Updated to use the Q3F location array.
	// If there are no ents in PVS, then just use the closest outside

	float			bestlen, secondbestlen, len;
	vec3_t			origin;
	int				index;
	q3f_data_t		*data;
	g_q3f_location_t *loc, *best, *secondbest;

	if( !level.locationarray )
		return( NULL );

	best = secondbest = NULL;
	bestlen = secondbestlen = 3*8192.0*8192.0;

	VectorCopy( pos, origin );

	for( index = -1; (data = G_Q3F_ArrayTraverse( level.locationarray, &index )) != NULL; )
	{
		loc = (g_q3f_location_t *) data->d.intdata;
		len = ( origin[0] - loc->pos[0] ) * ( origin[0] - loc->pos[0] )
			+ ( origin[1] - loc->pos[1] ) * ( origin[1] - loc->pos[1] )
			+ ( origin[2] - loc->pos[2] ) * ( origin[2] - loc->pos[2] );

		if ( len > bestlen ) {
			continue;
		}

		if ( !trap_InPVS( origin, loc->pos ) ) {
			if( len < secondbestlen )
			{
				secondbest = loc;
				secondbestlen = len;
			}
			continue;
		}

		bestlen = len;
		best = loc;
	}

	return( best ? best : secondbest );
}

g_q3f_location_t *Team_GetLocation( gentity_t *ent )
{
	// Golliwog: Updated to use the Q3F location array.
	// If there are no ents in PVS, then just use the closest outside

	return( Team_GetLocationFromPos( ent->r.currentOrigin ) );
}

/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
qboolean Team_GetLocationMsg(gentity_t *ent, char *loc, int loclen)
{
	g_q3f_location_t *best;

	best = Team_GetLocation( ent );
	
	if (!best)
		return qfalse;

	Com_sprintf(loc, loclen, "%s", best->str);

	return qtrue;
}


/*---------------------------------------------------------------------------*/

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_TEAM_SPAWN_POINTS	32
//gentity_t *SelectRandomTeamSpawnPoint( int teamstate, team_t team ) {
gentity_t *SelectRandomTeamSpawnPoint( int teamstate, q3f_team_t team ) {	// RR2DO2
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_TEAM_SPAWN_POINTS];
	char		*classname;

	if (teamstate == TEAM_BEGIN) {
		if (team == Q3F_TEAM_RED)
			classname = "team_CTF_redplayer";
		else if (team == Q3F_TEAM_BLUE)
			classname = "team_CTF_blueplayer";
		else
			return NULL;
	} else {
		if (team == Q3F_TEAM_RED)
			classname = "team_CTF_redspawn";
		else if (team == Q3F_TEAM_BLUE)
			classname = "team_CTF_bluespawn";
		else
			return NULL;
	}
	count = 0;

	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), classname)) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		if (++count == MAX_TEAM_SPAWN_POINTS)
			break;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), classname);
	}

	selection = rand() % count;
	return spots[ selection ];
}


/*
===========
SelectCTFSpawnPoint

============
*/
//gentity_t *SelectCTFSpawnPoint ( team_t team, int teamstate, vec3_t origin, vec3_t angles ) {
gentity_t *SelectCTFSpawnPoint ( q3f_team_t team, int teamstate, vec3_t origin, vec3_t angles ) {	// RR2DO2
	gentity_t	*spot;

	spot = SelectRandomTeamSpawnPoint ( teamstate, team );

	if (!spot) {
		return SelectSpawnPoint( vec3_origin, origin, angles, NULL );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*---------------------------------------------------------------------------*/

static int QDECL SortClients( const void *a, const void *b ) {
	return *(int *)a - *(int *)b;
}


/*
==================
TeamplayLocationsMessage

Old Format:
	clientNum location health armor weapon powerups

RR2DO2: New Format
	clientNum health armor weapon powerups

djbob: new new format
	clientNum health armor powerups
==================
*/
void TeamplayInfoMessage( q3f_team_t team ) {
	char		entry[1024];
	char		string[8192];
	int			stringlength;
	int			i, j;
	gentity_t	*player;
	int			cnt;
	int			h, a;
	int			clients[TEAM_MAXOVERLAY];
	char*		bufferedData;
	char*		tinfo;

	// figure out what client should be on the display
	// we are limited to 8, but we want to use the top eight players
	// but in client order (so they don't keep changing position on the overlay)
	for (i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++) {
		player = g_entities + level.sortedClients[i];
		if( player->inuse && (g_q3f_teamlist[player->client->sess.sessionTeam].allyteams & (1 << team))) {
		//if (player->inuse && player->client->sess.sessionTeam == team ) {
			clients[cnt++] = level.sortedClients[i];
		}
	}

	// We have the top eight players, sort them by clientNum
	qsort( clients, cnt, sizeof( clients[0] ), SortClients );

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	for (i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++) {
		player = g_entities + i;
		if( player->inuse && (g_q3f_teamlist[player->client->sess.sessionTeam].allyteams & (1 << team))) {
		//if (player->inuse && player->client->sess.sessionTeam == team ) {
			h = player->client->ps.stats[STAT_HEALTH];
			a = player->client->ps.stats[STAT_ARMOR];
			if (h < 0) h = 0;
			if (a < 0) a = 0;

			Com_sprintf (entry, sizeof(entry), " %i %i %i", i, h, a);
			j = strlen(entry);
			if (stringlength + j >= (int)sizeof(string))
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
			cnt++;
		}
	}
	bufferedData = level.tinfo[team];

	tinfo = va("tinfo %i%s", cnt, string);
	if(!Q_stricmp(bufferedData, tinfo)) { // Gordon: no change so just return
		return;
	}

	Q_strncpyz(bufferedData, tinfo, 1400);

	for(i = 0; i < level.numConnectedClients; i++) {
		player = g_entities + level.sortedClients[i];
		if( player->inuse && (g_q3f_teamlist[player->client->sess.sessionTeam].allyteams & (1 << team))) {
		//if (player->inuse && player->client->sess.sessionTeam == team) {
			trap_SendServerCommand( player-g_entities, tinfo);
		}
	}
}

void CheckTeamStatus(void) {
	if (level.time - level.lastTeamLocationTime > TEAM_LOCATION_UPDATE_TIME) {
		level.lastTeamLocationTime = level.time;

		TeamplayInfoMessage( Q3F_TEAM_RED );
		TeamplayInfoMessage( Q3F_TEAM_BLUE );
		TeamplayInfoMessage( Q3F_TEAM_YELLOW );
		TeamplayInfoMessage( Q3F_TEAM_GREEN);
	}
}

/*-----------------------------------------------------------------*/

/*QUAKED team_CTF_redplayer (1 0 0) (-16 -16 -16) (16 16 32)
Only in CTF games.  Red players spawn here at game start.
*/
void SP_team_CTF_redplayer( gentity_t *ent ) {
}


/*QUAKED team_CTF_blueplayer (0 0 1) (-16 -16 -16) (16 16 32)
Only in CTF games.  Blue players spawn here at game start.
*/
void SP_team_CTF_blueplayer( gentity_t *ent ) {
}


/*QUAKED team_CTF_redspawn (1 0 0) (-16 -16 -24) (16 16 32)
potential spawning position for red team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_redspawn(gentity_t *ent) {
}

/*QUAKED team_CTF_bluespawn (0 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for blue team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_bluespawn(gentity_t *ent) {
}



