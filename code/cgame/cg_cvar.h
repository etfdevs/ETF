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

#ifdef EXTERN_CG_CVAR
#define CG_NULLCVAR( cvarName, defaultString, cvarFlags )
#define CG_NULLEXTCVAR( cvarName, defaultString, cvarFlags, extCvarFlags ) extern vmCvar_t vmCvar;
#define CG_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags ) extern vmCvar_t vmCvar;
#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_CG_CVAR
#define CG_NULLCVAR( cvarName, defaultString, cvarFlags )
#define CG_NULLEXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags )
#define CG_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags ) vmCvar_t vmCvar;
#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) vmCvar_t vmCvar;
#endif

#ifdef CG_CVAR_LIST
#define CG_NULLCVAR( cvarName, defaultString, cvarFlags ) { NULL, cvarName, defaultString, cvarFlags, 0, 0 },
#define CG_NULLEXTCVAR( cvarName, defaultString, cvarFlags, extCvarFlags ) { NULL, cvarName, defaultString, cvarFlags, extCvarFlags, 0 },
#define CG_EXTCVAR( vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags ) { & vmCvar, cvarName, defaultString, cvarFlags, extCvarFlags, 0 },
#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) { & vmCvar, cvarName, defaultString, cvarFlags, 0, 0 },
#endif

CG_NULLCVAR( "r_dynamiclight",									"1",								CVAR_ARCHIVE )
CG_NULLCVAR( "cl_allowDownload",								"0",								CVAR_ARCHIVE )

