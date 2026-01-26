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

#include "g_local.h"
#include "g_q3f_mapents.h"
#include "g_q3f_grenades.h"
#include "g_q3f_weapon.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_team.h"
#include "g_q3f_flag.h"
#include "g_q3f_admin.h"
#include "g_q3f_mapselect.h"
#include "g_q3f_eventlog.h"

#include "g_bot_interface.h"

#ifdef BUILD_LUA
#include "g_lua.h"
#endif

level_locals_t	level;

typedef struct {
	vmCvar_t	*vmCvar;
	const char	*cvarName;
	const char	*defaultString;
	int			cvarFlags;
	int			extCvarFlags; // extension flags
	qboolean	trackChange;	    // track this variable, and announce if changed
	int			modificationCount;  // for tracking changes
} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

#define DECLARE_G_CVAR
	#include "g_cvar.h"
#undef DECLARE_G_CVAR

static cvarTable_t		gameCvarTable[] = {
#define G_CVAR_LIST
	#include "g_cvar.h"
#undef G_CVAR_LIST
	
};

static const int gameCvarTableSize = (int)ARRAY_LEN( gameCvarTable );


void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void CheckExitRules( void );

qboolean engine_is_ete = qfalse;
qboolean deleteFile = qfalse;

// extension interface
int dll_com_trapGetValue;
int cvar_notabcomplete = 0;
int cvar_nodefault = 0;
int cvar_archive_nd = 0;
int cvar_developer = 0;

int dll_trap_FS_Delete;

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4,
	intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9, intptr_t arg10, intptr_t arg11 )
{
	switch ( command ) {
	case GAME_INIT:
#ifdef BUILD_BOTS
		Bot_Interface_InitHandles();
#endif
		G_InitGame( arg0, arg1, arg2 );
#ifdef BUILD_BOTS
		if(!Bot_Interface_Init())
			G_Printf(S_COLOR_RED "Unable to Initialize Omni-Bot.\n");
#endif
#ifdef DREVIL_BOT_SUPPORT
		if(!Bot_Interface_Init())
			G_Printf(S_COLOR_RED "Unable to Initialize Omni-Bot.\n");
#endif
#if id386
		G_PatchEngine();
#endif
		return 0;
	case GAME_SHUTDOWN:
#ifdef DREVIL_BOT_SUPPORT
		if(!Bot_Interface_Shutdown())
			G_Printf(S_COLOR_RED "Error Shutting Down Omni-Bot.\n");
#endif
#ifdef BUILD_BOTS
		Bot_Interface_Shutdown();
#endif
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (intptr_t)ClientConnect( arg0, (qboolean)arg1, (qboolean)arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0, "game" );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
#ifdef DREVIL_BOT_SUPPORT
		// Called before G_RunFrame so the unlagged data will be correct.
		Bot_Interface_Update();
#endif
		G_RunFrame( arg0 );
#ifdef BUILD_BOTS
		Bot_Interface_Update();
#endif
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	}

	return -1;
}

void G_Q3F_Global_Think(void) {
}

void FORMAT_PRINTF(1,2) QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[BIG_INFO_STRING];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

#ifdef BUILD_LUA
	G_LuaHook_Print(GPRINT_TEXT, text);
#endif

	trap_Print( text );
}

void NORETURN FORMAT_PRINTF(1,2) QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[BIG_INFO_STRING];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

#ifdef BUILD_LUA
	G_LuaHook_Print(GPRINT_ERROR, text);
#endif

	trap_Error( text );
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=MAX_CLIENTS, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf ("%i teams with %i entities\n", c, c2);
}

/*
=================
G_Q3F_UpdateCvarLimits
=================
*/
static void G_Q3F_UpdateCvarLimits(void) {
	trap_SetConfigstring( CS_Q3F_CVARLIMITS, va( "snaps %i -1 rate %i -1", g_minSnaps.integer, g_minRate.integer ) );
}

/*
=================
G_RegisterCvars
=================
*/
static void G_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
//	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags, cv->extCvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;
	}

	// check some things
	trap_Cvar_Set( "g_gametype", "0" );

	/* Ensiform - Forcing this to 0 here as it can sometimes be imported from ET configs and set to 1, thus CVAR_ROM would be bad */
	trap_Cvar_Set( "sv_floodProtect", "0" );

	if ( g_maxNailBombs.integer < 0 ) {
		trap_Cvar_Set("g_maxNailBombs", "0");
		g_maxNailBombs.integer = 0;
	}
	else if (g_maxNailBombs.integer > 99 ) {
		trap_Cvar_Set("g_maxNailBombs", "99");
		g_maxNailBombs.integer = 99;
	}

	if ( g_maxGasGrens.integer < 0 ) {
		trap_Cvar_Set("g_maxGasGrens", "0");
		g_maxGasGrens.integer = 0;
	}
	else if (g_maxGasGrens.integer > 99 ) {
		trap_Cvar_Set("g_maxGasGrens", "99");
		g_maxGasGrens.integer = 99;
	}

	level.warmupModificationCount = g_warmup.modificationCount;

		// Golliwog: Set ceasefire automatically if we're in match mode.
	trap_SetConfigstring( CS_FORTS_CEASEFIRE, "0" );
	level.ceaseFire = qfalse;

		// Golliwog: Set local gravity value
	bg_evaluategravity = g_gravity.value;
	level.gravityModificationCount = g_gravity.modificationCount;

		// RR2DO2: Set cvar limiting values
	level.minRateModificationCount = g_minRate.modificationCount;
	level.minSnapsModificationCount = g_minSnaps.modificationCount;
	G_Q3F_UpdateCvarLimits();
}

/*
================
G_Q3F_ParseFriendlyFire

Parses numeric / text friendly fire string, converts to appropriate number
================
*/
static int G_Q3F_ParseFriendlyFire( char *ffstring )
{
	int val, mask;
	char buff[16];
	char *ptr;
	qboolean ismirror;

	val = Q_atoi( ffstring );
	if( val )
		return( val );

	mask = 0;
	while( *ffstring )
	{
		while( *ffstring == ' ' || *ffstring == '_' || *ffstring == '-' ) ffstring++;
		for( ptr = buff; (ptr - buff) < 15 && *ffstring != ' ' && *ffstring; ptr++, ffstring++ )
			*ptr = *ffstring;
		*ptr = '\0';

		ptr = buff;
		if( !Q_strncmp( ptr, "mirror", 6 ) )
		{
			ptr += 6;
			ismirror = qtrue;
		}
		else ismirror = qfalse;

		if( !Q_stricmp( ptr, "full" ) )
			mask |= ismirror ? FF_Q3F_MIRRORFULL : FF_Q3F_FULL;
		else if( !Q_stricmp( ptr, "half" ) )
			mask |= ismirror ? FF_Q3F_MIRRORHALF : FF_Q3F_HALF;
		else if( !Q_stricmp( ptr, "armor" ) || !Q_stricmp( ptr, "armour" ) )
			mask |= ismirror ? FF_Q3F_MIRRORARMOUR : FF_Q3F_ARMOUR;
	}
	return( mask );
}

static void G_Q3F_UpdateTeamNames(void) {
	trap_SetConfigstring(CS_TEAMNAMES, va("\\g_etf_redTeam\\%s\\g_etf_blueTeam\\%s\\g_etf_yellowTeam\\%s\\g_etf_greenTeam\\%s", 
		g_redteam.string, g_blueteam.string, g_yellowteam.string, g_greenteam.string));
}

