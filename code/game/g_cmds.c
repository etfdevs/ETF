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
#include "g_q3f_playerclass.h"
#include "g_q3f_grenades.h"
#include "g_q3f_charge.h"
#include "g_q3f_flag.h"
#include "g_q3f_weapon.h"
#include "g_q3f_mapents.h"
#include "g_q3f_team.h"
#include "g_q3f_admin.h"
#include "bg_q3f_util.h"
#include "g_q3f_mapselect.h"
#include "bg_q3f_tea.h"
#include "../../ui/menudef.h"			// for the voice chats

#include "g_bot_interface.h"
#ifdef BUILD_LUA
#include "g_lua.h"
#endif

// 2147483647 10 digits int32

// 2 digits cl num
// 10 digits score
// 3 digits ping
// 10 digits time
// 3 digits flags

// 2 + 10 + 3 + 10 + 3 = 28

// 28 * 32 clients = 896

// strlen("scores") + 5 spaces = 11
// 4 team scores
// num clients sending max 2 characters
// (4*10)=40 + 2=42

// 42+11=53

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[900];
	char		string[1000];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted;
	int			score;
	gentity_t	*gent;


	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	numSorted = level.numConnectedClients;

	for (i=0 ; i < numSorted ; i++) {
		int		ping, flags;

		cl = &level.clients[level.sortedClients[i]];
		gent = &g_entities[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else if( cl->pers.initializing ) {
			ping = -2;
		} else {
//unlagged - true ping
			ping = cl->pers.realPing < 999 ? cl->pers.realPing : 999;
//			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}
		
		flags = 0;
		if (cl->pers.isReady)
			flags |= SCORE_FLAG_READY;
		if (cl->sess.adminLevel)
			flags |= SCORE_FLAG_ADMIN;
		if (cl->sess.spectatorState == SPECTATOR_FOLLOW)
			flags |= SCORE_FLAG_SPECFOLLOW;
		if (cl->sess.spectatorState == SPECTATOR_CHASE)
			flags |= SCORE_FLAG_SPECCHASE;
		if (cl->sess.spectatorState == SPECTATOR_FREE)
			flags |= SCORE_FLAG_SPECFREE;
		if (cl->sess.spectatorState == SPECTATOR_FLYBY)
			flags |= SCORE_FLAG_SPECFLYBY;
		if (cl->sess.shoutcaster)
			flags |= SCORE_FLAG_SHOUTCAST;
		if(gent->r.svFlags & SVF_BOT)
			flags |= SCORE_BOT;

		score = cl->ps.persistant[PERS_SCORE];
		if(g_teamFrags.integer)
			score = level.teamScores[cl->sess.sessionTeam];

		if(cl->sess.sessionTeam == Q3F_TEAM_SPECTATOR)
			score = cl->sess.spectatorClient;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i", 
			level.sortedClients[i],
			score, 
			ping, 
			(level.time - cl->pers.enterTime)/60000,
			flags
		);
		j = strlen(entry);
		if (stringlength + j >= sizeof(string))
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i %i %i%s", i, 
		level.teamScores[Q3F_TEAM_RED], level.teamScores[Q3F_TEAM_BLUE], level.teamScores[Q3F_TEAM_YELLOW], level.teamScores[Q3F_TEAM_GREEN],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
Cmd_Stats_f

Request current statistics information
==================
*/
void Cmd_Stats_f( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	for (i=0 ; i < STATS_NUM ; i++) {
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i", 
			ent->client->pers.stats.data[i].shots,
			ent->client->pers.stats.data[i].hits,
			ent->client->pers.stats.data[i].kills,
			ent->client->pers.stats.data[i].deaths
			);
		j = strlen(entry);
		if (stringlength + j >= sizeof(string))
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("stats \"%i %i %i %i %s\"", 
		ent->client->pers.stats.caps, ent->client->pers.stats.assists, ent->client->pers.stats.defends, ent->client->pers.stats.teamkills,
		string ) );
}

/*
==================
CheatsOk
==================
*/
qboolean CheatsOk( gentity_t *ent, qboolean silent ) {
	if ( !g_cheats.integer ) {
		if( !silent )
			trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		if( !silent )
			trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
//	q3f_playerclass_t *cls;

	if ( !CheatsOk( ent, qfalse ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
//		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		ent->health = BG_Q3F_GetClass( &ent->client->ps )->maxhealth;
		if (!give_all)
			return;
	}

	/*if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - 
			- ( 1 << WP_NONE );
		if (!give_all)
			return;
	}*/

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		// Civilians don't get ammo
		if(ent->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_CIVILIAN)
			return;

		for ( i = 0; i < AMMO_CLIP1; i++ ) {
			if ( i == AMMO_GRENADES ) 
				ent->client->ps.ammo[i] = 99 + (99 << 8);
			else
				ent->client->ps.ammo[i] = 999;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = BG_Q3F_GetClass( &ent->client->ps )->maxarmour;		//  200;

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "quad") == 0) {
		ent->client->ps.powerups[PW_QUAD] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "haste") == 0) {
		ent->client->ps.powerups[PW_HASTE] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "invis") == 0) {
		ent->client->ps.powerups[PW_INVIS] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "flight") == 0) {
		ent->client->ps.powerups[PW_FLIGHT] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "regen") == 0) {
		ent->client->ps.powerups[PW_REGEN] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "suit") == 0) {
		ent->client->ps.powerups[PW_BATTLESUIT] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "pent") == 0 || Q_stricmp(name, "pentagram") == 0) {
		ent->client->ps.powerups[PW_PENTAGRAM] = level.time + 100*1000;
		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "humiliation") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		if ( !it_ent || !it_ent->inuse )
			return;
		FinishSpawningItem(it_ent );
		if ( !it_ent || !it_ent->inuse )
			return;
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent, qfalse ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent, qfalse ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent, qfalse ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent, qfalse ) ) {
		return;
	}

	// doesn't work in single player
/*	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities, 
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}*/

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	//if ( ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR  || ent->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL) {
	if( Q3F_IsSpectator(ent->client) || ent->health <= 0 )	// RR2DO2
		return;

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	// RR2DO2: if diseased, give medic a frag
	/* Ensiform - (MAPDISEASE) But not if disease is non-client */
	/* Don't need double penalty for map disease deaths */
	/* Check for ceasefire in case it turns on when we've already been diseased */
	if ( (ent->client->ps.eFlags & EF_Q3F_DISEASED) && ent->client->diseaseEnt && ent->client->diseaseEnt->client ) {
		if( !level.ceaseFire && !(ent->client->diseaseEnt->client->ps.powerups[PW_Q3F_CEASEFIRE]) )
			AddScore(ent->client->diseaseEnt, ent->client->diseaseEnt->r.currentOrigin, G_Q3F_IsAllied( ent->client->diseaseEnt, ent ) ? -1 : 1 );
	}
	// RR2DO2
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);

	// no suicide delay in warmup
	if ( g_matchState.integer > MATCH_STATE_PLAYING ) {
		ent->client->killDelayTime = level.time;
		return;
	}

	trap_SendServerCommand(ent->s.clientNum, va("print \"%d second delay...\n\"", g_suicideDelay.integer ));
	ent->client->killDelayTime = level.time + 1000 * g_suicideDelay.integer;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	// RR2DO2: eeks, hardcoded stuff
	// Golliwog: Change cp to print

	if( client->pers.connected != CON_CONNECTED )
		return;		// Golliwog: We don't care about connection (i.e. saved session) team messages.

	if ( client->sess.sessionTeam == Q3F_TEAM_FREE ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		client->pers.netname));
	}
	else if( client->sess.sessionTeam != Q3F_TEAM_SPECTATOR )
		trap_SendServerCommand( -1, va("print \"%s^7 joined the %s.\n\"", client->pers.netname, g_q3f_teamlist[client->sess.sessionTeam].description) );
	/*if ( client->sess.sessionTeam == Q3F_TEAM_RED ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the red team.\n\"",
			client->pers.netname) );
	} else if ( client->sess.sessionTeam == Q3F_TEAM_BLUE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == Q3F_TEAM_SPECTATOR && oldTeam != Q3F_TEAM_SPECTATOR ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		client->pers.netname));
	} else if ( client->sess.sessionTeam == Q3F_TEAM_FREE ) {
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		client->pers.netname));
	}*/
}

