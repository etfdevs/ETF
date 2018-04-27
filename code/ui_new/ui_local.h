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

#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "q_shared.h"
#include "tr_types.h"
#include "ui_public.h"
#include "keycodes.h"
#include "../game/bg_public.h"
#include "../game/bg_q3f_playerclass.h"
#include "../cgame/cg_q3f_f2r.h"
#include "ui_shared.h"

// global display context

extern vmCvar_t	ui_browserMaster;
extern vmCvar_t	ui_browserGameType;
extern vmCvar_t	ui_browserSortKey;
extern vmCvar_t	ui_browserShowFull;
extern vmCvar_t	ui_browserShowEmpty;
extern vmCvar_t	ui_browserShowPasswordProtected;
extern vmCvar_t	ui_browserShowVersion;

extern vmCvar_t	ui_brassTime;
extern vmCvar_t	ui_drawCrosshair;
extern vmCvar_t	ui_drawCrosshairNames;
extern vmCvar_t	ui_marks;

extern vmCvar_t	ui_server1;
extern vmCvar_t	ui_server2;
extern vmCvar_t	ui_server3;
extern vmCvar_t	ui_server4;
extern vmCvar_t	ui_server5;
extern vmCvar_t	ui_server6;
extern vmCvar_t	ui_server7;
extern vmCvar_t	ui_server8;
extern vmCvar_t	ui_server9;
extern vmCvar_t	ui_server10;
extern vmCvar_t	ui_server11;
extern vmCvar_t	ui_server12;
extern vmCvar_t	ui_server13;
extern vmCvar_t	ui_server14;
extern vmCvar_t	ui_server15;
extern vmCvar_t	ui_server16;

extern vmCvar_t	ui_cdkey;
extern vmCvar_t	ui_cdkeychecked;

//extern vmCvar_t	ui_captureLimit;
//extern vmCvar_t	ui_fragLimit;
//extern vmCvar_t	ui_gameType;
//extern vmCvar_t	ui_netGameType;
//extern vmCvar_t	ui_joinGameType;
extern vmCvar_t	ui_netSource;
extern vmCvar_t	ui_serverFilterType;
extern vmCvar_t	ui_dedicated;
extern vmCvar_t	ui_menuFiles;
extern vmCvar_t ui_ingameMenuFiles;
extern vmCvar_t	ui_currentMap;
extern vmCvar_t	ui_currentNetMap;
extern vmCvar_t	ui_mapIndex;
extern vmCvar_t	ui_selectedPlayer;
extern vmCvar_t	ui_selectedPlayerName;
extern vmCvar_t	ui_lastServerRefresh_0;
extern vmCvar_t	ui_lastServerRefresh_1;
extern vmCvar_t	ui_lastServerRefresh_2;
extern vmCvar_t	ui_lastServerRefresh_3;
extern vmCvar_t	ui_smallFont;
extern vmCvar_t	ui_bigFont;
extern vmCvar_t ui_serverStatusTimeOut;

// RR2DO2
//extern vmCvar_t ui_menuRotateSpeed;
extern vmCvar_t	r_vertexLight;
// RR2DO2

// djbob
extern vmCvar_t hud_allowClasses;
extern vmCvar_t	hud_maxClasses;
extern vmCvar_t	hud_currentClasses;

extern vmCvar_t	ui_specifyServer;
extern vmCvar_t	ui_specifyPort;

extern vmCvar_t	cg_ScoreSnapshot;
// djbob

// slothy
extern vmCvar_t cg_crosshairAlpha;
extern vmCvar_t cg_crosshairAlphaAlt;
extern vmCvar_t cg_crosshairColor;
extern vmCvar_t cg_crosshairColorAlt;
extern vmCvar_t	cg_crosshairSize;

extern vmCvar_t ui_addSpecifyFavorites;
extern vmCvar_t ui_checkversion;
// slothy

qboolean UI_Q3F_RegisterClassModels( int classNum );

//
// ui_qmenu.c
//

#define RCOLUMN_OFFSET			( BIGCHAR_WIDTH )
#define LCOLUMN_OFFSET			(-BIGCHAR_WIDTH )

#define SLIDER_RANGE			10
#define	MAX_EDIT_LINE			256

#define MAX_MENUDEPTH			8
//#define MAX_MENUITEMS			96

#define MTYPE_NULL				0
#define MTYPE_SLIDER			1	
#define MTYPE_ACTION			2
#define MTYPE_SPINCONTROL		3
#define MTYPE_FIELD				4
#define MTYPE_RADIOBUTTON		5
#define MTYPE_BITMAP			6	
#define MTYPE_TEXT				7
#define MTYPE_SCROLLLIST		8
#define MTYPE_PTEXT				9
#define MTYPE_BTEXT				10

