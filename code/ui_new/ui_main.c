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
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/

#include "ui_local.h"
#include "../game/bg_q3f_playerclass.h"

uiInfo_t uiInfo;
extern int menuCount;

static const char *MonthAbbrev[] = {
	"Jan","Feb","Mar",
		"Apr","May","Jun",
		"Jul","Aug","Sep",
		"Oct","Nov","Dec"
};

static const char *netSources[] = {
	"Local",
#ifdef API_Q3
	"Mplayer",
#endif
	"Internet",
	"Favorites"
};
static const int numNetSources = sizeof(netSources) / sizeof(const char*);

static const char *Q3FGameJoinTypes[] = {
	"All",
	GAME_NAME_CAP,   //keeg used to be "ETF"
};

static int const numQ3FGameJoinTypes = sizeof(Q3FGameJoinTypes) / sizeof(const char*);

static const char *sortKeys[] = {
	"Server Name",
	"Map Name",
	"Open Player Spots",
//	"Game Type",
	"Ping Time"
};
static const int numSortKeys = sizeof(sortKeys) / sizeof(const char*);

static char* netnames[] = {
	"???",
	"UDP",
	"IPX",
	NULL
};

static qhandle_t hudpreviews[ETF_HUDCOUNT][ETF_HUDVARCOUNT];

configData_t configDataTable_General[] = {	
	{"Sort Scoreboard",			"cg_scoreboardsortmode",		-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "By Team;0;By Name;1;By Score;2;By Ping;3"},
	{"Scoreboard Snapshot",		"cg_ScoreSnapshot",				-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Off;0;TGA;1;JPEG;2"},
	{"Sniper Dot Scale",		"cg_sniperDotScale",			 1,	 3, -1, CONFIG_TYPE_SLIDER,		10},
	{"Adjust Agent Speed",		"cg_adjustAgentSpeed",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Atmospheric Effects",		"cg_atmosphericEffects",		-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Panels",				"cg_drawPanel",					-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Use ALL Class Configs",	"cg_execClassConfigs",			-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Off;0;On First Spawn;1;On ALL Spawns;2"},
	{"Use Map Configs",			"cg_execMapConfigs",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Filter Obituaries",		"cg_filterObituaries",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Friendly Crosshair",		"cg_friendlyCrosshair",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Impact Vibrations",		"cg_impactVibration",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Low Effects",				"cg_lowEffects",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"No 3D Explosions",		"cg_no3DExplosions",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Old Style Popup Menus",	"cg_oldSkoolMenu",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Play Class Sound",		"cg_playClassSound",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Visual Grenade Timer 1",	"cg_showGrenadeTimer1",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Visual Grenade Timer 2",	"cg_showGrenadeTimer2",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Show Sentry Cam",			"cg_showSentryCam",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Sniper Dot Colors",		"cg_sniperDotColors",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Sniper Dot Prediction",	"cg_sniperHistoricalSight",		-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Visual Aids",		"cg_visualAids",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Flares",				"cg_flares",					-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Low Quality Sky",			"r_fastsky",					-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Portal Skies",		"cg_drawSkyPortal",				-1,	-1,	-1,	CONFIG_TYPE_YESNO},
	{"Sync Every Frame",		"g_synchronousClients",			-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Identify Targets",		"cg_drawCrosshairNames",		-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Team Overlay",		"cg_drawTeamOverlay",			-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "off;0;upper right;1;lower right;2;lower left;3"},
	{"Dynamic Lights",			"r_dynamiclight",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Ejecting Brass",			"cg_brassTime",					-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "High;2500;Med;1250;Off;0"},
	{"Simple Items",			"cg_simpleItems",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Auto Download",			"cl_allowDownload",				-1, -1, -1, CONFIG_TYPE_YESNO},
	{"Draw Team Markers",		"cg_drawFriend",				-1, -1, -1, CONFIG_TYPE_YESNO},
	//{"Draw Team Markers",		"cg_drawFriend",				-1, -1, -1, CONFIG_TYPE_TEXTLIST,	0,
	//"No;0;Yes;1;Team;team;White;white;Black;black;Red;red;Green;green;Blue;blue;Yellow;yellow;Magenta;magenta;Cyan;cyan;Gray;gray"},
	{"Grenade Timer",			"cg_grenadeprimesound",			-1, -1, -1, CONFIG_TYPE_TEXTLIST,	0, 
	"None;;Countdown;sound/grentimer/grentimer.wav;Countdown 2;sound/grentimer/grentimer_zzprime2.wav;Beep;"
	"sound/grentimer/grentimer1.wav;Beep 2;sound/grentimer/grentimer2.wav;Loud Beep;sound/grentimer/grentimer_deaf.wav;"
	"BH 1;sound/grentimer/grentimer_bh1.wav;BH 2;sound/grentimer/grentimer_bh2.wav;DON 1;sound/grentimer/grentimer_don1.wav;"
	"DON 2;sound/grentimer/grentimer_don2.wav;ELD 1;sound/grentimer/grentimer_eld1.wav;EQ 1;sound/grentimer/grentimer_eq1.wav;"
	"EQ 2;sound/grentimer/grentimer_eq2.wav;EQ 3;sound/grentimer/grentimer_eq3.wav;EQ 4;sound/grentimer/grentimer_eq4.wav;"
	"GBMH 1;sound/grentimer/grentimer_gmbh1.wav;GT 1;sound/grentimer/grentimer_gt1.wav;JM 1;sound/grentimer/grentimer_jm1.wav;"
	"JM 2;sound/grentimer/grentimer_jm2.wav;JM 3;sound/grentimer/grentimer_jm3.wav;JM 4;sound/grentimer/grentimer_jm4.wav;"
	"LOL 1;sound/grentimer/grentimer_lol1.wav;LOL 2;sound/grentimer/grentimer_lol2.wav;LOL 3;sound/grentimer/grentimer_lol3.wav;"
	"MOV;sound/grentimer/grentimer_mov1.wav;Swiss;sound/grentimer/grentimer_swiss.wav"},
};

int configDataTable_General_Size = sizeof(configDataTable_General)/sizeof(configData_t);

configData_t configDataTable_Move[] = {
	{"Move Forward",	"+forward",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Move Backward",	"+back",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Move Left",		"+moveleft",	-1, -1, -1, CONFIG_TYPE_BIND},
	{"Move Right",		"+moveright",	-1, -1, -1, CONFIG_TYPE_BIND},
	{"Jump",			"+moveup",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Crouch",			"+movedown",	-1, -1, -1, CONFIG_TYPE_BIND},
	{"Run/Walk",		"+speed",		-1, -1, -1, CONFIG_TYPE_BIND},
};

int configDataTable_Move_Size = sizeof(configDataTable_Move)/sizeof(configData_t);

configData_t configDataTable_Shoot[] = {
	{"Fire",					"+attack",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Weapon Slot 1",			"weaponslot 1",		-1,	-1,	-1,	CONFIG_TYPE_BIND},
	{"Weapon Slot 2",			"weaponslot 2",		-1,	-1,	-1,	CONFIG_TYPE_BIND},
	{"Weapon Slot 3",			"weaponslot 3",		-1,	-1,	-1,	CONFIG_TYPE_BIND},
	{"Weapon Slot 4",			"weaponslot 4",		-1,	-1,	-1,	CONFIG_TYPE_BIND},
	{"Reload Weapon",			"+reload",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Next Weapon",				"weapnext",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Previous Weapon",			"weapprev",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Prime Grenade 1",			"primeone",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Prime Grenade 2",			"primetwo",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Throw Grenade",			"throwgren",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Prime/Throw Gren1",		"toggleone",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Prime/Throw Gren2",		"toggletwo",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Hold/Throw Gren1",		"+gren1",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Hold/Throw Gren2",		"+gren2",			-1, -1, -1, CONFIG_TYPE_BIND},
};

int configDataTable_Shoot_Size = sizeof(configDataTable_Shoot)/sizeof(configData_t);

configData_t configDataTable_Misc[] = {
	{"Change Class",			"changeclass",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Change Team",				"changeteam",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Special",					"special",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Special2",				"special2",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Zoom",					"+newzoom",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Discard Ammo",			"discard",				-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Drop Ammo Menu",			"dropammo menu",		-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Inventory",				"inventory",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"HE Charge",				"charge menu",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Get Flaginfo",			"flaginfo",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Drop Flag",				"dropflag",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Scores (toggle)",			"+scores",				-1, -1, -1, CONFIG_TYPE_BIND},
	{"Scores (hold)",			"+alias score",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Screenshot (TGA)",		"screenshot_etf",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Screenshot (JPG)",		"screenshotJPEG_etf",	-1, -1, -1, CONFIG_TYPE_BIND},  //keeg
#ifdef _ETXREAL
	{"Screenshot (PNG)",		"screenshotPNG_etf",	-1, -1, -1, CONFIG_TYPE_BIND},  //Ensiform
#endif
	{"Record Demo",				"record_etf",			-1,	-1,	-1,	CONFIG_TYPE_BIND},
//	{"Use Item",				"+button2",				-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Scoreboard Down",			"hudscript scrolldown scoreboard scoreboard",	-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Scoreboard Up",			"hudscript scrollup scoreboard scoreboard",		-1, -1, -1, CONFIG_TYPE_BIND},
	{"Quick Menu",				"+quickmenu",			-1, -1, -1, CONFIG_TYPE_BIND},
};

int configDataTable_Misc_Size = sizeof(configDataTable_Misc)/sizeof(configData_t);

configData_t configDataTable_Coms[] = {
	{"Radio Comms",			"usermenu",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Call Engineer",		"armorme",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Call Medic",			"saveme",			-1, -1, -1, CONFIG_TYPE_BIND},
	{"Talk (global)",		"messagemode_etf",	-1, -1, -1, CONFIG_TYPE_BIND},
	{"Talk (team)",			"messagemode2_etf",	-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Talk (target)",		"messagemode3_etf",	-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Talk (attacker)",		"messagemode4_etf",	-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Taunt",				"+button3",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Wave No",				"+button5",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Wave Yes",			"+button6",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Look",				"+button7",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Becon",				"+button8",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Stop",				"+button9",			-1, -1, -1, CONFIG_TYPE_BIND},
//	{"Point",				"+button10",		-1, -1, -1, CONFIG_TYPE_BIND},
};

int configDataTable_Coms_Size = sizeof(configDataTable_Coms)/sizeof(configData_t);

configData_t systemDataTable_Graphics[] = {
	{"Driver",				"r_glDriver",					-1, -1, -1, CONFIG_TYPE_TEXTLIST,	0, ("Default;" OPENGL_DRIVER_NAME ";Voodoo;" _3DFX_DRIVER_NAME)},
	{"GL Extensions",		"r_allowExtensions",			-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "On;1;Off;0"},
	{"Video Mode",			"r_mode",						-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "320x240;0;400x300;1;512x384;2;640x480;3;800x600;4;960x720;5;1024x768;6;1152x864;7;1280x1024 (16:9);8;1600x1200;9;2048x1536;10;856x480 wide screen;11"},
	{"Color Depth",			"r_colorbits",					-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Default;0;16 bit;16;32 bit;32"},
	{"Z-Buffer Depth",		"r_depthbits",					-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Default;0;16 bit;16;24 bit;24"},
	{"Fullscreen",			"r_fullscreen",					-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "On;1;Off;0"},
	{"Lighting",			"r_vertexlight",				-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Lightmap;0;Vertex;1"},
	{"Curve Lod Bias",		"r_lodbias",					-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Low;1;High;0"},
	{"Curve Detail",		"r_subdivisions",				-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Low;20;Medium;12;High;4"},
	{"Texture Detail",		"r_picmip",						-3,  0, -1, CONFIG_TYPE_SLIDER,		-1},
	{"Texture Quality",		"r_texturebits",				-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Default;0;16 bit;16;32 bit;32"},
	{"Texture Filter",		"r_textureMode",				-1, -1, -1, CONFIG_TYPE_TEXTLIST,	0, "Bilinear;GL_LINEAR_MIPMAP_NEAREST;Trilinear;GL_LINEAR_MIPMAP_LINEAR"},
	{"Compressed Textures",	"r_ext_compressed_textures",	-1, -1, -2, CONFIG_TYPE_YESNO},
};

int systemDataTable_Graphics_Size = sizeof(systemDataTable_Graphics)/sizeof(configData_t);

configData_t systemDataTable_Display[] = {
	{"Brightness",			"r_gamma",		0.5,	2, -1, CONFIG_TYPE_SLIDER, 1},
	{"Screen Size",			"cg_viewsize",	 30,  100, -1, CONFIG_TYPE_SLIDER, 1},
};

int systemDataTable_Display_Size = sizeof(systemDataTable_Display)/sizeof(configData_t);

configData_t systemDataTable_Sound[] = {
	{"Effects Volume",		"s_volume",			 0,  1, -1, CONFIG_TYPE_SLIDER,		1},
	{"Music Volume",		"s_musicvolume",	 0,  1, -1, CONFIG_TYPE_SLIDER,		1},
	{"Quality",				"s_khz",			-1, -1, -1, CONFIG_TYPE_FLOATLIST,	0, "Low;11;High;22"},
};

int systemDataTable_Sound_Size = sizeof(systemDataTable_Sound)/sizeof(configData_t);

configData_t systemDataTable_Network[] = {
	{"Rate",				"rate",				-1, -1, -1, CONFIG_TYPE_FLOATLIST, 0, "<=28.8k;2500;33.6k;3000;56k;4000;ISDN;5000;LAN/CABLE/xDSl;25000"},
};

int systemDataTable_Network_Size = sizeof(systemDataTable_Network)/sizeof(configData_t);

static const char *gunNames[] = {
	"Assault Rifle",
	"Axe",
	"Dartgun",
	"Flamethrower",
	"Grenade Launcher",
	"Knife",
	"Minigun",
	"Nailgun",
	"Napalm Launcher",
	"Pipe Launcher",
	"Railgun",
	"Rocket Launcher",
	"Shotgun",
	"Sniper Rifle",
	"Super Nailgun",
	"Super Shotgun",
	"Syringe",
	"Wrench"
};
static const int numGunNames = sizeof(gunNames) / sizeof(const char*);
//static qhandle_t gunPreviews[18];

static const char *grenNames[] = {
	"Cluster Bomb",
	"Flash Grenade",
	"Gas Grenade",
	"Grenade",
	"Nail Grenade",
	"Napalm Grenade",
	"Pulse Grenade",
	"Stun Grenade"
};
static const int numGrenNames = sizeof(grenNames) / sizeof(const char*);
	
static const char *itemNames[] = {
	"Autosentry",
	"Backpack",
	"HE Charge",
	"Supply Station"
};
static const int numItemNames = sizeof(itemNames) / sizeof(const char*);


static int gamecodetoui[] = {4,2,3,0,5,1,6};
static int uitogamecode[] = {4,6,2,3,1,5,7};


static void UI_StartServerRefresh(qboolean full);
static void UI_StopServerRefresh( void );
static void UI_DoServerRefresh( void );
static void UI_FeederSelection(float feederID, int index);
static void UI_BuildServerDisplayList(qboolean force);
static void UI_BuildServerStatus(qboolean force);
static void UI_BuildFindPlayerList(qboolean force);
static void UI_BuildServerPlayerList(qboolean force);
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 );
//static void UI_ParseGameInfo(const char *teamFile);
//static void UI_ParseTeamInfo(const char *teamFile);
static const char *UI_SelectedMap(int index, int *actual);
//static int UI_GetIndexFromSelection(int actual);

//int ProcessNewUI( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
vmCvar_t  ui_new;
vmCvar_t  ui_debug;
vmCvar_t  ui_initialized;

void _UI_Init( qboolean );
void _UI_Shutdown( void );
void _UI_KeyEvent( int key, qboolean down );
void _UI_MouseEvent( int dx, int dy );
void _UI_Refresh( int realtime );
qboolean _UI_IsFullscreen( void );

Q_EXPORT intptr_t vmMain( int command, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4,
	intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9, intptr_t arg10, intptr_t arg11 ) {
  switch ( command ) {
	  case UI_GETAPIVERSION:
		  return UI_API_VERSION;

	  case UI_INIT:
		  _UI_Init((qboolean)arg0);
		  return 0;

	  case UI_SHUTDOWN:
		  _UI_Shutdown();
		  return 0;

	  case UI_KEY_EVENT:
		  _UI_KeyEvent( arg0, (qboolean)arg1 );
		  return 0;

	  case UI_MOUSE_EVENT:
		  _UI_MouseEvent( arg0, arg1 );
		  return 0;

	  case UI_REFRESH:
		  _UI_Refresh( arg0 );
		  return 0;

	  case UI_IS_FULLSCREEN:
		  return Menus_AnyFullScreenVisible();

	  case UI_SET_ACTIVE_MENU:
		if( arg0 == UIMENU_INGAME ) {
			char buffer[64];

			trap_GetConfigString( CS_INTERMISSION, buffer, sizeof(buffer));
			if(atoi( buffer )) {
				HUD_Setup_Menu("tab_scores");
				arg0 = UIMENU_ENDGAME;
			} else {
				trap_Cvar_VariableStringBuffer("hud_focustab", buffer, sizeof(buffer));
				if (!buffer[0]) {
					if (!UI_PlayerOnTeam()) {
						HUD_Setup_Menu("tab_chooseteam");
					} else if (!UI_PlayerHasClass()) {
						HUD_Setup_Menu("tab_chooseclass");
					} else {
						trap_GetConfigString(CS_VOTE_TIME, buffer, sizeof(buffer));
						if(atoi( buffer )) {
							HUD_Setup_Menu( "menu_dovote");
						}
					}
				}
			}
		}
		_UI_SetActiveMenu( (uiMenuCommand_t)arg0 );
		return 0;

	  case UI_CONSOLE_COMMAND:
		  return UI_ConsoleCommand(arg0);

	  case UI_DRAW_CONNECT_SCREEN:
		  UI_DrawConnectScreen( (qboolean)arg0 );
		  return 0;
	  case UI_HASUNIQUECDKEY:				// mod authors need to observe this
		  return qfalse;

	}

	return -1;
}



void AssetCache() {
	int n;
	uiClientState_t	cstate;

	uiInfo.uiDC.Assets.fxBasePic =				trap_R_RegisterShaderNoMip( ART_FX_BASE );
	uiInfo.uiDC.Assets.fxPic[0] =				trap_R_RegisterShaderNoMip( ART_FX_RED );
	uiInfo.uiDC.Assets.fxPic[1] =				trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	uiInfo.uiDC.Assets.fxPic[2] =				trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	uiInfo.uiDC.Assets.fxPic[3] =				trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	uiInfo.uiDC.Assets.fxPic[4] =				trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	uiInfo.uiDC.Assets.fxPic[5] =				trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	uiInfo.uiDC.Assets.fxPic[6] =				trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	uiInfo.uiDC.Assets.scrollBar =				trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	uiInfo.uiDC.Assets.scrollBar_hor =			trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_HOR );
	uiInfo.uiDC.Assets.scrollBarArrowDown =		trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	uiInfo.uiDC.Assets.scrollBarArrowUp =		trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	uiInfo.uiDC.Assets.scrollBarArrowLeft =		trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	uiInfo.uiDC.Assets.scrollBarArrowRight =	trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	uiInfo.uiDC.Assets.scrollBarThumb =			trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );

	uiInfo.uiDC.Assets.sliderBar =				trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	uiInfo.uiDC.Assets.sliderThumb =			trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
	uiInfo.uiDC.Assets.sliderEndLeft =			trap_R_RegisterShaderNoMip( ASSET_SLIDER_END_L );
	uiInfo.uiDC.Assets.sliderEndRight =			trap_R_RegisterShaderNoMip( ASSET_SLIDER_END_R );

	uiInfo.uiDC.Assets.LEDoff =					trap_R_RegisterShaderNoMip( ASSET_LED_OFF );
	uiInfo.uiDC.Assets.LEDon =					trap_R_RegisterShaderNoMip( ASSET_LED_ON );

	uiInfo.uiDC.Assets.checkboxCheck =			trap_R_RegisterShaderNoMip( ASSET_CHECKBOX_CHECK );
	uiInfo.uiDC.Assets.checkboxCheckNot =		trap_R_RegisterShaderNoMip( ASSET_CHECKBOX_CHECK_NOT );
	uiInfo.uiDC.Assets.checkboxCheckNo =		trap_R_RegisterShaderNoMip( ASSET_CHECKBOX_CHECK_NO );


	for( n = 0; n < NUM_CROSSHAIRS; n++ ) {
		uiInfo.uiDC.Assets.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );
		uiInfo.uiDC.Assets.crosshairAltShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c_alt", 'a' + n ) );
	}

	trap_GetClientState( &cstate );

	if( cstate.connState == CA_DISCONNECTED) {
//		uiInfo.uiDC.Assets.Q3F_BM_ContentModel =	trap_R_RegisterModel( "ui/models/content.md3" );
//		uiInfo.uiDC.Assets.Q3F_BM_InnerModel =		trap_R_RegisterModel( "ui/models/centre.md3" );
//		uiInfo.uiDC.Assets.Q3F_BM_OuterModel =		trap_R_RegisterModel( "ui/models/outer.md3" );
//		uiInfo.uiDC.Assets.Q3F_BM_MiddleModel =		trap_R_RegisterModel( "ui/models/middle.md3" );

// RR2DO2
		uiInfo.Q3F_BackModelOpenSound = trap_S_RegisterSound("ui/sound/menuopen.wav", qfalse);			// old: "ui/sound/backmodelopen.wav", qfalse);
		uiInfo.Q3F_BackModelReOpenSound = trap_S_RegisterSound("ui/sound/menureopen.wav", qfalse);		// "ui/sound/backmodelreopen.wav", qfalse);
		uiInfo.Q3F_BackModelCloseSound = trap_S_RegisterSound("ui/sound/menuclose.wav", qfalse);		// "ui/sound/backmodelclose.wav", qfalse);
	//	uiInfo.Q3F_BackModelStartupSound = trap_S_RegisterSound("ui/sound/backmodelstart.wav", qfalse);
		uiInfo.Q3F_TransitionStaticSound1 = trap_S_RegisterSound("ui/sound/transition1.wav", qfalse);
		uiInfo.Q3F_TransitionStaticSound2 = trap_S_RegisterSound("ui/sound/transition2.wav", qfalse);
		uiInfo.Q3F_TransitionStaticSound3 = trap_S_RegisterSound("ui/sound/transition3.wav", qfalse);
// RR2DO2
	}

	// slothy
	uiInfo.ModelImages[Q3F_CLASS_RECON] = trap_R_RegisterShaderNoMip( MODEL_RECON );
	uiInfo.ModelImages[Q3F_CLASS_SNIPER] = trap_R_RegisterShaderNoMip( MODEL_SNIPER );
	uiInfo.ModelImages[Q3F_CLASS_SOLDIER] = trap_R_RegisterShaderNoMip( MODEL_SOLDIER );
	uiInfo.ModelImages[Q3F_CLASS_GRENADIER] = trap_R_RegisterShaderNoMip( MODEL_GRENADIER );
	uiInfo.ModelImages[Q3F_CLASS_PARAMEDIC] = trap_R_RegisterShaderNoMip( MODEL_PARAMEDIC );
	uiInfo.ModelImages[Q3F_CLASS_MINIGUNNER] = trap_R_RegisterShaderNoMip( MODEL_MINIGUNNER );
	uiInfo.ModelImages[Q3F_CLASS_FLAMETROOPER] = trap_R_RegisterShaderNoMip( MODEL_FLAMETROOPER );
	uiInfo.ModelImages[Q3F_CLASS_AGENT] = trap_R_RegisterShaderNoMip( MODEL_AGENT );
	uiInfo.ModelImages[Q3F_CLASS_ENGINEER] = trap_R_RegisterShaderNoMip( MODEL_ENGINEER );
	uiInfo.ModelImages[Q3F_CLASS_CIVILIAN] = trap_R_RegisterShaderNoMip( MODEL_CIVILIAN );

	uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	
	uiInfo.uiDC.Assets.btnleft[0] = trap_R_RegisterShaderNoMip( ASSET_BUTTONLEFT );
	uiInfo.uiDC.Assets.btnmid[0] = trap_R_RegisterShaderNoMip( ASSET_BUTTONMID );
	uiInfo.uiDC.Assets.btnright[0] = trap_R_RegisterShaderNoMip( ASSET_BUTTONRIGHT );
	uiInfo.uiDC.Assets.btnleft[1] = trap_R_RegisterShaderNoMip( ASSET_BUTTONLEFTHI );
	uiInfo.uiDC.Assets.btnmid[1] = trap_R_RegisterShaderNoMip( ASSET_BUTTONMIDHI );
	uiInfo.uiDC.Assets.btnright[1] = trap_R_RegisterShaderNoMip( ASSET_BUTTONRIGHTHI );

	uiInfo.uiDC.Assets.lock = trap_R_RegisterShaderNoMip( "ui/gfx/lock" );
	uiInfo.uiDC.Assets.pureon = trap_R_RegisterShaderNoMip( "ui/gfx/pure_on" );
	uiInfo.uiDC.Assets.pureoff = trap_R_RegisterShaderNoMip( "ui/gfx/pure_off" );

	uiInfo.uiDC.Assets.xammo = trap_R_RegisterShaderNoMip( "ui/gfx/hud/x_ammo.tga" );		//ui\gfx\hud

	//slothy
}

void _UI_DrawSides(float x, float y, float w, float h, float size) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.xscale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void _UI_DrawTopBottom(float x, float y, float w, float h, float size) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.yscale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	trap_R_SetColor( color );

	_UI_DrawTopBottom(x, y, width, height, size);
	_UI_DrawSides(x, y, width, height, size);

	trap_R_SetColor( NULL );
}

int Text_Width(const char *text, float scale, int limit, fontStruct_t *parentfont) {
	int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= ui_smallFont.value) {
			font = &uiInfo.uiDC.Assets.font.smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &uiInfo.uiDC.Assets.font.bigFont;
		} else {
			font = &uiInfo.uiDC.Assets.font.textFont;
		}
	} else {
		if (scale <= ui_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;
	out = 0;
	if (text) {
		len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2)) || (*s == '\n')) {
				if ( Q_IsColorStringPtr(s) ) {
					s += 2;
					continue;
				} else if( *s == '\n' ) {
					s++;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					out += glyph->xSkip;
					s++;
					count++;
				}
			}
		}
	}
	return out * useScale;
}

int Text_Height(const char *text, float scale, int limit, fontStruct_t *parentfont) {
	int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= ui_smallFont.value) {
			font = &uiInfo.uiDC.Assets.font.smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &uiInfo.uiDC.Assets.font.bigFont;
		} else {
			font = &uiInfo.uiDC.Assets.font.textFont;
		}
	} else {
		if (scale <= ui_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	max = 0;
	if (text) {
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2)) || (*s == '\n')) {
			//if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				if ( Q_IsColorStringPtr(s) ) {
					s += 2;
					continue;
				} else if( *s == '\n' ) {
					s++;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					if (max < glyph->height) {
						max = glyph->height;
					}
					s++;
					count++;
				}
			}
		}
	}
	return max * useScale;
}

void Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader ) {
  float w, h;
  w = width * scale;
  h = height * scale;
  UI_AdjustFrom640( &x, &y, &w, &h );
  trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment) {
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;

	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= ui_smallFont.value) {
			font = &uiInfo.uiDC.Assets.font.smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &uiInfo.uiDC.Assets.font.bigFont;
		} else {
			font = &uiInfo.uiDC.Assets.font.textFont;
		}
	} else {
		if (scale <= ui_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	if (text) {
		const char *s = text;

		// RR2DO2: for alignment
		int	alignmentoffset;

		switch( textalignment )
		{
			case ITEM_ALIGN_LEFT: 
				alignmentoffset = 0; 
				break;
			case ITEM_ALIGN_CENTER: 
				alignmentoffset = -0.5f * Text_Width( text, scale, limit, parentfont ); 
				break;
			case ITEM_ALIGN_RIGHT: 
				alignmentoffset = -Text_Width( text, scale, limit, parentfont ); 
				break;
			default: 
				alignmentoffset = 0; 
				break;
		}
		// RR2DO2

		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);

		if (limit > 0 && len > limit) {
			len = limit;
		}

		count = 0;

		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				glyph = &font->glyphs[(unsigned char)*s];
				//int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
				//float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);

				if ( Q_IsColorStringPtr( s ) ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
					s += 2;
					continue;
				} else {
					float yadj = useScale * glyph->top;

					if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
						int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
						colorBlack[3] = newColor[3];
						trap_R_SetColor( colorBlack );
						Text_PaintChar(x + ofs + alignmentoffset, y - yadj + ofs, 
															glyph->imageWidth,
															glyph->imageHeight,
															useScale, 
															glyph->s,
															glyph->t,
															glyph->s2,
															glyph->t2,
															glyph->glyph );
						trap_R_SetColor( newColor );
						colorBlack[3] = 1.0;
					}
					Text_PaintChar(x + alignmentoffset, y - yadj, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph );

					x += (glyph->xSkip * useScale) + adjust;
				}
				s++;
				count++;
			}
		}
		trap_R_SetColor( NULL );
	}
}

void Text_Width_To_Max(char *text, float scale, int max, fontStruct_t *parentfont) {
	int count;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= ui_smallFont.value) {
			font = &uiInfo.uiDC.Assets.font.smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &uiInfo.uiDC.Assets.font.bigFont;
		} else {
			font = &uiInfo.uiDC.Assets.font.textFont;
		}
	} else {
		if (scale <= ui_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	out = 0;
	if (text) {
		count = 0;
		while (s && *s) {
			if ( out > max ) {
				*s = '\0';
				return;
			}
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				if ( Q_IsColorStringPtr(s) ) {
					s += 2;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					out += (glyph->xSkip * useScale);
					s++;
					count++;
				}
			}
		}
	}
}

void Text_Paint_MaxWidth(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment, int maxwidth) {
	static char buffer[1024];

	if(!maxwidth) {
		return;
	}

	Q_strncpyz(buffer, text, 1024);

	Text_Width_To_Max(buffer, scale, maxwidth, parentfont);
	Text_Paint(x, y, scale, color, buffer, adjust, limit, style, parentfont, textalignment);
}

void Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style, fontStruct_t *parentfont, int textalignment) {
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph, *glyph2;
	float yadj;
	float useScale;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= ui_smallFont.value) {
			font = &uiInfo.uiDC.Assets.font.smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &uiInfo.uiDC.Assets.font.bigFont;
		} else {
			font = &uiInfo.uiDC.Assets.font.textFont;
		}
	} else {
		if (scale <= ui_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > ui_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;
  if (text) {
		const char *s = text;
		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		glyph2 = &font->glyphs[(int)cursor];
		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				glyph = &font->glyphs[(unsigned char)*s];
		//int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
		//float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
				if ( Q_IsColorStringPtr( s ) ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
					s += 2;
					continue;
				} else {
					yadj = useScale * glyph->top;
					if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
						int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
						colorBlack[3] = newColor[3];
						trap_R_SetColor( colorBlack );
						Text_PaintChar(x + ofs, y - yadj + ofs, 
															glyph->imageWidth,
															glyph->imageHeight,
															useScale, 
															glyph->s,
															glyph->t,
															glyph->s2,
															glyph->t2,
															glyph->glyph );
						colorBlack[3] = 1.0;
						trap_R_SetColor( newColor );
					}
					Text_PaintChar(x, y - yadj, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph );

					// CG_DrawPic(x, y - yadj, scale * uiDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * uiDC.Assets.textFont.glyphs[text[i]].imageHeight, uiDC.Assets.textFont.glyphs[text[i]].glyph);
			yadj = useScale * glyph2->top;
				if (count == cursorPos && !((uiInfo.uiDC.realTime/BLINK_DIVISOR) & 1)) {
						Text_PaintChar(x, y - yadj, 
															glyph2->imageWidth,
															glyph2->imageHeight,
															useScale, 
															glyph2->s,
															glyph2->t,
															glyph2->s2,
															glyph2->t2,
															glyph2->glyph );
					}

					x += (glyph->xSkip * useScale);
				}
				s++;
				count++;
			}
    }
    // need to paint cursor at end of text
    if (cursorPos == len && !((uiInfo.uiDC.realTime/BLINK_DIVISOR) & 1)) {
        yadj = useScale * glyph2->top;
        Text_PaintChar(x, y - yadj, 
                          glyph2->imageWidth,
                          glyph2->imageHeight,
                          useScale, 
                          glyph2->s,
                          glyph2->t,
                          glyph2->s2,
                          glyph2->t2,
                          glyph2->glyph );

    }


	  trap_R_SetColor( NULL );
  }
}


static void Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit, fontStruct_t *parentfont) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;

	if (text) {
		const char *s = text;
		float max = *maxX;
		float useScale;
		fontInfo_t *font;

		if( !(parentfont) || !(parentfont->fontRegistered) ) {
			if (scale <= ui_smallFont.value) {
				font = &uiInfo.uiDC.Assets.font.smallFont;
			} else if (scale > ui_bigFont.value) {
				font = &uiInfo.uiDC.Assets.font.bigFont;
			} else {
				font = &uiInfo.uiDC.Assets.font.textFont;
			}
		} else {
			if (scale <= ui_smallFont.value) {
				font = &parentfont->smallFont;
			} else if (scale > ui_bigFont.value) {
				font = &parentfont->bigFont;
			} else {
				font = &parentfont->textFont;
			}
		}

		useScale = scale * font->glyphScale;
		trap_R_SetColor( color );
		len = strlen(text);					 
		
		if (limit > 0 && len > limit) {
			len = limit;
		}
		
		count = 0;
		
		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				glyph = &font->glyphs[(unsigned char)*s];
				if ( Q_IsColorStringPtr( s ) ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
					s += 2;
					continue;
				} else {
					float yadj = useScale * glyph->top;
					
					if (Text_Width(s, scale, 1, NULL) + x > max ) {
						*maxX = 0;
						break;
					}
			    
					Text_PaintChar(x, y - yadj, 
								glyph->imageWidth,
								glyph->imageHeight,
								useScale, 
									glyph->s,
										glyph->t,
										glyph->s2,
											glyph->t2,
											glyph->glyph );
					x += (glyph->xSkip * useScale) + adjust;
					*maxX = x;
				}
				count++;
				s++;
			}
		}
		trap_R_SetColor( NULL );
	}
}


/*void UI_ShowPostGame(qboolean newHigh) {
	trap_Cvar_Set ("cg_cameraOrbit", "0");
	trap_Cvar_Set("cg_thirdPerson", "0");
	trap_Cvar_Set( "sv_killserver", "1" );
	uiInfo.soundHighScore = newHigh;
  _UI_SetActiveMenu(UIMENU_POSTGAME);
}*/

void UI_ShowEndGame() {
	_UI_SetActiveMenu(UIMENU_ENDGAME);
}

void UI_ShowInGame() {
	_UI_SetActiveMenu(UIMENU_INGAME);  
}

/*
=================
_UI_Refresh
=================
*/

void UI_DrawCenteredPic(qhandle_t image, int w, int h) {
  int x, y;
  x = (SCREEN_WIDTH - w) / 2;
  y = (SCREEN_HEIGHT - h) / 2;
  UI_DrawHandlePic(x, y, w, h, image);
}

int frameCount = 0;
int startTime;

#define	UI_FPS_FRAMES	4
void _UI_Refresh( int realtime )
{
	static int index;
	static int	previousTimes[UI_FPS_FRAMES];
	static int nextframe = qtrue;
	char buffer[64];
	int intermissiontime;

	uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realtime;

	previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
	index++;
	if ( index > UI_FPS_FRAMES ) {
		int i, total;
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < UI_FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
	}



	UI_UpdateCvars();

	if (menuCount > 0) {
		// paint all the menus
		Menu_PaintAll();
		// refresh server browser list
		UI_DoServerRefresh();
		// refresh server status
		UI_BuildServerStatus(qfalse);
		// refresh find player list
		UI_BuildFindPlayerList(qfalse);
		// refresh server browser player list
		UI_BuildServerPlayerList(qfalse);
	} 
	
	trap_GetConfigString(CS_INTERMISSION, buffer, 64);
	intermissiontime = atoi( buffer );
	if( intermissiontime && !uiInfo.ScoreSnapshotTaken && uiInfo.ScoreFetched && cg_ScoreSnapshot.integer ) {//uiInfo.uiDC.realTime - intermissiontime > 1000 && uiInfo.Q3F_scoreNum) {
		if(nextframe)
			nextframe = qfalse;
		else
		{
			uiInfo.ScoreFetched = qfalse;
			nextframe = qtrue;
			if ( cg_ScoreSnapshot.integer == 1  ) {
				uiInfo.uiDC.executeText(EXEC_APPEND, "screenshot_etf\n");
				uiInfo.ScoreSnapshotTaken = qtrue;
			} else if ( cg_ScoreSnapshot.integer == 2  ) {
				uiInfo.uiDC.executeText(EXEC_APPEND, "screenshotJPEG_etf\n" );
				uiInfo.ScoreSnapshotTaken = qtrue;
			}
		}
	}

	// draw cursor
	UI_SetColor( NULL );
	if ( menuCount > 0 ) {
		uiClientState_t	cstate;
		trap_GetClientState( &cstate );
		if(cstate.connState <= CA_DISCONNECTED || cstate.connState >= CA_ACTIVE) {
			UI_DrawHandlePic( uiInfo.uiDC.cursorx-16, uiInfo.uiDC.cursory-6, 32, 32, uiInfo.uiDC.Assets.cursor);	// RR2DO2: Q3F Cursor art
		}
	}
}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown( void ) {
	trap_LAN_SaveCachedServers();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return defaultMenu;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return defaultMenu;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
  return buf;

}

qboolean Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(pc_token_t));

		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize)) {
				return qfalse;
			}
			//trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.font.textFont);
			UI_Q3F_LoadFontFile(tempStr, pointSize, &uiInfo.uiDC.Assets.font.textFont);
			uiInfo.uiDC.Assets.font.fontRegistered = qtrue;
			continue;
		}

		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize)) {
				return qfalse;
			}
			//trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.font.smallFont);
			UI_Q3F_LoadFontFile(tempStr, pointSize, &uiInfo.uiDC.Assets.font.smallFont);
			continue;
		}

		if (Q_stricmp(token.string, "bigFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize)) {
				return qfalse;
			}
			//trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.font.bigFont);
			UI_Q3F_LoadFontFile(tempStr, pointSize, &uiInfo.uiDC.Assets.font.bigFont);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &uiInfo.uiDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &uiInfo.uiDC.Assets.shadowColor)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
			continue;
		}

		// Slothy -- gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

	}
	return qfalse;
}

void Font_Report() {
	int i;
	Com_Printf("Font Info\n");
	Com_Printf("=========\n");
	for ( i = 32; i < 96; i++) {
		Com_Printf("Glyph handle %i: %i\n", i, uiInfo.uiDC.Assets.font.textFont.glyphs[i].glyph);
	}
}