/*
=================
SetTeam
=================
*/
qboolean SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	g_q3f_playerclass_t	*cls;

	if (!ent->inuse)
		return qfalse;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;

	if(level.clients[clientNum].punishTime) {
		if(level.clients[clientNum].punishTime < level.time) {
			level.clients[clientNum].punishTime = 0;
		}
		else
		{
			trap_SendServerCommand( clientNum, "print \"Sorry, you can't join the game yet\n\"");
			return qfalse;
		}
	}	

	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = Q3F_TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = Q3F_TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		// RR2DO2: not nice, but it's easier if not hardcoded ;)
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = Q3F_TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = Q3F_TEAM_BLUE;
		} else if ( !Q_stricmp( s, "yellow" ) || !Q_stricmp( s, "y" ) ) {
			team = Q3F_TEAM_YELLOW;
		} else if ( !Q_stricmp( s, "green" ) || !Q_stricmp( s, "g" ) ) {
			team = Q3F_TEAM_GREEN;
		} else {
			// pick the team with the least number of players
			//team = PickTeam( clientNum );
			team = G_Q3F_GetAutoTeamNum( clientNum );
		}

		// check, is "team" enabled? if no, then teamfree
		if(	g_q3f_allowedteams & (1 << team) ) { // is this team enabled?
		} else {
			team = Q3F_TEAM_SPECTATOR;
		}
		
		if ( g_q3f_teamlist[team].playerlimit > 0 && TeamCount( clientNum, team) >= g_q3f_teamlist[team].playerlimit ) // and can it handle more players?
			team = Q3F_TEAM_SPECTATOR;
	}

	if ( g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = Q3F_TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam ) {
		return qfalse;
	}

	// Golliwog: Check for sentry-dismantling cheats
	if( team != Q3F_TEAM_SPECTATOR )
	{
		if( client->sess.lastTeamChangeTime < client->sess.lastDismantleTime &&
			(client->sess.lastTeamChangeTime + Q3F_ADMIN_DISMANTLECHEAT_TIME) >= level.time &&
			g_banRules.value > 0)
		{
			G_Q3F_AdminTempBan( ent, "Swapped team and dismantled enemy buildings.", Q3F_ADMIN_TEMPBAN_TIME );
			return qfalse;
		}
		client->sess.lastTeamChangeTime = level.time;
	}
	// Golliwog.

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != Q3F_TEAM_SPECTATOR && !Q3F_IsSpectator(client)) {	// RR2DO2
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die (ent, ent, ent, 100000, MOD_SWITCHTEAM);

		/* Ensiform - Adding this because 'keepondeath' items should not still be carried across team changes */
		/* Disabling again for now since some of kaos' maps have BFF and might mess with certain incorrect ents for fast caps */
		//G_Q3F_DropAllFlags( ent, qtrue, qtrue );		// Golliwog: Drop any flags carried.

		cls = G_Q3F_GetClass( &ent->client->ps );
		if( cls->DeathCleanup )
			cls->DeathCleanup( ent );
		if( cls->EndClass )
			cls->EndClass( ent );		// Perform class term stuff (e.g. destroy sentries)
	}

	// Golliwog: Remove any active weapons, to stop team-switching cheats (e.g.
	// lob lots of grenades at a 'friendly' sentry then change team.
	G_Q3F_GenericEndCleanup( ent );

	// they go to the end of the line for tournements
	//if ( team == Q3F_TEAM_SPECTATOR ) {}
	if ( Q3F_IsSpectator(ent->client)) {	// RR2DO2
		client->sess.spectatorTime = level.time;
	} else {
		G_Q3F_SendClassMenu(ent, 0);
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	// RR2DO2 : set next class to nullclass, so game knows he is without one
	ent->client->ps.persistant[PERS_CURRCLASS] = Q3F_CLASS_NULL;
	ent->client->sess.sessionClass = Q3F_CLASS_NULL;
	// RR2DO2

#ifdef DREVIL_BOT_SUPPORT
	{
		BotUserData data;
		data.m_DataType = dtInt;
		data.udata.m_Int = team;
		Bot_Interface_SendEvent(MESSAGE_CHANGETEAM, clientNum, 0,0, &data);
	}
#endif

	BroadcastTeamChange( client, oldTeam );

	// Golliwog: Work out how many lives he has
	if( g_q3f_teamlist[team].maxlives >= 0 )
	{
		// Moving onto a lives-limited team

		if( client->sess.lives == -2 )
			client->sess.lives = g_q3f_teamlist[team].maxlives;		// First connection
		else {
			client->sess.lives--;
			if( client->sess.lives < 0 )
				client->sess.lives = 0;
			else if( client->sess.lives > g_q3f_teamlist[team].maxlives )
				client->sess.lives = g_q3f_teamlist[team].maxlives;
		}
	}
	else {
		// Unlimited lives for team
		if( client->sess.lives >= 0 || oldTeam != Q3F_TEAM_SPECTATOR )
			client->sess.lives = -1;
	}
	// Golliwog.

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum, "setteam" );

	ClientBegin( clientNum );

	G_Q3F_UpdateHUDIcons();		// Golliwog: HUD icons may no longer be valid for this player

	return qtrue;
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent, qboolean resetclient ) {
	ent->client->ps.persistant[ PERS_TEAM ] = Q3F_TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = Q3F_TEAM_SPECTATOR;	
	ent->client->ps.persistant[ PERS_CURRCLASS ] = Q3F_CLASS_NULL; // We are a null spectator again
	switch ( ent->client->sess.spectatorState )
	{
	default:
		break;
	case SPECTATOR_FOLLOW:
		ent->client->ps.pm_flags &= ~PMF_FOLLOW;
		break;
	case SPECTATOR_CHASE:
		ent->client->ps.pm_flags &= ~PMF_CHASE;
		break;
	}
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->spectatorTeam = Q3F_TEAM_SPECTATOR;

	if( resetclient )
		ent->client->sess.spectatorClient = -1;

	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.stats[STAT_HEALTH] = 100;			// WE LIVE,AGAIN!

	// Player won't be an engineer anymore so nuke the data
	G_Q3F_UpdateEngineerStats(ent);
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	oldTeam = ent->client->sess.sessionTeam;

	if ( trap_Argc() != 2 ) {
		switch ( oldTeam ) {
		case Q3F_TEAM_BLUE:
			trap_SendServerCommand( ent-g_entities, "print \"^4Blue team^7\n\"" );
			break;
		case Q3F_TEAM_RED:
			trap_SendServerCommand( ent-g_entities, "print \"^1Red team^7\n\"" );
			break;
		case Q3F_TEAM_GREEN:
			trap_SendServerCommand( ent-g_entities, "print \"^2Green team^7\n\"" );
			break;
		case Q3F_TEAM_YELLOW:
			trap_SendServerCommand( ent-g_entities, "print \"^3Yellow team^7\n\"" );
			break;
		case Q3F_TEAM_FREE:															// is this needed?
			trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
			break;
		case Q3F_TEAM_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
			break;
		}
		return;
	}

	if ( ent->client->sess.shoutcaster ) {
		trap_SendServerCommand( ent-g_entities, "print \"Shoutcasters may not join a team.\n\"" );
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	// RR2DO2: if in flyby, reset the flyby
	if( ent->client && ent->client->inFlyBy )
		trap_SendServerCommand( ent-g_entities, "flyby" );

	trap_Argv( 1, s, sizeof( s ) );
	
	if( SetTeam( ent, s ) ) {
		if ( oldTeam != ent->client->sess.sessionTeam ) {
			ent->client->switchTeamTime = level.time + 5000;
		}
	}
	ent->client->ps.persistant[PERS_FLAGS] |= PF_JOINEDTEAM;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent, spectatorState_t state ) {
	int		i, j;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 || ( Q3F_TEAM_RED <= ent->client->sess.sessionTeam && ent->client->sess.sessionTeam <= Q3F_TEAM_GREEN ) ) {
		if ( ent->client->sess.spectatorState == state ) {
			StopFollowing( ent, qfalse );
			//G_SetOrigin( ent, level.intermission_origin );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	// Golliwog: Start by checking if they want to follow a specific player
	for( i = Q3F_TEAM_RED; i <= Q3F_TEAM_GREEN; i++ )
	{
		//if( !(g_q3f_allowedteams & (1 << i)) )
		//	continue;		// Not allowed to follow this team
		if( Q_stricmp( g_q3f_teamlist[i].name, arg ) )
			continue;		// Not a string match
		/*if( ent->client->spectatorTeam == i )
			return;			// We're already on this team, do nothing.*/

		ent->client->spectatorTeam = i;
		for( j = 0; j < MAX_CLIENTS; j++ )
		{
			// Attempt to find a 'first' client to follow
			// Ensiform: Check all forms of spectator with the uniform function instead

			if(	&level.clients[j] != ent->client &&
				level.clients[j].sess.sessionTeam == (q3f_team_t)ent->client->spectatorTeam &&
				!Q3F_IsSpectator( &level.clients[j] ) )
			{
				ent->client->sess.spectatorState = state;
				ent->client->sess.spectatorClient = j;
				trap_SendServerCommand( -1, va(	"print \"%s^7 is spectating the %s team.\n",
												ent->client->pers.netname, g_q3f_teamlist[ent->client->spectatorTeam].name ) );
				return;
			}

//				// Failed to find someone, just go to intermission
//			StopFollowing( ent );
//			G_SetOrigin( ent, level.intermission_origin );
//			return;
		}
		if ( j == MAX_CLIENTS ) {
				// Failed to find someone, just go to intermission
			StopFollowing( ent, qtrue );
			G_SetOrigin( ent, level.intermission_origin );
			return;
		}
	}
	// Golliwog

	// Golliwog: Can't follow anyone without picking a spectate team first
	if( !ent->client->spectatorTeam )
		return;
	// Golliwog.

	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	//if ( level.clients[ i ].sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
	if (Q3F_IsSpectator(&level.clients[ i ])) {	// RR2DO2
		return;
	}

	// Golliwog: Can't spectate a different team
	//if( ent->client->spectatorTeam != level.clients[i].sess.sessionTeam )
	//	return;
	// Golliwog.

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}
	
	ent->client->sess.spectatorState = state;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir, spectatorState_t state  ) {
	int		clientnum;
	int		original;

	// Golliwog: Can't follow anyone without picking a spectate team first
	if( !ent->client->spectatorTeam )
		return;
	// Golliwog.

	// JT
	if(ent->client->sess.spectatorState != SPECTATOR_FOLLOW && ent->client->sess.spectatorState != SPECTATOR_CHASE)
		return;
	// JT

	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	while( 1 )
	{
		clientnum += dir;

		if( clientnum == original )
			break;		// Break the loop, we're back at the beginning

		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		//if ( level.clients[ clientnum ].sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		if ( Q3F_IsSpectator(&level.clients[ clientnum ] )) {	// RR2DO2
			continue;
		}

		// Golliwog: Can't spectate a different team
		if( ent->client->spectatorTeam != (int)level.clients[clientnum].sess.sessionTeam )
			continue;
		// Golliwog.

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = state;
		return;
	}

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static qboolean G_Q3F_CheckPunish( gentity_t *ent )
{
	gclient_t	*client;
	int			clientNum;

	client = ent->client;
	clientNum = client - level.clients;

	if(level.clients[clientNum].punishTime) {
		if(level.clients[clientNum].punishTime < level.time) {
			level.clients[clientNum].punishTime = 0;
			return qfalse;
		}

		return qtrue;
	}

	return qfalse;
}

static qboolean G_Q3F_CheckFlood( gentity_t *ent )
{
	// Check an entity isn't flooding, return TRUE if they are.
	// Credit builds up to a maximum of 6 seconds, each message uses up two seconds.
	// First violation brings a warning, subsequent violations silently add
	// 5 seconds to timeout.

	gclient_t *client;

	client = ent->client;

	if( g_adminFloodImmunity.integer && (client->sess.adminLevel == ADMIN_FULL || (client->pers.localClient && !(ent->r.svFlags & SVF_BOT))) )
	{
		// Flood Immunity to Full Admins
		return qfalse;
	}

	if( client->chatTime > level.time )
	{
		// Flooded out and still talking.

		if( !client->chatFloodWarning )
		{
			client->chatTime = level.time + 2000;
			trap_SendServerCommand( ent->s.number, "print \"Please do not flood the chat console.\n\"" );
			client->chatFloodWarning = qtrue;
		}
		else client->chatTime = level.time + 5000;	// They were warned.
		return( qtrue );
	}
	client->chatTime =	(client->chatTime > (level.time - 6000))
						? (client->chatTime + 2000)		// Add two seconds
						: (level.time - 4000);			// Cap at 4 seconds (6-2) in the past
	client->chatFloodWarning = qfalse;

	return( qfalse );
}

void _MS_TraceLocation( gentity_t *ent, vec3_t dest );

static char *G_Q3F_ParseSayString( const char *srcptr, gentity_t *activator, gentity_t *target, int colour )
{
	static char buf[2048];
	char *buffptr, *buffendptr;
	char curr;
	g_q3f_location_t *loc;
	bg_q3f_playerclass_t *cls;
	int colourstack[32], colourstacksize;
	vec3_t pos;

	if (!target->client)
		return "";

	if( colour < 0 || colour > 31 )
	{
		colour = ColorIndex( colour );
		if( colour < 0 || colour > 31 )
			colour = 7;
	}

	colourstack[0] = colour;
	colourstacksize = 0;

	buffendptr = &buf[sizeof( buf ) - 128];

	buf[0] = 0;
	for( buffptr = buf ; *srcptr && buffptr < buffendptr; )
	{
		curr = *srcptr++;
		if( curr != '%' && curr != '$' ) {
canthandle:
			*buffptr++ = curr;
			if( Q_IsColorStringPtr( srcptr - 1 ) )
			{
				if( *srcptr < '0' || toupper(*srcptr) > 'O' )
				{
					// A cancel, take the last one from the stack
					if( colourstacksize > 0 )
						colourstacksize--;
					colour = colourstack[colourstacksize];
					*buffptr++ = '0' + colour;
					srcptr++;
				} else {
					// A new colour, add it to the stack
					colour = ColorIndex( toupper(*srcptr) );
					*buffptr++ = *srcptr++;
					if( colourstacksize < 32 )
						colourstack[++colourstacksize] = colour;
				}
			}
			*buffptr = 0;
			continue;
		}
		curr = *srcptr++;
		switch( curr|32 )	{
		case 'h':	// Health
			Q_strcat( buffptr, buffendptr - buffptr, va("%d",activator->client->ps.stats[STAT_HEALTH]));
			break;
		case 'a':	// Armour
			Q_strcat( buffptr, buffendptr - buffptr, va("%d",activator->client->ps.stats[STAT_ARMOR]));
			break;
		case 'l':	// Location
			loc = Team_GetLocation( activator );
			Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location" );
			activator->client->reportLoc = loc;
			break;
		case 'd':	// Location of death
			loc = activator->client->deathLoc;
			Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location");
			break;
		case 'b':	// Last reported location
			loc = activator->client->reportLoc;
			Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location");
			break;
		case 'p':	// Location client is facing
			if( activator->health <= 0 )
				continue;
			_MS_TraceLocation( activator, pos );
			loc = Team_GetLocationFromPos( pos );
			Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location");
			break;
		case 'z':	// Sentry Location
			if( activator->client->sentry )
			{
				loc = Team_GetLocation( activator->client->sentry );
				Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location" );
			}
			else
			{
				Q_strcat( buffptr, buffendptr - buffptr, "no sentry" );
			}
			break;
		case 'x':	// Supply Station Location
			if( activator->client->supplystation )
			{
				loc = Team_GetLocation( activator->client->supplystation );
				Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location" );
			}
			else
			{
				Q_strcat( buffptr, buffendptr - buffptr, "no supply station" );
			}
			break;
		case 'v':	// Charge Location
			if( activator->client->chargeEntity )
			{
				loc = Team_GetLocation( activator->client->chargeEntity );
				Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location" );
			}
			else if( activator->client->chargeDisarmEnt )
			{
				loc = Team_GetLocation( activator->client->chargeDisarmEnt );
				Q_strcat( buffptr, buffendptr - buffptr, loc ? loc->str : "unknown location" );
			}
			else
			{
				Q_strcat( buffptr, buffendptr - buffptr, "no he charge" );
			}
			break;
		case 't':	// Team
			Q_strcat( buffptr, buffendptr - buffptr, g_q3f_teamlist[activator->client->sess.sessionTeam].description);
			break;
		case 'c':	// Team colour
			Q_strcat( buffptr, buffendptr - buffptr, g_q3f_teamlist[activator->client->sess.sessionTeam].name);
			break;
		case '1':	// Gren 1 Type
			cls = bg_q3f_classlist[activator->client->ps.persistant[PERS_CURRCLASS]];
			if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
				cls = BG_Q3F_GetClass( &activator->client->ps );
			Q_strcat( buffptr, buffendptr - buffptr, BG_Q3F_GetGrenade( cls->gren1type )->name );
			break;
		case '2':	// Gren 2 Type
			cls = bg_q3f_classlist[activator->client->ps.persistant[PERS_CURRCLASS]];
			if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
				cls = BG_Q3F_GetClass( &activator->client->ps );
			Q_strcat( buffptr, buffendptr - buffptr, BG_Q3F_GetGrenade( cls->gren2type )->name );
			break;
		case 'g':	// Disguise
			if( G_Q3F_IsDisguised( activator) ) 
			{
				if( activator->client->agentdata && (activator->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS )
				{
					Q_strcat( buffptr, buffendptr - buffptr, "invisible ");
				}
				Q_strcat( buffptr, buffendptr - buffptr, g_q3f_teamlist[activator->client->agentteam ? activator->client->agentteam : activator->client->sess.sessionTeam].description);
				while( *buffptr ) 
					buffptr++;	// Find end of string

				*buffptr++ = ' ';
				cls = bg_q3f_classlist[activator->client->agentclass ? activator->client->agentclass : activator->client->ps.persistant[PERS_CURRCLASS]];
				if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
					cls = BG_Q3F_GetClass( &activator->client->ps );
				Q_strcat( buffptr, buffendptr - buffptr, cls->title );
			}
			else {
				if( activator->client->agentdata && (activator->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS )
					Q_strcat( buffptr, buffendptr - buffptr, "invisible");
				else 
					Q_strcat( buffptr, buffendptr - buffptr, "undisguised");
			}
			break;
		case 's':	// Current class
			cls = bg_q3f_classlist[activator->client->ps.persistant[PERS_CURRCLASS]];
			if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
				cls = BG_Q3F_GetClass( &activator->client->ps );
			Q_strcat( buffptr, buffendptr - buffptr, cls->title);
			break;
		case 'n':	// Name
			Q_strcat( buffptr, buffendptr - buffptr, activator->client->pers.netname);
			break;
		case 'r':	// Name
			if ( target->client )
				Q_strcat( buffptr, buffendptr - buffptr, target->client->pers.netname);
			break;
		default:	// Another letter - leave it as-is, so it can be processed with va()
			goto canthandle;
		}
		buffptr = _MS_FixColour(buffptr, colour );
	}

	*buffptr = 0;	// Terminate buffer
	return( buf );
}

static qboolean G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, const char *channel, int soundindex, qboolean playstring ) {
	// Golliwog: Modified for channel filtering and returning whether or not it was sent.

	char *str;

	if (!other) {
		return( qfalse );
	}
	if (!other->inuse) {
		return( qfalse );
	}
	if (!other->client) {
		return( qfalse );
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return( qfalse );
	}
	if ( mode == SAY_TEAM && !G_Q3F_IsAllied( ent, other ) ) {
		return( qfalse );
	}

	if(	channel && other->client->chatchannels &&
		!G_Q3F_ArrayFind( other->client->chatchannels, (int) channel )
		&& ent != other )
			return( qfalse );	// This is a channel message, but the client isn't on this channel

	/* Ensiform - ET Ignore */
	if ( COM_BitCheck( other->client->sess.ignoreClients, ent - g_entities ) ) {
		// Return true to fool the counter so you can't see that someone has you ignored via "Nobody heard message."
		// Purposely fall through if ceasefire is on so that we can still chat
		if( !level.ceaseFire )
			return( qtrue );
	}

	if( *message )
	{
		str = G_Q3F_ParseSayString( message, ent, other, color );
		// limit the lenght of string
		*(str + (MAX_SAY_TEXT - 1)) = '\0';
		trap_SendServerCommand( other - g_entities, va("%s \"%s%c%c%s\" %i", 
			mode == SAY_TEAM ? "tchat" : "chat",
			name, Q_COLOR_ESCAPE, color, str,
			soundindex && ( ( mode == SAY_TEAM && g_teamChatSounds.integer >= 1 ) || g_teamChatSounds.integer >= 3 ) ? soundindex : 0 ));
		if( playstring && ( ( mode == SAY_TEAM && g_teamChatSounds.integer >= 2 ) || g_teamChatSounds.integer >= 4 ) )
			trap_SendServerCommand( other - g_entities, va( "tplaystring \"%s\"", str ) );

#ifdef BUILD_BOTS
		// Omni-bot: Tell the bot about the chat message
		Bot_Event_ChatMessage(other, ent, mode, str);
#endif
	}
	return( qtrue );
}

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatInput, int startIndex ) {
	int			j, k;
	gentity_t	*other;
	int			color, soundindex;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		sound[MAX_QPATH];
	char		*chanptr;
	qboolean	playstring;
	char		chatBuffer[MAX_SAY_TEXT];
	char		*chatText = chatBuffer;
	int			index = startIndex;

	if(!chatInput) {
		trap_Argv(index++, chatBuffer, MAX_SAY_TEXT);
	} else {
		Q_strncpyz(chatBuffer, chatInput, MAX_SAY_TEXT);
	}

	// Golliwog: Handle channel messages
	if( (*chatText == '#') ) {
		for( j = 0, chatText++; *chatText && *chatText != ' ' && j < 63; j++ ) {
			location[j] = *chatText++;
			if( location[j] >= 'A' && location[j] <= 'Z' )
				location[j] |= 32;		// Convert to lowercase
		}
		while( *chatText == ' ' )
			chatText++;		// Skip following space(s)
		location[j] = 0;
		if( qtrue /*mode == SAY_TEAM*/ )
			chanptr = G_Q3F_GetString( location );	// Find a stored copy
		else 
			chanptr = NULL;						// Don't use channel, it's not a team msg.

		if(!chatInput) {
			trap_Argv(index++, chatBuffer, MAX_SAY_TEXT);
			chatText = chatBuffer;
		}
	}
	else 
		chanptr = NULL;	// No channel
	// Golliwog.

	// Golliwog: Handle samples
	if( ( *chatText == '&') ) {
		for( j = 0, chatText++; *chatText && (!(*chatText == ' ' && chatInput)) && j < MAX_QPATH - 1; j++ ) {
			sound[j] = *chatText++;
			if( sound[j] >= 'A' && sound[j] <= 'Z' )
				sound[j] |= 32;		// Convert to lowercase
		}
		while( *chatText == ' ' )
			chatText++;		// Skip following space(s)
		sound[j] = 0;

		if( Q_strncmp( sound, "sound/voice/comms/", 18 ) && Q_strncmp( sound, "sound/voice/taunt/", 18 ) && Q_strncmp( sound, "sound/voices/spencer/", 21 ) && Q_strncmp( sound, "sound/player/medic", 18 ) && Q_strncmp( sound, "sound/player/engineer", 21 ) )		// Not a comms/taunt sample, fill in the path
			Q_strncpyz( sound, va( "sound/voice/comms/%s", sound ), sizeof(sound) );
		if( Q_stricmp( sound + strlen(sound) - 4, ".wav" ) )
			Q_strcat( sound, sizeof(sound), ".wav" );			// Add a .wav suffix
		soundindex = G_SoundIndex( sound );		// Register the sound

		if(!chatInput) {
			trap_Argv(index++, chatBuffer, MAX_SAY_TEXT);
			chatText = chatBuffer;
		}
	}
	else 
		soundindex = 0;	// No sound
	// Golliwog.

	// Golliwog: Handle sound dictionary messages
	if( *chatText == '@' ) {
		chatText++;
		playstring = qtrue;

		if(!chatInput) {
			trap_Argv(index++, chatBuffer, MAX_SAY_TEXT);
			chatText = chatBuffer;
		}
	}
	else {
		playstring = qfalse;
	}
	// Golliwog.

	if(!chatInput) {
		chatText = ConcatArgs( index-1 );
	}

 	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		// Target is assumed to be valid in cmd_tell_f when say_tell is passed
		G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]: ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		// echo the text to the console
		if ( g_dedicated.integer ) {
			G_Printf( "%s%s\n", name, text);
		}

		G_SayTo( ent, target, mode, color, name, text, chanptr, soundindex, playstring );
		G_SayTo( ent, ent, mode, color, name, text, chanptr, soundindex, playstring );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = k = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		if( G_SayTo( ent, other, mode, color, name, text, chanptr, soundindex, playstring ) && (ent != other) )
			k++;
	}
	if( !k && *text ) // Nobody heard!
		trap_SendServerCommand(	ent-g_entities, "print \"Nobody heard message.\n\"" );
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode ) {
	if ( trap_Argc () < 2 ) {
		return;
	}

	if( !G_Q3F_CheckFlood( ent ) && !G_Q3F_CheckPunish( ent ) )
		G_Say( ent, NULL, mode, NULL, 1 );
}