#define QMF_BLINK				0x00000001
#define QMF_SMALLFONT			0x00000002
#define QMF_LEFT_JUSTIFY		0x00000004
#define QMF_CENTER_JUSTIFY		0x00000008
#define QMF_RIGHT_JUSTIFY		0x00000010
#define QMF_NUMBERSONLY			0x00000020	// edit field is only numbers
#define QMF_HIGHLIGHT			0x00000040
#define QMF_HIGHLIGHT_IF_FOCUS	0x00000080	// steady focus
#define QMF_PULSEIFFOCUS		0x00000100	// pulse if focus
#define QMF_HASMOUSEFOCUS		0x00000200
#define QMF_NOONOFFTEXT			0x00000400
#define QMF_MOUSEONLY			0x00000800	// only mouse input allowed
#define QMF_HIDDEN				0x00001000	// skips drawing
#define QMF_GRAYED				0x00002000	// grays and disables
#define QMF_INACTIVE			0x00004000	// disables any input
#define QMF_NODEFAULTINIT		0x00008000	// skip default initialization
#define QMF_OWNERDRAW			0x00010000
#define QMF_PULSE				0x00020000
#define QMF_LOWERCASE			0x00040000	// edit field is all lower case
#define QMF_UPPERCASE			0x00080000	// edit field is all upper case
#define QMF_SILENT				0x00100000

// callback notifications
#define QM_GOTFOCUS				1
#define QM_LOSTFOCUS			2
#define QM_ACTIVATED			3

typedef struct _tag_menuframework
{
	int	cursor;
	int cursor_prev;

	int	nitems;
	void *items[MAX_MENUITEMS];

	void (*draw) (void);
	sfxHandle_t (*key) (int key);

	qboolean	wrapAround;
	qboolean	fullscreen;
	qboolean	showlogo;
} menuframework_s;

typedef struct
{
	int type;
	const char *name;
	int	id;
	int x, y;
	int left;
	int	top;
	int	right;
	int	bottom;
	menuframework_s *parent;
	int menuPosition;
	unsigned flags;

	void (*callback)( void *self, int event );
	void (*statusbar)( void *self );
	void (*ownerdraw)( void *self );
} menucommon_s;

typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
	int		maxchars;
} mfield_t;

typedef struct
{
	menucommon_s	generic;
	mfield_t		field;
} menufield_s;

typedef struct 
{
	menucommon_s generic;

	float minvalue;
	float maxvalue;
	float curvalue;

	float range;
} menuslider_s;

typedef struct
{
	menucommon_s generic;

	int	oldvalue;
	int curvalue;
	int	numitems;
	int	top;
		
	const char **itemnames;

	int width;
	int height;
	int	columns;
	int	seperation;
} menulist_s;

typedef struct
{
	menucommon_s generic;
} menuaction_s;

typedef struct
{
	menucommon_s generic;
	int curvalue;
} menuradiobutton_s;

typedef struct
{
	menucommon_s	generic;
	char*			focuspic;	
	char*			errorpic;
	qhandle_t		shader;
	qhandle_t		focusshader;
	int				width;
	int				height;
	float*			focuscolor;
} menubitmap_s;

typedef struct
{
	menucommon_s	generic;
	char*			string;
	int				style;
	float*			color;
} menutext_s;

extern void			Menu_Cache( void );
extern void			Menu_Focus( menucommon_s *m );
extern void			Menu_AddItem( menuframework_s *menu, void *item );
extern void			Menu_AdjustCursor( menuframework_s *menu, int dir );
extern void			Menu_Draw( menuframework_s *menu );
extern void			*Menu_ItemAtCursor( menuframework_s *m );
extern sfxHandle_t	Menu_ActivateItem( menuframework_s *s, menucommon_s* item );
extern void			Menu_SetCursor( menuframework_s *s, int cursor );
extern void			Menu_SetCursorToItem( menuframework_s *m, void* ptr );
extern sfxHandle_t	Menu_DefaultKey( menuframework_s *s, int key );
extern void			Bitmap_Init( menubitmap_s *b );
extern void			Bitmap_Draw( menubitmap_s *b );
extern void			ScrollList_Draw( menulist_s *l );
extern sfxHandle_t	ScrollList_Key( menulist_s *l, int key );
extern sfxHandle_t	menu_in_sound;
extern sfxHandle_t	menu_move_sound;
extern sfxHandle_t	menu_out_sound;
extern sfxHandle_t	menu_buzz_sound;
extern sfxHandle_t	menu_null_sound;
extern sfxHandle_t	weaponChangeSound;
extern vec4_t		menu_text_color;
extern vec4_t		menu_grayed_color;
extern vec4_t		menu_dark_color;
extern vec4_t		menu_highlight_color;
extern vec4_t		menu_red_color;
extern vec4_t		menu_black_color;
extern vec4_t		menu_dim_color;
extern vec4_t		color_black;
extern vec4_t		color_white;
extern vec4_t		color_yellow;
extern vec4_t		color_blue;
extern vec4_t		color_orange;
extern vec4_t		color_red;
extern vec4_t		color_dim;
extern vec4_t		name_color;
extern vec4_t		list_color;
extern vec4_t		listbar_color;
extern vec4_t		text_color_disabled; 
extern vec4_t		text_color_normal;
extern vec4_t		text_color_highlight;

