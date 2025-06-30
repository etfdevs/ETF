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

#include "../api/et/q_shared.h"
#include "../api/et/tr_types.h"
#include "../api/et/ui_public.h"
#include "../api/et/keycodes.h"
#include "../game/bg_public.h"
#include "../game/bg_q3f_playerclass.h"
#include "../cgame/cg_q3f_f2r.h"
#include "ui_shared.h"

// global display context

#define EXTERN_UI_CVAR
	#include "ui_cvar.h"
#undef EXTERN_UI_CVAR

qboolean UI_Q3F_RegisterClassModels( int classNum );

//
// ui_main.c
//
void UI_Report(void);
void UI_Load(void);
void UI_LoadMenus(const char *menuFile, qboolean reset);
void UI_SetActiveMenu( uiMenuCommand_t menu );
void UI_ShowInGame(void);
void UI_ShowEndGame(void);
void UI_ParseMapInfo( void );
void UI_ParseHudInfo( void );
void UI_ParseHudVariantInfo( void );
void UI_LoadHudPreview( void );
void UI_RegisterCvars( void );
void UI_UpdateCvars( void );
void UI_DrawConnectScreen( qboolean overlay );
void HUD_BuildPlayerIPList(void);
void HUD_BuildPlayerBANList(void);
void UI_Q3F_BuildServerMaplist(void);
qboolean UI_PlayerOnTeam(void);
qboolean UI_PlayerHasClass(void);
void HUD_ParseScoreInfo(void);
void UI_ParseStats( void );
void UI_ParseAwards( void );
void HUD_ParseTeamScoreInfo(void);
void HUD_ClearScoreInfo(void);
void UI_ReadBindings( void );

//
// ui_atoms.c
//

// new ui stuff
#define MAX_MAPS 128
#define MAX_ADDRESSLENGTH		64
#define MAX_DISPLAY_SERVERS		2048
#define MAX_SERVERSTATUS_LINES	128
#define MAX_SERVERSTATUS_TEXT	1024
#define MAX_FOUNDPLAYER_SERVERS	16
#define MAX_MODS 64
#define MAX_DEMOS 2048 // 256
#define MAX_CONFIGS 256 // 256
#define MAX_MOVIES 256

#define DEMO_DIRECTORY "demos"
#define DEMO_EXTENSION "dm_"
#define MAX_DEMOLIST (MAX_DEMOS * MAX_QPATH)

#define MAX_MAPLIST (MAX_MAPS * MAX_QPATH)
#define MAX_MODLIST (MAX_MODS * MAX_QPATH)
#define MAX_MOVIELIST (MAX_MOVIES * MAX_QPATH)
#define MAX_CONFIGLIST (MAX_CONFIGS * MAX_QPATH)

//slothy
#define MAX_HUDS 64
#define MAX_HUDLIST (MAX_HUDS * MAX_QPATH)

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

typedef struct serverStatus_s {
	int			refreshtime;
	int			sortKey;
	int			sortDir;
	qboolean	refreshActive;
	int			currentServer;
	int			displayServers[MAX_DISPLAY_SERVERS];
	int			numDisplayServers;
	int			numInvalidServers; // borked servers
	int			numIncompatibleServers; // non-ETF servers, or has punkbuster still
	int			numqueriedservers;
	
	int		maxservers;	

	int		numPlayersOnServers;
	int		numTotalPlayers;
	int		nextDisplayRefresh;
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
	mapInfo_t mapList[MAX_MAPS];

	int skillIndex;

	modInfo_t modList[MAX_MODS];
	int modCount;
	int modIndex;

	char demoList[MAX_DEMOS][MAX_QPATH];
	int demoCount;
	int demoIndex;
	int loadedDemos;

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

	qboolean inGameLoad;

// RR2DO2
	int			Q3F_BackModelStatus;			// status: 0 closed, 1 open, 2 closing, 3 opening, 4 reopening (close and open in one move), 5 setting new current
	int			Q3F_BackModelRotateEndTime;		// time rotate ends
	int			Q3F_BackModelRotateSpeed;		// cache to let rotation finish properly
	const char	*Q3F_BackModelMenuToOpen;		// menu to open after menu is rotated open
	const char	*Q3F_BackModelMenuCurrent;		// current open menu

// djbob
	int			Q3F_BackModelStartupTime;

	//sfxHandle_t	Q3F_BackModelOpenSound;
	//sfxHandle_t Q3F_BackModelReOpenSound;
	//sfxHandle_t	Q3F_BackModelCloseSound;
	//sfxHandle_t	Q3F_BackModelStartupSound;

	//sfxHandle_t Q3F_TransitionStaticSound1;
	//sfxHandle_t Q3F_TransitionStaticSound2;
	//sfxHandle_t Q3F_TransitionStaticSound3;
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


void			UI_Init( void );
void			UI_Shutdown( void );
void			UI_KeyEvent( int key, qboolean down );
void			UI_MouseEvent( int dx, int dy );
void			UI_Refresh( int realtime );
qboolean		UI_ConsoleCommand( int realTime );
float			UI_ClampCvar( float min, float max, float value );
void			UI_DrawNamedPic( float x, float y, float width, float height, const char *picname );
void			UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ); 
void			UI_DrawAdjustedPic( float x, float y, float w, float h, qhandle_t hShader );
void			UI_FillRect( float x, float y, float width, float height, const float *color );
void			UI_DrawRect( float x, float y, float width, float height, const float *color );
void			UI_DrawTopBottom(float x, float y, float w, float h);
void			UI_DrawSides(float x, float y, float w, float h);
void			UI_UpdateScreen( void );
void			UI_SetColor( const float *rgba );
qboolean 		UI_CursorInRect (int x, int y, int width, int height);
void			UI_AdjustFrom640( float *x, float *y, float *w, float *h );
const char			*UI_Argv( int arg );
const char			*UI_Cvar_VariableString( const char *var_name );
// RR2DO2
void			UI_Q3F_DrawProgress( rectDef_t *rect, int value, int maxvalue, vec4_t color, qhandle_t shader );
// RR2DO2

void	HUD_LoadLanguageData(void);
int		HUD_Q3F_GetChosenClass(void);
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

#ifdef API_ET

//
// ui_syscalls.c
//
void			trap_Print( const char *string );
void NORETURN	trap_Error( const char *string );
int				trap_Milliseconds( void );
void			trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags, int extflags );
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
void			trap_LAN_SaveCachedServers(void);
int				trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void			trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int				trap_LAN_AddServer(int source, const char *name, const char *addr);
void			trap_LAN_RemoveServer(int source, const char *addr);
int				trap_LAN_GetServerPing( int source, int n );
int				trap_LAN_ServerIsVisible( int source, int n);
int				trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void			trap_LAN_SaveCachedServers(void);
void			trap_LAN_LoadCachedServers(void);
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

// extension interface
extern	qboolean engine_is_ete;
extern  qboolean intShaderTime;
extern  qboolean linearLight;
extern	qboolean removeCommand;

qboolean trap_GetValue( char *value, int valueSize, const char *key );
void trap_R_AddRefEntityToScene2( const refEntity_t *re );
void trap_R_AddLinearLightToScene( const vec3_t start, const vec3_t end, float intensity, float r, float g, float b );
void trap_RemoveCommand( const char *cmdName );
extern int dll_com_trapGetValue;
extern int cvar_notabcomplete;
extern int cvar_nodefault;
extern int cvar_archive_nd;
extern int cvar_developer;
extern int dll_trap_R_AddRefEntityToScene2;
extern int dll_trap_R_AddLinearLightToScene;
extern int dll_trap_RemoveCommand;

#endif