void G_SetMatchState(int state) {
	const char		*s = va("%i",state);

	trap_Cvar_Set( "g_matchState", s );
	g_matchState.integer = state;
	trap_Cvar_Update( &g_matchState );
	trap_SetConfigstring( CS_MATCH_STATE, s );

}

/*
=================
G_UpdateCvars
=================
*/
static void G_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;
//	qboolean remapped = qfalse;
	qboolean namechange = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", 
						cv->cvarName, cv->vmCvar->string ) );
				}

				if (cv->vmCvar == &g_maxNailBombs) {
					if (g_maxNailBombs.integer < 0) {
						trap_Cvar_Set("g_maxNailBombs", "0");
						g_maxNailBombs.integer = 0;
						trap_Cvar_Update( cv->vmCvar );
					}
					else if (g_maxNailBombs.integer > 99) {
						trap_Cvar_Set("g_maxNailBombs", "99");
						g_maxNailBombs.integer = 99;
						trap_Cvar_Update( cv->vmCvar );
					}
				}

				if (cv->vmCvar == &g_maxGasGrens) {
					if (g_maxGasGrens.integer < 0) {
						trap_Cvar_Set("g_maxGasGrens", "0");
						g_maxGasGrens.integer = 0;
						trap_Cvar_Update( cv->vmCvar );
					}
					else if (g_maxGasGrens.integer > 99) {
						trap_Cvar_Set("g_maxGasGrens", "99");
						g_maxGasGrens.integer = 99;
						trap_Cvar_Update( cv->vmCvar );
					}
				}
				
#ifdef BUILD_LUA
				if( cv->vmCvar == &lua_modules || cv->vmCvar == &lua_allowedModules ) {
					G_LuaShutdown();
				}
#endif
			}
		}
	}

	if (pmove_msec.integer < 8)
		trap_Cvar_Set("pmove_msec", "8");
	else if (pmove_msec.integer > 33)
		trap_Cvar_Set("pmove_msec", "33");

	// Golliwog: Check the damage type and gravity
	if( g_friendlyFire.modificationCount != level.friendlyFireCount )
	{
		level.friendlyFireCount = g_friendlyFire.modificationCount;
		level.friendlyFire = G_Q3F_ParseFriendlyFire( g_friendlyFire.string );
	}
	if( g_gravity.modificationCount != level.gravityModificationCount )
	{
		bg_evaluategravity = g_gravity.value;
		level.gravityModificationCount = g_gravity.modificationCount;
	}
	// Golliwog.

	// RR2DO2: check for cvars that need to be sent to the client for limitting
	if( g_minRate.modificationCount != level.minRateModificationCount ||
		g_minSnaps.modificationCount != level.minSnapsModificationCount ) {
		G_Q3F_UpdateCvarLimits();
		level.minRateModificationCount = g_minRate.modificationCount;
		level.minSnapsModificationCount = g_minSnaps.modificationCount;
	}
	// RR2DO2

	// djbob
	if( g_redteam.modificationCount != level.redTeamModificationCount ) {
		G_Q3F_RemString(&g_q3f_teamlist[1].description);
		G_Q3F_AddString(&g_q3f_teamlist[1].description, g_redteam.string);
		level.redTeamModificationCount = g_redteam.modificationCount;
		namechange = qtrue;
	}
	if( g_blueteam.modificationCount != level.blueTeamModificationCount ) {
		G_Q3F_RemString(&g_q3f_teamlist[2].description);
		G_Q3F_AddString(&g_q3f_teamlist[2].description, g_blueteam.string);
		level.blueTeamModificationCount = g_blueteam.modificationCount;
		namechange = qtrue;
	}
	if( g_yellowteam.modificationCount != level.yellowTeamModificationCount ) {
		G_Q3F_RemString(&g_q3f_teamlist[3].description);
		G_Q3F_AddString(&g_q3f_teamlist[3].description, g_yellowteam.string);
		level.yellowTeamModificationCount = g_yellowteam.modificationCount;
		namechange = qtrue;
	}
	if( g_greenteam.modificationCount != level.greenTeamModificationCount ) {
		G_Q3F_RemString(&g_q3f_teamlist[4].description);
		G_Q3F_AddString(&g_q3f_teamlist[4].description, g_greenteam.string);
		level.greenTeamModificationCount = g_greenteam.modificationCount;
		namechange = qtrue;
	}

	if(namechange) {
		G_Q3F_UpdateTeamNames();
	}
	// djbob
}


