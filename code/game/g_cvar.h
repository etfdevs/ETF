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

#ifdef EXTERN_G_CVAR
#define G_NULLCVAR( cvarName, defaultString, cvarFlags, trackChange )
#define G_NULLEXTCVAR( cvarName, defaultString, cvarFlags, extCvarFlags, trackChange ) extern vmCvar_t vmCvar;
#define G_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags, trackChange ) extern vmCvar_t vmCvar;
#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, trackChange ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_G_CVAR
#define G_NULLCVAR( cvarName, defaultString, cvarFlags, trackChange )
#define G_NULLEXTCVAR( cvarName, defaultString, cvarFlags, extCvarFlags, trackChange )
#define G_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags, trackChange ) vmCvar_t vmCvar;
#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, trackChange ) vmCvar_t vmCvar;
#endif

#ifdef G_CVAR_LIST
#define G_NULLCVAR( cvarName, defaultString, cvarFlags, trackChange ) { NULL, cvarName, defaultString, cvarFlags, 0, trackChange, 0 },
#define G_NULLEXTCVAR( cvarName, defaultString, cvarFlags, extCvarFlags, trackChange ) { NULL, cvarName, defaultString, cvarFlags, extCvarFlags, trackChange, 0 },
#define G_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags, trackChange ) { & vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags, trackChange, 0 },
#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, trackChange ) { & vmCvar, cvarName, defaultString, cvarFlags, 0, trackChange, 0 },
#endif

// don't override the cheat state set by the system
G_CVAR( g_cheats, "sv_cheats", "", 0, qfalse )

// noset vars
G_NULLCVAR( "gamename", GAME_VERSION , CVAR_SERVERINFO | CVAR_ROM, qfalse )
G_NULLCVAR( "gamedate", __DATE__ , CVAR_ROM, qfalse )
G_NULLCVAR( "sv_pure", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, qtrue )

// server browser vars abusing vanilla ET info response for our cvars
G_NULLCVAR( "sv_numbots", "0", CVAR_SERVERINFO, qtrue )
G_NULLEXTCVAR( "g_maxlives", "1", CVAR_ROM, EXT_CVAR_NOTABCOMPLETE, qtrue )	// Slothy: pure info for server browser info
G_NULLEXTCVAR( "g_heavyWeaponRestriction", FORTS_SHORTVERSION, CVAR_ROM, EXT_CVAR_NOTABCOMPLETE, qtrue )		// Ensiform: ETF shortversion for server browser info
G_NULLEXTCVAR( "g_balancedteams", "0", CVAR_ROM, EXT_CVAR_NOTABCOMPLETE, qtrue )			// Ensiform: bot count for server browser info
G_NULLEXTCVAR( "g_antilag", "1", CVAR_ROM, EXT_CVAR_NOTABCOMPLETE, qfalse ) // gameindex for server browser

	// noset vars
G_CVAR( g_restarted, "g_restarted", "0", CVAR_ROM, qfalse )
G_CVAR( g_mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM, qfalse )

	// latched vars
G_CVAR( g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH, qfalse )

G_CVAR( g_gameindex, "g_gameindex", "1", CVAR_SERVERINFO | CVAR_LATCH, qfalse )

G_CVAR( g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, qfalse )
G_CVAR( g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, qfalse )

	// change anytime vars
