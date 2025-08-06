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
**	g_q3f_eventlog.c
**
*/

#include "g_q3f_eventlog.h"
#include "g_q3f_team.h"

#include <cJSON.h>

void G_EventLog_Init(void) {
	char eventlogname[MAX_CVAR_VALUE_STRING];
	qtime_t t;
	int teams[4] = { 0 };
	int i, numTeams = 0;
	trap_RealTime(&t);

	// loop to get count
	for (i = Q3F_TEAM_RED; i < Q3F_TEAM_SPECTATOR; i++) {
		if ((g_q3f_allowedteams & (1 << i))) {
			teams[i - Q3F_TEAM_RED] = i;
			numTeams++;
		}
	}

	Com_sprintf(eventlogname, sizeof(eventlogname), "%d-%02d-%02d-%02d-%02d-%02d_[%s]", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, level.rawmapname);

	if (numTeams > 0) {
		Q_strcat(eventlogname, sizeof(eventlogname), "_");
		// loop to build string
		for (i = 0; i < numTeams; i++) {
			if (i != 0)
				Q_strcat(eventlogname, sizeof(eventlogname), "_");
			Q_strcat(eventlogname, sizeof(eventlogname), g_q3f_teamlist[teams[i]].name);
			if (i != numTeams - 1) {
				Q_strcat(eventlogname, sizeof(eventlogname), "_vs");
			}
		}
	}

	Q_strcat(eventlogname, sizeof(eventlogname), ".json");

	trap_FS_FOpenFile(eventlogname, &level.eventLogFile, FS_WRITE);

	if (!level.eventLogFile) {
		G_Printf("Warning: Could not open event log file: %s\n", eventlogname);
	}
	else {
		G_Printf("Writing events to %s\n", eventlogname);
	}

	trap_FS_Write("[\n", 2, level.eventLogFile);
}

void G_EventLog_Shutdown(void) {
	if (level.eventLogFile) {
		G_EventLog_TeamScores();
		G_EventLog_GameEnd();
		trap_FS_FCloseFile(level.eventLogFile);
		level.eventLogFile = 0;
	}
}

// serialise a JSON object and write it to the specified file
static void Q_FSWriteJSON( cJSON *object, fileHandle_t f ) {
	char *serialised = NULL;

	serialised = cJSON_PrintUnformatted( object );
	trap_FS_Write( serialised, (int)strlen( serialised ), f );

	cJSON_free( serialised );
	cJSON_Delete( object );
}

// TODO implement the milliseconds from qwtf-live
// 
static const char *ISOTimemills(void) {
	static char timeuse[64] = { 0 };
	qtime_t now;
	trap_RealTime( &now );
	Com_sprintf(timeuse, sizeof(timeuse), "%d-%02d-%02dT%02d:%02d:%02d.%dZ", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, 100);
	return timeuse;
}

void G_EventLog_PlayerStart( gentity_t *player ) {
	cJSON *root;

	G_EventLog_GameStart(); // called here temporarily because on first player join in game so it at least shows 1 player

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "playerStart" );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", player->client->ps.persistant[PERS_CURRCLASS] );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

static qboolean didgamestart = qfalse;

// Note: CalculateRanks needs to have been called in order for level.numPlayingClients/numConnectedClients to be accurate
void G_EventLog_GameStart( void ) {
	cJSON *root;
	int i, numTeams = 0;
	char hostname[MAX_CVAR_VALUE_STRING*2];

	if (didgamestart)
		return;
	didgamestart = qtrue;

	for( i = Q3F_TEAM_RED; i < Q3F_TEAM_SPECTATOR; i++ ) {
		if ((g_q3f_allowedteams & (1 << i)))
			numTeams++;
	}

	trap_Cvar_VariableStringBuffer("sv_hostname", hostname, sizeof(hostname));

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "gameStart" );
	//cJSON_AddStringToObject( root, "wall_time", "todo" );
	cJSON_AddStringToObject( root, "map", g_mapname.string );
	cJSON_AddNumberToObject( root, "gameindex", g_gameindex.integer );
	cJSON_AddNumberToObject( root, "numPlayers", level.numConnectedClients );
	cJSON_AddNumberToObject( root, "numTeams", numTeams );
	//cJSON_AddStringToObject( root, "demo", "todo" );
	//cJSON_AddStringToObject( root, "gameToken", "todo" );
	cJSON_AddStringToObject( root, "server_name", hostname );
	//cJSON_AddStringToObject( root, "server_address", hostname ); net_ip is possible option but it is not the actual real ip unless manually specified on server startup
	//cJSON_AddStringToObject( root, "region", region );
	//cJSON_AddStringToObject( root, "shard", shard );
	//cJSON_AddStringToObject( root, "mvd", mvdname );

	Q_FSWriteJSON( root, level.eventLogFile );
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_GameEnd( void ) {
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "gameEnd" );
	//cJSON_AddStringToObject( root, "wall_time", "todo" );

	Q_FSWriteJSON( root, level.eventLogFile );
	trap_FS_Write( "\n]", 2, level.eventLogFile );
}