/*
======================
G_Q3F_LoadMapConfig

======================
*/
void G_Q3F_LoadMapConfig(void) {
	// RR2DO2: load any map-specific config

	char mapcfg[MAX_QPATH];
	fileHandle_t fh;

 	if( g_execMapConfigs.integer )
	{
		if( trap_FS_FOpenFile( "map_default.cfg", &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			trap_SendConsoleCommand( EXEC_APPEND, va( "exec map_default.cfg" ) );
		}

		Com_sprintf( mapcfg, sizeof(mapcfg), "%s.cfg", level.rawmapname );

		if( trap_FS_FOpenFile( mapcfg, &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			//G_Printf( "Executing %s...\n", mapcfg );
			trap_SendConsoleCommand( EXEC_APPEND, va( "exec %s\n", mapcfg ) );
		}
	}
}

static const char *MonthAbbrev[] = {
	"Jan","Feb","Mar",
		"Apr","May","Jun",
		"Jul","Aug","Sep",
		"Oct","Nov","Dec"
};

static const char *DayAbbrev[] = {
	"Sun",
		"Mon","Tue","Wed",
		"Thu","Fri","Sat"
};

void G_ETF_LogParseString(const char* in, const char *mapname, int gameindex, char* out, int size) {
	qtime_t time;
	char tmp[2] = {0, 0};
	const char* p;
	char* c;
	int yr;

	trap_RealTime(&time);

	out[0] = 0;
	yr = time.tm_year;
	while(yr >= 100)
		yr-=100;

	for(p = in; *p; p++) {
		if(*p == '$') {
			p++;
			switch(*p) {
				case '\0':
					p--;
					break;
				case 'M':
					Q_strcat(out, size, MonthAbbrev[time.tm_mon]);
					break;
				case 'D':
					Q_strcat(out, size, DayAbbrev[time.tm_wday]);
					break;
				case 'Y':
					Q_strcat(out, size, va("%i", time.tm_year + 1900));
					break;

				case 'a':
					Q_strcat(out, size, time.tm_mon+1 >= 10 ? va("%i", time.tm_mon+1) : va("0%i", time.tm_mon+1));
					break;
				case 'd':
					Q_strcat(out, size, time.tm_mday >= 10 ? va("%i", time.tm_mday) : va("0%i", time.tm_mday));
					break;
				case 'y':
					Q_strcat(out, size, yr >= 10 ? va("%i", yr) : va("0%i", yr));
					break;

				case 'm':
					Q_strcat(out, size, time.tm_min >= 10 ? va("%i", time.tm_min) : va("0%i", time.tm_min));
					break;
				case 's':
					Q_strcat(out, size, time.tm_sec >= 10 ? va("%i", time.tm_sec) : va("0%i", time.tm_sec));
					break;
				case 'h':
					Q_strcat(out, size, time.tm_hour >= 10 ? va("%i", time.tm_hour) : va("0%i", time.tm_hour));
					break;

				case 'l':
					{
						char buffer[ MAX_STRING_CHARS ];
						Q_strncpyz( buffer, mapname, sizeof( buffer ) );
						Q_strcat( out, size, buffer );
					}
					break;

				case 'i':
					Q_strcat(out, size, va("%i", gameindex));
					break;

				case '$':
					tmp[0] = '$';
					Q_strcat(out, size, tmp);
					break;
			}
		} else {
			tmp[0] = *p;
			Q_strcat(out, size, tmp);
		}
	}

	for(c = out; *c; c++) {
		if(*c == '/' || *c == '\\') {
			*c = '_';
		}
	}
}

/*
============
G_InitGame

============
*/

/* Ensiform - Fixes the misc_beam on blue base not returning back on after blueflag returns to base */
//void G_Q3F_MuonFix( void );
void G_Q3F_OdiumFix( void );

char bigTextBuffer[100000];

void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;
	intptr_t 			index;
	char mappath[MAX_QPATH], buff[MAX_QPATH+4];
	q3f_array_t *mapList;
	q3f_data_t *data;
	char mapbuffer[4096] = { 0 };
	char value[MAX_CVAR_VALUE_STRING];
	char cs[MAX_INFO_STRING];

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitorInit("performance_game.log");
	BG_Q3F_PerformanceMonitor_LogFunction("G_InitGame");
#endif

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAME_VERSION);
	G_Printf ("gamedate: %s\n", __DATE__);
	G_Printf ("gameversion: %s\n", FORTS_VERSION);

	// extension interface
	trap_Cvar_VariableStringBuffer( "//trap_GetValue", value, sizeof( value ) );
	if ( value[0] ) {
		dll_com_trapGetValue = Q_atoi( value );
		if ( trap_GetValue( value, sizeof( value ), "engine_is_ete" ) ) {
			engine_is_ete = qtrue;
		}
		if ( trap_GetValue( value, sizeof( value ), "CVAR_NOTABCOMPLETE_ETE" ) ) {
			cvar_notabcomplete = Q_atoi( value );
		}
		if ( trap_GetValue( value, sizeof( value ), "CVAR_NODEFAULT_ETE" ) ) {
			cvar_nodefault = Q_atoi( value );
		}
		if ( trap_GetValue( value, sizeof( value ), "CVAR_ARCHIVE_ND_ETE" ) ) {
			cvar_archive_nd = Q_atoi( value );
		}
		if ( trap_GetValue( value, sizeof( value ), "CVAR_DEVELOPER_ETE" ) ) {
			cvar_developer = Q_atoi( value );
		}
		if ( trap_GetValue( value, sizeof( value ), "trap_FS_Delete" ) ) {
			dll_trap_FS_Delete = Q_atoi( value );
			deleteFile = qtrue;
		}
	}

	trap_Cvar_Set("g_heavyWeaponRestriction", FORTS_SHORTVERSION);

	srand( randomSeed );

	/* Ensiform - make sure etf_pak6.pk3 gets referenced on server so it's in the pure list */
	trap_FS_FOpenFile( "pak6.dat", &i, FS_READ );
	trap_FS_FCloseFile( i );

	G_InitMemory();

	G_RegisterCvars();

	trap_Cvar_Set("g_antilag", va("%i", g_gameindex.integer));

	G_ProcessIPBans();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	if ( restart ) {
		// Nuke the big buffer on restarts
		memset( bigTextBuffer, 0, sizeof(bigTextBuffer) );
	}

	trap_GetServerinfo( cs, sizeof( cs ) );
	Q_strncpyz( level.rawmapname, Info_ValueForKey( cs, "mapname" ), sizeof( level.rawmapname ) );

	// Golliwog: Load the current map's configuration, and map information for the current map.
	G_Q3F_LoadServerConfiguration( qfalse );									// Load main config
	G_Q3F_ExecuteSetting( level.rawmapname, g_gameindex.integer );						// Execute settings for the current config.
	level.mapInfo = G_Q3F_LoadMapInfo( level.rawmapname );								// Load associated map information file.
	G_Q3F_CheckGameIndex();														// Check current gameindex is valid.
	Com_sprintf( buff, sizeof(buff), "%s+%d", level.rawmapname, g_gameindex.integer );
	G_Q3F_UpdateMapHistory( buff );												// Bump the map history.

	G_Printf ("mapname: %s, gameindex: %d\n", level.rawmapname, g_gameindex.integer );

	trap_Cvar_Set("g_maxlives", va("%i", trap_Cvar_VariableIntegerValue("sv_pure")));

	// build a configstring for the available map list
	mapbuffer[0] = '\0';
	mapList = G_Q3F_GetAvailableMaps();
	for( index = -1; (data = G_Q3F_ArrayTraverse( mapList, &index )) != NULL; ) {
		Q_strcat(mapbuffer, 4096, va("%s;", data->d.strdata));
	}
	trap_SetConfigstring(CS_MAPLIST, mapbuffer);
	trap_SetConfigstring(CS_INTERMISSION, "0");

	// Override potential CS_WOLFINFO (21)
	trap_SetConfigstring( CS_FORTS_TEAMPINGS, "0 0 0 0 0 0 0 0 0" );

	G_Q3F_UnloadServerConfiguration();											// Unload the config again.
	if( g_serverConfigMap.string[0] && Q_stricmp( g_serverConfigMap.string, buff ) )
	{
		// The map needs to be re-run to update latched variables.

		trap_Cvar_Set( "g_serverConfigMap", buff );
		// RR2DO2: cheats default to 1 in the engine. BLEH
		//trap_SendConsoleCommand( EXEC_APPEND, va( "%s %s\n", ( g_cheats.integer ? "devmap" : "map" ), level.rawmapname ) );
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", level.rawmapname ) );
	}
	else {
		// Unlock the config time so the next schedule can be selected properly.
		trap_Cvar_Set( "g_serverConfigTime", "" );
	}
	// Golliwog.

	level.snd_fry = G_SoundIndex("sound/world/lava_short.wav");	// FIXME standing in lava / slime

#ifdef DEBUG_MEM
	G_MemDebug_Init();
#endif
#if 0
	G_VersionDebug_Init();
#endif

	if ( g_logFile.string[0] ) {
		char buffer[MAX_CVAR_VALUE_STRING];
		G_ETF_LogParseString(g_logFile.string, level.rawmapname, g_gameindex.integer, buffer, MAX_CVAR_VALUE_STRING);
		if ( g_logSync.integer ) {
			trap_FS_FOpenFile( buffer, &level.logFile, FS_APPEND_SYNC );
		} else {
			trap_FS_FOpenFile( buffer, &level.logFile, FS_APPEND );
		}
		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", buffer );
		} else {
			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", cs );
			G_LogPrintf("Mapname: %s, gameindex: %d\n", level.rawmapname, g_gameindex.integer );
		}
	} else {
		G_Printf( "Not logging to disk.\n" );
	}

	G_ReadSessionData();

	// Load Camera Data
	Com_sprintf( mappath, sizeof(mappath), "maps/%s", level.rawmapname );
	level.camNumPaths = BG_Q3F_LoadCamPaths( mappath, (void *)level.campaths );	
	level.flybyPathIndex = BG_Q3F_LocateFlybyPath( level.camNumPaths, (void *)level.campaths );

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i=0 ; i<level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// Ensiform: Ensures we always have a non-null classname
	for ( i=0 ; i<MAX_CLIENTS ; i++ ) {
		g_entities[i].classname = "clientslot";
	}

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ), 
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// Setup the team data before parsing worldspawn.
	G_Q3F_InitTeams();

#ifdef BUILD_LUA
	G_LuaInit();