G_CVAR( g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, qtrue )
G_CVAR( g_fraglimit, "fraglimit", "0", CVAR_ARCHIVE | CVAR_NORESTART, qtrue )
G_CVAR( g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, qtrue )
G_CVAR( g_capturelimit, "capturelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, qtrue )

G_CVAR( g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, qfalse )

G_CVAR( g_friendlyFire, "g_friendlyFire", "Full", CVAR_ARCHIVE, qtrue )

G_CVAR( g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, qfalse )

G_CVAR( g_logFile, "g_log", "games.log", CVAR_ARCHIVE, qfalse )
G_CVAR( g_logSync, "g_logSync", "0", CVAR_ARCHIVE, qfalse )

G_CVAR( g_password, "g_password", "", CVAR_USERINFO, qfalse )

G_CVAR( g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, qfalse )
G_CVAR( g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, qfalse )

G_CVAR( g_redteam,		"g_etf_redteam",	"^1Red^7 Team",		0, qfalse )
G_CVAR( g_blueteam,		"g_etf_blueteam",	"^4Blue^7 Team",	0, qfalse )
G_CVAR( g_yellowteam,	"g_etf_yellowteam", "^3Yellow^7 Team",	0, qfalse )
G_CVAR( g_greenteam,	"g_etf_greenteam",	"^2Green^7 Team",	0, qfalse )

G_CVAR( g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, qfalse )

G_CVAR( g_dedicated, "dedicated", "0", 0, qfalse )

G_CVAR( g_speed, "g_speed", "320", 0, qtrue )
G_CVAR( g_gravity, "g_gravity", "800", CVAR_SYSTEMINFO, qtrue )
G_CVAR( g_knockback, "g_knockback", "900", CVAR_SERVERINFO, qtrue )
G_CVAR( g_quadfactor, "g_quadfactor", "4", 0, qtrue )		// Golliwog: Default to 4
G_CVAR( g_forcerespawn, "g_forcerespawn", "20", 0, qtrue )
G_CVAR( g_inactivity, "g_inactivity", "0", 0, qtrue )
G_CVAR( g_debugMove, "g_debugMove", "0", 0, qfalse )
G_CVAR( g_debugDamage, "g_debugDamage", "0", 0, qfalse )
G_CVAR( g_debugAlloc, "g_debugAlloc", "0", 0, qfalse )
G_CVAR( g_motd, "g_motd", "", 0, qfalse )
G_CVAR( g_blood, "com_blood", "1", 0, qfalse )

G_CVAR( g_teamFrags, "g_teamFrags", "0", CVAR_ARCHIVE, qfalse )

G_CVAR( g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, qfalse )
G_CVAR( g_listEntity, "g_listEntity", "0", 0, qfalse )

G_CVAR( pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, qfalse )
G_CVAR( pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, qfalse )
G_CVAR( pmove_float, "pmove_float", "1", CVAR_SYSTEMINFO, qfalse )

G_CVAR( g_suicideDelay, "g_suicideDelay", "7", 0, qfalse )		// Golliwog: Delay after suiciding
G_CVAR( g_teamChatSounds, "g_teamChatSounds", "4", 0, qfalse )	// Golliwog: Allow sounds/sounddict in team chat

G_CVAR(g_etfversion, "g_etfversion", FORTS_VERSION "-" GIT_COMMIT_HASH_SHORT, CVAR_ROM | CVAR_SERVERINFO, qfalse)	// Golliwog: Display to server browsers keeg: rename for etf

G_CVAR( g_adminPassword, "g_adminPassword", "", 0, qfalse )	// Golliwog: Allow admins (i.e. less access than rcon)

G_CVAR( g_minRate, "sv_minRate", "2500", CVAR_SERVERINFO, qtrue )			// Golliwog: Prevent players running at a lower rate to avoid sentries, etc.
G_CVAR( g_minSnaps, "sv_minSnaps", "20", CVAR_SERVERINFO, qtrue )			// RR2DO2: Prevent players for faking high ping
G_CVAR( g_teamKillRules, "g_teamKillRules", "2", 0, qtrue )

	// Golliwog: Handy Q3F debugging variables
//G_CVAR( g_grenadeScale, "g_grenadeScale", "1.3", 0, qfalse )
//G_CVAR( g_grenadeConcBlast, "g_grenadeConcBlast", "230", 0, qfalse )
//G_CVAR( g_grenadeThrow, "g_grenadeThrow", "600", 0, qfalse )
//G_CVAR( g_grenadeZorg, "g_grenadeZorg", "18", 0, qfalse )
//G_CVAR( g_grenadeZvel, "g_grenadeZvel", "200", 0, qfalse )
//G_CVAR( g_grenadeDropDelay, "g_grenadeDropDelay", "20", 0, qfalse )
//G_CVAR( g_grenadeDropZ, "g_grenadeDropZ", "0", 0, qfalse )
//G_CVAR( g_grenadeDropZVel, "g_grenadeDropZVel", "10", 0, qfalse )
//G_CVAR( g_grenadeBounce, "g_grenadeBounce", "0.45", 0, qfalse )
//G_CVAR( g_grenadeStick, "g_grenadeStick", "80", 0, qfalse )
//G_CVAR( g_grenadePJdelay, "g_grenadePJdelay", "0", 0, qfalse )
//G_CVAR( g_pipedetdelay, "g_pipedetdelay", "250", 0, qfalse )
//G_CVAR( g_pipeLauncherVel, "g_pipeLauncherVel", "700", 0, qfalse )
//G_CVAR( g_radiusDamageScale, "g_radiusDamageScale", "10", 0, qfalse )
//G_CVAR( g_nailGrenUpdateInterval, "g_nailGrenUpdateInterval", "500", 0, qfalse )
//G_CVAR( g_grenadeConcHBlast, "g_grenadeConcHBlast", "90", 0, qfalse )
//G_CVAR( g_grenadeConcVBlast, "g_grenadeConcVBlast", "100", 0, qfalse )
//G_CVAR( g_grenadeHBlast, "g_grenadeHBlast", "100", 0, qfalse )
//G_CVAR( g_grenadeVBlast, "g_grenadeVBlast", "100", 0, qfalse )
//G_CVAR( g_supplyStationMaxDamage, "g_supplyStationMaxDamage", "300", 0, qfalse},
//G_CVAR( g_napalmRocketVel, "g_napalmRocketVel", "750", 0, qfalse )
//G_CVAR( g_napalmKnockBack, "g_napalmKnockBack", "0.55", 0, qfalse )
//G_CVAR( g_MissileImpactKnockback, "g_MissileImpactKnockback", "1.44", 0, qtrue )
//G_CVAR( g_sentryRotSpeed,		"g_sentryRotSpeed",		"350", 0, qtrue )
//G_CVAR( g_sentryBulletDamage,	"g_sentryBulletDamage",	"7", 0, qtrue )
//G_CVAR( g_sentryLockDelay,		"g_sentryLockDelay",	"250", 0, qtrue )

G_CVAR( g_mapentDebug,				"g_mapentDebug",				"0", 0, qfalse )

	// Golliwog: Allow server admin to set class limits
G_CVAR( g_classReconLimit,			"g_classReconLimit",			"-1", 0, qfalse )
G_CVAR( g_classSniperLimit,			"g_classSniperLimit",			"-1", 0, qfalse )
G_CVAR( g_classSoldierLimit,		"g_classSoldierLimit",			"-1", 0, qfalse )
G_CVAR( g_classGrenadierLimit,		"g_classGrenadierLimit",		"-1", 0, qfalse )
G_CVAR( g_classParamedicLimit,		"g_classParamedicLimit",		"-1", 0, qfalse )
G_CVAR( g_classMinigunnerLimit,		"g_classMinigunnerLimit",		"-1", 0, qfalse )
G_CVAR( g_classFlametrooperLimit,	"g_classFlametrooperLimit",		"-1", 0, qfalse )
G_CVAR( g_classAgentLimit,			"g_classAgentLimit",			"-1", 0, qfalse )
G_CVAR( g_classEngineerLimit,		"g_classEngineerLimit",			"-1", 0, qfalse )
	// Golliwog.

	// Golliwog: Match-related cvars
G_CVAR( g_matchPassword,			"g_matchPassword",				"",			CVAR_ROM, qfalse )
G_CVAR( g_matchState,				"g_matchState",					"",			CVAR_ROM, qfalse )
G_CVAR( g_matchMode,				"g_matchMode",					"0",		0, qfalse )
G_CVAR( g_matchPlayers,				"g_matchPlayers",				"2",		CVAR_ARCHIVE, qtrue )
G_CVAR( g_warmup,					"g_warmup",						"120",		CVAR_ARCHIVE, qtrue )

G_CVAR( g_mapVote,					"g_mapVote", "1", CVAR_ARCHIVE, qfalse )
G_CVAR( g_execMapConfigs,			"g_execMapConfigs", "0", CVAR_ARCHIVE, qfalse )
G_CVAR( g_banRules, "g_banRules", "3", CVAR_ARCHIVE | CVAR_SERVERINFO, qfalse )

G_CVAR( score_red, "score_red", "0", CVAR_SERVERINFO, qfalse )
G_CVAR( score_blue, "score_blue", "0", CVAR_SERVERINFO, qfalse )
G_CVAR( score_yellow, "score_yellow", "0", CVAR_SERVERINFO, qfalse )
G_CVAR( score_green, "score_green", "0", CVAR_SERVERINFO, qfalse )
	// Golliwog.

	// RR2DO2: Serverbrowser related cvars
G_CVAR( players_red, "players_red", "", CVAR_SERVERINFO, qfalse )
G_CVAR( players_blue, "players_blue", "", CVAR_SERVERINFO, qfalse )
G_CVAR( players_yellow, "players_yellow", "", CVAR_SERVERINFO, qfalse )
G_CVAR( players_green, "players_green", "", CVAR_SERVERINFO, qfalse )
	// RR2DO2

	// RR2DO2: Spectator mode related
G_CVAR( g_spectatorMode, "g_spectatorMode", "0", CVAR_ARCHIVE, qfalse )
	// RR2DO2

	// RR2DO2: 
G_CVAR( g_showTeamBalanceWarning, "g_showTeamBalanceWarning", "1", 0, qfalse )
G_CVAR( g_agentHitBeep , "g_agentHitBeep", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, qfalse )

G_CVAR( g_serverConfigTime, "g_serverConfigTime", "", CVAR_ROM, qfalse )
G_CVAR( g_serverConfigMap, "g_serverConfigMap", "", CVAR_ROM, qfalse )

G_CVAR( sv_floodProtect, "sv_floodProtect", "0", CVAR_ROM, qfalse )

	// slothy
G_CVAR( g_allowAllVersions, "g_allowAllVersions", "0", CVAR_ARCHIVE, qfalse )
	// end slothy

G_CVAR( g_adminFloodImmunity, "g_adminFloodImmunity", "0", CVAR_ARCHIVE, qfalse )

G_CVAR( g_debugBullets, "g_debugBullets", "0", CVAR_CHEAT, qfalse )

	//Unlagged related 
G_CVAR( g_smoothClients, "g_smoothClients", "1", CVAR_ARCHIVE, qfalse )
G_CVAR( g_unlagged, "g_unlagged", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, qtrue )
G_CVAR( g_antilag_ms, "g_antilag_ms", "150", CVAR_ARCHIVE | CVAR_SERVERINFO, qtrue )
G_CVAR( g_experiment, "g_experiment", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, qtrue )
//G_CVAR( g_unlaggedVersion, "g_unlaggedVersion", "2.0", CVAR_ROM | CVAR_SERVERINFO, qfalse )

G_CVAR( g_truePing, "g_truePing", "1", CVAR_ARCHIVE, qtrue )
	// it's CVAR_SYSTEMINFO so the client's sv_fps will be automagically set to its value
G_CVAR( sv_fps, "sv_fps", "20", CVAR_SYSTEMINFO | CVAR_ARCHIVE, qfalse )

#ifdef BUILD_BOTS
	// Omni-bot user defined path to load bot library from.
G_CVAR( g_OmniBotPath, "omnibot_path", "", CVAR_ARCHIVE | CVAR_NORESTART, qfalse )
G_CVAR( g_OmniBotEnable, "omnibot_enable", "1", CVAR_ARCHIVE | CVAR_SERVERINFO_NOUPDATE | CVAR_NORESTART, qfalse )
G_CVAR( g_OmniBotPlaying, "omnibot_playing", "0", CVAR_SERVERINFO_NOUPDATE | CVAR_ROM, qfalse )
G_CVAR( g_OmniBotFlags, "omnibot_flags", "0", CVAR_ARCHIVE | CVAR_NORESTART, qfalse )
#endif

G_CVAR( g_shoutcastPassword, "g_shoutcastPassword", "", 0, qfalse )

#ifdef BUILD_LUA
G_CVAR( lua_modules, "lua_modules", "", 0, qfalse )
G_CVAR( lua_allowedModules, "lua_allowedModules", "", 0, qfalse )
#endif

G_CVAR( g_eventLog, "g_eventLog", "0", CVAR_LATCH | CVAR_ARCHIVE, qfalse)

G_CVAR( g_spawnFullStats, "g_spawnFullStats", "1", CVAR_ARCHIVE, qfalse )
G_CVAR( g_balancedDeathAmmo, "g_balancedDeathAmmo", "1", CVAR_ARCHIVE, qfalse )
G_CVAR( g_newPulseGren, "g_newPulseGren", "0", CVAR_ARCHIVE, qfalse )
G_CVAR( g_newStunGren, "g_newStunGren", "1", CVAR_ARCHIVE, qfalse )

	// Sets init/max nailbombs per player
G_CVAR( g_maxNailBombs, "g_maxNailBombs", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, qfalse )

G_CVAR( g_maxGasGrens, "g_maxGasGrens", "2", CVAR_SERVERINFO | CVAR_ARCHIVE, qfalse )

G_CVAR( g_suicideScorePenalty, "g_suicideScorePenalty", "1", CVAR_ARCHIVE, qfalse )

#undef G_NULLCVAR
#undef G_NULLEXTCVAR
#undef G_EXTCVAR
#undef G_CVAR
