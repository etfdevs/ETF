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

// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"
#include "cg_q3f_scriptlib.h"
#include "cg_q3f_menu.h"

#include "../ui_new/ui_shared.h"
displayContextDef_t cgDC;

const char *teamnames[4] = { "red", "blue", "yellow", "green" };

int drawSkyPortalModificationCount = -1;
int delayedSounds = 0;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );
static qboolean CG_CheckExecKey( int key );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/

Q_EXPORT intptr_t vmMain( int command, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4,
intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9, intptr_t arg10, intptr_t arg11 ) {
	switch ( command ) {
	case CG_INIT:
#ifdef PEFLOG
		BG_Q3F_PerformanceMonitorInit("performance_cgame.log");
#endif
/* Ensiform - Added demoPlayback (arg3) */
		CG_Q3F_Init( arg0, arg1, arg2, (qboolean)arg3 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
#ifdef PEFLOG
		BG_Q3F_PerformanceMonitorShutdown();
#endif
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, (stereoFrame_t)arg1, (qboolean)arg2 );
#ifdef PEFLOG
		BG_Q3F_FlushTraceBuffer();
#endif
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, (qboolean)arg1);
		return 0;
	case CG_MOUSE_EVENT:
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0, qtrue);
		return 0;
	case CG_CHECKEXECKEY:
		return CG_CheckExecKey( arg0 );
	case CG_WANTSBINDKEYS:
		return qfalse;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[WP_NUM_WEAPONS];		// Was MAX_WEAPONS
weaponInfo_t		cg_extendedweapons[MAX_EXTENDED_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];

//vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSpeedometer;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;

//keeg new xhair from et
vmCvar_t	cg_crosshairAlpha;
vmCvar_t	cg_crosshairAlphaAlt;
vmCvar_t	cg_crosshairColor;
vmCvar_t	cg_crosshairColorAlt;
vmCvar_t	cg_bloodFlash;  //for flame on hud

vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_markTime;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
//vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_fovAspectAdjust;
vmCvar_t	cg_fovViewmodel;
vmCvar_t	cg_fovViewmodelAdjust;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_hideScope;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_drawFriendSize;
vmCvar_t	cg_debugTime;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_scorePlum;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_sniperDotScale;
vmCvar_t	cg_impactVibration;
vmCvar_t	cg_sniperDotColors;
vmCvar_t	cg_execClassConfigs;
vmCvar_t	cg_execMapConfigs;
vmCvar_t	cg_lowEffects;
vmCvar_t	cg_no3DExplosions;
vmCvar_t	cg_atmosphericEffects;

vmCvar_t	cg_etfVersion;

//vmCvar_t	cg_menuSlideTime;
//vmCvar_t	cg_menuNoFade;
vmCvar_t	cg_teamChatSounds;
vmCvar_t	cg_sniperAutoZoom;
vmCvar_t	cg_sniperHistoricalSight;
vmCvar_t	cg_grenadePrimeSound;
//vmCvar_t	cg_teamChatBigFont;
//vmCvar_t	cg_newFlamer;
//vmCvar_t	cg_drawMem;
vmCvar_t	cg_autoReload;
vmCvar_t	g_spectatorMode; // RR2DO2
vmCvar_t	r_lodCurveError;
vmCvar_t	r_gamma;
vmCvar_t	r_showNormals;
vmCvar_t	r_showTris;
vmCvar_t	r_fastSky;
vmCvar_t	r_vertexLight;
vmCvar_t	r_loresskins;
vmCvar_t	r_fullBright;
vmCvar_t	cg_shadows;
vmCvar_t	rate;
vmCvar_t	snaps;
vmCvar_t	cl_maxpackets;
vmCvar_t	r_maxpolys;
vmCvar_t	r_maxpolyverts;
vmCvar_t	com_maxfps;
vmCvar_t	r_clear;
vmCvar_t	r_debugSort;
vmCvar_t	r_debugSurface;
vmCvar_t	r_directedScale;
vmCvar_t	r_drawworld;
vmCvar_t	r_lockpvs;
vmCvar_t	r_nocull;
vmCvar_t	r_nocurves;
vmCvar_t	r_noportals;
vmCvar_t	r_novis;
vmCvar_t	r_showclusters;
vmCvar_t	r_singleShader;
vmCvar_t	r_lightmap;
vmCvar_t	cg_fallingBob;
vmCvar_t	cg_weaponBob;
/*vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t 	cg_yellowTeamName;
vmCvar_t 	cg_greenTeamName;*/
vmCvar_t	cg_adjustAgentSpeed;
//vmCvar_t	cg_drawHudSlots;
vmCvar_t	cg_playClassSound;
vmCvar_t	cg_drawParticleCount;
//vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_initializing;
vmCvar_t	cg_debugPanel;
vmCvar_t	cg_drawPanel;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
//vmCvar_t	cg_hudFiles;
vmCvar_t	cg_userHud;
vmCvar_t	r_flares;
vmCvar_t	cg_flares;
vmCvar_t	cg_friendlyCrosshair;
vmCvar_t	cg_showGrenadeTimer1;
vmCvar_t	cg_showGrenadeTimer2;
vmCvar_t	cg_showGrenadeTimer3;
vmCvar_t	cg_scoreboardsortmode;
vmCvar_t	cg_visualAids;
vmCvar_t	con_notifytime_etf;
vmCvar_t	cg_filterObituaries;
vmCvar_t	cg_showSentryCam;
vmCvar_t	cg_oldSkoolMenu;
vmCvar_t	cg_gender;
vmCvar_t	com_hunkmegs;
vmCvar_t	cg_classicinit;
vmCvar_t	cg_drawSkyPortal;
vmCvar_t	developer;
vmCvar_t	cg_mergemm2;
vmCvar_t	cg_onlychats;
//unlagged - client options
vmCvar_t	cg_unlagged;
vmCvar_t	cg_debugDelag;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_predictWeapons;
vmCvar_t	cg_drawBBox;
vmCvar_t	cg_cmdTimeNudge;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cl_timeNudge;
vmCvar_t	cg_latentSnaps;
vmCvar_t	cg_latentCmds;
vmCvar_t	cg_plOut;

//slothy
vmCvar_t	r_subdivisions;
vmCvar_t	cg_altObits;

vmCvar_t	ui_altObitsY;
vmCvar_t	ui_altObitsX;
//slothy

vmCvar_t	cg_recording_statusline;			// slothy ET 2.60 cvar
vmCvar_t	cl_demorecording;
vmCvar_t	cl_demofilename;
vmCvar_t	cl_demooffset;

vmCvar_t	cl_waverecording;
vmCvar_t	cl_wavefilename;
vmCvar_t	cl_waveoffset;

vmCvar_t	demo_avifpsF1;
vmCvar_t	demo_avifpsF2;
vmCvar_t	demo_avifpsF3;
vmCvar_t	demo_avifpsF4;
vmCvar_t	demo_avifpsF5;
vmCvar_t	demo_avifpsF6;
vmCvar_t	demo_drawTimeScale;
vmCvar_t	demo_infoWindow;
//vmCvar_t	demo_baseFOV;
vmCvar_t	demo_scoresToggle;

vmCvar_t	cg_drawFollowText;

#ifdef BUILD_BOTS
vmCvar_t	cg_omnibotdrawing;
#endif

vmCvar_t	cg_rocketTrail;
vmCvar_t	cg_grenadeTrail;
vmCvar_t	cg_pipeTrail;
vmCvar_t	cg_napalmTrail;
vmCvar_t	cg_dartTrail;
vmCvar_t	cg_shotgunPuff;

vmCvar_t	cl_anonymous;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  //keeg for ET api
} cvarTable_t;