void UI_Report() {
	String_Report();
}

void UI_ParseMenu(const char *menuFile) {
	int handle;
	pc_token_t token;

	Com_Printf("Parsing menu file:%s\n", menuFile);

	handle = trap_PC_LoadSource(menuFile);
	if (!handle) {
		return;
	}

	while ( 1 ) {
		memset(&token, 0, sizeof(pc_token_t));
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (Asset_Parse(handle)) {
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

qboolean Load_Menu(int handle) {
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
    
		if ( token.string[0] == 0 ) {
			return qfalse;
		}

		if ( token.string[0] == '}' ) {
			return qtrue;
		}

		UI_ParseMenu(token.string); 
	}
	return qfalse;
}

static qboolean engine_is_ETE = qfalse;
const char *compiledate = __DATE__;
int compileday, compilemonth, compileyear;

void UI_LoadMenus(const char *menuFile, qboolean reset) {
	pc_token_t token;
	int handle;
	int start;

	start = trap_Milliseconds();

	if ( engine_is_ETE ) {
		trap_PC_AddGlobalDefine("ETE");
		trap_PC_AddGlobalDefine("NO_PUNKBUSTER");
	}

	trap_PC_AddGlobalDefine(va("BUILD_YEAR \"%d\"", compileyear));
	trap_PC_AddGlobalDefine(va("BUILD_MONTH \"%d\"", compilemonth));
	trap_PC_AddGlobalDefine(va("BUILD_DAY \"%d\"", compileday));

	handle = trap_PC_LoadSource( menuFile );
	if (!handle) {
		Com_Printf( S_COLOR_YELLOW "menu file not found: %s, using default^7\n", menuFile );
		handle = trap_PC_LoadSource( "ui/menus.txt" );
		if (!handle) {
			trap_Error( S_COLOR_RED "default menu file not found: ui/menus.txt, unable to continue!^7" );
		}
	}

	ui_new.integer = 1;

	if (reset) {
		Menu_Reset();
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			break;
		if( token.string[0] == 0 || token.string[0] == '}') {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "loadmenu") == 0) {
			if (Load_Menu(handle)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

	trap_PC_FreeSource( handle );
}

void UI_Load(void) {
	char lastName[1024];
	menuDef_t *menu = Menu_GetFocused();
	char *menuSet;

	String_Init();

	menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menu && menu->window.name) {
		strcpy(lastName, menu->window.name);
	} else lastName[0] = 0;

	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}

	UI_LoadMenus(menuSet, qtrue);

	menuSet = UI_Cvar_VariableString("ui_ingameMenuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/ingame.txt";
	}

	UI_LoadMenus(menuSet, qtrue);

	Menus_CloseAll();
	Menus_ActivateByName(lastName);
}

static const char *handicapValues[] = {"None","95","90","85","80","75","70","65","60","55","50","45","40","35","30","25","20","15","10","5",NULL};
//static int numHandicaps = sizeof(handicapValues) / sizeof(const char*);

static void UI_DrawHandicap(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
  int i, h;

  h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
  i = 20 - h / 5;

  Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, handicapValues[i], 0, 0, textStyle, font, textalignment);
}

/*static void UI_DrawClanName(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
  Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, UI_Cvar_VariableString("ui_teamName"), 0, 0, textStyle, font, textalignment);
}*/

/*static void UI_DrawGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, Q3FGameTypes[ui_gameType.integer], 0, 0, textStyle, font, textalignment);
}*/

/*static void UI_DrawNetGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if (ui_netGameType.integer < 0 || ui_netGameType.integer >= numQ3FGameTypes) {
		trap_Cvar_Set("ui_netGameType", "0");
	}
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, Q3FGameTypes[ui_netGameType.integer] , 0, 0, textStyle, font, textalignment);
}*/

/*static void UI_DrawJoinGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if (ui_joinGameType.integer < 0 || ui_joinGameType.integer >= numQ3FGameJoinTypes) {
		trap_Cvar_Set("ui_joinGameType", "0");
	}
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, Q3FGameJoinTypes[ui_joinGameType.integer] , 0, 0, textStyle, font, textalignment);
}*/

static void UI_DrawPreviewCinematic(rectDef_t *rect, float scale, vec4_t color) {
	if (uiInfo.previewMovie > -2) {
		uiInfo.previewMovie = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.movieList[uiInfo.movieIndex]), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		if (uiInfo.previewMovie >= 0) {
		  trap_CIN_RunCinematic(uiInfo.previewMovie);
			trap_CIN_SetExtents(uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h);
 			trap_CIN_DrawCinematic(uiInfo.previewMovie);
		} else {
			uiInfo.previewMovie = -2;
		}
	} 

}

static void UI_DrawEffects(rectDef_t *rect, float scale, vec4_t color) {
	UI_DrawHandlePic( rect->x, rect->y - 14, 128, 8, uiInfo.uiDC.Assets.fxBasePic );
	UI_DrawHandlePic( rect->x + uiInfo.effectsColor * 16 + 8, rect->y - 16, 16, 12, uiInfo.uiDC.Assets.fxPic[uiInfo.effectsColor] );
}

static void UI_DrawMapPreview(rectDef_t *rect, float scale, vec4_t color, qboolean net) {
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if (map < 0 || map > uiInfo.mapCount) {
		if (net) {
			ui_currentNetMap.integer = 0;
			trap_Cvar_Set("ui_currentNetMap", "0");
		} else {
			ui_currentMap.integer = 0;
			trap_Cvar_Set("ui_currentMap", "0");
		}
		map = 0;
	}

	if (uiInfo.mapList[map].levelShot == -1) {
		uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[map].imageName);
	}

	if (uiInfo.mapList[map].levelShot > 0) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot);
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
	}
}						 

static void UI_DrawMapCinematic(rectDef_t *rect, float scale, vec4_t color, qboolean net) {

	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer; 
	if (map < 0 || map > uiInfo.mapCount) {
		if (net) {
			ui_currentNetMap.integer = 0;
			trap_Cvar_Set("ui_currentNetMap", "0");
		} else {
			ui_currentMap.integer = 0;
			trap_Cvar_Set("ui_currentMap", "0");
		}
		map = 0;
	}

	if (uiInfo.mapList[map].cinematic >= -1) {
		if (uiInfo.mapList[map].cinematic == -1) {
			uiInfo.mapList[map].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[map].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}
		if (uiInfo.mapList[map].cinematic >= 0) {
		  trap_CIN_RunCinematic(uiInfo.mapList[map].cinematic);
		  trap_CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h);
 			trap_CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
		} else {
			uiInfo.mapList[map].cinematic = -2;
		}
	} else {
		UI_DrawMapPreview(rect, scale, color, net);
	}
}

static void UI_DrawNetSource(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if ( ui_netSource.integer < 0 || ui_netSource.integer >= numNetSources ) {
		trap_Cvar_Set( "ui_netSource", "1" );
		ui_netSource.integer = 1;
	}
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, va("Source: %s", netSources[ui_netSource.integer]), 0, 0, textStyle, font, textalignment);
}

static void UI_DrawNetMapPreview(rectDef_t *rect, float scale, vec4_t color) {

	if (uiInfo.serverStatus.currentServerPreview > 0) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview);
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
	}
}

static void UI_DrawNetMapCinematic(rectDef_t *rect, float scale, vec4_t color) {
	if (ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount) {
		ui_currentNetMap.integer = 0;
		trap_Cvar_Set("ui_currentNetMap", "0");
	}

	if (uiInfo.serverStatus.currentServerCinematic >= 0) {
		trap_CIN_RunCinematic(uiInfo.serverStatus.currentServerCinematic);
		trap_CIN_SetExtents(uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h);
		trap_CIN_DrawCinematic(uiInfo.serverStatus.currentServerCinematic);
	} else {
		UI_DrawNetMapPreview(rect, scale, color);
	}
}

/*static const char *UI_EnglishMapName(const char *map) {
	int i;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (Q_stricmp(map, uiInfo.mapList[i].mapLoadName) == 0) {
			return uiInfo.mapList[i].mapName;
		}
	}
	return "";
}*/

static void UI_DrawAllMapsSelection(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, qboolean net) {
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;

	if (map >= 0 && map < uiInfo.mapCount) {
		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, uiInfo.mapList[map].mapName, 0, 0, textStyle, font, textalignment);
	}
}

/*static void UI_DrawOpponentName(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, UI_Cvar_VariableString("ui_opponentName"), 0, 0, textStyle, font, textalignment);
}*/

static qboolean UI_OwnerDrawSize( int ownerDraw, rectDef_t* in, rectDef_t* out, itemDef_t* item, float* alpha) {
	switch(ownerDraw) {
	default:
		return qfalse;
	}
}

static int UI_OwnerDrawWidth(int ownerDraw, float scale, fontStruct_t *parentfont) {
	int i, h;
	const char *s = NULL;

	switch (ownerDraw) {
    case UI_HANDICAP:
		h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
		i = 20 - h / 5;
		s = handicapValues[i];
		break;
/*    case UI_GAMETYPE:
		s = Q3FGameTypes[ui_gameType.integer];
		break;*/
	case UI_NETSOURCE:
		if (ui_netSource.integer < 0 || ui_netSource.integer >= numNetSources) {
			trap_Cvar_Set( "ui_netSource", "1" );
			ui_netSource.integer = 1;
		}
		s = va("Source: %s", netSources[ui_netSource.integer]);
		break;
	case UI_ALLMAPS_SELECTION:
		break;
	case UI_KEYBINDSTATUS:
		if (Display_KeyBindPending()) {
			s = "Waiting for new key... Press ESCAPE to cancel";
		} else {
			s = "Press ENTER or CLICK to change, Press BACKSPACE to clear";
		}
		break;
	case UI_SERVERREFRESHDATE:
		s = UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer));
		break;
	default:
		break;
	}

	if (s) {
		return Text_Width(s, scale, 0, parentfont);
	}
	return 0;
}

void checkfname( const char * in, char *out) {
	int i;
	int num = strlen(in);

	for(i = 0; i < num; i++) {
		if(*in == ' ')
			*out = '_';
		else
			*out = tolower(*in);
		in++;
		out++;
	}
	*out = 0;
}

static void UI_DrawWeaponPreview(rectDef_t *rect, float scale, vec4_t color, qboolean net) {
	char filename[256];
	char cleanfile[256];

	if((uiInfo.uiDC.weapPreview == -1) && (uiInfo.uiDC.curWeapInt != -1)) {	// try to load shader for this image
		checkfname(uiInfo.uiDC.curWeapSource[uiInfo.uiDC.curWeapInt], cleanfile);
		Com_sprintf(filename, 256, "%s/%s", WEAPONSHOTDIR, cleanfile);

		uiInfo.uiDC.weapPreview = trap_R_RegisterShaderNoMip(filename);
	}

	if(uiInfo.uiDC.weapPreview > 0) {
		trap_R_SetColor(NULL);
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.uiDC.weapPreview);
	}
}

static void UI_DrawHudPreview(rectDef_t *rect, float scale, vec4_t color) {
	char filename[256];

	if(uiInfo.uiDC.hudPreviewCin == -1) {	// try to load shader for this image

		// try to load from cache
		uiInfo.uiDC.hudPreviewCin = hudpreviews[uiInfo.uiDC.curHudInt][uiInfo.uiDC.curHudVarInt];

		if(uiInfo.uiDC.hudPreviewCin == 0) {
			Com_sprintf(filename, 256, "%s/%s/%s.tga", HUDINFODIR, uiInfo.uiDC.curHud, uiInfo.uiDC.curHudVariant);

			uiInfo.uiDC.hudPreviewCin = trap_R_RegisterShaderNoMip(filename);
			if(!uiInfo.uiDC.hudPreviewCin) {
				Com_sprintf(filename, 256, "%s/hudprev.tga", HUDINFODIR);
				uiInfo.uiDC.hudPreviewCin = trap_R_RegisterShaderNoMip(filename);
				if(!uiInfo.uiDC.hudPreviewCin) {
					uiInfo.uiDC.hudPreviewCin = 0;
				}
			}

			hudpreviews[uiInfo.uiDC.curHudInt][uiInfo.uiDC.curHudVarInt] = uiInfo.uiDC.hudPreviewCin;
		}
	}

	
	if(uiInfo.uiDC.hudPreviewCin > 0) {
		trap_R_SetColor(NULL);
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.uiDC.hudPreviewCin);
	}
}

static void UI_DrawCrosshair(rectDef_t *rect, float scale, vec4_t color) {
/*
	if (uiInfo.currentCrosshair < 0 || uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
		uiInfo.currentCrosshair = 0;
	}

   //Keeg come back here and fix this later for size...after Slothy puts in new xhair section
   trap_R_SetColor( uiInfo.xhairColor );
   UI_DrawHandlePic( rect->x, rect->y - (rect->h/1.5f), rect->h, rect->h, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]);

   trap_R_SetColor(uiInfo.xhairColorAlt);
	UI_DrawHandlePic( rect->x, rect->y - (rect->h/1.5f), rect->h, rect->h, uiInfo.uiDC.Assets.crosshairAltShader[uiInfo.currentCrosshair]);

   trap_R_SetColor( NULL );

*/
	float size = cg_crosshairSize.integer;

	if(uiInfo.currentCrosshair < 0 || uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
		uiInfo.currentCrosshair = 0;
	}

	size = (rect->w / 96.0f) * ((size > 96.0f) ? 96.0f : ((size < 24.0f) ? 24.0f : size));

	trap_R_SetColor(uiInfo.xhairColor);
	UI_DrawHandlePic( rect->x + (rect->w - size)/2, rect->y + (rect->h - size)/2, size, size, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]);

	if(uiInfo.uiDC.Assets.crosshairAltShader[uiInfo.currentCrosshair]) {
		trap_R_SetColor(uiInfo.xhairColorAlt);
		UI_DrawHandlePic( rect->x + (rect->w - size)/2, rect->y + (rect->h - size)/2, size, size, uiInfo.uiDC.Assets.crosshairAltShader[uiInfo.currentCrosshair]);
	}

	trap_R_SetColor( NULL );
}

//slothy
static void UI_DrawPBStatus(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font)
{
	char outtext[10];
	float col[4];

	int pbstat = trap_Cvar_VariableValue("cl_punkbuster");

	switch(pbstat)
	{
	case 0: 
		Q_strncpyz(outtext, "Disabled", 10);
		col[0] = 1; col[1] = 0; col[2] = 0; col[3] = 1;
		break;
	case 1:
		Q_strncpyz(outtext, "Enabled", 10);
		col[0] = 0; col[1] = 1; col[2] = 0; col[3] = 1;
		break;
	default:
		Q_strncpyz(outtext, "Unknown", 10);
		col[0] = 1; col[1] = 0; col[2] = 0; col[3] = 1;
		break;
	}

	uiInfo.uiDC.drawText(rect->x + text_x, rect->y + text_y, scale, col, outtext, 0, 0, textStyle, font, textalignment);

	//DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, buff, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
}
/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList() {
	uiClientState_t	cs;
	int		n, count, team, team2, playerTeamNumber;
	char	info[MAX_INFO_STRING];

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	uiInfo.playerNumber = cs.clientNum;
	team = atoi(Info_ValueForKey(info, "t"));
	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	uiInfo.playerCount = 0;
	uiInfo.myTeamCount = 0;
	playerTeamNumber = 0;
	for( n = 0; n < count; n++ ) {
		trap_GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );

		if (info[0]) {
			Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
			Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] );

			uiInfo.playerCount++;
			team2 = atoi(Info_ValueForKey(info, "t"));
			if (team2 == team) {
				Q_strncpyz( uiInfo.teamNames[uiInfo.myTeamCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
				Q_CleanStr( uiInfo.teamNames[uiInfo.myTeamCount] );
				uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
				if (uiInfo.playerNumber == n) {
					playerTeamNumber = uiInfo.myTeamCount;
				}
				uiInfo.myTeamCount++;
			}
		}
	}

	trap_Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));

	n = trap_Cvar_VariableValue("cg_selectedPlayer");
	if (n < 0 || n > uiInfo.myTeamCount) {
		n = 0;
	}
	if (n < uiInfo.myTeamCount) {
		trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
	}
}

qboolean UI_PlayerOnTeam() {
	uiClientState_t	cs;
	int		team;
	char	info[MAX_INFO_STRING];

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	team = atoi(Info_ValueForKey(info, "t"));
	if(team >= Q3F_TEAM_RED && team <= Q3F_TEAM_GREEN) {
		return qtrue;
	}
	return qfalse;
}

qboolean UI_PlayerHasClass() {
	uiClientState_t	cs;
	int		team;
	char	info[MAX_INFO_STRING];

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	team = atoi(Info_ValueForKey(info, "cls"));
	if(team >= Q3F_CLASS_RECON && team <= Q3F_CLASS_CIVILIAN) {
		return qtrue;
	}
	return qfalse;
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if (uiInfo.serverStatus.refreshActive) {
		vec4_t lowLight, newColor;

		lowLight[0] = 0.8 * color[0]; 
		lowLight[1] = 0.8 * color[1]; 
		lowLight[2] = 0.8 * color[2]; 
		lowLight[3] = 0.8 * color[3]; 
		LerpColor(color,lowLight,newColor,0.5+0.5*sin(uiInfo.uiDC.realTime / PULSE_DIVISOR));
		Text_Paint(rect->x + text_x, rect->y + text_y, scale, newColor, va("Getting info for %d servers (ESC to cancel)", trap_LAN_GetServerCount(ui_netSource.integer)), 0, 0, textStyle, font, textalignment);
	} else {
		char buff[64];

		Q_strncpyz(buff, UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer)), 64);
		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, va("Refresh Time: %s", buff), 0, 0, textStyle, font, textalignment);
	}
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, vec4_t color) {
	if (uiInfo.serverStatus.motdLen) {
		float maxX;
	 
		if (uiInfo.serverStatus.motdWidth == -1) {
			uiInfo.serverStatus.motdWidth = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen) {
			uiInfo.serverStatus.motdOffset = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.uiDC.realTime > uiInfo.serverStatus.motdTime) {
			uiInfo.serverStatus.motdTime = uiInfo.uiDC.realTime + 10;
			if (uiInfo.serverStatus.motdPaintX <= rect->x + 2) {
				if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen) {
					uiInfo.serverStatus.motdPaintX += Text_Width(&uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], scale, 1, NULL) - 1;
					uiInfo.serverStatus.motdOffset++;
				} else {
					uiInfo.serverStatus.motdOffset = 0;
					if (uiInfo.serverStatus.motdPaintX2 >= 0) {
						uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
					} else {
						uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
					}
					uiInfo.serverStatus.motdPaintX2 = -1;
				}
			} else {
				//serverStatus.motdPaintX--;
				uiInfo.serverStatus.motdPaintX -= 2;
				if (uiInfo.serverStatus.motdPaintX2 >= 0) {
					//serverStatus.motdPaintX2--;
					uiInfo.serverStatus.motdPaintX2 -= 2;
				}
			}
		}

		maxX = rect->x + rect->w - 2;
		Text_Paint_Limit(&maxX, uiInfo.serverStatus.motdPaintX, rect->y + rect->h - 3, scale, color, &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0, NULL); 
		if (uiInfo.serverStatus.motdPaintX2 >= 0) {
			float maxX2 = rect->x + rect->w - 2;
			Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y + rect->h - 3, scale, color, uiInfo.serverStatus.motd, 0, uiInfo.serverStatus.motdOffset, NULL); 
		}
		if (uiInfo.serverStatus.motdOffset && maxX > 0) {
			// if we have an offset ( we are skipping the first part of the string ) and we fit the string
			if (uiInfo.serverStatus.motdPaintX2 == -1) {
						uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
			}
		} else {
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

	}
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if (Display_KeyBindPending()) {
		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Waiting for new key... Press ESCAPE to cancel", 0, 0, textStyle, font, textalignment);
	}
	else {
		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Double-click to set a key, BACKSPACE to clear", 0, 0, textStyle, font, textalignment);
	}
}

// djbob
static void UI_Q3F_DrawGameIndexMapName(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	int i;
	int number = trap_Cvar_VariableValue("hud_admingameindex");		// g_gameindex

	// Ensiform: Fixes this so that it isn't always spamming gameindex change with vote menu open
	if( (int)trap_Cvar_VariableValue("g_gameindex") != number )
		trap_Cvar_SetValue("g_gameindex", number);

	for( i = 0; i < uiInfo.mapList[ui_currentNetMap.integer].numGameIndicies; i++) {
		if(uiInfo.mapList[ui_currentNetMap.integer].gameIndiciesInfo[i].number == number) {
			Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, uiInfo.mapList[ui_currentNetMap.integer].gameIndiciesInfo[i].name, 0, 0, textStyle, font, textalignment);
			return;
		}
	}
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Unknown gameindex", 0, 0, textStyle, font, textalignment);
}

/*static void UI_Q3F_DrawKeyBinder(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Press a key to assign it to this command or escape to cancel", 0, 0, textStyle, font, textalignment);
}*/

static void UI_Q3F_DrawClassConfig_Active(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if(uiInfo.Q3F_SelectedClass > 0 && uiInfo.Q3F_SelectedClass < Q3F_CLASS_MAX) {
		bg_q3f_playerclass_t* cls = bg_q3f_classlist[uiInfo.Q3F_SelectedClass];
		int active = trap_Cvar_VariableValue(va("cg_execClass%sConfig", cls->commandstring));
		char* text;

		switch(active) {
		case 0:
			text = "^1Not Active";
			break;
		case 1:
			text = "^2Active on first spawn";
			break;
		case 2:
			text = "^2Active on all spawns";
			break;
		default:
			text = "";
			break;
		}

		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, text, 0, 0, textStyle, font, textalignment);
	}
}

/*static void UI_Q3F_DrawClassConfig_Name(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	if(uiInfo.Q3F_SelectedClass > 0 && uiInfo.Q3F_SelectedClass < Q3F_CLASS_MAX) {
		char text[128];
		bg_q3f_playerclass_t* cls = bg_q3f_classlist[uiInfo.Q3F_SelectedClass];

		Com_sprintf(text, sizeof(text), "This %s config is:", cls->title);

		Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, text, 0, 0, textStyle, font, textalignment);
	}
}*/

static void UI_Q3F_DrawFavServerName(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int number) {
	char addr[MAX_NAME_LENGTH];
	char buff[MAX_STRING_CHARS];

	addr[0] = '\0';

	if(trap_LAN_GetServerCount( AS_FAVORITES ) > number ) {
		trap_LAN_GetServerInfo( AS_FAVORITES, number, buff, MAX_STRING_CHARS );

		Q_strncpyz(addr, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
	
		if(!*addr) {
			Q_strncpyz(addr, "No Name", sizeof(addr));
		}
	}

	if(!*addr) {
		Q_strncpyz(addr, "- Empty -", sizeof(addr));
		textalignment = ITEM_ALIGN_CENTER;
		text_x = rect->w / 2.f;
	}

	Text_Paint_MaxWidth(rect->x + text_x, rect->y + text_y, scale, color, addr, 0, 0, textStyle, font, textalignment, rect->w);
}

static void UI_Q3F_DrawCDKey_Ok(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char* text;
	char buff[1024];

	buff[0] = '\0';
	Q_strncpyz(buff, UI_Cvar_VariableString("cdkey"), 1024); 
	buff[16] = '\0';

	if (trap_VerifyCDKey(buff, NULL)) {
		text = "CD Key Appears to be valid.";
		trap_SetCDKey(buff);
	} else {
		text = "CD Key does not appear to be valid.";
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, text, 0, 0, textStyle, font, textalignment);
}
// djbob

void UI_Q3F_SetVersion(void) {
	char buf[64];

	const char *mon_name[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// get the month
	Q_strncpyz( buf, compiledate, sizeof(buf) );
	buf[3] = '\0';

	for( compilemonth = 0; compilemonth < 12; compilemonth++ ) {
		if( !Q_stricmp( buf, mon_name[compilemonth] ) ) {
			compilemonth += 1;
			break;
		}
	}

	// get the day
	Q_strncpyz( buf, compiledate + 4, sizeof(buf) );
	buf[3] = '\0';

	compileday = atoi(buf);

	// get the year
	Q_strncpyz( buf, compiledate + 7, sizeof(buf) );

	compileyear = atoi(buf);
}

static void UI_DrawGLInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char * eptr;
	char buff[4096];
	const char *lines[64];
	int y, numLines, i;

	Text_Paint(rect->x + 2 + text_x, rect->y + text_y, scale, color, va("VENDOR: %s", uiInfo.uiDC.glconfig.vendor_string), 0, 30, textStyle, font, textalignment);
	Text_Paint(rect->x + 2 + text_x, rect->y + 15 + text_y, scale, color, va("VERSION: %s: %s", uiInfo.uiDC.glconfig.version_string,uiInfo.uiDC.glconfig.renderer_string), 0, 30, textStyle, font, textalignment);
	Text_Paint(rect->x + 2 + text_x, rect->y + 30 + text_y, scale, color, va ("PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)", uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits, uiInfo.uiDC.glconfig.stencilBits), 0, 30, textStyle, font, textalignment);

	// build null terminated extension strings
	Q_strncpyz(buff, uiInfo.uiDC.glconfig.extensions_string, 4096);
	eptr = buff;
	y = rect->y + 45;
	numLines = 0;

	while ( y < rect->y + rect->h && *eptr )
	{
		while ( *eptr && *eptr == ' ' )
			*eptr++ = '\0';

		// track start of valid string
		if (*eptr && *eptr != ' ') {
			lines[numLines++] = eptr;
		}

		while ( *eptr && *eptr != ' ' )
			eptr++;
	}

	i = 0;

	while (i < numLines) {
		Text_Paint(rect->x + 2 + text_x, y + text_y, scale, color, lines[i++], 0, 20, textStyle, font, textalignment);

		if (i < numLines) {
			Text_Paint(rect->x + rect->w / 2 + text_x, y + text_y, scale, color, lines[i++], 0, 20, textStyle, font, textalignment);
		}
		y += 10;

		if (y > rect->y + rect->h - 11) {
			break;
		}
	}
}

// RR2DO2
#define ABS_FLT(x)	((x)>=0?(float)(x):-(float)(x))

//static qboolean Q3F_PlayedReopenSound = qfalse;

static void UI_Q3F_DrawDuckModel( rectDef_t *rect ) {
	refdef_t	refdef;
	refEntity_t	ent;
	float		x, y, w, h;
	vec3_t		origin;
	vec3_t		angles;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 45;
	refdef.fov_y = 35;

	origin[0] = 50;
	origin[1] = 0;
	origin[2] = 0;

	refdef.time = uiInfo.uiDC.realTime;

	trap_R_ClearScene();

	// add the models
	memset( &ent, 0, sizeof(ent) );

	VectorSet( angles, 0, (float)uiInfo.uiDC.realTime / 10, 0 );

	AnglesToAxis( angles, ent.axis );
	ent.hModel = trap_R_RealRegisterModel( "models/mapobjects/duck/duck.md3" );
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	trap_R_AddRefEntityToScene( &ent );

	origin[0] = 118;
	origin[1] = sin((float)uiInfo.uiDC.realTime / 1000) * 100;
	origin[2] = -cos((float)uiInfo.uiDC.realTime / 1000) * 100;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.5f, 0.8f, 0, 0 );

	origin[1] = -cos((float)uiInfo.uiDC.realTime / 1000) * 50;
	origin[2] = sin((float)uiInfo.uiDC.realTime / 1000) * 50;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.8f, 0.7f, 0, 0 );

	trap_R_RenderScene( &refdef );
}

static void UI_Q3F_DrawFlagBack( rectDef_t *rect ) {
	refdef_t	refdef;
	refEntity_t	ent;
	float		x, y, w, h;
	vec3_t		origin;
	vec3_t		angles;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 45;
	refdef.fov_y = 35;

	refdef.time = uiInfo.uiDC.realTime;

	trap_R_ClearScene();

	// add the models
	memset( &ent, 0, sizeof(ent) );

	origin[0] = 180;
	origin[1] = -10;
	origin[2] = -30;

	VectorClear( angles );
	angles[PITCH]	= -30;
	angles[YAW]		= 90;
	angles[ROLL]	= 0;

	AnglesToAxis( angles, ent.axis );

	VectorScale(ent.axis[0], 1.1,	ent.axis[0]);
	VectorScale(ent.axis[1], 1.8,	ent.axis[1]);
	VectorScale(ent.axis[2], 1.2,	ent.axis[2]);

	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );
	ent.nonNormalizedAxes = qtrue;

	ent.hModel = trap_R_RealRegisterModel( "models/flags/b_flag.md3" );

	trap_R_AddRefEntityToScene( &ent );

	origin[0] = 180;
	origin[1] = 10;
	origin[2] = -30;

	VectorClear( angles );
	angles[PITCH]	= -30;
	angles[YAW]		= -90;
	angles[ROLL]	= 0;

	AnglesToAxis( angles, ent.axis );

	VectorScale(ent.axis[0], 1.1,	ent.axis[0]);
	VectorScale(ent.axis[1], 1.8,	ent.axis[1]);
	VectorScale(ent.axis[2], 1.2,	ent.axis[2]);

	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );
	ent.nonNormalizedAxes = qtrue;

	ent.hModel = trap_R_RealRegisterModel( "models/flags/r_flag.md3" );

	trap_R_AddRefEntityToScene( &ent );

	origin[0] = 118;
	origin[1] = sin((float)uiInfo.uiDC.realTime / 1000) * 100;
	origin[2] = -cos((float)uiInfo.uiDC.realTime / 1000) * 100;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.5f, 0.8f, 0, 0 );

	origin[1] = -cos((float)uiInfo.uiDC.realTime / 1000) * 50;
	origin[2] = sin((float)uiInfo.uiDC.realTime / 1000) * 50;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.8f, 0.7f, 0, 0 );

	trap_R_RenderScene( &refdef );

}

static void UI_Q3F_DrawBackModels( rectDef_t *rect ) {
	refdef_t	refdef;
	refEntity_t	ent;
	float		x, y, w, h;
	vec3_t		origin;
	vec3_t		angles;

	return;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 45;
	refdef.fov_y = 35;

	origin[0] = 120;
	origin[1] = 0;
	origin[2] = 0;

	refdef.time = uiInfo.uiDC.realTime;

	trap_R_ClearScene();

	// add the models
	memset( &ent, 0, sizeof(ent) );

	if(uiInfo.Q3F_BackModelStatus != Q3F_BM_STARTUP) {
		VectorSet( angles, 0, 0, 0 );
		AnglesToAxis( angles, ent.axis );
		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_OuterModel;
		VectorCopy( origin, ent.origin );
		VectorCopy( origin, ent.lightingOrigin );
		ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		VectorCopy( ent.origin, ent.oldorigin );

		trap_R_AddRefEntityToScene( &ent );
	}

/*	origin[0] = 120 - ABS_FLT(sin((float)uiInfo.uiDC.realTime / rot_speed / 360 * M_PI)) * ( 120 - 82 );

	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	VectorCopy( ent.origin, ent.oldorigin );

	// speed = 1, then it takes 180msec to rotate
	// speed = 10, then it takes 1800msec to rotate

	//VectorSet( angles, (float)abs((uiInfo.uiDC.realTime / 50)%180-90)-90, (float)uiInfo.uiDC.realTime / 50, 0 );
	//VectorSet( angles, (0.5 * sin((float)uiInfo.uiDC.realTime / 50 / 180 * M_PI) + 0.5 ) * 90, (float)uiInfo.uiDC.realTime / 50, 0 );
	VectorSet( angles, ABS_FLT(sin((float)uiInfo.uiDC.realTime / rot_speed / 180 * M_PI)) * 90, (float)uiInfo.uiDC.realTime / rot_speed, 0 );
	//VectorSet( angles, 0, 180, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = trap_R_RegisterModel( "models/fui/content.md3" );
	trap_R_AddRefEntityToScene( &ent );

	//VectorSet( angles, (float)abs((uiInfo.uiDC.realTime / 50)%180-90)-90, (float)uiInfo.uiDC.realTime / 50, (float)uiInfo.uiDC.realTime / 50 );
	VectorSet( angles, ABS_FLT(sin((float)uiInfo.uiDC.realTime / rot_speed / 180 * M_PI)) * 90, (float)uiInfo.uiDC.realTime / rot_speed, (float)uiInfo.uiDC.realTime / 15 );
	//VectorSet( angles, 0, 180, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = trap_R_RegisterModel( "models/fui/middle.md3" );
	trap_R_AddRefEntityToScene( &ent );

	//VectorSet( angles, (float)abs((uiInfo.uiDC.realTime / 50)%180-90)-90, (float)uiInfo.uiDC.realTime / 50, (float)uiInfo.uiDC.realTime / 5 );
	VectorSet( angles, ABS_FLT(sin((float)uiInfo.uiDC.realTime / rot_speed / 180 * M_PI)) * 90, (float)uiInfo.uiDC.realTime / rot_speed, (float)uiInfo.uiDC.realTime / 5 );
	//VectorSet( angles, 0, 180, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = trap_R_RegisterModel( "models/fui/centre.md3" );
	trap_R_AddRefEntityToScene( &ent );*/

	if( uiInfo.Q3F_BackModelRotateEndTime >= uiInfo.uiDC.realTime ) {
		if( uiInfo.Q3F_BackModelStatus == Q3F_BM_NEWCURRENT ) {
			origin[0] = 82;

			VectorCopy( origin, ent.origin );
			VectorCopy( origin, ent.lightingOrigin );
			VectorCopy( ent.origin, ent.oldorigin );

			VectorSet( angles, 0, 180, 0 );
		} else {
			//float rotatedTime = -( uiInfo.uiDC.realTime - uiInfo.Q3F_BackModelRotateEndTime );
			float rotatedTime = ( ( uiInfo.Q3F_BackModelStatus == Q3F_BM_REOPENING ? 2 : 1 ) * uiInfo.Q3F_BackModelRotateSpeed * 180 ) - ( uiInfo.Q3F_BackModelRotateEndTime - uiInfo.uiDC.realTime );

	/*		if ( uiInfo.Q3F_BackModelStatus == 4 &&
				 rotatedTime > uiInfo.Q3F_BackModelRotateSpeed * 180 &&
				 !Q3F_PlayedReopenSound ) {
				trap_S_StartLocalSound( uiInfo.Q3F_BackModelOpenSound, CHAN_ITEM );
				Q3F_PlayedReopenSound = qtrue;
			}*/

			if( uiInfo.Q3F_BackModelStatus == Q3F_BM_CLOSING || uiInfo.Q3F_BackModelStatus == Q3F_BM_REOPENING ) {
				rotatedTime += uiInfo.Q3F_BackModelRotateSpeed * 180;
			}

			origin[0] = 120 - ABS_FLT(sin(-rotatedTime / uiInfo.Q3F_BackModelRotateSpeed / 360 * M_PI)) * ( 120 - 82 );

			VectorCopy( origin, ent.origin );
			VectorCopy( origin, ent.lightingOrigin );
			VectorCopy( ent.origin, ent.oldorigin );

			//VectorSet( angles, ABS_FLT(sin(rotatedTime / uiInfo.Q3F_BackModelRotateSpeed / 180 * M_PI)) * 90, rotatedTime / uiInfo.Q3F_BackModelRotateSpeed, 0 );
			VectorSet( angles, 0, -rotatedTime / uiInfo.Q3F_BackModelRotateSpeed, 0 );
		}
	} else {
		if ( uiInfo.Q3F_BackModelStatus == Q3F_BM_CLOSING ) {
			uiInfo.Q3F_BackModelStatus = Q3F_BM_CLOSED;
		} else if ( uiInfo.Q3F_BackModelStatus == Q3F_BM_STARTUP ) {
			if(!uiInfo.Q3F_BackModelStartupTime)
			{
//				trap_S_StartLocalSound(uiInfo.Q3F_BackModelReOpenSound, CHAN_ITEM);

				uiInfo.Q3F_BackModelStartupTime = uiInfo.uiDC.realTime;
				uiInfo.Q3F_BackModelRotateEndTime = uiInfo.uiDC.realTime + Q3F_BM_STARTUPTIME;
			}
			else
			{
				VectorSet( angles, 0, 0, 0 );
				AnglesToAxis( angles, ent.axis );
				//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_OuterModel;
				VectorCopy( origin, ent.origin );
				VectorCopy( origin, ent.lightingOrigin );
				ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
				VectorCopy( ent.origin, ent.oldorigin );

				trap_R_AddRefEntityToScene( &ent );

				uiInfo.Q3F_BackModelStatus = Q3F_BM_CLOSED;
			}
		} else if ( uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENING || uiInfo.Q3F_BackModelStatus == Q3F_BM_REOPENING ) {
			uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;
//			Q3F_PlayedReopenSound = qfalse;
			if( uiInfo.Q3F_BackModelMenuToOpen ) {
				uiInfo.Q3F_BackModelMenuCurrent = uiInfo.Q3F_BackModelMenuToOpen;
				uiInfo.Q3F_BackModelMenuToOpen = NULL;
				Menus_ShowByName( uiInfo.Q3F_BackModelMenuCurrent );
			}
		} else if ( uiInfo.Q3F_BackModelStatus == Q3F_BM_NEWCURRENT ) {
			uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;
			trap_R_RemapShader( "ui/gfx/content", "ui/gfx/content", va("%f", (float)uiInfo.uiDC.realTime * 0.0001f) );
			if( uiInfo.Q3F_BackModelMenuToOpen ) {
				uiInfo.Q3F_BackModelMenuCurrent = uiInfo.Q3F_BackModelMenuToOpen;
				uiInfo.Q3F_BackModelMenuToOpen = NULL;
				Menus_ShowByName( uiInfo.Q3F_BackModelMenuCurrent );
			}
		}

		// now we know the state
		if ( uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENED ) {
			origin[0] = 82;

			VectorCopy( origin, ent.origin );
			VectorCopy( origin, ent.lightingOrigin );
			VectorCopy( ent.origin, ent.oldorigin );

			VectorSet( angles, 0, 180, 0 );
		}

		/*if ( ( uiInfo.Q3F_BackModelStatus == Q3F_BM_CLOSED ||
					uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENED ) &&
					uiInfo.uiDC.realTime - uiInfo.Q3F_BackModelRotateEndTime + 8 < 55 ) {

			// shaky shaky
			origin[0] += -sin( uiInfo.uiDC.realTime ) * .75f;
			origin[1] = sin( uiInfo.uiDC.realTime ) * .75f;
			origin[2] = cos( uiInfo.uiDC.realTime ) * .75f;

			VectorCopy( origin, ent.origin );
			VectorCopy( origin, ent.lightingOrigin );
			VectorCopy( ent.origin, ent.oldorigin );
		}*/
	}

	if(uiInfo.Q3F_BackModelStatus == Q3F_BM_STARTUP) {
		int elapsed = uiInfo.uiDC.realTime - uiInfo.Q3F_BackModelStartupTime;
		
		//origin[0] = 120 - (Q3F_BM_BACK_START_DIST+20) + ((Q3F_BM_BACK_START_DIST+20)*elapsed/Q3F_BM_STARTUPTIME);
		/*if( elapsed < Q3F_BM_STARTUPTIME * Q3F_BM_BACK_STOPSCALE )
			origin[0] = ( 120 - Q3F_BM_BACK_START_DIST ) + ( Q3F_BM_BACK_START_DIST * elapsed / ( Q3F_BM_STARTUPTIME * Q3F_BM_BACK_STOPSCALE ) );
		else
			origin[0] = 120;*/

		if( elapsed <= 700.f ) {
			origin[0] = 120.f - 130.f;
		} else if( elapsed > 700.f && elapsed < 1000.f ) {
			origin[0] = ( 120.f - 130.f ) + ( 140.f / 300.f ) * ( (float)elapsed - 700.f );
		} else if( elapsed >= 800.f && elapsed <= 1900.f ) {
			origin[0] = 120.f + 10.f;
		} else {
			origin[0] = ( 120.f + 10.f ) - ( 10.f / 100.f ) * ( (float)elapsed - 1900.f );
		}

		VectorSet( angles, 0.f, 0.f, 0.f );
		/*angles[0] = 0.f;
		angles[1] = 0.f;
		if( elapsed <= 1100.f ) {
			angles[2] = 15.f;
		} else if( elapsed > 1100.f && elapsed < 1550.f ) {
			angles[2] = 15.f - ( 20.f / 450.f ) * ( (float)elapsed - 1100.f );
		} else if( elapsed >= 1550.f && elapsed <= 1650.f ) {
			angles[2] = -5.f;
		} else if( elapsed > 1650.f && elapsed < 1900.f ) {
			angles[2] = -5.f + ( 5.f / 250.f ) * ( (float)elapsed - 1650.f );
		} else {
			angles[2] = 0.f;
		}*/

		VectorCopy( origin, ent.origin );
		VectorCopy( origin, ent.lightingOrigin );
		ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		VectorCopy( ent.origin, ent.oldorigin );

		//VectorSet( angles, 0, 0, 0 );
		AnglesToAxis( angles, ent.axis );
		
		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_OuterModel;
		//trap_R_AddRefEntityToScene( &ent );

//		-------------------------------------------------------------

		//origin[0] = 120 + Q3F_BM_FRONT_START_DIST - (Q3F_BM_FRONT_START_DIST*elapsed/Q3F_BM_STARTUPTIME);
		/*if( elapsed < Q3F_BM_STARTUPTIME * Q3F_BM_FRONT_STOPSCALE ) {
			origin[0] = 120 + Q3F_BM_FRONT_START_DIST - ( Q3F_BM_FRONT_START_DIST * elapsed / ( Q3F_BM_STARTUPTIME * Q3F_BM_FRONT_STOPSCALE ) );
			VectorSet( angles, Q3F_BM_FRONT_NO_ROTATIONS * 360 * elapsed / ( Q3F_BM_STARTUPTIME * Q3F_BM_FRONT_STOPSCALE ), 0, 0 );
		} else {
			origin[0] = 120;
			VectorSet( angles, 0, 0, 0 );
		}*/
		if( elapsed <= 150.f ) {
			origin[0] = 120.f + 160.f;
		} else if( elapsed > 150.f && elapsed < 1100.f ) {
			origin[0] = ( 120.f + 160.f ) - ( 165.f / 950.f ) * ( (float)elapsed - 150.f );
		} else if( elapsed >= 1100.f && elapsed <= 1900.f ) {
			origin[0] = 120.f - 5.f;
		} else {
			origin[0] = ( 120.f - 5.f ) + ( 5.f / 100.f ) * ( (float)elapsed - 1900.f );
		}

		angles[0] = 0.f;
		angles[1] = 0.f;
		if( elapsed <= 1100.f ) {
			angles[0] = 720.f * ( elapsed / 1100.f );
			angles[2] = -25.f;
		} else if( elapsed > 1100.f && elapsed < 1550.f ) {
			angles[2] = -25.f + ( 30.f / 450.f ) * ( (float)elapsed - 1100.f );
		} else if( elapsed >= 1550.f && elapsed <= 1650.f ) {
			angles[2] = 5.f;
		} else if( elapsed > 1650.f && elapsed < 1900.f ) {
			angles[2] = 5.f - ( 5.f / 250.f ) * ( (float)elapsed - 1650.f );
		} else {
			angles[2] = 0.f;
		}

		VectorCopy( origin, ent.origin );
		VectorCopy( origin, ent.lightingOrigin );
		VectorCopy( ent.origin, ent.oldorigin );

		//VectorSet( angles, Q3F_BM_FRONT_NO_ROTATIONS * 360 * elapsed / Q3F_BM_STARTUPTIME, 0, 0 );
		AnglesToAxis( angles, ent.axis );
		
		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_ContentModel;
		//trap_R_AddRefEntityToScene( &ent );

		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_MiddleModel;
		//trap_R_AddRefEntityToScene( &ent );

//		-------------------------------------------------------------

		/*if(elapsed < Q3F_BM_STARTUPTIME*Q3F_BM_LOGO_STOPSCALE)
			origin[0] = 120 - Q3F_BM_BACK_START_DIST + (Q3F_BM_BACK_START_DIST*elapsed/(Q3F_BM_STARTUPTIME*Q3F_BM_LOGO_STOPSCALE));
		else
			origin[0] = 120;*/
		/*origin[0] = ( 120 - Q3F_BM_INNER_START_DIST ) + ( Q3F_BM_INNER_START_DIST * elapsed / Q3F_BM_STARTUPTIME );*/
		if( elapsed <= 150.f ) {
			origin[0] = 120.f + 160.f;
		} else if( elapsed > 150.f && elapsed < 1100.f ) {
			origin[0] = ( 120.f + 160.f ) - ( 170.f / 950.f ) * ( (float)elapsed - 150.f );
		} else if( elapsed >= 1100.f && elapsed <= 1900.f ) {
			origin[0] = 120.f - 10.f;
		} else {
			origin[0] = ( 120.f - 10.f ) + ( 10.f / 100.f ) * ( (float)elapsed - 1900.f );
		}

		angles[0] = 0.f;
		angles[1] = 0.f;
		if( elapsed <= 1100.f ) {
			angles[0] = 720.f * ( elapsed / 1100.f );
			angles[2] = 35.f;
		} else if( elapsed > 1100.f && elapsed < 1550.f ) {
			angles[2] = 35.f - ( 40.f / 450.f ) * ( (float)elapsed - 1100.f );
		} else if( elapsed >= 1550.f && elapsed <= 1650.f ) {
			angles[2] = -5.f;
		} else if( elapsed > 1650.f && elapsed < 1900.f ) {
			angles[2] = -5.f + ( 5.f / 250.f ) * ( (float)elapsed - 1650.f );
		} else {
			angles[2] = 0.f;
		}

		VectorCopy( origin, ent.origin );
		VectorCopy( origin, ent.lightingOrigin );
		VectorCopy( ent.origin, ent.oldorigin );

		//VectorSet( angles, 0, 0, 0 );
		AnglesToAxis( angles, ent.axis );
		
		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_InnerModel;
		//trap_R_AddRefEntityToScene( &ent );
	}
	else {
		AnglesToAxis( angles, ent.axis );
		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_ContentModel;
		//trap_R_AddRefEntityToScene( &ent );

		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_MiddleModel;
		//trap_R_AddRefEntityToScene( &ent );

		//ent.hModel = uiInfo.uiDC.Assets.Q3F_BM_InnerModel;
		//trap_R_AddRefEntityToScene( &ent );
	}

	/*angles[2] = (float)uiInfo.uiDC.realTime / 80;
	AnglesToAxis( angles, ent.axis );
	ent.hModel = trap_R_RegisterModel( "ui/models/middle.md3" );
	trap_R_AddRefEntityToScene( &ent );*/

	// add the lights
	origin[0] = 118;
	origin[1] = sin((float)uiInfo.uiDC.realTime / 1000) * 100;
	origin[2] = -cos((float)uiInfo.uiDC.realTime / 1000) * 100;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.5f, 0.8f, 0, 0 );

	origin[1] = -cos((float)uiInfo.uiDC.realTime / 1000) * 50;
	origin[2] = sin((float)uiInfo.uiDC.realTime / 1000) * 50;

	trap_R_AddLightToScene( origin, 200, 1.f, 0.5f, 0.8f, 0.7f, 0, 0 );

	//trap_R_RenderScene( &refdef );
}