extern char	*ui_medalNames[];
extern char	*ui_medalPicNames[];
extern char	*ui_medalSounds[];

//
// ui_mfield.c
//
extern void			MField_Clear( mfield_t *edit );
extern void			MField_KeyDownEvent( mfield_t *edit, int key );
extern void			MField_CharEvent( mfield_t *edit, int ch );
extern void			MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color );
extern void			MenuField_Init( menufield_s* m );
extern void			MenuField_Draw( menufield_s *f );
extern sfxHandle_t	MenuField_Key( menufield_s* m, int* key );

//
// ui_main.c
//
void UI_Report();
void UI_Load();
void UI_LoadMenus(const char *menuFile, qboolean reset);
void _UI_SetActiveMenu( uiMenuCommand_t menu );
int UI_AdjustTimeByGame(int time);
void UI_ShowPostGame(qboolean newHigh);
void UI_ShowInGame();
void UI_ShowEndGame();
void UI_ShowCustomMenu();
void UI_ClearScores();
void UI_ParseMapInfo( void );
void UI_ParseHudInfo( void );
void UI_ParseHudVariantInfo( void );
void UI_LoadHudPreview( void );
void UI_RegisterCvars( void );
void UI_UpdateCvars( void );
void UI_DrawConnectScreen( qboolean overlay );
void HUD_BuildPlayerIPList();
void HUD_BuildPlayerBANList();
void UI_Q3F_BuildServerMaplist();
qboolean UI_PlayerOnTeam();
qboolean UI_PlayerHasClass();
void HUD_ParseScoreInfo();
void UI_ParseStats( void );
void UI_ParseAwards( void );
void HUD_ParseTeamScoreInfo();
void HUD_ClearScoreInfo();
void UI_ReadBindings( void );

//
// ui_atoms.c
//
// this is only used in the old ui, the new ui has it's own version
typedef struct {
	int					frametime;
	int					realtime;
	int					cursorx;
	int					cursory;
	glconfig_t 	glconfig;
	qboolean		debug;
	qhandle_t		whiteShader;
	qhandle_t		menuBackShader;
	qhandle_t		menuBackShader2;
	qhandle_t		menuBackNoLogoShader;
	qhandle_t		charset;
	qhandle_t		charsetProp;
	qhandle_t		charsetPropGlow;
	qhandle_t		charsetPropB;
	qhandle_t		cursor;
	qhandle_t		rb_on;
	qhandle_t		rb_off;
	float				scale;
	float				bias;
	qboolean		demoversion;
	qboolean		firstdraw;
} uiStatic_t;


// new ui stuff
#define UI_NUMFX 7
#define MAX_HEADS 64
#define MAX_ALIASES 64
#define MAX_HEADNAME  32
#define MAX_TEAMS 64
#define MAX_GAMETYPES 16
#define MAX_MAPS 128
#define MAX_SPMAPS 16
#define PLAYERS_PER_TEAM 5
//#define MAX_PINGREQUESTS		32
#define MAX_ADDRESSLENGTH		64
#define MAX_HOSTNAMELENGTH		22
#define MAX_MAPNAMELENGTH		16
#define MAX_STATUSLENGTH		64
#define MAX_LISTBOXWIDTH		59
#define UI_FONT_THRESHOLD		0.1
#define MAX_DISPLAY_SERVERS		2048
#define MAX_SERVERSTATUS_LINES	128
#define MAX_SERVERSTATUS_TEXT	1024
#define MAX_FOUNDPLAYER_SERVERS	16
#define TEAM_MEMBERS 5
#define GAMES_ALL			0
#define GAMES_FFA			1
#define GAMES_TEAMPLAY		2
#define GAMES_TOURNEY		3
#define GAMES_CTF			4
#define MAPS_PER_TIER 3
#define MAX_TIERS 16
#define MAX_MODS 64
#define MAX_DEMOS 256
#define MAX_CONFIGS 256
#define MAX_MOVIES 256
#define MAX_PLAYERMODELS 256

//slothy
#define MAX_HUDS 64

/*typedef struct {
	const char *name;
	const char *imageName;
	qhandle_t headImage;
	qboolean female;
} characterInfo;

typedef struct {
	const char *name;
	const char *ai;
	const char *action;
} aliasInfo;

typedef struct {
	const char *teamName;
	const char *imageName;
	const char *teamMembers[TEAM_MEMBERS];
	qhandle_t teamIcon;
	qhandle_t teamIcon_Metal;
	qhandle_t teamIcon_Name;
	int cinematic;
} teamInfo;*/

/*typedef struct {
	const char *tierName;
	const char *maps[MAPS_PER_TIER];
	int gameTypes[MAPS_PER_TIER];
	qhandle_t mapHandles[MAPS_PER_TIER];
} tierInfo;*/

typedef struct serverFilter_s {
	const char *description;
	const char *basedir;
} serverFilter_t;