static cvarTable_t		cvarTable[] = {
	{ &cg_grenadePrimeSound,		"cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE },
	{ &cg_scoreboardsortmode,		"cg_scoreboardsortmode",	"0",								CVAR_ARCHIVE },
	{ &cg_sniperDotScale,			"cg_sniperDotScale",		"0.3",								CVAR_ARCHIVE },
	{ &cg_adjustAgentSpeed,			"cg_adjustAgentSpeed",		"1",								CVAR_ARCHIVE | CVAR_USERINFO},
	{ &cg_atmosphericEffects,		"cg_atmosphericEffects",	"1",								CVAR_ARCHIVE },
//	{ &cg_drawHudSlots,				"cg_drawHudSlots",			"1",								CVAR_ARCHIVE },
	{ &cg_drawPanel,				"cg_drawPanel",				"1",								CVAR_ARCHIVE },
	{ &cg_execClassConfigs,			"cg_execClassConfigs",		"0",								CVAR_ARCHIVE },	
	{ &cg_execMapConfigs,			"cg_execMapConfigs",		"1",								CVAR_ARCHIVE },
	{ &cg_filterObituaries,			"cg_filterObituaries",		"0",								CVAR_ARCHIVE },
	{ &cg_friendlyCrosshair,		"cg_friendlyCrosshair",		"1",								CVAR_ARCHIVE },
	{ &cg_impactVibration,			"cg_impactVibration",		"1",								CVAR_ARCHIVE },
	{ &cg_lowEffects,				"cg_lowEffects",			"0",								CVAR_ARCHIVE },
	{ &cg_no3DExplosions,			"cg_no3DExplosions",		"0",								CVAR_ARCHIVE },
	{ &cg_oldSkoolMenu,				"cg_oldSkoolMenu",			"0",								CVAR_ARCHIVE },
	{ &cg_playClassSound,			"cg_playClassSound",		"1",								CVAR_ARCHIVE },
	{ &cg_showGrenadeTimer1,		"cg_showGrenadeTimer1",		"0",								CVAR_ARCHIVE },
	{ &cg_showGrenadeTimer2,		"cg_showGrenadeTimer2",		"1",								CVAR_ARCHIVE },
	{ &cg_showGrenadeTimer3,		"cg_showGrenadeTimer3",		"0",								CVAR_ARCHIVE },
	{ &cg_visualAids,				"cg_visualAids",			"0",								CVAR_ARCHIVE },
	{ &cg_showSentryCam,			"cg_showSentryCam",			"1",								CVAR_ARCHIVE },
	{ &cg_sniperDotColors,			"cg_sniperDotColors",		"1",								CVAR_ARCHIVE },
	{ &cg_sniperHistoricalSight,	"cg_sniperHistoricalSight", "0",								CVAR_ARCHIVE },
	{ &cg_flares,					"cg_flares",				"1",								CVAR_ARCHIVE },
	{ &r_fastSky,					"r_fastSky",				"0",								CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames,		"cg_drawCrosshairNames",	"1",								CVAR_ARCHIVE },
	{ NULL,							"r_dynamiclight",			"1",								CVAR_ARCHIVE },
	{ &cg_brassTime,				"cg_brassTime",				"2500",								CVAR_ARCHIVE },
	{ &cg_markTime,					"cg_markTime",				"20000",							CVAR_ARCHIVE },
	{ &cg_simpleItems,				"cg_simpleItems",			"0",								CVAR_ARCHIVE },
	{ NULL,							"cl_allowDownload",			"0",								CVAR_ARCHIVE },
	{ &cg_grenadePrimeSound,		"cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE },
	{ &cg_drawFriend,				"cg_drawFriend",			"1",								CVAR_ARCHIVE },
	{ &cg_drawFriendSize,			"cg_drawFriendSize",		"6",								CVAR_ARCHIVE },
	{ &cg_mergemm2,					"cg_mergemm2",				"0",								CVAR_ARCHIVE },
	{ &cg_onlychats,				"cg_onlychats",				"0",								CVAR_ARCHIVE },
	{ &cg_debugTime,				"cg_debugTime",				"0",								CVAR_ARCHIVE },
	{ &cg_drawGun,					"cg_drawGun",				"1",								CVAR_ARCHIVE },
	{ &cg_zoomFov,					"cg_zoomfov",				"22.5",								CVAR_ARCHIVE },
	{ &cg_fov,						"cg_fov",					"90",								CVAR_ARCHIVE },
	{ &cg_fovAspectAdjust,			"cg_fovAspectAdjust",		"0",								CVAR_ARCHIVE },
	{ &cg_fovViewmodel,				"cg_fovViewmodel",			"0",								CVAR_ARCHIVE },
	{ &cg_fovViewmodelAdjust,		"cg_fovViewmodelAdjust",	"1",								CVAR_ARCHIVE },
	{ &cg_viewsize,					"cg_viewsize",				"100",								CVAR_ARCHIVE },
	{ &cg_stereoSeparation,			"cg_stereoSeparation",		"0.4",								CVAR_ARCHIVE },
	{ &cg_shadows,					"cg_shadows",				"1",								CVAR_ARCHIVE },
	{ &cg_gibs,						"cg_gibs",					"1",								CVAR_ARCHIVE },
	{ &cg_draw2D,					"cg_draw2D",				"1",								CVAR_ARCHIVE },
	{ &cg_drawStatus,				"cg_drawStatus",			"2",								CVAR_ARCHIVE },
	{ &cg_drawTimer,				"cg_drawTimer",				"1",								CVAR_ARCHIVE },
	{ &cg_drawSpeedometer,			"cg_drawSpeedometer",		"0",								CVAR_ARCHIVE },
	{ &cg_drawFPS,					"cg_drawFPS",				"0",								CVAR_ARCHIVE },
	{ &cg_drawSnapshot,				"cg_drawSnapshot",			"0",								CVAR_ARCHIVE },
	{ &cg_draw3dIcons,				"cg_draw3dIcons",			"1",								CVAR_ARCHIVE },
	{ &cg_drawIcons,				"cg_drawIcons",				"1",								CVAR_ARCHIVE },
	{ &cg_drawAmmoWarning,			"cg_drawAmmoWarning",		"1",								CVAR_ARCHIVE },
	{ &cg_drawAttacker,				"cg_drawAttacker",			"1",								CVAR_ARCHIVE },
	{ &cg_drawCrosshair,			"cg_drawCrosshair",			"4",								CVAR_ARCHIVE },
	{ &cg_drawRewards,				"cg_drawRewards",			"1",								CVAR_ARCHIVE },
	{ &cg_crosshairSize,			"cg_crosshairSize",			"24",								CVAR_ARCHIVE },
// slothy - xhair health defaults to off until it's properly implemented
	{ &cg_crosshairHealth,			"cg_crosshairHealth",		"0",								CVAR_ARCHIVE },
	{ &cg_crosshairAlpha,			"cg_crosshairAlpha",		"1.0",								CVAR_ARCHIVE },
	{ &cg_crosshairAlphaAlt,		"cg_crosshairAlphaAlt",		"1.0",								CVAR_ARCHIVE },
	{ &cg_crosshairColor,			"cg_crosshairColor",		"White",							CVAR_ARCHIVE },
	{ &cg_crosshairColorAlt,		"cg_crosshairColorAlt",		"White",							CVAR_ARCHIVE },
	{ &cg_crosshairX,				"cg_crosshairX",			"0",								CVAR_ARCHIVE },
	{ &cg_crosshairY,				"cg_crosshairY",			"0",								CVAR_ARCHIVE },
	{ &cg_bloodFlash,				"cg_bloodFlash",			"1.0",								CVAR_ARCHIVE },	
	{ &cg_lagometer,				"cg_lagometer",				"1",								CVAR_ARCHIVE },
	{ &cg_runpitch,					"cg_runpitch",				"0.002",							CVAR_ARCHIVE },
	{ &cg_runroll,					"cg_runroll",				"0.005",							CVAR_ARCHIVE },
	{ &cg_bobup ,					"cg_bobup",					"0.005",							CVAR_ARCHIVE },
	{ &cg_bobpitch,					"cg_bobpitch",				"0.002",							CVAR_ARCHIVE },
	{ &cg_bobroll,					"cg_bobroll",				"0.002",							CVAR_ARCHIVE },
	{ &cg_footsteps,				"cg_footsteps",				"1",								CVAR_ARCHIVE },
	{ &cg_teamChatTime,				"cg_teamChatTime",			"3000",								CVAR_ARCHIVE },
	{ &cg_teamChatHeight,			"cg_teamChatHeight",		"0",								CVAR_ARCHIVE },
	{ &cg_forceModel,				"cg_forceModel",			"0",								CVAR_ARCHIVE },
	{ &cg_teamChatsOnly,			"cg_teamChatsOnly",			"0",								CVAR_ARCHIVE },
	{ &cg_teamChatSounds,			"cg_teamChatSounds",		"1",								CVAR_ARCHIVE },
	{ &cg_fallingBob,				"cg_fallingBob",			"1",								CVAR_ARCHIVE },
	{ &cg_weaponBob,				"cg_weaponBob",				"1",								CVAR_ARCHIVE },
	{ &cg_smallFont,				"ui_smallFont",				"0.25",								CVAR_ARCHIVE },
	{ &cg_bigFont,					"ui_bigFont",				"0.4",								CVAR_ARCHIVE },
	{ &cg_smallFont,				"ui_smallFont",				"0.25",								CVAR_ARCHIVE },
	{ &cg_hudFiles,					"cg_hudFiles",				"ui/hud/default/medium.menu",		CVAR_ARCHIVE },
	{ &cg_userHud,					"cg_userHud",				"0",								CVAR_ARCHIVE },
	{ &r_flares,					"r_flares",					"1",								CVAR_ARCHIVE },
	{ &con_notifytime_etf,			"con_notifytime_etf",		"5",								CVAR_ARCHIVE },
	{ &r_lodCurveError,				"r_lodCurveError",			"250",								CVAR_ARCHIVE },
	{ &r_gamma,						"r_gamma",					"1",								CVAR_ARCHIVE },
	{ &cg_shadows,					"cg_shadows",				"1",								CVAR_ARCHIVE },
	{ &cl_maxpackets,				"cl_maxpackets",			"60",								CVAR_ARCHIVE },
	{ &r_maxpolys,					"r_maxpolys",				"1800",								CVAR_ARCHIVE },
	{ &r_maxpolyverts,				"r_maxpolyverts",			"9000",								CVAR_ARCHIVE },
	{ &com_maxfps,					"com_maxfps",				"85",								CVAR_ARCHIVE },
	{ &cg_blood,					"com_blood",				"1",								CVAR_ARCHIVE },
	{ &cg_cameraOrbitDelay,			"cg_cameraOrbitDelay",		"50",								CVAR_ARCHIVE },
	{ &cg_scorePlum,				"cg_scorePlums",			"1",								CVAR_ARCHIVE},

	{ &cg_errorDecay,				"cg_errordecay",			"100",		0 },
	{ &cg_nopredict,				"cg_nopredict",				"0",		0 },
	{ &cg_showmiss,					"cg_showmiss",				"0",		0 },
	{ &cg_stats,					"cg_stats",					"0",		0 },
	{ &cg_sniperAutoZoom,			"cg_sniperAutoZoom",		"0",		0 },
	{ &cg_buildScript,				"com_buildScript",			"0",		0 },	// force loading of all possible data and error on failures
	{ &cg_timescaleFadeEnd,			"cg_timescaleFadeEnd",		"1",		0 },
	{ &cg_timescaleFadeSpeed,		"cg_timescaleFadeSpeed",	"0",		0 },
	{ &cg_timescale,				"timescale",				"1",		0 },
	{ &r_clear,						"r_clear",					"0",		0 },


	{ &r_debugSort,					"r_debugSort",				"0",		CVAR_CHEAT },
	{ &r_debugSurface,				"r_debugSurface",			"0",		CVAR_CHEAT },
	{ &r_directedScale,				"r_directedScale",			"0",		CVAR_CHEAT },
	{ &r_drawworld,					"r_drawworld",				"1",		CVAR_CHEAT },
	{ &r_lockpvs,					"r_lockpvs",				"0",		CVAR_CHEAT },
	{ &r_nocull,					"r_nocull",					"0",		CVAR_CHEAT },
	{ &r_nocurves,					"r_nocurves",				"0",		CVAR_CHEAT },
	{ &r_noportals,					"r_noportals",				"0",		CVAR_CHEAT },
	{ &r_novis,						"r_novis",					"0",		CVAR_CHEAT },
	{ &r_showclusters,				"r_showclusters",			"0",		CVAR_CHEAT },
	{ &r_lightmap,					"r_lightmap",				"0",		CVAR_CHEAT },
	{ &r_showNormals,				"r_showNormals",			"0",		CVAR_CHEAT },
	{ &r_showTris,					"r_showTris",				"0",		CVAR_CHEAT },
	{ &cg_debugPanel,				"cg_debugpanel",			"0",		CVAR_CHEAT },
	{ &cg_drawParticleCount,		"cg_drawParticleCount",		"0",		CVAR_CHEAT },
	{ &cg_cameraMode,				"com_cameraMode",			"0",		CVAR_CHEAT },
	{ &cg_cameraOrbit,				"cg_cameraOrbit",			"0",		CVAR_CHEAT },
	{ &cg_gun_x,					"cg_gunX",					"0",		CVAR_CHEAT },
	{ &cg_gun_y,					"cg_gunY",					"0",		CVAR_CHEAT },
	{ &cg_gun_z,					"cg_gunZ",					"0",		CVAR_CHEAT },
	{ &cg_centertime,				"cg_centertime",			"3",		CVAR_CHEAT },
	{ &cg_swingSpeed,				"cg_swingSpeed",			"0.3",		CVAR_CHEAT },
	{ &cg_animSpeed,				"cg_animspeed",				"1",		CVAR_CHEAT },
	{ &cg_debugAnim,				"cg_debuganim",				"0",		CVAR_CHEAT },
	{ &cg_debugPosition,			"cg_debugposition",			"0",		CVAR_CHEAT },
	{ &cg_debugEvents,				"cg_debugevents",			"0",		CVAR_CHEAT },
	{ &cg_noPlayerAnims,			"cg_noplayeranims",			"0",		CVAR_CHEAT },
	{ &cg_tracerChance,				"cg_tracerchance",			"0.4",		CVAR_CHEAT },
	{ &cg_thirdPersonRange,			"cg_thirdPersonRange",		"40",		CVAR_CHEAT },
	{ &cg_thirdPersonAngle,			"cg_thirdPersonAngle",		"0",		CVAR_CHEAT },
	{ &cg_thirdPerson,				"cg_thirdPerson",			"0",		CVAR_CHEAT },
	{ &cg_hideScope,				"cg_hidescope",				"0",		CVAR_CHEAT },			// Slothy

	{ &r_singleShader,				"r_singleShader",			"0",		CVAR_CHEAT | CVAR_LATCH},
	{ &r_fullBright,				"r_fullBright",				"0",		CVAR_CHEAT | CVAR_LATCH },

	{ &cg_autoReload,				"cg_autoReload",			"1",		CVAR_USERINFO },
	{ &rate,						"rate",						"25000",	CVAR_USERINFO | CVAR_ARCHIVE },
	{ &snaps,						"snaps",					"40",		CVAR_USERINFO | CVAR_ARCHIVE },
	{ &cg_gender,					"cg_gender",				"0",		CVAR_USERINFO | CVAR_ARCHIVE },
	{ &cg_initializing,				"init",						"1",		CVAR_USERINFO | CVAR_ROM },
    { &cg_etfVersion,				"cg_etfversion",			"0",		CVAR_USERINFO | CVAR_ROM },

	{ &g_spectatorMode,				"g_spectatorMode",			"0",		CVAR_SYSTEMINFO },

	{ &cg_paused,					"cl_paused",				"0",		CVAR_ROM },

	// Changing the default here to 0 to keep it in sync with ui code
	{ &r_loresskins,				"r_loresskins",				"0",		CVAR_ARCHIVE | CVAR_LATCH },
	{ &cg_classicinit,				"cg_classicInit",			"1",		CVAR_ARCHIVE | CVAR_LATCH },
	{ &r_vertexLight,				"r_vertexLight",			"0",		CVAR_ARCHIVE | CVAR_LATCH },
	{ &com_hunkmegs,				"com_hunkmegs",				"128",		CVAR_ARCHIVE | CVAR_LATCH },

	{ &cg_drawSkyPortal,			"cg_drawSkyPortal",			"1",		CVAR_ARCHIVE },
	//Unlagged
	{ &cg_unlagged,					"cg_unlagged",				"1",		CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_debugDelag,				"cg_debugDelag",			"0",		CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_drawBBox,					"cg_drawBBox",				"0",		CVAR_CHEAT },
	{ &cg_cmdTimeNudge,				"cg_cmdTimeNudge",			"0",		CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_projectileNudge,			"cg_projectileNudge",		"0",		CVAR_ARCHIVE },
	{ &cg_optimizePrediction,		"cg_optimizePrediction",	"1",		CVAR_ARCHIVE },
	{ &cl_timeNudge,				"cl_timeNudge",				"0",		CVAR_ARCHIVE },
	{ &cg_latentSnaps,				"cg_latentSnaps",			"0",		CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_latentCmds,				"cg_latentCmds",			"0",		CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_plOut,					"cg_plOut",					"0",		CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_predictItems,				"cg_predictItems",			"1",		CVAR_ARCHIVE },
	{ &cg_predictWeapons,			"cg_predictWeapons",		"0",		CVAR_ARCHIVE },

	{ &cg_recording_statusline,		"cg_recording_statusline",	"9",		CVAR_ARCHIVE },
	{ &cl_demorecording,			"cl_demorecording",			"0",		CVAR_ROM },
	{ &cl_demofilename,				"cl_demofilename",			"",			CVAR_ROM },
	{ &cl_demooffset,				"cl_demooffset",			"0",		CVAR_ROM },

	{ &cg_altObits,					"cg_alternateObituaries",	"0",		CVAR_ARCHIVE },			// Slothy
	{ &ui_altObitsX,				"ui_altObitsX",				"570",		CVAR_ARCHIVE },			// Slothy
	{ &ui_altObitsY,				"ui_altObitsY",				"100",		CVAR_ARCHIVE },			// Slothy


	{ &cl_waverecording,			"cl_waverecording",			"0",		CVAR_ROM },
	{ &cl_wavefilename,				"cl_wavefilename",			"",			CVAR_ROM },
	{ &cl_waveoffset,				"cl_waveoffset",			"0",		CVAR_ROM },

	{ &developer,					"developer",				"0",		CVAR_CHEAT },
	{ NULL,							"userinfo",					"",			CVAR_CHEAT },
	{ &r_subdivisions,				"r_subdivisions",			"4",		CVAR_ARCHIVE },

	{ &demo_avifpsF1,				"demo_avifpsF1",			"0",		CVAR_ARCHIVE },
	{ &demo_avifpsF2,				"demo_avifpsF2",			"10",		CVAR_ARCHIVE },
	{ &demo_avifpsF3,				"demo_avifpsF3",			"15",		CVAR_ARCHIVE },
	{ &demo_avifpsF4,				"demo_avifpsF4",			"20",		CVAR_ARCHIVE },
	{ &demo_avifpsF5,				"demo_avifpsF5",			"24",		CVAR_ARCHIVE },
	{ &demo_avifpsF6,				"demo_avifpsF6",			"30",		CVAR_ARCHIVE },
	{ &demo_drawTimeScale,			"demo_drawTimeScale",		"1",		CVAR_ARCHIVE },
	{ &demo_infoWindow,				"demo_infoWindow",			"1",		CVAR_ARCHIVE },
	//{ &demo_baseFOV,				"demo_baseFOV",				"120",		CVAR_ARCHIVE },
	{ &demo_scoresToggle,			"demo_scoresToggle",		"1",		CVAR_ARCHIVE },

	{ &cg_drawFollowText,			"cg_drawFollowText",		"1",		CVAR_ARCHIVE },

#ifdef BUILD_BOTS
	{ &cg_omnibotdrawing,			"cg_omnibotdrawing",		"0",		CVAR_ARCHIVE },
#endif

	{ &cg_rocketTrail,				"cg_rocketTrail",			"1",		CVAR_ARCHIVE },
	{ &cg_grenadeTrail,				"cg_grenadeTrail",			"1",		CVAR_ARCHIVE },
	{ &cg_pipeTrail,				"cg_pipeTrail",				"1",		CVAR_ARCHIVE },
	{ &cg_napalmTrail,				"cg_napalmTrail",			"1",		CVAR_ARCHIVE },
	{ &cg_dartTrail,				"cg_dartTrail",				"1",		CVAR_ARCHIVE },
	{ &cg_shotgunPuff,				"cg_shotgunPuff",			"1",		CVAR_ARCHIVE },

	{ &cl_anonymous,				"cl_anonymous",				"0",		CVAR_CHEAT },
};

static int cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	int			defaultValue;
	int			min;
	int			max;

	// these aren't really usefull for cvars that allow big values
	int			includeValues;
	int			excludeValues;
	qboolean	includeNull;		// special as we need it at times
} cvarLimitTable_t;

static cvarLimitTable_t cvarLimitTable[] = {
	{ &r_singleShader,		"r_singleShader",		0,		0,		0,		0,	0,	qfalse },
	{ &r_fullBright,		"r_fullBright",			0,		0,		0,		0,	0,	qfalse },
	{ &r_showTris,			"r_showTris",			0,		0,		0,		0,	0,	qfalse },
	{ &r_showNormals,		"r_showNormals",		0,		0,		0,		0,	0,	qfalse },
	{ &cg_shadows,			"cg_shadows",			1,		0,		3,		0,	0,	qfalse },
	{ &rate,				"rate",					3000,	2500,	-1,		0,	0,	qfalse },
	{ &snaps,				"snaps",				20,		20,		-1,		0,	0,	qfalse },
	{ &cl_maxpackets,		"cl_maxpackets",		40,		30,		-1,		0,	0,	qfalse },
	{ &r_maxpolys,			"r_maxpolys",			1800,	1800,	-1,		0,	0,	qfalse },
	{ &r_maxpolyverts,		"r_maxpolyverts",		9000,	9000,	-1,		0,	0,	qfalse },
	{ &com_maxfps,			"com_maxfps",			85,		30,		130,	0,	0,	qfalse },
	{ &cg_fov,				"cg_fov",				90,		5,		135,	0,	0,	qfalse },
	{ &cg_thirdPerson,		"cg_thirdPerson",		0,		0,		0,		0,	0,	qfalse },
	{ &r_lodCurveError,		"r_lodCurveError",		250,	1,		-1,		0,	0,	qfalse },
	{ &cg_drawFriendSize,	"cg_drawFriendSize",	6,		6,		24,		0,	0,	qfalse },

	/* Ensiform - Adding this to make sure it can only be enabled during demos or developer */
	{ &cg_hideScope,		"cg_hideScope",		0,		0,		0,		0,	0,	qfalse },

	//slothy
	//{ &r_subdivisions,		"r_subdivisions",		4,		2,		150,	0,	0,	qfalse },
	// slothy

	// taken from ut
	{ &r_clear,				"r_clear",				0,		0,		0,		0,	0,	qfalse },
	{ &r_debugSort,			"r_debugSort",			0,		0,		0,		0,	0,	qfalse },
	{ &r_debugSurface,		"r_debugSurface",		0,		0,		0,		0,	0,	qfalse },
	{ &r_directedScale,		"r_directedScale",		0,		0,		2,		0,	0,	qfalse },
	{ &r_drawworld,			"r_drawworld",			1,		1,		1,		0,	0,	qfalse },
	{ &r_lockpvs,			"r_lockpvs",			0,		0,		0,		0,	0,	qfalse },
	{ &r_nocull,			"r_nocull",				0,		0,		0,		0,	0,	qfalse },
	{ &r_nocurves,			"r_nocurves",			0,		0,		0,		0,	0,	qfalse },
	{ &r_noportals,			"r_noportals",			0,		0,		0,		0,	0,	qfalse },
	{ &r_novis,				"r_novis",				0,		0,		0,		0,	0,	qfalse },
	{ &r_showclusters,		"r_showclusters",		0,		0,		0,		0,	0,	qfalse },
	{ &r_lightmap,			"r_lightmap",			0,		0,		0,		0,	0,	qfalse },
	//Unlagged cvars
	{ &cg_cmdTimeNudge,		"cg_cmdTimeNudge",		0,		0,		999,	0,	0,	qtrue },
	{ &cl_timeNudge,		"cl_timeNudge",			0,		-50,	50,		0,	0,	qtrue },
	{ &cg_latentSnaps,		"cg_latentSnaps",		0,		0,		10,		0,	0,	qtrue },
	{ &cg_latentCmds,		"cg_latentCmds",		0,		0,	MAX_LATENT_CMDS - 1 ,	0,	0,	qtrue },
	{ &cg_plOut,			"cg_plOut",				0,		0,		100 ,	0,	0,	qtrue },
	// hunkmegs
	{ &com_hunkmegs,		"com_hunkmegs",			128,		128,		-1,		0,	0,	qfalse },

	{ &cl_anonymous,		"cl_anonymous",			0,		0,		0,		0,	0,	qfalse },
};

static int cvarLimitTableSize = sizeof( cvarLimitTable ) / sizeof( cvarLimitTable[0] );

/*
=================
CG_LimitCvars
=================
*/
void CG_LimitCvars( void ) {
	int					i;
	cvarLimitTable_t	*cvl;

	if ( developer.integer || cg.demoPlayback ) {
		return;
	}

	if ( r_subdivisions.value < 0.5f ) {
		trap_Cvar_Set( "r_subdivisions", "0.5" );
		trap_Cvar_Update( &r_subdivisions );
	}
	else if ( r_subdivisions.integer > 150 ) {
		trap_Cvar_Set( "r_subdivisions", "150" );
		trap_Cvar_Update( &r_subdivisions );
	}

	for ( i = 0, cvl = cvarLimitTable; i < cvarLimitTableSize ; i++, cvl++ ) {
		if( cvl->vmCvar->integer == 0 && cvl->includeNull )
			continue;

		if( cvl->min != -1 ) {
			if( cvl->vmCvar->integer < cvl->min ) {
				if( cvl->includeValues && (cvl->includeValues & (1<<cvl->vmCvar->integer)) )
					continue;

				trap_Cvar_Set( cvl->cvarName, va( "%i", cvl->min ) );
				cvl->vmCvar->integer = cvl->min;
				continue;
			}
		}
		if( cvl->max != -1 ) {
			if( cvl->vmCvar->integer > cvl->max ) {
				if( cvl->includeValues && (cvl->includeValues & (1<<cvl->vmCvar->integer)) )
					continue;

				trap_Cvar_Set( cvl->cvarName, va( "%i", cvl->max ) );
				cvl->vmCvar->integer = cvl->max;
				continue;
			}
		}
		if( cvl->excludeValues && (cvl->excludeValues & (1<<cvl->vmCvar->integer)) ) {
			trap_Cvar_Set( cvl->cvarName, va( "%i", cvl->defaultValue ) );
			cvl->vmCvar->integer = cvl->defaultValue;
			continue;
		}
	}
}

/*
=================
CG_Q3F_UpdateCvarLimits
=================
*/
void CG_Q3F_UpdateCvarLimit( const char *cvarname, int *min, int *max ) {
	int					i;
	cvarLimitTable_t	*cvl;

	for ( i = 0, cvl = cvarLimitTable; i < cvarLimitTableSize ; i++, cvl++ ) {
		if( !Q_stricmp( cvarname, cvl->cvarName ) ) {
			cvl->min = *min;
			cvl->max = *max;
			break;
		}
	}
}

void CG_Q3F_UpdateCvarLimits( ) {
	char *ptr;
	char cvarname[64], buff[16];
	int min, max;
	size_t len;

	ptr = (char *)CG_ConfigString( CS_Q3F_CVARLIMITS );

	while( *ptr ) {
		for( len = 0; *ptr && *ptr != ' ' && len < (sizeof(cvarname) - 1); len++ ) {
			cvarname[len] = *ptr++;
		}
		while( *ptr && *ptr == ' ' )
			ptr++;
		cvarname[len] = 0;

		for( len = 0; *ptr && *ptr != ' ' && len < (sizeof(buff) - 1); len++ ) {
			buff[len] = *ptr++;
		}
		while( *ptr && *ptr == ' ' )
			ptr++;
		buff[len] = 0;
		min = atoi(buff);

		for( len = 0; *ptr && *ptr != ' ' && len < (sizeof(buff) - 1); len++ ) {
			buff[len] = *ptr++;
		}
		while( *ptr && *ptr == ' ' )
			ptr++;
		buff[len] = 0;
		max = atoi(buff);

		CG_Q3F_UpdateCvarLimit( cvarname, &min, &max );
	}
}

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
      if(cv->vmCvar != NULL) cv->modificationCount = cv->vmCvar->modificationCount;
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	drawSkyPortalModificationCount = cg_drawSkyPortal.modificationCount;

   //keeg set crosshair colors, taken from ET code
	BG_setCrosshair(cg_crosshairColor.string, cg.xhairColor, cg_crosshairAlpha.value, "cg_crosshairColor");
	BG_setCrosshair(cg_crosshairColorAlt.string, cg.xhairColorAlt, cg_crosshairAlphaAlt.value, "cg_crosshairColorAlt");

	// limit cvars
	CG_LimitCvars();
}