#endif // BUILD_LUA

	// parse the key/value pairs and spawn gentities
	level.ctfcompat = qfalse;
	level.wfacompat = qfalse;
	G_SpawnEntitiesFromString();

	/* Ensiform - Fixes the misc_beam on blue base not returning back on after blueflag returns to base */
	//if(!Q_stricmp(level.rawmapname, "etf_muon"))
	//	G_Q3F_MuonFix();

	if(!Q_stricmp(level.rawmapname, "etf_odium"))
		G_Q3F_OdiumFix();

	G_Q3F_CTFCompatAdjust();

	G_Q3F_SetFlagHudTimers();
	G_Q3F_SetSecurityHudTimers();

	// Sort the location data
	G_Q3F_LocationSort();
	G_Q3F_WaypointBuildArray();

	// Spawn additional speakers from soundscript
	G_Q3F_SSCR_ParseSoundScript( mappath );

	// Find out what teams are allowed to play by checking spawnpoints,
	// And then make the data available to the client for cacheing purposes.
	G_Q3F_SetAllowedTeams();
	G_Q3F_SetAlliedTeams();
	G_Q3F_SetClassMaskString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	G_CheckTeamItems();

	// Golliwog: Clean up a little.
	G_Q3F_NoBuildFinish();
	G_Q3F_NoAnnoyFinish();
	G_Q3F_LinkOnKillChain();

	G_Q3F_MapHasNoAnnoys(); // Sets the qboolean

	SaveRegisteredItems();

	G_Printf ("-----------------------------------\n");

	//G_SoundIndex("sound/items/use_medikit.wav");			// JT: Make sure medikit noise is available

	// Attempt to clean up our memory pool for the game proper.
	G_Q3F_KeyPairArrayDestroy( level.mapInfo );
	G_DefragmentMemory();
	level.mapInfo = NULL;

	trap_Cvar_Set( "players_red", "" );
	trap_Cvar_Set( "players_blue", "" );
	trap_Cvar_Set( "players_yellow", "" );
	trap_Cvar_Set( "players_green", "" );
	// RR2DO2

#ifdef DEBUGLOG
	G_DebugLog( "Started map.\n" );
#endif

	G_EventLog_Init();

#ifdef BUILD_LUA
	G_LuaHook_InitGame( levelTime, randomSeed, restart );
#endif // BUILD_LUA

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) 
{
#ifdef BUILD_LUA
	G_LuaHook_ShutdownGame( restart );
	G_LuaShutdown();
#endif // BUILD_LUA

#ifdef DEBUG_MEM
	G_MemDebug_Close();
#endif

	G_Printf ("==== ShutdownGame ====\n");

	if ( level.logFile ) {
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
		level.logFile = NULL_FILE;
	}

	G_EventLog_Shutdown();

	if (!restart) 
	{
		G_SetMatchState( MATCH_STATE_NORMAL );
		level.warmupTime = 0;
	}
	trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );

	// write all the client session data so we can get it back
	G_WriteSessionData();

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitorShutdown();
#endif
}

//===================================================================

#define G_MAXPRINTMSG 8192
void FORMAT_PRINTF(1,2) QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[G_MAXPRINTMSG];

	if ( !trap_Cvar_VariableIntegerValue("developer") ) {
		return;
	}

	va_start( argptr, fmt );
	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	trap_Print( msg );
}

void QDECL FORMAT_PRINTF(2,3) Com_Error ( int _level, const char *error, ... ) {
	va_list		argptr;
	char		text[G_MAXPRINTMSG];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL FORMAT_PRINTF(1,2) Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[G_MAXPRINTMSG];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
SortRanks
=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	const gclient_t	*ca, *cb;

	ca = &level.clients[*(const int *)a];
	cb = &level.clients[*(const int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}

	// then spectators
	if ( ca->sess.sessionTeam == Q3F_TEAM_SPECTATOR && cb->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		if ( ca->pers.enterTime < cb->pers.enterTime ) {
			return -1;
		}
		if ( ca->pers.enterTime > cb->pers.enterTime ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		return -1;
	}

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		> cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		< cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int			i;
	gclient_t	*cl;
	//char		buff[512];

	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
				level.numNonSpectatorClients++;
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {
					level.numPlayingClients++;
					level.numVotingClients++;
				}
			}
		}
	}

	qsort( level.sortedClients, level.numConnectedClients, 
		sizeof(level.sortedClients[0]), SortRanks );

	// set the rank value for all teams that are connected
	for ( i = 0;  i < level.numConnectedClients; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];
		if ( level.teamScores[Q3F_TEAM_RED] == level.teamScores[Q3F_TEAM_BLUE] ) {
			cl->ps.persistant[PERS_RANK] = 2;
		} else if ( level.teamScores[Q3F_TEAM_RED] > level.teamScores[Q3F_TEAM_BLUE] ) {
			cl->ps.persistant[PERS_RANK] = 0;
		} else {
			cl->ps.persistant[PERS_RANK] = 1;
		}
	}

	/*memset( buff, 0, sizeof(buff) );
	if( g_q3f_allowedteams & (1 << Q3F_TEAM_RED) )
		Q_strcat( buff, sizeof(buff), va("\\r\\%i", level.teamScores[Q3F_TEAM_RED] ) );
	else
		Q_strcat( buff, sizeof(buff), va("\\r\\%i", SCORE_NOT_PRESENT ) );

	if( g_q3f_allowedteams & (1 << Q3F_TEAM_BLUE) )
		Q_strcat( buff, sizeof(buff), va("\\b\\%i", level.teamScores[Q3F_TEAM_BLUE] ) );
	else
		Q_strcat( buff, sizeof(buff), va("\\b\\%i", SCORE_NOT_PRESENT ) );

	if( g_q3f_allowedteams & (1 << Q3F_TEAM_YELLOW) )
		Q_strcat( buff, sizeof(buff), va("\\y\\%i", level.teamScores[Q3F_TEAM_YELLOW] ) );
	else
		Q_strcat( buff, sizeof(buff), va("\\y\\%i", SCORE_NOT_PRESENT ) );

	if( g_q3f_allowedteams & (1 << Q3F_TEAM_GREEN) )
		Q_strcat( buff, sizeof(buff), va("\\g\\%i", level.teamScores[Q3F_TEAM_GREEN] ) );
	else
		Q_strcat( buff, sizeof(buff), va("\\g\\%i", SCORE_NOT_PRESENT ) );

	trap_SetConfigstring( CS_Q3F_SCORES, buff );*/

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			level.clients[i].wantsscore = qtrue;
		}
	}
}