void G_EventLog_RoundStart( void ) {
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "roundStart" );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_RoundEnd( void ) {
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "roundEnd" );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_ChangeClass( gentity_t *player, int prevclass, int nextclass, int timeplayed ) {
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "changeClass" );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", prevclass );
	cJSON_AddNumberToObject( root, "nextClass", nextclass );
	cJSON_AddNumberToObject( root, "team", player->client->sess.sessionTeam );
	cJSON_AddNumberToObject( root, "timePlayed", timeplayed );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_Damage( gentity_t *attacker, gentity_t *target, gentity_t *inflictor, meansOfDeath_t mod, int damage ) {
	cJSON *root;
	const char *damageKind = "enemy";
	const char *attackerName, *targetName;

	if ( attacker ) {
		if ( attacker->client ) {
			attackerName = attacker->client->pers.netname;
		} else {
			attackerName = "world";
		}
	} else {
		attackerName = "world";
	}

	if (attacker == target) {
		damageKind = "self";
	}
	else if (G_Q3F_IsAllied(attacker, target)) {
		damageKind = "team";
	}

	if ( target && target->client )
		targetName = target->client->pers.netname;
	else
		targetName = "unknown";

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "damageDone" );
	cJSON_AddStringToObject( root, "kind", damageKind );
	cJSON_AddStringToObject( root, "player", attackerName );
	cJSON_AddNumberToObject( root, "playerClass", attacker && attacker->client ? attacker->client->ps.persistant[PERS_CURRCLASS] : Q3F_CLASS_NULL );
	cJSON_AddNumberToObject( root, "playerTeam", attacker && attacker->client ? attacker->client->sess.sessionTeam : Q3F_TEAM_FREE );
	cJSON_AddStringToObject( root, "target", targetName );
	cJSON_AddNumberToObject( root, "targetClass", target && target->client ? target->client->ps.persistant[PERS_CURRCLASS] : Q3F_CLASS_NULL );
	cJSON_AddNumberToObject( root, "targetTeam", target && target->client ? target->client->sess.sessionTeam : Q3F_TEAM_FREE );
	cJSON_AddNumberToObject( root, "damage", damage );
	cJSON_AddStringToObject( root, "inflictor", mod >= MOD_UNKNOWN && mod < MOD_LASTONE && modNames[mod] ? modNames[mod] : "MOD_UNKNOWN" );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_Death( gentity_t *attacker, gentity_t *target, gentity_t *inflictor, meansOfDeath_t mod ) {
	cJSON *root;
	const char *killKind = "enemy";
	const char *attackerName, *targetName;

	if ( attacker ) {
		if ( attacker->client ) {
			attackerName = attacker->client->pers.netname;
		} else {
			attackerName = "world";
		}
	} else {
		attackerName = "world";
	}

	if (attacker == target) {
		killKind = "self";
	}
	else if (G_Q3F_IsAllied(attacker, target)) {
		killKind = "team";
	}

	if ( target && target->client )
		targetName = target->client->pers.netname;
	else
		targetName = "unknown";

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "kill" );
	cJSON_AddStringToObject( root, "kind", killKind );
	cJSON_AddStringToObject( root, "player", attackerName );
	cJSON_AddNumberToObject( root, "playerClass", attacker && attacker->client ? attacker->client->ps.persistant[PERS_CURRCLASS] : Q3F_CLASS_NULL );
	cJSON_AddNumberToObject( root, "playerTeam", attacker && attacker->client ? attacker->client->sess.sessionTeam : Q3F_TEAM_FREE );
	cJSON_AddStringToObject( root, "target", targetName );
	cJSON_AddNumberToObject( root, "targetClass", target && target->client ? target->client->ps.persistant[PERS_CURRCLASS] : Q3F_CLASS_NULL );
	cJSON_AddNumberToObject( root, "targetTeam", target && target->client ? target->client->sess.sessionTeam : Q3F_TEAM_FREE );
	cJSON_AddStringToObject( root, "inflictor", mod >= MOD_UNKNOWN && mod < MOD_LASTONE && modNames[mod] ? modNames[mod] : "MOD_UNKNOWN" );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

static const char *axeNames[Q3F_CLASS_MAX] = {
	"axe",
	"axe",
	"axe",
	"axe",
	"axe",
	"syringe",
	"axe",
	"axe",
	"knife",
	"wrench",
	"axe",
};