static qboolean CG_CheckExecKey( int key ) {
	if (cgs.eventHandling == CGAME_EVENT_MENUMODE ) {
		if ( key >= '0' && key <= '9')
            return qtrue;
	} else if (cgs.eventHandling == CGAME_EVENT_CUSTOMMENU ) {
		return CG_Q3F_CustomMenuExecKey( key );
	}
	return qfalse;
}

/*
===================
CG_Q3F_RemapSkyShader
===================
*/
void CG_Q3F_RemapSkyShader( void ) {
	if (!cgs.skyportal.hasportal )
		return;
	if( cg_drawSkyPortal.integer ) {
		trap_R_RemapShader( cgs.skyportal.portalShader, cgs.skyportal.portalShader, "0" );
	} else {
		trap_R_RemapShader( cgs.skyportal.portalShader, cgs.skyportal.disableShader, "0" );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		if( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

         //Keeger:  bring in xhair from ET
         if(cv->modificationCount != cv->vmCvar->modificationCount) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if(cv->vmCvar == &cg_crosshairColor || cv->vmCvar == &cg_crosshairAlpha) {
					BG_setCrosshair(cg_crosshairColor.string, cg.xhairColor, cg_crosshairAlpha.value, "cg_crosshairColor");
				}

				else if(cv->vmCvar == &cg_crosshairColorAlt || cv->vmCvar == &cg_crosshairAlphaAlt) {
					BG_setCrosshair(cg_crosshairColorAlt.string, cg.xhairColorAlt, cg_crosshairAlphaAlt.value, "cg_crosshairColorAlt");
				}
         }  
		}
	}

	// check for modications here

	if( cg_grenadePrimeSound.modificationCount != cgs.grenadePrimeSoundModificationCount ) {
		cgs.media.grenadePrimeSound = 0;
	}

	// if cg_drawSkyPortal changed, update skybox shader remapping
	if( drawSkyPortalModificationCount != cg_drawSkyPortal.modificationCount ) {
		drawSkyPortalModificationCount = cg_drawSkyPortal.modificationCount;
		CG_Q3F_RemapSkyShader();
	}

	// limit cvars
	CG_LimitCvars();
}