static void UI_Q3F_DrawServerRefreshProgress( rectDef_t *rect, vec4_t color, qhandle_t shader ) {
	//uiInfo.serverStatus.refreshActive
//	Com_Printf("Progress bar: %d filled out of %d max\n", uiInfo.serverStatus.numqueriedservers, uiInfo.serverStatus.maxservers);
	UI_Q3F_DrawProgress( rect, uiInfo.serverStatus.numqueriedservers, uiInfo.serverStatus.maxservers, color, shader );
}
// RR2DO2

void UI_DrawClassModel(			rectDef_t *rect );
void UI_DrawClassTitle(			rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font);
void HUD_DrawClassModel(		rectDef_t *rect );
void HUD_DrawClassHeadModel(	rectDef_t *rect );
void HUD_DrawClassDisabled(		rectDef_t *rect );
void HUD_DrawClassTitle(		rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font);
void HUD_DrawClassStat(			rectDef_t *rect, vec4_t color1, vec4_t color2, float pos, int stat, qhandle_t shader);
void HUD_DrawClassButtonText(	rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int cls);
void HUD_DrawClassButtonNew(	rectDef_t *rect, int cls, qboolean mouseover);
void HUD_DrawClassButtonActive(	rectDef_t *rect, qboolean mouseover);
void HUD_DrawClassButton(		rectDef_t *rect, vec4_t color, int cls);
void HUD_DrawTeamButton(		rectDef_t *rect, int team, qboolean mouseover);
void HUD_DrawTeamButtonText(	rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team);
void HUD_DrawChaseText(			rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team);
void HUD_DrawFollowText(		rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team);
void HUD_DrawClassInv(			rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawClassInfo(			rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont, int cls);
void HUD_DrawMapInfoBlurb(		rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawMapName(			rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawMatchString(		rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawVoteString(		rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawVoteTally(			rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont, qboolean yes);
void HUD_DrawTeamName(			rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, q3f_team_t team );
void HUD_DrawTeamCount(			rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, q3f_team_t team );
void HUD_DrawEndGameTeamScores( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font );
void HUD_DrawMapVoteName(		rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int num );
void HUD_DrawMapVoteTally(		rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int num );
void HUD_DrawChatBox (			rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font );
void HUD_DrawVoteMapInfoBlurb(	rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont);
void HUD_DrawMapVoteTallyBar(	rectDef_t* rect, int num );
void HUD_DrawMapVoteLevelshot(	rectDef_t* rect );
void HUD_DrawMapLvlShot(		rectDef_t* rect );
int  HUD_ClassUnavailable(		int cls);

// FIXME: table drive
//
static void UI_OwnerDraw( itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment) {
	rectDef_t rect;
	menuDef_t *parent;
	fontStruct_t *parentfont;

	rect.x = x + text_x;
	rect.y = y + text_y;
	rect.w = w;
	rect.h = h;

	parent = (menuDef_t*)item->parent;
	parentfont = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

	switch (ownerDraw) {
	case UI_HANDICAP:
		UI_DrawHandicap(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
    case UI_EFFECTS:
		UI_DrawEffects(&rect, scale, color);
		break;
    case UI_PREVIEWCINEMATIC:
		UI_DrawPreviewCinematic(&rect, scale, color);
		break;
/*    case UI_GAMETYPE:
		UI_DrawGameType(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;*/
/*    case UI_NETGAMETYPE:
		UI_DrawNetGameType(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;*/
/*    case UI_JOINGAMETYPE:
		UI_DrawJoinGameType(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;*/
    case UI_MAPPREVIEW:
		UI_DrawMapPreview(&rect, scale, color, qtrue);
		break;
    case UI_MAPCINEMATIC:
		UI_DrawMapCinematic(&rect, scale, color, qfalse);
		break;
    case UI_STARTMAPCINEMATIC:
		UI_DrawMapCinematic(&rect, scale, color, qtrue);
		break;
	case UI_NETSOURCE:
		UI_DrawNetSource(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
    case UI_NETMAPPREVIEW:
		UI_DrawNetMapPreview(&rect, scale, color);
		break;
    case UI_NETMAPCINEMATIC:
		UI_DrawNetMapCinematic(&rect, scale, color);
		break;
	case UI_ALLMAPS_SELECTION:
		UI_DrawAllMapsSelection(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont , qtrue);
		break;
	case UI_MAPS_SELECTION:
		UI_DrawAllMapsSelection(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont , qfalse);
		break;
	case UI_CROSSHAIR:
		UI_DrawCrosshair(&rect, scale, color);
		break;
	case UI_SERVERREFRESHDATE:
		UI_DrawServerRefreshDate(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	case UI_SERVERMOTD:
		UI_DrawServerMOTD(&rect, scale, color);
		break;
	case UI_GLINFO:
		UI_DrawGLInfo(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	case UI_KEYBINDSTATUS:
		UI_DrawKeyBindStatus(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	// RR2DO2
	case UI_FORT_BACKMODELS:
		UI_Q3F_DrawBackModels( &rect );
		break;	
	case UI_SERVERREFRESHPROGRESS:
		UI_Q3F_DrawServerRefreshProgress( &rect, color, shader );
		break;
	// RR2DO2
	// djbob
	case UI_GAMEINDEX_MAPNAME:
		UI_Q3F_DrawGameIndexMapName(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
//	case UI_CLASSCONFIG_NAME:
//		UI_Q3F_DrawClassConfig_Name(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont );
//		break;
	case UI_CLASSCONFIG_ACTIVE:
		UI_Q3F_DrawClassConfig_Active(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	case UI_FAV_SERVER_0:
	case UI_FAV_SERVER_1:
	case UI_FAV_SERVER_2:
	case UI_FAV_SERVER_3:
	case UI_FAV_SERVER_4:
	case UI_FAV_SERVER_5:
	case UI_FAV_SERVER_6:
	case UI_FAV_SERVER_7:
	case UI_FAV_SERVER_8:
	case UI_FAV_SERVER_9:
		UI_Q3F_DrawFavServerName(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont, ownerDraw - UI_FAV_SERVER_0);
		break;
	case UI_CDKEY_OK:
		UI_Q3F_DrawCDKey_Ok(&rect,scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case UI_SPINNING_DUCK:
		UI_Q3F_DrawDuckModel( &rect );
		break;
//	case UI_FORTLOGO:
//		UI_Q3F_DrawFortModel( &rect );
//		break;
	case UI_FLAGBACK:
		UI_Q3F_DrawFlagBack( &rect );
		break;

// slothy
	case UI_PBSTATUS:
		UI_DrawPBStatus(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case UI_HUD_PREVIEW:
		UI_DrawHudPreview(&rect, scale, color);
		break;
	case UI_CLASS_MODEL:
		UI_DrawClassModel(&rect);
		break;
	case UI_CLASS_TITLE:
		UI_DrawClassTitle(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case UI_WEAPON_PREVIEW:
		UI_DrawWeaponPreview(&rect, scale, color, qtrue);
		break;
	case HUD_MATCH_STRING:
		HUD_DrawMatchString(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;		
// end slothy

	case HUD_CLASS_TITLE:
		HUD_DrawClassTitle(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case HUD_MAP_BLURB:
		HUD_DrawMapInfoBlurb(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case HUD_MAP_NAME:
		HUD_DrawMapName(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;		
	case HUD_VOTE_STRING:
		HUD_DrawVoteString(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;		
	case HUD_VOTE_TALLY_YES:
		HUD_DrawVoteTally(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, qtrue);
		break;		
	case HUD_VOTE_TALLY_NO:
		HUD_DrawVoteTally(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, qfalse);
		break;		
	case HUD_MAP_LVLSHOT:
		HUD_DrawMapLvlShot(&rect);
		break;
	case HUD_CLASS_INV:
		HUD_DrawClassInv(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;
	case HUD_CLASS_INFO:
		HUD_DrawClassInfo(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, item->special);
		break;
	case HUD_CLASS_MODEL:
		HUD_DrawClassModel(&rect);
		break;
	case HUD_CLASS_HEADMODEL:
		HUD_DrawClassHeadModel(&rect);
		break;
	case HUD_CLASS_DISABLED:
		HUD_DrawClassDisabled(&rect);
		break;
	case HUD_CLASS_STAT_HEALTH:
		HUD_DrawClassStat(&rect, color, item->window.backColor, item->special, 0, item->window.background);
		break;
	case HUD_CLASS_STAT_ARMOUR:
		HUD_DrawClassStat(&rect, color, item->window.backColor, item->special, 1, item->window.background);
		break;
	case HUD_CLASS_STAT_SPEED:
		HUD_DrawClassStat(&rect, color, item->window.backColor, item->special, 2, item->window.background);
		break;
	case HUD_CLASS_CHOOSE_BUTTON:
		HUD_DrawClassButtonNew(&rect, item->special, item->window.flags & WINDOW_MOUSEOVER);
		break;
	case HUD_CLASS_CHOOSE_BTNFLAT:
		HUD_DrawClassButton(&rect, item->window.backColor, item->special);
		break;
	case HUD_CLASS_CHOOSE_BTN:
		HUD_DrawClassButtonActive(&rect, item->window.flags & WINDOW_MOUSEOVER);
		break;
	case HUD_CLASS_CHOOSE_BTNTEXT:
		HUD_DrawClassButtonText(&rect,scale, item->window.foreColor, textStyle, textalignment, text_x, text_y, parentfont, item->special);
		break;
	case HUD_TEAM_CHOOSE_BUTTON:
		HUD_DrawTeamButton(&rect, item->special, item->window.flags & WINDOW_MOUSEOVER);
		break;
	case HUD_TEAM_CHOOSE_TEXT:
		HUD_DrawTeamButtonText(&rect,scale, item->window.foreColor, textStyle, textalignment, text_x, text_y, parentfont, item->special);
		break;
	case HUD_FOLLOW_TEXT:
		HUD_DrawFollowText(&rect, scale, item->window.foreColor, textStyle, textalignment, text_x, text_y, parentfont, item->special);
		break;
	case HUD_CHASE_TEXT:
		HUD_DrawChaseText(&rect, scale, item->window.foreColor, textStyle, textalignment, text_x, text_y, parentfont, item->special);
		break;
	case HUD_TEAM_NAME_RED:
	case HUD_TEAM_NAME_BLUE:
	case HUD_TEAM_NAME_YELLOW:
	case HUD_TEAM_NAME_GREEN:
		HUD_DrawTeamName(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, ownerDraw - HUD_TEAM_NAME_RED);
		break;
	case CG_RED_TEAMCOUNT:
	case CG_BLUE_TEAMCOUNT:
	case CG_YELLOW_TEAMCOUNT:
	case CG_GREEN_TEAMCOUNT:
		HUD_DrawTeamCount(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, ownerDraw - CG_RED_TEAMCOUNT);
		break;
	case HUD_ENDGAME_TEAMSCORES:
		HUD_DrawEndGameTeamScores(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	case HUD_CHAT:
		HUD_DrawChatBox(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
		break;
	case HUD_MAPVOTE_NAME_0:
	case HUD_MAPVOTE_NAME_1:
	case HUD_MAPVOTE_NAME_2:
	case HUD_MAPVOTE_NAME_3:
	case HUD_MAPVOTE_NAME_4:
	case HUD_MAPVOTE_NAME_5:
	case HUD_MAPVOTE_NAME_6:
	case HUD_MAPVOTE_NAME_7:
	case HUD_MAPVOTE_NAME_8:
	case HUD_MAPVOTE_NAME_9:
		HUD_DrawMapVoteName(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, ownerDraw - HUD_MAPVOTE_NAME_0);
		break;
	case HUD_MAPVOTE_TALLY_0:
	case HUD_MAPVOTE_TALLY_1:
	case HUD_MAPVOTE_TALLY_2:
	case HUD_MAPVOTE_TALLY_3:
	case HUD_MAPVOTE_TALLY_4:
	case HUD_MAPVOTE_TALLY_5:
	case HUD_MAPVOTE_TALLY_6:
	case HUD_MAPVOTE_TALLY_7:
	case HUD_MAPVOTE_TALLY_8:
	case HUD_MAPVOTE_TALLY_9:
		HUD_DrawMapVoteTally(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, ownerDraw - HUD_MAPVOTE_TALLY_0);
		break;
	case HUD_MAPVOTE_BAR_0:
	case HUD_MAPVOTE_BAR_1:
	case HUD_MAPVOTE_BAR_2:
	case HUD_MAPVOTE_BAR_3:
	case HUD_MAPVOTE_BAR_4:
	case HUD_MAPVOTE_BAR_5:
	case HUD_MAPVOTE_BAR_6:
	case HUD_MAPVOTE_BAR_7:
	case HUD_MAPVOTE_BAR_8:
	case HUD_MAPVOTE_BAR_9:
		HUD_DrawMapVoteTallyBar(&rect, ownerDraw - HUD_MAPVOTE_BAR_0);
		break;
	case HUD_MAPVOTE_LEVELSHOT:
		HUD_DrawMapVoteLevelshot(&rect);
		break;
	case HUD_MAPVOTE_BLURB:
		HUD_DrawVoteMapInfoBlurb(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
		break;

// djbob
    default:
		break;
	}
}

static qboolean UI_OwnerDrawVisible(int flags) {
	uiClientState_t	cs;
	int		team;
	char	info[MAX_INFO_STRING];

	if (flags & UI_SHOW_FAVORITESERVERS) {
		// this assumes you only put this type of display flag on something showing in the proper context
		if (ui_netSource.integer != AS_FAVORITES) {
			return qfalse;
		}
	}
	
	if (flags & UI_SHOW_NOTFAVORITESERVERS) {
		// this assumes you only put this type of display flag on something showing in the proper context
		if (ui_netSource.integer == AS_FAVORITES) {
			return qfalse;
		}
	} 
		
	if (flags & UI_SHOW_DEMOAVAILABLE) {
		if (!uiInfo.demoAvailable) {
			return qfalse;
		}
	}

	if (flags & UI_SHOW_RED_TEAM_EXISTS) {
		char buffer[16];
		int mask;

		trap_GetConfigString(CS_TEAMMASK, buffer, 16);

		mask = atoi(buffer);

		if(!(mask & (1 << Q3F_TEAM_RED))) {
			return qfalse;
		}
	}

	if (flags & UI_SHOW_BLUE_TEAM_EXISTS) {
		char buffer[16];
		int mask;

		trap_GetConfigString(CS_TEAMMASK, buffer, 16);

		mask = atoi(buffer);

		if(!(mask & (1 << Q3F_TEAM_BLUE))) {
			return qfalse;
		}
	}

	if (flags & UI_SHOW_GREEN_TEAM_EXISTS) {
		char buffer[16];
		int mask;

		trap_GetConfigString(CS_TEAMMASK, buffer, 16);

		mask = atoi(buffer);

		if(!(mask & (1 << Q3F_TEAM_GREEN))) {
			return qfalse;
		}
	}

	if (flags & UI_SHOW_YELLOW_TEAM_EXISTS) {
		char buffer[16];
		int mask;

		trap_GetConfigString(CS_TEAMMASK, buffer, 16);

		mask = atoi(buffer);

		if(!(mask & (1 << Q3F_TEAM_YELLOW))) {
			return qfalse;
		}
	}

	if(flags & CG_SHOW_ON_RED_TEAM) {
		trap_GetClientState( &cs );
		trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

		team = atoi(Info_ValueForKey(info, "t"));
		if(team != Q3F_TEAM_RED) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_BLUE_TEAM ) {
		trap_GetClientState( &cs );
		trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

		team = atoi(Info_ValueForKey(info, "t"));
		if(team != Q3F_TEAM_BLUE) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_YELLOW_TEAM ) {
		trap_GetClientState( &cs );
		trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

		team = atoi(Info_ValueForKey(info, "t"));
		if(team != Q3F_TEAM_YELLOW) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_GREEN_TEAM ) {
		trap_GetClientState( &cs );
		trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

		team = atoi(Info_ValueForKey(info, "t"));
		if(team != Q3F_TEAM_GREEN) {
			return qfalse;
		}
	}

	return qtrue;
}

static qboolean UI_Handicap_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
    int h;
    h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
		if (key == K_MOUSE2) {
	    h -= 5;
		} else {
	    h += 5;
		}
    if (h > 100) {
      h = 5;
    } else if (h < 0) {
			h = 100;
		}
  	trap_Cvar_Set( "handicap", va( "%i", h) );
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_Effects_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
	    uiInfo.effectsColor--;
		} else {
	    uiInfo.effectsColor++;
		}

    if( uiInfo.effectsColor > 6 ) {
	  	uiInfo.effectsColor = 0;
		} else if (uiInfo.effectsColor < 0) {
	  	uiInfo.effectsColor = 6;
		}

	  trap_Cvar_SetValue( "color", uitogamecode[uiInfo.effectsColor] );
    return qtrue;
  }
  return qfalse;
}

/*static qboolean UI_JoinGameType_HandleKey(int flags, float *special, int key) {
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
			ui_joinGameType.integer--;
		} else {
			ui_joinGameType.integer++;
		}

		if (ui_joinGameType.integer < 0) {
			ui_joinGameType.integer = numQ3FGameJoinTypes - 1;
		} else if (ui_joinGameType.integer >= numQ3FGameJoinTypes) {
			ui_joinGameType.integer = 0;
		}

		trap_Cvar_Set( "ui_joinGameType", va("%d", ui_joinGameType.integer));
		UI_BuildServerDisplayList(qtrue);
		return qtrue;
	}
	return qfalse;
}*/



static qboolean UI_NetSource_HandleKey(int flags, float *special, int key) {
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		int value = ui_netSource.integer;
		
		if (key == K_MOUSE2) {
			value--;
		} else {
			value++;
		}
    
		if (value >= numNetSources) {
			value = 0;
		} else if (value < 0) {
			value = numNetSources - 1;
		}

		trap_Cvar_Set( "ui_netSource", va("%d", value));
		ui_netSource.integer = value;

		UI_BuildServerDisplayList(qtrue);
		if (ui_netSource.integer != AS_GLOBAL) {
			UI_StartServerRefresh(qtrue);
		}
		return qtrue;
	}
	return qfalse;
}

/*static qboolean UI_RedBlue_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		uiInfo.redBlue ^= 1;
		return qtrue;
	}
	return qfalse;
}*/

static qboolean UI_Crosshair_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			uiInfo.currentCrosshair--;
		} else {
			uiInfo.currentCrosshair++;
		}

		if (uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
			uiInfo.currentCrosshair = 0;
		} else if (uiInfo.currentCrosshair < 0) {
			uiInfo.currentCrosshair = NUM_CROSSHAIRS - 1;
		}
		trap_Cvar_Set("cg_drawCrosshair", va("%d", uiInfo.currentCrosshair)); 
		return qtrue;
	}
	return qfalse;
}

//slothy
static qboolean UI_PBStatus_HandleKey(int flags, float *special, int key) {
//int pbstatus;

  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
  {
//	pbstatus = (int)trap_Cvar_VariableValue("cl_punkbuster");
//	pbstatus ? trap_SetPbClStatus(0) : trap_SetPbClStatus(1);
	return qtrue;
  }
  return qfalse;
}

static qboolean UI_ClassConfig_Active_HandleKey(int flags, float *special, int key) {
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if(uiInfo.Q3F_SelectedClass > 0 && uiInfo.Q3F_SelectedClass < Q3F_CLASS_MAX) {
			bg_q3f_playerclass_t* cls = bg_q3f_classlist[uiInfo.Q3F_SelectedClass];
			int active = trap_Cvar_VariableValue(va("cg_execClass%sConfig", cls->commandstring));
			char buf[32];

			active++;
			active %= 3;

			Com_sprintf(buf, sizeof(buf), "%i", active);

			trap_Cvar_Set(va("cg_execClass%sConfig", cls->commandstring), buf);
		}

		return qtrue;
	}
	return qfalse;
}

static qboolean UI_CheckFavServerVersion( const char *info )
{
	const char *mapname = NULL;
	const char *val = NULL;
	int pure = 0;

	val = Info_ValueForKey(info, "balancedteams");
	pure = atoi(Info_ValueForKey(info, "maxlives")) != 0;
	if( strcmp(val, FORTS_SHORTVERSION) != 0 && pure )
	{
		Com_Error(ERR_DROP, "This server is NOT running " FORTS_VERSION "!\nJoining this server will not function properly with this version." );
		return qfalse;
	}

	mapname = Info_ValueForKey( info, "mapname");
	if(mapname && *mapname)
	{
		char bspName[MAX_QPATH] = { 0 };
		fileHandle_t fp = NULL_FILE;
		Com_sprintf( bspName, sizeof(bspName), "maps/%s.bsp", mapname );
		if ( trap_FS_FOpenFile( bspName, &fp, FS_READ ) <= 0 ) {
			if( fp != NULL_FILE )
				trap_FS_FCloseFile( fp );
			Com_Error( ERR_DROP, "You don't have the map running on this server: \"%s\"\nCheck with the community to find proper version!", mapname );
			return qfalse;
		}
	}

	return qtrue;
}

static qboolean	UI_FavouriteServer_HandleKey(int flags, float *special, int key, int number) {
	char addr[MAX_NAME_LENGTH];
	char buff[MAX_STRING_CHARS];

	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if(trap_LAN_GetServerCount( AS_FAVORITES ) <= number )
			return qfalse;

		trap_LAN_GetServerInfo( AS_FAVORITES, number, buff, MAX_STRING_CHARS );

		addr[0] = '\0';
		Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);

		if(!*addr) {
			return qfalse;
		}

		if ( !UI_CheckFavServerVersion( buff ) )
			return qfalse;

		uiInfo.uiDC.executeText( EXEC_APPEND, va( "connect %s\n", addr ) );

		return qtrue;
	}

	return qfalse;
}

static qboolean UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	switch (ownerDraw) {
    case UI_HANDICAP:
		return UI_Handicap_HandleKey(flags, special, key);
		break;
    case UI_EFFECTS:
		return UI_Effects_HandleKey(flags, special, key);
		break;
/*    case UI_JOINGAMETYPE:
		return UI_JoinGameType_HandleKey(flags, special, key);
		break;*/
	case UI_NETSOURCE:
		UI_NetSource_HandleKey(flags, special, key);
		break;
	case UI_CROSSHAIR:
		UI_Crosshair_HandleKey(flags, special, key);
		break;
	case UI_CLASSCONFIG_ACTIVE:
		UI_ClassConfig_Active_HandleKey(flags, special, key);
		break;
	case UI_FAV_SERVER_0:
	case UI_FAV_SERVER_1:
	case UI_FAV_SERVER_2:
	case UI_FAV_SERVER_3:
	case UI_FAV_SERVER_4:
	case UI_FAV_SERVER_5:
	case UI_FAV_SERVER_6:
	case UI_FAV_SERVER_7:
	case UI_FAV_SERVER_8:
	case UI_FAV_SERVER_9:
		UI_FavouriteServer_HandleKey(flags, special, key, ownerDraw - UI_FAV_SERVER_0);
		break;

// slothy
	case UI_PBSTATUS:
		UI_PBStatus_HandleKey(flags, special, key);
		break;


    default:
		break;
	}

	return qfalse;
}


static float UI_GetValue(int ownerDraw) {
  return 0;
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 ) {
	return trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int*)arg1, *(int*)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force) {

	if ( !force ) {
		if ( uiInfo.serverStatus.sortKey == column ) {
			return;
		}
	}

	uiInfo.serverStatus.sortKey = column;
	qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);

	// update displayed levelshot
	UI_FeederSelection( FEEDER_SERVERS, uiInfo.serverStatus.currentServer );
}

/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods() {
	int		numdirs;
	char	dirlist[2048];
	char	*dirptr;
	char  *descptr;
	int		i;
	int		dirlen;

	uiInfo.modCount = 0;
	numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for( i = 0; i < numdirs; i++ ) {
		dirlen = strlen( dirptr ) + 1;
		descptr = dirptr + dirlen;
		uiInfo.modList[uiInfo.modCount].modName = String_Alloc(dirptr);
		uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc(descptr);
		dirptr += dirlen + strlen(descptr) + 1;
		uiInfo.modCount++;
		if (uiInfo.modCount >= MAX_MODS) {
			break;
		}
	}

}

/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies() {
	char	movielist[4096];
	char	*moviename;
	int		i, len;

	uiInfo.movieCount = trap_FS_GetFileList( "video", "roq", movielist, 4096 );

	if (uiInfo.movieCount) {
		if (uiInfo.movieCount > MAX_MOVIES) {
			uiInfo.movieCount = MAX_MOVIES;
		}
		moviename = movielist;
		for ( i = 0; i < uiInfo.movieCount; i++ ) {
			len = strlen( moviename );
			if (!Q_stricmp(moviename +  len - 4,".roq")) {
				moviename[len-4] = '\0';
			}
			Q_strupr(moviename);
			uiInfo.movieList[i] = String_Alloc(moviename);
			moviename += len + 1;
		}
	}
}



/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos() {
	char	demolist[3000];
	char demoExt[32];
	char	*demoname;
	int		i = 0;
	int		j, len;
	int		olddemocount;

	Com_sprintf(demoExt, sizeof(demoExt), "dm_%d", (int)trap_Cvar_VariableValue("protocol"));

	uiInfo.demoCount = trap_FS_GetFileList( "demos", demoExt, demolist, sizeof(demolist) );

	Com_sprintf(demoExt, sizeof(demoExt), ".dm_%d", (int)trap_Cvar_VariableValue("protocol"));

	if (uiInfo.demoCount) {
		if (uiInfo.demoCount > MAX_DEMOS) {
			uiInfo.demoCount = MAX_DEMOS;
		}
		demoname = demolist;
		for ( i = 0; i < uiInfo.demoCount; i++ ) {
			len = strlen( demoname );
//			if (!Q_stricmp(demoname +  len - strlen(demoExt), demoExt)) {
//				demoname[len-strlen(demoExt)] = '\0';
//			}
//			Q_strupr(demoname);			// slothy - don't uppercase the filename, it breaks on linux!
			uiInfo.demoList[i] = String_Alloc(demoname);
			demoname += len + 1;
		}
	}

	if((int)trap_Cvar_VariableValue("protocol") == 84) {
		// this can also play dm_83 demos
		Com_sprintf(demoExt, sizeof(demoExt), "dm_%d", 83);

		olddemocount = trap_FS_GetFileList( "demos", demoExt, demolist, 4096 );

		Com_sprintf(demoExt, sizeof(demoExt), ".dm_%d", 83);

		if (olddemocount) {
			uiInfo.demoCount += olddemocount;
			if (uiInfo.demoCount > MAX_DEMOS) {
				olddemocount -= (uiInfo.demoCount - MAX_DEMOS);
				uiInfo.demoCount = MAX_DEMOS;
			}
			demoname = demolist;
			for ( j = 0; j < olddemocount; j++ ) {
				len = strlen( demoname );
//				if (!Q_stricmp(demoname +  len - strlen(demoExt), demoExt)) {
//					demoname[len-strlen(demoExt)] = '\0';
//				}
				//			Q_strupr(demoname);			// slothy - don't uppercase the filename, it breaks on linux!
				uiInfo.demoList[j + i] = String_Alloc(demoname);
				demoname += len + 1;
			}
		}
	}
}

/*
===============
UI_LoadConfigs
===============
*/
static void UI_LoadConfigs() {
	char	cfglist[4096];
	char	*cfgname;
	int		i, len;

	uiInfo.cfgCount = trap_FS_GetFileList( "", ".cfg", cfglist, 4096 );

	if (uiInfo.cfgCount) {
		if (uiInfo.cfgCount > MAX_CONFIGS) {
			uiInfo.cfgCount = MAX_CONFIGS;
		}
		cfgname = cfglist;
		for ( i = 0; i < uiInfo.cfgCount; i++ ) {
			len = strlen( cfgname );
			if (!Q_stricmp(cfgname +  len - 4, ".cfg")) {
				cfgname[len-4] = '\0';
			}
			Q_strupr(cfgname);
			uiInfo.cfgList[i] = String_Alloc(cfgname);
			cfgname += len + 1;
		}
	}

}

/*static qboolean UI_SetNextMap(int actual, int index) {
	int i;
	for (i = actual + 1; i < uiInfo.mapCount; i++) {
		Menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
		return qtrue;
	}
	return qfalse;
}*/

void UI_SetupGameIndexMulti() {
	char buf[256];
	char *p, *start;
	menuDef_t *menu;

	trap_Cvar_Set( "g_gameindex", "1" );
	trap_Cvar_Set( "hud_admingameindex", "1");

	menu = Menus_FindByName(uiInfo.mapMenuName);
	if(menu) {
		itemDef_t* item = Menu_FindItemByName(menu, "gameIndexList");
		if(item) {
			multiDef_t *multiPtr;
			multiPtr = (multiDef_t*)item->typeData;
			multiPtr->strDef = qfalse;
			multiPtr->count = 0;

			if(uiInfo.mapList[ui_currentNetMap.integer].gameIndicies) {
//				Com_Printf("%s\n", uiInfo.mapList[ui_currentNetMap.integer].gameIndicies);

				Q_strncpyz(buf, uiInfo.mapList[ui_currentNetMap.integer].gameIndicies, 256);

				start = buf;
				while((p = strchr(start, ',')) != NULL) {
					*p = '\0';

					multiPtr->cvarList[multiPtr->count] = String_Alloc(start);
					multiPtr->cvarValue[multiPtr->count] = atof(start);

					multiPtr->count++;

					start = p + 1;

					if (multiPtr->count >= MAX_MULTI_CVARS) {
						return;
					}
				}

				if(*start) {
					multiPtr->cvarList[multiPtr->count] = String_Alloc(start);
					sscanf(start, "%f", &multiPtr->cvarValue[multiPtr->count]);
					multiPtr->count++;
				}
			}
			else {
				item->window.flags &= ~WINDOW_VISIBLE;
				multiPtr->cvarList[multiPtr->count] = "1";
				multiPtr->cvarValue[multiPtr->count] = 1;
				multiPtr->count++;
			}
		}
	}
}

void UI_ChangeGameIndex(void) {
	int i;
	int value = -1;
	int number = trap_Cvar_VariableValue("g_gameindex");

	for( i = 0; i < uiInfo.mapList[ui_currentNetMap.integer].numGameIndicies; i++) {
		if(uiInfo.mapList[ui_currentNetMap.integer].gameIndiciesInfo[i].number == number) {
			value = i;
			break;
		}
	}

	if(value != -1) {
		value++;

		if(value >= uiInfo.mapList[ui_currentNetMap.integer].numGameIndicies)
			value = 0;

		trap_Cvar_Set("g_gameindex", va("%i", uiInfo.mapList[ui_currentNetMap.integer].gameIndiciesInfo[value].number));
		trap_Cvar_Set("hud_admingameindex", va("%i", uiInfo.mapList[ui_currentNetMap.integer].gameIndiciesInfo[value].number));
	}
}

static void UI_SetupBindingTable(configData_t* start, int size) {
	configData_t* configData = start;
	int i;
	int twokeys[2];
	char buffer[1024];

	for(i = 0; i < size; i++) {
		switch(configData->type) {
		case CONFIG_TYPE_BIND:
			configData->primary = configData->secondary = -1;
			Controls_GetKeyAssignment(configData->command, twokeys);
			configData->primary = twokeys[0];
			configData->secondary = twokeys[1];
			break;
		case CONFIG_TYPE_YESNO:
			configData->value = trap_Cvar_VariableValue(configData->command) ? qtrue : qfalse;
			break;
		case CONFIG_TYPE_SLIDER:
			configData->value = trap_Cvar_VariableValue(configData->command) * configData->scale;
			break;
		case CONFIG_TYPE_FLOATLIST:
			configData->value = trap_Cvar_VariableValue(configData->command);
			break;
		case CONFIG_TYPE_TEXTLIST:
			trap_Cvar_VariableStringBuffer(configData->command, buffer, 1024);
			configData->strvalue = String_Alloc(buffer);
			break;
		default:
			break;
		}					  

		configData++;
	}
}

static void UI_ApplyBindingTable(configData_t* start, int size) {
	configData_t* configData = start;
	int i;

	for(i = 0; i < size; i++) {
		switch(configData->type) {
		case CONFIG_TYPE_BIND:
			if(configData->primary != -1) {
				uiInfo.uiDC.setBinding(configData->primary, configData->command);
				if(configData->secondary != -1)
					uiInfo.uiDC.setBinding(configData->secondary, configData->command);
			}
			break;
		case CONFIG_TYPE_SLIDER:
			trap_Cvar_SetValue(configData->command, configData->value / configData->scale);
			break;
		case CONFIG_TYPE_YESNO:
		case CONFIG_TYPE_FLOATLIST:
			trap_Cvar_SetValue(configData->command, configData->value);
			break;
		case CONFIG_TYPE_TEXTLIST:
			trap_Cvar_Set(configData->command, configData->strvalue);
			break;
		default:
			break;
		}

		configData++;
	}
}

configData_t* configDataTableList[] = {
	configDataTable_Move,
	configDataTable_Shoot,
	configDataTable_Misc,
	configDataTable_Coms,
	NULL
};

int* configDataTableList_size[] = {
	&configDataTable_Move_Size,
	&configDataTable_Shoot_Size,
	&configDataTable_Misc_Size,
	&configDataTable_Coms_Size,
	NULL
};

configData_t* systemDataTableList[] = {
	systemDataTable_Graphics,
	systemDataTable_Display,
	systemDataTable_Sound,
	systemDataTable_Network,
	NULL
};

int* systemDataTableList_size[] = {
	&systemDataTable_Graphics_Size,
	&systemDataTable_Display_Size,
	&systemDataTable_Sound_Size,
	&systemDataTable_Network_Size,
	NULL
};

void UI_ReadBindings() {	
	int i;
	for(i = 0; configDataTableList[i]; i++)
		UI_SetupBindingTable(configDataTableList[i], *configDataTableList_size[i]);
}

static void UI_ApplyBindings() {	
	int i;
	for(i = 0; configDataTableList[i]; i++)
		UI_ApplyBindingTable(configDataTableList[i], *configDataTableList_size[i]);

#if !defined( __MACOS__ )
	uiInfo.uiDC.executeText(EXEC_APPEND, "in_restart\n");
#endif
}

static void UI_ReadSystemSettings() {	
	int i;
	for(i = 0; systemDataTableList[i]; i++)
		UI_SetupBindingTable(systemDataTableList[i], *systemDataTableList_size[i]);
}

static void UI_ETFReadSystemSettings() {
	char buf[MAX_STRING_CHARS];

	trap_Cvar_SetValue("ui_r_mode", trap_Cvar_VariableValue("r_mode"));
	trap_Cvar_SetValue("ui_r_colorbits", trap_Cvar_VariableValue("r_colorbits"));
	trap_Cvar_SetValue("ui_r_depthbits", trap_Cvar_VariableValue("r_depthbits"));
	trap_Cvar_SetValue("ui_r_fullscreen", trap_Cvar_VariableValue("r_fullscreen"));
	trap_Cvar_SetValue("ui_r_lodbias", trap_Cvar_VariableValue("r_lodbias"));
	trap_Cvar_SetValue("ui_r_subdivisions", trap_Cvar_VariableValue("r_subdivisions"));
	trap_Cvar_SetValue("ui_r_picmip", (int)trap_Cvar_VariableValue("r_picmip"));
	trap_Cvar_SetValue("ui_r_texturebits", trap_Cvar_VariableValue("r_texturebits"));
	trap_Cvar_SetValue("ui_r_ext_compressed_textures", trap_Cvar_VariableValue("r_ext_compressed_textures"));
	trap_Cvar_SetValue("ui_r_gamma", trap_Cvar_VariableValue("r_gamma"));
	trap_Cvar_SetValue("ui_cg_viewsize", trap_Cvar_VariableValue("cg_viewsize"));
	trap_Cvar_SetValue("ui_r_ignorehwgamma", trap_Cvar_VariableValue("r_ignorehwgamma"));
	trap_Cvar_SetValue("ui_com_maxfps", trap_Cvar_VariableValue("com_maxfps"));
	trap_Cvar_SetValue("ui_r_customwidth", trap_Cvar_VariableValue("r_customwidth"));
	trap_Cvar_SetValue("ui_r_customheight", trap_Cvar_VariableValue("r_customheight"));

	trap_Cvar_VariableStringBuffer( "r_textureMode", buf, MAX_STRING_CHARS );
	trap_Cvar_Set( "ui_r_textureMode", buf );
/*
	value = trap_Cvar_VariableValue(configData->command);
	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );


	trap_Cvar_Set( const char *var_name, const char *value );
	trap_Cvar_SetValue( const char *var_name, float value );
	*/
	
//		UI_SetupBindingTable(systemDataTableList[i], *systemDataTableList_size[i]);
}

static void UI_ETFApplySystemSettings() {
	char buf[MAX_STRING_CHARS];
//	float value;
	
	trap_Cvar_SetValue("r_mode", trap_Cvar_VariableValue("ui_r_mode"));
	trap_Cvar_SetValue("r_colorbits", trap_Cvar_VariableValue("ui_r_colorbits"));
	trap_Cvar_SetValue("r_depthbits", trap_Cvar_VariableValue("ui_r_depthbits"));
	trap_Cvar_SetValue("r_fullscreen", trap_Cvar_VariableValue("ui_r_fullscreen"));
	trap_Cvar_SetValue("r_lodbias", trap_Cvar_VariableValue("ui_r_lodbias"));
	trap_Cvar_SetValue("r_subdivisions", trap_Cvar_VariableValue("ui_r_subdivisions"));
	trap_Cvar_SetValue("r_picmip", trap_Cvar_VariableValue("ui_r_picmip"));
	trap_Cvar_SetValue("r_texturebits", trap_Cvar_VariableValue("ui_r_texturebits"));
	trap_Cvar_SetValue("r_ext_compressed_textures", trap_Cvar_VariableValue("ui_r_ext_compressed_textures"));
	trap_Cvar_SetValue("r_gamma", trap_Cvar_VariableValue("ui_r_gamma"));
	trap_Cvar_SetValue("cg_viewsize", trap_Cvar_VariableValue("ui_cg_viewsize"));
	trap_Cvar_SetValue("r_ignorehwgamma", trap_Cvar_VariableValue("ui_r_ignorehwgamma"));
	trap_Cvar_SetValue("com_maxfps", trap_Cvar_VariableValue("ui_com_maxfps"));
	trap_Cvar_SetValue("r_customwidth", trap_Cvar_VariableValue("ui_r_customwidth"));
	trap_Cvar_SetValue("r_customheight", trap_Cvar_VariableValue("ui_r_customheight"));

	trap_Cvar_VariableStringBuffer( "ui_r_textureMode", buf, MAX_STRING_CHARS );
	trap_Cvar_Set( "r_textureMode", buf );
}

static void UI_ApplySystemSettings() {
	int i;
	for(i = 0; systemDataTableList[i]; i++)
		UI_ApplyBindingTable(systemDataTableList[i], *systemDataTableList_size[i]);
}

static void UI_ReadGeneralSettings() {	
	UI_SetupBindingTable(configDataTable_General, configDataTable_General_Size);
}

static void UI_ApplyGeneralSettings() {	
	UI_ApplyBindingTable(configDataTable_General, configDataTable_General_Size);
}

qboolean KeyBinder_HandleKey(int key, qboolean down) {
	configData_t* configData;

	if(uiInfo.ETF_CurrentBindingTable_Pos < 0 || 
		uiInfo.ETF_CurrentBindingTable_Pos >= uiInfo.ETF_CurrentBindingTable_Size) {
		g_waitingForKey = qfalse;
		return qtrue;
	}

	if (key & K_CHAR_FLAG) {
		return qtrue;
	}

	switch (key)
	{
		case K_ESCAPE:
			g_waitingForKey = qfalse;
			return qtrue;
	
		case K_BACKSPACE:
			return qtrue;

		case '`':
			return qtrue;
	}

	if (key != -1) {
		int i;
		for(i = 0; configDataTableList[i]; i++) {
			int j;
			configData_t* bind = configDataTableList[i];
			for(j = 0; j < *configDataTableList_size[i]; j++, bind++) {
				if(bind->primary == key) {
					bind->primary	= bind->secondary;
					bind->secondary = -1;
				}
				else if(bind->secondary == key) {
					bind->secondary = -1;
				}
			}
		}
	}

	configData = &uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos];
	if(configData->primary != -1) {
		configData->secondary = configData->primary;
	}
	configData->primary = key;

	//set new

//	Controls_SetConfig(qtrue);	
	g_waitingForKey = qfalse;

	return qtrue;
}

static void HUD_BuildPlayerList();

static const char * keyStr[] = {
	"COMMAND",
	"CAPSLOCK",
	"",
	"PAUSE",

	"UPARROW",
	"DOWNARROW",
	"LEFTARROW",
	"RIGHTARROW",

	"ALT",
	"CTRL",
	"SHIFT",
	"INS",
	"DEL",
	"PGDN",
	"PGUP",
	"HOME",
	"END",

	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",

	"KP_HOME",
	"KP_UPARROW",
	"KP_PGUP",
	"KP_LEFTARROW",
	"KP_5",
	"KP_RIGHTARROW",
	"KP_END",
	"KP_DOWNARROW",
	"KP_PGDN",
	"KP_ENTER",
	"KP_INS",
	"KP_DEL",
	"KP_SLASH",
	"KP_MINUS",
	"KP_PLUS",
	"KP_NUMLOCK",
	"KP_STAR",
	"KP_EQUALS",

	"MOUSE1",
	"MOUSE2",
	"MOUSE3",
	"MOUSE4",
	"MOUSE5",

	"MWHEELDOWN",
	"MWHEELUP"
};

static qboolean UI_CheckVersion( void )
{
	static char info[MAX_STRING_CHARS];
	const char *mapname = NULL;
	const char *val = NULL;
	int pure = 0;

	int index = uiInfo.serverStatus.currentServer;
	if( (index < 0) || (index >= uiInfo.serverStatus.numDisplayServers) )
	{	// warning?
		return qfalse;
	}

	trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);

	val = Info_ValueForKey(info, "balancedteams");
	pure = atoi(Info_ValueForKey(info, "maxlives")) != 0;
	if( strcmp(val, FORTS_SHORTVERSION) != 0 && pure )
	{
		Com_Error(ERR_DROP, "This server is NOT running " FORTS_VERSION "!\nJoining this server will not function properly with this version." );
		return qfalse;
	}

	mapname = Info_ValueForKey( info, "mapname");
	if(mapname && *mapname)
	{
		char bspName[MAX_QPATH] = { 0 };
		fileHandle_t fp = NULL_FILE;
		Com_sprintf( bspName, sizeof(bspName), "maps/%s.bsp", mapname );
		if ( trap_FS_FOpenFile( bspName, &fp, FS_READ ) <= 0 ) {
			if( fp != NULL_FILE )
				trap_FS_FCloseFile( fp );
			Com_Error( ERR_DROP, "You don't have the map running on this server: \"%s\"\nCheck with the community to find proper version!", mapname );
			return qfalse;
		}
	}

	return qtrue;
}

static void UI_JoinServer(void) {
	char buff[MAX_STRING_CHARS] = {0};
	trap_Cvar_Set("cg_thirdPerson", "0");
	trap_Cvar_Set("cg_cameraOrbit", "0");
	if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers) {
		trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, sizeof(buff));
		trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
	}
}

static void UI_RunMenuScript(char **args) {
	const char *name;
	char buff[1024];

	if (String_Parse(args, &name)) {
		if (Q_stricmp(name, "StartServer") == 0) {
			int /*i,*/ clients, oldclients;
			//float skill;

			trap_Cvar_Set("g_gameindex", UI_Cvar_VariableString("hud_admingameindex"));
			trap_Cvar_Set("cg_thirdPerson", "0");
			trap_Cvar_Set("cg_cameraOrbit", "0");
			trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
//			trap_Cvar_SetValue( "g_gametype", Com_Clamp( 0, numQ3FGameTypes, ui_netGameType.integer ) );	// RR2DO2
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );
			//skill = trap_Cvar_VariableValue( "g_spSkill" );
			// set max clients based on spots
			oldclients = trap_Cvar_VariableValue( "sv_maxClients" );
			clients = 0;
			if (clients == 0) {
				clients = 8;
			}
			
			if (oldclients > clients) {
				clients = oldclients;
			}

			trap_Cvar_Set("sv_maxClients", va("%d",clients));
		} else if (Q_stricmp(name, "resetDefaults") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
			trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
//			Controls_SetDefaults();
			trap_Cvar_Set("com_introPlayed", "1" );
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else if (Q_stricmp(name, "getCDKey") == 0) {
			char out[17];
			trap_GetCDKey(buff, 17);
			trap_Cvar_Set("cdkey1", "");
			trap_Cvar_Set("cdkey2", "");
			trap_Cvar_Set("cdkey3", "");
			trap_Cvar_Set("cdkey4", "");
			if (strlen(buff) == CDKEY_LEN) {
				Q_strncpyz(out, buff, 5);
				trap_Cvar_Set("cdkey1", out);
				Q_strncpyz(out, buff + 4, 5);
				trap_Cvar_Set("cdkey2", out);
				Q_strncpyz(out, buff + 8, 5);
				trap_Cvar_Set("cdkey3", out);
				Q_strncpyz(out, buff + 12, 5);
				trap_Cvar_Set("cdkey4", out);
			}

		} else if (Q_stricmp(name, "verifyCDKey") == 0) {
			buff[0] = '\0';
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4")); 
			trap_Cvar_Set("cdkey", buff);
			if (trap_VerifyCDKey(buff, UI_Cvar_VariableString("cdkeychecksum"))) {
				trap_Cvar_Set("ui_cdkeyvalid", "CD Key Appears to be valid.");
				trap_SetCDKey(buff);
			} else {
				trap_Cvar_Set("ui_cdkeyvalid", "CD Key does not appear to be valid.");
			}
		} else if (Q_stricmp(name, "loadArenas") == 0) {
			const char *menuname;
//			
			if (String_Parse(args, &menuname) && menuname[0] != ';') {
				if(Q_stricmp(menuname, uiInfo.mapMenuName) != 0) {
					Q_strncpyz(uiInfo.mapMenuName, menuname, MAX_NAME_LENGTH);
					UI_ParseMapInfo();
					uiInfo.ETF_current_mapQuote = -1;
					uiInfo.Q3F_current_classQuote = -1;
					Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, menuname);
				}
			} else {
				Com_Printf("loadArenas script requires menu name parameter\n");
			}
		} else if (Q_stricmp(name, "setmapcvar") == 0) {
			Com_Printf("TODO: setmapcvar\n");
		} else if (Q_stricmp(name, "clearError") == 0) {
			trap_Cvar_Set("com_errorMessage", "");
#ifdef API_ET
			trap_Cvar_Set("com_errorDiagnoseIP", "");
			trap_Cvar_Set("com_missingFiles", "");
#endif // API_ET
		} else if (Q_stricmp(name, "RefreshServers") == 0) {
			UI_StartServerRefresh(qtrue);
			UI_BuildServerDisplayList(qtrue);
		} else if (Q_stricmp(name, "RefreshFilter") == 0) {
			UI_StartServerRefresh( uiInfo.serverStatus.numDisplayServers ? qfalse : qtrue );	// if we don't have any valid servers, it's kinda safe to assume we would like to get a full new list
			UI_BuildServerDisplayList(qtrue);
		} else if (Q_stricmp(name, "LoadDemos") == 0) {
			UI_LoadDemos();
		} else if (Q_stricmp(name, "LoadMovies") == 0) {
			UI_LoadMovies();
		} else if (Q_stricmp(name, "LoadMods") == 0) {
			UI_LoadMods();
		} else if (Q_stricmp(name, "playMovie") == 0) {
			if (uiInfo.previewMovie >= 0) {
			  trap_CIN_StopCinematic(uiInfo.previewMovie);
			}
			trap_Cmd_ExecuteText( EXEC_APPEND, va("cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex]));
		} else if(Q_stricmp(name, "readname") == 0) {
			trap_Cvar_Set("ui_name", UI_Cvar_VariableString("name"));
		} else if(Q_stricmp(name, "setname") == 0) {
			trap_Cvar_Set("name", UI_Cvar_VariableString("ui_name"));
		} else if(Q_stricmp(name, "initClasses") == 0) {
			uiInfo.Q3F_current_classQuote= -1;
			uiInfo.Q3F_current_classQuote_lines = 0;
			Menu_SetFeederSelection(NULL, FEEDER_CLASSINFO, 0, NULL);
		} else if(Q_stricmp(name, "initWeapons") == 0) {
			uiInfo.uiDC.curWeapInt= -1;
			uiInfo.uiDC.weapPreview = -1;
			uiInfo.ETF_current_weapQuote_num = -1;
			uiInfo.ETF_current_weapQuote_lines = 0;
			if(uiInfo.uiDC.lastFeeder) {
				int numsel = Menu_GetFeederSelection(NULL, uiInfo.uiDC.lastFeeder, NULL);
				Menu_SetFeederSelection(NULL, uiInfo.uiDC.lastFeeder, numsel, NULL);
			} else
				Menu_SetFeederSelection(NULL, FEEDER_WEAPONS, 0, NULL);
		} else if(Q_stricmp(name, "resetWeapon") == 0) {
			int listnum;
			if(Int_Parse(args, &listnum)) {
				uiInfo.uiDC.weapPreview = -1;
				uiInfo.ETF_current_weapQuote_num = -1;
				switch(listnum) {
					case FEEDER_ITEMS:
						uiInfo.uiDC.curWeapSource = itemNames;
						uiInfo.uiDC.curWeapInt = Menu_GetFeederSelection(NULL, FEEDER_ITEMS, NULL);
						break;
					case FEEDER_GRENADES:
						uiInfo.uiDC.curWeapSource = grenNames;
						uiInfo.uiDC.curWeapInt = Menu_GetFeederSelection(NULL, FEEDER_GRENADES, NULL);
						break;

					case FEEDER_WEAPONS:
						uiInfo.uiDC.curWeapSource = gunNames;
						uiInfo.uiDC.curWeapInt = Menu_GetFeederSelection(NULL, FEEDER_WEAPONS, NULL);
						break;
				}		
			}
		} else if (Q_stricmp(name, "RunMod") == 0) {
			trap_Cvar_Set( "fs_game", uiInfo.modList[uiInfo.modIndex].modName);
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else if (Q_stricmp(name, "RunDemo") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va("demo \"%s\"\n", uiInfo.demoList[uiInfo.demoIndex]));
		} else if (Q_stricmp(name, "Quake3") == 0) {
			trap_Cvar_Set( "fs_game", "");
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else if (Q_stricmp(name, "closeJoin") == 0) {
			if (uiInfo.serverStatus.refreshActive) {
				UI_StopServerRefresh();
				uiInfo.serverStatus.nextDisplayRefresh = 0;
				uiInfo.nextServerStatusRefresh = 0;
				uiInfo.nextFindPlayerRefresh = 0;
				UI_BuildServerDisplayList(qtrue);
			} else {
				Menus_CloseByName("joinserver");
				Menus_OpenByName("main");
			}
		} else if (Q_stricmp(name, "StopRefresh") == 0) {
			UI_StopServerRefresh();
			uiInfo.serverStatus.nextDisplayRefresh = 0;
			uiInfo.nextServerStatusRefresh = 0;
			uiInfo.nextFindPlayerRefresh = 0;
		} else if (Q_stricmp(name, "UpdateFilter") == 0) {
			trap_Cvar_Update( &ui_netSource );
			if ( ui_netSource.integer == AS_LOCAL || !uiInfo.serverStatus.numDisplayServers ) {
				UI_StartServerRefresh(qtrue);
			}
			UI_BuildServerDisplayList(qtrue);
			uiInfo.numServerPlayers = 0;
		} else if (Q_stricmp(name, "ServerStatus") == 0) {
			trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(qtrue);
		} else if (Q_stricmp(name, "InGameServerStatus") == 0) {
			uiClientState_t cstate;
			trap_GetClientState( &cstate );
			Q_strncpyz(uiInfo.serverStatusAddress, cstate.servername, sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(qtrue);
		} else if (Q_stricmp(name, "FoundPlayerServerStatus") == 0) {
			Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(qtrue);
			Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} else if (Q_stricmp(name, "FindPlayer") == 0) {
			UI_BuildFindPlayerList(qtrue);
			// clear the displayed server status info
			uiInfo.serverStatusInfo.numLines = 0;
			Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} else if (Q_stricmp(name, "JoinServer") == 0) {
			if(UI_CheckVersion())
				UI_JoinServer();
			/*trap_Cvar_Set("cg_thirdPerson", "0");
			trap_Cvar_Set("cg_cameraOrbit", "0");
			if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers) {
				trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024);
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
			}*/
		} else if (Q_stricmp(name, "FoundPlayerJoinServer") == 0) {
			if (uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
			}
		} else if (Q_stricmp(name, "Quit") == 0) {
			trap_Cmd_ExecuteText( EXEC_NOW, "quit\n");
		} else if (Q_stricmp(name, "Controls") == 0) {
		  trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("setup_menu2");
			UI_SetEventHandling(UI_EVENT_NONE);
		} else if (Q_stricmp(name, "Leave") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("main");
			UI_SetEventHandling(UI_EVENT_NONE);
		} else if (Q_stricmp(name, "ServerSort") == 0) {
			int sortColumn;
			if (Int_Parse(args, &sortColumn)) {
				// RR2DO2: hack!
#ifdef API_Q3
				if( sortColumn == 3 )
					sortColumn = 4;
#endif // API_Q3
#ifdef API_ET
				if( sortColumn >= 3 )
					sortColumn += 1;
#endif // API_ET

				// if same column we're already sorting on then flip the direction
				if (sortColumn == uiInfo.serverStatus.sortKey) {
					uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
				}
				// make sure we sort again
				UI_ServersSort(sortColumn, qtrue);
			}
		} else if (Q_stricmp(name, "closeingame") == 0) {
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
			Menus_CloseAll();
			UI_SetEventHandling(UI_EVENT_NONE);
/*		} else if (Q_stricmp(name, "voteMap") == 0) {
			if (ui_currentNetMap.integer >=0 && ui_currentNetMap.integer < uiInfo.mapCount) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote map %s\n",uiInfo.mapList[ui_currentNetMap.integer].mapLoadName) );
			}*/
		} else if (Q_stricmp(name, "voteKick") == 0) {
			if (uiInfo.Q3F_playerindex >= 0 && uiInfo.Q3F_playerindex < uiInfo.playerCount) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote clientkick %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "voteMute") == 0) {
			if (uiInfo.Q3F_playerindex >= 0 && uiInfo.Q3F_playerindex < uiInfo.playerCount) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote mute %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "voteUnMute") == 0) {
			if (uiInfo.Q3F_playerindex >= 0 && uiInfo.Q3F_playerindex < uiInfo.playerCount) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote unmute %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "voteLeader") == 0) {
			if (uiInfo.teamIndex >= 0 && uiInfo.teamIndex < uiInfo.myTeamCount) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callteamvote leader %s\n",uiInfo.teamNames[uiInfo.teamIndex]) );
			}
		} else if (Q_stricmp(name, "addFavorite") == 0) {
			if (ui_netSource.integer != AS_FAVORITES) {
				char sv_name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				sv_name[0] = addr[0] = '\0';
				Q_strncpyz(sv_name, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(sv_name) > 0 && strlen(addr) > 0) {
					res = trap_LAN_AddServer(AS_FAVORITES, sv_name, addr);
					if (res == 0) {
						// server already in the list
						Com_Printf("Favorite already in list\n");
					}
					else if (res == -1) {
						// list full
						Com_Printf("Favorite list full\n");
					}
					else {
						// successfully added
						Com_Printf("Added favorite server %s\n", addr);
					}
				}
			}
		} else if (Q_stricmp(name, "deleteFavorite") == 0) {
			if (ui_netSource.integer == AS_FAVORITES) {
				char addr[MAX_NAME_LENGTH];
				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				addr[0] = '\0';
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(addr) > 0) {
					trap_LAN_RemoveServer(AS_FAVORITES, addr);
				}
			}
		} else if (Q_stricmp(name, "createFavorite") == 0) {
			if (ui_netSource.integer == AS_FAVORITES) {
				char sv_name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				sv_name[0] = addr[0] = '\0';
				Q_strncpyz(sv_name, 	UI_Cvar_VariableString("ui_favoriteName"), MAX_NAME_LENGTH);
				Q_strncpyz(addr, 	UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);
				if (strlen(sv_name) > 0 && strlen(addr) > 0) {
					res = trap_LAN_AddServer(AS_FAVORITES, sv_name, addr);
					if (res == 0) {
						// server already in the list
						Com_Printf("Favorite already in list\n");
					}
					else if (res == -1) {
						// list full
						Com_Printf("Favorite list full\n");
					}
					else {
						// successfully added
						Com_Printf("Added favorite server %s\n", addr);
					}
				}
			}
		} else if (Q_stricmp(name, "orders") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount) {
					strcpy(buff, orders);
					trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				} else {
					int i;
					for (i = 0; i < uiInfo.myTeamCount; i++) {
						if (Q_stricmp(UI_Cvar_VariableString("name"), uiInfo.teamNames[i]) == 0) {
							continue;
						}
						strcpy(buff, orders);
						trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamNames[i]) );
						trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
					}
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
				UI_SetEventHandling(UI_EVENT_NONE);
			}
		} else if (Q_stricmp(name, "voiceOrdersTeam") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer == uiInfo.myTeamCount) {
					trap_Cmd_ExecuteText( EXEC_APPEND, orders );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
				UI_SetEventHandling(UI_EVENT_NONE);
			}
		} else if (Q_stricmp(name, "voiceOrders") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount) {
					strcpy(buff, orders);
					trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
				UI_SetEventHandling(UI_EVENT_NONE);
			}
		} else if (Q_stricmp(name, "glCustom") == 0) {
			trap_Cvar_Set("ui_glCustom", "4");
// RR2DO2
		} else if (Q_stricmp(name, "openFORTBackModels") == 0 ) {	// rotates and zooms the backmodels to show a menu
			if( (uiInfo.Q3F_BackModelStatus == Q3F_BM_CLOSED) || (uiInfo.Q3F_BackModelStatus == Q3F_BM_STARTUP)) {			// closed
				uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;				// opening
				trap_S_StartLocalSound(uiInfo.Q3F_BackModelOpenSound, CHAN_ITEM);
				String_Parse(args, &uiInfo.Q3F_BackModelMenuToOpen);
				Menus_ShowByName( uiInfo.Q3F_BackModelMenuToOpen );
				uiInfo.Q3F_BackModelMenuCurrent = uiInfo.Q3F_BackModelMenuToOpen;
			} else if( uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENED ) {	// opened (close it then open it again)
				String_Parse(args, &uiInfo.Q3F_BackModelMenuToOpen);
				if(Q_stricmp(uiInfo.Q3F_BackModelMenuToOpen, uiInfo.Q3F_BackModelMenuCurrent) != 0) {
					trap_S_StartLocalSound(uiInfo.Q3F_BackModelReOpenSound, CHAN_ITEM);
					Menus_CloseByName( uiInfo.Q3F_BackModelMenuCurrent );
					Menus_ShowByName( uiInfo.Q3F_BackModelMenuToOpen );
					uiInfo.Q3F_BackModelMenuCurrent = uiInfo.Q3F_BackModelMenuToOpen;
					uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;
				}
			}
		} else if (Q_stricmp(name, "closeFORTBackModels") == 0 ) {	// rotates and zooms the backmodels to hide a menu
			if( uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENED ) {			// opened
				uiInfo.Q3F_BackModelStatus = Q3F_BM_CLOSED;				// closing
				trap_S_StartLocalSound(uiInfo.Q3F_BackModelCloseSound, CHAN_ITEM);
				Menus_CloseByName( uiInfo.Q3F_BackModelMenuCurrent );
				uiInfo.Q3F_BackModelMenuCurrent = NULL;
			}
		} else if (Q_stricmp(name, "setFORTBackModelOpenMenu" ) == 0 ) { // sets the current open backmenu, without rotating, requires menu to be open already
			if( uiInfo.Q3F_BackModelStatus == Q3F_BM_OPENED ) {
				//static int staticSound;
				String_Parse(args, &uiInfo.Q3F_BackModelMenuToOpen);
				if( !Q_stricmp( uiInfo.Q3F_BackModelMenuCurrent, uiInfo.Q3F_BackModelMenuToOpen ) ) {
					uiInfo.Q3F_BackModelMenuToOpen = NULL;
					return;
				}
				if( uiInfo.Q3F_BackModelMenuCurrent ) {
					Menus_CloseByName( uiInfo.Q3F_BackModelMenuCurrent );
					uiInfo.Q3F_BackModelMenuCurrent = NULL;
				}
			}
			trap_R_RemapShader( "ui/gfx/content", "ui/gfx/content", va("%f", (float)uiInfo.uiDC.realTime * 0.0001f) );
			uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;
			if( uiInfo.Q3F_BackModelMenuToOpen ) {
				uiInfo.Q3F_BackModelMenuCurrent = uiInfo.Q3F_BackModelMenuToOpen;
				uiInfo.Q3F_BackModelMenuToOpen = NULL;
				trap_S_StartLocalSound(uiInfo.Q3F_BackModelOpenSound, CHAN_ITEM);
				Menus_ShowByName( uiInfo.Q3F_BackModelMenuCurrent );
			}

// RR2DO2
// djbob
		} else if (Q_stricmp(name, "startupFORTBackModels" ) == 0 ) { // sets the current open backmenu, without rotating, requires menu to be open already
			uiInfo.Q3F_BackModelStatus = Q3F_BM_STARTUP;
			uiInfo.Q3F_BackModelStartupTime = 0;
			uiInfo.Q3F_BackModelRotateEndTime = uiInfo.uiDC.realTime;
			trap_S_StartLocalSound(uiInfo.Q3F_BackModelStartupSound, CHAN_ITEM);
		} else if (Q_stricmp(name, "MoveMenuToY" ) == 0 ) { 
			const char *txt1;
			int newY;
			menuDef_t *targetmenu;
			
			if(!String_Parse(args, &txt1))
				return;
			if(!Int_Parse(args, &newY))
				return;

			targetmenu = Menus_FindByName(txt1);
			if(targetmenu)
				Menus_MoveToY(targetmenu, newY);
		} else if (Q_stricmp(name, "setupGameIndexMulti" ) == 0 ) { 
			UI_SetupGameIndexMulti();
		} else if (Q_stricmp(name, "changeGameIndex" ) == 0 ) { 
			UI_ChangeGameIndex();
		} else if (Q_stricmp(name, "readBindings" ) == 0 ) { 
			uiInfo.uiDC.executeText(EXEC_APPEND, "readbindings\n");
			//UI_ReadBindings();
			uiInfo.ETF_CurrentBindingTable_Pos = 0;
			uiInfo.ETF_CurBindMove = 0;
			uiInfo.ETF_CurBindShoot = 0;
			uiInfo.ETF_CurBindMisc = 0;
			uiInfo.ETF_CurBindComs = 0;
			//uiInfo.ETF_CurrentBindingTable = NULL;
		} else if (Q_stricmp(name, "setBindingFeederMode" ) == 0 ) { 
			int mode;
			if(Int_Parse(args, &mode)) {
				uiInfo.ETF_CurrentBindingTable_Pos = 0;
				switch(mode) {
				case BINDING_TABLE_MOVE:
					uiInfo.ETF_CurrentBindingTable		= configDataTable_Move;
					uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Move_Size;
					break;
				case BINDING_TABLE_SHOOT:
					uiInfo.ETF_CurrentBindingTable		= configDataTable_Shoot;
					uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Shoot_Size;
					break;
				case BINDING_TABLE_MISC:
					uiInfo.ETF_CurrentBindingTable		= configDataTable_Misc;
					uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Misc_Size;
					break;
				case BINDING_TABLE_COMS:
					uiInfo.ETF_CurrentBindingTable		= configDataTable_Coms;
					uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Coms_Size;
					break;
				}
			}
		} else if (Q_stricmp(name, "readSystemSettings" ) == 0 ) { 
			UI_ReadSystemSettings();
		} else if (Q_stricmp(name, "ETFreadSystemSettings" ) == 0 ) { 
			UI_ETFReadSystemSettings();
		} else if (Q_stricmp(name, "readGeneralSettings" ) == 0 ) { 
			UI_ReadGeneralSettings();
		} else if (Q_stricmp(name, "setSystemFeederMode" ) == 0 ) { 
			int mode;
			if(Int_Parse(args, &mode)) {
				switch(mode) {
				case SYSTEM_TABLE_GRAPHICS:
					uiInfo.Q3F_CurrentSystemTable		= systemDataTable_Graphics;
					uiInfo.Q3F_CurrentSystemTable_Size	= systemDataTable_Graphics_Size;
					break;
				case SYSTEM_TABLE_DISPLAY:
					uiInfo.Q3F_CurrentSystemTable		= systemDataTable_Display;
					uiInfo.Q3F_CurrentSystemTable_Size	= systemDataTable_Display_Size;
					break;
				case SYSTEM_TABLE_SOUND:
					uiInfo.Q3F_CurrentSystemTable		= systemDataTable_Sound;
					uiInfo.Q3F_CurrentSystemTable_Size	= systemDataTable_Sound_Size;
					break;
				case SYSTEM_TABLE_NETWORK:
					uiInfo.Q3F_CurrentSystemTable		= systemDataTable_Network;
					uiInfo.Q3F_CurrentSystemTable_Size	= systemDataTable_Network_Size;
					break;
				}
			}
		} else if (Q_stricmp(name, "grabKey" ) == 0 ) { 
			if(uiInfo.ETF_CurrentBindingTable_Pos >= 0 && 
				uiInfo.ETF_CurrentBindingTable_Pos < uiInfo.ETF_CurrentBindingTable_Size) {
				
				switch(uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].type) {
				case CONFIG_TYPE_BIND:
					g_waitingForKey = qtrue;
					break;
				}
			}
		} else if (Q_stricmp(name, "grabFeedKey" ) == 0 ) { 
			int feeder;
			if(Int_Parse(args, &feeder)) {
				switch(feeder) {
					case FEEDER_BIND_MOVE :
						uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindMove;
						uiInfo.ETF_CurrentBindingTable		= configDataTable_Move;
						uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Move_Size;
						break;
					case FEEDER_BIND_SHOOT :
						uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindShoot;
						uiInfo.ETF_CurrentBindingTable		= configDataTable_Shoot;
						uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Shoot_Size;
						break;
					case FEEDER_BIND_MISC :
						uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindMisc;
						uiInfo.ETF_CurrentBindingTable		= configDataTable_Misc;
						uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Misc_Size;
						break;
					case FEEDER_BIND_COMS :
						uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindComs;
						uiInfo.ETF_CurrentBindingTable		= configDataTable_Coms;
						uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Coms_Size;
						break;
				}

				if(uiInfo.ETF_CurrentBindingTable_Pos >= 0 && 
					uiInfo.ETF_CurrentBindingTable_Pos < uiInfo.ETF_CurrentBindingTable_Size) {
					
					switch(uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].type) {
					case CONFIG_TYPE_BIND:
						g_waitingForKey = qtrue;
						break;
					}
				}
			}
		} else if (Q_stricmp(name, "clearBinding" ) == 0 ) {
			if(uiInfo.ETF_CurrentBindingTable_Pos >= 0 && 
				uiInfo.ETF_CurrentBindingTable_Pos < uiInfo.ETF_CurrentBindingTable_Size && uiInfo.ETF_CurrentBindingTable) {

				if(	uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].primary != -1) {
					if(uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].primary > K_COMMAND)
						uiInfo.uiDC.executeText(EXEC_NOW, va("unbind %s\n", keyStr[(int)uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].primary - K_COMMAND]));
					else
						uiInfo.uiDC.executeText(EXEC_NOW, va("unbind %c\n", (byte)uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].primary));
					uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].primary = -1;
				}
				if(	uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].secondary != -1) {
					if(uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].secondary > K_COMMAND)
						uiInfo.uiDC.executeText(EXEC_NOW, va("unbind %s\n", keyStr[(int)uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].secondary - K_COMMAND]));
					else
						uiInfo.uiDC.executeText(EXEC_NOW, va("unbind %c\n", (byte)uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].secondary));
					uiInfo.ETF_CurrentBindingTable[uiInfo.ETF_CurrentBindingTable_Pos].secondary = -1;
				}
			}
		} else if (Q_stricmp(name, "applyBindings" ) == 0 ) {
			UI_ApplyBindings();
		} else if (Q_stricmp(name, "saveBindings" ) == 0 ) {
			if(uiInfo.Q3F_ConfigReadMode == CONFIG_READ_NORMAL) {
				uiInfo.uiDC.executeText(EXEC_NOW, va("writeconfig etconfig.cfg\n"));  //keeg rename default file
			}
			else {
				if(uiInfo.Q3F_SelectedClass > 0 && uiInfo.Q3F_SelectedClass < Q3F_CLASS_MAX) {
					bg_q3f_playerclass_t* cls = bg_q3f_classlist[uiInfo.Q3F_SelectedClass];
					uiInfo.uiDC.executeText(EXEC_NOW, va("writeconfig classconfigs/%s.cfg\n", cls->commandstring));
				}
			}
		} else if (Q_stricmp(name, "LoadConfigs") == 0) {
			UI_LoadConfigs();
		} else if (Q_stricmp(name, "LoadConfig") == 0) {
			if (uiInfo.cfgIndex >= 0 && uiInfo.cfgIndex < uiInfo.cfgCount) {
				uiInfo.uiDC.executeText(EXEC_APPEND, va("exec %s.cfg\n", uiInfo.cfgList[uiInfo.cfgIndex]));
			}			
		} else if (Q_stricmp(name, "ExecConfig") == 0) {
			if(uiInfo.Q3F_ConfigReadMode == CONFIG_READ_NORMAL) {
				uiInfo.uiDC.executeText(EXEC_NOW, va("exec etconfig.cfg\n"));  //keeg rename default file
			}
			else {
				if(uiInfo.Q3F_SelectedClass > 0 && uiInfo.Q3F_SelectedClass < Q3F_CLASS_MAX) {
					bg_q3f_playerclass_t* cls = bg_q3f_classlist[uiInfo.Q3F_SelectedClass];
					uiInfo.uiDC.executeText(EXEC_NOW, va("exec classconfigs/%s.cfg\n", cls->commandstring));
				}
			}
		} else if (Q_stricmp(name, "setConfigReadMode" ) == 0 ) { 
			int mode;
			if(Int_Parse(args, &mode)) {				
				uiInfo.Q3F_ConfigReadMode = mode;
			}
		} else if (Q_stricmp(name, "setClassConfigNum" ) == 0 ) {
			int classnum;
			if(Int_Parse(args, &classnum)) {
				uiInfo.Q3F_SelectedClass = classnum;
			}
			else
				Com_Printf("Warning: setClassConfigNum requires a parameter\n");
		} else if (Q_stricmp(name, "applySystemSettings" ) == 0 ) {
			UI_ApplySystemSettings();
		} else if (Q_stricmp(name, "ETFapplySystemSettings" ) == 0 ) {
			UI_ETFApplySystemSettings();
		} else if (Q_stricmp(name, "applyGeneralSettings" ) == 0 ) {
			UI_ApplyGeneralSettings();
		} else if (Q_stricmp(name, "removeFavServer" ) == 0 ) {
			if(uiInfo.Q3F_CurrentFavServer_Pos >= 0 && uiInfo.Q3F_CurrentFavServer_Pos < trap_LAN_GetServerCount( AS_FAVORITES )) {
				char addr[MAX_NAME_LENGTH];
				//char buff[MAX_STRING_CHARS];
				trap_LAN_GetServerInfo( AS_FAVORITES, uiInfo.Q3F_CurrentFavServer_Pos, buff, sizeof(buff));

				addr[0] = '\0';
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (*addr) {
					trap_LAN_RemoveServer(AS_FAVORITES, addr);
				}
			}
		} else if (Q_stricmp(name, "addFavServer" ) == 0 ) {
			char sv_name[MAX_NAME_LENGTH];
			char addr[MAX_NAME_LENGTH];
			int res;

			sv_name[0] = addr[0] = '\0';
			Q_strncpyz(sv_name, 	UI_Cvar_VariableString("ui_favServerName"), MAX_NAME_LENGTH);
			Q_strncpyz(addr, 	UI_Cvar_VariableString("ui_favServerAddress"), MAX_NAME_LENGTH);
			if (*sv_name && *addr) {
				res = trap_LAN_AddServer(AS_FAVORITES, sv_name, addr);
				if (res == 0) {
					// server already in the list
					Com_Printf("Favorite already in list\n");
				}
				else if (res == -1) {
					// list full
					Com_Printf("Favorite list full\n");
				}
				else {
					// successfully added
					Com_Printf("Added favorite server %s\n", addr);
				}
			}
		} else if (Q_stricmp(name, "restartVideo" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else if (Q_stricmp(name, "setupCDKey" ) == 0 ) {
			char out[17];
			trap_GetCDKey(out, 17);
			trap_Cvar_Set("cdkey", out);
		} else if (Q_stricmp(name, "acceptClass" ) == 0 ) {
			int cls = HUD_Q3F_GetChosenClass();
			bg_q3f_playerclass_t* pcls = bg_q3f_classlist[cls+1];

			if(hud_allowClasses.string[cls] != '1')  {
				return;
			}

			uiInfo.Q3F_BackModelStatus = Q3F_BM_CLOSED;				// closing
			trap_S_StartLocalSound(uiInfo.Q3F_BackModelCloseSound, CHAN_ITEM);
			Menus_CloseByName( uiInfo.Q3F_BackModelMenuCurrent );
			uiInfo.Q3F_BackModelMenuCurrent = NULL;

			Menus_CloseByName("ingame");

			trap_Cmd_ExecuteText( EXEC_APPEND, va("%s\n", pcls->commandstring) );
			
		} else if (Q_stricmp(name, "getClassInfo" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "ClassinfoRequest\n" );
		} else if (Q_stricmp(name, "buildPlayerList" ) == 0 ) {
			HUD_BuildPlayerList();
			
			if(trap_Cvar_VariableValue("hud_rcon_auth") || trap_Cvar_VariableValue("hud_admin_auth")) {
				trap_Cmd_ExecuteText(EXEC_APPEND, "admin2 iplist\n");
				trap_Cmd_ExecuteText(EXEC_APPEND, "admin2 banlist\n");
			}
		} else if (Q_stricmp(name, "say") == 0) {
			const char *txt1, *txt2, *txt3, *dummy;
			if(!String_Parse(args, &txt1)) {
				return;
			}
			if(!String_Parse(args, &dummy)) {
				return;
			}
			if(!String_Parse(args, &txt2)) {
				return;
			}
			if(!String_Parse(args, &dummy)) {
				return;
			}
			if(!String_Parse(args, &txt3)) {
				return;
			}

			trap_Cmd_ExecuteText( EXEC_APPEND, va("say \"%s\" \"%s\" \"%s\"\n", txt1, txt2, txt3));
		} else if (Q_stricmp(name, "say_team") == 0) {
			const char *txt1, *txt2, *txt3, *dummy;
			if(!String_Parse(args, &txt1)) {
				return;
			}
			if(!String_Parse(args, &dummy)) {
				return;
			}
			if(!String_Parse(args, &txt2)) {
				return;
			}
			if(!String_Parse(args, &dummy)) {
				return;
			}
			if(!String_Parse(args, &txt3)) {
				return;
			}

			trap_Cmd_ExecuteText( EXEC_APPEND, va("say_team \"%s\" \"%s\" \"%s\"\n", txt1, txt2, txt3));
		} else if (Q_stricmp(name, "JoinSpecifiedServer") == 0) {
			// slothy
			// check for save-as-favourites check
			if(ui_addSpecifyFavorites.integer) {
				char sv_name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				Q_strncpyz(sv_name, UI_Cvar_VariableString("ui_favServerName"), MAX_NAME_LENGTH);
				if(!strlen(sv_name)) {
					Q_strncpyz(sv_name, "QuickConnect", MAX_NAME_LENGTH);
				}
				Q_strncpyz(addr, ui_specifyServer.string, MAX_NAME_LENGTH);
				if(strlen(ui_specifyPort.string)) {
					Q_strcat(addr, MAX_NAME_LENGTH, ":");
					Q_strcat(addr, MAX_NAME_LENGTH, ui_specifyPort.string);
				}
				res = trap_LAN_AddServer(AS_FAVORITES, sv_name, addr);
				if (res == 0) {
					// server already in the list
					Com_Printf("Favorite already in list\n");
				}
				else if (res == -1) {
					// list full
					Com_Printf("Favorite list full\n");
				}
				else {
					// successfully added
					Com_Printf("Added favorite server %s\n", addr);
				}
			}

			trap_Cmd_ExecuteText( EXEC_APPEND, va("connect %s:%s\n", ui_specifyServer.string, ui_specifyPort.string));
		} else if (Q_stricmp(name, "AuthorizeClient") == 0) {
			char pass1[128], pass2[128];

			trap_Cvar_VariableStringBuffer("hud_adminpassword", pass1, 128);
			trap_Cvar_VariableStringBuffer("hud_rconpassword", pass2, 128);

			if(*pass1) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("adminpassword %s\n", pass1));
			}
			if(*pass2) {
				uiClientState_t cstate;
				trap_GetClientState( &cstate );

				trap_Cvar_Set("rconAddress", cstate.servername);
				trap_Cvar_Set("rconPassword", pass2);

				trap_Cmd_ExecuteText( EXEC_APPEND, va("authrc %s\n", pass2));
			}
		} else if (Q_stricmp(name, "ExecRconCommand") == 0) {
			char command[256];
			trap_Cvar_VariableStringBuffer("hud_rconcommand", command, 256);

			if(trap_Cvar_VariableValue("hud_rcon_auth")) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("rcon %s\n", command));
			} else if(trap_Cvar_VariableValue("hud_admin_auth")) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin %s\n", command));
			}
		} else if (Q_stricmp(name, "AdminSetTimelimit") == 0) {
			char timelimit[128];

			trap_Cvar_VariableStringBuffer("hud_admintimelimit", timelimit, 128);
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin timelimit %s\n", timelimit));
		} else if (Q_stricmp(name, "AdminMapChange") == 0) {
			char command[256], gameindex[16];
			
			trap_Cvar_VariableStringBuffer("hud_adminmap", command, 256);
			trap_Cvar_VariableStringBuffer("hud_admingameindex", gameindex, 16);

			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin gameindex %s\n", gameindex));
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin map %s\n", command));
		} else if (Q_stricmp(name, "AdminStartMatch") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin match start%s%s\n",
				trap_Cvar_VariableValue("hud_admin_matchmode") != 0 ? " matchmode" : "",
				trap_Cvar_VariableValue("hud_admin_match_noreadyup") != 0 ? " noreadyup" : "" ));
		} else if (Q_stricmp(name, "AdminEndMatch") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "admin match end\n");
		} else if (Q_stricmp(name, "AdminForceReady") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "admin match forceready\n");
		} else if (Q_stricmp(name, "AdminSetWarmup") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin match warmup %d\n", (int)trap_Cvar_VariableValue("hud_adminmatchwarmup")));
		} else if (Q_stricmp(name, "AdminMapChangeList") == 0) {
			if(ui_mapIndex.integer >= 0 && ui_mapIndex.integer < uiInfo.mapCount) {
				char buffer[64];
				int gindex;

				Q_strncpyz(buffer, uiInfo.mapList[ui_mapIndex.integer].mapLoadName, 64);
				gindex = (int)trap_Cvar_VariableValue("g_gameindex");

				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin gameindex %i\n", gindex));
				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin map %s\n", buffer));	
			}
		} else if (Q_stricmp(name, "AdminMapVote") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "admin map vote\n");
		} else if (Q_stricmp(name, "AdminCeaseFire") == 0) {
			const char* command;
			if(String_Parse(args, &command)) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin ceasefire %s\n", command));
			}
		} else if (Q_stricmp(name, "AdminKick") == 0) {
			if(uiInfo.Q3F_playerindex >= 0 && (uiInfo.Q3F_playerindex < uiInfo.Q3F_playercount)) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("rcon clientkick %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "AdminMute") == 0) {
			if(uiInfo.Q3F_playerindex >= 0 && (uiInfo.Q3F_playerindex < uiInfo.Q3F_playercount)) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("rcon mute %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "AdminUnMute") == 0) {
			if(uiInfo.Q3F_playerindex >= 0 && (uiInfo.Q3F_playerindex < uiInfo.Q3F_playercount)) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("rcon unmute %d\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex]));
			}
		} else if (Q_stricmp(name, "AdminWarn") == 0) {
			if(uiInfo.Q3F_playerindex >= 0 && (uiInfo.Q3F_playerindex < uiInfo.Q3F_playercount)) {
				char reason[256];
				trap_Cvar_VariableStringBuffer("hud_adminbanmsg", reason, 256);
				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin warn %d \"%s\"\n", uiInfo.Q3F_clientNumber[uiInfo.Q3F_playerindex], reason));
			}
		} else if (Q_stricmp(name, "AdminBAN") == 0) {
			if(uiInfo.Q3F_playerindex >= 0 && (uiInfo.Q3F_playerindex < uiInfo.Q3F_playerIPcount)) {
				int bantime = (int)trap_Cvar_VariableValue("hud_adminbantime");
				char reason[256];

				trap_Cvar_VariableStringBuffer("hud_adminbanmsg", reason, 256);

				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin addip %s %d \"%s\"\n", uiInfo.Q3F_playerIPs[uiInfo.Q3F_playerindex], bantime, reason));
			}
			else
				Com_Printf("Select a player to ban\n");
		} else if (Q_stricmp(name, "AdminRemove") == 0) {			
			if(uiInfo.Q3F_playerBANindex >= 0 && (uiInfo.Q3F_playerBANindex < uiInfo.Q3F_playerBANcount)) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va("admin removeip %s\n", uiInfo.Q3F_playerBANs[uiInfo.Q3F_playerBANindex]));
			}
			else
				Com_Printf("Select an ip to unban first\n");
		} else if (Q_stricmp(name, "BotMinMax") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin bot maxbots %d\n", (int)trap_Cvar_VariableValue("hud_botmax")));
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin bot minbots %d\n", (int)trap_Cvar_VariableValue("hud_botmin")));
		} 
		else if (Q_stricmp(name, "BotAdd") == 0) {
			char botname[128];
			trap_Cvar_VariableStringBuffer("hud_addbotname", botname, 128);
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin bot addbot %s %d %d\n", botname, (int)trap_Cvar_VariableValue("hud_botteam") + 1, (int)trap_Cvar_VariableValue("hud_botclass") + 1));
		} 
		else if (Q_stricmp(name, "BotKick") == 0) {
			char botname[128];
			trap_Cvar_VariableStringBuffer("hud_botname", botname, 128);
			trap_Cmd_ExecuteText( EXEC_APPEND, va("admin bot kickbot %s\n", botname));
		} 
		else if (Q_stricmp(name, "ExecText") == 0) {
			const char* command;
			String_Parse(args, &command);
			
			trap_Cmd_ExecuteText( EXEC_APPEND, va("%s\n", command));
		} 
		else if (Q_stricmp(name, "VoteMap") == 0) {
			if(ui_mapIndex.integer >= 0 && ui_mapIndex.integer < uiInfo.mapCount) {
//				char *p;
				char buffer[64];
				int gindex;

				Q_strncpyz(buffer, uiInfo.mapList[ui_mapIndex.integer].mapLoadName, 64);
				gindex = (int)trap_Cvar_VariableValue("g_gameindex");
//				p = strchr(buffer, '+');
//				*p++ = '\0';

				trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote map %s+%i\n", buffer, gindex));
			}
		} else if (Q_stricmp(name, "VoteTimelimit") == 0) {
			char buffer[64];

			trap_Cvar_VariableStringBuffer("hud_voteTimelimit", buffer, 64);

			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote timelimit %s\n", buffer));
		} else if (Q_stricmp(name, "VoteFraglimit") == 0) {
			char buffer[64];

			trap_Cvar_VariableStringBuffer("hud_voteFraglimit", buffer, 64);

			trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote fraglimit %s\n", buffer));
		} else if (Q_stricmp(name, "SendUIChat") == 0) {
			char buffer[MAX_SAY_TEXT];

			trap_Cvar_VariableStringBuffer("hud_uichat", buffer, MAX_SAY_TEXT);

			if(*buffer) {
				trap_Cmd_ExecuteText(EXEC_APPEND, va("say \"%s\"\n", buffer));
			}

			trap_Cvar_Set("hud_uichat", "");
// djbob
// slothy
		} else if (Q_stricmp(name, "FindHuds") == 0) {
			uiInfo.ETF_current_hudNameQuote = -1;
			uiInfo.ETF_current_hudVarQuote = -1;
			UI_ParseHudInfo();

			// possibly also set feeder selection
			Menu_SetFeederSelection(NULL, FEEDER_HUDS, uiInfo.uiDC.curHudInt, NULL);
			Menu_SetFeederSelection(NULL, FEEDER_HUDSVARIANT, uiInfo.uiDC.curHudVarInt, NULL);
		} else if (Q_stricmp(name, "applyHud") == 0) {
			uiClientState_t cstate;
			char hudname[256];

			if(strlen(uiInfo.uiDC.curHud) && strlen(uiInfo.uiDC.curHudVariant)) {
				Com_sprintf(hudname, 256, "%s/%s/%s.menu", HUDINFODIR, uiInfo.uiDC.curHud, uiInfo.uiDC.curHudVariant);
				trap_Cvar_Set("cg_hudfiles", hudname);

				trap_GetClientState( &cstate );
				if(cstate.connState == CA_ACTIVE)
					trap_Cmd_ExecuteText(EXEC_APPEND, "loadhud\n");
			}
		} else if (Q_stricmp(name, "reconnect") == 0) {
			// TODO: if dumped because of cl_allowdownload problem, toggle on first (we don't have appropriate support for this yet)
			trap_Cmd_ExecuteText( EXEC_APPEND, "reconnect\n" );
		} else if (Q_stricmp(name, "resetmenu") == 0) {
#ifdef CGAME
			CG_Q3F_MenuCancel(qfalse);
#endif
		} else if (Q_stricmp(name, "playGrenSound") == 0) {
			char buffer[1024];
			trap_Cvar_VariableStringBuffer( "cg_grenadePrimeSound", buffer, 1024 );
			if(buffer[0])
				trap_S_StartLocalSound( trap_S_RegisterSound(buffer, qfalse) , CHAN_LOCAL_SOUND );
		} else if (Q_stricmp(name, "setMem") == 0) {
			int memval;
			memval = (int)trap_Cvar_VariableValue( "ui_memsize" );
			switch(memval) {
				case 1:		// 256 - 512MB
					trap_Cvar_Set("com_hunkmegs", "192");
					trap_Cvar_Set("com_zonemegs", "32");
					trap_Cvar_Set("com_soundmegs", "24");
					break;
				case 2:		// 512MB +
					trap_Cvar_Set("com_hunkmegs", "256");
					trap_Cvar_Set("com_zonemegs", "64");
					trap_Cvar_Set("com_soundmegs", "32");
					break;
				default:	// < 256
//					trap_Cvar_Set("ui_memsize", "0");
					trap_Cvar_Set("com_hunkmegs", "128");
					trap_Cvar_Set("com_zonemegs", "24");
					trap_Cvar_Set("com_soundmegs", "24");
					break;
			}
		} else if (Q_stricmp(name, "setNetwork") == 0) {
			int netspeed;
			netspeed = (int)trap_Cvar_VariableValue( "ui_netspeed" );
			switch(netspeed) {
				case 1:		// isdn
					trap_Cvar_Set("rate", "5000");
					trap_Cvar_Set("cl_maxpackets", "40");
					trap_Cvar_Set("snaps", "30");
					break;
				case 2:		// lan/cable/dsl
					trap_Cvar_Set("rate", "25000");
					trap_Cvar_Set("cl_maxpackets", "60");
					trap_Cvar_Set("snaps", "40");
					break;
				default:	// dialup modem
//					trap_Cvar_Set("ui_netspeed", 0);
					trap_Cvar_Set("rate", "3500");
					trap_Cvar_Set("cl_maxpackets", "30");
					trap_Cvar_Set("snaps", "20");
					break;
			}
		} else {
			Com_Printf("unknown UI script %s\n", name);
		}
	}
}