static const char *gunNames[] = {
	"none",
	"axe",
	"shotgun",
	"supershotgun",
	"nailgun",
	"supernailgun",
	"grenadelauncher",
	"rocketlauncher",
	"sniperrifle",
	"railgun",
	"flamethrower",
	"minigun",
	"assaultrifle",
	"dartgun",
	"pipelauncher",
	"napalmcannon",
	""
};

void G_EventLog_Attack(gentity_t *player, int weaponid) {
	cJSON *root;

	if ( !player || !player->inuse || !player->client || player == &g_entities[ENTITYNUM_WORLD])
		return;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "attack" );
	cJSON_AddNumberToObject( root, "team", player->client->sess.sessionTeam );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", player->client->ps.persistant[PERS_CURRCLASS] );
	if (weaponid == WP_AXE)
		cJSON_AddStringToObject( root, "inflictor", axeNames[player->client->ps.persistant[PERS_CURRCLASS]]);
	else
		cJSON_AddStringToObject( root, "inflictor", gunNames[weaponid]);

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_Goal(gentity_t *player) {
	cJSON *root;

	if ( !player || !player->inuse || !player->client || player == &g_entities[ENTITYNUM_WORLD])
		return;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "goal" );
	cJSON_AddNumberToObject( root, "team", player->client->sess.sessionTeam );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", player->client->ps.persistant[PERS_CURRCLASS] );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_GoalPickup(gentity_t *player) {
	cJSON *root;

	//if ( !player || !player->inuse || !player->client || player == &g_entities[ENTITYNUM_WORLD])
	//	return;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "pickup" );
	cJSON_AddNumberToObject( root, "team", player->client->sess.sessionTeam );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", player->client->ps.persistant[PERS_CURRCLASS] );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_GoalFumble(gentity_t *player, int timecarried) {
	cJSON *root;

	//if ( !player || !player->inuse || !player->client || player == &g_entities[ENTITYNUM_WORLD])
	//	return;

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "fumble" );
	cJSON_AddNumberToObject( root, "team", player->client->sess.sessionTeam );
	cJSON_AddStringToObject( root, "player", player->client->pers.netname );
	cJSON_AddNumberToObject( root, "playerClass", player->client->ps.persistant[PERS_CURRCLASS] );
	cJSON_AddNumberToObject( root, "timeCarried", timecarried );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}

void G_EventLog_TeamScores(void) {
	cJSON *root;
	int i, numTeams = 0;
	int winScore = 0, winTeam = 0;

	for( i = Q3F_TEAM_RED; i < Q3F_TEAM_SPECTATOR; i++ ) {
		if ((g_q3f_allowedteams & (1 << i)))
			numTeams++;
	}

	if (level.teamScores[Q3F_TEAM_RED] > winScore) {
		winScore = level.teamScores[Q3F_TEAM_RED];
		winTeam = Q3F_TEAM_RED;
	}

	if (level.teamScores[Q3F_TEAM_BLUE] > winScore) {
		winScore = level.teamScores[Q3F_TEAM_BLUE];
		winTeam = Q3F_TEAM_BLUE;
	}
	else if (level.teamScores[Q3F_TEAM_BLUE] == winScore ) {
		winTeam = 0;
	}

	if (level.teamScores[Q3F_TEAM_YELLOW] > winScore) {
		winScore = level.teamScores[Q3F_TEAM_YELLOW];
		winTeam = Q3F_TEAM_YELLOW;
	}
	else if (level.teamScores[Q3F_TEAM_YELLOW] == winScore ) {
		winTeam = 0;
	}

	if (level.teamScores[Q3F_TEAM_GREEN] > winScore) {
		winScore = level.teamScores[Q3F_TEAM_GREEN];
		winTeam = Q3F_TEAM_GREEN;
	}
	else if (level.teamScores[Q3F_TEAM_GREEN] == winScore ) {
		winTeam = 0;
	}

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject( root, "time", level.time );
	cJSON_AddStringToObject( root, "type", "teamScores" );
	cJSON_AddNumberToObject( root, "winningTeam", winTeam );
	cJSON_AddNumberToObject( root, "numTeams", numTeams );
	cJSON_AddNumberToObject( root, "team1Score", level.teamScores[Q3F_TEAM_RED] );
	cJSON_AddNumberToObject( root, "team2Score", level.teamScores[Q3F_TEAM_BLUE] );
	cJSON_AddNumberToObject( root, "team3Score", level.teamScores[Q3F_TEAM_YELLOW] );
	cJSON_AddNumberToObject( root, "team4Score", level.teamScores[Q3F_TEAM_GREEN] );

	Q_FSWriteJSON(root, level.eventLogFile);
	trap_FS_Write( ",\n", 2, level.eventLogFile );
}