void Cmd_Ignore_f( gentity_t* ent ) {
	char cmd[MAX_TOKEN_CHARS];
	int cnum;
	gentity_t *player;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( !*cmd ) {
		trap_SendServerCommand( ent - g_entities, "print \"usage: ignore <clientnum>\n\"" );
		return;
	}

	cnum = atoi(cmd);

	// sanity check if it's a number
	if(cmd[0] >= '0' && cmd[0] <= '9')
	{
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != cnum)
				continue;

			if ( cnum == ent->s.number )
			{
				trap_SendServerCommand( ent - g_entities, "print \"Cannot ignore oneself\n\"" );
				return;
			}

			if ( COM_BitCheck( ent->client->sess.ignoreClients, cnum ) )
			{
				trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 is already on your ignore list.\n\"", player->client->pers.netname ) );
				return;
			}

			COM_BitSet( ent->client->sess.ignoreClients, cnum );
			trap_SendServerCommand( ent-g_entities, va( "print \"Added %s^7 to your ignore list.\n\"", player->client->pers.netname ) );
			return;
		}
	}

	trap_SendServerCommand( ent-g_entities, "print \"Invalid player index\n\"");
}

void Cmd_UnIgnore_f( gentity_t* ent ) {
	char cmd[MAX_TOKEN_CHARS];
	int cnum;
	gentity_t *player;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( !*cmd ) {
		trap_SendServerCommand( ent - g_entities, "print \"usage: unignore <clientnum>\n\"" );
		return;
	}

	cnum = atoi(cmd);

	// sanity check if it's a number
	if(cmd[0] >= '0' && cmd[0] <= '9')
	{
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != cnum)
				continue;

			if ( cnum == ent->s.number )
			{
				//trap_SendServerCommand( ent - g_entities, "print \"Cannot unignore oneself\n\"" );
				return;
			}

			if ( !COM_BitCheck( ent->client->sess.ignoreClients, cnum ) )
			{
				trap_SendServerCommand( ent - g_entities, va( "print \"%s^7 is not on your ignore list.\n\"", player->client->pers.netname ) );
				return;
			}

			COM_BitClear( ent->client->sess.ignoreClients, cnum );
			trap_SendServerCommand( ent-g_entities, va( "print \"Removed %s^7 from your ignore list.\n\"", player->client->pers.netname ) );
			return;
		}
	}

	trap_SendServerCommand( ent-g_entities, "print \"Invalid player index\n\"");
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	//char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 3 ) {
		trap_SendServerCommand( ent-g_entities, "print \"Usage: tell <player id> <message>\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg );
	if ( targetNum == -1 ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client ) {
		return;
	}

	/*p = ConcatArgs( 2 );
	
	if ( strlen( p ) >= MAX_SAY_TEXT ) {
		p[MAX_SAY_TEXT-1] = '\0';
	}*/

	if( !G_Q3F_CheckFlood( ent ) && !G_Q3F_CheckPunish( ent ) )
	{
		//G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
		G_Say( ent, target, SAY_TELL, NULL, 2 );
		//G_Say( ent, ent, SAY_TELL, NULL, 2 );
		// don't tell to the player self if it was already directed to this player
		// also don't send the chat back to a bot
		//if ( ent != target /*&& !(ent->r.svFlags & SVF_BOT)*/) {
		//	G_Say( ent, ent, SAY_TELL, NULL, 2 );
		//}
	}
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static qboolean CheckValidMap(const char *mapname) 
{
	char buffer[4096];
	char *p, *s;
	size_t len = strlen( mapname );

	trap_GetConfigstring(CS_MAPLIST, buffer, 4096);
	s = buffer;

	while(s[0]) {
		if (!Q_stricmpn(mapname, s, len )) {
			p = s+len;
			if (*p == '+' || *p == ';' || !(*p)) return qtrue;
		}
		p = strchr(s, ';');
		if (!p)
			break;
		s = p+1;
	}
	return qfalse;
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	char	*check;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_SAY_TEXT];

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
	if ( Q3F_IsSpectator( ent->client ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// check for command separators in arg2
	/* Ensiform - Fixes callvote server exploit */
	for ( check = arg2; *check; ++check ) {
		switch ( *check ) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand( ent - g_entities, "print \"Invalid vote string\n\"" );
				return;
			break;
		}
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	//} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "mute" ) ) {
	} else if ( !Q_stricmp( arg1, "unmute" ) ) {
//	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "capturelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "ceasefire" ) &&
		( !Q_stricmp( arg2, "on" ) || 
		  !Q_stricmp( arg2, "off" ) ) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname[+gameindex]>, kick <player>, clientkick <clientnum>, mute <clientnum>, unmute <clientnum>, timelimit <time>, capturelimit <captures>, ceasefire <on|off>.\n\"" );
		return;
	}

	if ( ent->client->sess.muted && Q_stricmp( arg1, "unmute" ) && !level.ceaseFire ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string while muted.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"You can only start a vote to unmute yourself.\n\"" );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	if ( !Q_stricmp( arg1, "map" ) ) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];
		char	name[MAX_STRING_CHARS];
		int		gi = 0;
		q3f_keypairarray_t *_mapinfo;		// Golliwog: The loaded .mpi/.mapinfo data (only during init).
		char *ptr;

		char *p = strchr(arg2, '+');
		if(p) {
			*p++ = '\0';
			gi = atoi(p);
		}

		if (!CheckValidMap(arg2)) {
			trap_SendServerCommand( ent-g_entities, "print \"Map is not in server list\n\"" );
			return;
		}

		name[0] = 0;
		_mapinfo = G_Q3F_LoadMapInfo( arg2 );								// Load associated map information file.
		if(_mapinfo) {
			ptr = G_Q3F_GetMapInfoEntry( _mapinfo, "longname", gi ? gi : 1, "<unknown>" );
			if(ptr)
				Q_strncpyz(name, ptr, MAX_STRING_CHARS);
			G_Q3F_KeyPairArrayDestroy( _mapinfo );
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "g_gameindex %d; %s %s; set nextmap \"%s\"", gi ? gi : 1, arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "g_gameindex %d; %s %s", gi ? gi : 1, arg1, arg2 );
		}
		if(*name)
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Change Map To %s", name );
		else
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Change Map To %s^7, gameindex %d", arg2, gi ? gi : 1 );
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");

		// ---
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Go To Next Map (%s)", s );
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
		int tlimit = atoi(arg2);

		if(arg2[0] == '+') {
			tlimit = ((level.time - level.startTime) / 60000) + tlimit;		// elapsed minutes + 
		} else if(arg2[0] == '-') {
			tlimit = ((level.time - level.startTime) / 60000) - tlimit;		// elapsed minutes -
		}

		if(tlimit < 0)
			tlimit = 0;

		if(tlimit > 999)
			tlimit = 999;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%d\"", arg1, tlimit );

		// ---
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Set Timelimit to %d", tlimit);
	} else if ( !Q_stricmp( arg1, "capturelimit" ) ) {
		int climit = atoi(arg2);

		if(arg2[0] == '+') {
			climit += g_capturelimit.integer;		// number of captures + 
		} else if(arg2[0] == '-') {
			climit -= g_capturelimit.integer;		// number of captures -
		}

		if(climit < 0)
			climit = 0;

		if(climit > INT_MAX-1)
			climit = INT_MAX-1;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%d\"", arg1, climit );

		// ---
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Set Capturelimit to %d", climit);
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
		gentity_t *player;
		qboolean found = qfalse;
		// name = arg2;

		for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ )
		{
			if( player->inuse && (Q_stricmp(player->client->pers.netname, arg2) == 0) )
			{
				if(player->client->sess.adminLevel != ADMIN_NONE)
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick an admin.\n\"" );
					return;
				}

				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick host player\n\"" );
					return;
				}

				found = qtrue;
				break;
			}
			else if( player->client && player->client->pers.connected == CON_CONNECTING && (Q_stricmp(player->client->pers.netname, arg2) == 0) )
			{
				if(player->client->sess.adminLevel != ADMIN_NONE)
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick an admin.\n\"" );
					return;
				}

				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick host player\n\"" );
					return;
				}

				found = qtrue;
				break;
			}
		}

		if(found) {
			if( (player->r.svFlags & SVF_BOT) )
				Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", (int)(player-g_entities));
			else
				Com_sprintf( level.voteString, sizeof( level.voteString ), "addip %s %d \"Vote Kicked\"", player->client->sess.ipStr, Q3F_ADMIN_TEMPBAN_TIME);
			//Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", count);
		}
		else {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid kick name.\n\"" );
			return;
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s", player->client->pers.netname);
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
		int pnum = atoi(arg2);
		gentity_t *player;
		qboolean found = qfalse;

		//Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", pnum);
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(player->inuse && player->client && player->s.number == pnum) {
				if(player->client->sess.adminLevel != ADMIN_NONE)
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick an admin.\n\"" );
					return;
				}

				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick host player\n\"" );
					return;
				}

				found = qtrue;
				break;
			}
			else if(player->client && player->client->pers.connected == CON_CONNECTING && (player-g_entities) == pnum) {
				if(player->client->sess.adminLevel != ADMIN_NONE)
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick an admin.\n\"" );
					return;
				}

				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					trap_SendServerCommand( ent-g_entities, "print \"Cannot kick host player\n\"" );
					return;
				}

				found = qtrue;
				break;
			}
		}

		if(found) {
			if( (player->r.svFlags & SVF_BOT) )
				Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", pnum);
			else
				Com_sprintf( level.voteString, sizeof( level.voteString ), "addip %s %d \"Vote Kicked\"", player->client->sess.ipStr, Q3F_ADMIN_TEMPBAN_TIME);
			//Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick %d", count);
		}
		else {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid clientkick client number.\n\"" );
			return;
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s", player->client->pers.netname);
	} else if ( !Q_stricmp( arg1, "mute" ) ) {
		int pnum = atoi(arg2);
		gentity_t *player;
		qboolean found = qfalse;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "mute %d", pnum);
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != pnum)
				continue;

			if(player->client->sess.adminLevel != ADMIN_NONE)
			{
				trap_SendServerCommand( ent-g_entities, "print \"Cannot mute an admin.\n\"" );
				return;
			}

			if(player->client->sess.muted) {
				trap_SendServerCommand( ent-g_entities, "print \"Player is already muted.\n\"" );
				return;
			}

			found = qtrue;
			break;
		}

		if(!found) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid mute client number.\n\"" );
			return;
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "mute %s", player->client->pers.netname);
	} else if ( !Q_stricmp( arg1, "unmute" ) ) {
		int pnum = atoi(arg2);
		gentity_t *player;
		qboolean found = qfalse;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "unmute %d", pnum);
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != pnum)
				continue;

			if(!player->client->sess.muted) {
				trap_SendServerCommand( ent-g_entities, "print \"Player is not muted.\n\"" );
				return;
			}

			if(ent->client->sess.muted && player != ent && !level.ceaseFire) {
				trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string while muted.\n\"" );
				trap_SendServerCommand( ent-g_entities, "print \"You can only start a vote to unmute yourself.\n\"" );
				return;
			}

			found = qtrue;
			break;
		}

		if(!found) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid unmute client number.\n\"" );
			return;
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "unmute %s", player->client->pers.netname);
	} else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );

		// ---
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s^7 called a vote. Go to the Vote Menu to Vote Yes or No\n\"", ent->client->pers.netname ) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if((level.clients[i].pers.connected == CON_CONNECTED) && (level.clients[i].ps.clientNum != ent->client->ps.clientNum) )
			trap_SendServerCommand( i, "menu vote");
	} 

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%d", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%d", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%d", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}
	if ( Q3F_IsSpectator( ent->client ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || tolower( msg[0] ) == 't' || msg[0] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

/*
=================
G_Q3F_ChannelCommand
=================
*/
static void q3f_cc_preprocess( char *str )
{
	if( *str == '#' )
		strcpy( str, str + 1 );	// 'lose' the leading #

	while( *str )
	{
		if( *str >= 'A' && *str <= 'Z' )
			*str |= 32;
		if( (*str < 'a' || *str > 'z') &&
			(*str < '0' || *str > '9') &&
			*str != '_' )
			*str = '_';
		str++;
	}
}
static void G_Q3F_ChannelCommand( gentity_t *ent )
{
	// Allow the user to specify which channels they want for team messages.

	char chanbuff[64];
	int numargs, curr, index;
	q3f_array_t *array;
	q3f_data_t *data;
	char *str;
	//qboolean usage;

	numargs = trap_Argc();
	//usage = 0;

	trap_Argv( 1, chanbuff, 64 );
	curr = 2;
	array = ent->client->chatchannels;
	if( !Q_stricmp( chanbuff, "add" ) && numargs > 2 )
	{
		// Add new channels
		
		if( !array )
			array = ent->client->chatchannels = G_Q3F_ArrayCreate();
		while( curr < numargs )
		{
			trap_Argv( curr, chanbuff, 64 );
			q3f_cc_preprocess( chanbuff );
			if( !G_Q3F_ArrayFind( array, (int) G_Q3F_GetString( chanbuff ) ) )
			{
				if( array->used >= Q3F_CHANNEL_MAX )
				{
					trap_SendServerCommand( ent->s.number, va( "print \"You can only have %d channels at a time.\n\"", Q3F_CHANNEL_MAX ) );
					break;
				}
				G_Q3F_ArrayAdd( array, Q3F_TYPE_STRING, 0, (int) chanbuff );
			}
			curr++;
		}
		G_Q3F_ArraySort( array );
	}
	else if( !Q_stricmp( chanbuff, "rem" ) && numargs > 2  )
	{
		// Remove channels

		if( array )
		{
			while( curr < numargs )
			{
				trap_Argv( curr, chanbuff, 64 );
				q3f_cc_preprocess( chanbuff );
				str = G_Q3F_GetString( chanbuff );
				if( str )
				{
					index = -1;
					while( (data = G_Q3F_ArrayTraverse( array, &index ) ) != NULL )
					{
						if( data->d.strdata == str )
						{
							G_Q3F_ArrayDel( array, index );
							break;
						}
					}
				}
				curr++;
			}
			if( array->used )
				G_Q3F_ArraySort( array );
			else {
				G_Q3F_ArrayDestroy( array );
				ent->client->chatchannels = NULL;
			}
		}
	}
	else if( !Q_stricmp( chanbuff, "set" ) && numargs > 2 )
	{
		// Set channels (ignore previous)

		G_Q3F_ArrayDestroy( array );
		array = ent->client->chatchannels = G_Q3F_ArrayCreate();

		while( curr < numargs )
		{
			trap_Argv( curr, chanbuff, 64 );
			q3f_cc_preprocess( chanbuff );
			if( !G_Q3F_ArrayFind( array, (int) G_Q3F_GetString( chanbuff ) ) )
			{
				if( array->used >= Q3F_CHANNEL_MAX )
				{
					trap_SendServerCommand( ent->s.number, va( "print \"You can only have %d channels at a time.\n\"", Q3F_CHANNEL_MAX ) );
					break;
				}
				G_Q3F_ArrayAdd( array, Q3F_TYPE_STRING, 0, (int) chanbuff );
			}
			curr++;
		}
		G_Q3F_ArraySort( array );
	}
	else if( !Q_stricmp( chanbuff, "clear" ) )
	{
		// Clear channels
		
		G_Q3F_ArrayDestroy( array );
		ent->client->chatchannels = NULL;
	}
	else if( *chanbuff )
	{
		// Not an 'empty' command

		trap_SendServerCommand( ent-g_entities, va("print \"usage: channel add|rem|set|clear channel1 channel2 ...\n\""));
		return;
	}

	// Now, show the channels we're on.

	if( !ent->client->chatchannels )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Now listening on all channels.\n\""));
		return;
	}

	array = ent->client->chatchannels;
	for( curr = 32, index = -1; (data = G_Q3F_ArrayTraverse( array, &index )) != NULL; )
		curr += 2 + strlen( data->d.strdata );
	str = G_Alloc( curr );
	strcpy( str, "print \"Now listening on:" );
	for( index = -1; (data = G_Q3F_ArrayTraverse( array, &index )) != NULL; )
	{
		Q_strcat( str, curr, " #" );
		Q_strcat( str, curr, data->d.strdata );
	}
	Q_strcat( str, curr, ".\n\"\n" );
	trap_SendServerCommand( ent - g_entities, str );
	G_Free( str );
}