int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}


int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void CG_PlayDelayedSounds( void ) {
	int i;
	bufferedSound_t *s;

	if(!delayedSounds)
		return;

	i = 0;
	while((i < MAX_BUFFERSOUNDS) && delayedSounds) {
		s = &cg.bufferedSounds[i];
		if(s->valid) {
			if(s->maxtime < cg.time) {
				s->valid = qfalse;
				--delayedSounds;
			} else {
				if(cgs.gameSounds[ s->soundnum ]) {
					s->valid = qfalse;
					--delayedSounds;
					trap_S_StartLocalSound( cgs.gameSounds[ s->soundnum ], CHAN_LOCAL );
				}
			}
		}
		i++;
	}
}

void CG_BufferSound(int soundnum) {
	int i;
	bufferedSound_t *s;

	for(i = 0; i < MAX_BUFFERSOUNDS; i++) {
		s = &cg.bufferedSounds[i];
		if(!s->valid) {
			s->valid = qtrue;
			s->soundnum = soundnum;
			s->maxtime = cg.time + 1000;
			++delayedSounds;
			return;
		}
	}
}

// Golliwog: Play a generic local sound on the client side
void CG_Q3F_PlayLocalSound( const char *sound )
{
	// If sound is an integer, assume it's a cached config sound, otherwise
	// load it in specially.

	int soundid;
	char *ptr;
	//sfxHandle_t soundhandle;

	if( !sound || !*sound )
		return;

		// Check it's numeric
	soundid = -2;
	for( ptr = (char *) sound; *ptr; ptr++ )
	{
		if( *ptr < '0' || *ptr > '9' )
		{
			soundid = -1;
			break;
		}
	}
	if( soundid == -2 )
		soundid = atoi( sound );

	if ( soundid >= 0 )
	{
		// Play the sound
		if( cgs.gameSounds[ soundid ] )
			trap_S_StartLocalSound( cgs.gameSounds[ soundid ], CHAN_LOCAL );
		else
			CG_BufferSound(soundid);

	} else {
		// Play a custom sound - hopefully it only registers a sound once,
		// but I don't intend to ever reach here if I can get away with it.
		sfxHandle_t sh = trap_S_RegisterSound( sound, qtrue );
		if( sh )
			trap_S_StartLocalSound( sh, CHAN_LOCAL );
	}
}
// Golliwog.