static void UI_GetTeamColor(vec4_t *color) {
	Com_Printf("Oops!\n");
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position) {
	int i;

	if (position < 0 || position > uiInfo.serverStatus.numDisplayServers ) {
		return;
	}
	//
	uiInfo.serverStatus.numDisplayServers++;
	for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--) {
		uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i-1];
	}
	uiInfo.serverStatus.displayServers[position] = num;

	// update displayed levelshot
	if ( position == uiInfo.serverStatus.currentServer ) {
		UI_FeederSelection( FEEDER_SERVERS, uiInfo.serverStatus.currentServer );
	}
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num) {
	int i, j;

	for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++) {
		if (uiInfo.serverStatus.displayServers[i] == num) {
			uiInfo.serverStatus.numDisplayServers--;
			for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++) {
				uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j+1];
			}
			return;
		}
	}
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num) {
	int mid, offset, res, len;

	// use binary search to insert server
	len = uiInfo.serverStatus.numDisplayServers;
	mid = len;
	offset = 0;
	res = 0;
	while(mid > 0) {
		mid = len >> 1;
		//
		res = trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
					uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset+mid]);
		// if equal
		if (res == 0) {
			UI_InsertServerIntoDisplayList(num, offset+mid);
			return;
		}
		// if larger
		else if (res == 1) {
			offset += mid;
			len -= mid;
		}
		// if smaller
		else {
			len -= mid;
		}
	}
	if (res == 1) {
		offset++;
	}
	UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(qboolean force) {
	int i, count, clients, maxClients, ping/*, game*/, len, visible, passw;
	char *val;
#ifdef API_ET
//	int punkbuster, antilag;
#endif // API_ET
	char info[MAX_STRING_CHARS];
//	qboolean startRefresh = qtrue;
	static int numinvisible;

	if (!(force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh)) {
		return;
	}
	// if we shouldn't reset
	if ( force == 2 ) {
		force = 0;
	}

	// do motd updates here too
	trap_Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof(uiInfo.serverStatus.motd) );
	len = strlen(uiInfo.serverStatus.motd);
	if (len == 0) {
		strcpy(uiInfo.serverStatus.motd, "Welcome to ETF!");
		len = strlen(uiInfo.serverStatus.motd);
	} 
	if (len != uiInfo.serverStatus.motdLen) {
		uiInfo.serverStatus.motdLen = len;
		uiInfo.serverStatus.motdWidth = -1;
	} 

	if (force) {
		numinvisible = 0;
		// clear number of displayed servers
		uiInfo.serverStatus.numqueriedservers = 0;
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.maxservers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		uiInfo.serverStatus.numTotalPlayers = 0;
		uiInfo.serverStatus.nextpingtime = 0;
		// set list box index to zero
		Menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
		// mark all servers as visible so we store ping updates for them
		trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
	}

	// get the server count (comes from the master)
	count = trap_LAN_GetServerCount(ui_netSource.integer);
	if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0) ) {
		// still waiting on a response from the master
		uiInfo.serverStatus.numqueriedservers = 0;
		uiInfo.serverStatus.maxservers = 0;
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		uiInfo.serverStatus.numTotalPlayers = 0;
		uiInfo.serverStatus.nextpingtime = 0;
		uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
		return;
	}

	trap_Cvar_Update( &ui_browserShowEmpty );
	trap_Cvar_Update( &ui_browserShowFull );
	trap_Cvar_Update( &ui_browserShowPasswordProtected );
	trap_Cvar_Update( &ui_browserShowVersion );

	uiInfo.serverStatus.maxservers = count;
	visible = qfalse;
	for (i = 0; i < count; i++) {
		// if we already got info for this server
		if (!trap_LAN_ServerIsVisible(ui_netSource.integer, i)) {
			continue;
		}

		visible = qtrue;
		// get the ping for this server
		ping = trap_LAN_GetServerPing(ui_netSource.integer, i);
		if (ping > 0 || ui_netSource.integer == AS_FAVORITES) {

			uiInfo.serverStatus.numqueriedservers++;

			trap_LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

			clients = atoi(Info_ValueForKey(info, "clients"));
			uiInfo.serverStatus.numTotalPlayers += clients;

			if ( ui_browserShowEmpty.integer == 0 ) {
				if ( clients == 0 ) {
					trap_LAN_MarkServerVisible( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( ui_browserShowFull.integer == 0 ) {
				maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
				if (clients == maxClients) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}

			if ( ui_browserShowPasswordProtected.integer == 0 ) {
				passw = atoi(Info_ValueForKey(info, "needpass"));
				if (passw && !ui_browserShowPasswordProtected.integer) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}
	// weaprestrict
			if ( ui_browserShowVersion.integer == 0 ) {
				val = Info_ValueForKey(info, "balancedteams");				// version check, stupid int no decimals allowed
				if(strcmp(val, FORTS_SHORTVERSION))							// only show servers of this ETF version
				{
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);	
					continue;
				}
			}

#ifdef API_ET
			/*trap_Cvar_Update( &ui_browserShowPasswordProtected );
			if( ui_browserShowPasswordProtected.integer ) {
				password = atoi(Info_ValueForKey( info, "needpass" ) );
				if( ( password && ui_browserShowPasswordProtected.integer == 2 ) ||
					( !password && ui_browserShowPasswordProtected.integer == 1 ) ) {
					trap_LAN_MarkServerVisible( ui_netSource.integer, i, qfalse );
					continue;
				}
			}*/

			/*trap_Cvar_Update( &ui_browserShowPunkBuster );
			if ( ui_browserShowPunkBuster.integer ) {
				punkbuster = atoi(Info_ValueForKey(info, "punkbuster"));

				if( ( punkbuster && ui_browserShowPunkBuster.integer == 2 ) ||
					( !punkbuster && ui_browserShowPunkBuster.integer == 1 ) ) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}*/
#endif //API_ET

			// RR2DO2: modified this
/*			trap_Cvar_Update( &ui_joinGameType );
			if ( ui_joinGameType.integer != 0) {
				game = atoi(Info_ValueForKey(info, "gametype"));
				if (game != ui_joinGameType.integer) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}*/
				
			/*trap_Cvar_Update( &ui_serverFilterType );
			if (ui_serverFilterType.integer > 0) {
				if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}*/

			// RR2DO2
			if ( Q_stricmp( Info_ValueForKey( info, "game" ), GAME_VERSION ) != 0 ) {
				trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
				continue;
			}
			// RR2DO2

			// make sure we never add a favorite server twice
			if (ui_netSource.integer == AS_FAVORITES) {
				UI_RemoveServerFromDisplayList(i);
			}

			// insert the server into the list
			UI_BinaryServerInsertion(i);
			// done with this server
			if ( ping > 0 ) {
				trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
				uiInfo.serverStatus.numPlayersOnServers += clients;
				numinvisible++;
			}
		}
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;

	// if there were no servers visible for ping updates
	if (!visible) {
		UI_StopServerRefresh();
		uiInfo.serverStatus.nextDisplayRefresh = 0;
	}
}

typedef struct
{
	char *name, *altName;
} serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = {
	{"sv_hostname", "Name"},
	{"Address", ""},
	{"gamename", "Game name"},
	{"g_gametype", "Game type"},
	{"mapname", "Map"},
	{"version", ""},
	{"protocol", ""},
	{"timelimit", ""},
	{"fraglimit", ""},
	{NULL, NULL}
};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t *info ) {
	int i, j, index;
	char *tmp1, *tmp2;

	index = 0;
	for (i = 0; serverStatusCvars[i].name; i++) {
		for (j = 0; j < info->numLines; j++) {
			if ( !info->lines[j][1] || info->lines[j][1][0] ) {
				continue;
			}
			if ( !Q_stricmp(serverStatusCvars[i].name, info->lines[j][0]) ) {
				// swap lines
				tmp1 = info->lines[index][0];
				tmp2 = info->lines[index][3];
				info->lines[index][0] = info->lines[j][0];
				info->lines[index][3] = info->lines[j][3];
				info->lines[j][0] = tmp1;
				info->lines[j][3] = tmp2;
				//
				if ( strlen(serverStatusCvars[i].altName) ) {
					info->lines[index][0] = serverStatusCvars[i].altName;
				}
				index++;
			}
		}
	}
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo( const char *serverAddress, serverStatusInfo_t *info ) {
	char *p, *score, *ping, *name;
	int i, len;

	if (info) {
		memset(info, 0, sizeof(*info));
	}

	// ignore initial unset addresses
	if (serverAddress && *serverAddress == '\0') {
		return qfalse;
	}

	// reset server status request for this address
	if (!info) {
		trap_LAN_ServerStatus( serverAddress, NULL, 0);
		return qfalse;
	}

	if ( trap_LAN_ServerStatus( serverAddress, info->text, sizeof(info->text)) ) {
		Q_strncpyz(info->address, serverAddress, sizeof(info->address));
		p = info->text;
		info->numLines = 0;
		info->lines[info->numLines][0] = "Address";
		info->lines[info->numLines][1] = "";
		info->lines[info->numLines][2] = "";
		info->lines[info->numLines][3] = info->address;
		info->numLines++;
		// get the cvars
		while (p && *p) {
			p = strchr(p, '\\');
			if (!p)
				break;
			*p++ = '\0';
			if (*p == '\\')
				break;
			info->lines[info->numLines][0] = p;
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			p = strchr(p, '\\');
			if (!p)
				break;
			*p++ = '\0';
			info->lines[info->numLines][3] = p;

			info->numLines++;
			if (info->numLines >= MAX_SERVERSTATUS_LINES)
				break;
		}
		// get the player list
		if (info->numLines < MAX_SERVERSTATUS_LINES-3) {
			// empty line
			info->lines[info->numLines][0] = "";
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			info->lines[info->numLines][3] = "";
			info->numLines++;
			// header
			info->lines[info->numLines][0] = "num";
			info->lines[info->numLines][1] = "score";
			info->lines[info->numLines][2] = "ping";
			info->lines[info->numLines][3] = "name";
			info->numLines++;
			// parse players
			i = 0;
			len = 0;
			while (p && *p) {
				if (*p == '\\')
					*p++ = '\0';
				if (!p)
					break;
				score = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				ping = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				name = p;
				Com_sprintf(&info->pings[len], sizeof(info->pings)-len, "%d", i);
				info->lines[info->numLines][0] = &info->pings[len];
				len += strlen(&info->pings[len]) + 1;
				info->lines[info->numLines][1] = score;
				info->lines[info->numLines][2] = ping;
				info->lines[info->numLines][3] = name;
				info->numLines++;
				if (info->numLines >= MAX_SERVERSTATUS_LINES)
					break;
				p = strchr(p, '\\');
				if (!p)
					break;
				*p++ = '\0';
				//
				i++;
			}
		}
		UI_SortServerStatusInfo( info );
		return qtrue;
	}
	return qfalse;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset) {
	int i;

	while(*str) {
		for (i = 0; charset[i] && str[i]; i++) {
			if (toupper(charset[i]) != toupper(str[i])) break;
		}
		if (!charset[i]) return str;
		str++;
	}
	return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(qboolean force) {
	static int numFound, numTimeOuts;
	int i, j, resend;
	serverStatusInfo_t info;
	char name[MAX_NAME_LENGTH+2];
	char infoString[MAX_STRING_CHARS];

	if (!force) {
		if (!uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime) {
			return;
		}
	}
	else {
		memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
		uiInfo.numFoundPlayerServers = 0;
		uiInfo.currentFoundPlayerServer = 0;
		trap_Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof(uiInfo.findPlayerName));
		Q_CleanStr(uiInfo.findPlayerName);
		// should have a string of some length
		if (!strlen(uiInfo.findPlayerName)) {
			uiInfo.nextFindPlayerRefresh = 0;
			return;
		}
		// set resend time
		resend = ui_serverStatusTimeOut.integer / 2 - 10;
		if (resend < 50) {
			resend = 50;
		}
		trap_Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0);
		//
		uiInfo.numFoundPlayerServers = 1;
		Com_sprintf(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1],
						sizeof(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1]),
							"searching %d...", uiInfo.pendingServerStatus.num);
		uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1][0] = '\0';
		numFound = 0;
		numTimeOuts++;
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		// if this pending server is valid
		if (uiInfo.pendingServerStatus.server[i].valid) {
			// try to get the server status for this server
			if (UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) ) {
				//
				numFound++;
				// parse through the server status lines
				for (j = 0; j < info.numLines; j++) {
					// should have ping info
					if ( !info.lines[j][2] || !info.lines[j][2][0] ) {
						continue;
					}
					// clean string first
					Q_strncpyz(name, info.lines[j][3], sizeof(name));
					Q_CleanStr(name);

					if(!Q_stricmp(name, "name"))
						continue;
					// dont check the name etc line


					// if the player name is a substring
					if (stristr(name, uiInfo.findPlayerName)) {
						// add to found server list if we have space (always leave space for a line with the number found)
						if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS-1) {
							//
							Q_strncpyz(uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].adrstr,
											sizeof(uiInfo.foundPlayerServerAddresses[0]));
							Q_strncpyz(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].name,
											sizeof(uiInfo.foundPlayerServerNames[0]));
							Q_strncpyz(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1],
										name,
											sizeof(uiInfo.foundPlayerNames[0]));
							uiInfo.numFoundPlayerServers++;
						}
						else {
							// can't add any more so we're done
							uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
						}
					}
				}
				Com_sprintf(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", numFound, uiInfo.pendingServerStatus.num);
				uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1][0] = '\0';
				// retrieved the server status so reuse this spot
				uiInfo.pendingServerStatus.server[i].valid = qfalse;
			}
		}
		// if empty pending slot or timed out
		if (!uiInfo.pendingServerStatus.server[i].valid ||
			uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer) {
			if (uiInfo.pendingServerStatus.server[i].valid) {
				numTimeOuts++;
			}
			// reset server status request for this address
			UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
			// reuse pending slot
			uiInfo.pendingServerStatus.server[i].valid = qfalse;
			// if we didn't try to get the status of all servers in the main browser yet
			if (uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers) {
				uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
				trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
							uiInfo.pendingServerStatus.server[i].adrstr, sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof(infoString));
				Q_strncpyz(uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey(infoString, "hostname"), sizeof(uiInfo.pendingServerStatus.server[0].name));
				uiInfo.pendingServerStatus.server[i].valid = qtrue;
				uiInfo.pendingServerStatus.num++;
				Com_sprintf(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", numFound, uiInfo.pendingServerStatus.num);
				uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1][0] = '\0';
			}
		}
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if (uiInfo.pendingServerStatus.server[i].valid) {
			break;
		}
	}
	// if still trying to retrieve server status info
	if (i < MAX_SERVERSTATUSREQUESTS) {
		uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
	}
	else {
		// add a line that shows the number of servers found
		if (!uiInfo.numFoundPlayerServers) {
			Com_sprintf(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
			uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1][0] = '\0';
		}
		else {
			Com_sprintf(uiInfo.foundPlayerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]),
						"%d server%s found.", uiInfo.numFoundPlayerServers-1,
						uiInfo.numFoundPlayerServers == 2 ? "":"s");
			uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1][0] = '\0';  
		}
		uiInfo.nextFindPlayerRefresh = 0;
		// show the server status info for the selected server
		UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
	}
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(qboolean force) {
	uiClientState_t cstate;
	trap_GetClientState( &cstate );

	if (uiInfo.nextFindPlayerRefresh) {
		return;
	}
	if (!force) {
		if (!uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime) {
			return;
		}
	}
	else {
		Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
		uiInfo.serverStatusInfo.numLines = 0;
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0);
	}
	if(cstate.connState < CA_CONNECTED) {
		if (uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0) {
			return;
		}
	}
	if (UI_GetServerStatusInfo( uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo ) ) {
		uiInfo.nextServerStatusRefresh = 0;
		UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
	}
	else {
		uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
	}
}