/*
======================
G_Q3F_DiscardCommand
======================
*/

void G_Q3F_DiscardCommand( gentity_t *ent )
{
	// See if there's any discardable ammo, and dump it.

	qboolean shells, nails, rockets, cells;
	int index, temp;
	bg_q3f_playerclass_t *cls;
	playerState_t *ps;
	gentity_t *drop;
	gitem_t *item;
	vec3_t velocity, pvel;
	char buffer[16];
	int cellcount, shellcount, rocketcount;

	if( !ent->client || ent->health <= 0 || level.ceaseFire || ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
		return;

		// Work out what we don't need
	ps = &ent->client->ps;
	cls = BG_Q3F_GetClass( ps );
	shells = nails = rockets = cells = qtrue;
	for( index = 0; index < Q3F_NUM_WEAPONSLOTS; index++ )
	{
		switch( cls->weaponslot[index] )
		{
			case WP_SHOTGUN:			shells = qfalse;	break;
			case WP_SUPERSHOTGUN:		shells = qfalse;	break;
			case WP_NAILGUN:			nails = qfalse;		break;
			case WP_SUPERNAILGUN:		nails = qfalse;		break;
			case WP_GRENADE_LAUNCHER:	rockets = qfalse;	break;
			case WP_ROCKET_LAUNCHER:	rockets = qfalse;	break;
			case WP_SNIPER_RIFLE:		nails = qfalse;		break;
			case WP_RAILGUN:			nails = qfalse;		break;
			case WP_FLAMETHROWER:		cells = qfalse;		break;
			case WP_MINIGUN:			shells = qfalse;	cells = qfalse;	break;
			case WP_ASSAULTRIFLE:		shells = qfalse;	break;
			case WP_DARTGUN:			nails = qfalse;		break;
			case WP_PIPELAUNCHER:		rockets = qfalse;	break;
			case WP_NAPALMCANNON:		rockets = qfalse;	break;
		}
	}

	if( ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT )
		cells = qfalse;
	else if( ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER )
		shells = cells = qfalse;
	else if( ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_RECON )
		cells = qfalse;

	trap_Argv(1, buffer, 16);
	if(*buffer) {
		cellcount = atoi(buffer);
	} else {
		cellcount = -1;
	}

	trap_Argv(2, buffer, 16);
	if(*buffer) {
		shellcount = atoi(buffer);
	} else {
		shellcount = -1;
	}

	trap_Argv(3, buffer, 16);
	if(*buffer) {
		rocketcount = atoi(buffer);
	} else {
		rocketcount = -1;
	}

	temp = 0;

	if(cellcount >= 0) {
		index = ent->client->ps.weapon;
/*		if( index == WP_FLAMETHROWER )
			temp = 50;
		else if( index == WP_MINIGUN )
			temp = 20;*/

		cellcount = ps->ammo[AMMO_CELLS] - temp - cellcount;

		if(cellcount > 0) {
			cells = qtrue;
		} else {
			cells = qfalse;
		}
	} else {
		cellcount = ps->ammo[AMMO_CELLS];
	}

	temp = 0;

	if(shellcount >= 0) {
		if( (index = Q3F_GetClipValue( WP_SHOTGUN, ps )) > temp )
			temp = index;
		if( (index = Q3F_GetClipValue( WP_SUPERSHOTGUN, ps )) > temp )
			temp = index;
/*		if( ps->weapon == WP_MINIGUN && temp < 50 )
			temp = 50;*/

		shellcount = ps->ammo[AMMO_SHELLS] - temp - shellcount;

		if(shellcount > 0) {
			shells = qtrue;
		} else {
			shells = qfalse;
		}
	} else {
		shellcount = ps->ammo[AMMO_SHELLS];
	}

	temp = 0;

	if(rocketcount >= 0) {
		if( (index = Q3F_GetClipValue( WP_ROCKET_LAUNCHER, ps )) > temp )
			temp = index;
		if( (index = Q3F_GetClipValue( WP_GRENADE_LAUNCHER, ps )) > temp )
			temp = index;
		if( (index = Q3F_GetClipValue( WP_PIPELAUNCHER, ps )) > temp )
			temp = index;
/*		if( ps->weapon == WP_NAPALMCANNON && temp < 18 )
			temp = 18;*/
		rocketcount = ps->ammo[AMMO_ROCKETS] - temp - rocketcount;
		
		if(rocketcount > 0) {
			rockets = qtrue;
		} else {
			rockets = qfalse;
		}
	} else {
		rocketcount = ps->ammo[AMMO_ROCKETS];
	}

		// See if we have anything to dump
	if(	(shells &&	ps->ammo[AMMO_SHELLS]) ||
		(nails &&	ps->ammo[AMMO_NAILS]) ||
		(rockets &&	ps->ammo[AMMO_ROCKETS]) ||
		(cells &&	ps->ammo[AMMO_CELLS]) )
	{
		// Spawn a backpack and dump it
		item = BG_FindItem( "Backpack" );

		VectorCopy( ent->client->ps.viewangles, velocity );
		velocity[PITCH] = 0;
		//AngleVectors( velocity, velocity, NULL, NULL );
		AngleVectors( ent->client->ps.viewangles, velocity, NULL, NULL );
		VectorScale( velocity, 400, velocity );
		VectorScale( ent->client->ps.velocity, 0.25, pvel );
		VectorAdd( pvel, velocity, velocity ); // RR2DO2: add player velocity
		velocity[2] += 100 + Q_flrand(-1.0f, 1.0f) * 50;

		drop = LaunchItem( item, ent->r.currentOrigin, velocity );
		drop->activator = ent;
		if( shells )
		{
			drop->s.time2			= shellcount;
			ps->ammo[AMMO_SHELLS]	-= shellcount;
		}
		if( nails )
		{
			drop->s.legsAnim		= ps->ammo[AMMO_NAILS];
			ps->ammo[AMMO_NAILS]	= 0;
		}
		if( rockets )
		{
			drop->s.torsoAnim		= rocketcount;
			ps->ammo[AMMO_ROCKETS]	-= rocketcount;
		}
		if( cells )
		{
			drop->s.weapon			= cellcount;
			ps->ammo[AMMO_CELLS]	-= cellcount;
		}
		drop->flags			|= FL_DROPPED_ITEM;
		drop->s.angles2[0]	= 1;		// Mark it as a dropped item;
		drop->s.time = level.time;
		drop->r.ownerNum = ent->s.number;
		drop->s.otherEntityNum = ent->s.number;

		G_AddEvent( ent, EV_ETF_DISCARD_AMMO, 0 );

		// throw animation
		if( !(ent->client->ps.extFlags & EXTF_ANI_THROWING) ) {
			ent->client->ps.extFlags |= EXTF_ANI_THROWING;
			ent->client->torsoanimEndTime = level.time + Q3F_THROW_ANIM_DURATION;
		}
	}
//	else {
//		trap_SendServerCommand( ent->s.number, "print \"You don't have any useless ammo to discard.\n\"" );
//	}
}

void G_Q3F_DropAmmoToCommand( gentity_t *ent ) {
	// See if there's any discardable ammo, and dump it.

	char buff[16];
	int index, type, temp, inclip, total;
	int count = -1;
	//bg_q3f_playerclass_t *cls;
	playerState_t *ps;
	gentity_t *drop;
	gitem_t *item;
	vec3_t velocity, pvel;
	char *ammoname;

	if( !ent->client || ent->health <= 0 || level.ceaseFire || ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
		return;

	if( ent->client->NextAmmoBoxDropTime > level.time ) {
		return;
	}

		// Work out what they actually want.
	index = 1;
	type = 0;
	while( index < trap_Argc() )
	{
		trap_Argv( index, buff, sizeof(buff) );
		if( index == 1 )
		{
			// It's a count of ammo to drop
			count = atoi( buff );
		}
		else {
					if(	!Q_stricmp( buff, "shell" ) ||
						!Q_stricmp( buff, "shells" ) )
				type = 1;
			else if(	!Q_stricmp( buff, "nail" ) ||
						!Q_stricmp( buff, "nails" ) )
				type = 2;
			else if(	!Q_stricmp( buff, "rocket" ) ||
						!Q_stricmp( buff, "rockets" ) )
				type = 3;
			else if(	!Q_stricmp( buff, "cell" ) ||
						!Q_stricmp( buff, "cells" ) )
				type = 4;
		}

		index++;
	}
	if( !type || count == -1)
	{
		trap_SendServerCommand( ent->s.number, "print \"Usage: dropammoto <count> <type>\n\"" );
		return;
	}

	// Find the amount of ammo to drop
	ps = &ent->client->ps;
	//cls = BG_Q3F_GetClass( ps );

	// Check we don't drop more ammo than we have
	switch( type )
	{
		case 1:		temp = ps->ammo[AMMO_SHELLS]; inclip = Q3F_GetAmmoNumInClip(AMMO_SHELLS, ps); break;
		case 2:		temp = ps->ammo[AMMO_NAILS]; inclip = Q3F_GetAmmoNumInClip(AMMO_NAILS, ps);	break;
		case 3:		temp = ps->ammo[AMMO_ROCKETS]; inclip = Q3F_GetAmmoNumInClip(AMMO_ROCKETS, ps);	break;
		case 4:		temp = ps->ammo[AMMO_CELLS]; inclip = Q3F_GetAmmoNumInClip(AMMO_CELLS, ps); break;
	}

	if(temp - inclip <= count)
	{
		trap_SendServerCommand( ent->s.number, "print \"Sorry, you have no ammo to drop.\n\"" );
		return;
	}

	// Spawn an ammo box and dump it
	switch( type )
	{
		case 1:	ammoname = "Ammo (Shells)";		break;
		case 2:	ammoname = "Ammo (Nails)";		break;
		case 3:	ammoname = "Ammo (Rockets)";	break;
		case 4:	ammoname = "Ammo (Cells)";		break;
	}
	
	item = BG_FindItem( ammoname );
	if( !item )
		G_Error( "Unable to find item '%s'.", ammoname );

	VectorCopy( ent->client->ps.viewangles, velocity );
	velocity[PITCH] = 0;
	//AngleVectors( velocity, velocity, NULL, NULL );
	AngleVectors( ent->client->ps.viewangles, velocity, NULL, NULL );
	VectorScale( velocity, 400, velocity );
	VectorScale( ent->client->ps.velocity, 0.25, pvel );
	VectorAdd( pvel, velocity, velocity ); // RR2DO2: add player velocity
	velocity[2] += 100 + Q_flrand(-1.0f, 1.0f) * 50;

	// allocate a new ammobox slot, if needed, free the other one
	for ( index = 0; index < Q3F_MAX_AMMOBOXES; index++ ) {
		if ( !ent->client->pers.AmmoBoxes[index] )
			break;
	}
	if ( index == Q3F_MAX_AMMOBOXES ) {
		// we have no null field, so delete the oldest ammo box
		G_Q3F_FreeAmmoBox( ent->client->pers.AmmoBoxes[0] );
		index = Q3F_MAX_AMMOBOXES - 1;
	}

	drop = LaunchItem( item, ent->r.currentOrigin, velocity );

	drop->activator = ent;
	total = temp - inclip - count;
	switch( type ) {
		case 1:	drop->s.time2			=	total; ps->ammo[AMMO_SHELLS]	-=	total; break;
		case 2:	drop->s.legsAnim		=	total; ps->ammo[AMMO_NAILS]		-=	total; break;
		case 3:	drop->s.torsoAnim		=	total; ps->ammo[AMMO_ROCKETS]	-=	total; break;
		case 4:	drop->s.weapon			=	total; ps->ammo[AMMO_CELLS]		-=	total; break;
	}
	drop->free = G_Q3F_FreeAmmoBox;
	drop->flags			|= FL_DROPPED_ITEM;
	drop->s.angles2[0]	= 2;		// Mark it as an ammobox;
	drop->s.time = level.time;
	drop->r.ownerNum = ent->s.number;
	drop->s.otherEntityNum = ent->s.number;

	// store the box data
	ent->client->pers.AmmoBoxes[index] = drop;

	ent->client->NextAmmoBoxDropTime = level.time + Q3F_AMMOBOX_DELAY;

	G_AddEvent( ent, EV_ETF_DISCARD_AMMO, 0 );

	// throw animation
	if( !(ent->client->ps.extFlags & EXTF_ANI_THROWING) ) {
		ent->client->ps.extFlags |= EXTF_ANI_THROWING;
		ent->client->torsoanimEndTime = level.time + Q3F_THROW_ANIM_DURATION;
	}
}

/*
======================
G_Q3F_DropAmmoCommand
======================
*/

void G_Q3F_DropAmmoCommand( gentity_t *ent )
{
	// See if there's any discardable ammo, and dump it.

	char buff[16];
	int index, type, count, temp, inclip, avail_cells;
	float conv_fact;
	//bg_q3f_playerclass_t *cls;
	playerState_t *ps;
	gentity_t *drop;
	gitem_t *item;
	vec3_t velocity, pvel;
	char *ammoname;

	if( !ent->client || ent->health <= 0 || level.ceaseFire || ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
		return;

	if( ent->client->NextAmmoBoxDropTime > level.time ) {
		return;
	}

		// Work out what they actually want.
	count = 0;
	index = 1;
	type = 0;
	avail_cells = 0;
	while( index < trap_Argc() )
	{
		trap_Argv( index, buff, sizeof(buff) );
		index++;
		temp = atoi( buff );
		if( temp && temp >= 0 )
		{
			// It's a count of ammo to drop
			count = temp;
		}
		else {
			// It's the type of ammo to drop, or a menu or 'all' command
			if( !Q_stricmp( buff, "menu" ) )
				type = -1;
			else if( !Q_stricmp( buff, "all" ) )
				//count = 1000;
				count = 0; // Stop "dropammo all <type>" from doing bad hacky things

			else if(	!Q_stricmp( buff, "shell" ) ||
						!Q_stricmp( buff, "shells" ) )
				type = 1;
			else if(	!Q_stricmp( buff, "nail" ) ||
						!Q_stricmp( buff, "nails" ) )
				type = 2;
			else if(	!Q_stricmp( buff, "rocket" ) ||
						!Q_stricmp( buff, "rockets" ) )
				type = 3;
			else if(	!Q_stricmp( buff, "cell" ) ||
						!Q_stricmp( buff, "cells" ) )
				type = 4;
		}
	}
	if( !type )
	{
		trap_SendServerCommand( ent->s.number, "print \"Usage: dropammo <count> <type>\n\"" );
		return;
	}
	if( type == -1 )
	{
		trap_SendServerCommand( ent->s.number, "menu dropammo" );
		return;
	}

		// Find the amount of ammo to drop
	ps = &ent->client->ps;
	//cls = BG_Q3F_GetClass( ps );
	if( !count )
	{
		// Automatically obtain ammo to drop, without dropping ammo in clip,
		// or a small amount of ammo for current weapon.

		temp = 0;
		switch( type )
		{
			case 1:		if( (index = Q3F_GetClipValue( WP_SHOTGUN, ps )) > temp )
							temp = index;
						if( (index = Q3F_GetClipValue( WP_SUPERSHOTGUN, ps )) > temp )
							temp = index;
						if( ps->weapon == WP_MINIGUN && temp < 50 )
							temp = 50;
						count = ps->ammo[AMMO_SHELLS] - temp;
						break;
			case 2:		index = ps->weapon;
						if( index == WP_NAILGUN || index == WP_SUPERNAILGUN || index == WP_ASSAULTRIFLE )
							temp = 50;
						else if( index == WP_RAILGUN || index == WP_DARTGUN || index == WP_SNIPER_RIFLE )
							temp = 10;
						count = ps->ammo[AMMO_NAILS] - temp;
						break;
			case 3:		if( (index = Q3F_GetClipValue( WP_ROCKET_LAUNCHER, ps )) > temp )
							temp = index;
						if( (index = Q3F_GetClipValue( WP_GRENADE_LAUNCHER, ps )) > temp )
							temp = index;
						if( (index = Q3F_GetClipValue( WP_PIPELAUNCHER, ps )) > temp )
							temp = index;
						if( ps->weapon == WP_NAPALMCANNON && temp < 18 )
							temp = 18;
						count = ps->ammo[AMMO_ROCKETS] - temp;
						break;
			case 4:		index = ps->weapon;
						if( index == WP_FLAMETHROWER )
							temp = 50;
						else if( index == WP_MINIGUN )
							temp = 20;
						count = ps->ammo[AMMO_CELLS] - temp;
						break;
		}
		if( count <= 0 )
		{
			trap_SendServerCommand( ent->s.number, "print \"Sorry, you have no free ammo to drop.\n\"" );
			return;
		}
	}
	else {
		// Check we don't drop more ammo than we have

		switch( type )
		{
			case 1:		temp = ps->ammo[AMMO_SHELLS]; inclip = Q3F_GetAmmoNumInClip(AMMO_SHELLS, ps); avail_cells = ps->ammo[AMMO_CELLS] - Q3F_GetAmmoNumInClip(AMMO_CELLS, ps);	break;
			case 2:		temp = ps->ammo[AMMO_NAILS]; inclip = Q3F_GetAmmoNumInClip(AMMO_NAILS, ps); avail_cells = ps->ammo[AMMO_CELLS] - Q3F_GetAmmoNumInClip(AMMO_CELLS, ps);	break;
			case 3:		temp = ps->ammo[AMMO_ROCKETS]; inclip = Q3F_GetAmmoNumInClip(AMMO_ROCKETS, ps); avail_cells = ps->ammo[AMMO_CELLS] - Q3F_GetAmmoNumInClip(AMMO_CELLS, ps);	break;
			case 4:		temp = ps->ammo[AMMO_CELLS]; inclip = Q3F_GetAmmoNumInClip(AMMO_CELLS, ps);	avail_cells = 0; break;
		}

		if ( ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER && avail_cells ) {
			if ( count > temp - inclip ) {
				// try to make some ammo out of cells
				switch( type )
				{
					case 1:	conv_fact = 1.f / 3.f; break;
					case 2:	conv_fact = 0.5f; break;
					case 3:	conv_fact = 0.2f; break;
					//case 4:		break; // no need for this, will never be reached.
				}

				if ( count > temp+(avail_cells*conv_fact)-inclip )
					count = temp+(avail_cells*conv_fact)-inclip;
				else {
					avail_cells = (count-(temp-inclip)) / conv_fact;
				}
			} else {
				avail_cells = 0;
			}
		} else {
			if( count > temp - inclip )
				count = temp - inclip;
			avail_cells = 0;
		}

		if( count <= 0 )
		{
			trap_SendServerCommand( ent->s.number, "print \"Sorry, you have no ammo to drop.\n\"" );
			return;
		}
	}

	// Spawn an ammo box and dump it
	switch( type )
	{
		case 1:	ammoname = "Ammo (Shells)";		break;
		case 2:	ammoname = "Ammo (Nails)";		break;
		case 3:	ammoname = "Ammo (Rockets)";	break;
		case 4:	ammoname = "Ammo (Cells)";		break;
	}
	
	item = BG_FindItem( ammoname );
	if( !item )
		G_Error( "Unable to find item '%s'.", ammoname );

	VectorCopy( ent->client->ps.viewangles, velocity );
	velocity[PITCH] = 0;
	//AngleVectors( velocity, velocity, NULL, NULL );
	AngleVectors( ent->client->ps.viewangles, velocity, NULL, NULL );
	VectorScale( velocity, 400, velocity );
	VectorScale( ent->client->ps.velocity, 0.25, pvel );
	VectorAdd( pvel, velocity, velocity ); // RR2DO2: add player velocity
	velocity[2] += 100 + Q_flrand(-1.0f, 1.0f) * 50;

	// allocate a new ammobox slot, if needed, free the other one
	for ( index = 0; index < Q3F_MAX_AMMOBOXES; index++ ) {
		if ( !ent->client->pers.AmmoBoxes[index] )
			break;
	}
	if ( index == Q3F_MAX_AMMOBOXES ) {
		// we have no null field, so delete the oldest ammo box
		G_Q3F_FreeAmmoBox( ent->client->pers.AmmoBoxes[0] );
		index = Q3F_MAX_AMMOBOXES - 1;
	}

	drop = LaunchItem( item, ent->r.currentOrigin, velocity );
	drop->activator = ent;
	switch( type )
	{
		case 1:	drop->s.time2			=	count;
				if (avail_cells) {
					ps->ammo[AMMO_CELLS]	-=	avail_cells;
					count					-=	avail_cells*conv_fact;
				}
				ps->ammo[AMMO_SHELLS]	-=	count;
				break;
		case 2:	drop->s.legsAnim		=	count;
				if (avail_cells) {
					ps->ammo[AMMO_CELLS]	-=	avail_cells;
					count					-=	avail_cells*conv_fact;
				}
				ps->ammo[AMMO_NAILS]	-=	count;
				break;
		case 3:	drop->s.torsoAnim		=	count;
				if (avail_cells) {
					ps->ammo[AMMO_CELLS]	-=	avail_cells;
					count					-=	avail_cells*conv_fact;
				}
				ps->ammo[AMMO_ROCKETS]	-=	count;
				break;
		case 4:	drop->s.weapon			=	count;
				ps->ammo[AMMO_CELLS]	-=	count;
				break;
	}
	drop->free = G_Q3F_FreeAmmoBox;
	drop->flags			|= FL_DROPPED_ITEM;
	drop->s.angles2[0]	= 2;		// Mark it as an ammobox;
	drop->s.time = level.time;
	drop->r.ownerNum = ent->s.number;
	drop->s.otherEntityNum = ent->s.number;
	drop->freeAfterEvent = qfalse;

	// store the box data
	ent->client->pers.AmmoBoxes[index] = drop;

	ent->client->NextAmmoBoxDropTime = level.time + Q3F_AMMOBOX_DELAY;

	G_AddEvent( ent, EV_ETF_DISCARD_AMMO, 0 );

	// throw animation
	if( !(ent->client->ps.extFlags & EXTF_ANI_THROWING) ) {
		ent->client->ps.extFlags |= EXTF_ANI_THROWING;
		ent->client->torsoanimEndTime = level.time + Q3F_THROW_ANIM_DURATION;
	}
}

/*
======================
G_Q3F_SavemeCommand
======================
*/

void G_Q3F_SavemeCommand( gentity_t *ent )
{
	// Call for a paramedic. If there's no paramedics around,
	// Or they're nearby, just call 'medic'. Otherwise,
	// randomly play a longer sample.

	float mediclen, currlen;
	gentity_t *player, *te;
	int soundindex;

	if( !ent->client || ent->health <= 0 )
		return;
	if( ent->client->callTime )
		return;

	mediclen = 5000;

	for( player = g_entities; player < &g_entities[level.maxclients]; player++ )
	{
		if(	player->inuse &&
			player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC &&
			player->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			player != ent && 
			trap_InPVS( ent->r.currentOrigin, player->r.currentOrigin ) )
		{
			currlen = Distance( ent->r.currentOrigin, player->r.currentOrigin );
			if( currlen < mediclen )
				mediclen = currlen;
		}
	}

	if( mediclen <= 200 || mediclen >= 5000 || (rand() & 3) > 0 )
	{
		soundindex = G_SoundIndex( "sound/player/medic1.wav" );
		ent->client->callTime = level.time + 2000;
	}
	else {
		if( rand() & 1 )
			soundindex = G_SoundIndex( "sound/player/medic2.wav" );
		else 
			soundindex = G_SoundIndex( "sound/player/medic3.wav" );
		ent->client->callTime = level.time + 5000;
	}

#ifdef BUILD_BOTS
	Bot_Event_MedicCall(ent);
#endif
	ent->client->ps.eFlags	|= EF_Q3F_SAVEME;	// Assumes call flags have been reset
	ent->s.eFlags			|= EF_Q3F_SAVEME;	// Should really be a PlayerStateToEntityState call

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundindex;
}

/*
======================
G_Q3F_ArmormeCommand
======================
*/

void G_Q3F_ArmormeCommand( gentity_t *ent )
{
	// Call for an engineer. If there's no engineers around,
	// Or they're nearby, just call 'engineer'. Otherwise,
	// randomly play a longer sample.

	float englen, currlen;
	gentity_t *player, *te;
	int soundindex;

	if( !ent->client || ent->health <= 0 )
		return;
	if( ent->client->callTime )
		return;

	englen = 5000;

	for( player = g_entities; player < &g_entities[level.maxclients]; player++ )
	{
		if(	player->inuse &&
			player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER &&
			player->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			player != ent && 
			trap_InPVS( ent->r.currentOrigin, player->r.currentOrigin ) )
		{
			currlen = Distance( ent->r.currentOrigin, player->r.currentOrigin );
			if( currlen < englen )
				englen = currlen;
		}
	}

	if( englen <= 200 || englen >= 5000 || (rand() & 3) > 0 )
	{
		soundindex = G_SoundIndex( "sound/player/engineer2.wav" );
		ent->client->callTime = level.time + 2000;
	}
	else 
	{
		ent->client->callTime = level.time + 5000;
		if( rand() & 1 )
			soundindex = G_SoundIndex( "sound/player/engineer1.wav" );
		else 
			soundindex = G_SoundIndex( "sound/player/engineer3.wav" );
	}

#ifdef BUILD_BOTS
	Bot_Event_EngineerCall(ent);
#endif
	ent->client->ps.eFlags	|= EF_Q3F_ARMORME;	// Assumes call flags have been reset
	ent->s.eFlags			|= EF_Q3F_ARMORME;	// Should really be a PlayerStateToEntityState call

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundindex;
}

/*
======================
G_Q3F_ArmormeCommand
======================
*/

void G_Q3F_AmmomeCommand( gentity_t *ent )
{
	// Call for an engineer. If there's no engineers around,
	// Or they're nearby, just call 'engineer'. Otherwise,
	// randomly play a longer sample.

	float englen, currlen;
	gentity_t *player, *te;
	int soundindex;

	if( !ent->client || ent->health <= 0 )
		return;
	if( ent->client->callTime )
		return;

	englen = 5000;

	for( player = g_entities; player < &g_entities[level.maxclients]; player++ )
	{
		if(	player->inuse &&
			player->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER &&
			player->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			player != ent && 
			trap_InPVS( ent->r.currentOrigin, player->r.currentOrigin ) )
		{
			currlen = Distance( ent->r.currentOrigin, player->r.currentOrigin );
			if( currlen < englen )
				englen = currlen;
		}
	}

	if( englen <= 200 || englen >= 5000 || (rand() & 3) > 0 )
	{
		soundindex = G_SoundIndex( "sound/player/engineer2.wav" );
		ent->client->callTime = level.time + 2000;
	}
	else 
	{
		ent->client->callTime = level.time + 5000;
		if( rand() & 1 )
			soundindex = G_SoundIndex( "sound/player/engineer1.wav" );
		else 
			soundindex = G_SoundIndex( "sound/player/engineer3.wav" );
	}

#ifdef BUILD_BOTS
	Bot_Event_EngineerCall(ent);
#endif
	ent->client->ps.eFlags	|= EF_Q3F_ARMORME;	// Assumes call flags have been reset
	ent->s.eFlags			|= EF_Q3F_ARMORME;	// Should really be a PlayerStateToEntityState call

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundindex;
}

qboolean G_Q3F_SpecialCommand( gentity_t *ent )
{
	if(ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR && (ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_CHASE))
	{
		if(ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
		{
			Cmd_FollowCycle_f(ent, -1, SPECTATOR_FOLLOW);
		}
		else if(ent->client->sess.spectatorState == SPECTATOR_CHASE)
		{
			Cmd_FollowCycle_f(ent, -1, SPECTATOR_CHASE);
		}
		return (qtrue);
	}

	switch(ent->client->ps.persistant[PERS_CURRCLASS])
	{
		case Q3F_CLASS_RECON:	
								if(level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE])
									return( qtrue );
								if(Q3F_IsSpectator(ent->client))
									return( qtrue );
								G_Q3F_ToggleScanner( ent );
								return( qtrue );
		case Q3F_CLASS_SNIPER:
			// TODO: AutoZoom
								return( qfalse );
		case Q3F_CLASS_SOLDIER:	BG_Q3F_Request_Reload(&ent->client->ps);
								return( qtrue );
//		case Q3F_CLASS_GRENADIER:
//			trap_SendServerCommand( ent->s.number, "menu charge" );
//								return( qtrue );
		case Q3F_CLASS_AGENT:	trap_SendServerCommand( ent->s.number, "menu disguise" );
								return( qtrue );
		case Q3F_CLASS_ENGINEER:
								if( ent->client->sentry && G_Q3F_SentryCancel( ent->client->sentry ) ) {
									trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
									return( qtrue );
								}
								else if( ent->client->supplystation && G_Q3F_SupplyStationCancel( ent->client->supplystation ) ) {
									trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
									return( qtrue );
								}
								else {
									// see if we're able to hit a sentry
									vec3_t		muzzle, forward, end;
									trace_t		tr;
									gentity_t	*traceEnt;
									CalcMuzzlePoint ( ent,  muzzle, forward );
									VectorMA (muzzle, 55, forward, end);
									G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
									if ( tr.surfaceFlags & SURF_NOIMPACT ) {
										trap_SendServerCommand( ent->s.number, "menu build" );
										return( qtrue );
									}
									traceEnt = &g_entities[ tr.entityNum ];
									if( traceEnt->s.eType == ET_Q3F_SENTRY && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) ) {
										trap_SendServerCommand(	ent->s.number, va( "menu upgradeautosentry %d", traceEnt->s.number ) );
										ent->client->repairEnt = traceEnt;
										ent->client->repairTime = level.time;
									} else if( traceEnt->s.eType == ET_Q3F_SUPPLYSTATION && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) ) {
										trap_SendServerCommand(	ent->s.number, va( "menu upgradesupplystation %d", traceEnt->s.number ) );
										ent->client->repairEnt = traceEnt;
										ent->client->repairTime = level.time;
									} else {
										trap_SendServerCommand( ent->s.number, "menu build" );
									}
									return( qtrue );
								}
		default:				return( qfalse );
	}
}

/*
=================
G_Q3F_Flyby
=================
*/
static void G_Q3F_Flyby( gentity_t *ent ) {
//	vec3_t vec_angle;
	char strbuff[16];	

	if( level.flybyPathIndex == -1 || !ent->client )
		return;

	trap_Argv( 1, strbuff, 16 );

	if( !Q_stricmp( strbuff, "start" ) ) {
		vec3_t /*vec_angle,*/ next_origin;
		int clienttime;

		trap_Argv( 2, strbuff, 16 );
		clienttime = atoi( strbuff );
	
		SetTeam( ent, "s" ); // join the spectators

		ent->client->sess.spectatorState = SPECTATOR_FLYBY;

		/*ent->client->currtrajindex++;
		if ( ent->client->currtrajindex >= level.campaths[level.flybyPathIndex].numsplines )
			ent->client->currtrajindex = 0;*/
		ent->client->currtrajindex = 0;
		VectorCopy( level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex].SegmentVtx[0], ent->client->camtraj.trBase );
		ent->client->camtraj.trDuration = BG_Q3F_CubicSpline_Length(&level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex])/level.campaths[level.flybyPathIndex].camsplines[ent->client->currtrajindex].speed * 1000;
		//ent->client->camtraj.trTime = level.time;
		ent->client->camtraj.trTime = clienttime;
		ent->client->camtraj.trType = TR_CUBIC_SPLINE_PATH;

		// Origin
		VectorCopy( ent->client->camtraj.trBase, ent->client->ps.origin );
		//VectorCopy( ent->client->camtraj.trBase, ent->client->oldCamPos );
		BG_Q3F_EvaluateSplineTrajectory( &ent->client->camtraj, NULL, &level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex], level.time + FRAMETIME, next_origin );
		VectorSubtract( next_origin, ent->client->ps.origin, ent->client->ps.velocity );	

		// Angles
		/*VectorSet( vec_angle, 200000, 200000, 200000 );
		if( !VectorCompare( level.campaths[level.flybyPathIndex].camsplines[ent->client->currtrajindex].lookat, vec_angle ) ) {
			VectorSubtract( level.campaths[level.flybyPathIndex].camsplines[ent->client->currtrajindex].lookat, level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex].ControlPoint[0], vec_angle );
		} else {
			VectorSubtract ( level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex].ControlPoint[1], level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex].ControlPoint[0], vec_angle );
		}
		vectoangles( vec_angle, vec_angle );
		vec_angle[ROLL] = level.campaths[level.flybyPathIndex].camsplines[ent->client->currtrajindex].roll;

		SetClientViewAngle( ent, vec_angle );*/

		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

		ent->client->inFlyBy = qtrue;
	} else if ( !Q_stricmp( strbuff, "stop" ) ) {
		SetTeam( ent, "s" ); // join the spectators

		ent->client->sess.spectatorState = SPECTATOR_FREE;

		ent->client->inFlyBy = qfalse;
	} else if ( !Q_stricmp( strbuff, "nexttraj" ) && ent->client->sess.spectatorState == SPECTATOR_FLYBY ) {
		vec3_t next_origin;
		int clienttime;

		trap_Argv( 2, strbuff, 16 );
		clienttime = atoi( strbuff );

		ent->client->currtrajindex++;
		if ( ent->client->currtrajindex >= level.campaths[level.flybyPathIndex].numsplines )
				ent->client->currtrajindex = 0;;
		VectorCopy( level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex].SegmentVtx[0], ent->client->camtraj.trBase );
		ent->client->camtraj.trDuration = BG_Q3F_CubicSpline_Length(&level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex])/level.campaths[level.flybyPathIndex].camsplines[ent->client->currtrajindex].speed * 1000;
		//ent->client->camtraj.trTime = level.time;
		ent->client->camtraj.trTime = clienttime;
		ent->client->camtraj.trType = TR_CUBIC_SPLINE_PATH;

		// Origin
		VectorCopy( ent->client->camtraj.trBase, ent->client->ps.origin );
		//VectorCopy( ent->client->camtraj.trBase, ent->client->oldCamPos );
		BG_Q3F_EvaluateSplineTrajectory( &ent->client->camtraj, NULL, &level.campaths[level.flybyPathIndex].splines[ent->client->currtrajindex], level.time + FRAMETIME, next_origin );
		VectorSubtract( next_origin, ent->client->ps.origin, ent->client->ps.velocity );	

		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
	}
}