void CG_AddToTextBox(char* text, char* buffer, int* times, int max, int bufferwidth) {
	int i;

	for(i = max-1; i > 0 ; i--) {
		times[i] = times[i-1];
		Q_strncpyz(&buffer[i*bufferwidth], &buffer[(i-1)*bufferwidth], bufferwidth);
	}

	times[0] = cg.time + (con_notifytime_etf.integer*1000);
	Q_strncpyz(buffer, text, bufferwidth);

	trap_Print(text);
	trap_Print("\n");
}

void QDECL CG_Printf( int mode, const char *msg, ... ) {
	va_list		argptr;
	char		buffer[1024];
	char		*p, *s;
	char*		buf;
	int*		times;
	int			max;

	va_start (argptr, msg);
	Q_vsnprintf (buffer, sizeof(buffer), msg, argptr);
	va_end (argptr);

	if(!Q_IsColorString(buffer)) {
		Q_strncpyz(buffer, va("^7%s", buffer), 1024);
	}

	if(cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
		trap_SendConsoleCommand(va("ui_chat %s\n", buffer));
	}

	switch(mode) {
	case BOX_PRINT_MODE_CONSOLE:
		s = buffer;
		for( p = s; *p;) {
			if(*p == '\n') {
				*p++ = '\0';
				trap_Print(s);
				trap_Print("\n");
				s = p;
			}
			else {
				p++;
			}
		}
		return;
	case BOX_PRINT_MODE_CHAT:
		if(cg_onlychats.integer) {
			s = buffer;
			for( p = s; *p;) {
				if(*p == '\n') {
					*p++ = '\0';
					trap_Print(s);
					trap_Print("\n");
					s = p;
				}
				else {
					p++;
				}
			}
			//trap_Print(buffer);
			//trap_Print("\n");
			return;
		}
	case BOX_PRINT_MODE_CHAT_REAL:
		buf = cg.basicChat;
		times = cg.basicChatTimes;
		max = MAX_BASICCHAT_STRINGS;
		cg.basicChatLinesWrapped = qfalse;
		cg.basicChatLinesChanged = qtrue;

		s = buffer;
		for( p = s; *p;) {
			if(*p == '\n') {
				*p++ = '\0';
				CG_AddToTextBox(s, buf, times, max, MAX_SAY_TEXT);
				s = p;
			}
			else {
				p++;
			}
		}

		break;
	case BOX_PRINT_MODE_TEAMCHAT:

		if(cg_mergemm2.integer) {
			cg.basicChatLinesWrapped = qfalse;
			cg.basicChatLinesChanged = qtrue;
		}

		if(cg_mergemm2.integer != 1) {
			cg.teamChatLinesWrapped = qfalse;
			cg.teamChatLinesChanged = qtrue;
		}

		s = buffer;
		for( p = s; *p;) {
			if(*p == '\n') {
				*p++ = '\0';
				if(cg_mergemm2.integer) {
					CG_AddToTextBox(s, cg.basicChat, cg.basicChatTimes, MAX_BASICCHAT_STRINGS, MAX_SAY_TEXT);
				}
				if(cg_mergemm2.integer != 1) {
					CG_AddToTextBox(s, cg.teamChat, cg.teamChatTimes, MAX_TEAMCHAT_STRINGS, MAX_SAY_TEXT);
				}
				s = p;
			}
			else {
				p++;
			}
		}
		break;
	case BOX_PRINT_MODE_CENTER:
		Q_strncpyz(cg.centerPrintText, buffer, 1024);
		cg.centerPrintTime = cg.time + 5000;
		cg.centerPrintRealigned = qfalse;
		cg.centerPrintSolid = qfalse;
		cg.centerprintWrapped = qfalse;
		return;
	default:
		return;
	}
}

void QDECL CG_LowPriority_Printf( int mode, const char *msg, ... ) {
	va_list		argptr;
	char		buffer[1024];
	char		*p, *s;
	char*		buf;
	int*		times;
	int			max;

	va_start (argptr, msg);
	Q_vsnprintf (buffer, sizeof(buffer), msg, argptr);
	va_end (argptr);

	if(!Q_IsColorString(buffer)) {
		Q_strncpyz(buffer, va("^7%s", buffer), 1024);
	}

	if(cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
		trap_SendConsoleCommand(va("ui_chat %s\n", buffer));
	}

	switch(mode) {
	case BOX_PRINT_MODE_CHAT:
		buf = cg.basicChat;
		times = cg.basicChatTimes;
		max = MAX_BASICCHAT_STRINGS;
		cg.teamChatLinesWrapped = qfalse;
		cg.teamChatLinesChanged = qtrue;
		break;
	case BOX_PRINT_MODE_TEAMCHAT:
		buf = cg.teamChat;
		times = cg.teamChatTimes;
		max = MAX_TEAMCHAT_STRINGS;
		cg.teamChatLinesWrapped = qfalse;
		cg.teamChatLinesChanged = qtrue;
		break;
	case BOX_PRINT_MODE_CENTER:
		if(cg.centerPrintTime + 1500 > cg.time) {
 			return;
 		}
		Q_strncpyz(cg.centerPrintText, buffer, 1024);
		cg.centerPrintTime = cg.time + 1000;
		cg.centerPrintRealigned = qfalse;
		cg.centerPrintSolid = qtrue;
		cg.centerprintWrapped = qfalse;
		return;
	default:
		return;
	}

	if(times[0] > cg.time) {
		return;
	}

	s = buffer;
	for( p = s; *p;) {
		if(*p == '\n') {
			*p++ = '\0';
			CG_AddToTextBox(s, buf, times, max, MAX_SAY_TEXT);
			s = p;
		}
		else {
			p++;
		}
	}
}


void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	//CG_Printf ( BOX_PRINT_MODE_CHAT, "%s", text);
	trap_Print( text );
}

#endif



/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}

/*																																			
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString() {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == Q3F_TEAM_SPECTATOR ) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s^7     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2, 0 );
}

// RR2DO2 : hud scripting stuff
char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.font.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.font.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.font.bigFont);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse;
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
		handle = trap_PC_LoadSource("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token); 
	}
	return qfalse;
}

void CG_LoadMenus(const char *menuFile, qboolean resetHud) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		Com_Printf( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
		len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			trap_Error( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!^7" );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	
	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	// Read hud item positions from config file
	if( !resetHud && cg_userHud.integer >= 0) {
		char buff[1024];
		const char *hudSet;

		trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));

		hudSet = buff;
		if (hudSet[0] == '\0') {
			hudSet = "ui/hud.txt";
		}
		//len = trap_FS_FOpenFile( va( "ui/userhud%i.cfg", cg_userHud.integer ), &f, FS_READ );
		Com_ExtractFilePath( hudSet, buff );
		len = trap_FS_FOpenFile( va( "%s/userhud%i.cfg", buff, cg_userHud.integer ), &f, FS_READ );

		if( len ) {
			int bytesread = 0;
			char linebuff[256];

			while( UMC_ReadLineSkipEmpty( f, &bytesread, len, linebuff, sizeof(linebuff) ) ) {
				float x, y;
				menuDef_t *menu;
				char *p, *p2;
				
				for(p = linebuff; *p; p++) {
					if(*p == ' ') {
						*p++ = '\0';
						break;
					}
				}

				for(p2 = p; *p2; p2++) {
					if(*p2 == ' ') {
						*p2++ = '\0';
						break;
					}
				}

				x = atof(p);
				y = atof(p2);

				menu = Menus_FindByName( linebuff );

				if( menu ) {
					menu->realRect.x = x;
					menu->realRect.y = y;
					menu->window.rect.x = x;
					menu->window.rect.y = y;
					Menu_UpdatePosition(menu);
				}
			}

			trap_FS_FCloseFile( f );
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	return qfalse;
}


static int CG_FeederCount(float feederID, itemDef_t* item) {

	if (feederID == FEEDER_SCOREBOARD) {
		return cg.numScores;
	} else if (feederID == FEEDER_CUSTOMMENU) {
		return CG_Q3F_CustomMenuItems();
	} 
	
	return 0;
}

#if 0 // UNUSED
// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;

	count = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == team) {
			if (count == index) {
				*scoreIndex = i;
				return &cgs.clientinfo[cg.scores[i].client];
			}
			count++;
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}
#endif

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle, itemDef_t* item) {
	*handle = -1;
	if (feederID == FEEDER_CUSTOMMENU) {
		return CG_Q3F_CustomMenuGetItem( index, column );
	} 
	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index, itemDef_t* item) {
	return 0;
}

static void CG_FeederSelection(float feederID, int index) {
}

typedef enum { 
	SCORE_SORT_TEAM,
	SCORE_SORT_NAME,
	SCORE_SORT_FRAGS,
	SCORE_SORT_PING,
	SCORE_SORT_MAX
} scoreSort_t;

typedef struct {
	score_t*		score;
	clientInfo_t*	client;
} scoreInfo_t;

scoreInfo_t sortedScores[MAX_CLIENTS];

/*static score_t* CG_ScoreFromClientinfo(clientInfo_t* client) {
	int i;

	for (i = 0; i < cg.numScores; i++) {
		if (&cgs.clientinfo[cg.scores[i].client] == client) {
			return &cg.scores[i];
		}
	}
	return NULL;
}*/