typedef struct {
	char	adrstr[MAX_ADDRESSLENGTH];
	int		start;
} pinglist_t;


typedef struct serverStatus_s {
	pinglist_t pingList[MAX_PINGREQUESTS];
	int		numqueriedservers;
	int		currentping;
	int		nextpingtime;
	int		maxservers;
	int		refreshtime;
	int		numServers;
	int		sortKey;
	int		sortDir;
	int		lastCount;
	qboolean refreshActive;
	int		currentServer;
	int		displayServers[MAX_DISPLAY_SERVERS];
	int		numDisplayServers;
	int		numPlayersOnServers;
	int		numTotalPlayers;
	int		nextDisplayRefresh;
	int		nextSortTime;
	qhandle_t currentServerPreview;
	int		currentServerCinematic;
	int		motdLen;
	int		motdWidth;
	int		motdPaintX;
	int		motdPaintX2;
	int		motdOffset;
	int		motdTime;
	char	motd[MAX_STRING_CHARS];
} serverStatus_t;


typedef struct {
	char		adrstr[MAX_ADDRESSLENGTH];
	char		name[MAX_ADDRESSLENGTH];
	int			startTime;
	int			serverNum;
	qboolean	valid;
} pendingServer_t;

typedef struct {
	int num;
	pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
} pendingServerStatus_t;

typedef struct {
	char address[MAX_ADDRESSLENGTH];
	char *lines[MAX_SERVERSTATUS_LINES][4];
	char text[MAX_SERVERSTATUS_TEXT];
	char pings[MAX_CLIENTS * 3];
	int numLines;
} serverStatusInfo_t;

typedef struct {
	const char *modName;
	const char *modDescr;
} modInfo_t;

// RR2DO2

typedef enum {
	Q3F_BM_CLOSED = 0,
	Q3F_BM_OPENED,
	Q3F_BM_CLOSING,
	Q3F_BM_OPENING,
	Q3F_BM_REOPENING,
	Q3F_BM_NEWCURRENT,
	Q3F_BM_STARTUP
} Q3F_backmodelState_t;

// djbob
#define Q3F_BM_STARTUPTIME			2000.f
#define Q3F_BM_INNER_START_DIST		250
#define Q3F_BM_BACK_START_DIST		150
#define Q3F_BM_FRONT_START_DIST		160
#define Q3F_BM_FRONT_NO_ROTATIONS	3
#define	Q3F_BM_BACK_STOPSCALE		0.8f
#define	Q3F_BM_FRONT_STOPSCALE		0.4f
// djbob

#define MAX_BLURB_TEXT 1024

typedef struct {
	char	name[64];
	int		score;
	int		time;
	int		team;
	int		ping;
	qboolean bot;
} ui_score_t;

#define MAX_UICHAT_STRINGS	5
#define ETF_MAPSELECT_SELECTCOUNT 10

//slothy
#define ETF_HUDCOUNT 20
#define ETF_HUDVARCOUNT 10

#define MAX_ETF_STATS	31

typedef struct stat_s {
	int		shots, hits;
	int		kills, deaths;
	int		given, taken;
} stat_t;
//end slothy