/*
=================
G_Q3F_DumpServer

Dumps a summary of all server entity values to a text file

 FIXME: make this buffer size safe someday
=================
*/
/*static void FS_printf( fileHandle_t *fh, char *format, ... ) {
	va_list		argptr;
	static char		string[2][32000];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;

	buf = string[index & 1];
	index++;

	va_start (argptr, format);
	vsprintf (buf, format,argptr);
	va_end (argptr);

	trap_FS_Write( buf, strlen( buf ), *fh );
}*/

static void G_Q3F_DumpServer( gentity_t *ent ) {
	char buff[MAX_QPATH];
	fileHandle_t fh;

	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: dumpserver <filename>\n\""));
		return;
	}

	trap_Argv( 1, buff, sizeof( buff ) );
	if( trap_FS_FOpenFile( buff, &fh, FS_WRITE ) >= 0 ) {
		trap_FS_Write( &g_entities, sizeof( g_entities ), fh );
		/*gentity_t *sv_ent;
		
		for( sv_ent = g_entities; sv_ent < &g_entities[level.num_entities]; sv_ent++ ) {
			FS_printf( &fh, "\nEntity %i\n", sv_ent->s.number );
			FS_printf( &fh, "+ s\n" );
			FS_printf( &fh, "| + number: %i\n", sv_ent->s.number );
			FS_printf( &fh, "| + eType: %i\n", sv_ent->s.eType );
			FS_printf( &fh, "| + eFlags: %i\n", sv_ent->s.eFlags );
			FS_printf( &fh, "| + pos\n" );
			FS_printf( &fh, "| | + trType: %i\n", sv_ent->s.pos.trType );
			FS_printf( &fh, "| | + trTime: %i\n", sv_ent->s.pos.trTime );
			FS_printf( &fh, "| | + trDuration: %i\n", sv_ent->s.pos.trDuration );
			FS_printf( &fh, "| | + trBase: %f %f %f\n", sv_ent->s.pos.trBase[0], sv_ent->s.pos.trBase[1], sv_ent->s.pos.trBase[2] );
			FS_printf( &fh, "| | + trDelta: %f %f %f\n", sv_ent->s.pos.trDelta[0], sv_ent->s.pos.trDelta[1], sv_ent->s.pos.trDelta[2] );
			FS_printf( &fh, "| + apos\n" );
			FS_printf( &fh, "| | + trType: %i\n", sv_ent->s.apos.trType );
			FS_printf( &fh, "| | + trTime: %i\n", sv_ent->s.apos.trTime );
			FS_printf( &fh, "| | + trDuration: %i\n", sv_ent->s.apos.trDuration );
			FS_printf( &fh, "| | + trBase: %f %f %f\n", sv_ent->s.apos.trBase[0], sv_ent->s.apos.trBase[1], sv_ent->s.apos.trBase[2] );
			FS_printf( &fh, "| | + trDelta: %f %f %f\n", sv_ent->s.apos.trDelta[0], sv_ent->s.apos.trDelta[1], sv_ent->s.apos.trDelta[2] );
			FS_printf( &fh, "| + time: %i\n", sv_ent->s.time );
			FS_printf( &fh, "| + time2: %i\n", sv_ent->s.time2 );
			FS_printf( &fh, "| + origin: %f %f %f\n", sv_ent->s.origin[0], sv_ent->s.origin[1], sv_ent->s.origin[2] );
			FS_printf( &fh, "| + origin2: %f %f %f\n", sv_ent->s.origin2[0], sv_ent->s.origin2[1], sv_ent->s.origin2[2] );
			FS_printf( &fh, "| + angles: %f %f %f\n", sv_ent->s.angles[0], sv_ent->s.angles[1], sv_ent->s.angles[2] );
			FS_printf( &fh, "| + angles2: %f %f %f\n", sv_ent->s.angles2[0], sv_ent->s.angles2[1], sv_ent->s.angles2[2] );
			FS_printf( &fh, "| + otherEntityNum: %i\n", sv_ent->s.otherEntityNum );
			FS_printf( &fh, "| + otherEntityNum2: %i\n", sv_ent->s.otherEntityNum2 );
			FS_printf( &fh, "| + groundEntityNum: %i\n", sv_ent->s.groundEntityNum );
			FS_printf( &fh, "| + constantLight: %i\n", sv_ent->s.constantLight );
			FS_printf( &fh, "| + loopSound: %i\n", sv_ent->s.loopSound );
			FS_printf( &fh, "| + modelindex: %i\n", sv_ent->s.modelindex );
			FS_printf( &fh, "| + modelindex2: %i\n", sv_ent->s.modelindex2 );
			FS_printf( &fh, "| + clientNum: %i\n", sv_ent->s.clientNum );
			FS_printf( &fh, "| + frame: %i\n", sv_ent->s.frame );
			FS_printf( &fh, "| + solid: %i\n", sv_ent->s.solid );
			FS_printf( &fh, "| + event: %i\n", sv_ent->s.event );
			FS_printf( &fh, "| + eventParm: %i\n", sv_ent->s.eventParm );
			FS_printf( &fh, "| + powerups: %i\n", sv_ent->s.powerups );
			FS_printf( &fh, "| + weapon: %i\n", sv_ent->s.weapon );
			FS_printf( &fh, "| + legsAnim: %i\n", sv_ent->s.legsAnim );
			FS_printf( &fh, "| + powerups: %i\n", sv_ent->s.powerups );
			FS_printf( &fh, "| + powerups: %i\n", sv_ent->s.powerups );
			FS_printf( &fh, "| + torsoAnim: %i\n", sv_ent->s.torsoAnim );
			FS_printf( &fh, "| + generic1: %i\n", sv_ent->s.generic1 );
			FS_printf( &fh, "+ r\n" );
			FS_printf( &fh, "| + s\n" );
			FS_printf( &fh, "| | + number: %i\n", sv_ent->r.s.number );
			FS_printf( &fh, "| | + eType: %i\n", sv_ent->r.s.eType );
			FS_printf( &fh, "| | + eFlags: %i\n", sv_ent->r.s.eFlags );
			FS_printf( &fh, "| | + pos\n" );
			FS_printf( &fh, "| | | + trType: %i\n", sv_ent->r.s.pos.trType );
			FS_printf( &fh, "| | | + trTime: %i\n", sv_ent->r.s.pos.trTime );
			FS_printf( &fh, "| | | + trDuration: %i\n", sv_ent->r.s.pos.trDuration );
			FS_printf( &fh, "| | | + trBase: %f %f %f\n", sv_ent->r.s.pos.trBase[0], sv_ent->r.s.pos.trBase[1], sv_ent->r.s.pos.trBase[2] );
			FS_printf( &fh, "| | | + trDelta: %f %f %f\n", sv_ent->r.s.pos.trDelta[0], sv_ent->r.s.pos.trDelta[1], sv_ent->r.s.pos.trDelta[2] );
			FS_printf( &fh, "| | + apos\n" );
			FS_printf( &fh, "| | | + trType: %i\n", sv_ent->r.s.apos.trType );
			FS_printf( &fh, "| | | + trTime: %i\n", sv_ent->r.s.apos.trTime );
			FS_printf( &fh, "| | | + trDuration: %i\n", sv_ent->r.s.apos.trDuration );
			FS_printf( &fh, "| | | + trBase: %f %f %f\n", sv_ent->r.s.apos.trBase[0], sv_ent->r.s.apos.trBase[1], sv_ent->r.s.apos.trBase[2] );
			FS_printf( &fh, "| | | + trDelta: %f %f %f\n", sv_ent->r.s.apos.trDelta[0], sv_ent->r.s.apos.trDelta[1], sv_ent->r.s.apos.trDelta[2] );
			FS_printf( &fh, "| | + time: %i\n", sv_ent->r.s.time );
			FS_printf( &fh, "| | + time2: %i\n", sv_ent->r.s.time2 );
			FS_printf( &fh, "| | + origin: %f %f %f\n", sv_ent->r.s.origin[0], sv_ent->r.s.origin[1], sv_ent->r.s.origin[2] );
			FS_printf( &fh, "| | + origin2: %f %f %f\n", sv_ent->r.s.origin2[0], sv_ent->r.s.origin2[1], sv_ent->r.s.origin2[2] );
			FS_printf( &fh, "| | + angles: %f %f %f\n", sv_ent->r.s.angles[0], sv_ent->r.s.angles[1], sv_ent->r.s.angles[2] );
			FS_printf( &fh, "| | + angles2: %f %f %f\n", sv_ent->r.s.angles2[0], sv_ent->r.s.angles2[1], sv_ent->r.s.angles2[2] );
			FS_printf( &fh, "| | + otherEntityNum: %i\n", sv_ent->r.s.otherEntityNum );
			FS_printf( &fh, "| | + otherEntityNum2: %i\n", sv_ent->r.s.otherEntityNum2 );
			FS_printf( &fh, "| | + groundEntityNum: %i\n", sv_ent->r.s.groundEntityNum );
			FS_printf( &fh, "| | + constantLight: %i\n", sv_ent->r.s.constantLight );
			FS_printf( &fh, "| | + loopSound: %i\n", sv_ent->r.s.loopSound );
			FS_printf( &fh, "| | + modelindex: %i\n", sv_ent->r.s.modelindex );
			FS_printf( &fh, "| | + modelindex2: %i\n", sv_ent->r.s.modelindex2 );
			FS_printf( &fh, "| | + clientNum: %i\n", sv_ent->r.s.clientNum );
			FS_printf( &fh, "| | + frame: %i\n", sv_ent->r.s.frame );
			FS_printf( &fh, "| | + solid: %i\n", sv_ent->r.s.solid );
			FS_printf( &fh, "| | + event: %i\n", sv_ent->r.s.event );
			FS_printf( &fh, "| | + eventParm: %i\n", sv_ent->r.s.eventParm );
			FS_printf( &fh, "| | + powerups: %i\n", sv_ent->r.s.powerups );
			FS_printf( &fh, "| | + weapon: %i\n", sv_ent->r.s.weapon );
			FS_printf( &fh, "| | + legsAnim: %i\n", sv_ent->r.s.legsAnim );
			FS_printf( &fh, "| | + powerups: %i\n", sv_ent->r.s.powerups );
			FS_printf( &fh, "| | + powerups: %i\n", sv_ent->r.s.powerups );
			FS_printf( &fh, "| | + torsoAnim: %i\n", sv_ent->r.s.torsoAnim );
			FS_printf( &fh, "| | + generic1: %i\n", sv_ent->r.s.generic1 );
			FS_printf( &fh, "| + linked: %i\n", sv_ent->r.linked );
			FS_printf( &fh, "| + linkcount: %i\n", sv_ent->r.linkcount );
			FS_printf( &fh, "| + svFlags: %i\n", sv_ent->r.svFlags );
			FS_printf( &fh, "| + singleClient: %i\n", sv_ent->r.singleClient );
			FS_printf( &fh, "| + bmodel: %i\n", sv_ent->r.bmodel );
			FS_printf( &fh, "| + mins: %f %f %f\n", sv_ent->r.mins[0], sv_ent->r.mins[1], sv_ent->r.mins[2] );
			FS_printf( &fh, "| + maxs: %f %f %f\n", sv_ent->r.maxs[0], sv_ent->r.maxs[1], sv_ent->r.maxs[2] );
			FS_printf( &fh, "| + contents: %i\n", sv_ent->r.contents );
			FS_printf( &fh, "| + absmin: %f %f %f\n", sv_ent->r.absmin[0], sv_ent->r.absmin[1], sv_ent->r.absmin[2] );
			FS_printf( &fh, "| + absmax: %f %f %f\n", sv_ent->r.absmax[0], sv_ent->r.absmax[1], sv_ent->r.absmax[2] );
			FS_printf( &fh, "| + currentOrigin: %f %f %f\n", sv_ent->r.currentOrigin[0], sv_ent->r.currentOrigin[1], sv_ent->r.currentOrigin[2] );
			FS_printf( &fh, "| + currentAngles: %f %f %f\n", sv_ent->r.currentAngles[0], sv_ent->r.currentAngles[1], sv_ent->r.currentAngles[2] );
			FS_printf( &fh, "| + ownerNum: %i\n", sv_ent->r.ownerNum );
			if( sv_ent->client ) {
				FS_printf( &fh, "+ client\n" );
				FS_printf( &fh, "+ + TODO: fill this in\n" );
			} else {
				FS_printf( &fh, "+ client: NULL\n" );
			}
			FS_printf( &fh, "+ inuse: %i\n", sv_ent->inuse );
			FS_printf( &fh, "+ classname: %s\n", sv_ent->classname );
			FS_printf( &fh, "+ spawnflags: %i\n", sv_ent->spawnflags );
			FS_printf( &fh, "+ neverFree: %i\n", sv_ent->neverFree );
			FS_printf( &fh, "+ flags: %i\n", sv_ent->flags );
			FS_printf( &fh, "+ model: %s\n", sv_ent->model );
			FS_printf( &fh, "+ model2: %s\n", sv_ent->model2 );
			FS_printf( &fh, "+ freetime: %i\n", sv_ent->freetime );
			FS_printf( &fh, "+ eventTime: %i\n", sv_ent->eventTime );
			FS_printf( &fh, "+ freeAfterEvent: %i\n", sv_ent->freeAfterEvent );
			FS_printf( &fh, "+ unlinkAfterEvent: %i\n", sv_ent->unlinkAfterEvent );
			FS_printf( &fh, "+ physicsObject: %i\n", sv_ent->physicsObject );
			FS_printf( &fh, "+ physicsBounce: %f\n", sv_ent->physicsBounce );
			FS_printf( &fh, "+ clipmask: %i\n", sv_ent->clipmask );
			FS_printf( &fh, "+ moverState\n" );
			FS_printf( &fh, "+ + TODO: fill this in\n" );
			FS_printf( &fh, "+ soundPos1: %i\n", sv_ent->soundPos1 );
			FS_printf( &fh, "+ sound1to2: %i\n", sv_ent->sound1to2 );
			FS_printf( &fh, "+ sound2to1: %i\n", sv_ent->sound2to1 );
			FS_printf( &fh, "+ soundPos2: %i\n", sv_ent->soundPos2 );
			FS_printf( &fh, "+ soundLoop: %i\n", sv_ent->soundLoop );
			if( sv_ent->parent) {
				FS_printf( &fh, "+ parent: %i\n", sv_ent->parent->s.number );
			} else {
				FS_printf( &fh, "+ parent: NULL\n" );
			}
			if( sv_ent->nextTrain) {
				FS_printf( &fh, "+ nextTrain: %i\n", sv_ent->nextTrain->s.number );
			} else {
				FS_printf( &fh, "+ nextTrain: NULL\n" );
			}
			if( sv_ent->prevTrain) {
				FS_printf( &fh, "+ prevTrain: %i\n", sv_ent->prevTrain->s.number );
			} else {
				FS_printf( &fh, "+ prevTrain: NULL\n" );
			}
			FS_printf( &fh, "+ pos1 %f %f %f\n", sv_ent->pos1[0], sv_ent->pos1[1], sv_ent->pos1[2] );
			FS_printf( &fh, "+ pos2 %f %f %f\n", sv_ent->pos2[0], sv_ent->pos2[1], sv_ent->pos2[2] );
			FS_printf( &fh, "+ message: %s\n", sv_ent->message );
			FS_printf( &fh, "+ timestamp: %i\n", sv_ent->timestamp );
			FS_printf( &fh, "+ angle: %f\n", sv_ent->angle );
			FS_printf( &fh, "+ target: %s\n", sv_ent->target );
			FS_printf( &fh, "+ targetname: %s\n", sv_ent->targetname );
			FS_printf( &fh, "+ team: %s\n", sv_ent->team );
			if( sv_ent->target_ent) {
				FS_printf( &fh, "+ target_ent: %i\n", sv_ent->target_ent->s.number );
			} else {
				FS_printf( &fh, "+ target_ent: NULL\n" );
			}
			FS_printf( &fh, "+ targetShaderName: %s\n", sv_ent->targetShaderName );
			FS_printf( &fh, "+ targetShaderNewName: %s\n", sv_ent->targetShaderNewName );
			FS_printf( &fh, "+ speed: %f\n", sv_ent->speed );
			FS_printf( &fh, "+ movedir %f %f %f\n", sv_ent->movedir[0], sv_ent->movedir[1], sv_ent->movedir[2] );
			FS_printf( &fh, "+ nextthink: %i\n", sv_ent->nextthink );
			FS_printf( &fh, "+ think: %s\n", ( sv_ent->think ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ reached: %s\n", ( sv_ent->reached ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ blocked: %s\n", ( sv_ent->blocked ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ touch: %s\n", ( sv_ent->touch ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ use: %s\n", ( sv_ent->use ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ pain: %s\n", ( sv_ent->pain ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ drop: %s\n", ( sv_ent->drop ? "EXISTS" : "NULL" ) );
			FS_printf( &fh, "+ pain_debounce_time: %i\n", sv_ent->pain_debounce_time );
			FS_printf( &fh, "+ fly_sound_debounce_time: %i\n", sv_ent->fly_sound_debounce_time );
			FS_printf( &fh, "+ last_move_time: %i\n", sv_ent->last_move_time );
			FS_printf( &fh, "+ health: %i\n", sv_ent->health );
			FS_printf( &fh, "+ takedamage: %i\n", sv_ent->takedamage );
			FS_printf( &fh, "+ damage: %i\n", sv_ent->damage );
			FS_printf( &fh, "+ splashDamage: %i\n", sv_ent->splashDamage );
			FS_printf( &fh, "+ splashRadius: %i\n", sv_ent->splashRadius );
			FS_printf( &fh, "+ methodOfDeath: %i\n", sv_ent->methodOfDeath );
			FS_printf( &fh, "+ splashMethodOfDeath: %i\n", sv_ent->splashMethodOfDeath );
			FS_printf( &fh, "+ count: %i\n", sv_ent->count );
			if( sv_ent->chain) {
				FS_printf( &fh, "+ chain: %i\n", sv_ent->chain->s.number );
			} else {
				FS_printf( &fh, "+ chain: NULL\n" );
			}
			if( sv_ent->enemy) {
				FS_printf( &fh, "+ enemy: %i\n", sv_ent->enemy->s.number );
			} else {
				FS_printf( &fh, "+ enemy: NULL\n" );
			}
			if( sv_ent->activator) {
				FS_printf( &fh, "+ activator: %i\n", sv_ent->activator->s.number );
			} else {
				FS_printf( &fh, "+ activator: NULL\n" );
			}
			if( sv_ent->teamchain) {
				FS_printf( &fh, "+ teamchain: %i\n", sv_ent->teamchain->s.number );
			} else {
				FS_printf( &fh, "+ teamchain: NULL\n" );
			}
			if( sv_ent->teammaster) {
				FS_printf( &fh, "+ teammaster: %i\n", sv_ent->teammaster->s.number );
			} else {
				FS_printf( &fh, "+ teammaster: NULL\n" );
			}
			FS_printf( &fh, "+ watertype: %i\n", sv_ent->watertype );
			FS_printf( &fh, "+ waterlevel: %i\n", sv_ent->waterlevel );
			FS_printf( &fh, "+ noise_index: %i\n", sv_ent->noise_index );
			FS_printf( &fh, "+ wait: %f\n", sv_ent->wait );
			FS_printf( &fh, "+ random: %f\n", sv_ent->random );
			if( sv_ent->item ) {
				FS_printf( &fh, "+ item\n" );
				FS_printf( &fh, "+ + TODO: fill this in\n" );
			} else {
				FS_printf( &fh, "+ item: NULL\n" );
			}
			if( sv_ent->mapdata ) {
				FS_printf( &fh, "+ mapdata\n" );
				FS_printf( &fh, "+ + TODO: fill this in\n" );
			} else {
				FS_printf( &fh, "+ mapdata: NULL\n" );
			}
			FS_printf( &fh, "+ nextbeamhittime: %i\n", sv_ent->nextbeamhittime );
		}*/

		trap_FS_FCloseFile( fh );
	}
}