#define ta ((scoreInfo_t*)a)
#define tb ((scoreInfo_t*)b)

static int SB_SortFunc(const void* a, const void* b) {
	int tmp;

	switch(cg_scoreboardsortmode.integer % SCORE_SORT_MAX) {
		case SCORE_SORT_TEAM:
		default:
			return	((ta->client->team < Q3F_TEAM_RED) || (ta->client->team > Q3F_TEAM_GREEN)) ? 1 : 
					((tb->client->team < Q3F_TEAM_RED) || (tb->client->team > Q3F_TEAM_GREEN)) ? -1 :
					cg.teamScores[ta->client->team] > cg.teamScores[tb->client->team] ? 1 : 
					cg.teamScores[ta->client->team] < cg.teamScores[tb->client->team] ? -1 : 
					ta->client->team > tb->client->team ? -1 :
					ta->client->team < tb->client->team ? 1 :
					ta->score->score > tb->score->score ? -1 : 
					ta->score->score < tb->score->score ? 1 : 0;

		case SCORE_SORT_NAME:
			tmp = Q_stricmp(ta->client->name, tb->client->name);

			return	tmp ? tmp : 
					ta->score->score > tb->score->score ? -1 : 
					ta->score->score < tb->score->score ? 1 : 0;
		case SCORE_SORT_FRAGS:
			return	ta->score->score > tb->score->score ? -1 : 
					ta->score->score < tb->score->score ? 1 : 0;
		case SCORE_SORT_PING:			
			return	ta->score->ping > tb->score->ping ? -1 : 
					ta->score->ping < tb->score->ping ? 1 : 
					ta->score->score > tb->score->score ? -1 : 
					ta->score->score < tb->score->score ? 1 : 0;
	}
}

void CG_SortScoreboard() {
	int i;

	for(i = 0; i < cg.numScores; i++) {
		sortedScores[i].score	= &cg.scores[i];
		sortedScores[i].client	= &cgs.clientinfo[sortedScores[i].score->client];
	}

	qsort( sortedScores, cg.numScores, sizeof(scoreInfo_t), SB_SortFunc);
}

void PaintScoreHeader(itemDef_t* item, int teamnum, int *x, int *y, int height, vec4_t backClr) {
	int x2, myx;
	int j;
	char buffer[64];
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );
	static qhandle_t bar = NULL_HANDLE;
	float x1, y1, w1, h1, w;

	x1 = myx = *x;
	x1 -= 5;
	w1 = 0;
	
//	CG_FillRect(*x + 2, *y + 2, item->window.rect.w - SCROLLBAR_SIZE - 32, listPtr->elementHeight, item->window.outlineColor);
	for (j = 0; j < listPtr->numColumns; j++) {

		x2 = myx + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5); 
		w = myx + 2 + listPtr->columnInfo[j].pos + listPtr->columnInfo[j].width;
		if(w > w1)
			w1 = w;

		switch(j) {
			case 0: // blank
				CG_FillRect(myx + 2 + listPtr->columnInfo[j].pos,*y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				break;
			case 6:
				if(cg.warmup) 
					CG_FillRect(myx + 2 + listPtr->columnInfo[j].pos,*y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				break;
			case 3:	// team ping
				CG_FillRect(myx + 2 + listPtr->columnInfo[j].pos,*y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if( cg.teamCounts[teamnum-1] ) {
					Com_sprintf( buffer, sizeof(buffer), "%i", cg.teamPings[teamnum-1] );
				} else {
					Com_sprintf( buffer, sizeof(buffer), "0" );
				}

				CG_Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 2:	// team score
				CG_FillRect(myx + 2 + listPtr->columnInfo[j].pos,*y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( cg.teamScores[teamnum-1] == SCORE_NOT_PRESENT ) {
					Com_sprintf (buffer, sizeof(buffer), "-");
				}
				else {
					Com_sprintf (buffer, sizeof(buffer), "%i", ( cg.teamScores[teamnum-1] > 9999 ? 9999 : cg.teamScores[teamnum-1] ) );
				}
				CG_Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 1:	// team name
				trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[teamnum-1]), buffer, sizeof(buffer));
				CG_Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
		}
	}
	*y += height;

	if(bar == 0)
	{
		bar = trap_R_RegisterShaderNoMip("ui/gfx/load_bar_fill_full");
		if(!bar)
			bar = -1;
	}

	if(bar > 0){
		y1 = *y + 2;
		h1 = 2;
		w1 -= x1;
		CG_AdjustFrom640(&x1, &y1, &w1, &h1);
		trap_R_DrawStretchPic(x1, y1, w1, h1, 0.02f, 0.02f, 0.98f, 0.98f, bar);
		*y += 2;
	}
}

void PaintScoreFeeder(itemDef_t* item, int teamnum) {
	int x, y;
	int count = cgDC.feederCount(FEEDER_SCOREBOARD, item);
	int i;
	vec4_t backClr;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );

	x = item->window.rect.x + 1;
	y = item->window.rect.y + 1;

	memcpy( backClr, CG_TeamColor_Scoreboard(teamnum), sizeof(vec4_t));
	backClr[3] *= 0.3f;
	PaintScoreHeader(item, teamnum, &x, &y, listPtr->elementHeight, backClr);

	for (i = listPtr->startPos; i < count; i++) {
		int j;
		
		if(sortedScores[i].client->team != (q3f_team_t)teamnum) {
			continue;
		}

		if (cg.snap->ps.clientNum == sortedScores[i].score->client) {
			CG_FillRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 32, listPtr->elementHeight, item->window.outlineColor);
		}

		for (j = 0; j < listPtr->numColumns; j++) {
			int x2 = x;
			char* buf = NULL;

			x2 = x + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5);

			switch(j) {
			case 0: // class picture

				if(!CG_AlliedTeam(sortedScores[i].client->team, cgs.clientinfo[cg.clientNum].team)) {
					break;
				}
				// Ensiform: Switching below to the above statement so that team allies can see the class pictures
				//if(sortedScores[i].client->team != cgs.clientinfo[cg.clientNum].team) {
				//	break;
				//}

				if(sortedScores[i].client->cls <= Q3F_CLASS_NULL || sortedScores[i].client->cls >= Q3F_CLASS_MAX) {
					break;
				}

				x2 = x + 2 + listPtr->columnInfo[j].pos + ((listPtr->columnInfo[j].width - listPtr->elementHeight) * 0.5f);

				CG_FillRect( x2, y + 2, listPtr->elementHeight, listPtr->elementHeight, backClr);

				{
					float x1, y1, w1, h1;
					x1 = x2;
					y1 = y + 2;
					w1 = listPtr->elementHeight;
					h1 = listPtr->elementHeight;
					CG_AdjustFrom640(&x1, &y1, &w1, &h1);
					trap_R_DrawStretchPic(x1, y1, w1, h1, 0.02f, 0.02f, 0.98f, 0.98f, *CG_Q3F_ModelIcon( sortedScores[i].client->cls ));
				}
				break;
			case 1:
				CG_Text_Paint_MaxWidth(x + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, sortedScores[i].client->name, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT, listPtr->columnInfo[j].width - item->textalignx);
				break;
			case 2:
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping < 0 ) {
					break;
				}
				CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->score), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 3:
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping < 0 ) {
					break;
				}
				if (sortedScores[i].score->flags & SCORE_BOT) 
					CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, "BOT" , 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				else
					CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->ping), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 4:
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping < 0 ) {
					break;
				}
				CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->time), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 5:
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping == -1 ) {
					buf = "connecting";
				} else if ( sortedScores[i].score->ping == -2 ) {
					buf = "initializing";
				}

				if(buf)
					CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, buf, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 6:
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if(cg.warmup) {
					if (sortedScores[i].score->flags & SCORE_FLAG_READY) {
						// client is ready
						CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, "R", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					} else
						CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, "N", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				}
				break;
			}
		}

		size -= listPtr->elementHeight;
		if (size < listPtr->elementHeight) {
			listPtr->drawPadding = listPtr->elementHeight - size;
			break;
		}
		listPtr->endPos++;
		y += listPtr->elementHeight;
		// fit++;
	}
}

// find the n'th valid team and display it
void PaintScoreFeederTeam(itemDef_t* item, int teamnum) {
	int i, j;

	j = 0;
	for(i = 1; i <= 4; i++)
	{
		if(cgs.teams & ( 1 << i ))
			j++;
		if(j == teamnum) {
			PaintScoreFeeder(item, i);
			return;
		}
	}
}