static void UI_BuildServerPlayerList(qboolean force) {
	char buf[4096];
	char *p, *score, *ping, *name;

	if (!force) {
		if (!uiInfo.nextServerPlayerRefresh || uiInfo.nextServerPlayerRefresh > uiInfo.uiDC.realTime) {
			return;
		}
	}

	trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buf, sizeof(buf));
	if (Q_stricmp(Info_ValueForKey(buf, "game"), "etf")) {
		return;
	}

	trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
	trap_LAN_ServerStatus( uiInfo.serverStatusAddress, NULL, 0);

	if ( trap_LAN_ServerStatus( uiInfo.serverStatusAddress, buf, sizeof(buf)) ) {
		p = buf;
		// get the cvars
		while (p && *p) {
			p = strchr(p, '\\');
			if (!p)
				break;
			*p++ = '\0';
			if (*p == '\\')
				break;
			p = strchr(p, '\\');
			if (!p)
				break;
			*p++ = '\0';
		}
		// get the player list

		// parse players
		uiInfo.numServerPlayers = 0;
		while (p && *p) {
			if (*p == '\\')
				*p++ = '\0';
			if (!p)
				break;
			score = p;
			p = strchr(p, ' ');
			if (!p)
				break;
			*p++ = '\0';
			ping = p;
			p = strchr(p, ' ');
			if (!p)
				break;
			*p++ = '\0';
			name = p;
			if(name[0] == '\"')
				name++;
			p = strchr(p, '\\');
			if (!p)
				break;
			if(*(--p) == '\"')
				*(p) = '\0';
			p++;
			*p++ = '\0';

			uiInfo.ServerPlayerScores[uiInfo.numServerPlayers] = atoi(score);
			uiInfo.ServerPlayerPings[uiInfo.numServerPlayers] = atoi(ping);
			Q_strncpyz(uiInfo.ServerPlayers[uiInfo.numServerPlayers], name, 3 * MAX_NAME_LENGTH);
			uiInfo.numServerPlayers++;
		}
	}

	uiInfo.nextServerPlayerRefresh = uiInfo.uiDC.realTime + 500;

}