static void G_Q3F_SetTalkIcon_f( gentity_t *ent, qboolean enabled ) {
	if(enabled) {
		ent->s.eFlags |= EF_TALK;
	} else {
		ent->s.eFlags &= ~EF_TALK;
	}
}

/*static void G_Q3F_CheatLog( gentity_t *ent ) {
	qtime_t time;
	fileHandle_t fh;
	char buf[1024];
	char userinfo[MAX_INFO_STRING];
	char gluid[64];

	trap_RealTime( &time );

	if( trap_FS_FOpenFile( "cheatlog.log", &fh, FS_APPEND ) >= 0 ) {
		trap_GetUserinfo( ent->client->ps.clientNum, userinfo, sizeof( userinfo ) );

		Q_strncpyz( gluid, Info_ValueForKey (userinfo, "cl_guid"), sizeof(gluid) );

		Com_sprintf( buf, sizeof(buf), "%i-%i-%i (%i:%i): Caught %s(%i) cheating, ip: %s, cl_guid: %s\n",
											time.tm_mday, time.tm_mon + 1, time.tm_year + 1900,
											time.tm_hour, time.tm_min,
											ent->client->pers.netname, ent->client->ps.clientNum,
											ent->client->sess.ipStr, gluid );
		trap_FS_Write( buf, strlen(buf), fh );
		trap_FS_FCloseFile( fh );
	}
}*/