/*
==================
Cmd_Awards_f

Request current statistics information
==================
*/
void Cmd_Awards_f( void ) {
	int			stringlength;
	int			i, j;
	gentity_t	*gent;
	int			teamkills;
	int			flagcaps;
	int			given[STATS_NUM];
	char		givers[STATS_NUM][128];
	char		teamkiller[128], capper[128];
	char		terminator[128], cannonfodder[128];
	int			kills, deaths;
	int			maxkills, maxdeaths;

	// send the latest information on all clients
	stringlength = 0;
	teamkills = -1;
	flagcaps = -1;

	memset(givers, 0, sizeof(givers));
	memset(given, 0, sizeof(given));
	teamkiller[0] = 0;
	capper[0] = 0;
	terminator[0] = 0;
	cannonfodder[0] = 0;
	maxkills = 0;
	maxdeaths = 0;

	for (i=0 ; i < MAX_CLIENTS ; i++) {
		gent = &g_entities[i];
		if(gent->inuse) {
			if(gent->client->pers.stats.teamkills && (gent->client->pers.stats.teamkills >= teamkills)) {
				if(gent->client->pers.stats.teamkills > teamkills) {
					teamkills = gent->client->pers.stats.teamkills;
					teamkiller[0] = 0;
				}
				stringlength = strlen(teamkiller);
				if(stringlength)
					Q_strcat(teamkiller, 128, va("^7, %s",gent->client->pers.netname));
				else
					Q_strncpyz(teamkiller, gent->client->pers.netname, 128);
			}

			if(gent->client->pers.stats.caps && (gent->client->pers.stats.caps >= flagcaps)) {
				if(gent->client->pers.stats.caps > flagcaps) {
					flagcaps = gent->client->pers.stats.caps;
					capper[0] = 0;
				}
				stringlength = strlen(capper);
				if(stringlength)
					Q_strcat(capper, 128, va("^7,%s",gent->client->pers.netname));
				else
					Q_strncpyz(capper, gent->client->pers.netname, 128);
			}

			kills = deaths = 0;

			for(j = 0; j < STATS_NUM; j++) {
				kills += gent->client->pers.stats.data[j].kills;
				deaths += gent->client->pers.stats.data[j].deaths;

				if(gent->client->pers.stats.data[j].given && (gent->client->pers.stats.data[j].given >= given[j]))
				{
					if(gent->client->pers.stats.data[j].given > given[j]) {
						given[j] = gent->client->pers.stats.data[j].given;
						givers[j][0] = 0;
					}
					stringlength = strlen(givers[j]);
					if(stringlength)
						Q_strcat(givers[j], 128, va("^7, %s",gent->client->pers.netname));
					else
						Q_strncpyz(givers[j], gent->client->pers.netname, 128);
				}
			}

			if(kills && (kills >= maxkills)) {
				if(kills > maxkills) {
					maxkills = kills;
					terminator[0] = 0;
				}
				stringlength = strlen(terminator);
				if(stringlength)
					Q_strcat(terminator, 128, va("^7, %s",gent->client->pers.netname));
				else
					Q_strncpyz(terminator, gent->client->pers.netname, 128);
			}

			if(deaths && (deaths >= maxdeaths)) {
				if(deaths > maxdeaths) {
					maxdeaths = deaths;
					cannonfodder[0] = 0;
				}
				stringlength = strlen(cannonfodder);
				if(stringlength)
					Q_strcat(cannonfodder, 128, va("^7, %s",gent->client->pers.netname));
				else
					Q_strncpyz(cannonfodder, gent->client->pers.netname, 128);
			}
		}
	}

	// build the award string
	level.awards[0] = 0;

	if(teamkills > 0)
		Com_sprintf(level.awards, 1400, "\"%s with %d kills\"", teamkiller, teamkills);
	else
		Com_sprintf(level.awards, 1400, "\"\"");

	if(flagcaps > 0)
		Q_strcat(level.awards, 1400, va(" \"%s with %d caps\"", capper, flagcaps) );
	else
		Q_strcat(level.awards, 1400, " \"\"");

	Q_strcat(level.awards, 1400, va(" \"%s with %d kills\"", terminator, maxkills) );
	Q_strcat(level.awards, 1400, va(" \"%s with %d deaths\"", cannonfodder, maxdeaths) );

	for(j = 0; j < STATS_NUM; j++) {
		Q_strcat(level.awards, 1400, va(" \"%s\"", givers[j]) );
	}

	for (i=0 ; i < MAX_CLIENTS ; i++) {
		gent = &g_entities[i];
		if(gent->inuse)
			trap_SendServerCommand( i, va("awards %s", level.awards ) );
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_CHASE ) {
		StopFollowing( ent, qtrue );
	}


	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );
	// clean up events
	ent->s.events[0] = ent->s.events[1] = ent->s.events[2] = ent->s.events[3] = 0;

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_INVISIBLE;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent ) {	// the map creator forgot to put in an intermission point...
		SelectSpawnPoint ( vec3_origin, level.intermission_origin, level.intermission_angle, NULL );
	} else {
		VectorCopy (ent->s.origin, level.intermission_origin);
		VectorCopy (ent->s.angles, level.intermission_angle);
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int			i;
	gentity_t	*client;
	g_q3f_playerclass_t *cls;

	if ( level.intermissiontime ) {
		return;		// already active
	}

	// Golliwog: Start the map cycle if desired
	//if( level.mapSelectState == Q3F_MAPSELECT_NONE && g_mapVote.integer )
	//	G_Q3F_MapSelectInit();

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	Cmd_Awards_f();

	// move all clients to the intermission point
	for (i=0 ; i< level.maxclients ; i++) {
		client = g_entities + i;
		if (!client->inuse)
			continue;
		// respawn if dead
		if (client->health <= 0) {
			respawn(client);
		}
		G_Q3F_DropAllFlags( client, qtrue, qtrue );	// Golliwog: Drop any flags carried.
		cls = G_Q3F_GetClass( &client->client->ps );
		if( cls->DeathCleanup )
			cls->DeathCleanup( client );
		if( cls->EndClass )
			cls->EndClass( client );		// Perform class term stuff (e.g. destroy sentries)
		G_Q3F_GenericEndCleanup( client );
		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
//	SendScoreboardMessageToAllClients();
}
void G_Q3F_CeaseFire(qboolean state) {


}

void G_Q3F_RestartMap(void) {
	trap_Cvar_Set( "g_restarted", "1" );
	trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
}

/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar 

=============
*/
void ExitLevel (void) {
	int		i;
	gclient_t *cl;

	// RR2DO2
	if( level.mapSelectState == Q3F_MAPSELECT_NONE && g_mapVote.integer ) {
		G_Q3F_MapSelectInit();
		return;
	} else if( level.mapSelectState != Q3F_MAPSELECT_NONE && level.mapSelectState != Q3F_MAPSELECT_CHANGEMAP )
		return;
	// RR2DO2

	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	//level.changemap = NULL;
	level.intermissiontime = 0;
	level.nextMapTime = level.time + 500;		// Golliwog: Allow half a second to switch the map

	// reset all the scores so we don't enter the intermission again
	level.teamScores[Q3F_TEAM_RED] = 0;
	level.teamScores[Q3F_TEAM_BLUE] = 0;
	level.teamScores[Q3F_TEAM_YELLOW] = 0;
	level.teamScores[Q3F_TEAM_GREEN] = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	trap_SetConfigstring( CS_INTERMISSION, "0" );

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for (i=0 ; i< g_maxclients.integer ; i++) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL FORMAT_PRINTF(1,2) G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;
	int			offset;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );
	offset = strlen(string);

	va_start( argptr, fmt );
	Q_vsnprintf( string + offset, sizeof(string) - offset, fmt, argptr );
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + offset );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int				i, numSorted;
	gclient_t		*cl;
//	qboolean		won = qtrue;

	G_LogPrintf( "Exit: %s\n", string );

	if ( g_matchState.integer ) {
		G_LogPrintf( "Match has ended.");
		G_SetMatchState( MATCH_STATE_NORMAL );
		level.warmupTime = 0;
		trap_SetConfigstring( CS_WARMUP, "" );
	}

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, va( "%i", level.time ) );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	for (i=Q3F_TEAM_FREE + 1; i < Q3F_TEAM_SPECTATOR; i++)
	{
		if( g_q3f_allowedteams & (1 << i) )	// is this team enabled?
			G_LogPrintf( "%s:%i  ", g_q3f_teamlist[i].name, level.teamScores[i] );
	}
	// close the line
	G_LogPrintf( "\n" );
	// RR2DO2

	// NERVE - SMF - send gameCompleteStatus message to master servers
	trap_SendConsoleCommand( EXEC_APPEND, "gameCompleteStatus\n" );

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		//if ( cl->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		if ( Q3F_IsSpectator(cl) ) {	// RR@DO2
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s^7\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],	cl->pers.netname );
	}
}