typedef struct {
	displayContextDef_t uiDC;
	int newHighScoreTime;
	int newBestTime;
	int showPostGameTime;
	qboolean newHighScore;
	qboolean demoAvailable;
	qboolean soundHighScore;
	
	int redBlue;
	int playerCount;
	int myTeamCount;
	int teamIndex;
	int playerRefresh;
	int playerIndex;
	int playerNumber; 
	char playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	char teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	int teamClientNums[MAX_CLIENTS];

	int mapCount;
	mapInfo mapList[MAX_MAPS];

	int skillIndex;

	modInfo_t modList[MAX_MODS];
	int modCount;
	int modIndex;

	const char *demoList[MAX_DEMOS];
	int demoCount;
	int demoIndex;

	const char *cfgList[MAX_CONFIGS];
	int cfgCount;
	int cfgIndex;

	const char *movieList[MAX_MOVIES];
	int movieCount;
	int movieIndex;
	int previewMovie;

	serverStatus_t serverStatus;

	// for the showing the status of a server
	char serverStatusAddress[MAX_ADDRESSLENGTH];
	serverStatusInfo_t serverStatusInfo;
	int nextServerStatusRefresh;

	// to retrieve the status of server to find a player
	pendingServerStatus_t pendingServerStatus;
	char findPlayerName[MAX_STRING_CHARS];
	char foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	char foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	char foundPlayerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	int currentFoundPlayerServer;
	int numFoundPlayerServers;
	int nextFindPlayerRefresh;

	int currentCrosshair;
	int startPostGameTime;

/*	int				q3HeadCount;
	char			q3HeadNames[MAX_PLAYERMODELS][64];
	qhandle_t		q3HeadIcons[MAX_PLAYERMODELS];
	int				q3SelectedHead;*/

	int effectsColor;

	qboolean inGameLoad;

// RR2DO2
	int			Q3F_BackModelStatus;			// status: 0 closed, 1 open, 2 closing, 3 opening, 4 reopening (close and open in one move), 5 setting new current
	int			Q3F_BackModelRotateEndTime;		// time rotate ends
	int			Q3F_BackModelRotateSpeed;		// cache to let rotation finish properly
	const char	*Q3F_BackModelMenuToOpen;		// menu to open after menu is rotated open
	const char	*Q3F_BackModelMenuCurrent;		// current open menu

// djbob
	int			Q3F_BackModelStartupTime;

	sfxHandle_t	Q3F_BackModelOpenSound;
	sfxHandle_t Q3F_BackModelReOpenSound;
	sfxHandle_t	Q3F_BackModelCloseSound;
	sfxHandle_t	Q3F_BackModelStartupSound;

	sfxHandle_t Q3F_TransitionStaticSound1;
	sfxHandle_t Q3F_TransitionStaticSound2;
	sfxHandle_t Q3F_TransitionStaticSound3;
// RR2DO2

// djbob
	configData_t*	ETF_CurrentBindingTable;
	int				ETF_CurrentBindingTable_Size;
	int				ETF_CurrentBindingTable_Pos;

	// slothy
	int				ETF_CurBindMove;
	int				ETF_CurBindShoot;
	int				ETF_CurBindMisc;	
	int				ETF_CurBindComs;
	// slothy

	int				Q3F_GeneralSettingsTable_Pos;

	int				Q3F_SelectedClass;
	int				Q3F_ConfigReadMode;

	configData_t*	Q3F_CurrentSystemTable;
	int				Q3F_CurrentSystemTable_Size;
	int				Q3F_CurrentSystemTable_Pos;

	int				Q3F_CurrentFavServer_Pos;

	char			Q3F_blurb_buffer[MAX_BLURB_TEXT];
	const char		(*Q3F_class_quotes[10]);
	const char		(*Q3F_class_inv[10]);
	int				Q3F_current_classQuote;
	int				Q3F_current_classQuote_lines;
	int				Q3F_current_classQuote_time;
	float			Q3F_classstats_oldvalues[3];
	float			Q3F_classstats_currentvalues[3];

	//slothy
	int				ETF_current_mapQuote;
	int				ETF_current_mapQuote_lines;
	
	int				ETF_current_hudNameQuote;
	int				ETF_current_hudVarQuote;
	int				ETF_current_hudQuote_lines;

	int				ETF_current_weapQuote_lines;
	int				ETF_current_weapQuote_num;

	int				eventHandling; // for handling messagemode through ui

	qhandle_t		modelcache[Q3F_CLASS_MAX][3]; // model loadin ui side
	F2RDef_t		*f2rcache[Q3F_CLASS_MAX][3];
	qhandle_t		skincache[Q3F_CLASS_MAX][3];
	qhandle_t		modeliconcache[Q3F_CLASS_MAX];
	byte			skincolours[Q3F_CLASS_MAX][4][3][3];
	sfxHandle_t		soundcache[Q3F_CLASS_MAX][MAX_CUSTOM_SOUNDS];
	sfxHandle_t		classnamesounds[Q3F_CLASS_MAX];

	qhandle_t		weaponModels[WP_NUM_WEAPONS];
	qhandle_t		weaponBarrelModels[WP_NUM_WEAPONS];

	int				Q3F_playercount;
	int				Q3F_playerindex;
	char			Q3F_playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	int				Q3F_clientNumber[MAX_CLIENTS];

	int				Q3F_playerIPcount;
	char			Q3F_playerIPs[MAX_CLIENTS][64];

	int				Q3F_playerBANcount;
	int				Q3F_playerBANindex;
	char			Q3F_playerBANs[MAX_CLIENTS][64];

	qboolean		Q3F_serverMaplistBuilt;
	char			Q3F_serverMaplist[MAX_MAPS][64];
	int				Q3F_serverMaplistCount;
	int				Q3F_serverMaplistIndex;

	int				Q3F_scoreTeams;
	int				Q3F_scoreNum;
	ui_score_t		Q3F_uiScores[MAX_CLIENTS];
	int				Q3F_teamScores[4];

	char			Q3F_uiChat[MAX_SAY_TEXT*MAX_UICHAT_STRINGS];
	int				Q3F_uiChatTimes[MAX_UICHAT_STRINGS];

	int				mapSelectCount;
	char			mapSelectNames	[ETF_MAPSELECT_SELECTCOUNT][64];
	qboolean		mapSelectPresent[ETF_MAPSELECT_SELECTCOUNT];
	int				mapSelectTally	[ETF_MAPSELECT_SELECTCOUNT];
	char			mapMenuName		[MAX_NAME_LENGTH];

	qboolean		ScoreSnapshotTaken;
	qboolean		ScoreFetched;
	// djbob

   //keeg for xhair
	vec4_t		xhairColor;
	vec4_t		xhairColorAlt;

	//slothy for model drawing
	qhandle_t ModelImages[Q3F_CLASS_MAX];

	// slothy for Hud files
	int			hudCount;
	char		hudFiles[ETF_HUDCOUNT][64];
	int			hudVariationCount;
	char		hudVariants[ETF_HUDVARCOUNT][64];

	// slothy stats
	stat_t		stats[MAX_ETF_STATS];
	char		awards[WP_NUM_WEAPONS][64];
	char		teamkiller[128], capper[128];
	char		terminator[128], cannonfodder[128];
	int			caps, assists, defends, teamkills;
	qboolean	statsvalid;
	qboolean	awardsvalid;

	int			displayClass;

	// slothy server browser
	int			numServerPlayers;
	char		ServerPlayers[MAX_CLIENTS][MAX_NAME_LENGTH * 3];
	int			ServerPlayerScores[MAX_CLIENTS];
	int			ServerPlayerPings[MAX_CLIENTS];
	int			nextServerPlayerRefresh;

}	uiInfo_t;