CG_CVAR( cg_grenadePrimeSound,		"cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE )
CG_CVAR( cg_scoreboardsortmode,		"cg_scoreboardsortmode",	"0",								CVAR_ARCHIVE )
CG_CVAR( cg_sniperDotScale,			"cg_sniperDotScale",		"0.3",								CVAR_ARCHIVE )
CG_CVAR( cg_adjustAgentSpeed,		"cg_adjustAgentSpeed",		"1",								CVAR_ARCHIVE | CVAR_USERINFO )
CG_CVAR( cg_atmosphericEffects,		"cg_atmosphericEffects",	"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawPanel,				"cg_drawPanel",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_execClassConfigs,		"cg_execClassConfigs",		"0",								CVAR_ARCHIVE )	
CG_CVAR( cg_execMapConfigs,			"cg_execMapConfigs",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_filterObituaries,		"cg_filterObituaries",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_friendlyCrosshair,		"cg_friendlyCrosshair",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_impactVibration,		"cg_impactVibration",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_lowEffects,				"cg_lowEffects",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_no3DExplosions,			"cg_no3DExplosions",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_oldSkoolMenu,			"cg_oldSkoolMenu",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_playClassSound,			"cg_playClassSound",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_showGrenadeTimer1,		"cg_showGrenadeTimer1",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_showGrenadeTimer2,		"cg_showGrenadeTimer2",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_showGrenadeTimer3,		"cg_showGrenadeTimer3",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_visualAids,				"cg_visualAids",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_showSentryCam,			"cg_showSentryCam",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_sniperDotColors,		"cg_sniperDotColors",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_sniperHistoricalSight,	"cg_sniperHistoricalSight", "0",								CVAR_ARCHIVE )
CG_CVAR( cg_flares,					"cg_flares",				"1",								CVAR_ARCHIVE )
CG_CVAR( r_fastSky,					"r_fastSky",				"0",								CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshairNames,		"cg_drawCrosshairNames",	"1",								CVAR_ARCHIVE )
CG_CVAR( cg_brassTime,				"cg_brassTime",				"2500",								CVAR_ARCHIVE )
CG_CVAR( cg_markTime,				"cg_markTime",				"20000",							CVAR_ARCHIVE )
CG_CVAR( cg_simpleItems,			"cg_simpleItems",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_grenadePrimeSound,		"cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE )
CG_CVAR( cg_drawFriend,				"cg_drawFriend",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawFriendSize,			"cg_drawFriendSize",		"6",								CVAR_ARCHIVE )
CG_CVAR( cg_mergemm2,				"cg_mergemm2",				"0",								CVAR_ARCHIVE )
CG_CVAR( cg_onlychats,				"cg_onlychats",				"0",								CVAR_ARCHIVE )
CG_CVAR( cg_debugTime,				"cg_debugTime",				"0",								CVAR_ARCHIVE )
CG_CVAR( cg_drawGun,				"cg_drawGun",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_zoomFov,				"cg_zoomfov",				"22.5",								CVAR_ARCHIVE )
CG_CVAR( cg_fov,					"cg_fov",					"90",								CVAR_ARCHIVE )
CG_CVAR( cg_fovAspectAdjust,		"cg_fovAspectAdjust",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_fovViewmodel,			"cg_fovViewmodel",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_fovViewmodelAdjust,		"cg_fovViewmodelAdjust",	"1",								CVAR_ARCHIVE )
CG_CVAR( cg_viewsize,				"cg_viewsize",				"100",								CVAR_ARCHIVE )
CG_CVAR( cg_stereoSeparation,		"cg_stereoSeparation",		"0.4",								CVAR_ARCHIVE )
CG_CVAR( cg_shadows,				"cg_shadows",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_gibs,					"cg_gibs",					"1",								CVAR_ARCHIVE )
CG_CVAR( cg_draw2D,					"cg_draw2D",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawStatus,				"cg_drawStatus",			"2",								CVAR_ARCHIVE )
CG_CVAR( cg_drawTimer,				"cg_drawTimer",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawSpeedometer,		"cg_drawSpeedometer",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_drawFPS,				"cg_drawFPS",				"0",								CVAR_ARCHIVE )
CG_CVAR( cg_drawSnapshot,			"cg_drawSnapshot",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_draw3dIcons,			"cg_draw3dIcons",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawIcons,				"cg_drawIcons",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawAmmoWarning,		"cg_drawAmmoWarning",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawAttacker,			"cg_drawAttacker",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshair,			"cg_drawCrosshair",			"4",								CVAR_ARCHIVE )
CG_CVAR( cg_drawRewards,			"cg_drawRewards",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_crosshairSize,			"cg_crosshairSize",			"24",								CVAR_ARCHIVE )
// slothy - xhair health defaults to off until it's properly implemented
CG_CVAR( cg_crosshairHealth,		"cg_crosshairHealth",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_crosshairAlpha,			"cg_crosshairAlpha",		"1.0",								CVAR_ARCHIVE )
CG_CVAR( cg_crosshairAlphaAlt,		"cg_crosshairAlphaAlt",		"1.0",								CVAR_ARCHIVE )
CG_CVAR( cg_crosshairColor,			"cg_crosshairColor",		"White",							CVAR_ARCHIVE )
CG_CVAR( cg_crosshairColorAlt,		"cg_crosshairColorAlt",		"White",							CVAR_ARCHIVE )
CG_CVAR( cg_crosshairX,				"cg_crosshairX",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_crosshairY,				"cg_crosshairY",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_bloodFlash,				"cg_bloodFlash",			"1.0",								CVAR_ARCHIVE )	
CG_CVAR( cg_lagometer,				"cg_lagometer",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_runpitch,				"cg_runpitch",				"0.002",							CVAR_ARCHIVE )
CG_CVAR( cg_runroll,				"cg_runroll",				"0.005",							CVAR_ARCHIVE )
CG_CVAR( cg_bobup ,					"cg_bobup",					"0.005",							CVAR_ARCHIVE )
CG_CVAR( cg_bobpitch,				"cg_bobpitch",				"0.002",							CVAR_ARCHIVE )
CG_CVAR( cg_bobroll,				"cg_bobroll",				"0.002",							CVAR_ARCHIVE )
CG_CVAR( cg_footsteps,				"cg_footsteps",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_teamChatTime,			"cg_teamChatTime",			"3000",								CVAR_ARCHIVE )
CG_CVAR( cg_teamChatHeight,			"cg_teamChatHeight",		"0",								CVAR_ARCHIVE )
CG_CVAR( cg_forceModel,				"cg_forceModel",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_teamChatsOnly,			"cg_teamChatsOnly",			"0",								CVAR_ARCHIVE )
CG_CVAR( cg_teamChatSounds,			"cg_teamChatSounds",		"1",								CVAR_ARCHIVE )
CG_CVAR( cg_fallingBob,				"cg_fallingBob",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_weaponBob,				"cg_weaponBob",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_damageKick,				"cg_damageKick",			"1",								CVAR_ARCHIVE )
CG_CVAR( cg_smallFont,				"ui_smallFont",				"0.25",								CVAR_ARCHIVE )
CG_CVAR( cg_bigFont,				"ui_bigFont",				"0.4",								CVAR_ARCHIVE )
CG_CVAR( cg_hudFiles,				"cg_hudFiles",				"ui/hud/default/medium.menu",		CVAR_ARCHIVE )
CG_CVAR( cg_userHud,				"cg_userHud",				"0",								CVAR_ARCHIVE )
CG_CVAR( r_flares,					"r_flares",					"1",								CVAR_ARCHIVE )
CG_CVAR( con_notifytime_etf,		"con_notifytime_etf",		"5",								CVAR_ARCHIVE )
CG_CVAR( r_lodCurveError,			"r_lodCurveError",			"250",								CVAR_ARCHIVE )
CG_CVAR( r_gamma,					"r_gamma",					"1",								CVAR_ARCHIVE )
CG_CVAR( cg_shadows,				"cg_shadows",				"1",								CVAR_ARCHIVE )
CG_CVAR( cl_maxpackets,				"cl_maxpackets",			"60",								CVAR_ARCHIVE )
CG_CVAR( r_maxpolys,				"r_maxpolys",				"1800",								CVAR_ARCHIVE )
CG_CVAR( r_maxpolyverts,			"r_maxpolyverts",			"9000",								CVAR_ARCHIVE )
CG_CVAR( com_maxfps,				"com_maxfps",				"125",							CVAR_CHEAT)
CG_CVAR( cl_maxfps,					"cl_maxfps",				"-1",							CVAR_ARCHIVE )
CG_CVAR( cg_blood,					"com_blood",				"1",								CVAR_ARCHIVE )
CG_CVAR( cg_cameraOrbitDelay,		"cg_cameraOrbitDelay",		"50",								CVAR_ARCHIVE )
CG_CVAR( cg_scorePlum,				"cg_scorePlums",			"1",								CVAR_ARCHIVE )

CG_CVAR( cg_errorDecay,				"cg_errordecay",			"100",		0 )
CG_CVAR( cg_nopredict,				"cg_nopredict",				"0",		0 )
CG_CVAR( cg_showmiss,				"cg_showmiss",				"0",		0 )
CG_CVAR( cg_stats,					"cg_stats",					"0",		0 )
CG_CVAR( cg_sniperAutoZoom,			"cg_sniperAutoZoom",		"0",		0 )
CG_CVAR( cg_buildScript,			"com_buildScript",			"0",		0 )	// force loading of all possible data and error on failures
CG_CVAR( cg_timescaleFadeEnd,		"cg_timescaleFadeEnd",		"1",		0 )
CG_CVAR( cg_timescaleFadeSpeed,		"cg_timescaleFadeSpeed",	"0",		0 )
CG_CVAR( cg_timescale,				"timescale",				"1",		0 )
CG_CVAR( r_clear,					"r_clear",					"0",		0 )
CG_CVAR( cg_packetdelay,				"cg_packetdelay",				"0",	CVAR_USERINFO )
CG_CVAR( cl_packetdelay,				"cl_packetdelay",				"0",	CVAR_CHEAT )
CG_CVAR( cg_pmovesmooth,		"cg_pmovesmooth",		"1",								CVAR_ARCHIVE )


CG_CVAR( r_debugSort,				"r_debugSort",				"0",		CVAR_CHEAT )
CG_CVAR( r_debugSurface,			"r_debugSurface",			"0",		CVAR_CHEAT )
CG_CVAR( r_directedScale,			"r_directedScale",			"0",		CVAR_CHEAT )
CG_CVAR( r_drawworld,				"r_drawworld",				"1",		CVAR_CHEAT )
CG_CVAR( r_lockpvs,					"r_lockpvs",				"0",		CVAR_CHEAT )
CG_CVAR( r_nocull,					"r_nocull",					"0",		CVAR_CHEAT )
CG_CVAR( r_nocurves,				"r_nocurves",				"0",		CVAR_CHEAT )
CG_CVAR( r_noportals,				"r_noportals",				"0",		CVAR_CHEAT )
CG_CVAR( r_novis,					"r_novis",					"0",		CVAR_CHEAT )
CG_CVAR( r_lightmap,				"r_lightmap",				"0",		CVAR_CHEAT )
CG_CVAR( r_showNormals,				"r_showNormals",			"0",		CVAR_CHEAT )
CG_CVAR( r_showTris,				"r_showTris",				"0",		CVAR_CHEAT )
CG_CVAR( cg_debugPanel,				"cg_debugpanel",			"0",		CVAR_CHEAT )
CG_CVAR( cg_drawParticleCount,		"cg_drawParticleCount",		"0",		CVAR_CHEAT )
CG_CVAR( cg_cameraMode,				"com_cameraMode",			"0",		CVAR_CHEAT )
CG_CVAR( cg_cameraOrbit,			"cg_cameraOrbit",			"0",		CVAR_CHEAT )
CG_CVAR( cg_gun_x,					"cg_gunX",					"0",		CVAR_ARCHIVE )
CG_CVAR( cg_gun_y,					"cg_gunY",					"0",		CVAR_ARCHIVE )
CG_CVAR( cg_gun_z,					"cg_gunZ",					"0",		CVAR_ARCHIVE )
CG_CVAR( cg_centertime,				"cg_centertime",			"3",		CVAR_CHEAT )
CG_CVAR( cg_swingSpeed,				"cg_swingSpeed",			"0.3",		CVAR_CHEAT )
CG_CVAR( cg_animSpeed,				"cg_animspeed",				"1",		CVAR_CHEAT )
CG_CVAR( cg_debugAnim,				"cg_debuganim",				"0",		CVAR_CHEAT )
CG_CVAR( cg_debugPosition,			"cg_debugposition",			"0",		CVAR_CHEAT )
CG_CVAR( cg_debugEvents,			"cg_debugevents",			"0",		CVAR_CHEAT )
CG_CVAR( cg_noPlayerAnims,			"cg_noplayeranims",			"0",		CVAR_CHEAT )
CG_CVAR( cg_tracerChance,			"cg_tracerchance",			"0.4",		CVAR_CHEAT )
CG_CVAR( cg_thirdPersonRange,		"cg_thirdPersonRange",		"40",		CVAR_CHEAT )
CG_CVAR( cg_thirdPersonAngle,		"cg_thirdPersonAngle",		"0",		CVAR_CHEAT )
CG_CVAR( cg_thirdPerson,			"cg_thirdPerson",			"0",		CVAR_CHEAT )
CG_CVAR( cg_hideScope,				"cg_hidescope",				"0",		CVAR_CHEAT )			// Slothy

CG_CVAR( r_singleShader,			"r_singleShader",			"0",		CVAR_CHEAT | CVAR_LATCH )
CG_CVAR( r_fullBright,				"r_fullBright",				"0",		CVAR_CHEAT | CVAR_LATCH )

CG_CVAR( cg_autoReload,				"cg_autoReload",			"1",		CVAR_USERINFO )
CG_CVAR( rate,						"rate",						"25000",	CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( snaps,						"snaps",					"40",		CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_gender,					"cg_gender",				"0",		CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_initializing,			"init",						"1",		CVAR_USERINFO | CVAR_ROM )
CG_CVAR( cg_etfVersion,				"cg_etfversion",			"",			CVAR_USERINFO | CVAR_ROM )

CG_CVAR( g_spectatorMode,			"g_spectatorMode",			"0",		CVAR_SYSTEMINFO )

CG_CVAR( cg_paused,					"cl_paused",				"0",		CVAR_ROM )

	// Changing the default here to 0 to keep it in sync with ui code
CG_CVAR( r_loresskins,				"r_loresskins",				"0",		CVAR_ARCHIVE | CVAR_LATCH )
CG_CVAR( cg_classicinit,			"cg_classicInit",			"1",		CVAR_ARCHIVE | CVAR_LATCH )
CG_CVAR( com_hunkmegs,				"com_hunkmegs",				"128",		CVAR_ARCHIVE | CVAR_LATCH )

CG_CVAR( cg_drawSkyPortal,			"cg_drawSkyPortal",			"1",		CVAR_ARCHIVE )
	//Unlagged
CG_CVAR( cg_unlagged,				"cg_unlagged",				"1",		CVAR_ARCHIVE | CVAR_USERINFO )
CG_CVAR( cg_debugDelag,				"cg_debugDelag",			"0",		CVAR_USERINFO | CVAR_CHEAT )
CG_CVAR( cg_drawBBox,				"cg_drawBBox",				"0",		CVAR_CHEAT )
CG_CVAR( cg_cmdTimeNudge,			"cg_cmdTimeNudge",			"0",		CVAR_ARCHIVE | CVAR_USERINFO )
CG_CVAR( cg_projectileNudge,		"cg_projectileNudge",		"0",		CVAR_ARCHIVE )
CG_CVAR( cg_optimizePrediction,		"cg_optimizePrediction",	"1",		CVAR_ARCHIVE )
CG_CVAR( cl_timeNudge,				"cl_timeNudge",				"0",		CVAR_ARCHIVE | CVAR_USERINFO )
CG_CVAR( cg_plOut,					"cg_plOut",					"0",		CVAR_USERINFO | CVAR_CHEAT )
CG_CVAR( cg_predictItems,			"cg_predictItems",			"1",		CVAR_ARCHIVE )
CG_CVAR( cg_predictWeapons,			"cg_predictWeapons",		"0",		CVAR_ARCHIVE )

CG_CVAR( cg_drawDemoRecording,		"cg_drawDemoRecording",		"1",		CVAR_ARCHIVE )
CG_CVAR( cg_demoLineX,				"cg_demoLineX",				"5",		CVAR_ARCHIVE )
CG_CVAR( cg_demoLineY,				"cg_demoLineY",				"60",		CVAR_ARCHIVE )
CG_CVAR( cl_demorecording,			"cl_demorecording",			"0",		CVAR_ROM )
CG_CVAR( cl_demofilename,			"cl_demofilename",			"",			CVAR_ROM )
CG_CVAR( cl_demooffset,				"cl_demooffset",			"0",		CVAR_ROM )

CG_CVAR( cg_killFeed,				"cg_killFeed",				"0",		CVAR_ARCHIVE )			// Slothy
CG_CVAR( cg_killFeedX,				"cg_killFeedX",				"640",		CVAR_ARCHIVE )			// Slothy
CG_CVAR( cg_killFeedY,				"cg_killFeedY",				"125",		CVAR_ARCHIVE )			// Slothy


CG_CVAR( cl_waverecording,			"cl_waverecording",			"0",		CVAR_ROM )
CG_CVAR( cl_wavefilename,			"cl_wavefilename",			"",			CVAR_ROM )
CG_CVAR( cl_waveoffset,				"cl_waveoffset",			"0",		CVAR_ROM )

CG_CVAR( developer,					"developer",				"0",		CVAR_CHEAT )
CG_CVAR( r_subdivisions,			"r_subdivisions",			"4",		CVAR_ARCHIVE )

CG_CVAR( demo_avifpsF1,				"demo_avifpsF1",			"0",		CVAR_ARCHIVE )
CG_CVAR( demo_avifpsF2,				"demo_avifpsF2",			"10",		CVAR_ARCHIVE )
CG_CVAR( demo_avifpsF3,				"demo_avifpsF3",			"15",		CVAR_ARCHIVE )
CG_CVAR( demo_avifpsF4,				"demo_avifpsF4",			"20",		CVAR_ARCHIVE )
CG_CVAR( demo_avifpsF5,				"demo_avifpsF5",			"24",		CVAR_ARCHIVE )
CG_CVAR( demo_avifpsF6,				"demo_avifpsF6",			"30",		CVAR_ARCHIVE )
CG_CVAR( demo_drawTimeScale,		"demo_drawTimeScale",		"1",		CVAR_ARCHIVE )
CG_CVAR( demo_infoWindow,			"demo_infoWindow",			"1",		CVAR_ARCHIVE )
//CG_CVAR( demo_baseFOV,				"demo_baseFOV",				"120",		CVAR_ARCHIVE )
CG_CVAR( demo_scoresToggle,			"demo_scoresToggle",		"1",		CVAR_ARCHIVE )

CG_CVAR( cg_drawFollowText,			"cg_drawFollowText",		"1",		CVAR_ARCHIVE )

#ifdef BUILD_BOTS
CG_CVAR( cg_omnibotdrawing,			"cg_omnibotdrawing",		"0",		CVAR_ARCHIVE )
CG_CVAR( cg_omnibot_render_distance,"omnibot_render_distance",	"2048",		CVAR_ARCHIVE )
#endif

CG_CVAR( cg_rocketTrail,			"cg_rocketTrail",			"1",		CVAR_ARCHIVE )
CG_CVAR( cg_grenadeTrail,			"cg_grenadeTrail",			"1",		CVAR_ARCHIVE )
CG_CVAR( cg_pipeTrail,				"cg_pipeTrail",				"1",		CVAR_ARCHIVE )
CG_CVAR( cg_napalmTrail,			"cg_napalmTrail",			"1",		CVAR_ARCHIVE )
CG_CVAR( cg_dartTrail,				"cg_dartTrail",				"1",		CVAR_ARCHIVE )
CG_CVAR( cg_shotgunPuff,			"cg_shotgunPuff",			"1",		CVAR_ARCHIVE )

CG_CVAR( cl_anonymous,				"cl_anonymous",				"0",		CVAR_CHEAT )

CG_EXTCVAR( cg_cl_yawspeed,			"cl_yawspeed",				"140",		0, EXT_CVAR_ARCHIVE_ND )
CG_EXTCVAR( cg_cl_pitchspeed,		"cl_pitchspeed",			"140",		0, EXT_CVAR_ARCHIVE_ND )
CG_EXTCVAR( cg_cl_freelook,			"cl_freelook",				"1",		0, EXT_CVAR_ARCHIVE_ND )

CG_CVAR( cg_hitBeep,				"cg_hitBeep",				"1",		CVAR_ARCHIVE )
CG_CVAR( cg_killBeep,				"cg_killBeep",				"0",		CVAR_ARCHIVE )

#undef CG_NULLCVAR
#undef CG_NULLEXTCVAR
#undef CG_EXTCVAR
#undef CG_CVAR