/*
==================
UI_FeederCount
==================
*/
static int UI_FeederCount(float feederID, itemDef_t* item) {
	if (feederID == FEEDER_CINEMATICS) {
		return uiInfo.movieCount;
	} else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		return uiInfo.mapCount;
	} else if (feederID == FEEDER_SERVERS) {
		return uiInfo.serverStatus.numDisplayServers;
	} else if (feederID == FEEDER_SERVERSTATUS) {
		return uiInfo.serverStatusInfo.numLines;
	} else if (feederID == FEEDER_FINDPLAYER) {
		return uiInfo.numFoundPlayerServers;
	} else if (feederID == FEEDER_PLAYER_LIST) {
		if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
			uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.playerCount;
	} else if (feederID == FEEDER_TEAM_LIST) {
		if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
			uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.myTeamCount;
	} else if (feederID == FEEDER_MODS) {
		return uiInfo.modCount;
	} else if (feederID == FEEDER_DEMOS) {
		return uiInfo.demoCount;
	} else if (feederID == FEEDER_BINDINGS) {
		return uiInfo.ETF_CurrentBindingTable_Size;
	} else if (feederID == FEEDER_BIND_MOVE) {
		return configDataTable_Move_Size;
	} else if (feederID == FEEDER_BIND_SHOOT) {
		return configDataTable_Shoot_Size;
	} else if (feederID == FEEDER_BIND_MISC) {
		return configDataTable_Misc_Size;
	} else if (feederID == FEEDER_BIND_COMS) {
		return configDataTable_Coms_Size;
	} else if (feederID == FEEDER_CONFIGS) {
		return uiInfo.cfgCount;
	} else if (feederID == FEEDER_CLASS_CONFIGS) {
		return Q3F_CLASS_MAX-1;
	} else if (feederID == FEEDER_SYSTEM) {
		return uiInfo.Q3F_CurrentSystemTable_Size;
	} else if (feederID == FEEDER_FAVSERVERS) {
		return trap_LAN_GetServerCount( AS_FAVORITES );
	} else if (feederID == FEEDER_CLASSINFO) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

		if(HUD_Q3F_GetChosenClass() != uiInfo.Q3F_current_classQuote) {
			HUD_SetupClassQuoteBuffer(item->textscale, ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &((menuDef_t*)(item->parent))->font, listPtr->columnInfo[0].width);
		}

		return uiInfo.Q3F_current_classQuote_lines;
	} else if (feederID == FEEDER_MAPBLURB) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
		UI_SetupMapQuoteBuffer(item->textscale, ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &((menuDef_t*)(item->parent))->font, listPtr->columnInfo[0].width);
		return uiInfo.ETF_current_mapQuote_lines;
	} else if (feederID == FEEDER_HUDDESCR) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
		UI_SetupHudQuoteBuffer(item->textscale, ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &((menuDef_t*)(item->parent))->font, listPtr->columnInfo[0].width);
		return uiInfo.ETF_current_hudQuote_lines;
	} else if (feederID == FEEDER_WEAPINFO) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
		UI_SetupWeapQuoteBuffer(item->textscale, ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &((menuDef_t*)(item->parent))->font, listPtr->columnInfo[0].width);
		return uiInfo.ETF_current_weapQuote_lines;
	} 
	else if (feederID == FEEDER_ADMIN_PLAYERS) {
		if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) {
			uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.Q3F_playercount;
	} else if (feederID == FEEDER_ADMIN_IPS) {
		return uiInfo.Q3F_playerIPcount;
	} else if (feederID == FEEDER_ADMIN_BANS) {
		return uiInfo.Q3F_playerBANcount;
	} else if (feederID == FEEDER_SERVER_MAPLIST) {
		if(!uiInfo.Q3F_serverMaplistBuilt) {
			UI_Q3F_BuildServerMaplist();
		}
		return uiInfo.Q3F_serverMaplistCount;
	} else if (feederID == FEEDER_SCOREBOARD) {
		return uiInfo.Q3F_scoreNum;
	} else if (feederID == FEEDER_GENERAL_SETTINGS) {
		return configDataTable_General_Size;
	} else if (feederID == FEEDER_HUDS) {
		return uiInfo.hudCount;
	} else if (feederID == FEEDER_HUDSVARIANT) {
		return uiInfo.hudVariationCount;
	} else if (feederID == FEEDER_WEAPONS) {
		return numGunNames;
	} else if (feederID == FEEDER_GRENADES) {
		return numGrenNames;
	} else if (feederID == FEEDER_ITEMS) {
		return numItemNames;
	} else if (feederID == FEEDER_SERVER_PLAYERS) {
		return uiInfo.numServerPlayers;
	}
	return 0;
}

static const char *UI_SelectedMap(int index, int *actual) {
	int i, c;
	c = 0;
	*actual = 0;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (c == index) {
			*actual = i;
			return uiInfo.mapList[i].mapName;
		} else {
			c++;
		}
	}
	return "";
}

/*static int UI_GetIndexFromSelection(int actual) {
	int i, c;
	c = 0;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (i == actual) {
			return c;
		}
		c++;
	}
  return 0;
}*/

static void UI_UpdatePendingPings() { 
	trap_LAN_ResetPings(ui_netSource.integer);
	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
}

static configData_t *UI_FeederItemInfo(float feederID, int index, itemDef_t* item) {
	if (feederID == FEEDER_BINDINGS) {
		if(index >= 0 && index < uiInfo.ETF_CurrentBindingTable_Size) {
			return &uiInfo.ETF_CurrentBindingTable[index];
		}
	} else if (feederID == FEEDER_SYSTEM) {
		if(index >= 0 && index < uiInfo.Q3F_CurrentSystemTable_Size) {
			return &uiInfo.Q3F_CurrentSystemTable[index];
		}
	} else if (feederID == FEEDER_GENERAL_SETTINGS) {
		if(index >= 0 && index < configDataTable_General_Size) {
			return &configDataTable_General[index];
		}
	} else 	if (feederID == FEEDER_BIND_MOVE) {
		if(index >= 0 && index < configDataTable_Move_Size) {
			return &configDataTable_Move[index];
		}
	} else 	if (feederID == FEEDER_BIND_SHOOT) {
		if(index >= 0 && index < configDataTable_Shoot_Size) {
			return &configDataTable_Shoot[index];
		}
	} else 	if (feederID == FEEDER_BIND_MISC) {
		if(index >= 0 && index < configDataTable_Misc_Size) {
			return &configDataTable_Misc[index];
		}
	} else 	if (feederID == FEEDER_BIND_COMS) {
		if(index >= 0 && index < configDataTable_Coms_Size) {
			return &configDataTable_Coms[index];
		}
	}
	return NULL;
}

static const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle, itemDef_t* item) {
	static char info[MAX_STRING_CHARS];
	static char hostname[1024];
	static char clientBuff[32];
	static int lastColumn = -1;
	static int lastTime = 0;
	*handle = -1;
	
	if (feederID == FEEDER_HUDS) {
		if(index < uiInfo.hudCount)
			return uiInfo.hudFiles[index];
	} else if (feederID == FEEDER_HUDSVARIANT) {
		if(index < uiInfo.hudVariationCount)
			return uiInfo.hudVariants[index];
	} else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		int actual;
		return UI_SelectedMap(index, &actual);
	} else if (feederID == FEEDER_SERVERS) {
		if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers) {
			int ping/*, game*/;
			if (lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000) {
				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
				lastColumn = column;
				lastTime = uiInfo.uiDC.realTime;
			}
			ping = atoi(Info_ValueForKey(info, "ping"));
			if (ping == -1) {
				// if we ever see a ping that is out of date, do a server refresh
				// UI_UpdatePendingPings();
			}
			switch (column) {
				case SORT_HOST : 
					if (ping <= 0) {
						return Info_ValueForKey(info, "addr");
					} else {
						if ( ui_netSource.integer == AS_LOCAL ) {
							Com_sprintf( hostname, sizeof(hostname), "%s [%s]",
											Info_ValueForKey(info, "hostname"),
											netnames[atoi(Info_ValueForKey(info, "nettype"))] );
							return hostname;
						}
						else {
							if (atoi(Info_ValueForKey(info, "sv_allowAnonymous")) != 0) {				// anonymous server
								Com_sprintf( hostname, sizeof(hostname), "(A) %s",
												Info_ValueForKey(info, "hostname"));
							} else {
								Com_sprintf( hostname, sizeof(hostname), "%s",
												Info_ValueForKey(info, "hostname"));
							}
							return hostname;
						}
					}
				case SORT_MAP:
					return Info_ValueForKey(info, "mapname");
				case SORT_CLIENTS : 
					//Com_sprintf( clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
					Com_sprintf( clientBuff, sizeof(clientBuff), "%s/%s", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
					return clientBuff;
				case SORT_PING : 
					if (ping <= 0) {
						return "...";
					} else {
						return Info_ValueForKey(info, "ping");
					}
				case SORT_PASSWORDED:
					if (atoi(Info_ValueForKey(info, "needpass")) != 0) {
						*handle = uiInfo.uiDC.Assets.lock;
						return NULL;
					}
					else
						return "";
				case SORT_PURE:
					if (atoi(Info_ValueForKey(info, "maxlives")) != 0) {
						*handle = uiInfo.uiDC.Assets.pureon;
					}
					else {
						*handle = uiInfo.uiDC.Assets.pureoff;
					}
					return NULL;
#if 0
				case SORT_NUMBOTS:
					if (atoi(Info_ValueForKey(info, "weaprestrict")) != 0) {
						*handle = uiInfo.uiDC.Assets.pureon;
					}
					else {
						*handle = uiInfo.uiDC.Assets.pureoff;
					}
					return "";
#endif
				case SORT_PUNKBUSTER:// {
					//char *s;
					//s = Info_ValueForKey(info, "punkbuster");
					//if (atoi(s) != 0) {
					if (atoi(Info_ValueForKey(info, "punkbuster")) != 0) {
						*handle = uiInfo.uiDC.Assets.pureon;
					}
					else {
						*handle = uiInfo.uiDC.Assets.pureoff;
					}
					return NULL;
				//}
#if 0
				case SORT_VERSION:
					if (atoi(Info_ValueForKey(info, "balancedteams")) == FORTS_VERSIONINT) {
						*handle = uiInfo.uiDC.Assets.pureon;
					}
					else {
						*handle = uiInfo.uiDC.Assets.pureoff;
					}
					return "";
#endif
			}
		}
	} else if (feederID == FEEDER_SERVERSTATUS) {
		if ( index >= 0 && index < uiInfo.serverStatusInfo.numLines ) {
			if ( column >= 0 && column < 4 ) {
				return uiInfo.serverStatusInfo.lines[index][column];
			}
		}
	} else if (feederID == FEEDER_FINDPLAYER) {
		if ( index >= 0 && index < uiInfo.numFoundPlayerServers ) {
			//return uiInfo.foundPlayerServerAddresses[index];
			if(column == 0)
				return uiInfo.foundPlayerNames[index];
			else
				return uiInfo.foundPlayerServerNames[index];
		}
	} else if (feederID == FEEDER_PLAYER_LIST) {
		if (index >= 0 && index < uiInfo.playerCount) {
			return uiInfo.playerNames[index];
		}
	} else if (feederID == FEEDER_TEAM_LIST) {
		if (index >= 0 && index < uiInfo.myTeamCount) {
			return uiInfo.teamNames[index];
		}
	} else if (feederID == FEEDER_MODS) {
		if (index >= 0 && index < uiInfo.modCount) {
			if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr) {
				return uiInfo.modList[index].modDescr;
			} else {
				return uiInfo.modList[index].modName;
			}
		}
	} else if (feederID == FEEDER_CINEMATICS) {
		if (index >= 0 && index < uiInfo.movieCount) {
			return uiInfo.movieList[index];
		}
	} else if (feederID == FEEDER_DEMOS) {
		if (index >= 0 && index < uiInfo.demoCount) {
			return uiInfo.demoList[index];
		}
	} else if (feederID == FEEDER_CONFIGS) {
		if (index >= 0 && index < uiInfo.cfgCount) {
			return uiInfo.cfgList[index];
		}
	} else if (feederID == FEEDER_CLASS_CONFIGS) {		
		if(index >= 0 && index < Q3F_CLASS_MAX-1) {
			return bg_q3f_classlist[index+1]->title;
		}
	} else if (feederID == FEEDER_FAVSERVERS) {
		if(index >= 0 && index < trap_LAN_GetServerCount( AS_FAVORITES )) {
			static char favinfo[MAX_NAME_LENGTH];
			char buff[MAX_STRING_CHARS];

			trap_LAN_GetServerInfo( AS_FAVORITES, index, buff, MAX_STRING_CHARS );

			favinfo[0] = '\0';

			switch(column) {
			case 0:
				Q_strncpyz(favinfo, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				break;
			case 1:
				Q_strncpyz(favinfo, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
				break;
			}

			if(!*favinfo) {
				return "";
			}

			return favinfo;
		}
	} else if (feederID == FEEDER_CLASSINFO) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

		if(HUD_Q3F_GetChosenClass() != uiInfo.Q3F_current_classQuote) {
			HUD_SetupClassQuoteBuffer(item->textscale, ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &((menuDef_t*)(item->parent))->font, listPtr->columnInfo[0].width);
		}

		return UI_GetBlurbLine(index);
	} else if ((feederID == FEEDER_MAPBLURB) || (feederID == FEEDER_HUDDESCR) || (feederID == FEEDER_WEAPINFO)) {
		return UI_GetBlurbLine(index);
	} else if (feederID == FEEDER_ADMIN_PLAYERS) {
		if(index >= 0 && index < uiInfo.Q3F_playercount) {
			return uiInfo.Q3F_playerNames[index];
		}
	} else if (feederID == FEEDER_ADMIN_IPS) {
		if(index >= 0 && index < uiInfo.Q3F_playerIPcount) {
			return uiInfo.Q3F_playerIPs[index];
		}
	} else if (feederID == FEEDER_ADMIN_BANS) {
		if(index >= 0 && index < uiInfo.Q3F_playerBANcount) {
			return uiInfo.Q3F_playerBANs[index];
		}
	} else if (feederID == FEEDER_SERVER_MAPLIST) {
		if(index >= 0 && index < uiInfo.Q3F_serverMaplistCount) {
			char *p;
			char buffer[64];
			int i, j;
			mapInfo* mInfo = NULL;
			gameIndexInfo_t* gIInfo = NULL;

			Q_strncpyz(buffer, uiInfo.Q3F_serverMaplist[index], 64);
			p = strchr(buffer, '+');
			*p++ = '\0';

			for(i = 0; i < uiInfo.mapCount; i++) {
				if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, buffer))) {
					mInfo = &uiInfo.mapList[i];
					break;
				}
			}

			if(!mInfo) {
				return va("%s: %s", p, buffer);
			}

			j = atoi(p);

			for(i = 0; i < mInfo->numGameIndicies; i++) {
				if(mInfo->gameIndiciesInfo[i].number == j) {
					gIInfo = &mInfo->gameIndiciesInfo[i];
				}
			}

			if(!gIInfo) {
				return va("%s: %s", p, buffer);
			}

			return gIInfo->name;
		}
	}
	else if (feederID == FEEDER_WEAPONS) {
		if(index < numGunNames)
			return gunNames[index];
	}
	else if (feederID == FEEDER_GRENADES) {
		if(index < numGrenNames)
			return grenNames[index];
	}
	else if (feederID == FEEDER_ITEMS) {
		if(index < numItemNames)
			return itemNames[index];
	}
	else if (feederID == FEEDER_SERVER_PLAYERS) {
		if (index >= 0 && index < uiInfo.numServerPlayers) {
			switch(column) {
				case 0:										// name
                    return uiInfo.ServerPlayers[index];
				case 1:										// scores
					return va("%d", uiInfo.ServerPlayerScores[index]);
				case 2:										// ping
					return va("%d", uiInfo.ServerPlayerPings[index]);
			}
		}
	}
	return "";
}


static qhandle_t UI_FeederItemImage(float feederID, int index, itemDef_t* item) {
	if (feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS) {
		int actual;
		UI_SelectedMap(index, &actual);
		index = actual;
		if (index >= 0 && index < uiInfo.mapCount) {
			if (uiInfo.mapList[index].levelShot == -1) {
				uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[index].imageName);
			}
			return uiInfo.mapList[index].levelShot;
		}	
	}
	return 0;
}

void PaintStatsHeader(itemDef_t* item, int *x, int *y, int height) {
	int x2, myx;
	int j;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );
	static qhandle_t bar = NULL_HANDLE;
	float x1, y1, w1, h1, w;

	x1 = myx = *x;
	x1 -= 4;
	w1 = 0;

	//	CG_FillRect(*x + 2, *y + 2, item->window.rect.w - SCROLLBAR_SIZE - 32, listPtr->elementHeight, item->window.outlineColor);
	for (j = 0; j < listPtr->numColumns; j++) {

		x2 = myx + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5); 
		w = myx + 2 + listPtr->columnInfo[j].pos + listPtr->columnInfo[j].width;
		if(w > w1)
			w1 = w;

		switch(j) {
			case 0: // weapon name
				x2 += (listPtr->columnInfo[j].width*0.5); 
				Text_Paint(x2 + item->textalignx, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Weapon", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_RIGHT);
				break;
			case 1:  // shots
				Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Shots", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				Text_Paint(x2 + item->textalignx, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Fired", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 2:	// hit %
				Text_Paint(x2 + item->textalignx, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Hit %", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 3:	// Kills by this weapon
				Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Your", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				Text_Paint(x2 + item->textalignx, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Kills", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 4:	// Deaths
				Text_Paint(x2 + item->textalignx, *y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Your", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				Text_Paint(x2 + item->textalignx, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Deaths", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
				break;
			case 5: // Award
				x2 -= (listPtr->columnInfo[j].width*0.5);
				Text_Paint(x2 + item->textalignx + 8, *y + (2 * listPtr->elementHeight) + item->textaligny, item->textscale, item->window.foreColor, "Most Proficient User", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
				break;
		}
	}
	*y += (2 * height);

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
		UI_AdjustFrom640(&x1, &y1, &w1, &h1);
		trap_R_DrawStretchPic(x1, y1, w1 + 8, h1, 0.02f, 0.02f, 0.98f, 0.98f, bar);
		*y += 2;
	}
}

float *UI_TeamColor_Scoreboard( int team ) {
	static vec4_t	red = {1, 0, 0, 1};
	static vec4_t	blue = {0, 0, 1, 1};
	static vec4_t	yellow = {1, 1, 0, 1};
	static vec4_t	green = {0, 1, 0, 1};
	static vec4_t	other = {1, 1, 1, 1};

	switch ( team ) {
	case Q3F_TEAM_RED:
		return red;
	case Q3F_TEAM_BLUE:
		return blue;
	case Q3F_TEAM_YELLOW:
		return yellow;
	case Q3F_TEAM_GREEN:
		return green;
	default:
		return other;
	}
}

static const char *StatNames[] = {
	"", "Axe","Shotgun","Super Shotgun", "Nailgun", "Super Nailgun", "Grenade Launcher",
	"Rocket Launcher","Sniper Rifle","Railgun", "Flamethrower", "Minigun", "Assault Rifle",
	"Dartgun","Pipe Launcher","Napalm Cannon", "", "Handgrenade", "Concussion Grenade", "Flash Grenade",
	"", "Nail Grenade", "Clusterbomb", "Bomblet", "Napalm Grenade", "Gas Grenade",
	"EMP Grenade", "HE Charge", "Sentry Gun", "Supply Station", "Other"
};


static qboolean UI_FeederPaintSpecial(itemDef_t* item) {
	int x, y, x2;
	int count = uiInfo.uiDC.feederCount(item->special, item);
	int i;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	fontStruct_t* parentFont = ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font );
	char buffer[64];
	vec4_t backClr;
	int disp;
	vec4_t red, green;

	red[0] = red[3] = 1;
	red[1] = red[2] = 0;
	green[1] = green[3] = 1;
	green[0] = green[2] = 0;


	switch((int)item->special) {
	case FEEDER_SCOREBOARD:

		x = item->window.rect.x + 1;
		y = item->window.rect.y + 1;

		for (i = listPtr->startPos; i < count; i++) {
			int j;
			
			memcpy( backClr, UI_TeamColor_Scoreboard(uiInfo.Q3F_uiScores[i].team), sizeof(vec4_t));
			backClr[3] *= 0.3f;

			for (j = 0; j < listPtr->numColumns; j++) {
				/*int */x2 = x;

				x2 = x + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5);

				switch(j) {
				case 0:
					Text_Paint(x + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, uiInfo.Q3F_uiScores[i].name, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
					break;
				case 1:
					UI_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( uiInfo.Q3F_uiScores[i].ping < 0 ) {
						break;
					}
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", uiInfo.Q3F_uiScores[i].score), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 2:
					UI_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( uiInfo.Q3F_uiScores[i].ping < 0 ) {
						break;
					}
					if(uiInfo.Q3F_uiScores[i].bot)
						Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, "BOT", 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					else
						Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", uiInfo.Q3F_uiScores[i].ping), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 3:
					UI_FillRect(x + 2 + listPtr->columnInfo[j].pos, y + 2, listPtr->columnInfo[j].width, listPtr->elementHeight, backClr);
					if ( uiInfo.Q3F_uiScores[i].ping < 0 ) {
						break;
					}
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, va("%i", uiInfo.Q3F_uiScores[i].time), 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
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
	case FEEDER_STATS:
		x = item->window.rect.x + 1; 
		y = item->window.rect.y + 1;
		i = 1;
		disp = 0;
		backClr[0] = 1.0f;
		backClr[1] = 1.0f;
		backClr[2] = 1.0f;
		backClr[3] = 0.3f;

		PaintStatsHeader(item, &x, &y, listPtr->elementHeight);
		count = y;

		if(!uiInfo.statsvalid)
			return qtrue;

		while(i < MAX_ETF_STATS) {
			int j;

			if(StatNames[i][0] == 0) {
				i++;
				continue;
			}

			if(!uiInfo.stats[i].shots && !uiInfo.stats[i].hits && !uiInfo.stats[i].kills && !uiInfo.stats[i].deaths) {
				i++;
				continue;
			}

			if((disp % 2) == 1) {
				if( i < 17)
					x2 = listPtr->columnInfo[listPtr->numColumns - 1].pos + listPtr->columnInfo[listPtr->numColumns - 1].width;
				else
					x2 = listPtr->columnInfo[listPtr->numColumns - 2].pos + listPtr->columnInfo[listPtr->numColumns - 2].width;
				UI_FillRect(x + 2, y + 3, x2, listPtr->elementHeight, backClr);
			}
			disp++;

			for (j = 0; j < listPtr->numColumns; j++) {

				x2 = x + listPtr->columnInfo[j].pos + (listPtr->columnInfo[j].width*0.5);

				switch(j) {
				case 0: // weapon name
					x2 += (listPtr->columnInfo[j].width*0.5);
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, StatNames[i], 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_RIGHT);
					break;
				case 1:  // shots
					Com_sprintf(buffer, sizeof(buffer), "%d", uiInfo.stats[i].shots);
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 2:	// hit %
					Com_sprintf(buffer, sizeof(buffer), "%d", (int)(((float)uiInfo.stats[i].hits / (float)uiInfo.stats[i].shots) * 100.0));
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 3:	// Kills by this weapon
					Com_sprintf(buffer, sizeof(buffer), "%d", uiInfo.stats[i].kills);
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, uiInfo.stats[i].kills ? green : item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 4:	// Deaths
					Com_sprintf(buffer, sizeof(buffer), "%d", uiInfo.stats[i].deaths);
					Text_Paint(x2 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, uiInfo.stats[i].deaths ? red : item->window.foreColor, buffer, 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_CENTER);
					break;
				case 5: // Award
					x2 -= (listPtr->columnInfo[j].width*0.5);
					if(uiInfo.awardsvalid && (i < WP_NUM_WEAPONS)) {
						Text_Paint(x2 + item->textalignx + 8, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, uiInfo.awards[i], 0, listPtr->columnInfo[j].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
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
			i++;
			// fit++;
		}

		if(listPtr->numColumns < 5)
			return qtrue;

		y = count + (WP_NUM_WEAPONS + 2) * listPtr->elementHeight;
		count = listPtr->numColumns - 1;
		x2 = x + listPtr->columnInfo[count].pos;

		Text_Paint(x2 + item->textalignx + 20, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Most Kills:", 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
		y += listPtr->elementHeight;
		Text_Paint(x2 + item->textalignx + 40, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, uiInfo.terminator, 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);

		y += (2 * listPtr->elementHeight);
		Text_Paint(x2 + item->textalignx + 20, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Most Deaths:", 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
		y += listPtr->elementHeight;
		Text_Paint(x2 + item->textalignx + 40, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, uiInfo.cannonfodder, 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);

		y += (2 * listPtr->elementHeight);
		Text_Paint(x2 + item->textalignx + 20, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, "Most Teamkills:", 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
		y += listPtr->elementHeight;
		Text_Paint(x2 + item->textalignx + 40, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, uiInfo.teamkiller, 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);

		y += (2 * listPtr->elementHeight);
		Text_Paint(x2 + item->textalignx + 20, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, va("Your Teamkills: ^1%d", uiInfo.teamkills), 0, listPtr->columnInfo[count].maxChars, item->textStyle, parentFont, ITEM_ALIGN_LEFT);
		return qtrue;
	}

	return qfalse;
}

static void UI_FeederSelection(float feederID, int index) {
	static char info[MAX_STRING_CHARS];

	if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		int actual, map;

		map = (feederID == FEEDER_ALLMAPS) ? ui_currentNetMap.integer : ui_currentMap.integer;
		if (uiInfo.mapList[map].cinematic >= 0) {
			trap_CIN_StopCinematic(uiInfo.mapList[map].cinematic);
			uiInfo.mapList[map].cinematic = -1;
		}
		UI_SelectedMap(index, &actual);
		trap_Cvar_Set("ui_mapIndex", va("%d", index));
		trap_Cvar_Set("hud_adminmap", va("%s", uiInfo.mapList[actual].mapLoadName));
		trap_Cvar_Set("hud_admingameindex", "1");
		ui_mapIndex.integer = index;

		if (feederID == FEEDER_MAPS) {
			ui_currentMap.integer = actual;
			trap_Cvar_Set("ui_currentMap", va("%d", actual));
			uiInfo.mapList[ui_currentMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		} else {			
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set("ui_currentNetMap", va("%d", actual));
			uiInfo.mapList[ui_currentNetMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );

			UI_SetupGameIndexMulti();
		}

	} else if (feederID == FEEDER_HUDS) {
		if( index < uiInfo.hudCount ) {
			Q_strncpyz(uiInfo.uiDC.curHud, uiInfo.hudFiles[index], 64);
			uiInfo.uiDC.curHudInt = index;
			UI_ParseHudVariantInfo();		// load up the relevant variants
			Menu_SetFeederSelection(NULL, FEEDER_HUDSVARIANT, uiInfo.uiDC.curHudVarInt, NULL);
		}
	} else if (feederID == FEEDER_HUDSVARIANT) {
		if(index < uiInfo.hudVariationCount) {
			Q_strncpyz(uiInfo.uiDC.curHudVariant, uiInfo.hudVariants[index], 64);
			uiInfo.uiDC.curHudVarInt = index;
			UI_LoadHudPreview();			// load preview image
		}
	} else if (feederID == FEEDER_SERVERS) {
		const char *mapName = NULL;

		uiInfo.serverStatus.currentServer = index;
		trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
		// RR2DO2: ugly hack as server doesn't presort q3f servers and we don't want a "bad" mapview to show up
		if (Q_stricmp(Info_ValueForKey(info, "game"), "etf")) {
			uiInfo.serverStatus.currentServerPreview = 0;
		} else {
			uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(va("levelshots/%s", Info_ValueForKey(info, "mapname")));
		}

		if (uiInfo.serverStatus.currentServerCinematic >= 0) {
			trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
			uiInfo.serverStatus.currentServerCinematic = -1;
		}
		mapName = Info_ValueForKey(info, "mapname");
		if (mapName && *mapName) {
			uiInfo.serverStatus.currentServerCinematic = trap_CIN_PlayCinematic(va("%s.roq", mapName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}

		// build the player list
		uiInfo.numServerPlayers = 0;
		Menu_SetFeederSelection(NULL, FEEDER_SERVER_PLAYERS, 0, NULL);
		UI_BuildServerPlayerList(qtrue);
	} else if (feederID == FEEDER_SERVERSTATUS) {
		//
	} else if (feederID == FEEDER_FINDPLAYER) {
		uiInfo.currentFoundPlayerServer = index;
	} else if (feederID == FEEDER_PLAYER_LIST) {
		uiInfo.playerIndex = index;
	} else if (feederID == FEEDER_TEAM_LIST) {
		uiInfo.teamIndex = index;
	} else if (feederID == FEEDER_MODS) {
		uiInfo.modIndex = index;
	} else if (feederID == FEEDER_CINEMATICS) {
		uiInfo.movieIndex = index;
		if (uiInfo.previewMovie >= 0) {
		  trap_CIN_StopCinematic(uiInfo.previewMovie);
		}
		uiInfo.previewMovie = -1;
	} else if (feederID == FEEDER_DEMOS) {
		uiInfo.demoIndex = index;
	} else if (feederID == FEEDER_CONFIGS) {
		uiInfo.cfgIndex = index;
	} else if (feederID == FEEDER_BINDINGS) {
		uiInfo.ETF_CurrentBindingTable_Pos = index;
	} else if (feederID == FEEDER_BIND_MOVE) {
		uiInfo.ETF_CurBindMove = index;
		uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindMove;
		uiInfo.ETF_CurrentBindingTable		= configDataTable_Move;
		uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Move_Size;
	} else if (feederID == FEEDER_BIND_SHOOT) {
		uiInfo.ETF_CurBindShoot = index;
		uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindShoot;
		uiInfo.ETF_CurrentBindingTable		= configDataTable_Shoot;
		uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Shoot_Size; 
	} else if (feederID == FEEDER_BIND_MISC) {
		uiInfo.ETF_CurBindMisc = index;
		uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindMisc;
		uiInfo.ETF_CurrentBindingTable		= configDataTable_Misc;
		uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Misc_Size;
	} else if (feederID == FEEDER_BIND_COMS) {
		uiInfo.ETF_CurBindComs = index;
		uiInfo.ETF_CurrentBindingTable_Pos	= uiInfo.ETF_CurBindComs;
		uiInfo.ETF_CurrentBindingTable		= configDataTable_Coms;
		uiInfo.ETF_CurrentBindingTable_Size = configDataTable_Coms_Size;
	} else if (feederID == FEEDER_CLASS_CONFIGS) {
		uiInfo.Q3F_SelectedClass = index+1;
	} else if (feederID == FEEDER_SYSTEM) {
		uiInfo.Q3F_CurrentSystemTable_Pos = index;
	} else if (feederID == FEEDER_FAVSERVERS) {
		uiInfo.Q3F_CurrentFavServer_Pos = index;
	} else if (feederID == FEEDER_ADMIN_PLAYERS) {
		uiInfo.Q3F_playerindex = index;
		Menus_SetFeederSelection(FEEDER_ADMIN_IPS, index);
	} else if (feederID == FEEDER_ADMIN_IPS) {
		uiInfo.Q3F_playerindex = index;
		Menus_SetFeederSelection(FEEDER_ADMIN_PLAYERS, index);
	} else if (feederID == FEEDER_ADMIN_BANS) {
		uiInfo.Q3F_playerBANindex = index;
	} else if (feederID == FEEDER_SERVER_MAPLIST) {		
		uiInfo.Q3F_serverMaplistIndex = index;
	} else if (feederID == FEEDER_GENERAL_SETTINGS) {
		uiInfo.Q3F_GeneralSettingsTable_Pos = index;
	} else if (feederID == FEEDER_WEAPONS) {
		uiInfo.uiDC.lastFeeder = FEEDER_WEAPONS;
		uiInfo.uiDC.curWeapInt = index;
		uiInfo.uiDC.weapPreview = -1;
		uiInfo.ETF_current_weapQuote_num = -1;
		uiInfo.uiDC.curWeapSource = gunNames;
	} else if (feederID == FEEDER_GRENADES) {
		uiInfo.uiDC.lastFeeder = FEEDER_GRENADES;
		uiInfo.uiDC.curWeapInt = index;
		uiInfo.uiDC.weapPreview = -1;
		uiInfo.ETF_current_weapQuote_num = -1;
		uiInfo.uiDC.curWeapSource = grenNames;
	} else if (feederID == FEEDER_ITEMS) {
		uiInfo.uiDC.lastFeeder = FEEDER_ITEMS;
		uiInfo.uiDC.curWeapInt = index;
		uiInfo.uiDC.weapPreview = -1;
		uiInfo.ETF_current_weapQuote_num = -1;
		uiInfo.uiDC.curWeapSource = itemNames;
	}
}

static void UI_Pause(qboolean b) {
	if (b) {
		// pause the game and set the ui keycatcher
		trap_Cvar_Set( "cl_paused", "1" );
		trap_Key_SetCatcher( KEYCATCH_UI );
	} else {
		// unpause the game and clear the ui keycatcher
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
		trap_Key_ClearStates();
		trap_Cvar_Set( "cl_paused", "0" );
		UI_SetEventHandling(UI_EVENT_NONE);
	}
}

static int UI_PlayCinematic(const char *name, float x, float y, float w, float h) {
	return trap_CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

static void UI_StopCinematic(int handle) {
	if (handle >= 0) {
	  trap_CIN_StopCinematic(handle);
	} else {
		handle = abs(handle);
		if (handle == UI_MAPCINEMATIC) {
			if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0) {
			  trap_CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
			  uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
			}
		} else if (handle == UI_NETMAPCINEMATIC) {
			if (uiInfo.serverStatus.currentServerCinematic >= 0) {
			  trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
				uiInfo.serverStatus.currentServerCinematic = -1;
			}
		}
	}
}

static void UI_DrawCinematic(int handle, float x, float y, float w, float h) {
	trap_CIN_SetExtents(handle, x, y, w, h);
	trap_CIN_DrawCinematic(handle);
}

static void UI_RunCinematicFrame(int handle) {
	trap_CIN_RunCinematic(handle);
}

void UI_Q3F_SetVersion(void);

/*
=================
UI_Init
=================
*/
void _UI_Init( qboolean inGameLoad ) {
	char ver[MAX_CVAR_VALUE_STRING];
	const char *menuSet;
	//int start;

	//uiInfo.inGameLoad = inGameLoad;

	UI_Q3F_SetVersion();

	UI_RegisterCvars();
	UI_InitMemory();
	trap_PC_RemoveAllGlobalDefines();

	trap_Cvar_VariableStringBuffer( "version", ver, sizeof( ver ) );
	if ( !strcmp( ver, "ET 2.60e") )
		engine_is_ETE = qtrue;
	else
		engine_is_ETE = qfalse;

	// cache redundant calulations
	trap_GetGlconfig( &uiInfo.uiDC.glconfig );

	// for 640x480 virtualized screen
	uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight * (1.0/480.0);
	uiInfo.uiDC.xscale = uiInfo.uiDC.glconfig.vidWidth * (1.0/640.0);
	if ( uiInfo.uiDC.glconfig.vidWidth * 480 > uiInfo.uiDC.glconfig.vidHeight * 640 ) {
		// wide screen
		uiInfo.uiDC.bias = 0.5 * ( uiInfo.uiDC.glconfig.vidWidth - ( uiInfo.uiDC.glconfig.vidHeight * (640.0/480.0) ) );
	}
	else {
		// no wide screen
		uiInfo.uiDC.bias = 0;
	}

	//UI_Load();
	uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	uiInfo.uiDC.setColor = &UI_SetColor;
	uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
	uiInfo.uiDC.drawAdjustedPic = &UI_DrawAdjustedPic;
	uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
	uiInfo.uiDC.drawText = &Text_Paint;
	uiInfo.uiDC.textWidth = &Text_Width;
	uiInfo.uiDC.textHeight = &Text_Height;
	uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
	uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
	uiInfo.uiDC.fillRect = &UI_FillRect;
	uiInfo.uiDC.drawRect = &_UI_DrawRect;
	uiInfo.uiDC.drawSides = &_UI_DrawSides;
	uiInfo.uiDC.drawTopBottom = &_UI_DrawTopBottom;
	uiInfo.uiDC.clearScene = &trap_R_ClearScene;
	uiInfo.uiDC.drawSides = &_UI_DrawSides;
	uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	uiInfo.uiDC.addLightToScene = &trap_R_AddLightToScene;
	uiInfo.uiDC.renderScene = &trap_R_RenderScene;
	uiInfo.uiDC.registerFont = &UI_Q3F_LoadFontFile;	//&trap_R_RegisterFont;
	uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
	uiInfo.uiDC.getValue = &UI_GetValue;
	uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
	uiInfo.uiDC.runScript = &UI_RunMenuScript;
	uiInfo.uiDC.getTeamColor = &UI_GetTeamColor;
	uiInfo.uiDC.setCVar = trap_Cvar_Set;
	uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
	uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
	uiInfo.uiDC.drawTextWithCursor = &Text_PaintWithCursor;
	uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	uiInfo.uiDC.startLocalSound = &trap_S_StartLocalSound;
	uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
	uiInfo.uiDC.feederCount = &UI_FeederCount;
	uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
	uiInfo.uiDC.feederItemText = &UI_FeederItemText;
	uiInfo.uiDC.feederItemInfo = &UI_FeederItemInfo;
	uiInfo.uiDC.feederSelection = &UI_FeederSelection;
	uiInfo.uiDC.feederPaintSpecial = &UI_FeederPaintSpecial;
	uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
	uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
	uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
	uiInfo.uiDC.Error = &Com_Error; 
	uiInfo.uiDC.Print = &Com_Printf; 
	uiInfo.uiDC.Pause = &UI_Pause;
	uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
	uiInfo.uiDC.ownerDrawSize = &UI_OwnerDrawSize;
	uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
	uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	uiInfo.uiDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
	uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
	uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
	uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;
	uiInfo.uiDC.openFile = &trap_FS_FOpenFile;
	uiInfo.uiDC.fRead = &trap_FS_Read;
	uiInfo.uiDC.fWrite = &trap_FS_Write;
	uiInfo.uiDC.closeFile = &trap_FS_FCloseFile;
	uiInfo.uiDC.keyIsDown = trap_Key_IsDown;
	uiInfo.uiDC.adjustFrom640 = UI_AdjustFrom640;

	//slothy
	uiInfo.uiDC.playerClass = 0;
	memset(hudpreviews, 0, sizeof(hudpreviews));

	// let's add some of our commands
	trap_AddCommand("screenshot_etf");
	trap_AddCommand("screenshotJPEG_etf");
#ifdef _ETXREAL
	trap_AddCommand("screenshotPNG_etf");
#endif
	trap_AddCommand("etfmap");
	trap_AddCommand("etfdevmap");
	// slothy

	Init_Display(&uiInfo.uiDC);

	String_Init();
  
	uiInfo.uiDC.cursor	= trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip( "white" );

	*uiInfo.mapMenuName = 0;

	AssetCache();

	//start = trap_Milliseconds();

	menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}

	UI_LoadMenus(menuSet, qtrue);

	menuSet = UI_Cvar_VariableString("ui_ingameMenuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/ingame.txt";
	}

	UI_LoadMenus(menuSet, qfalse);
	
	Menus_CloseAll();

	trap_LAN_LoadCachedServers();

	// sets defaults for ui temp cvars
	uiInfo.effectsColor = gamecodetoui[(int)trap_Cvar_VariableValue("color")-1];
	uiInfo.currentCrosshair = ((int)trap_Cvar_VariableValue("cg_drawCrosshair") % NUM_CROSSHAIRS);
	trap_Cvar_Set("ui_mousePitch", (trap_Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

	uiInfo.serverStatus.currentServerCinematic = -1;
	uiInfo.previewMovie = -1;

	uiInfo.hudCount = 0;
	uiInfo.hudVariationCount = 0;

	trap_Cvar_Register(NULL, "debug_protocol", "", 0 );

//	trap_Cvar_Set("ui_actualNetGameType", va("%d", ui_netGameType.integer));

	HUD_LoadLanguageData();

#ifdef API_ET
	trap_S_FadeAllSound( 1.0f, 1000, qfalse );	// keeg enable sounds always
#endif

}


/*
=================
UI_KeyEvent
=================
*/
void _UI_KeyEvent( int key, qboolean down ) {
	switch(uiInfo.eventHandling) {
	default:
		if (menuCount > 0) {
			menuDef_t *menu = Menu_GetFocused();
			if (menu) {
				if (key == K_ESCAPE && down && !Menus_AnyFullScreenVisible()) {
					Menus_CloseAll();
				} else {
					Menu_HandleKey( menu, key, down );
				}
			} else {
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
			}
		}

	//if ((s > 0) && (s != menu_null_sound)) {
		//  trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
	//}
		break;
	}
}

/*
=================
UI_MouseEvent
=================
*/
void _UI_MouseEvent( int dx, int dy )
{
	// update mouse screen position
	uiInfo.uiDC.cursorx += dx;
	if (uiInfo.uiDC.cursorx < 0)
		uiInfo.uiDC.cursorx = 0;
	else if (uiInfo.uiDC.cursorx > SCREEN_WIDTH)
		uiInfo.uiDC.cursorx = SCREEN_WIDTH;

	uiInfo.uiDC.cursory += dy;
	if (uiInfo.uiDC.cursory < 0)
		uiInfo.uiDC.cursory = 0;
	else if (uiInfo.uiDC.cursory > SCREEN_HEIGHT)
		uiInfo.uiDC.cursory = SCREEN_HEIGHT;

	if (menuCount > 0) {
		//menuDef_t *menu = Menu_GetFocused();
		//Menu_HandleMouseMove(menu, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
		Display_MouseMove(NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
	}

}

void UI_LoadNonIngame() {
	const char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}
	UI_LoadMenus(menuSet, qfalse);

	uiInfo.inGameLoad = qfalse;
}

void _UI_SetActiveMenu( uiMenuCommand_t menu ) {
	char buf[512];

	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	if (menuCount > 0) {
		menuDef_t* menus;
		menuDef_t* menubar;

		switch ( menu ) {
			default:
				return;
			case UIMENU_NONE:
				UI_SetEventHandling(UI_EVENT_NONE);
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
				return;
			case UIMENU_MAIN:
				//trap_Cvar_Set( "sv_killserver", "1" );
				trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_UI );
				UI_SetEventHandling(UI_EVENT_NONE);
				//trap_S_StartLocalSound( trap_S_RegisterSound("sound/misc/menu_background.wav", qfalse) , CHAN_LOCAL_SOUND );
//				trap_S_StartBackgroundTrack("music/sm_kesselrun.wav", NULL);
				if (uiInfo.inGameLoad) {
					UI_LoadNonIngame();
				}
				Menus_CloseAll();
				Menus_ActivateByName("main");

				trap_Cvar_VariableStringBuffer("name", buf, sizeof(buf));
				if(Q_stricmp(buf, "ETF_Player") == 0)
					Menus_ActivateByName("menu_profile");
				else
					Menus_ActivateByName("menubar");	// RR2DO2: spawning the Q3F menubar
				uiInfo.Q3F_BackModelStatus = Q3F_BM_STARTUP;
				uiInfo.Q3F_BackModelStartupTime = 0;
				uiInfo.Q3F_BackModelRotateEndTime = uiInfo.uiDC.realTime;
				trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
				if (strlen(buf)) {
					if( !Q_stricmpn( buf, "Invalid password", 16 ) ) {
						trap_Cvar_Set( "com_errorMessage", trap_TranslateString( buf ) );		// NERVE - SMF
						Menus_ActivateByName("popupPassword");
					}
					else {
						if( strstr( buf, "Hunk_Alloc" ) ) {
							Q_strcat( buf, sizeof(buf), "\n\nETF needs more memory!\nPlease append\n^5+set com_hunkMegs 128^7\nto the commandline." );
						}
						Q_strcat( buf, sizeof(buf), "\nRead the FAQ at www.etfgame.com for more info!\n" );
						trap_Cvar_Set( "com_errorMessage", buf );
						Menus_ActivateByName("error_popmenu");
					}
				}
				return;
			case UIMENU_TEAM:
				trap_Key_SetCatcher( KEYCATCH_UI );
				UI_SetEventHandling(UI_EVENT_NONE);
				Menus_ActivateByName("team");
			return;
			case UIMENU_NEED_CD:
				// no cd check in TA
				//trap_Key_SetCatcher( KEYCATCH_UI );
				//Menus_ActivateByName("needcd");
				//UI_ConfirmMenu( "Insert the CD", NULL, NeedCDAction );
				return;
			case UIMENU_BAD_CD_KEY:
				return;
			case UIMENU_POSTGAME:
				return;
			case UIMENU_INGAME:
				if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
					trap_Cmd_ExecuteText(EXEC_APPEND, "cg_cleanhandling\n");
				}

				trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_UI ); // dont keep closing the console
				UI_SetEventHandling(UI_EVENT_NONE);

				Menus_CloseAll();
				menubar = Menus_FindByName("ingame");
				if(menubar) {
					char buffer[256];

					Menus_Activate(menubar);
					uiInfo.uiDC.getCVarString("hud_focustab", buffer, sizeof(buffer));
/*					item = Menu_FindItemByName(menus, buffer);
					if(item) {
						if(item->action) {
							Item_RunScript(item, item->action);
						}
					}
*/

					menus = NULL;
					if(buffer[0]) {
						menus = Menus_FindByName(buffer);
						if(menus) {
							Menus_ShowByName(buffer);
							uiInfo.Q3F_BackModelStatus = Q3F_BM_OPENED;
							uiInfo.Q3F_BackModelMenuCurrent = String_Alloc(buffer);
							uiInfo.Q3F_BackModelMenuToOpen = NULL;
						}
					}

					//if(!menus)
					//	Menus_Activate(menubar);

					uiInfo.uiDC.setCVar("hud_focustab", "");
				}

				return;
			case UIMENU_ENDGAME:
				if(trap_Key_GetCatcher() & KEYCATCH_UI) {
					return;
				}

				trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_UI ); // dont keep closing the console
				UI_SetEventHandling(UI_EVENT_NONE);

				Menus_CloseAll();
				menus = Menus_FindByName("endgame");
				if(menus) {
					itemDef_t* item;
					char buffer[256];

					Menus_Activate(menus);
					uiInfo.uiDC.getCVarString("hud_focustab", buffer, sizeof(buffer));
					item = Menu_FindItemByName(menus, buffer);
					if(item) {
						if(item->action) {
							Item_RunScript(item, item->action);
						}
					}
				}

				return;
		} //Case closed
	}
}

static connstate_t	lastConnState;
static char			lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize ( char *buf, int bufsize, int value )
{
	if (value > 1024*1024*1024 ) { // gigs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d GB", 
			(value % (1024*1024*1024))*100 / (1024*1024*1024) );
	} else if (value > 1024*1024 ) { // megs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d MB", 
			(value % (1024*1024))*100 / (1024*1024) );
	} else if (value > 1024 ) { // kilos
		Com_sprintf( buf, bufsize, "%d KB", value / 1024 );
	} else { // bytes
		Com_sprintf( buf, bufsize, "%d bytes", value );
	}
}

// Assumes time is in msec
static void UI_PrintTime ( char *buf, int bufsize, int time ) {
	time /= 1000;  // change to seconds

	if (time > 3600) { // in the hours range
		Com_sprintf( buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60 );
	} else if (time > 60) { // mins
		Com_sprintf( buf, bufsize, "%d min %d sec", time / 60, time % 60 );
	} else  { // secs
		Com_sprintf( buf, bufsize, "%d sec", time );
	}
}

void Text_PaintCenter(float x, float y, float scale, vec4_t color, const char *text, float adjust, fontStruct_t *parentfont) {
	//int len = Text_Width(text, scale, 0, parentfont);
	Text_Paint(x /*- len / 2*/, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, parentfont, ITEM_ALIGN_CENTER);
}


static void UI_DisplayDownloadInfo( const char *downloadName, float centerPoint, float yStart, float scale ) {
	static char dlText[]	= "Downloading:";
	static char etaText[]	= "Estimated time left:";
	static char xferText[]	= "Transfer rate:";

	int downloadSize, downloadCount, downloadTime;
	char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int xferRate;
	int leftWidth;
	const char *s;

	downloadSize = trap_Cvar_VariableValue( "cl_downloadSize" );
	downloadCount = trap_Cvar_VariableValue( "cl_downloadCount" );
	downloadTime = trap_Cvar_VariableValue( "cl_downloadTime" );

	leftWidth = 320;

	UI_SetColor(colorWhite);
	Text_PaintCenter(centerPoint, yStart + 112, scale, colorWhite, dlText, 0, NULL);
	Text_PaintCenter(centerPoint, yStart + 192, scale, colorWhite, etaText, 0, NULL);
	Text_PaintCenter(centerPoint, yStart + 248, scale, colorWhite, xferText, 0, NULL);

	if (downloadSize > 0) {
		s = va( "%s (%d%%)", downloadName, (int) ( (float)downloadCount * 100.0f / downloadSize ) );
	} else {
		s = downloadName;
	}

	Text_PaintCenter(centerPoint, yStart+136, scale, colorWhite, s, 0, NULL);

	UI_ReadableSize( dlSizeBuf,		sizeof dlSizeBuf,		downloadCount );
	UI_ReadableSize( totalSizeBuf,	sizeof totalSizeBuf,	downloadSize );

	if (downloadCount < 4096 || !downloadTime) {
		Text_PaintCenter(leftWidth, yStart+216, scale, colorWhite, "estimating", 0, NULL);
		Text_PaintCenter(leftWidth, yStart+160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0, NULL);
	} else {
		if ((uiInfo.uiDC.realTime - downloadTime) / 1000) {
			xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
		} else {
			xferRate = 0;
		}
		UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );

		// Extrapolate estimated completion time
		if (downloadSize && xferRate) {
			int n = downloadSize / xferRate; // estimated time for entire d/l in secs

			// We do it in K (/1024) because we'd overflow around 4MB
			UI_PrintTime ( dlTimeBuf, sizeof dlTimeBuf, 
				(n - (((downloadCount/1024) * n) / (downloadSize/1024))) * 1000);

			Text_PaintCenter(leftWidth, yStart+216, scale, colorWhite, dlTimeBuf, 0, NULL);
			Text_PaintCenter(leftWidth, yStart+160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0, NULL);
		} else {
			Text_PaintCenter(leftWidth, yStart+216, scale, colorWhite, "estimating", 0, NULL);
			if (downloadSize) {
				Text_PaintCenter(leftWidth, yStart+160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0, NULL);
			} else {
				Text_PaintCenter(leftWidth, yStart+160, scale, colorWhite, va("(%s copied)", dlSizeBuf), 0, NULL);
			}
		}

		if (xferRate) {
			Text_PaintCenter(leftWidth, yStart+272, scale, colorWhite, va("%s/Sec", xferRateBuf), 0, NULL);
		}
	}
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( qboolean overlay ) {
	char			*s;
	uiClientState_t	cstate;
	char			info[MAX_INFO_VALUE];
	char text[256];
	float centerPoint, yStart, scale;
	
	menuDef_t *menu = Menus_FindByName("Connect");


	if ( !overlay && menu ) {
		Menu_Paint(menu, qtrue);
	}

	if (!overlay) {
		centerPoint = 320;
		yStart = 130;
		scale = 0.5f;
	} else {
		centerPoint = 320;
		yStart = 32;
		scale = 0.6f;
		return;
	}

	// see what information we should display
	trap_GetClientState( &cstate );

	if ( cstate.connState < CA_CONNECTED && *cstate.messageString) {
		char *ptr;
		char buf[512];

		if ((ptr = strstr(cstate.messageString, "ETF://")) != NULL ) {
			if((int)trap_Cvar_VariableValue("allowRedirect") == 1) {
				ptr += 6;
				Com_sprintf(buf, sizeof(buf), "connect %s\n", ptr);
				trap_Cmd_ExecuteText( EXEC_APPEND, buf);
				return;
			}
		}
	}

	info[0] = '\0';
	if( trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) ) ) {
		Text_PaintCenter(centerPoint, yStart, scale, colorWhite, va( "Loading %s", Info_ValueForKey( info, "mapname" )), 0, NULL);
	}

	if(ui_checkversion.integer) { 
		s = Info_ValueForKey( info, "g_etfversion" ); 
		if (s && s[0] && strcmp(s, FORTS_VERSION) ) { 
			trap_Error(va("Server version mismatch. Server has version %s, you have %s.  You can force the connection by setting cvar ui_checkversion to 0. In this case it is also recommended to set cl_autodownload to 1 to resolve any file differences between you and the server.", s, FORTS_VERSION));
		} 
	} 
	if (!Q_stricmp(cstate.servername,"localhost")) {
		Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite, va("Starting up..."), ITEM_TEXTSTYLE_SHADOWEDMORE, NULL);
	} else {
		strcpy(text, va("Connecting to %s", cstate.servername));
		Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite,text , ITEM_TEXTSTYLE_SHADOWEDMORE, NULL);
	}

	//UI_DrawProportionalString( 320, 96, "Press Esc to abort", UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color );

	// display global MOTD at bottom
	Text_PaintCenter(centerPoint, 600, scale, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0, NULL);
	// print any server info (server full, bad version, etc)
	if ( cstate.connState < CA_CONNECTED && *cstate.messageString) {
		Text_PaintCenter(centerPoint, yStart + 176, scale, colorWhite, cstate.messageString, 0, NULL);
	}

	if ( lastConnState > cstate.connState ) {
		lastLoadingText[0] = '\0';
	}
	lastConnState = cstate.connState;

	switch ( cstate.connState ) {
	case CA_CONNECTING:
		s = va("Awaiting connection...%i", cstate.connectPacketCount);
		break;
	case CA_CHALLENGING:
		s = va("Awaiting challenge...%i", cstate.connectPacketCount);
		break;
	case CA_CONNECTED: {
		char downloadName[MAX_INFO_VALUE];

			trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof(downloadName) );
			if (*downloadName) {
				UI_DisplayDownloadInfo( downloadName, centerPoint, yStart, scale );
				return;
			}
		}
		s = "Awaiting gamestate...";
		break;
	case CA_LOADING:
		return;
	case CA_PRIMED:
		return;
	default:
		return;
	}


	if (Q_stricmp(cstate.servername,"localhost")) {
		Text_PaintCenter(centerPoint, yStart + 80, scale, colorWhite, s, 0, NULL);
	}

	// password required / connection rejected information goes here
}


/*
================
cvars
================
*/

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;	// for tracking changes
} cvarTable_t;