/*
**	Admin functions
*/

static void G_Q3F_PlayerStatus( gentity_t *ent )
{
	// Print off a list of clients, so they can easily be ignored or muted.

	gentity_t *player;

	trap_SendServerCommand( ent-g_entities, "print \"^3 ID Ping Colour Score Name\n\"" );
	for( player = g_entities; player < &g_entities[level.maxclients]; player++ )
	{
		if( player->inuse && player->client )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%3d %4d %6s %5d %s\n\"",
								player->s.number,
								player->client->ps.ping,
								(player->client->sess.sessionTeam ? g_q3f_teamlist[player->client->sess.sessionTeam].name : "spec"),
								player->client->ps.persistant[PERS_SCORE],
								player->client->pers.netname ) );
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%3d %4d %6s %5d %s\n\"",
								(int)(player-g_entities),
								player->client->ps.ping,
								"conn",
								player->client->ps.persistant[PERS_SCORE],
								player->client->pers.netname ) );
		}
	}
	trap_SendServerCommand( ent-g_entities, "print \"Complete\n\"" );
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];
	g_q3f_playerclass_t *cls;
#ifdef _DEBUG
	int index;

	G_Printf( "%d %d: ", clientNum, trap_Argc() );
	for( index = 0; index < trap_Argc(); index++ )
	{
		trap_Argv( index, cmd, sizeof( cmd ) );
		G_Printf( "'%s' ", cmd);
	}
	G_Printf( "\n" );