static void G_ETF_ReloadMap(void)
{
	// Golliwog: 'failsafe' map restart

	char buff[1024];
	fileHandle_t fh;

	Com_sprintf( buff, sizeof(buff), "maps/%s.bsp", level.rawmapname );
	if( trap_FS_FOpenFile( buff, &fh, FS_READ ) < 0 )
	{
		// Map doesn't exist?

		Q_strncpyz( buff, "maps/etf_forts.bsp", sizeof(buff) );	// Fallback to forts in an emergency
		if( trap_FS_FOpenFile( buff, &fh, FS_READ ) < 0 )
			G_Error( "G_ETF_ReloadMap: No map to reload." );

		trap_FS_FCloseFile( fh );

		trap_SendConsoleCommand( EXEC_APPEND, "map etf_forts\n" );
		level.nextMapTime = level.time + 1000;
		return;
	}
	trap_FS_FCloseFile( fh );

	trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", level.rawmapname ) );
	level.nextMapTime = level.time + 1000;
}

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	// Golliwog: Check the nextmap hasn't been screwed
	if( level.nextMapTime && level.nextMapTime <= level.time )
	{
		G_ETF_ReloadMap();
		return;
	}
	// Golliwog.

	if( !level.numPlayingClients )
	{
		// No clients, cycle immediately.

		ExitLevel();
		return;
	}

	// Golliwog: Don't exit if map voting is in progress
	if( level.mapSelectState != Q3F_MAPSELECT_NONE )
		return;

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited six seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 2000 ) {
		return;
	}

	ExitLevel();
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
//	int			i;
//	gclient_t	*cl;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit ();
		return;
	}

	if ( level.intermissionQueued ) {
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}

	if ( g_timelimit.integer && g_matchState.integer <= MATCH_STATE_PLAYING ) {
		if( g_timelimit.integer >= 999 ) {
			trap_Cvar_Set( "timelimit", "999" );
		}
		if ( level.time - level.startTime >= g_timelimit.integer*60000 ) {
			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"");
			LogExit( "Timelimit hit." );
			return;
		}
	}

#if 0
	if ( level.numPlayingClients < 2 ) {
		return;
	}