vmCvar_t	ui_browserMaster;
vmCvar_t	ui_browserGameType;
vmCvar_t	ui_browserSortKey;
vmCvar_t	ui_browserShowFull;
vmCvar_t	ui_browserShowEmpty;
vmCvar_t	ui_browserShowPasswordProtected;
vmCvar_t	ui_browserShowVersion;

vmCvar_t	ui_brassTime;
vmCvar_t	ui_drawCrosshair;
vmCvar_t	ui_drawCrosshairNames;
vmCvar_t	ui_marks;

vmCvar_t	ui_server1;
vmCvar_t	ui_server2;
vmCvar_t	ui_server3;
vmCvar_t	ui_server4;
vmCvar_t	ui_server5;
vmCvar_t	ui_server6;
vmCvar_t	ui_server7;
vmCvar_t	ui_server8;
vmCvar_t	ui_server9;
vmCvar_t	ui_server10;
vmCvar_t	ui_server11;
vmCvar_t	ui_server12;
vmCvar_t	ui_server13;
vmCvar_t	ui_server14;
vmCvar_t	ui_server15;
vmCvar_t	ui_server16;

vmCvar_t	ui_cdkeychecked;

vmCvar_t	ui_dedicated;
//vmCvar_t	ui_gameType;
//vmCvar_t	ui_netGameType;
//vmCvar_t	ui_joinGameType;
vmCvar_t	ui_netSource;
vmCvar_t	ui_serverFilterType;
vmCvar_t	ui_menuFiles;
vmCvar_t	ui_ingameMenuFiles;
vmCvar_t	ui_currentMap;
vmCvar_t	ui_currentNetMap;
vmCvar_t	ui_mapIndex;
vmCvar_t	ui_selectedPlayer;
vmCvar_t	ui_selectedPlayerName;
vmCvar_t	ui_lastServerRefresh_0;
vmCvar_t	ui_lastServerRefresh_1;
vmCvar_t	ui_lastServerRefresh_2;
vmCvar_t	ui_lastServerRefresh_3;
//vmCvar_t	ui_captureLimit;
//vmCvar_t	ui_fragLimit;
vmCvar_t	ui_smallFont;
vmCvar_t	ui_bigFont;
vmCvar_t	ui_findPlayer;
vmCvar_t	ui_hudFiles;
vmCvar_t	ui_recordSPDemo;
vmCvar_t	ui_realCaptureLimit;
vmCvar_t	ui_realWarmUp;
vmCvar_t	ui_serverStatusTimeOut;
// RR2DO2
//vmCvar_t	ui_menuRotateSpeed;
vmCvar_t	r_vertexLight;
// RR2DO2
// djbob
vmCvar_t	hud_chosenClass;
vmCvar_t	hud_allowClasses;
vmCvar_t	hud_maxClasses;
vmCvar_t	hud_currentClasses;

vmCvar_t	ui_specifyServer;
vmCvar_t	ui_specifyPort;

vmCvar_t	cg_ScoreSnapshot;

vmCvar_t	ui_language;
// djbob

vmCvar_t	com_hunkmegs;

//cgame mappings
vmCvar_t	cg_crosshairColor;
vmCvar_t	cg_crosshairColorAlt;
vmCvar_t	cg_crosshairAlpha;
vmCvar_t	cg_crosshairAlphaAlt;

// slothy
vmCvar_t	cg_crosshairSize;
vmCvar_t	ui_addSpecifyFavorites;
vmCvar_t	ui_checkversion;

static cvarTable_t		cvarTable[] = {
	{ NULL,						"cg_grenadePrimeSound",		"sound/grentimer/grentimer.wav",	CVAR_ARCHIVE },
	{ NULL,						"cg_scoreboardsortmode",	"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_sniperDotScale",		"0.3",								CVAR_ARCHIVE },
	{ NULL,						"cg_adjustAgentSpeed",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_atmosphericEffects",	"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_drawPanel",				"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_execClassConfigs",		"0",								CVAR_ARCHIVE },	
	{ NULL,						"cg_execMapConfigs",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_filterObituaries",		"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_friendlyCrosshair",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_impactVibration",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_lowEffects",			"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_no3DExplosions",		"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_oldSkoolMenu",			"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_playClassSound",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_showGrenadeTimer1",		"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_showGrenadeTimer2",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_visualAids",			"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_showSentryCam",			"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_sniperDotColors",		"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_sniperHistoricalSight", "0",								CVAR_ARCHIVE },
	{ NULL,						"cg_flares",				"0",								CVAR_ARCHIVE },
	{ NULL,						"r_fastSky",				"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_drawSkyPortal",			"0",								CVAR_ARCHIVE },
	{ &ui_drawCrosshairNames,	"cg_drawCrosshairNames",	"1",								CVAR_ARCHIVE },
	{ NULL,						"r_dynamiclight",			"0",								CVAR_ARCHIVE },
	{ &ui_brassTime,			"cg_brassTime",				"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_simpleItems",			"0",								CVAR_ARCHIVE },
	{ NULL,						"cl_allowDownload",			"0",								CVAR_ARCHIVE },
	{ NULL,						"cg_drawFriend",			"1",								CVAR_ARCHIVE },
	{ NULL,						"cg_bloodFlash",			"1.0",								CVAR_ARCHIVE },
   //keeg unsure what this is for
//  	{ NULL, "cg_crosshairColor", "White", CVAR_ARCHIVE },
//	{ NULL, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE },
//	{ NULL, "cg_crosshairColorAlt", "White", CVAR_ARCHIVE },
 
	// cgame mappings
	{ &cg_crosshairAlpha,		"cg_crosshairAlpha",		"1.0",								CVAR_ARCHIVE },
	{ &cg_crosshairAlphaAlt,	"cg_crosshairAlphaAlt",		"1.0",								CVAR_ARCHIVE },
	{ &cg_crosshairColor,		"cg_crosshairColor",		"White",							CVAR_ARCHIVE },
	{ &cg_crosshairColorAlt,	"cg_crosshairColorAlt",		"White",							CVAR_ARCHIVE },
	{ NULL,						"cg_crosshairHealth",		"0",								CVAR_ARCHIVE },

// slothy
	{ &cg_crosshairSize,		"cg_crosshairSize",			"48",								CVAR_ARCHIVE },
	{ NULL,						"cg_drawSpeedometer",		"0",								CVAR_ARCHIVE },
// end slothy

// cana
	{ NULL,						"cg_markTime",				"20000",							CVAR_ARCHIVE },
// end cana

   { &ui_browserMaster,			"ui_browserMaster",			"0",								CVAR_ARCHIVE },
	{ &ui_browserGameType,		"ui_browserGameType",		"0",								CVAR_ARCHIVE },
	{ &ui_browserSortKey,		"ui_browserSortKey",		"4",								CVAR_ARCHIVE },
	{ &ui_browserShowFull,		"ui_browserShowFull",		"1",								CVAR_ARCHIVE },
	{ &ui_browserShowEmpty,		"ui_browserShowEmpty",		"1",								CVAR_ARCHIVE },
	{ &ui_browserShowPasswordProtected,		"ui_browserShowPasswordProtected",	"1",								CVAR_ARCHIVE },
	{ &ui_browserShowVersion,	"ui_browserShowVersion",	"1",								CVAR_ARCHIVE },

	{ &ui_drawCrosshair,		"cg_drawCrosshair",			"4",								CVAR_ARCHIVE },

	{ &ui_server1,				"server1",					"",									CVAR_ARCHIVE },
	{ &ui_server2,				"server2",					"",									CVAR_ARCHIVE },
	{ &ui_server3,				"server3",					"",									CVAR_ARCHIVE },
	{ &ui_server4,				"server4",					"",									CVAR_ARCHIVE },
	{ &ui_server5,				"server5",					"",									CVAR_ARCHIVE },
	{ &ui_server6,				"server6",					"",									CVAR_ARCHIVE },
	{ &ui_server7,				"server7",					"",									CVAR_ARCHIVE },
	{ &ui_server8,				"server8",					"",									CVAR_ARCHIVE },
	{ &ui_server9,				"server9",					"",									CVAR_ARCHIVE },
	{ &ui_server10,				"server10",					"",									CVAR_ARCHIVE },
	{ &ui_server11,				"server11",					"",									CVAR_ARCHIVE },
	{ &ui_server12,				"server12",					"",									CVAR_ARCHIVE },
	{ &ui_server13,				"server13",					"",									CVAR_ARCHIVE },
	{ &ui_server14,				"server14",					"",									CVAR_ARCHIVE },
	{ &ui_server15,				"server15",					"",									CVAR_ARCHIVE },
	{ &ui_server16,				"server16",					"",									CVAR_ARCHIVE },
	{ &ui_cdkeychecked,			"ui_cdkeychecked",			"0",								CVAR_ROM },
	{ &ui_new,					"ui_new",					"0",								CVAR_TEMP },
	{ &ui_debug,				"ui_debug",					"0",								CVAR_TEMP },
	{ &ui_initialized,			"ui_initialized",			"0",								CVAR_TEMP },
	{ &ui_currentMap,			"ui_currentMap",			"0",								CVAR_TEMP },
	{ &ui_currentNetMap,		"ui_currentNetMap",			"0",								CVAR_TEMP },
	{ &ui_mapIndex,				"ui_mapIndex",				"0",								CVAR_TEMP },
	{ &hud_chosenClass,			"hud_chosenClass",			"0",								CVAR_TEMP },
	{ &ui_dedicated,			"ui_dedicated",				"0",								CVAR_ARCHIVE },
	{ &ui_netSource,			"ui_netSource",				"1",								CVAR_ARCHIVE },
	{ &ui_menuFiles,			"ui_menuFiles",				"ui/menus.txt",						CVAR_ARCHIVE },
	{ &ui_ingameMenuFiles,		"ui_ingameMenuFiles",		"ui/ingame.txt",					CVAR_ARCHIVE },
	{ &ui_selectedPlayer,		"cg_selectedPlayer",		"0",								CVAR_ARCHIVE},
	{ &ui_selectedPlayerName,	"cg_selectedPlayerName",	"",									CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_0,	"ui_lastServerRefresh_0",	"",									CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_1,	"ui_lastServerRefresh_1",	"",									CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_2,	"ui_lastServerRefresh_2",	"",									CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_3,	"ui_lastServerRefresh_3",	"",									CVAR_ARCHIVE},
	{ &ui_smallFont,			"ui_smallFont",				"0.19",								CVAR_ARCHIVE},
	{ &ui_bigFont,				"ui_bigFont",				"0.3",								CVAR_ARCHIVE},
	{ &ui_findPlayer,			"ui_findPlayer",			"ETF_PLAYER",						CVAR_ARCHIVE},
	{ &ui_hudFiles,				"cg_hudFiles",				"ui/hud/default/medium.menu",		CVAR_ARCHIVE},
	{ &ui_recordSPDemo,			"ui_recordSPDemo",			"0",								CVAR_ARCHIVE},
	{ &ui_realWarmUp,			"g_warmup",					"20",								CVAR_ARCHIVE},
	{ &ui_realCaptureLimit,		"capturelimit",				"0",								CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART},
	{ &ui_serverStatusTimeOut,	"ui_serverStatusTimeOut",	"7000",								CVAR_ARCHIVE},
// RR2DO2
//	{ &ui_menuRotateSpeed,		"ui_menuRotateSpeed",		"1",								CVAR_ARCHIVE },	// range 0 - 10
	{ &r_vertexLight,			"r_vertexLight",			"0",								CVAR_TEMP },
// RR2DO2
// djbob

	{ &hud_allowClasses,		"hud_allowClasses",			"0000000000",						CVAR_ROM | CVAR_TEMP },
	{ &hud_currentClasses,		"hud_currentClasses",		"00000000000000000000",				CVAR_ROM | CVAR_TEMP },
	{ &hud_maxClasses,			"hud_maxClasses",			"00000000000000000000",				CVAR_ROM | CVAR_TEMP },

	{ NULL,						"hud_admin_auth",			"0",								CVAR_ROM | CVAR_TEMP },
	{ NULL,						"hud_rcon_auth",			"0",								CVAR_ROM | CVAR_TEMP },
	
	{ &ui_specifyServer,		"ui_specifyServer",			"",									CVAR_ARCHIVE },
	{ &ui_specifyPort,			"ui_specifyPort",			"27960",							CVAR_ARCHIVE },

	{ NULL,						"discard_shells",			"-1",								CVAR_ARCHIVE },
	{ NULL,						"discard_cells",			"-1",								CVAR_ARCHIVE },
	{ NULL,						"discard_rockets",			"-1",								CVAR_ARCHIVE },
	{ NULL,						"r_loresskins",				"0",								CVAR_ARCHIVE | CVAR_LATCH }, 

	{ &cg_ScoreSnapshot,		"cg_ScoreSnapshot",			"0",								CVAR_ARCHIVE },

	{ &ui_language,				"ui_language",				"english",							CVAR_ARCHIVE },
// djbob

// slothy
	{ NULL,						"ui_showtooltips",			"1",								CVAR_ARCHIVE },
	{ &ui_addSpecifyFavorites,	"ui_addSpecifyFavorites",	"0",								CVAR_TEMP },
	{ NULL,						"ui_favServerName",			"",									CVAR_ARCHIVE },
	{ NULL,						"scr_conspeed",				"3",								CVAR_ARCHIVE },
	{ NULL,						"ui_memsize",				"0",								CVAR_ARCHIVE },
	{ NULL,						"ui_netspeed",				"2",								CVAR_ARCHIVE },
	
// slothy	

	{ &com_hunkmegs,			"com_hunkmegs",				"128",								CVAR_ARCHIVE | CVAR_LATCH },
	{ NULL,						"com_zonemegs",				"24",								CVAR_ARCHIVE },
	{ NULL,						"com_soundmegs",			"24",								CVAR_ARCHIVE },

// slothy - adding these because they weren't being saved
	{ NULL,						"r_displayrefresh",			"0",								CVAR_ARCHIVE },
	{ NULL,						"r_intensity",				"1",								CVAR_ARCHIVE },
	{ NULL,						"allowRedirect",			"0",								CVAR_ARCHIVE },
	{ &ui_checkversion,			"ui_checkversion",			"0",								CVAR_ARCHIVE },
};

static int		cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

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
	// hunkmegs
	{ &com_hunkmegs,		"com_hunkmegs",			128,		128,		-1,		0,	0,	qfalse },
};

static int cvarLimitTableSize = sizeof( cvarLimitTable ) / sizeof( cvarLimitTable[0] );

/*
=================
UI_LimitCvars
=================
*/
void UI_LimitCvars( void ) {
	int					i;
	cvarLimitTable_t	*cvl;
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( atoi(var) ) {
		return;
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
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
		if(cv->vmCvar != NULL) {
			cv->modificationCount = cv->vmCvar->modificationCount;
      }
	}

	for ( i = Q3F_CLASS_RECON ; i < Q3F_CLASS_MAX ; i++ ) {
		trap_Cvar_Register( NULL, va("discard_%s_shells",	bg_q3f_classlist[i]->commandstring), "-1", CVAR_ARCHIVE );
		trap_Cvar_Register( NULL, va("discard_%s_cells",	bg_q3f_classlist[i]->commandstring), "-1", CVAR_ARCHIVE );
		trap_Cvar_Register( NULL, va("discard_%s_rockets",	bg_q3f_classlist[i]->commandstring), "-1", CVAR_ARCHIVE );		
	}

	trap_Cvar_Set("ui_currentNetMap", "0");
	trap_Cvar_Set("ui_currentMap", "0");

	BG_setCrosshair(cg_crosshairColor.string, uiInfo.xhairColor, cg_crosshairAlpha.value, "cg_crosshairColor");
	BG_setCrosshair(cg_crosshairColorAlt.string, uiInfo.xhairColorAlt, cg_crosshairAlphaAlt.value, "cg_crosshairColorAlt");

	// limit cvars
	UI_LimitCvars();
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );
         //Keeg from et xhair code
         if(cv->modificationCount != cv->vmCvar->modificationCount) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				// OSP
				if(cv->vmCvar == &cg_crosshairColor || cv->vmCvar == &cg_crosshairAlpha) {
					BG_setCrosshair(cg_crosshairColor.string, uiInfo.xhairColor, cg_crosshairAlpha.value, "cg_crosshairColor");
				}

				if(cv->vmCvar == &cg_crosshairColorAlt || cv->vmCvar == &cg_crosshairAlphaAlt) {
					BG_setCrosshair(cg_crosshairColorAlt.string, uiInfo.xhairColorAlt, cg_crosshairAlphaAlt.value, "cg_crosshairColorAlt");
				}
			} 
		}
	}

	// limit cvars
	UI_LimitCvars();
}


/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
	int count;

	if (!uiInfo.serverStatus.refreshActive) {
		// not currently refreshing
		return;
	}
	uiInfo.serverStatus.refreshActive = qfalse;
	uiInfo.serverStatus.maxservers = uiInfo.serverStatus.numqueriedservers;
	Com_Printf("%d servers listed in browser with %d players.\n",
					uiInfo.serverStatus.numDisplayServers,
					uiInfo.serverStatus.numPlayersOnServers);
	count = trap_LAN_GetServerCount(ui_netSource.integer);
	if (count - uiInfo.serverStatus.numDisplayServers > 0) {
#ifdef API_Q3
		Com_Printf("%d servers not listed due to packet loss, pings higher than %d or being filtered out\n",
						count - uiInfo.serverStatus.numDisplayServers,
						(int) trap_Cvar_VariableValue("cl_maxPing"));
#endif
#ifdef API_ET
		Com_Printf("%d players on %d servers not listed (filtered out by game browser settings)\n",
						uiInfo.serverStatus.numTotalPlayers - uiInfo.serverStatus.numPlayersOnServers, 
						count - uiInfo.serverStatus.numDisplayServers);
#endif
	}

}

/*
=================
ArenaServers_MaxPing
=================
*/
/*static int ArenaServers_MaxPing( void ) {
	int		maxPing;

	maxPing = (int)trap_Cvar_VariableValue( "cl_maxPing" );
	if( maxPing < 100 ) {
		maxPing = 100;
	}
	return maxPing;
}*/