#endif

	ent = g_entities + clientNum;
	if ( !ent->client || ent->client->pers.initializing ) {
		return;		// not fully in game yet
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

#ifdef BUILD_LUA
	// LUA API callbacks
	if (G_LuaHook_ClientCommand(clientNum, cmd))
	{
		return;
	}

	if (Q_stricmp(cmd, "lua_status") == 0)
	{
		G_LuaStatus(ent);
		return;
	}
#endif

	if (Q_stricmp (cmd, "say") == 0) {
		if ( !ent->client->sess.muted || level.ceaseFire ) {
			Cmd_Say_f (ent, SAY_ALL );
		}
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		if ( !ent->client->sess.muted || level.ceaseFire ) {
			Cmd_Say_f (ent, SAY_TEAM );
		}
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		if ( !ent->client->sess.muted || level.ceaseFire ) {
			Cmd_Tell_f ( ent );
		}
		return;
	}
#if 0
	/* Ensiform - These go bye bye */
	if (Q_stricmp (cmd, "vsay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vsay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "vtell") == 0) {
		Cmd_VoiceTell_f ( ent, qfalse );
		return;
	}
	if (Q_stricmp (cmd, "vosay") == 0) {
		Cmd_Voice_f (ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "vosay_team") == 0) {
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp (cmd, "votell") == 0) {
		Cmd_VoiceTell_f ( ent, qtrue );
		return;
	}
#endif
/*	if (Q_stricmp (cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f ( ent );
		return;
	}*/
	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "getstats") == 0) {
 		Cmd_Stats_f (ent);
		return;
	}

	if( Q_stricmp (cmd, "tgren") == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
		G_Q3F_GrenadeCommand( ent );
		return;
	}
	if( Q_stricmp("votemap", cmd) == 0 ) {
		G_Q3F_MapSelectVote(ent);
		return;
	}

	/* Ensiform - These were moved up from below since they *SHOULD* be allowed during intermission, no? */
	if( Q_stricmp("adminpassword", cmd) == 0 ) {
		G_Q3F_AdminPasswordCommand(ent);
		return;
	}
// djbob: rcon authing for hud display
	if( Q_stricmp("authrc", cmd) == 0 ) {
		G_Q3F_RCONPasswordCommand(ent);
		return;
	}
// djbob
	if( Q_stricmp("admin", cmd) == 0 ) {
		G_Q3F_AdminCommand(ent);
		return;
	}
// djbob: cant be issued from rcon normally, it'd be usless for a start :)
	if( Q_stricmp("admin2", cmd) == 0 ) {
		G_Q3F_AdminCommand_ClientOnly(ent);
		return;
	}
// djbob
/* Ensiform - Player Status */
	if( Q_stricmp("playerstatus", cmd) == 0) {
		G_Q3F_PlayerStatus(ent);
		return;
	}
	/* Ensiform - End Move */

	if( Q_stricmp(cmd, "ignore") == 0 ) {
		Cmd_Ignore_f( ent );
		return;
	}

	if( Q_stricmp(cmd, "unignore") == 0 ) {
		Cmd_UnIgnore_f( ent );
		return;
	}

	/* Ensiform - These were moved up from below since they *SHOULD* be allowed during intermission, no? */
	if( Q_stricmp("etf_starttalk", cmd) == 0 ) {
		G_Q3F_SetTalkIcon_f( ent, qtrue );
		return;
	}
	if( Q_stricmp("etf_stoptalk", cmd) == 0 ) {
		G_Q3F_SetTalkIcon_f( ent, qfalse );
		return;
	}
	/* Ensiform - End Move */

	if( Q_stricmp("ClassinfoRequest", cmd) == 0 ) {
		G_Q3F_SendClassInfo( ent );
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	//else if (Q_stricmp (cmd, "levelshot") == 0)
	//	Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "followmode") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
		{
			//Cmd_FollowMode_f (ent);
			//StopFollowing(ent, qfalse);
		}
	}
	else if (Q_stricmp (cmd, "stopfollow") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			StopFollowing(ent, qfalse);
	}
	else if (Q_stricmp (cmd, "follow") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_Follow_f (ent, SPECTATOR_FOLLOW);
	}
	else if (Q_stricmp (cmd, "follownext") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_FollowCycle_f (ent, 1, SPECTATOR_FOLLOW);
	}
	else if (Q_stricmp (cmd, "followprev") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_FollowCycle_f (ent, -1, SPECTATOR_FOLLOW);
	}
	else if (Q_stricmp (cmd, "chase") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_Follow_f (ent, SPECTATOR_CHASE);
	}
	else if (Q_stricmp (cmd, "chasenext") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_FollowCycle_f (ent, 1, SPECTATOR_CHASE);
	}
	else if (Q_stricmp (cmd, "chaseprev") == 0) {
		if ( ent->client->sess.adminLevel >= ADMIN_MATCH || g_spectatorMode.value == 0 )
			Cmd_FollowCycle_f (ent, -1, SPECTATOR_CHASE);
	}
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	else if (Q_stricmp (cmd, "charge") == 0 && (ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR) )
		G_Q3F_ChargeCommand( ent );
	else if (Q_stricmp (cmd, "changeclass") == 0)
		G_Q3F_SendClassMenu( ent, 0 );
	else if (Q_stricmp (cmd, "changeteam") == 0)
		G_Q3F_SendTeamMenu( ent, qfalse );
	else if (Q_stricmp (cmd, "channel") == 0)
		G_Q3F_ChannelCommand( ent );
	else if ((Q_stricmp (cmd, "disguise") == 0) && (ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR) && (!level.ceaseFire))
		G_Q3F_DisguiseCommand( ent );
	else if (Q_stricmp (cmd, "flaginfo") == 0)
		G_Q3F_FlagInfo( ent );
	else if (Q_stricmp("reload", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		BG_Q3F_Request_Reload(&ent->client->ps);
	else if ((Q_stricmp("invisible", cmd) == 0) && (ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR) && (!level.ceaseFire))
		G_Q3F_InvisibleCommand(ent);
	else if (Q_stricmp("discard_etf", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_DiscardCommand(ent);
	else if (Q_stricmp("dropammo", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_DropAmmoCommand(ent);
	else if (Q_stricmp("dropammoto", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_DropAmmoToCommand(ent);
	else if (Q_stricmp("saveme", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_SavemeCommand(ent);
	else if (Q_stricmp("armorme", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_ArmormeCommand(ent);
	else if (Q_stricmp("build", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_EngineerBuild_Command(ent);
	else if (Q_stricmp("destroy", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_EngineerDestroy_Command(ent);
	else if (Q_stricmp("supply", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_Supply_Command(ent);
	else if (Q_stricmp("dropflag", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_DropAllFlags( ent, qfalse, qtrue );
	else if (Q_stricmp("useflag", cmd) == 0 && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR)
		G_Q3F_FlagUseHeld( ent );
#if 0
	/* Ensiform - Moved up above intermission check */
	else if (Q_stricmp("adminpassword", cmd) == 0)
		G_Q3F_AdminPasswordCommand(ent);
// djbob: rcon authing for hud display
	else if (Q_stricmp("authrc", cmd) == 0)
		G_Q3F_RCONPasswordCommand(ent);
// djbob
	else if (Q_stricmp("admin", cmd) == 0)
		G_Q3F_AdminCommand(ent);
// djbob: cant be issued from rcon normally, it'd be usless for a start :)
	else if (Q_stricmp("admin2", cmd) == 0) {
		G_Q3F_AdminCommand_ClientOnly(ent);
	}
// djbob
/* Ensiform - Player Status */
	else if (Q_stricmp("playerstatus", cmd) == 0)
		G_Q3F_PlayerStatus(ent);
	/* End Move */
#endif
//	else if (Q_stricmp("mapresponse", cmd) == 0)
//		G_Q3F_MapSelectResponse(ent);
	else if (Q_stricmp("spectatemenu", cmd) == 0)
		G_Q3F_SendSpectateMenu( ent );
	else if (Q_stricmp("flyby", cmd) == 0)
		G_Q3F_Flyby( ent );
	else if (Q_stricmp("dumpserver", cmd) == 0)
		G_Q3F_DumpServer( ent );
	else if (Q_stricmp("waypoint", cmd) == 0)
		G_Q3F_WaypointCommand( ent );
#if 0
	/* Ensiform - Also moving these */
	else if( Q_stricmp("etf_starttalk", cmd) == 0 )
		G_Q3F_SetTalkIcon_f( ent, qtrue );
	else if( Q_stricmp("etf_stoptalk", cmd) == 0 )
		G_Q3F_SetTalkIcon_f( ent, qfalse );
	/* Ensiform - End Move */
#endif
#if 0
	/* Ensiform - This should always work (during intermission) */
	else if( Q_stricmp("ClassinfoRequest", cmd) == 0 )
		G_Q3F_SendClassInfo( ent );
#endif
	// RR2DO2 - osp style ready
	else if (Q_stricmp (cmd, "ready") == 0)
		Cmd_Ready_f( ent );
	else if (Q_stricmp (cmd, "unready") == 0)
		Cmd_UnReady_f( ent );
	// RR2DO2
	// RR2DO2: anti cheat
	/*else if (Q_stricmp( cmd, level.cheatcmd ) == 0 )
		G_Q3F_CheatLog( ent );*/
	else if (Q_stricmp("special", cmd) == 0  && ent->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR && G_Q3F_SpecialCommand(ent) )
		{}		// We've done our stuff in the command in the if()
	else if(!Q_stricmp("special", cmd) && ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR && (ent->client->sess.spectatorState == SPECTATOR_FOLLOW ||
		ent->client->sess.spectatorState == SPECTATOR_CHASE) && G_Q3F_SpecialCommand(ent))
		{}		// We've done our stuff in the command in the if()
   else if (Q_stricmp (cmd, "cpm") == 0)
      {}  //keeg: ignore this and just return  
   // Golliwog: Allow class changes and custom commands
	else {
		if( !G_Q3F_ChangeClassCommand( ent, cmd) )
		{
			cls = G_Q3F_GetClass( &ent->client->ps );
			if( !(cls->ClientCommand && cls->ClientCommand( ent, cmd )) )
			{
				if((Q_stricmp (cmd, "det") == 0) || (Q_stricmp (cmd, "detpipe") == 0)|| (Q_stricmp (cmd, "scanner") == 0) || (Q_stricmp (cmd, "special") == 0))
					trap_SendServerCommand( clientNum, va("print \"Command %s not valid for your class!\n\"", cmd ) );
				else
					trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
            //keeg change the wording for the error message on class-specific commands..
            //trap_SendServerCommand( clientNum, va("print \"Command %s not valid for your class!\n\"", cmd) );
			}
		}
	}
// Golliwog.
}
