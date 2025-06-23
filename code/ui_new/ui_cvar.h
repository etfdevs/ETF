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

#ifdef EXTERN_UI_CVAR
#define UI_NULLCVAR( cvarName, defaultString, cvarFlags )
#define UI_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_UI_CVAR
#define UI_NULLCVAR( cvarName, defaultString, cvarFlags )
#define UI_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) vmCvar_t vmCvar;
#endif

#ifdef UI_CVAR_LIST
#define UI_NULLCVAR( cvarName, defaultString, cvarFlags ) { NULL, cvarName, defaultString, cvarFlags, 0 },
#define UI_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) { & vmCvar, cvarName, defaultString, cvarFlags, 0 },
#endif

UI_NULLCVAR( "allowRedirect",				"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_adjustAgentSpeed",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_atmosphericEffects",		"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_bloodFlash",				"1.0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_crosshairHealth", "0",		CVAR_ARCHIVE)
UI_NULLCVAR( "cg_drawFriend",				"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_drawPanel",				"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_drawSkyPortal",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_drawSpeedometer",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_execClassConfigs",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_execMapConfigs",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_filterObituaries",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_flares",					"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_friendlyCrosshair",		"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE )
UI_NULLCVAR( "cg_impactVibration",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_lowEffects",				"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_markTime",					"20000",							CVAR_ARCHIVE )
UI_NULLCVAR( "cg_no3DExplosions",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_oldSkoolMenu",				"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_playClassSound",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_scoreboardsortmode",		"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_showGrenadeTimer1",		"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_showGrenadeTimer2",		"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_showSentryCam",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_simpleItems",				"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_sniperDotColors",			"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_sniperDotScale",			"0.3",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_sniperHistoricalSight",	"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cg_visualAids",				"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "cl_allowDownload",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "com_soundmegs",				"24",								CVAR_ARCHIVE )
UI_NULLCVAR( "com_zonemegs",				"24",								CVAR_ARCHIVE )
UI_NULLCVAR( "discard_cells",				"-1",								CVAR_ARCHIVE )
UI_NULLCVAR( "discard_rockets",				"-1",								CVAR_ARCHIVE )
UI_NULLCVAR( "discard_shells",				"-1",								CVAR_ARCHIVE )
UI_NULLCVAR( "hud_admin_auth",				"0",								CVAR_ROM | CVAR_TEMP )
UI_NULLCVAR( "hud_rcon_auth",				"0",								CVAR_ROM | CVAR_TEMP )
UI_NULLCVAR( "r_displayrefresh",			"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "r_dynamiclight",				"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "r_fastSky",					"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "r_intensity",					"1",								CVAR_ARCHIVE )
UI_NULLCVAR( "r_loresskins",				"0",								CVAR_ARCHIVE | CVAR_LATCH )
UI_NULLCVAR( "scr_conspeed",				"3",								CVAR_ARCHIVE )
UI_NULLCVAR( "ui_favServerName",			"",									CVAR_ARCHIVE )
UI_NULLCVAR( "ui_memsize",					"0",								CVAR_ARCHIVE )
UI_NULLCVAR( "ui_netspeed",					"2",								CVAR_ARCHIVE )
UI_NULLCVAR( "ui_showtooltips",				"1",								CVAR_ARCHIVE )
UI_CVAR( cg_crosshairAlpha,		"cg_crosshairAlpha",		"1.0",								CVAR_ARCHIVE )
UI_CVAR( cg_crosshairAlphaAlt,	"cg_crosshairAlphaAlt",		"1.0",								CVAR_ARCHIVE )
UI_CVAR( cg_crosshairColor,		"cg_crosshairColor",		"White",							CVAR_ARCHIVE )
UI_CVAR( cg_crosshairColorAlt,	"cg_crosshairColorAlt",		"White",							CVAR_ARCHIVE )
UI_CVAR( cg_crosshairSize,		"cg_crosshairSize",			"48",								CVAR_ARCHIVE )
UI_CVAR( cg_scoreSnapshot,		"cg_scoreSnapshot",			"0",								CVAR_ARCHIVE )
UI_CVAR( com_hunkmegs,			"com_hunkmegs",				"128",								CVAR_ARCHIVE | CVAR_LATCH )
UI_CVAR( hud_allowClasses,		"hud_allowClasses",			"0000000000",						CVAR_ROM | CVAR_TEMP )
UI_CVAR( hud_chosenClass,			"hud_chosenClass",			"0",								CVAR_TEMP )
UI_CVAR( hud_currentClasses,		"hud_currentClasses",		"00000000000000000000",				CVAR_ROM | CVAR_TEMP )
UI_CVAR( hud_maxClasses,			"hud_maxClasses",			"00000000000000000000",				CVAR_ROM | CVAR_TEMP )
UI_CVAR( ui_addSpecifyFavorites,	"ui_addSpecifyFavorites",	"0",								CVAR_TEMP )
UI_CVAR( ui_bigFont,				"ui_bigFont",				"0.3",								CVAR_ARCHIVE )
UI_CVAR( ui_brassTime,				"cg_brassTime",				 "1",								CVAR_ARCHIVE)
UI_CVAR( ui_browserGameType,		"ui_browserGameType",		"0",								CVAR_ARCHIVE )
UI_CVAR( ui_browserMaster,			"ui_browserMaster",			"0",								CVAR_ARCHIVE )
UI_CVAR( ui_browserShowEmpty,		"ui_browserShowEmpty",		"1",								CVAR_ARCHIVE )
UI_CVAR( ui_browserShowFull,		"ui_browserShowFull",		"1",								CVAR_ARCHIVE )
UI_CVAR( ui_browserShowPasswordProtected,		"ui_browserShowPasswordProtected",	"1",								CVAR_ARCHIVE )
UI_CVAR( ui_browserShowVersion,	"ui_browserShowVersion",	"1",								CVAR_ARCHIVE )
UI_CVAR( ui_browserSortKey,		"ui_browserSortKey",		"4",								CVAR_ARCHIVE )
UI_CVAR( ui_cdkeychecked,			"ui_cdkeychecked",			"0",								CVAR_ROM )
//UI_CVAR( ui_checkversion,			"ui_checkversion",			"0",								CVAR_ARCHIVE )
UI_CVAR( ui_currentMap,			"ui_currentMap",			"0",								CVAR_TEMP )
UI_CVAR( ui_currentNetMap,		"ui_currentNetMap",			"0",								CVAR_TEMP )
UI_CVAR( ui_dedicated,			"ui_dedicated",				"0",								CVAR_ARCHIVE )
UI_CVAR( ui_drawCrosshair,		"cg_drawCrosshair",			"4",								CVAR_ARCHIVE )
UI_CVAR( ui_drawCrosshairNames,		"cg_drawCrosshairNames",	"1",								CVAR_ARCHIVE )
UI_CVAR( ui_findPlayer,			"ui_findPlayer",			"ETF_PLAYER",						CVAR_ARCHIVE )
UI_CVAR( ui_hudFiles,				"cg_hudFiles",				"ui/hud/default/medium.menu",		CVAR_ARCHIVE )
UI_CVAR( ui_ingameMenuFiles,		"ui_ingameMenuFiles",		"ui/ingame.txt",					CVAR_ARCHIVE )
UI_CVAR( ui_language,				"ui_language",				"english",							CVAR_ARCHIVE )
UI_CVAR( ui_lastServerRefresh_0,	"ui_lastServerRefresh_0",	"",									CVAR_ARCHIVE )
UI_CVAR( ui_lastServerRefresh_1,	"ui_lastServerRefresh_1",	"",									CVAR_ARCHIVE )
UI_CVAR( ui_lastServerRefresh_2,	"ui_lastServerRefresh_2",	"",									CVAR_ARCHIVE )
UI_CVAR( ui_lastServerRefresh_3,	"ui_lastServerRefresh_3",	"",									CVAR_ARCHIVE )
UI_CVAR( ui_mapIndex,				"ui_mapIndex",				"0",								CVAR_TEMP )
UI_CVAR( ui_menuFiles,			"ui_menuFiles",				"ui/menus.txt",						CVAR_ARCHIVE )
UI_CVAR( ui_netSource,			"ui_netSource",				"1",								CVAR_ARCHIVE )
UI_CVAR( ui_realCaptureLimit,		"capturelimit",				"0",								CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART )
UI_CVAR( ui_realWarmUp,			"g_warmup",					"20",								CVAR_ARCHIVE )
UI_CVAR( ui_selectedPlayer,		"cg_selectedPlayer",		"0",								CVAR_ARCHIVE )
UI_CVAR( ui_selectedPlayerName,	"cg_selectedPlayerName",	"",									CVAR_ARCHIVE )
UI_CVAR( ui_server1,				"server1",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server10,				"server10",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server11,				"server11",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server12,				"server12",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server13,				"server13",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server14,				"server14",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server15,				"server15",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server16,				"server16",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server2,				"server2",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server3,				"server3",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server4,				"server4",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server5,				"server5",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server6,				"server6",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server7,				"server7",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server8,				"server8",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_server9,				"server9",					"",									CVAR_ARCHIVE )
UI_CVAR( ui_serverStatusTimeOut,	"ui_serverStatusTimeOut",	"7000",								CVAR_ARCHIVE )
UI_CVAR( ui_smallFont,			"ui_smallFont",				"0.19",								CVAR_ARCHIVE )
UI_CVAR( ui_specifyPort,			"ui_specifyPort",			"27960",							CVAR_ARCHIVE )
UI_CVAR( ui_specifyServer,		"ui_specifyServer",			"",									CVAR_ARCHIVE )

#undef UI_NULLCVAR
#undef UI_CVAR