#endif

	if ( g_capturelimit.integer && g_matchState.integer <= MATCH_STATE_PLAYING ) {

		if ( level.teamScores[Q3F_TEAM_RED] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[Q3F_TEAM_BLUE] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[Q3F_TEAM_YELLOW] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Yellow hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
		if ( level.teamScores[Q3F_TEAM_GREEN] >= g_capturelimit.integer ) {
			trap_SendServerCommand( -1, "print \"Green hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}


/*
=================
G_DebugLog

Print to the debug logfile
=================
*/
void QDECL G_DebugLog( const char *fmt, ... )
{
#ifdef DEBUGLOG
	va_list		argptr;
	char		string[8196];
	int			min, tens, sec;
	fileHandle_t fh;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf( string + 7, sizeof(string) - 7, fmt, argptr );
	va_end( argptr );

	if( trap_FS_FOpenFile( "game_debug.log", &fh, FS_APPEND_SYNC ) < -1 )
		return;
	trap_FS_Write( string, strlen( string ), fh );
	trap_FS_FCloseFile( fh );
#endif
}

/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckMatches

Once a frame, check for changes in tournement player state
=============
*/
static void CheckMatches( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( !g_matchState.integer )
		return;

	if ( !level.numPlayingClients )
		return;

	if (g_matchState.integer == MATCH_STATE_WAITING) {

		if ( level.numPlayingClients < 2)
			return;
		G_SetMatchState( MATCH_STATE_READYUP );
	}

	if (g_matchState.integer == MATCH_STATE_READYUP) {
		if ( g_matchMode.integer & MATCH_MODE_NOREADYUP ) {
			G_SetMatchState( MATCH_STATE_WARMUP );
			level.warmupTime = level.time + g_warmup.integer * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		}
		if( !CheckEveryoneReady() ) {
			if( !(level.time % 10000) ) {	// display message every 10 seconds
				char names[256];
				NotReadyNames( names, sizeof(names) );
				WarnNotReadyClients();
				trap_SendServerCommand( -1, va( "print \"Waiting for %s to ready up...\n\"", names ) );
				trap_SetConfigstring( CS_WARMUP, "-2" );
			}
			return;
		} else {
			G_SetMatchState( MATCH_STATE_WARMUP );
			level.warmupTime = level.time + g_warmup.integer * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		}
	}

	// Check for direct g_warmup changes
	if (g_matchState.integer == MATCH_STATE_WARMUP) {
		if ( g_warmup.modificationCount != level.warmupModificationCount ) {
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = level.time + g_warmup.integer * 1000;
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		}
		if ( level.time >= level.warmupTime ) {
			level.warmupTime = 0;
			trap_SetConfigstring( CS_WARMUP, "" );
			G_SetMatchState( MATCH_STATE_PLAYING );
			G_Q3F_RestartMap();
		}
	}
}

static void G_Q3F_ExecuteVotestring( void ) {
	char cmdbuff[MAX_STRING_CHARS];
	char databuff[MAX_STRING_CHARS];
	char *ptr;
	int spaceindex;

	spaceindex = 0;
	for( ptr = level.voteString; *ptr && *ptr != ' '; ptr++ ) {
		cmdbuff[spaceindex++] = *ptr;
	}
	cmdbuff[spaceindex++] = '\0';

	// level.voteString + spaceindex is now the start of the arguments
	Q_strncpyz( databuff, level.voteString + spaceindex, sizeof(databuff) );

	if( !Q_stricmp( cmdbuff, "ceasefire" ) ) {
		if( !Q_stricmp( databuff, "\"off\"" ) ) {
			if( level.ceaseFire ) {
				level.ceaseFire = qfalse;
				trap_SetConfigstring( CS_FORTS_CEASEFIRE, "0" );
				trap_SendServerCommand( -1, "print \"Ceasefire off\n\"" );
			}
		} else {
			if( !level.ceaseFire ) {
				trap_SetConfigstring( CS_FORTS_CEASEFIRE, "1" );
				trap_SendServerCommand( -1, "print \"Ceasefire on\n\"" );
				level.ceaseFire = qtrue;
			}
		}
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote( void ) {
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) {
		level.voteExecuteTime = 0;
		G_Q3F_ExecuteVotestring(); //trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}
	//if ( 1 ) {//!level.voteTime ) {
	if ( !level.voteTime ) {
		return;
	}
	if ( level.time - level.voteTime >= VOTE_TIME ) {
		// Timed out, check we have a reasonable number of votes, before proceeding.
		if(	level.voteYes + level.voteNo >= level.numVotingClients / 3 &&
			level.voteYes > level.voteNo )
		{
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			level.voteExecuteTime = level.time + 3000;
		}
		else {
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		}
	} else {
		if ( level.voteYes > level.numVotingClients/2 ) {
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			G_Q3F_ExecuteVotestring();
		} else if ( level.voteNo >= level.numVotingClients/2 ) {
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		} else {
			// still waiting for a majority
			return;
		}
	}
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
==================
CountPipes
==================
*/
static void CountPipes( void ) {
	int i;
	int	teamcount[Q3F_TEAM_NUM_TEAMS];
	gentity_t *cl;

	// team pipe count
	// have to do this one somewhat different
	memset(teamcount, 0, sizeof(teamcount));
	for(i = 0; i < MAX_CLIENTS; i++) {
		cl = &g_entities[i];
		if(cl->client && cl->inuse && (cl->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_GRENADIER)) {
			teamcount[cl->client->ps.persistant[PERS_TEAM]] += cl->client->pipecount;
		}
	}

	for(i = 0; i < MAX_CLIENTS; i++) {
		cl = &g_entities[i];
		if(cl->client && cl->inuse && (cl->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_GRENADIER) ) {
			cl->client->ps.ammoclip[1] = teamcount[cl->client->ps.persistant[PERS_TEAM]];
		}
	}
}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand( i, message );
	}
}

/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent) {
	float	thinktime;

	thinktime = ent->nextthink;
	if (thinktime <= 0) {
		return;
	}
	if (thinktime > level.time) {
		return;
	}
	
	ent->nextthink = 0;
	if (!ent->think) {
		G_DumpEntityInfo( ent, qtrue, "", "NULL ent->think", qfalse );
		G_Error ( "NULL ent->think");
	} else {
		ent->think (ent);
	}
}

/*
================
G_Q3F_CalculateTeamPings

Work out average team pings, send to all clients as a string
================
*/
void G_Q3F_CalculateTeamPings(void)
{
	int pings[Q3F_TEAM_NUM_TEAMS], counts[Q3F_TEAM_NUM_TEAMS], index;
	gentity_t *scan;
	int warningmask;
	int lowcount, highcount, lowping, highping;
	int lowcountindex, highcountindex, lowpingindex, highpingindex;
	int nextlowcount, nexthighcount, nextlowping, nexthighping;

	if( level.teamPingTime > level.time )
		return;
	level.teamPingTime = level.time + 2500;

	lowcount = lowping = nextlowcount = nextlowping = 10000;
	highcount = highping = nexthighcount = nexthighping = -1;
	lowpingindex = highpingindex = lowcountindex = highcountindex = 0;

	for( index = 0; index < Q3F_TEAM_NUM_TEAMS; index++ )
		pings[index] = counts[index] = 0;
	for( scan = g_entities; scan < &g_entities[MAX_CLIENTS]; scan++ )
	{
		if( !scan->inuse || !scan->client )
			continue;
		counts[scan->client->sess.sessionTeam]++;
		pings[scan->client->sess.sessionTeam] += scan->client->ps.ping;
	}

	for( index = 0; index < Q3F_TEAM_NUM_TEAMS - 1; index++ )
	{
		// Get ping, update low and high values for warnings.

		if( !counts[index] )
			continue;

		pings[index] /= counts[index];

		if( counts[index] < g_q3f_teamlist[index].playerlimit || g_q3f_teamlist[index].playerlimit < 0 )
		{
			// We only care about low counts if the team isn't at the limit
			if( counts[index] < lowcount )
			{
				nextlowcount = lowcount;
				lowcount = counts[index];
				lowcountindex = index - Q3F_TEAM_RED + 1;
			}
			else if( counts[index] < nextlowcount )
				nextlowcount = counts[index];
		}
		if( counts[index] > highcount )
		{
			nexthighcount = highcount;
			highcount = counts[index];
			highcountindex = index - Q3F_TEAM_RED + 1;
		}
		else if( counts[index] > nexthighcount )
			nexthighcount = counts[index];

		if( pings[index] < lowping )
		{
			nextlowping = lowping;
			lowping = pings[index];
			lowpingindex = index - Q3F_TEAM_RED + 1;
		}
		else if( pings[index] < nextlowping )
			nextlowping = pings[index];
		if( pings[index] > highping )
		{
			nexthighping = highping;
			highping = pings[index];
			highpingindex = index - Q3F_TEAM_RED + 1;
		}
		else if( pings[index] > nexthighping )
			nexthighping = pings[index];
	}

	warningmask = 0;
	if( lowcount < nextlowcount - 1 && nextlowcount < 10000 )
		warningmask |= 0x0007 & lowcountindex;				// We have a low count
	if( highcount > nexthighcount + 1 && nextlowcount != counts[highcountindex] && nexthighcount > -1 )
		warningmask |= 0x00F8 & (highcountindex << 4);		// We have a high count
	if( (lowping + 80) < (nextlowping * 0.8) && nexthighping != pings[lowpingindex] && nexthighping > -1 )
		warningmask |= 0x0700 & (lowpingindex << 8);		// We have a low ping
	if( (highping * 0.8) > (nexthighping + 80) && nextlowping < 10000 )
		warningmask |= 0xF800 & (highpingindex << 12);		// We have a high ping (30% diff)

	trap_SetConfigstring(	CS_FORTS_TEAMPINGS, va( "%i %i %i %i %i %i %i %i %i",
							counts[Q3F_TEAM_RED],		pings[Q3F_TEAM_RED],
							counts[Q3F_TEAM_BLUE],		pings[Q3F_TEAM_BLUE],
							counts[Q3F_TEAM_YELLOW],	pings[Q3F_TEAM_YELLOW],
							counts[Q3F_TEAM_GREEN],		pings[Q3F_TEAM_GREEN],
							warningmask ) );

	if( (warningmask & 0xFF) && (warningmask & 0xFF) != level.teamPreviousBalanceWarning && g_showTeamBalanceWarning.integer )
	{
//		G_LogPrintf(	"CalculateTeamPings: %d/%d-%d %d/%d-%d/%d : %d/%d-%d %d/%d-%d/%d.",
//						lowcountindex,	lowcount, nextlowcount, highcountindex, highcount, nexthighcount,
//						lowpingindex,	lowping, nextlowping, highpingindex, highping, nexthighping );
		level.teamPreviousBalanceWarning = (warningmask & 0xFF);
		trap_SendServerCommand( -1, va( "cp \"Warning, %s have too %s players.\n\"",
			g_q3f_teamlist[((warningmask & 0x07) ? lowcountindex : highcountindex) + Q3F_TEAM_RED - 1].description,
			(warningmask & 0x07) ? "few" : "many" ) );
	}
}

/*static void G_Q3F_CheckCorruptClientSlots(void)
{
	// Check all empty client slot data, and if they're not all zero, dump them - there's
	// obviously some kind of memory corruption going on.

	int index, checkSize;
	char *checkPtr;
	qhandle_t fhandle;

	for( index = 0; index < MAX_CLIENTS; index++ )
	{
		if( level.connectedClients[index] )
			continue;
		for( checkSize = sizeof(gclient_t), checkPtr = (char *) (level.clients + index); checkSize; checkSize-- )
		{
			if( *checkPtr++ )
				break;
		}
		if( checkSize > 0 )
		{
			// Argh.

			if( trap_FS_FOpenFile( va( "dump/clientdump_%d_%d.cfg", index, level.time ), &fhandle, FS_WRITE ) < 0 )
				continue;
			trap_FS_Write( level.clients + index, sizeof(gclient_t), fhandle );
			trap_FS_FCloseFile( fhandle );
		}
	}
}*/

void G_RunEntities(void) {
	gentity_t* ent;
	int i,e;
	trace_t trace;
	int projectile_now;

	//
	// go through all allocated objects
	//
	
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse ) {
			continue;
		}

		// ydnar: hack for instantaneous velocity
		VectorCopy( ent->r.currentOrigin, ent->oldOrigin );

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			ent->s.event = 0;
			ent->s.eventSequence = 0;
			for (e=0; e<MAX_EVENTS; e++)
				ent->s.events[e] = 0;
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		// Golliwog: We need to let our entities pass back to the 'inactive' state
		if(	ent->mapdata &&
			ent->mapdata->state == Q3F_STATE_ACTIVE &&
			ent->mapdata->waittime > 0 &&
			ent->mapdata->waittime <= level.time )
			G_Q3F_TriggerEntity( ent, ent->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, &trace, qtrue );
		// Golliwog.

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}


//unlagged - backward reconciliation #2
		// we'll run missiles separately to save CPU in backward reconciliation
		if ( ent->s.eType == ET_MISSILE )
			continue;
//unlagged - backward reconciliation #2

		// Golliwog: Custom types
		if ( ent->s.eType == ET_Q3F_GRENADE ) {
			G_Q3F_RunGrenade( ent );
			continue;
		}

		if ( ent->s.eType == ET_Q3F_GOAL ) {
			G_Q3F_RunGoal( ent );
			continue;
		}

		if( ent->s.eType == ET_Q3F_AGENTDATA || ent->s.eType == ET_Q3F_SCANNERDATA) {
			G_Q3F_RunAgentData( ent );
			continue;
		}

		if( ent->s.eType == ET_SNIPER_DOT ) {
			G_Q3F_RunSniperDot( ent );
			continue;
		}

		if( ent->s.eType == ET_Q3F_SENTRY ) {
			G_Q3F_RunSentry( ent );
			continue;
		}

		if( ent->s.eType == ET_Q3F_SUPPLYSTATION ) {
			G_Q3F_RunSupplystation( ent );
			continue;
		}

		if( ent->s.eType == ET_Q3F_MAPSENTRY ) {
			G_Q3F_RunMapSentry( ent );
			continue;
		}
		// Golliwog.

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER ) {
			G_RunMover( ent );
			continue;
		}

		if ( ent->s.eType == ET_PORTAL && !ent->message) {
			G_RunPortal( ent );
			continue;
		}

		if ( ent->s.eType == ET_Q3F_TELEPORTTRANSITION ) {
			G_RunTeleportTransition( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}
//unlagged - backward reconciliation #3
	// NOW run the missiles, with all players backward-reconciled
	// to the positions they were at the end of the last server frame
	projectile_now = level.previousTime;
	G_TimeShiftAllClients( projectile_now, NULL );

	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++) {
		if ( !ent->inuse || ent->freeAfterEvent)
			continue;

		switch (ent->s.eType) {
		case ET_MISSILE: {
			if (ent->antilag_time) {
				int snap_ms = 1000 / sv_fps.integer;
				int prestep = ent->s.pos.trTime - projectile_now;
				int steps = (projectile_now - ent->antilag_time) / snap_ms, rs = 0;
				ent->s.pos.trTime = ent->antilag_time + prestep;

					while (steps > 0) {
						int now = projectile_now - steps * snap_ms;
						G_TimeShiftAllClients(now, ent->parent);
						G_RunMissile(ent, now);

						if (!ent->inuse)
							break;

						rs = 1;
						steps--;
					}
					ent->antilag_time = 0;
					if (rs)
						G_TimeShiftAllClients( projectile_now, NULL );
				}

				if (ent->inuse)
					G_RunMissile( ent, level.time );
				break;
			}
		}
	}

	G_UnTimeShiftAllClients( NULL );
//unlagged - backward reconciliation #2
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i, j;
	gentity_t	*ent;
	//int			msec;
	char teamstrings[4][MAX_CVAR_VALUE_STRING];

	level.frameTime = trap_Milliseconds();

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunction("G_RunFrame");
#endif

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	//msec = level.time - level.previousTime;

	// get any cvar changes
#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunction("G_UpdateCvars");
#endif
	G_UpdateCvars();
#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif

	// Golliwog: Expire any bans
	if( g_q3f_banCheckTime && g_q3f_banCheckTime <= level.time ) {
#ifdef PERFLOG
		BG_Q3F_PerformanceMonitor_LogFunction("G_Q3F_AdminNextExpireBans");
#endif
		g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
#ifdef PERFLOG
		BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif
	}

#if 0
	// Ensiform: Expire any mutes
	if( g_q3f_muteCheckTime && g_q3f_muteCheckTime <= level.time ) {
#ifdef PERFLOG
		BG_Q3F_PerformanceMonitor_LogFunction("G_Q3F_AdminNextExpireMutes");
#endif
		g_q3f_muteCheckTime = G_Q3F_AdminNextExpireMutes();
#ifdef PERFLOG
		BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif
	}
#endif

	// RR2DO2: Set the cvars for the server browser
	if( score_red.value != level.teamScores[Q3F_TEAM_RED] )
		trap_Cvar_Set( "score_red", va("%i",level.teamScores[Q3F_TEAM_RED]));
	if( score_blue.value != level.teamScores[Q3F_TEAM_BLUE] )
		trap_Cvar_Set( "score_blue", va("%i",level.teamScores[Q3F_TEAM_BLUE]));
	if( score_yellow.value != level.teamScores[Q3F_TEAM_YELLOW] )
		trap_Cvar_Set( "score_yellow", va("%i",level.teamScores[Q3F_TEAM_YELLOW]));
	if( score_green.value != level.teamScores[Q3F_TEAM_GREEN] )
		trap_Cvar_Set( "score_green", va("%i",level.teamScores[Q3F_TEAM_GREEN]));

	if( !level.lastteamplayersupdate || level.time - level.lastteamplayersupdate > 1000 ) {
		level.lastteamplayersupdate = level.time;

		memset( teamstrings, 0 ,sizeof(teamstrings) );
		for ( i = 0 ; i < level.numConnectedClients; i++ ) {
			j = level.sortedClients[i];

			if( level.clients[j].sess.sessionTeam >= Q3F_TEAM_RED && level.clients[j].sess.sessionTeam <= Q3F_TEAM_GREEN ) {
				Q_strcat( teamstrings[level.clients[j].sess.sessionTeam - 1], sizeof( teamstrings[0]), va( "%i ", i ) );
			}
		}

		if( Q_stricmp( players_red.string, teamstrings[0] ) ) {
			trap_Cvar_Set( "players_red" , teamstrings[0] );
		}
		if( Q_stricmp( players_blue.string, teamstrings[1] ) ) {
			trap_Cvar_Set( "players_blue" , teamstrings[1] );
		}
		if( Q_stricmp( players_yellow.string, teamstrings[2] ) ) {
			trap_Cvar_Set( "players_yellow" , teamstrings[2] );
		}
		if( Q_stricmp( players_green.string, teamstrings[3] ) ) {
			trap_Cvar_Set( "players_green" , teamstrings[3] );
		}
	}
	// RR2DO2

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunction("G_RunEntities");
#endif
	G_RunEntities();
#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif

	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}

	// see if it is time to do a tournament restart
	CheckMatches();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();
	// for tracking changes
	CheckCvars();

	// Golliwog: Calculate team pings
	G_Q3F_CalculateTeamPings();

	// Golliwog: Run map select
	if( level.mapSelectState != Q3F_MAPSELECT_NONE )
		G_Q3F_MapSelectQuery();

	G_Q3F_Global_Think();

	if (g_listEntity.integer) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}

	G_Q3F_ValidateEntities();

//	G_Q3F_CheckCorruptClientSlots();

	CountPipes();

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif

#ifdef BUILD_LUA
	G_LuaHook_RunFrame( levelTime );
#endif // BUILD_LUA


	// record the time at the end of this frame - it should be about
	// the time the next frame begins - when the server starts
	// accepting commands from connected clients
	level.frameStartTime = trap_Milliseconds();
}