/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh( void )
{
	qboolean wait = qfalse;

	if (!uiInfo.serverStatus.refreshActive) {
		return;
	}
	if (ui_netSource.integer != AS_FAVORITES) {
		if (ui_netSource.integer == AS_LOCAL) {
			if (!trap_LAN_GetServerCount(ui_netSource.integer)) {
				wait = qtrue;
			}
		} else {
			int count = trap_LAN_GetServerCount(ui_netSource.integer);
			if (count < 0) {
				wait = qtrue;
			}
		}
	}

	if (uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime) {
		if (wait) {
			return;
		}
	}

	// if still trying to retrieve pings
	if (trap_LAN_UpdateVisiblePings(ui_netSource.integer)) {
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
	} else if (!wait) {	
		if(!uiInfo.serverStatus.nextpingtime)
			uiInfo.serverStatus.nextpingtime = uiInfo.uiDC.realTime + 3000;

		// get the last servers in the list
		UI_BuildServerDisplayList(2);

	}

	UI_BuildServerDisplayList(qfalse);

	if(uiInfo.serverStatus.nextpingtime && (uiInfo.serverStatus.nextpingtime < uiInfo.uiDC.realTime))
	{
		UI_StopServerRefresh();
	}
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh(qboolean full)
{
	int		i;
	char	buff[64];
	char	*ptr;

	qtime_t q;
	trap_RealTime(&q);
	Com_sprintf( buff, sizeof(buff), "%s-%i, %i at %s:%s", MonthAbbrev[q.tm_mon],q.tm_mday, 1900+q.tm_year,q.tm_hour<10?va("0%i",q.tm_hour):va("%i",q.tm_hour),
																										   q.tm_min<10?va("0%i",q.tm_min):va("%i",q.tm_min) );
 	trap_Cvar_Set( va("ui_lastServerRefresh_%i", ui_netSource.integer), buff );

	if (!full) {
		UI_UpdatePendingPings();
		return;
	}

	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
	// clear number of displayed servers
	uiInfo.serverStatus.numqueriedservers = 0;
	uiInfo.serverStatus.maxservers = 0;
	uiInfo.serverStatus.numDisplayServers = 0;
	uiInfo.serverStatus.numPlayersOnServers = 0;
	uiInfo.serverStatus.numTotalPlayers = 0;
	// mark all servers as visible so we store ping updates for them
	trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
	// reset all the pings
	trap_LAN_ResetPings(ui_netSource.integer);
	//
	if( ui_netSource.integer == AS_LOCAL ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "localservers\n" );
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
		return;
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
	if( ui_netSource.integer == AS_GLOBAL
#ifdef API_Q3
		|| ui_netSource.integer == AS_MPLAYER
#endif
		) {
		if( ui_netSource.integer == AS_GLOBAL ) {
			i = 0;
		}
		else {
			i = 1;
		}

		ptr = UI_Cvar_VariableString("protocol");
		if( *ptr ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %s full empty\n", i, ptr));  // \\game\\etf
			//trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %s \\game\\etf\n", i, ptr));  // \\game\\etf
		}
		else {
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %d full empty\n", i, (int)trap_Cvar_VariableValue( "protocol" ) ) );
			//trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %d \\game\\etf\n", i, (int)trap_Cvar_VariableValue( "protocol" ) ) );
			//trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %d q3f\n", i, (int)trap_Cvar_VariableValue( "protocol" ) ) );
		}
	}
}

int HUD_WrapText( char* instr, float scale, fontStruct_t* font, float w, int size) {
	char buffer[1024];
	char	*s, *p, *c, *ls;
	int lines = 0;
	
	strcpy(buffer, instr);

	memset(instr, 0, size);

	c = s = instr;
	p = buffer;
	ls = NULL;
	while(*p) {
		*c = *p++;

		if(*c == ' ' || *c == '\t') {
			ls = c;
		} // store last space/tab, to try not to break mid word

		c++;

		if(*p == '\n') {
			s = c+1;
			lines++;
		} else if(Text_Width(s, scale, 0, font) > w) {
			if(ls) {
				*ls = '\n';
				s = ls+1;
			} else {
				*c = *(c-1);
				*(c-1) = '\n';
				s = c++;
			}

			ls = NULL;

			lines++;
		}
	}

	if(c != buffer && (*(c-1) != '\n')) {
		*c++ = '\n';
		lines++;
	}
	
	*c = '\0';

	return lines;
}

int HUD_Q3F_GetChosenClass () {
	int cls = trap_Cvar_VariableValue("hud_chosenClass");
	int k = 0;
	if(cls == 10) { // 10 = choose random
		
		do{ // make sure we always select a new class
			cls = rand() % 10;
			k++;
		} while((cls == uiInfo.Q3F_current_classQuote || HUD_ClassUnavailable(cls)) && k < 128);

		trap_Cvar_Set("hud_chosenClass", va("%i", cls));
	}

	return (cls) % (Q3F_CLASS_MAX - Q3F_CLASS_RECON);
}

void UI_SetupWeapQuoteBuffer(float scale, fontStruct_t* font, float w) {
	char filename[256];
	char weapfname[256];
	int				len;
	fileHandle_t	f;

	if(uiInfo.ETF_current_weapQuote_num == uiInfo.uiDC.curWeapInt)
		return;

	uiInfo.Q3F_blurb_buffer[0] = 0;
	checkfname(uiInfo.uiDC.curWeapSource[uiInfo.uiDC.curWeapInt], weapfname);
	Com_sprintf(filename, 256, "%s/%s.cfg", WEAPONSHOTDIR, weapfname);

	len = uiInfo.uiDC.openFile(filename, &f, FS_READ);
	if(len >= MAX_BLURB_TEXT)
		len = MAX_BLURB_TEXT - 1;
	uiInfo.uiDC.fRead(uiInfo.Q3F_blurb_buffer, len, f);
	uiInfo.uiDC.closeFile(f);
	uiInfo.Q3F_blurb_buffer[len] = 0;

	uiInfo.ETF_current_weapQuote_lines = HUD_WrapText(uiInfo.Q3F_blurb_buffer, scale, font, w, MAX_BLURB_TEXT);
	uiInfo.ETF_current_weapQuote_num = uiInfo.uiDC.curWeapInt;
}

void UI_SetupHudQuoteBuffer(float scale, fontStruct_t* font, float w) {
	char filename[256];
	int				len;
	fileHandle_t	f;

	if((uiInfo.ETF_current_hudNameQuote == uiInfo.uiDC.curHudInt) && (uiInfo.ETF_current_hudVarQuote == uiInfo.uiDC.curHudVarInt))
		return;

	uiInfo.Q3F_blurb_buffer[0] = 0;
	Com_sprintf(filename, 256, "%s/%s/%s.cfg", HUDINFODIR, uiInfo.uiDC.curHud, uiInfo.uiDC.curHudVariant);

	len = uiInfo.uiDC.openFile(filename, &f, FS_READ);
	if(len >= MAX_BLURB_TEXT)
		len = MAX_BLURB_TEXT - 1;
	uiInfo.uiDC.fRead(uiInfo.Q3F_blurb_buffer, len, f);
	uiInfo.uiDC.closeFile(f);
	uiInfo.Q3F_blurb_buffer[len] = 0;

	uiInfo.ETF_current_hudQuote_lines = HUD_WrapText(uiInfo.Q3F_blurb_buffer, scale, font, w, MAX_BLURB_TEXT);
	uiInfo.ETF_current_hudNameQuote = uiInfo.uiDC.curHudInt;
	uiInfo.ETF_current_hudVarQuote = uiInfo.uiDC.curHudVarInt;
}


void UI_SetupMapQuoteBuffer(float scale, fontStruct_t* font, float w) {
	gameIndexInfo_t *gii;
	int number = trap_Cvar_VariableValue("ui_mapIndex");
	char tempbuf[MAX_BLURB_TEXT];
	int i;

	if(uiInfo.ETF_current_mapQuote == number)
		return;

	uiInfo.Q3F_blurb_buffer[0] = 0;

	for(i = 0; i < uiInfo.mapList[number].numGameIndicies; i++)
	{
		gii = &uiInfo.mapList[number].gameIndiciesInfo[i];
		if(!i)
			Com_sprintf(tempbuf, MAX_BLURB_TEXT, "%s\n%s", gii->name, gii->description);
		else
			Com_sprintf(tempbuf, MAX_BLURB_TEXT, "\n\n%s\n%s", gii->name, gii->description);
	
		Q_strcat(uiInfo.Q3F_blurb_buffer, MAX_BLURB_TEXT, tempbuf);
	}

	uiInfo.ETF_current_mapQuote_lines = HUD_WrapText(uiInfo.Q3F_blurb_buffer, scale, font, w, MAX_BLURB_TEXT);
	uiInfo.ETF_current_mapQuote = number;
}

char* UI_GetBlurbLine(int i) {
	static char buf[MAX_BLURB_TEXT];
	char buffer[MAX_BLURB_TEXT];
	char *s, *p;
	int line = 0;

	Q_strncpyz(buffer, uiInfo.Q3F_blurb_buffer, MAX_BLURB_TEXT);

	if(!*buffer) {
		return "";
	}

	s = p = buffer;
	while(*p) {
		if(*p == '\n') {
			*p++ = '\0';

			if(line++ == i) {
				Q_strncpyz(buf, s, MAX_BLURB_TEXT);
				return buf;
			}

			s = p;
		} else {
			p++;
		}
	}

	return "";
}



playerInfo_t hud_pi, hud_head;

int hud_class_weapanims[Q3F_CLASS_MAX] = {
	0,
	ANI_WEAPON_NAILGUN_IDLE,
	ANI_WEAPON_SRIFLE_IDLE,
	ANI_WEAPON_RLAUNCHER_IDLE,
	ANI_WEAPON_PLAUNCHER_IDLE,
	ANI_WEAPON_SNAILGUN_IDLE,
	ANI_WEAPON_MINIGUN_IDLE,
	ANI_WEAPON_FTHROWER_IDLE,
	ANI_WEAPON_DARTGUN_IDLE,
	ANI_WEAPON_RAILGUN_IDLE,
	ANI_WEAPON_AXE_IDLE,
};

int hud_class_weapons[Q3F_CLASS_MAX] = {
	0,
	WP_NAILGUN,
	WP_SNIPER_RIFLE,
	WP_ROCKET_LAUNCHER,
	WP_PIPELAUNCHER,
	WP_SUPERNAILGUN,
	WP_MINIGUN,
	WP_FLAMETHROWER,
	WP_DARTGUN,
	WP_RAILGUN,
	WP_AXE,
};

void HUD_SetupClassQuoteBuffer(float scale, fontStruct_t* font, float w) {
	int cls = HUD_Q3F_GetChosenClass();
//	vec3_t angles = {0, 180, 0};
//	vec3_t angles2 = {0, 0, 0};
//	vec3_t head_angles = {0, 90, 0};
//	vec3_t head_angles2 = {0, 0, 0};

	uiInfo.Q3F_current_classQuote = cls;

	Q_strncpyz(uiInfo.Q3F_blurb_buffer, uiInfo.Q3F_class_quotes[cls] ? uiInfo.Q3F_class_quotes[cls] : "", MAX_BLURB_TEXT);

	uiInfo.Q3F_current_classQuote_lines = HUD_WrapText(uiInfo.Q3F_blurb_buffer, scale, font, w, MAX_BLURB_TEXT);
	uiInfo.Q3F_current_classQuote_time = uiInfo.uiDC.realTime;

//	if(!HUD_ClassUnavailable(cls)) {
//		UI_Q3F_RegisterClassModels(cls+1);
//		UI_PlayerInfo_SetInfo(&hud_pi, ANI_MOVE_IDLESTAND, hud_class_weapanims[cls+1], angles, angles2, hud_class_weapons[cls+1], cls+1);
//		UI_PlayerInfo_SetInfo(&hud_head, ANI_MOVE_IDLESTAND, hud_class_weapanims[cls+1], head_angles, head_angles2, hud_class_weapons[cls+1], cls+1);
//	}

	uiInfo.Q3F_classstats_oldvalues[0] = uiInfo.Q3F_classstats_currentvalues[0];
	uiInfo.Q3F_classstats_oldvalues[1] = uiInfo.Q3F_classstats_currentvalues[1];
	uiInfo.Q3F_classstats_oldvalues[2] = uiInfo.Q3F_classstats_currentvalues[2];
}

int HUD_LoadData(const char* filename, const char** buffer, int buffersize) {
	fileHandle_t fh;
	int length;
	char buf[2][2048];
	char *p, *p2;

	*buffer = NULL;

	length = trap_FS_FOpenFile(filename, &fh, FS_READ);
	if( fh ) {
		if(length >= buffersize) {
			trap_FS_FCloseFile(fh);			
			return 2;
		}
		trap_FS_Read(buf[0], length, fh);
		buf[0][length] = '\0';

		for(p = buf[0], p2 = buf[1]; *p; p++) {
			if(*p != '\r') {
				*p2++ = *p;
			}
		}
		*p2 = '\0';

		*buffer = String_Alloc(buf[1]);

		trap_FS_FCloseFile(fh);
	} else {		
		return 1;
	}

	return 0;
}

void HUD_LoadClassBlurbs() {
	int i;

	uiInfo.Q3F_current_classQuote = -1;

	for(i = Q3F_CLASS_RECON; i < Q3F_CLASS_MAX; i++) {
		int res = HUD_LoadData(va("ui/data/%s_blurb_%s.cfg", bg_q3f_classlist[i]->commandstring, ui_language.string), &uiInfo.Q3F_class_quotes[i-Q3F_CLASS_RECON], MAX_BLURB_TEXT);
		if(res == 2) {
			Com_Printf("Class info length exceeded\n");
		} else if( res == 1) {
			Com_Printf("Class info file load failed: ui/data/%s_blurb_%s.cfg\n", bg_q3f_classlist[i]->commandstring, ui_language.string);
		}
	}
}

void HUD_LoadClassInvs() {
	int i;

	for(i = Q3F_CLASS_RECON; i < Q3F_CLASS_MAX; i++) {
		int res = HUD_LoadData(va("ui/data/%s_inv_%s.cfg", bg_q3f_classlist[i]->commandstring, ui_language.string), &uiInfo.Q3F_class_inv[i-Q3F_CLASS_RECON], 256);
		if(res == 2) {
			Com_Printf("Class inv length exceeded\n");
		} else if( res == 1) {
			Com_Printf("Class inv file load failed: ui/data/%s_inv_%s.cfg\n", bg_q3f_classlist[i]->commandstring, ui_language.string);
		}
	}
}

void HUD_LoadLanguageData() {
	HUD_LoadClassBlurbs();
	HUD_LoadClassInvs();
}

void HUD_DrawClassTitle(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font) {
	int cls = HUD_Q3F_GetChosenClass();
	char buffer[128];
	int res;

	bg_q3f_playerclass_t* pcls = bg_q3f_classlist[cls+1];

	Q_strncpyz(buffer, pcls->title, 128);

	res = HUD_ClassUnavailable(cls);
	if(res == 1) {
		strcat(buffer, " - Class Disabled");
	} else if (res == 2) {
		strcat(buffer, " - Class Full");
	}

	Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
}

void UI_DrawClassTitle(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font) {
	int cls = HUD_Q3F_GetChosenClass();
	char buffer[128];

	bg_q3f_playerclass_t* pcls = bg_q3f_classlist[cls+1];

	Q_strncpyz(buffer, pcls->title, 128);

	Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
}

int class_stats[Q3F_CLASS_MAX-Q3F_CLASS_RECON][3] = {
	{75,	17,		100},
	{90,	17,		64},
	{100,	67,		55},
	{90,	40,		64},
	{90,	33,		73},
	{100,	100,	45},
	{100,	50,		68},
	{90,	33,		68},
	{90,	33,		68},
	{50,	0,		68},
};

void HUD_DrawClassStat(rectDef_t *rect, vec4_t color1, vec4_t color2, float pos, int stat, qhandle_t shader) {
	int x, y, w, h;
	int cls = HUD_Q3F_GetChosenClass();
	float width;
	float scale;
	float diff;

	float value = class_stats[cls][stat%3];

	scale = (uiInfo.uiDC.realTime - uiInfo.Q3F_current_classQuote_time) * 0.001f;
	if(scale > 1.f) {
		scale = 1.f;
	}

	diff = value - uiInfo.Q3F_classstats_oldvalues[stat];

	value = uiInfo.Q3F_classstats_oldvalues[stat] + (diff * scale);

	uiInfo.Q3F_classstats_currentvalues[stat] = value;

	value -= pos * 10;
	if(value < 0) {
		value = 0;
	} else if(value > 10) {
		value = 10;
	}

	x = rect->x;
	y = rect->y;
	h = rect->h;
	w = 0;

	width = (value / 10);

	if(width) {
		w = rect->w * width;

		UI_FillRect(x, y, w, h, color1);
	}

	if(width != 1) {
		x += rect->w * width;
		w -= rect->w * width;

		UI_FillRect(x, y, w, h, color2);
	}

	if(shader) {
		UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, shader);
	}
	_UI_DrawRect(rect->x, rect->y, rect->w, rect->h, 1, colorBlack);
}

const char *class_choosetext[] = {
	"1. Recon",
	"2. Sniper",
	"3. Soldier",
	"4. Grenadier",
	"5. Paramedic",
	"6. Minigunner",
	"7. Flametrooper",
	"8. Agent",
	"9. Engineer",
	"0. Civilian",
	"R. Random"
};


void HUD_DrawClassButtonText(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font, int cls) {
	vec4_t clr;
	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];
	clr[3] = color[3];

	if(hud_allowClasses.string[cls] != '1') {
		clr[3] *= 0.2f;
	} else if(cls != HUD_Q3F_GetChosenClass()) {
		clr[3] = 1.0f;
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, clr, class_choosetext[cls], 0, 0, textStyle, font, textalignment);
}

void HUD_DrawClassButtonNew(rectDef_t *rect, int cls, qboolean mouseover) {
	vec4_t clr;
	qboolean enabled = qtrue;
	clr[0] = 1;
	clr[1] = 1;
	clr[2] = 1;
	clr[3] = 1;

	if(hud_allowClasses.string[cls] != '1') {
		enabled = qfalse;
		clr[3] *= 0.2f;
	} else if(cls != HUD_Q3F_GetChosenClass()) {
		clr[3] *= 0.6f;
	}

	trap_R_SetColor(clr);
	
	if(mouseover && enabled) {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[1]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[1]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[1]);
	} else {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[0]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[0]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[0]);
	}

	trap_R_SetColor( NULL );
}

void HUD_DrawClassButtonActive(rectDef_t *rect, qboolean mouseover) {
	vec4_t clr;
	clr[0] = 1;
	clr[1] = 1;
	clr[2] = 1;
	clr[3] = 1;

	trap_R_SetColor(clr);
	
	if(mouseover) {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[1]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[1]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[1]);
	} else {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[0]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[0]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[0]);
	}
	trap_R_SetColor( NULL );
}
									  
void HUD_DrawClassButton(rectDef_t *rect, vec4_t color, int cls) {
	vec4_t clr;
	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];
	clr[3] = color[3];

	if(hud_allowClasses.string[cls] != '1') {
		clr[3] *= 0.2f;
	} else if(cls != HUD_Q3F_GetChosenClass()) {
		clr[3] *= 0.5f;
	}

	UI_FillRect(rect->x, rect->y, rect->w, rect->h, clr);
}

 void HUD_DrawTeamButton(rectDef_t *rect, int team, qboolean mouseover) {
	uiClientState_t	cs;
	char	info[MAX_INFO_STRING];
	int		selteam;
	char buffer[16];
	int mask;
	qboolean enabled = qtrue;
	vec4_t clr;
	clr[0] = 1;
	clr[1] = 1;
	clr[2] = 1;
	clr[3] = 1;

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	selteam = atoi(Info_ValueForKey(info, "t"));

	trap_GetConfigString(CS_TEAMMASK, buffer, 16);
	mask = atoi(buffer);

	if(!(mask & (1 << team))) {
		clr[3] *= 0.4f;
		enabled = qfalse;
	} else if (team != selteam)
		clr[3] = 0.6f;

	trap_R_SetColor(clr);
	
	if(mouseover && enabled) {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[1]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[1]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[1]);
	} else {
		UI_DrawHandlePic(rect->x, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnleft[0]);
		UI_DrawHandlePic(rect->x + 16, rect->y, rect->w - 32, rect->h, uiInfo.uiDC.Assets.btnmid[0]);
		UI_DrawHandlePic(rect->x + rect->w - 16, rect->y, 16, rect->h, uiInfo.uiDC.Assets.btnright[0]);
	}

	trap_R_SetColor( NULL );
}

static const char* teamnames[4] = {
	"red",
	"blue",
	"yellow",
	"green"
};

void HUD_DrawFollowText(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team) {
	char buffer[16];
	int mask;
	vec4_t clr;
	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];
	clr[3] = color[3];


	trap_GetConfigString(CS_TEAMMASK, buffer, 16);
	mask = atoi(buffer);

	if(!(mask & (1 << team))) {
		clr[3] *= 0.2f;
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, clr, "1st Person", 0, 0, textStyle, font, textalignment);
}

void HUD_DrawChaseText(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team) {
	char buffer[16];
	int mask;
	vec4_t clr;
	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];
	clr[3] = color[3];


	trap_GetConfigString(CS_TEAMMASK, buffer, 16);
	mask = atoi(buffer);

	if(!(mask & (1 << team))) {
		clr[3] *= 0.2f;
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, clr, "3rd Person", 0, 0, textStyle, font, textalignment);
}


void HUD_DrawTeamButtonText(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int team) {
	char buffer[32];
	int mask;
	vec4_t clr;
	char tbuffer[64];
	clr[0] = color[0];
	clr[1] = color[1];
	clr[2] = color[2];
	clr[3] = color[3];


	trap_GetConfigString(CS_TEAMMASK, buffer, sizeof(buffer));
	mask = atoi(buffer);

	if(!(mask & (1 << team))) {
		Q_strncpyz(tbuffer, "Disabled", 64);
		clr[3] *= 0.2f;
	} else {
		trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[team - 1]), buffer, sizeof(buffer));
		Com_sprintf(tbuffer, 64, "%i. %s", team, buffer);
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, clr, tbuffer, 0, 0, textStyle, font, textalignment);
}

void HUD_DrawClassModel( rectDef_t *rect ) {
	if(!HUD_ClassUnavailable(HUD_Q3F_GetChosenClass())) {
		UI_DrawPlayer(rect->x, rect->y, rect->w, rect->h, &hud_pi, uiInfo.uiDC.realTime/2);
	}
}

void UI_DrawClassModel( rectDef_t *rect ) {
	UI_DrawPlayer(rect->x, rect->y, rect->w, rect->h, &hud_pi, uiInfo.uiDC.realTime/2);
}

void HUD_DrawClassHeadModel( rectDef_t *rect ) {
	if(!HUD_ClassUnavailable(HUD_Q3F_GetChosenClass())) {
		UI_DrawPlayerHead(rect->x, rect->y, rect->w, rect->h, &hud_head, uiInfo.uiDC.realTime/2);
	}
}

void HUD_DrawClassDisabled( rectDef_t *rect ) {
	if(HUD_ClassUnavailable(HUD_Q3F_GetChosenClass())) {
		UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("ui/gfx/hud/noway"));
	}
}

void HUD_DrawClassInv( rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	char buffer[1024];
	float y;
	char *s, *p;

	int cls = HUD_Q3F_GetChosenClass();

	Q_strncpyz(buffer, uiInfo.Q3F_class_inv[cls] ? uiInfo.Q3F_class_inv[cls] : "", 1024);

	HUD_WrapText(buffer, scale, parentfont, rect->w, 1024);

	y = rect->y + text_y;

	s = p = buffer;
	while(*p) {
		if(*p == '\n') {
			*p++ = '\0';

			Text_Paint(rect->x + text_x, y, scale, color, s, 0, 0, textStyle, parentfont, textalignment);

			y += Text_Height(s, scale, 0, parentfont) + 4;

			s = p;
		} else {
			p++;
		}
	}
}

void HUD_DrawClassInfo( rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont, int cls) {
	char out[6];
	char buf[2][3];
	int max, current;

	if(cls < 0 || cls >= 10) {
		return;
	}


	if((uiInfo.uiDC.realTime - uiInfo.displayClass) > 3000) {
		uiInfo.displayClass = uiInfo.uiDC.realTime;
		trap_Cmd_ExecuteText(EXEC_APPEND, "UpdateClassinfo\n");
	}

	memset(buf, 0, sizeof(buf));
	memset(out, 0, sizeof(out));

	buf[0][0] = hud_currentClasses.string[(cls * 2)];
	buf[0][1] = hud_currentClasses.string[(cls * 2)+1];
	buf[0][2] = '\0';

	buf[1][0] = hud_maxClasses.string[(cls * 2)];
	buf[1][1] = hud_maxClasses.string[(cls * 2)+1];
	buf[1][2] = '\0';

	current	= atoi(buf[0]);
	max		= atoi(buf[1]);

	Com_sprintf(out, 6, "%i", current);
	if(max) {
		strcat(out, va("/%i", max));
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, out, 0, 0, textStyle, parentfont, textalignment);
}

int HUD_ClassUnavailable (int cls) {
	char out[6];
	char buf[2][3];
	int max, current;
//	int cls = HUD_Q3F_GetChosenClass();

	if(hud_allowClasses.string[cls] != '1') {
		return 1;
	}

	memset(out, 0, sizeof(out));

	buf[0][0] = hud_currentClasses.string[(cls * 2)];
	buf[0][1] = hud_currentClasses.string[(cls * 2)+1];
	buf[0][2] = '\0';

	buf[1][0] = hud_maxClasses.string[(cls * 2)];
	buf[1][1] = hud_maxClasses.string[(cls * 2)+1];
	buf[1][2] = '\0';

	current	= atoi(buf[0]);
	max		= atoi(buf[1]);

	if(max && current == max) {
		return 2;
	}

	return 0;
}

void HUD_DrawMapInfoBlurb(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	char mapname[64];
	char buffer[1024];
	int i;
	int y;
	int index;
	mapInfo* mInfo = NULL;
	char *s, *p;
	gameIndexInfo_t* gIInfo = NULL;

	*buffer = '\0';

	trap_Cvar_VariableStringBuffer("mapname", mapname, 64);

	for(i = 0; i < uiInfo.mapCount; i++) {
		if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, mapname))) {
			mInfo = &uiInfo.mapList[i];
			break;
		}
	}

	if(!mInfo) {
		return;
	}

	index = trap_Cvar_VariableValue("g_gameindex");

	for(i = 0; i < mInfo->numGameIndicies; i++) {
		if(mInfo->gameIndiciesInfo[i].number == index) {
			gIInfo = &mInfo->gameIndiciesInfo[i];
		}
	}

	if(!gIInfo) {
		return;
	}

	Q_strncpyz(buffer, gIInfo->description, 1024);

	HUD_WrapText(buffer, scale, parentfont, rect->w, 1024);

	y = rect->y + text_y;

	s = p = buffer;
	while(*p) {
		if(*p == '\n') {
			*p++ = '\0';

			Text_Paint(rect->x + text_x, y, scale, color, s, 0, 0, textStyle, parentfont, textalignment);

			y += Text_Height(s, scale, 0, parentfont) + 4;

			s = p;
		} else {
			p++;
		}
	}
}

static void HUD_BuildPlayerList() {
	int		n, count;
	char	info[MAX_INFO_STRING];

	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	
	uiInfo.Q3F_playercount = 0;
	uiInfo.Q3F_playerindex = -1;

	for( n = 0; n < count; n++ ) {
		trap_GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );

		if (info[0]) {
			uiInfo.Q3F_clientNumber[uiInfo.Q3F_playercount] = n;

			Q_strncpyz( uiInfo.Q3F_playerNames[uiInfo.Q3F_playercount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
			Q_CleanStr( uiInfo.Q3F_playerNames[uiInfo.Q3F_playercount] );
			
			uiInfo.Q3F_playercount++;
		}
	}
}

void HUD_BuildPlayerIPList() {
	int		n, count;
	char	info[MAX_INFO_STRING];

	uiInfo.Q3F_playerIPcount = 0;

	count = trap_Argc() - 1;

	for( n = 0; n < count; n++ ) {
		trap_Argv(n+1, info, MAX_INFO_STRING );

		if (info[0]) {
			Q_strncpyz( uiInfo.Q3F_playerIPs[uiInfo.Q3F_playerIPcount], info, 64 );
			
			uiInfo.Q3F_playerIPcount++;
		}
	}
}

void HUD_BuildPlayerBANList() {
	int		n, count;
	char	info[MAX_INFO_STRING];

	uiInfo.Q3F_playerBANcount = 0;
	uiInfo.Q3F_playerBANindex = -1;

	count = trap_Argc() - 1;

	for( n = 0; n < count; n++ ) {
		trap_Argv(n+1, info, MAX_INFO_STRING );

		if (info[0]) {
			Q_strncpyz( uiInfo.Q3F_playerBANs[uiInfo.Q3F_playerBANcount], info, 64 );
			
			uiInfo.Q3F_playerBANcount++;
		}
	}
}

void HUD_DrawMapLvlShot( rectDef_t* rect ) {
	char mapname[64];
	int i;
	mapInfo* mInfo = NULL;

	trap_Cvar_VariableStringBuffer("mapname", mapname, 64);

	for(i = 0; i < uiInfo.mapCount; i++) {
		if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, mapname))) {
			mInfo = &uiInfo.mapList[i];
			break;
		}
	}

	if (mInfo && mInfo->levelShot == -1) {
		mInfo->levelShot = trap_R_RegisterShaderNoMip(mInfo->imageName);
	}

	UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, mInfo && mInfo->levelShot > 0 ? mInfo->levelShot : trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
}

void HUD_DrawMapName(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	char mapname[64];
	int i;
	mapInfo* mInfo = NULL;

	trap_Cvar_VariableStringBuffer("mapname", mapname, 64);

	for(i = 0; i < uiInfo.mapCount; i++) {
		if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, mapname))) {
			mInfo = &uiInfo.mapList[i];
			break;
		}
	}

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, mInfo ? mInfo->mapName : "Unknown Map", 0, 0, textStyle, parentfont, textalignment);
}

void HUD_DrawMatchString(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	char buffer[256];
	char msg[256];

	trap_GetConfigString(CS_WARMUP, buffer, 256);
	if(*buffer)
		Com_sprintf(msg, sizeof(msg), "Waiting to start match, everybody ready up!");
	else
		Com_sprintf(msg, sizeof(msg), "No match started");

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, msg, 0, 0, textStyle, parentfont, textalignment);
}

void HUD_DrawVoteString(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	char buffer[256];
	char timebuffer[256];

	trap_GetConfigString(CS_VOTE_STRING, buffer, 256);
	trap_GetConfigString(CS_VOTE_TIME, timebuffer, 256);

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, *timebuffer ? buffer : "No Vote In Progress", 0, 0, textStyle, parentfont, textalignment);
}

void HUD_DrawVoteTally(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont, qboolean yes) {
	char buffer[256];
	char timebuffer[256];

	trap_GetConfigString(yes ? CS_VOTE_YES : CS_VOTE_NO, buffer, 16);
	trap_GetConfigString(CS_VOTE_TIME, timebuffer, 16);

	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, *timebuffer ? buffer : "0", 0, 0, textStyle, parentfont, textalignment);
}

void UI_Q3F_BuildServerMaplist() {
	char buffer[4096];
	char *p, *s;

	trap_GetConfigString(CS_MAPLIST, buffer, 4096);
	s = buffer;
	p = strchr(s, ';');

	while(p) {
		*p++ = '\0';
		Q_strncpyz(uiInfo.Q3F_serverMaplist[uiInfo.Q3F_serverMaplistCount++], s, 128);

		s = p;
		p = strchr(p, ';');
	}

	uiInfo.Q3F_serverMaplistBuilt = qtrue;
}

void HUD_DrawTeamName( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, q3f_team_t team ) {
	char buffer[64];
	trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[team]), buffer, 64);
	Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
}

void HUD_DrawTeamCount( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, q3f_team_t team ) {
	char buffer[64];
	int count[4];
	int pings[4];
	int warn;

	memset(count, 0, sizeof(count));
	memset(pings, 0, sizeof(pings));

	trap_GetConfigString(CS_FORTS_TEAMPINGS, buffer, sizeof(buffer));
	sscanf(	buffer, "%i %i %i %i %i %i %i %i %i",
		&count[0], &pings[0],
		&count[1], &pings[1],
		&count[2], &pings[2],
		&count[3], &pings[3],
		&warn );

	Com_sprintf(buffer, 64, "%i", count[team]);
	Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
}

void HUD_ClearScoreInfo() {
	uiInfo.Q3F_scoreNum = 0;
//	uiInfo.statsvalid = qfalse;
}

void HUD_ParseScoreInfo() {
	int k;
	char buffer[64];

	k = uiInfo.Q3F_scoreNum;
	uiInfo.Q3F_uiScores[k].bot = qfalse;

	trap_Argv(1, uiInfo.Q3F_uiScores[k].name, 64);

	trap_Argv(2, buffer, 64);
	uiInfo.Q3F_uiScores[k].score = atoi(buffer);

	trap_Argv(3, buffer, 64);
	uiInfo.Q3F_uiScores[k].team = atoi(buffer);

	trap_Argv(4, buffer, 64);
//	uiInfo.Q3F_uiScores[k].class;

	trap_Argv(5, buffer, 64);
	uiInfo.Q3F_uiScores[k].time = atoi(buffer);

	trap_Argv(6, buffer, 64);
	if(Q_stricmp(buffer, "BOT") == 0)
		uiInfo.Q3F_uiScores[k].bot = qtrue;
	else
		uiInfo.Q3F_uiScores[k].ping = atoi(buffer);

	uiInfo.Q3F_scoreNum++;
}

void HUD_ParseTeamScoreInfo() {
	int i;
	char buffer[64];

	for(i = 1; i < 5; i++) {
		trap_Argv(i, buffer, 64);
		uiInfo.Q3F_teamScores[i-1] = atoi(buffer);
	}

	trap_Argv(5, buffer, 64);
	uiInfo.Q3F_scoreTeams = atoi(buffer);
}

int UI_intArgv(int argnum)
{
	char buffer[64];
	trap_Argv(argnum, buffer, 64);
	return atoi(buffer);
}

/*
=================
UI_ParseStats

=================
*/
void UI_ParseStats( void ) {
	int		i;

	uiInfo.caps = UI_intArgv( 1 );
	uiInfo.assists = UI_intArgv( 2 );
	uiInfo.defends = UI_intArgv( 3 );
	uiInfo.teamkills = UI_intArgv( 4 );

	memset( uiInfo.stats, 0, sizeof( uiInfo.stats ) );
	for ( i = 0 ; i < MAX_ETF_STATS ; i++ ) {
		uiInfo.stats[i].shots =	UI_intArgv(i * 4 + 5);
		uiInfo.stats[i].hits =	UI_intArgv(i * 4 + 6);
		uiInfo.stats[i].kills =	UI_intArgv(i * 4 + 7);
		uiInfo.stats[i].deaths=	UI_intArgv(i * 4 + 8);
	}

	uiInfo.statsvalid = qtrue;
}

/*
=================
UI_ParseAwards

=================
*/
void UI_ParseAwards( void ) {
	int		i;

	trap_Argv(1, uiInfo.teamkiller, 128);
	trap_Argv(2, uiInfo.capper, 128);
	trap_Argv(3, uiInfo.terminator, 128);
	trap_Argv(4, uiInfo.cannonfodder, 128);

	memset( uiInfo.awards, 0, sizeof( uiInfo.awards ) );
	for ( i = 0 ; i < WP_NUM_WEAPONS ; i++ ) {
		trap_Argv( i + 5, uiInfo.awards[i], 64);
	}

	uiInfo.awardsvalid = qtrue;
}

void HUD_CalculateTeamTieString( int *order, int first, int last, char* buffer, int size )
{
	// Return a string like "Red, Blue and Yellow have 5"

	int i;
	char *s;
	char namebuffer[64];

	for( i = first; i <= last; i++ )
	{
		switch( order[i] )
		{
			case Q3F_TEAM_RED:		s = "Red";		break;
			case Q3F_TEAM_BLUE:		s = "Blue";		break;
			case Q3F_TEAM_YELLOW:	s = "Yellow";	break;
			case Q3F_TEAM_GREEN:	s = "Green";	break;
			default:				s = "?";
		}
		trap_Cvar_VariableStringBuffer(va("cg_%steam", s), namebuffer, 64);
		Q_strcat( buffer, size, namebuffer );
		if( i < last )
			Q_strcat( buffer, size, (i < (last - 1)) ? ", " : " and " );
	}

	if( last > first )
	{
		if( uiInfo.Q3F_teamScores[last] == uiInfo.Q3F_teamScores[first] )
			s = " tied at ";
		else if( last - first > 1 )
			s = first ? " each had " : " led with ";
		else s = first ? " both had " : " were tied at ";
	}
	else s = first ? " had " : " won with ";
	Q_strcat( buffer, size, s );
	Q_strcat( buffer, size, va( "%d", uiInfo.Q3F_teamScores[order[first]-Q3F_TEAM_RED] ) );
}

void HUD_DrawEndGameTeamScores( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	int order[4];
	int i, j, temp;
	char teamposbuff[1024];

	// Find our teams
	for( i = 0; i < 4; i++ )
		order[i] = (uiInfo.Q3F_scoreTeams & (1<<(Q3F_TEAM_RED+i))) ? (Q3F_TEAM_RED+i) : Q3F_TEAM_NUM_TEAMS;

	// Sort  them
	for( i = 0; i < 3; i++ )
	{
		if( order[i+1] < Q3F_TEAM_NUM_TEAMS && uiInfo.Q3F_teamScores[order[i]-Q3F_TEAM_RED] < uiInfo.Q3F_teamScores[order[i+1]-Q3F_TEAM_RED] )
		{
			temp = order[i];
			order[i] = order[i+1];
			order[i+1] = temp;
			i = -1;		// Reset loop (it's a swap-sort)
		}
	}

	*teamposbuff = '\0';

		// Make a string
	for( i = 0; i < 4 && order[i] < Q3F_TEAM_NUM_TEAMS; i = j )
	{
		for( j = i+1; j < 4 &&
			(uiInfo.Q3F_teamScores[order[j]-Q3F_TEAM_RED] >= uiInfo.Q3F_teamScores[order[i]-Q3F_TEAM_RED]) &&
			(order[j] < Q3F_TEAM_NUM_TEAMS) ; j++ );
			// i to j-1 are on the same score, so...
		HUD_CalculateTeamTieString( order, i, j-1, teamposbuff, sizeof(teamposbuff) );
		if( j < 4 && order[j] < Q3F_TEAM_NUM_TEAMS )
			Q_strcat( teamposbuff, sizeof(teamposbuff), ", " );
	}

	Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, teamposbuff, 0, 0, textStyle, font, textalignment);
}

void HUD_DrawChatBox (rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char		buffer[1024];
	char		*s, *p;
	rectDef_t	r;
	vec4_t		newcolor;
	qboolean	useClr = qfalse;
	char		clrCode = 0;
	char		cacheClrCode = 0;
	int i;

	p = buffer;

//	memcpy(&newcolor, &color, sizeof(vec3_t));
	newcolor[0] = newcolor[1] = newcolor[2] = newcolor[3] = 1.0f;

	for(i = 0; i < MAX_UICHAT_STRINGS; i++) {
		if(uiInfo.Q3F_uiChatTimes[i] <= uiInfo.uiDC.realTime) {
			break;
		}
	}

	if(!i) {
		return;
	}

	*p = '\0';
	for(i-- ; i >= 0; i--) {
		strcpy(p, &uiInfo.Q3F_uiChat[i*MAX_SAY_TEXT]);
		p += strlen(p);
		*p++ = '\n';
	}
	*p = '\0';

	r.x = rect->x + text_x;
	r.y = rect->y + text_y;
	
	s = p = buffer;
	while(*p) {
		if(Q_IsColorStringPtr(p)) {
			clrCode = *(p+1);
			p++;
		} else if(*p == '\n') {
			*p++ = '\0';

			Text_Paint_MaxWidth( r.x , r.y, scale, newcolor, useClr ? va("^%c%s", cacheClrCode, s) : s, 0, 0, textStyle, font, textalignment, rect->w);

			if(clrCode) {
				cacheClrCode = clrCode;
				useClr = qtrue;
			}

			r.y += Text_Height(s, scale, 0, font)+2;
			s = p;
		}
		else {
			p++;
		}
	}
}

void HUD_DrawMapVoteName( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int num ) {
	char *p;
	char buffer[64];
	int i, j;
	mapInfo* mInfo = NULL;
	gameIndexInfo_t* gIInfo = NULL;

	if( num >= uiInfo.mapSelectCount) {
		return;
	}

	Q_strncpyz(buffer, uiInfo.mapSelectNames[num], 64);
	p = strchr(buffer, '+');
	if(p) {
		*p++ = '\0';

		for(i = 0; i < uiInfo.mapCount; i++) {
			if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, buffer))) {
				mInfo = &uiInfo.mapList[i];
				break;
			}
		}

		if(!mInfo) {
			p = buffer;
		} else {

			j = atoi(p);

			for(i = 0; i < mInfo->numGameIndicies; i++) {
				if(mInfo->gameIndiciesInfo[i].number == j) {
					gIInfo = &mInfo->gameIndiciesInfo[i];
				}
			}

			if(!gIInfo) {
				p =  buffer;
			} else {
				p = gIInfo->name+3;
			}
		}
	} else {
		p = buffer;
	}

	Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, p, 0, 0, textStyle, font, textalignment);	
}

void HUD_DrawMapVoteTally(	rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int num ) {

	if( num >= uiInfo.mapSelectCount) {
		return;
	}

	Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, va("%d", uiInfo.mapSelectTally[num]), 0, 0, textStyle, font, textalignment);	
}

void HUD_DrawMapVoteTallyBar( rectDef_t* rect, int num )
{
	vec4_t		hcolor;
	float		valuefact;
	int			i, mapSelectTotal = 0;

	for( i = 0; i < uiInfo.mapSelectCount; i ++ ) {
		mapSelectTotal += uiInfo.mapSelectTally[i];
	}

	if(mapSelectTotal > 0) {
		valuefact = ((float)uiInfo.mapSelectTally[num]) / mapSelectTotal;
	} else {
		valuefact = 0.f;
	}

	hcolor[2] = 0;
	hcolor[3] = 0.3f;
	if ( valuefact <= 0.5f ) {
		hcolor[0] = 1;
		hcolor[1] = (valuefact*2);
	}
	else if ( valuefact <= 1.0f ) {
		hcolor[0] = 1 - ( ( valuefact - 0.5f ) * 2 );
		hcolor[1] = 1;
	}

	UI_FillRect( rect->x, rect->y, ((rect->w - 10) * valuefact) + 10, rect->h, hcolor );
}


void HUD_DrawMapVoteLevelshot( rectDef_t* rect ) {
	int map = trap_Cvar_VariableValue("hud_chosenvotemap");
	int i;
	mapInfo* mInfo = NULL;
	char buffer[64];
	char* p;

	if(map < 0 || map > uiInfo.mapSelectCount) {
	} else {
		Q_strncpyz(buffer, uiInfo.mapSelectNames[map], 64);
		p = strchr(buffer, '+');
		if(p) {
			*p = '\0';
		}

		for(i = 0; i < uiInfo.mapCount; i++) {
			if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, buffer))) {
				mInfo = &uiInfo.mapList[i];
				break;
			}
		}

		if (mInfo && mInfo->levelShot == -1) {
			mInfo->levelShot = trap_R_RegisterShaderNoMip(mInfo->imageName);
		}		
	}
	UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, mInfo && mInfo->levelShot > 0 ? mInfo->levelShot : trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
}

void HUD_DrawVoteMapInfoBlurb(rectDef_t* rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t* parentfont) {
	int map = trap_Cvar_VariableValue("hud_chosenvotemap");
	char mapname[64];
	char buffer[1024];
	int i, j;
	int y;
	mapInfo* mInfo = NULL;
	char *s, *p;
	gameIndexInfo_t* gIInfo = NULL;

	*buffer = '\0';

	if(map < 0 || map > uiInfo.mapSelectCount) {
		return;
	}

	Q_strncpyz(mapname, uiInfo.mapSelectNames[map], 64);
	p = strchr(mapname, '+');
	if(p) {
		*p++ = '\0';
		j = atoi(p);
	} else {
		j = 1;
	}

	for(i = 0; i < uiInfo.mapCount; i++) {
		if(!(Q_stricmp(uiInfo.mapList[i].mapLoadName, mapname))) {
			mInfo = &uiInfo.mapList[i];
			break;
		}
	}

	if(!mInfo) {
		return;
	}

	for(i = 0; i < mInfo->numGameIndicies; i++) {
		if(mInfo->gameIndiciesInfo[i].number == j) {
			gIInfo = &mInfo->gameIndiciesInfo[i];
		}
	}

	if(!gIInfo) {
		return;
	}

	Q_strncpyz(buffer, gIInfo->description, 1024);

	HUD_WrapText(buffer, scale, parentfont, rect->w, 1024);

	y = rect->y + text_y;

	s = p = buffer;
	while(*p) {
		if(*p == '\n') {
			*p++ = '\0';

			Text_Paint(rect->x + text_x, y, scale, color, s, 0, 0, textStyle, parentfont, textalignment);

			y += Text_Height(s, scale, 0, parentfont) + 4;

			s = p;
		} else {
			p++;
		}
	}
}