extern uiInfo_t uiInfo;

enum {
  UI_EVENT_NONE,
//  UI_EVENT_MESSAGEMODE,
};

//
// ui_players.c
//

//FIXME ripped from cg_local.h
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
	animation_t	*animblock;			// Golliwog: The 'animation' block (so we know to instant-switch)
} lerpFrame_t;

typedef struct {
	// model info
	qhandle_t		legsModel;
	qhandle_t		legsSkin;
	lerpFrame_t		legs;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;
	lerpFrame_t		torso;

	qhandle_t		headModel;
	qhandle_t		headSkin;

//	animation_t		animations[MAX_TOTALANIMATIONS];

	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;
	vec3_t			flashDlightColor;
	int				muzzleFlashTime;

	// currently in use drawing parms
	vec3_t			viewAngles;
	vec3_t			moveAngles;
	weapon_t		currentWeapon;
	int				legsAnim;
	int				torsoAnim;

	// animation vars
	weapon_t		weapon;
	weapon_t		lastWeapon;
	weapon_t		pendingWeapon;
	int				weaponTimer;
	int				pendingLegsAnim;
	int				torsoAnimationTimer;

	int				pendingTorsoAnim;
	int				legsAnimationTimer;

	qboolean		newModel;

	qboolean		barrelSpinning;
	float			barrelAngle;
	int				barrelTime;

	int				realWeapon;

	// djbob
	int				classnum;
} playerInfo_t;


extern void			UI_Init( void );
extern void			UI_Shutdown( void );
extern void			UI_KeyEvent( int key );
extern void			UI_MouseEvent( int dx, int dy );
extern void			UI_Refresh( int realtime );
extern qboolean		UI_ConsoleCommand( int realTime );
extern float		UI_ClampCvar( float min, float max, float value );
extern void			UI_DrawNamedPic( float x, float y, float width, float height, const char *picname );
extern void			UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ); 
extern void			UI_DrawAdjustedPic( float x, float y, float w, float h, qhandle_t hShader );
extern void			UI_FillRect( float x, float y, float width, float height, const float *color );
extern void			UI_DrawRect( float x, float y, float width, float height, const float *color );
extern void			UI_DrawTopBottom(float x, float y, float w, float h);
extern void			UI_DrawSides(float x, float y, float w, float h);
extern void			UI_UpdateScreen( void );
extern void			UI_SetColor( const float *rgba );
extern void			UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
extern void			UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color );
extern float		UI_ProportionalSizeScale( int style );
extern void			UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
extern int			UI_ProportionalStringWidth( const char* str );
extern void			UI_DrawString( int x, int y, const char* str, int style, vec4_t color );
extern void			UI_DrawChar( int x, int y, int ch, int style, vec4_t color );
extern qboolean 	UI_CursorInRect (int x, int y, int width, int height);
extern void			UI_AdjustFrom640( float *x, float *y, float *w, float *h );
//extern void			UI_DrawTextBox (int x, int y, int width, int lines);
extern qboolean		UI_IsFullscreen( void );
extern void			UI_SetActiveMenu( uiMenuCommand_t menu );
extern void			UI_PushMenu ( menuframework_s *menu );
extern void			UI_PopMenu (void);
extern void			UI_ForceMenuOff (void);
extern char			*UI_Argv( int arg );
extern char			*UI_Cvar_VariableString( const char *var_name );
extern void			UI_Refresh( int time );
extern void			UI_KeyEvent( int key );
extern void			UI_StartDemoLoop( void );
extern qboolean		m_entersound;
void UI_SetEventHandling(int mode);
void UI_LoadBestScores(const char *map, int game);
// RR2DO2
extern void			UI_Q3F_DrawProgress( rectDef_t *rect, int value, int maxvalue, vec4_t color, qhandle_t shader );
// RR2DO2

void	HUD_LoadLanguageData();
int		HUD_Q3F_GetChosenClass();
void	HUD_SetupClassQuoteBuffer(float scale, fontStruct_t* font, float w);
char*	UI_GetBlurbLine(int i);