void PaintSpecFeeder(itemDef_t* item) {
	int x, y;
	int count = cgDC.feederCount(FEEDER_SCOREBOARD, item);
	int i, following;
	vec4_t backClr;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );
	char buf[50]; // Ensiform: was 42 should have been 43 but now should be 47 but im bumping it to 50 to be safe with (S)
	x = item->window.rect.x + 1; 
	y = item->window.rect.y + 1 + item->window.rect.h;
	i = listPtr->startPos;

	while(i < count) {
		int j;
		int col;

		if(sortedScores[i].client->team != Q3F_TEAM_SPECTATOR) {
			i++;
			continue;
		}

		for (j = 0; j < listPtr->numColumns; j++) {
			int x2 = x;

			col = j % 4;
			if((!col) && (j))
			{
				i++;
				if(i == count)
					break;
			}

			x2 = x + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5);
			following = sortedScores[i].score->score;

			if(sortedScores[i].score->flags & SCORE_FLAG_SPECFREE)
                memcpy( backClr, CG_TeamColor_Scoreboard(Q3F_TEAM_SPECTATOR), sizeof(vec4_t));
			else if(sortedScores[i].score->flags & SCORE_FLAG_SPECFLYBY)
				memcpy( backClr, CG_TeamColor_Scoreboard(Q3F_TEAM_SPECTATOR), sizeof(vec4_t));
			else 
				memcpy(backClr, CG_TeamColor_Scoreboard(cgs.clientinfo[following].team), sizeof(vec4_t));

			backClr[3] *= 0.3f; 

			switch(col) {
			case 0:	// ping
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping < 0 ) {
					break;
				}
				CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->ping), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 1:	// time
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if ( sortedScores[i].score->ping < 0 ) {
					break;
				}
				CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->time), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 2: // name
				memcpy( backClr, CG_TeamColor_Scoreboard(Q3F_TEAM_SPECTATOR), sizeof(vec4_t));
				backClr[3] *= 0.3f;
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				CG_Text_Paint_MaxWidth(x + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, sortedScores[i].client->name, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT, listPtr->columnInfo[j].width - item->textalignx);
				break;
			case 3:	// activity
				memcpy( backClr, CG_TeamColor_Scoreboard(Q3F_TEAM_SPECTATOR), sizeof(vec4_t));
				backClr[3] *= 0.3f;
				CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
				if(sortedScores[i].score->ping == -1)
					Com_sprintf(buf, sizeof(buf), "Connecting");
				else if(sortedScores[i].score->ping == -2)
					Com_sprintf(buf, sizeof(buf), "Initializing");
				else if(sortedScores[i].score->flags & SCORE_FLAG_SPECFREE)
					Com_sprintf(buf, sizeof(buf), "%sFree Flight", (sortedScores[i].score->flags & SCORE_FLAG_SHOUTCAST) ? "(S) " : "");
				else if(sortedScores[i].score->flags & SCORE_FLAG_SPECFLYBY)
					Com_sprintf(buf, sizeof(buf), "%sFlyby", (sortedScores[i].score->flags & SCORE_FLAG_SHOUTCAST) ? "(S) " : "");
				else if(sortedScores[i].score->flags & SCORE_FLAG_SPECFOLLOW)
					Com_sprintf(buf, sizeof(buf), "%sFollow %s", (sortedScores[i].score->flags & SCORE_FLAG_SHOUTCAST) ? "(S) " : "", cgs.clientinfo[following].name);
				else
					Com_sprintf(buf, sizeof(buf), "%sChases %s", (sortedScores[i].score->flags & SCORE_FLAG_SHOUTCAST) ? "(S) " : "", cgs.clientinfo[following].name);

				CG_Text_Paint_MaxWidth(x + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, buf, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT, listPtr->columnInfo[j].width - item->textalignx);
				break;
			}
		}

		size -= listPtr->elementHeight;
		if (size < listPtr->elementHeight) {
			listPtr->drawPadding = listPtr->elementHeight - size;
			break;
		}
		listPtr->endPos++;
		y -= listPtr->elementHeight;
		i++;
		// fit++;
	}
}

static qboolean CG_FeederPaintSpecial(itemDef_t* item) {
	int x, y;
	int count = cgDC.feederCount(item->special, item);
	int i;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );

	switch((int)item->special) {
	case FEEDER_SCOREBOARD:

		x = item->window.rect.x + 1;
		y = item->window.rect.y + 1;

		for (i = listPtr->startPos; i < count; i++) {
			vec4_t backClr;
			int j;
			
			memcpy( backClr, CG_TeamColor_Scoreboard(sortedScores[i].score->team), sizeof(vec4_t));
			backClr[3] *= 0.3f;

			if (cg.snap->ps.clientNum == sortedScores[i].score->client) {
				CG_FillRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 32, listPtr->elementHeight, item->window.outlineColor);
			}
				
			for (j = 0; j < listPtr->numColumns; j++) {
				int x2 = x;
				char* buf = NULL;

				x2 = x + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5);

				switch(j) {
				case 0: // class pciture
					if(sortedScores[i].client->team < Q3F_TEAM_RED || sortedScores[i].client->team > Q3F_TEAM_GREEN) {
						break;					
					}

					if(!CG_AlliedTeam(sortedScores[i].client->team, cgs.clientinfo[cg.clientNum].team)) {
						break;
					}
					// Ensiform: Switching below to the above statement so that team allies can see the class pictures
					//if(sortedScores[i].client->team != cgs.clientinfo[cg.clientNum].team) {
					//	break;
					//}

					if(sortedScores[i].client->cls <= Q3F_CLASS_NULL || sortedScores[i].client->cls >= Q3F_CLASS_MAX) {
						break;
					}

					x2 = x + 2 + listPtr->columnInfo[j].pos + ((listPtr->columnInfo[j].width - listPtr->elementHeight) * 0.5f);

					CG_FillRect( x2, y + 2, listPtr->elementHeight, listPtr->elementHeight, backClr);

					{
						float x1, y1, w1, h1;
						x1 = x2;
						y1 = y + 2;
						w1 = listPtr->elementHeight;
						h1 = listPtr->elementHeight;
						CG_AdjustFrom640(&x1, &y1, &w1, &h1);
						trap_R_DrawStretchPic(x1, y1, w1, h1, 0.02f, 0.02f, 0.98f, 0.98f, *CG_Q3F_ModelIcon( sortedScores[i].client->cls ));
					}
					break;
				case 1:
					CG_Text_Paint(x + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, sortedScores[i].client->name, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
					break;
				case 2:
					CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( sortedScores[i].score->ping < 0 ) {
						break;
					}
					CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->score), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 3:
					CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( sortedScores[i].score->ping < 0 ) {
						break;
					}

					if(sortedScores[i].score->flags & SCORE_BOT)
						CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, "BOT", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					else
						CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->ping), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 4:
					CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( sortedScores[i].score->ping < 0 ) {
						break;
					}
					CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", sortedScores[i].score->time), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 5:
					CG_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( sortedScores[i].score->ping == -1 ) {
						buf = "connecting";
					} else if ( sortedScores[i].score->ping == -2 ) {
						buf = "initializing";
					}

					if(buf)
						CG_Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, buf, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				}
			}

			size -= listPtr->elementHeight;
			if (size < listPtr->elementHeight) {
				listPtr->drawPadding = listPtr->elementHeight - size;
				break;
			}
			listPtr->endPos++;
			y += listPtr->elementHeight;
			// fit++;
		}

		return qtrue;

	case FEEDER_SCOREBOARD_TEAM1:
	case FEEDER_SCOREBOARD_TEAM2:
	case FEEDER_SCOREBOARD_TEAM3:
	case FEEDER_SCOREBOARD_TEAM4:
/*	Slothy - consider enabling this if you want scoreboards in HUDs to be uptodate
		if ( cg.scoresRequestTime + 2000 < cg.time ) {
			// the scores are more than two seconds out of data,
			// so request new ones
			cg.scoresRequestTime = cg.time;
			trap_SendClientCommand( "score" );
		}
*/
		PaintScoreFeederTeam(item, (int)item->special - FEEDER_SCOREBOARD_TEAM1 + 1);
		return qtrue;
	case FEEDER_SCOREBOARD_SPECS:
		PaintSpecFeeder(item);
		return qtrue;
	}
	return qfalse;
}

static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}

void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style, fontStruct_t *parentfont, int textalignment) {
	CG_Text_Paint(x, y, scale, color, text, 0, limit, style, parentfont, textalignment);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale, fontStruct_t *parentfont) {
	switch (ownerDraw) {
		case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0, parentfont);
			break;
		case CG_GAME_STATUS:
			return CG_Text_Width(CG_Q3F_TeamStatus(), scale, 0, parentfont);
			break;
		case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, 0, parentfont);
			break;
	}
	return 0;
}

static qboolean CG_OwnerDrawSize(int ownerDraw, rectDef_t* in, rectDef_t* out, itemDef_t* item, float* alpha) {
	menuDef_t		*parent;
	fontStruct_t	*parentfont;
	char			buffer[1024];
	qboolean		dummy;

	parent = (menuDef_t*)item->parent;
	parentfont = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

	if(cgs.eventHandling == CGAME_EVENT_EDITHUD) {
		out->x = in->x;
		out->y = in->y;
		out->w = in->w;
		out->h = in->h;
		return qtrue;
	}

	switch(ownerDraw) {
	case CG_CHATBOX_CONTENT:
	case CG_TEAMCHATBOX_CONTENT:
	case CG_CENTERPRINTBOX_CONTENT:
	case CG_CROSSHAIRINFO_BOX:
	case CG_CHATEDIT_CONTENT:
		if(!CG_GetExpandingTextBox_Text(ownerDraw, buffer, alpha, &dummy)) {
			return qfalse;
		}
		return CG_GetExpandingTextBox_Extents(in, out, item->textscale, parentfont, item->anchorx, item->anchory, item->special, buffer, ownerDraw);
	case CG_WEAPONSWITCH_BOX:
		return CG_GetWeaponSwitchBoxExtents(in, out, item->anchorx, item->anchory);
//	case CG_MENUBOX_CONTENT:
//		return CG_Q3F_GetMenuExtents(in, out, item->textscale, parentfont, item->anchorx, item->anchory, item->special);
	case CG_ALERT_ICON:
		return CG_Q3f_GetAlertIconExtents(in, out, item->anchorx, item->anchory, item->special);
	case CG_POWERUP_ICON:
		return CG_Q3F_GetPowerupIconExtents(in, out, item->anchorx, item->anchory, item->special);
	case CG_FORT_MENU_BORDER:
		return CG_Q3F_GetMenuAlpha(in, out, alpha);
	case CG_DRAWATTACKER:
		return CG_GetAttackerBoxExtents( in, out );
	default:
		return qfalse;
	}
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
	return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
	trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
	trap_CIN_SetExtents(handle, x, y, w, h);
	trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
	trap_CIN_RunCinematic(handle);
}