// Slothy
void	UI_SetupMapQuoteBuffer(float scale, fontStruct_t* font, float w);
void	UI_SetupHudQuoteBuffer(float scale, fontStruct_t* font, float w);
void	UI_SetupWeapQuoteBuffer(float scale, fontStruct_t* font, float w);
// Slothy

void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNumber, int cls );
void UI_DrawPlayer( float x, float y, float w, float h, playerInfo_t *pi, int time );
void UI_DrawPlayerHead( float x, float y, float w, float h, playerInfo_t *pi, int time );

#ifdef API_Q3

//
// ui_syscalls.c
//
void			trap_Print( const char *string );
void			trap_Error( const char *string );
int				trap_Milliseconds( void );
void			trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void			trap_Cvar_Update( vmCvar_t *vmCvar );
void			trap_Cvar_Set( const char *var_name, const char *value );
float			trap_Cvar_VariableValue( const char *var_name );
void			trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void			trap_Cvar_SetValue( const char *var_name, float value );
void			trap_Cvar_Reset( const char *name );
void			trap_Cvar_Create( const char *var_name, const char *var_value, int flags );
void			trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize );
int				trap_Argc( void );
void			trap_Argv( int n, char *buffer, int bufferLength );
void			trap_Cmd_ExecuteText( int exec_when, const char *text );	// don't use EXEC_NOW!
int				trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void			trap_FS_Read( void *buffer, int len, fileHandle_t f );
void			trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void			trap_FS_FCloseFile( fileHandle_t f );
int				trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
qhandle_t		trap_R_RegisterModel( const char *name );
qhandle_t		trap_R_RegisterSkin( const char *name );
qhandle_t		trap_R_RegisterShaderNoMip( const char *name );
qhandle_t		trap_R_RealRegisterModel( const char *name );
void			trap_R_ClearScene( void );
void			trap_R_AddRefEntityToScene( const refEntity_t *re );
void			trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
//void			trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void			trap_R_RenderScene( const refdef_t *fd );
void			trap_R_SetColor( const float *rgba );
void			trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void			trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
void			trap_UpdateScreen( void );
//int				trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName );
void			trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
sfxHandle_t		trap_S_RegisterSound( const char *sample, qboolean compressed );
void			trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void			trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void			trap_Key_SetBinding( int keynum, const char *binding );
qboolean		trap_Key_IsDown( int keynum );
qboolean		trap_Key_GetOverstrikeMode( void );
void			trap_Key_SetOverstrikeMode( qboolean state );
void			trap_Key_ClearStates( void );
int				trap_Key_GetCatcher( void );
void			trap_Key_SetCatcher( int catcher );
void			trap_GetClipboardData( char *buf, int bufsize );
void			trap_GetClientState( uiClientState_t *state );
void			trap_GetGlconfig( glconfig_t *glconfig );
int				trap_GetConfigString( int index, char* buff, int buffsize );
int				trap_LAN_GetServerCount( int source );
void			trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void			trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int				trap_LAN_GetServerPing( int source, int n );
int				trap_LAN_GetPingQueueCount( void );
void			trap_LAN_ClearPing( int n );
void			trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void			trap_LAN_GetPingInfo( int n, char *buf, int buflen );
void			trap_LAN_LoadCachedServers();
void			trap_LAN_SaveCachedServers();
void			trap_LAN_MarkServerVisible(int source, int n, qboolean visible);
int				trap_LAN_ServerIsVisible( int source, int n);
qboolean		trap_LAN_UpdateVisiblePings( int source );
int				trap_LAN_AddServer(int source, const char *name, const char *addr);
void			trap_LAN_RemoveServer(int source, const char *addr);
void			trap_LAN_ResetPings(int n);
int				trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
int				trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
int				trap_MemoryRemaining( void );
void			trap_GetCDKey( char *buf, int buflen );
void			trap_SetCDKey( char *buf );
void			trap_R_RegisterFont(const char *pFontname, int pointSize, fontInfo_t *font);
void			trap_S_StopBackgroundTrack( void );
//void			trap_S_StartBackgroundTrack( const char *intro, const char *loop);
int				trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status		trap_CIN_StopCinematic(int handle);
e_status		trap_CIN_RunCinematic (int handle);
void			trap_CIN_DrawCinematic (int handle);
void			trap_CIN_SetExtents (int handle, int x, int y, int w, int h);
int				trap_RealTime(qtime_t *qtime);
void			trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean		trap_VerifyCDKey( const char *key, const char *chksum);

#endif // API_Q3
#ifdef API_ET

//
// ui_syscalls.c
//
void			trap_Print( const char *string );
void			trap_Error( const char *string );
int				trap_Milliseconds( void );
void			trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void			trap_Cvar_Update( vmCvar_t *vmCvar );
void			trap_Cvar_Set( const char *var_name, const char *value );
float			trap_Cvar_VariableValue( const char *var_name );
void			trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void			trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void			trap_Cvar_SetValue( const char *var_name, float value );
void			trap_Cvar_Reset( const char *name );
void			trap_Cvar_Create( const char *var_name, const char *var_value, int flags );
void			trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize );
int				trap_Argc( void );
void			trap_Argv( int n, char *buffer, int bufferLength );
void			trap_Cmd_ExecuteText( int exec_when, const char *text );	// don't use EXEC_NOW!
void			trap_AddCommand( const char *cmdName );
int				trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void			trap_FS_Read( void *buffer, int len, fileHandle_t f );
void			trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void			trap_FS_FCloseFile( fileHandle_t f );
int				trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int				trap_FS_Delete(const char *filename);
qhandle_t		trap_R_RealRegisterModel( const char *name );
qhandle_t		trap_R_RegisterSkin( const char *name );
qhandle_t		trap_R_RegisterShaderNoMip( const char *name );
void			trap_R_ClearScene( void );
void			trap_R_AddRefEntityToScene( const refEntity_t *re );
void			trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
//void			trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void			trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible);
void			trap_R_RenderScene( const refdef_t *fd );
void			trap_R_SetColor( const float *rgba );
void			trap_R_Add2dPolys( polyVert_t* verts, int numverts, qhandle_t hShader );
void			trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void			trap_R_DrawRotatedPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );
void			trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
void			trap_UpdateScreen( void );
//int				trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void			trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
sfxHandle_t		trap_S_RegisterSound( const char *sample, qboolean compressed );
void			trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void			trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void			trap_Key_KeysForBinding( const char* binding, int* key1, int* key2 );
void			trap_Key_SetBinding( int keynum, const char *binding );
qboolean		trap_Key_IsDown( int keynum );
qboolean		trap_Key_GetOverstrikeMode( void );
void			trap_Key_SetOverstrikeMode( qboolean state );
void			trap_Key_ClearStates( void );
int				trap_Key_GetCatcher( void );
void			trap_Key_SetCatcher( int catcher );
void			trap_GetClipboardData( char *buf, int bufsize );
void			trap_GetClientState( uiClientState_t *state );
void			trap_GetGlconfig( glconfig_t *glconfig );
int				trap_GetConfigString( int index, char* buff, int buffsize );
int				trap_LAN_GetServerCount( int source );			// NERVE - SMF
int				trap_LAN_GetLocalServerCount( void );
void			trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen );
int				trap_LAN_GetGlobalServerCount( void );
void			trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen );
int				trap_LAN_GetPingQueueCount( void );
void			trap_LAN_ClearPing( int n );
void			trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void			trap_LAN_GetPingInfo( int n, char *buf, int buflen );
int				trap_MemoryRemaining( void );

// NERVE - SMF - multiplayer traps
qboolean		trap_LAN_UpdateVisiblePings( int source );
void			trap_LAN_MarkServerVisible(int source, int n, qboolean visible);
void			trap_LAN_ResetPings(int n);
void			trap_LAN_SaveCachedServers();
int				trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void			trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int				trap_LAN_AddServer(int source, const char *name, const char *addr);
void			trap_LAN_RemoveServer(int source, const char *addr);
int				trap_LAN_GetServerPing( int source, int n );
int				trap_LAN_ServerIsVisible( int source, int n);
int				trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void			trap_LAN_SaveCachedServers();
void			trap_LAN_LoadCachedServers();
qboolean		trap_LAN_ServerIsInFavoriteList( int source, int n );

void			trap_SetPbClStatus( int status );								// DHM - Nerve
void			trap_SetPbSvStatus( int status );								// TTimo


// -NERVE - SMF

void			trap_GetCDKey( char *buf, int buflen );
void			trap_SetCDKey( char *buf );
void			trap_R_RegisterFont(const char *pFontname, int pointSize, fontInfo_t *font);
void			trap_S_StopBackgroundTrack( void );
//void			trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime);
void			trap_S_FadeAllSound( float targetvol, int time, qboolean stopsound );
int				trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status		trap_CIN_StopCinematic(int handle);
e_status		trap_CIN_RunCinematic (int handle);
void			trap_CIN_DrawCinematic (int handle);
void			trap_CIN_SetExtents (int handle, int x, int y, int w, int h);
int				trap_RealTime(qtime_t *qtime);
void			trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean		trap_VerifyCDKey( const char *key, const char *chksum);
qboolean		trap_GetLimboString( int index, char *buf );			// NERVE - SMF
void			trap_CheckAutoUpdate( void );							// DHM - Nerve
void			trap_GetAutoUpdate( void );								// DHM - Nerve

void			trap_openURL( const char *url ); // TTimo
void			trap_GetHunkData( int* hunkused, int* hunkexpected );


char*			trap_TranslateString( const char *string );				// NERVE - SMF - localization

#endif // API_ET

//
// ui_shared_syscalls.c
//
void			trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
int				trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void			trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime );

// new ui 

#define ASSET_BACKGROUND "uiBackground"

#endif