static void CG_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	trap_S_RealStartLocalSound(sfx, channelNum, __FILE__, __LINE__ );
}

qboolean CG_FileExists( const char *filename ) {
	fileHandle_t f = NULL_FILE;
	int len = 0;
	if ( !filename || !*filename ) {
		return qfalse;
	}
	len = trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( f != NULL_FILE )
		trap_FS_FCloseFile( f );

	return ( f != NULL_FILE && len > 0 ) ? qtrue : qfalse;
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu() {
	char buff[1024];

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawAdjustedPic = &CG_DrawAdjustedPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;   
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_RealAddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &UI_Q3F_LoadFontFile;//&trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_Q3F_GetTeamColor2;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &CG_S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	cgDC.feederPaintSpecial = &CG_FeederPaintSpecial;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error; 
	cgDC.Print = &Com_Printf; 
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	cgDC.ownerDrawSize = &CG_OwnerDrawSize;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	cgDC.openFile = &trap_FS_FOpenFile;
	cgDC.fRead = &trap_FS_Read;
	cgDC.closeFile = &trap_FS_FCloseFile;
	cgDC.keyIsDown = trap_Key_IsDown;
	cgDC.adjustFrom640 = CG_AdjustFrom640;

	//slothy
	cgDC.playerClass = 0;
	cgDC.weapon = 0;
	
	Init_Display(&cgDC);

	Menu_Reset();
	
	Q_strncpyz(buff, cg_hudFiles.string, sizeof(buff));
	if (!*buff) {
		Q_strncpyz(buff, "ui/hud.txt", sizeof(buff));
	}

	CG_LoadMenus(buff, qfalse);
}

void CG_AssetCache() {
	cgDC.Assets.fxBasePic =				trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] =				trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] =				trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] =				trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] =				trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] =				trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] =				trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] =				trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar =				trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown =	trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp =		trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft =	trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight =	trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb =		trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar =				trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb =			trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
	cgDC.Assets.LEDoff =				trap_R_RegisterShaderNoMip( ASSET_LED_OFF );
	cgDC.Assets.LEDon =					trap_R_RegisterShaderNoMip( ASSET_LED_ON );

	cgDC.Assets.xammo =					trap_R_RegisterShaderNoMip( "ui/gfx/hud/x_ammo.tga" );

	cgDC.Assets.ArmorTypes[0] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/asbestos_icon.tga" );
	cgDC.Assets.ArmorTypes[1] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/blast_icon.tga" );
	cgDC.Assets.ArmorTypes[2] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/kevlar_icon.tga" );
	cgDC.Assets.ArmorTypes[3] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/shock_icon.tga" );
	cgDC.Assets.ArmorTypes[4] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/wood_icon.tga" );

	cgDC.Assets.ArmorColor[0] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/armor_green.tga" );
	cgDC.Assets.ArmorColor[1] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/armor_red.tga" );
	cgDC.Assets.ArmorColor[2] = trap_R_RegisterShaderNoMip( "ui/gfx/hud/icons/armor_yellow.tga" );
}

// RR2DO2

/*
======================
CG_Q3F_LoadMapConfig

======================
*/
void CG_Q3F_LoadMapConfig( void ) {
	// Golliwog: load any map-specific config

	char mapcfg[MAX_QPATH], buff[MAX_QPATH];
	fileHandle_t fh;

 	if( cg_execMapConfigs.integer )
	{
		if( trap_FS_FOpenFile( "maps/map_default.cfg", &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			CG_Printf( BOX_PRINT_MODE_CHAT, "Executing map_default.cfg...\n" );
			trap_SendConsoleCommand( va( "exec map_default.cfg\n" ) );
		}

		Q_strncpyz( mapcfg, COM_SkipPath( cgs.mapname ), sizeof(mapcfg) );
		COM_StripExtension( mapcfg, buff, sizeof(buff) );
		Com_sprintf( mapcfg, sizeof(mapcfg), "%s+%d.cfg", buff, cgs.gameindex );

		if( trap_FS_FOpenFile( mapcfg, &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			CG_Printf( BOX_PRINT_MODE_CHAT, "Executing %s...\n", mapcfg );
			trap_SendConsoleCommand( va( "exec %s\n", mapcfg ) );
			return;
		}

		Q_strncpyz( mapcfg, COM_SkipPath( cgs.mapname ), sizeof(mapcfg) );
		COM_StripExtension( mapcfg, buff, sizeof(buff) );
		Com_sprintf( mapcfg, sizeof(mapcfg), "%s.cfg", buff );

		if( trap_FS_FOpenFile( mapcfg, &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			CG_Printf( BOX_PRINT_MODE_CHAT, "Executing %s...\n", mapcfg );
			trap_SendConsoleCommand( va( "exec %s\n", mapcfg ) );
		}
	}
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data

	CG_EventHandling( CGAME_EVENT_NONE, qtrue );
	if ( cg.demoPlayback ) {
		trap_Cvar_Set( "timescale", "1" );
	}
}

char *CG_Q3F_AddString( const char *str )
{
	// Duplicate a string.

	int len;
	char *newstr;

	len = strlen( str ) + 1;
	if( cgs.stringDataLen + len >= sizeof(cgs.stringData) )
		CG_Error( "CG_Q3F_AddString: Out of space." );
	newstr = strcpy( &cgs.stringData[cgs.stringDataLen], str );
	cgs.stringDataLen += len;

	return( newstr );
}

void *CG_Q3F_AddBlock( int size, int alignment )
{
	// Allocate a block of memory with the specified alignment.
	// It should generally work with a 4-byte alignment for anything...
	// but better to play safe generally and set to the size of the
	// structure involved.

	int offset;
	offset = (cgs.stringDataLen + alignment - 1) & ~(alignment - 1);
	if( offset + size >= sizeof(cgs.stringData) )
		CG_Error( "CG_Q3F_AddBlock: Out of space." );
	cgs.stringDataLen = offset + size;
	return( cgs.stringData + offset );
}

char *CG_Q3F_GetLocation( vec3_t origin, qboolean doTrace )
{
	// If there are no ents in PVS, then just use the closest outside
	// A trace is used in cgame to calculate PVS, since there's no PVS trap..

	float			bestlen, secondbestlen, len;
	int				index;
	cg_q3f_location_t *loc, *best, *secondbest;
	trace_t			tr;

	if( !cgs.numLocations )
		return( NULL );

	best = secondbest = NULL;
	bestlen = secondbestlen = 3*8192.0*8192.0;

	for( index = 0; index < cgs.numLocations; index++ )
	{
		loc = &cgs.locations[index];
		len = ( origin[0] - loc->pos[0] ) * ( origin[0] - loc->pos[0] )
			+ ( origin[1] - loc->pos[1] ) * ( origin[1] - loc->pos[1] )
			+ ( origin[2] - loc->pos[2] ) * ( origin[2] - loc->pos[2] );

		if ( len > bestlen ) {
			continue;
		}

		if( doTrace )
		{
			CG_Trace( &tr, origin, NULL, NULL, loc->pos, ENTITYNUM_NONE, CONTENTS_SOLID );
			if( !tr.startsolid && tr.fraction < 1.0f )
				continue;
		}

		if( len < secondbestlen )
		{
			secondbest = loc;
			secondbestlen = len;
		}

		bestlen = len;
		best = loc;
	}

	return( best ? best->str : (secondbest ? secondbest->str : NULL) );
}

void CG_IntermissionScoreDump() {
	int i;
	char buffer[4096];

	trap_SendConsoleCommand("ui_scoreclear\n");

	*buffer = '\0';
	for(i = 0; i < 4; i++) {
		Q_strcat(buffer, 256, va(i ? " %d" : "%d", cg.teamScores[i]));
	}

	trap_SendConsoleCommand(va("ui_teamscoredump %s %d\n", buffer, cgs.teams));

	for(i = 0; i < cg.numScores; i++) {
		*buffer = '\0';
		if(sortedScores[i].score->team >= Q3F_TEAM_RED && sortedScores[i].score->team <= Q3F_TEAM_GREEN) {
			Q_strcat(buffer, 256, va("\"%s\"", sortedScores[i].client->name));
			Q_strcat(buffer, 256, va(" %d", sortedScores[i].score->score));
			Q_strcat(buffer, 256, va(" %d", sortedScores[i].score->team));
			Q_strcat(buffer, 256, va(" %d", sortedScores[i].client->cls));
			Q_strcat(buffer, 256, va(" %d", sortedScores[i].score->time));
			if(sortedScores[i].score->flags & SCORE_BOT)
				Q_strcat(buffer, 256, " BOT");
			else
				Q_strcat(buffer, 256, va(" %d", sortedScores[i].score->ping));
			trap_SendConsoleCommand(va("ui_scoredump %s\n", buffer));
		}
	}
	trap_SendConsoleCommand("ui_scorecomplete\n");
}

void CG_SetupIntermissionMenu() {
	trap_SendConsoleCommand( "hud_ingame\n" );
}
