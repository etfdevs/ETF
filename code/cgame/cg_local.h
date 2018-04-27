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

#ifndef	__CG_LOCAL_H
#define	__CG_LOCAL_H

#include "q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"
#include "../game/bg_q3f_playerclass.h"
#include "../game/bg_q3f_splines.h"
#include "../game/bg_q3f_flyby.h"
#include "cg_q3f_flyby.h"
#include "cg_q3f_spirit.h"
#include "cg_q3f_f2r.h"
#include "cg_q3f_scriptlib.h"
#include "../ui_new/ui_shared.h"

//disable deprecated warnings
#if defined(_MSC_VER)
#pragma warning( disable : 4996 )
#endif

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

//#define Q3F_WATER

#define MAX_EXTENDED_WEAPONS	4
#define MAX_Q3F_ALERTS			3

#define Q3F_SCOPE_FADEINTIME	150.f
#define Q3F_SCOPE_FADEOUTTIME	250.f

#define Q3F_SENTRYCAM_BLINKTIME	300.f

#define Q3F_BLOODFLASH_TIME		400.f

#define Q3F_ALERTICON_TIME		3000.f

#define CG_FONT_THRESHOLD	0.1

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	AUTOZOOM_TIME		800			// Golliwog: Time to autozoom
//#define	ZOOM_TIME		100				// JT - Make that much faster.
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		39	// 80
#define TEAMCHAT_HEIGHT		10
#define	TEAMCHAT_BYTEWIDTH	(3 * TEAMCHAT_WIDTH + 1)	// To allow for colour codes

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10
#define	NUM_FLAME_SPRITES		45

//CG_Trails.c related defines
#define	STYPE_STRETCH	0
#define	STYPE_REPEAT	1

#define	TJFL_FADEIN		(1<<0)
#define	TJFL_CROSSOVER	(1<<1)
#define	TJFL_NOCULL		(1<<2)
#define	TJFL_FIXDISTORT	(1<<3)
#define TJFL_SPARKHEADFLARE (1<<4)
#define	TJFL_NOPOLYMERGE	(1<<5)

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"sarge"
#define	DEFAULT_TEAM_MODEL		"sarge"
#define	DEFAULT_TEAM_HEAD		"sarge"

// Demo controls
#define DEMO_THIRDPERSONUPDATE  0
#define DEMO_RANGEDELTA         6
#define DEMO_ANGLEDELTA         4

// on-hand firing weapons
#define NG_FLASH_RADIUS			200 // original: 300
#define MINI_FLASH_RADIUS		200
#define AR_FLASH_RADIUS			200
#define WEAPON_FLASH_RADIUS		300
#define WEAPON_FLASH_RADIUS_MOD	31
#define WEAPON_FLASH_INTENSITY 1.0

extern const char *teamnames[4];

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

typedef enum { 
	Q3F_ALERT_NULL,
	Q3F_ALERT_WAVES,
	Q3F_ALERT_PAIN,
	Q3F_ALERT_TRIGGER,
	Q3F_ALERT_BUILD,
	Q3F_ALERT_HE_SET,
	Q3F_ALERT_HE_BLOW,
	Q3F_ALERT_JUMPPAD,
	Q3F_ALERT_SENTRY,
	Q3F_ALERT_EXPLOSION,
	Q3F_ALERT_FIRE,
	Q3F_ALERT_GRENBOUNCE,
	Q3F_ALERT_GUNFIRE,
	Q3F_ALERT_MINIGUN,
	Q3F_ALERT_NAILGREN,
	Q3F_ALERT_DOOR,				// slothy
	Q3F_ALERT_LIFT,				// slothy	
	Q3F_ALERT_MAX,
} q3f_alert_t;


	// Locations stored in the map.
typedef struct cg_q3f_location_s {
	vec3_t pos;
	char *str;
} cg_q3f_location_t;

	// Light- and lensflares stored in the map
typedef enum {
	FL_FLARE,
	FL_LENSFLARE,
	FL_LENSBLIND,
	FL_LENSFLAREBLIND,
} flare_t;

typedef struct cg_q3f_flare_s {
	vec3_t pos;
	float radius;
	float intensity;
	int rotation;
	vec3_t color;
	qhandle_t shader;
	flare_t type;

	int flareFadeTime;
	float flareFadeValue;
} cg_q3f_flare_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
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
	lerpFrame_t		legs, torso, hands, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1

	int				EffectFlags;
	// Golliwog: Minigun effects
	int				minigunTime;
	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;

	// Golliwog: Apparent team and class (e.g. after spy effects)
	int				visibleTeam, visibleClass;
	// Golliwog.
} playerEntity_t;

#define PE_EF_STUNNED		0x1
#define PE_EF_DISEASED		0x2
#define PE_EF_GASSED		0x4
#define PE_EF_FLASHED		0x8
#define PE_EF_TRANQED		0x10
#define PE_EF_LEGWOUNDS		0x20

//=================================================

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				ejectBrassTime;		// ^^
	int				previousEvent;
	int				previousEventSequence;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	/* Ensiform - Unused */
//	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_FADE,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_CONST_RGB,
	LE_SCOREPLUM,
	LE_SHRAPNEL,
	LE_EXPAND_SHRAPNEL,
	LE_EXPAND_FADE_RING,
	LE_BEAM,
	LE_Q3F_PANELRADARBLIP,
	LE_LIGHT,
	LE_BULLET_EXPLOSION,
	LE_NAPALM_FLAME,
	LE_DEBUG_BOX,
	LE_SCALE_FADE,
} leType_t;

typedef enum {
	LEF_TUMBLE				= 0x0001,			// tumble over time, used for ejecting shells

	//LEF_PUFF_DONT_SCALE		= 0x0010,			// do not scale size over time

	LEF_MARK_BURN			= 0x0100,
	LEF_MARK_BLOOD			= 0x0200,
	LEF_MARKS				= 0x0700,

	LEF_SOUND_BRASS			= 0x1000,
	LEF_SOUND_BLOOD			= 0x2000,
	LEF_SOUNDS				= 0x7000,
} leFlag_t;

typedef enum {
	LEMFT_TEMP,
	LEMFT_NORMAL,
	LEMFT_ALPHA
} leMarkFadeType_t;

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			endtime,fadetime;
	qhandle_t	markShader;
	leMarkFadeType_t	fadeType;	// fade alpha instead of rgb
	int			extFlags;			// Copy of player flags to indicate what side of mirror they're on.
	byte		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	float			fadeRate;
	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius, radiusrate;

	float			light;
	vec3_t			lightColor;

	refEntity_t		refEntity;
} localEntity_t;

//======================================================================

typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				team;
	int				flags;
} score_t;

typedef struct flamerstruct_s {
	vec3_t org;								// trajectory origin
	vec3_t pos, oldpos;						// current pos
	vec3_t delta;
	int starttime;
	int dietime;

	struct flamerstruct_s *next;
	struct flamerstruct_s *prev;
} flamerstruct_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	q3f_team_t		team;			//RR2DO2
	int				cls;			// Golliwog: Player's current (real) class

	qboolean		shoutcaster;

	int				score;			// updated by score servercmds
	int				health;			// you only get this info about your teammates
	int				armor;

	int				medkitUsageTime;
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;

	int				breathPuffTime;

	gender_t		gender;			// from model
	int				flashtagnumber;
// RR2DO2: footprints
	qboolean		lastfootdown; // qtrue = right, qfalse = left
	orientation_t	tag_pack;
	orientation_t	torso_refent;
	float			torso_backlerp;
} clientInfo_t;

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t *, refEntity_t * );

	vec4_t			trailColor;
	int				trailRadius;
	int				trailStep;
	int				trailDuration;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
	qboolean		loopFireSound;
	sfxHandle_t		wallSound[2][3];	// Golliwog: melee weapon hitting wall.
	sfxHandle_t		fleshSound[3];		// Golliwog: melee weapon hitting flesh.

	F2RDef_t		*F2RScript;		// F2RScript for handsModel
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
	qhandle_t		icon_df;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20
#define MAX_BUFFERSOUNDS	4

typedef struct bufferedSound_s
{
	qboolean valid;
	int soundnum;
	int maxtime;
} bufferedSound_t;

typedef struct altObit_s
{
	int attacklen, viclen;
	char attacker[34];
	char victim[34];
	char mod[20];
	int modlen;
	int endtime;
} altObit_t;

//======================================================================

	// Map information parsing fills in this structure.
typedef struct {
	const char *key;
	char *value;
	int valueSize;
} cg_q3f_mapinfo_t;


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after
 
#define MAX_PREDICTED_EVENTS	16
//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)

#define MAX_BASICCHAT_STRINGS	4
#define MAX_TEAMCHAT_STRINGS	8

#define MAX_OBIT_STRINGS	4

typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;
	
	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
//	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	qboolean	renderingFlyBy;		// RR2DO2: during flyby
	qboolean	rendering2ndRefDef;	// RR2DO2: 2ndary refdef (sentrycam/portalsky) rendering
	qboolean	renderingSkyPortal;	// RR2DO2: during portalsky rendering

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// RR2DO2
	refdef_t	*currentrefdef;			// set to main refdef of the scene

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;
	qboolean	autoZoomed;				// Golliwog: For sniper auto-zoom
	int			autoZoomTime;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[4];			// Enough for all 4 teams
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	qboolean	showQuickMenu;
	int			nextQuickMenu;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];
	char		spectatorList[MAX_STRING_CHARS];		// list of names
	int			spectatorLen;							// length of list
	float		spectatorWidth;							// width in device units
	int			spectatorTime;							// next time to offset
	int			spectatorPaintX;						// current paint x
	int			spectatorPaintX2;						// current paint x
	int			spectatorOffset;						// current offset from start
	int			spectatorPaintLen; 						// current offset from start

	// centerprinting
	qboolean centerPrintRealigned;
	qboolean centerPrintSolid;
	char centerPrintText[1024];
	int centerPrintTime;
	qboolean centerprintWrapped;

	char basicChat		[MAX_SAY_TEXT*MAX_BASICCHAT_STRINGS];
	char teamChat		[MAX_SAY_TEXT*MAX_TEAMCHAT_STRINGS];

	int basicChatTimes	[MAX_BASICCHAT_STRINGS];
	int teamChatTimes	[MAX_TEAMCHAT_STRINGS];

	int basicChatLinesUsed;
	int teamChatLinesUsed;

	char basicChatBuffer[1024];
	char teamChatBuffer[1024];
	char centreprintBuffer[1024];

	qboolean basicChatLinesChanged;
	qboolean teamChatLinesChanged;

	qboolean basicChatLinesWrapped;
	qboolean teamChatLinesWrapped;

	char obit		[MAX_SAY_TEXT*MAX_OBIT_STRINGS];
	int obitTimes	[MAX_OBIT_STRINGS];
	int obitLinesUsed;
	int obitBuffer	[1024];
	qboolean obitLinesChanged;
	qboolean obitLinesWrapped;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;		// Golliwog: Slow down ID system
	int			crosshairSentryLevel;		// Golliwog: Display sentry stats (which may vanish outside PVS)
	int			crosshairSentrySet;
	int			crosshairSentryHealth;
	int			crosshairSentryShells;
	int			crosshairSentryRockets;
	int			crosshairSentryBored;
	int			crosshairSupplyLevel;
	int			crosshairSupplyHealth;
	int			crosshairSupplyShells;
	int			crosshairSupplyNails;
	int			crosshairSupplyCells;
	int			crosshairSupplyRockets;
	int			crosshairSupplyArmor;
	int			crosshairSupplyGrenades;
	int			lastCrosshairCheck;			// digibob: caching this now for a little bit

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// warmup countdown
	int			warmup;
	int			warmupCount;
	int			matchState;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int			nextOrbitTime;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	int			nextMenuAutoTime;		// Golliwog: Timer to stop flooding for auto-menu checks
	qboolean	ceaseFire;				// Golliwog: If set, no shooting/grenades/pickups

	// Golliwog: Grenade handling variables
	char		grenadetype, grenade1latch, grenade2latch;
	int			grenadeprimeTime, grenadethrowTime;
	int			grenadenextTime;
	int			grenadeHudTime;
	// Golliwog.

	// Golliwog: Vibration variables
	int			vibrateTime;
	float		vibrateMagnitude;
	float		vibrateOffset;
	// Golliwog.

	// Golliwog: HUD slots
	centity_t *hudslots[Q3F_SLOT_MAX];
	// Golliwog.

	// Golliwog: Hallucination variables
	unsigned char gasPlayerTeam[MAX_CLIENTS];
	unsigned char gasPlayerClass[MAX_CLIENTS];
	vec4_t		gasColour, gasEndColour, gasCurrColour;
	int			gasTime, gasEndTime, gasFlashTime;
	// Golliwog

	// Jules: Minigun Shudder variables
	int			minigunHeat;
	int			minigunLast;
	// Jules

	// Golliwog: Map info fadeout
	int			mapInfoFadeTime;

	// Golliwog: Team kill
	int			teamKillWarnTime;

	// RR2DO2: agentdata
	centity_t	*agentDataEntity;
	int			agentLastClass;

	// Golliwog: Autosentry / supplystation HUD markers
	int			sentryHealth, sentryDamageTime;
	int			supplystationHealth, supplystationDamageTime;
	int			supplyStationUsedTime;
	qboolean	supplyStationUserIsEnemy;
	int			usingSupplystation;		// 0: not using - 1: using - 2: used
	// Golliwog.

	// Golliwog: Used for scrolling scoreboard
	int			scoreBoardTime, scoreBoardMode;
	int			scoreBoardHeight, scoreBoardCalcHeight;
	int			scoreBoardDuration;
	// Golliwog.

	// Golliwog: Used for player counts and average team pings
	int			teamCounts[4], teamPings[4], teamBalanceWarnings;
	int			teamAllies[4];
	// Golliwog.

	// Golliwog: Used to allow weapon switching.
	int			lastWeapon, currWeapon;
	// Golliwog:

	vec4_t		sniperWashColour;		// Golliwog: Used for "dot on face" effect.
	centity_t	*sniperDotEnt;			// Golliwog: The sniper dot (if found).
	int			mapSelectState;			// Golliwog: Indicates state of map vote system

	// RR2DO2: for the autoscreenshot option
	qboolean	ScoreSnapshotTaken;

	// RR2DO2: sentry cam
	vec3_t		sentrycam_origin;
	vec3_t		sentrycam_angles;
	entityState_t *sentrycam_entityState;

	// RR2DO2: lensflare blinding
	vec4_t		lensflare_blinded;

	qboolean drawFilter;					// Golliwog: Block any render calls.

   // Keeger
#ifdef API_ET
   refdef_t			*refdef_current;		// Handling of some drawing elements for MV
	vec2_t		mapcoordsMins;
	vec2_t		mapcoordsMaxs;
	vec2_t		mapcoordsScale;
   qboolean	mapcoordsValid;

   qboolean	spawning;				// the CG_Spawn*() functions are valid
#endif
	// RR2DO2: scopes
	int			scopeTime;
	qboolean	scopeEnabled;

	// djbob: sentry cam fx check
	int			sentryCamTime_end;
	int			sentryCamTime;
	qboolean	sentryCam_on;

	// djbob: self blood flash
	int			bleedtime;
	int			bleeddmg;

	// djbob: underwater time
	int			waterundertime;

	// message mode buffer and info
	int			q3f_messagemode_mode;
	char		q3f_messagemode_buffer[MAX_SAY_TEXT];

	int			q3f_alerticontime[MAX_Q3F_ALERTS];
	q3f_alert_t	q3f_alerticons[MAX_Q3F_ALERTS];

	// RR2DO2: reload animation
	int			reloadendtime;

   // Keeger: crosshair colors
	vec4_t		xhairColor;
	vec4_t		xhairColorAlt;
	qhandle_t	crosshairShaderAlt[NUM_CROSSHAIRS];

	//Canabis, special 2 tracking
	int			special2_lastweapon;
	char		ucmd_flags;

	//Unlagged optimzied prediction
	int			lastPredictedCommand;
	int			lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int			stateHead, stateTail;

	// slothy buffered sounds
	bufferedSound_t bufferedSounds[MAX_BUFFERSOUNDS];

	// slothy flaginfo hud item
	int			fi_endtime;
	char		finfo[MAX_SAY_TEXT];

	// slothy class info
	int			classInfoTime;

	// slothy alternate obits
	altObit_t	obits[MAX_OBITS];
	int			numObits;

	// slothy kill/death stats
	int			kills[MAX_CLIENTS];
	int			deaths[MAX_CLIENTS];
	int			mykills, mydeaths;

	// Canabis, logging
	fileHandle_t		matchLogFileHandle;
	char				matchLogFileName[MAX_QPATH];

	qboolean	serverRespawning;
} cg_t;

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;
	qhandle_t	whiteAdditiveShader;	// RR2DO2: this should work better with blinding for lensflares

	qhandle_t	redCubeModel;
	qhandle_t	blueCubeModel;
	qhandle_t	redCubeIcon;
	qhandle_t	blueCubeIcon;
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];
	qhandle_t	flagShader[4];

	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	qhandle_t	redFlagFlapSkin;
	qhandle_t	blueFlagFlapSkin;
	qhandle_t	neutralFlagFlapSkin;

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

	// FALCON : START : Modified to suit Q3F
/*	qhandle_t	armorGreenModel;
	qhandle_t	armorGreenIcon;

	qhandle_t	armorYellowModel;
	qhandle_t	armorYellowIcon;

	qhandle_t	armorRedModel;
	qhandle_t	armorRedIcon;*/
	// FALCON : END

	qhandle_t	teamStatusBar;

//	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	smoke2;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	railRingsShader;
	qhandle_t	railCoreShader;

	//qhandle_t	laserShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;

	qhandle_t	selectShader;
	//qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
	qhandle_t	tracer2Shader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	scannerShader;
	qhandle_t	scannerblipShader;
	qhandle_t	scannerupShader;
	qhandle_t	scannerdownShader;
	qhandle_t	lagometerShader;
	qhandle_t	lagometermaskShader; // RR2DO2
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	smokePuffShader;
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	flameShader;
//	qhandle_t	flameShader2;
//	qhandle_t	fthrowBlueFlameShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;

	qhandle_t	numberShaders[11];

	qhandle_t	shadowMarkShader;

//	qhandle_t	botSkillShaders[5];

	// RR2DO2: oldhud shaders
	qhandle_t	hud_armourShader;
	qhandle_t	hud_healthShader;

	// New hud shader
	qhandle_t	hud_icon_shells;
	qhandle_t	hud_icon_nails;
	qhandle_t	hud_icon_rockets;
	qhandle_t	hud_icon_cells;

	qhandle_t	hud_sniperscope;
	qhandle_t	hud_sniperscopeXhair;
	qhandle_t	hud_binoculars;
	qhandle_t	hud_binocularsXhair;
	qhandle_t	hud_binocularsTarget;

	//qhandle_t	dots;
	// RR2DO2

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;
	qhandle_t	footprintRightMarkShader;
	qhandle_t	footprintLeftMarkShader;

	// powerup shaders
	qhandle_t	quadShader;
	qhandle_t	redQuadShader;
	qhandle_t	yellowQuadShader;
	qhandle_t	greenQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	redQuadWeaponShader;
	qhandle_t	yellowQuadWeaponShader;
	qhandle_t	greenQuadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	battleWeaponShader;
	qhandle_t	hastePuffShader;
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;
	qhandle_t	onFireShader0;
	qhandle_t	onFireShader1;

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	sphereFlashModel;
	
	// weapon effect shaders
	qhandle_t	railExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShaders[3];
	qhandle_t	rocketExplosionShader;
	qhandle_t	rocket3DExplosionShader;
	//qhandle_t	smokeExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	grenade3DExplosionShader;
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;
	qhandle_t	napalmFlameShader;
	qhandle_t	flameEffectShader;
	qhandle_t	flameModel;
	qhandle_t	pulseExplosionShader;
	qhandle_t	pulse3DExplosionShader;
	qhandle_t	pulseRingShader;
	qhandle_t	pulseBeamShader;

	// lens flares
	qhandle_t	flare1;
	qhandle_t	flare2;
	qhandle_t	flare3;
	qhandle_t	flare4;

	// Sun lens flare
	qhandle_t	bluedisc;
	qhandle_t	bluediscweak;
	qhandle_t	bluegradient;
	qhandle_t	browndisc;
	qhandle_t	brownring;
	qhandle_t	greenring;
	qhandle_t	rainbowring;
	qhandle_t	whitegradient;
	qhandle_t	whiteredring2;
	qhandle_t	whiteredring66;
	qhandle_t	whitering;

	// special effects models
	qhandle_t	dustPuffShader;

	// scoreboard headers
/*	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;*/

	// medals shown during gameplay
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;

	// sounds
	sfxHandle_t	quadSound;
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	sfxHandle_t	sfx_metal_ric1;
	sfxHandle_t	sfx_metal_ric2;
	sfxHandle_t	sfx_metal_ric3;
	//sfxHandle_t	sfx_railg;
	sfxHandle_t sfx_railhit;
	sfxHandle_t	sfx_rockexp;
//	sfxHandle_t	sfx_plasmaexp;
	sfxHandle_t sfx_pulseexp;
	sfxHandle_t	gasSmokeSound; 
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
	sfxHandle_t brassBounce1Sound;
	sfxHandle_t brassBounce2Sound;
	sfxHandle_t brassBounce3Sound;
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

/*	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;*/

	sfxHandle_t hitSound;
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

	sfxHandle_t weaponHoverSound;

	// teamplay sounds
//	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t neutralFlagReturnedSound;
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t	youHaveFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
	sfxHandle_t holyShitSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;

	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
	sfxHandle_t	n_healthSound;
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;

	// q3f sounds

	sfxHandle_t	sfx_minigun_windup;
	sfxHandle_t sfx_minigun_winddown;
	sfxHandle_t sfx_minigun_loop;
	//sfxHandle_t	sfx_minigun_fire;
	sfxHandle_t	sfx_flamethrower_windup;
	sfxHandle_t sfx_flamethrower_winddown;
	sfxHandle_t sfx_flamethrower_fire;
	sfxHandle_t sfx_flamethrower_firewater;
	sfxHandle_t sfx_napalmExplode, sfx_napalmBurn, sfx_napalmWater;
	sfxHandle_t	grenadePrimeSound;
	sfxHandle_t	chargebeep1;
	sfxHandle_t	chargebeep2;
	sfxHandle_t	sfx_chargeexp;
	sfxHandle_t	discardSound;
	sfxHandle_t cockSound;
	sfxHandle_t cockGrenSound;

	// JT: Q3F Stuff
	qhandle_t	sniperDot;
	qhandle_t	sniperLaser;

	qhandle_t	savemeShader;		// Golliwog: Displayed above player's heads
	qhandle_t	armormeShader;

	qhandle_t	agentShader;		// Spy morph shader

//	qhandle_t	flamethrowerShader;	// Used for the flame sections.

	qhandle_t	minigunSmokeTag1;
	qhandle_t	minigunSmokeTag2;
	qhandle_t	minigunFlashTag;

	// Golliwog: Sentry data
	qhandle_t	sentryBase, sentryBarrel, sentryRocketLauncher, sentryFlash;
	qhandle_t	sentryConstruct_Base, sentryConstructShader_1, sentryConstructShader_2;
	qhandle_t	sentryTurret1, sentryCannon1;
	qhandle_t	sentryTurret2, sentryCannon2;
	qhandle_t	sentryTurret3, sentryCannon3;
	qhandle_t	sentryBits[4];
	sfxHandle_t	sentrySpinupSound, sentryFireSound;
	sfxHandle_t sentryStartSound, sentryStopSound;
	sfxHandle_t	sentryBuildSound, sentryUpgradeSound;
	sfxHandle_t	sentryExplodeSound;
	// Golliwog.

#ifdef RIMLIGHTING_OUTLINE
	qhandle_t	outlineShader;
#endif

	// Supplystation data
	qhandle_t	supplystationBase, supplystationHUD;
	qhandle_t	supplystationConstruct_Base, supplystationConstruct_Screen;
	qhandle_t	supplystationBits[3];
	sfxHandle_t	supplyPopup, supplyRetract;
	sfxHandle_t	supplyBuildSound;
	sfxHandle_t	supplyExplodeSound;

	//djbob: sentry cam tv fx
	qhandle_t	sentryTvFx;

	// RR2DO2: Class media
	qhandle_t	modelcache[Q3F_CLASS_MAX][3];
	F2RDef_t	*f2rcache[Q3F_CLASS_MAX][3];
	qhandle_t	skincache[Q3F_CLASS_MAX][3];
	byte		skincolours[Q3F_CLASS_MAX][4][3][3];
	qhandle_t	modeliconcache[Q3F_CLASS_MAX];
	sfxHandle_t	soundcache[Q3F_CLASS_MAX][MAX_CUSTOM_SOUNDS];
	sfxHandle_t classnamesounds[Q3F_CLASS_MAX][2];

	qhandle_t	celshader;

	qhandle_t	backpack;

   //keeger shader from ET for flamethrower
   qhandle_t	sparkFlareShader;
   qhandle_t	flamethrowerFireStream;
   qhandle_t	flamethrowerBlueStream;
   qhandle_t	flameShaders[NUM_FLAME_SPRITES];

   qhandle_t	hudTimerShader;

   qhandle_t	repairmeShader;
   qhandle_t	upgrademeShader;
   qhandle_t	refillmeShader;

   qhandle_t	disconnectIcon;
} cgMedia_t;



typedef struct {
	//Canabis, Spirit scripts used ingame
	SpiritScript_t	*diseased;
	SpiritScript_t	*gassed;
	SpiritScript_t	*flashed;
	SpiritScript_t	*spawn;
	SpiritScript_t	*watersplash;
	SpiritScript_t	*forcefieldspark;
	SpiritScript_t	*stunned;
	SpiritScript_t	*tranqed;
	SpiritScript_t	*legshot;
	SpiritScript_t	*explosion_concussion;
	SpiritScript_t	*explosion_flash;
	SpiritScript_t	*explosion_simple;
	SpiritScript_t	*explosion_normal;
	SpiritScript_t	*explosion_he;
	SpiritScript_t	*explosion_napalm;
	SpiritScript_t	*explosion_quad;
} cgSpirit_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];
	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];
	char			yellowTeam[MAX_QPATH];
	char			greenTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				levelStartTime;

	qboolean		newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];
	qhandle_t		gameShaders[MAX_SHADERS];	// Golliwog: Flat graphics for various things
	SpiritScript_t	*gameSpiritScript[MAX_SPIRITSCRIPTS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_BYTEWIDTH];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	int				cursorX;
	int				cursorY;
	int				eventHandling;
	qboolean		mouseCaptured;
	qboolean		sizingHud;
	void			*capturedItem;
	qhandle_t		activeCursor;

	int				currentVoiceClient;

	// media
	cgMedia_t		media;
	cgSpirit_t		spirit;
	
	mapInfo			mapinfo;
	qboolean		mapInfoLoaded;

	// Golliwog: Teams and classes playing this map.
	int				teams, classes;

	// Golliwog: Gravity modification coutn
	int				gravityModificationCount;
	int				grenadePrimeSoundModificationCount;

	// RR2DO2: flyby cam data
	campath_t		campaths[Q3F_MAX_PATHS];
	int				camNumPaths;
	int				flybyPathIndex;

	// Golliwog: Initialization data.
	int					initPhase, initIndex, initFHandle, initFileSize;
	char				initBuff[1024];
	int					initTime;
	qboolean			initScreenRendered, initDemoFrameRender;

	// Golliwog: Location data
	cg_q3f_location_t	locations[1024];				// Some locations to store information in.
	int					numLocations;
	char				*currentLocation;
	int					currentLocationTime;

	// Golliwog: String table. Also used for other general memory stuff?
	char				stringData[65536];
	int					stringDataLen;

	// Golliwog: Generic entity-associated data. Based on spawn index rather than entityState number.
	size_t numEntityData;
	int entityIndex[512];
	void *entityData[512];

	// RR2DO2: Flare data
	cg_q3f_flare_t		flares[512];
	int					numFlares;

	// RR2DO2: Sunflares
	cg_q3f_flare_t		sunFlares[5];
	vec3_t				sunFlareTraceDelta[5];
	int					numSunFlares;

	struct {
		char			portalShader[MAX_QPATH];
		char			disableShader[MAX_QPATH];
		qboolean		hasportal;
		vec3_t			origin;
	} skyportal;

	struct {
		vec3_t				origin;
		qboolean			exists;
	} sunportal;

	int gameindex;		// Golliwog: The server-side game index.
	char *longName;		// Golliwog: The 'long' name of the map.
	char *description;	// Golliwog: The map description.
	char *shaderRemap;	// Golliwog: A list of shaders to remap on startup.

#ifdef _DEBUG
	char *registeredShaders[1024], *registeredSkins[1024], *registeredModels[1024], *registeredSounds[1024];
	int registeredShaderCounts[1024], registeredSkinCounts[1024], registeredModelCounts[1024], registeredSoundCounts[1024];
	int registeredShaderSizes[1024], registeredSkinSizes[1024], registeredModelSizes[1024], registeredSoundSizes[1024];
#endif

#ifdef API_ET
	qboolean		initing;
	char			rawmapname[MAX_QPATH];
#endif
	int				unlagged;

	int aviDemoRate;                                    // Demo playback recording
	qboolean fKeyPressed[256];                          // Key status to get around console issues
	int timescaleUpdate;                                // Timescale display for demo playback
	qboolean demoPaused;								// Paused with spacebar in demo viewer
	int thirdpersonUpdate;
	float oldtimescale;									// Timescale value prior to pausing

	qboolean		pmove_fixed;
	int				pmove_msec;

	int				sv_fps;

	qboolean		synchronousClients;

	qboolean		sv_cheats;

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[WP_NUM_WEAPONS];		// Was MAX_WEAPONS
extern	weaponInfo_t	cg_extendedweapons[MAX_EXTENDED_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawSpeedometer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern  vmCvar_t		cg_crosshairAlpha;  //keeg from ET xhair
extern  vmCvar_t		cg_crosshairAlphaAlt;
extern	vmCvar_t		cg_crosshairColor;
extern  vmCvar_t		cg_crosshairColorAlt;
extern   vmCvar_t		cg_bloodFlash;  //keeg for flamed hud

extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_markTime;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_fovAspectAdjust;
extern	vmCvar_t		cg_fovViewmodel;
extern	vmCvar_t		cg_fovViewmodelAdjust;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_hideScope;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_drawFriendSize;
extern	vmCvar_t		cg_debugTime;
extern	vmCvar_t		cg_teamChatsOnly;
extern  vmCvar_t		cg_scorePlum;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_sniperDotScale;
extern	vmCvar_t		cg_impactVibration;
extern	vmCvar_t		cg_sniperDotColors;
extern	vmCvar_t		cg_execClassConfigs;
extern	vmCvar_t		cg_execMapConfigs;
extern	vmCvar_t		cg_lowEffects;
extern	vmCvar_t		cg_no3DExplosions;
extern	vmCvar_t		cg_atmosphericEffects;
extern	vmCvar_t		cg_etfVersion;
//extern	vmCvar_t		cg_menuSlideTime;
//extern	vmCvar_t		cg_menuNoFade;
extern	vmCvar_t		cg_teamChatSounds;
extern	vmCvar_t		cg_sniperAutoZoom;
extern	vmCvar_t		cg_sniperHistoricalSight;
extern	vmCvar_t		cg_grenadePrimeSound;
//extern	vmCvar_t		cg_teamChatBigFont;
//extern	vmCvar_t		cg_newFlamer;
//extern	vmCvar_t		cg_drawMem;
extern	vmCvar_t		g_spectatorMode; // RR2DO2

extern	vmCvar_t		r_lodCurveError;
extern	vmCvar_t		r_gamma;
extern	vmCvar_t		r_showNormals;
extern	vmCvar_t		r_showTris;
extern	vmCvar_t		r_fastSky;
extern	vmCvar_t		r_vertexLight;
extern	vmCvar_t		r_loresskins;
extern	vmCvar_t		r_singleShader;
extern	vmCvar_t		r_fullBright;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		rate;
extern	vmCvar_t		snaps;
extern	vmCvar_t		cl_maxpackets;
extern	vmCvar_t		r_maxpolys;
extern	vmCvar_t		r_maxpolyverts;
extern	vmCvar_t		com_maxfps;
extern	vmCvar_t		r_clear;
extern	vmCvar_t		r_debugSort;
extern	vmCvar_t		r_debugSurface;
extern	vmCvar_t		r_directedScale;
extern	vmCvar_t		r_drawworld;
extern	vmCvar_t		r_lockpvs;
extern	vmCvar_t		r_nocull;
extern	vmCvar_t		r_nocurves;
extern	vmCvar_t		r_noportals;
extern	vmCvar_t		r_novis;
extern	vmCvar_t		r_showclusters;
extern	vmCvar_t		r_lightmap;
extern	vmCvar_t		cg_fallingBob;
extern	vmCvar_t		cg_weaponBob;
/*extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_yellowTeamName;
extern	vmCvar_t		cg_greenTeamName;*/
extern	vmCvar_t		cg_adjustAgentSpeed;
//extern	vmCvar_t		cg_drawHudSlots;
extern	vmCvar_t		cg_playClassSound;
extern	vmCvar_t		cg_drawParticleCount;
//extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_initializing;
extern	vmCvar_t		cg_drawPanel;
extern	vmCvar_t		cg_debugPanel;
extern	vmCvar_t		cg_userHud;
extern	vmCvar_t		r_flares;
extern	vmCvar_t		cg_flares;
extern	vmCvar_t		cg_friendlyCrosshair;
extern	vmCvar_t		cg_showGrenadeTimer1;
extern	vmCvar_t		cg_showGrenadeTimer2;
extern	vmCvar_t		cg_showGrenadeTimer3;
extern	vmCvar_t		cg_scoreboardsortmode;
extern	vmCvar_t		cg_visualAids;
extern	vmCvar_t		cg_filterObituaries;
extern	vmCvar_t		cg_showSentryCam;
extern	vmCvar_t		cg_oldSkoolMenu;
extern	vmCvar_t		cg_gender;
extern	vmCvar_t		com_hunkmegs;
extern	vmCvar_t		cg_classicinit;
extern	vmCvar_t		cg_drawSkyPortal;
extern	vmCvar_t		developer;
extern	vmCvar_t		cg_mergemm2;
extern	vmCvar_t		cg_onlychats;

extern	vmCvar_t		cg_altObits;
extern	vmCvar_t		ui_altObitsY;
extern	vmCvar_t		ui_altObitsX;

// demo recording
extern	vmCvar_t		cl_demorecording;
extern	vmCvar_t		cg_recording_statusline;
extern	vmCvar_t		cl_demofilename;
extern	vmCvar_t		cl_demooffset;
extern	vmCvar_t		cl_waverecording;
extern	vmCvar_t		cl_wavefilename;
extern	vmCvar_t		cl_waveoffset;

//unlagged - client options
extern	vmCvar_t		cg_unlagged;
extern	vmCvar_t		cg_debugDelag;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_cmdTimeNudge;
extern	vmCvar_t		sv_fps;
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cg_optimizePrediction;
extern	vmCvar_t		cl_timeNudge;
extern	vmCvar_t		cg_latentSnaps;
extern	vmCvar_t		cg_latentCmds;
extern	vmCvar_t		cg_plOut;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_predictWeapons;


extern	vmCvar_t		demo_avifpsF1;
extern	vmCvar_t		demo_avifpsF2;
extern	vmCvar_t		demo_avifpsF3;
extern	vmCvar_t		demo_avifpsF4;
extern	vmCvar_t		demo_avifpsF5;
extern	vmCvar_t		demo_avifpsF6;
extern	vmCvar_t		demo_drawTimeScale;
extern	vmCvar_t		demo_infoWindow;
extern	vmCvar_t		demo_baseFOV;
extern	vmCvar_t		demo_scoresToggle;


extern	vmCvar_t		cg_drawFollowText;

extern	vmCvar_t		cg_rocketTrail;
extern	vmCvar_t		cg_grenadeTrail;
extern	vmCvar_t		cg_pipeTrail;
extern	vmCvar_t		cg_napalmTrail;
extern	vmCvar_t		cg_dartTrail;
extern	vmCvar_t		cg_shotgunPuff;

extern	vmCvar_t		cl_anonymous;

//unlagged - client options
//unlagged - cg_unlagged.c
void CG_PredictWeaponEffects( centity_t *cent );
void CG_AddBoundingBox( centity_t *cent, vec3_t color );
void CG_DrawBoundingBox( vec3_t origin, vec3_t mins,vec3_t maxs, vec3_t color );

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );
void CG_Q3F_UpdateCvarLimits( );
void CG_Q3F_RemapSkyShader( void );

#define BOX_PRINT_MODE_CHAT			0
#define BOX_PRINT_MODE_TEAMCHAT		1
#define BOX_PRINT_MODE_CENTER		2
#define BOX_PRINT_MODE_CHAT_REAL	3
#define BOX_PRINT_MODE_CONSOLE		4

void QDECL CG_Printf( int mode, const char *msg, ... );
void QDECL CG_LowPriority_Printf( int mode, const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

void CG_UpdateCvars( void );
void CG_RegisterCvars( void );

void CG_RegisterItemSounds( int itemNum );

void CG_Q3F_LoadMapConfig( void );
void CG_PlayDelayedSounds( void );

// RR2DO2: why did you forget this golli?
void CG_Q3F_PlayLocalSound( const char *sound );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile, qboolean resetHud);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type, qboolean fForced);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore();
void CG_BuildSpectatorString();
void Menu_Reset();

char *CG_Q3F_AddString( const char *str );
void *CG_Q3F_AddBlock( int size, int alignment );
char *CG_Q3F_GetLocation( vec3_t origin, qboolean doTrace );

void CG_AssetCache();
void CG_LoadHudMenu();

void CG_Q3F_DrawScoreboardTeamScores(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font);

void CG_SetupIntermissionMenu();

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);
void CG_CalcVrect (void);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_FillRectAdditive( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawAdjustedPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string, float charWidth, float charHeight, const float *modulate );
void CG_Item_AutoAnchor(rectDef_t* rect, const rectDef_t* newRect, int anchorx, int anchory);

int CG_DrawStrlen( const char *str );

float *CG_FadeColor( int startMsec, int totalMsec );
float *CG_RFadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
float *CG_TeamColor_Scoreboard( int team );
void CG_Q3F_GetTeamColor (vec4_t hcolor, q3f_team_t teamnum);
void CG_Q3F_GetTeamColor2 (vec4_t *hcolor );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
qboolean CG_AlliedTeam( int team, int isAlliedTeam);
void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_ScanForCrosshairEntity( void );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_ETF_DrawSkyPortal( refdef_t *parentrefdef, vec4_t *parentflareblind, stereoFrame_t stereoView, vec3_t sky_origin );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw( itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment );
void CG_Text_Paint_MaxWidth(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment, int maxwidth);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment);
int CG_Text_Width(const char *text, float scale, int limit, fontStruct_t *parentfont);
int CG_Text_Height(const char *text, float scale, int limit, fontStruct_t *parentfont);
void CG_SelectPrevPlayer();
void CG_SelectNextPlayer();
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead();
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat();
void CG_GetTeamColor(vec4_t *color);
const char *CG_Q3F_TeamStatus();
const char *CG_GetKillerText();
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending();
const char *CG_GameTypeString();
qboolean CG_YourTeamHasFlag();
qboolean CG_OtherTeamHasFlag();
qhandle_t CG_StatusHandle(int task);
// RR2DO2
//float CG_DrawOldLagometer( );	// put here to be found
void CG_Q3F_DrawProgress( int x, int y, int maxwidth, int height, int maxvalue, int absmaxvalue, int value, qhandle_t icon, vec4_t iconcolor );
void CG_Q3F_DrawWeaponInfo( int weapon, int clipcount, int ammocount );
// RR2DO2

void CG_ExpandingTextBox_AutoWrap( char* instr, float scale, fontStruct_t*font, float w, int size);
qboolean CG_GetWeaponSwitchBoxExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory );
qboolean CG_Q3f_GetAlertIconExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory, int border );
qboolean CG_Q3F_GetPowerupIconExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory, int border );
qboolean CG_GetExpandingTextBox_Text( int ownerdraw, char* out, float* alpha, qboolean* changed);
qboolean CG_GetExpandingTextBox_Extents(rectDef_t* in, rectDef_t* out, float scale, fontStruct_t* font, int anchorx, int anchory, int border, char* instr, int ownerDraw);
qboolean CG_GetAttackerBoxExtents(rectDef_t* in, rectDef_t* out);

void CG_Q3F_AddAlertIcon(vec3_t origin, q3f_alert_t alert);

const char *CG_Q3F_GetTeamColorString( int teamnum );

//
// cg_player.c
//
qhandle_t *CG_Q3F_LegsModel( int classNum );
qhandle_t *CG_Q3F_TorsoModel( int classNum );
qhandle_t *CG_Q3F_HeadModel( int classNum );
qhandle_t *CG_Q3F_LegsSkin( int classNum );
qhandle_t *CG_Q3F_TorsoSkin( int classNum );
qhandle_t *CG_Q3F_HeadSkin( int classNum );
qhandle_t *CG_Q3F_ModelIcon( int classNum );
qhandle_t *CG_Q3F_TorsoModel( int classNum );
qhandle_t *CG_Q3F_HeadModel( int classNum );
F2RDef_t *CG_Q3F_LegsF2RScript( int classNum );
F2RDef_t *CG_Q3F_TorsoF2RScript( int classNum );
F2RDef_t *CG_Q3F_HeadF2RScript( int classNum );
byte *CG_Q3F_LegsColour( int classNum, q3f_team_t teamNum );
byte *CG_Q3F_TorsoColour( int classNum, q3f_team_t teamNum );
byte *CG_Q3F_HeadColour( int classNum, q3f_team_t teamNum );
qboolean CG_Q3F_RegisterClassModels( int classNum );
qboolean CG_Q3F_RegisterClassSounds( int classNum );

void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum );
//void CG_Q3F_NewClientInfo( int clientNum );	// RR2DO2
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );
void CG_TrailItem( centity_t *cent, qhandle_t hModel );
qboolean CG_Q3F_UseFakeAgentModel( int modelpart );
void CG_Q3F_CalcAgentVisibility( qboolean *drawmodel, float *shaderalpha, qboolean *newmodel, float fracstart, float fracend, entityState_t *agentstate );
qboolean CG_Q3F_RegisterClientModelName( clientInfo_t *ci, const char *modelName, const char *skinName );
void CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts, qboolean keepSrcColor );
void CG_RunLerpFrame( F2RDef_t *F2RScript, lerpFrame_t *lf, int newAnimation, float speedScale );
qhandle_t       CG_Q3F_ShaderForQuad(int team);
float          *CG_Q3F_LightForQuad(int team);

extern byte cg_q3f_agentcolours[5][3];		// Golliwog: Agent colour array
extern char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS];
extern char	*cg_customSoundPaths_Light[MAX_CUSTOM_SOUNDS];
extern char	*cg_customSoundPaths_Medium[MAX_CUSTOM_SOUNDS];
extern char	*cg_customSoundPaths_Heavy[MAX_CUSTOM_SOUNDS];

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
				int skipNumber, int mask );
void CG_PredictPlayerState( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );
void CG_Q3F_Vibrate( int val, vec3_t position );
void CG_GurpEvent( centity_t *cent, int health );
void CG_BurnEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_Q3F_Beam( centity_t *cent );
void CG_Q3F_Flamer( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, const char *tagName, int startIndex, vec3_t *offset );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, const char *tagName );
void CG_GetTagFromModel( orientation_t *tag, qhandle_t hModel, const char *tagName );



//
// cg_weapons.c
//
qboolean CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle, vec3_t forward );
void CG_Tracer( vec3_t source, vec3_t dest, qboolean sound, float width, qhandle_t shader, vec4_t rgba );

void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );
void CG_WeaponSlot_f( void );
void CG_LastWeapon_f( void );

void CG_RegisterExtendedWeapon( int weaponNum );
void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int clientNum, int entityNum );
void CG_SingleShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed );
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed );
void CG_MinigunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed, int spread );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );
void CG_Sniper_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );

//void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, centity_t *agentdata );
void CG_DrawWeaponSelect( void );
void CG_BotDebugLine(vec3_t start, vec3_t end, vec3_t color);
void CG_OutOfAmmoChange( void );	// should this be in pmove?

weaponInfo_t *CG_Q3F_GetWeaponStruct(int clsnum, int weapon);
void CG_AddWeaponWithPowerups( refEntity_t *gun, entityState_t *state, int team );
//
// cg_marks.c
//

void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void CG_OldMark(qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
				    float orientation,float radius,vec4_t color, 
					int lifetime,leMarkFadeType_t fadetype);
void CG_DecalMark(qhandle_t markShader, const vec3_t origin, vec4_t projection, 
				    float orientation,float radius,vec4_t color, 
					int lifetime,int fadetime
					);
void	CG_ExplosionMark(vec3_t origin,float radius,vec4_t color );
void	CG_BulletMark(qhandle_t shader,vec3_t origin,vec3_t dir,float radius,vec4_t color);
qboolean CG_ShadowMark(vec3_t origin, float radius, float height, float *shadowPlane );

//
// cg_trails.c
//
int CG_AddTrailJunc(int headJuncIndex, qhandle_t shader, int spawnTime, int sType, vec3_t pos, int trailLife, float alphaStart, float alphaEnd, float startWidth, float endWidth, int flags, vec3_t colorStart, vec3_t colorEnd, float sRatio, float animSpeed);
int CG_AddSparkJunc(int headJuncIndex, qhandle_t shader, vec3_t pos, int trailLife, float alphaStart, float alphaEnd, float startWidth, float endWidth);
int CG_AddSmokeJunc(int headJuncIndex, qhandle_t shader, vec3_t pos, int trailLife, float alpha, float startWidth, float endWidth);
int CG_AddFireJunc(int headJuncIndex, qhandle_t shader, vec3_t pos, int trailLife, float alpha, float startWidth, float endWidth);
void CG_AddTrails(void);
void CG_ClearTrails (void);
//
//cg_flamethrower.c
//
void CG_FireFlameChunks( centity_t *cent, vec3_t origin, vec3_t angles, qboolean newchain );
void CG_InitFlameChunks(void);
void CG_ClearFlameChunks (void);
void CG_AddFlameChunks(void);
void CG_UpdateFlamethrowerSounds(void);

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( int duration );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
					int duration, int fadedelay,
				    float startradius, float endradius,
				    const vec4_t color, qhandle_t hShader );

void CG_NormalExplosion( const vec3_t origin );
void CG_QuadExplosion( const vec3_t origin, int team );
void CG_BulletExplosion( const vec3_t origin, const vec3_t dir );
localEntity_t *CG_OldNapalmFlame( const vec3_t p, const vec3_t vel, 
				   float radius, float alpha, int duration, qhandle_t hShader );

void CG_SpawnSmokeSprite(const vec3_t origin, int life, const vec4_t color, float startradius, float endradius);
qboolean CG_SpawnGasSprite( centity_t * cent, float fraction);
void CG_BurnGasSprites( centity_t * cent);
void CG_InitSmokeSprites( void );
void CG_AddSmokeSprites( void );

void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
void CG_ScorePlum( int client, vec3_t org, int score );
void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin,
								int duration, float startradius, float endradius,
								qhandle_t hModel, qhandle_t shader
								);

localEntity_t *CG_MakeLight( const vec3_t origin, int duration, 
				  int fadedelay, const vec4_t color,
				  float startradius, float endradius );

localEntity_t *CG_Q3F_MakeRing( const vec3_t origin, 
							    const vec3_t angles,
							    int duration,
								float startradius,
								float endradius,
								int fadedelay,
								qhandle_t hShader );


localEntity_t *CG_Q3F_MakeBeam( const vec3_t origin, 
							    const vec3_t dest,
							    float max_offset,
								int numSubdivisions,
								float radius,
								int leFlags,
								vec4_t colour,
								int duration,
								float speedscale,
								qhandle_t hShader );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
//unlagged - early transitioning
void CG_TransitionEntity( centity_t *cent );

//
// cg_info.c
//
void CG_ETF_DemoParseString(const char* in, char* out, int size);
void CG_MatchLog_f( void );
void CG_MatchLogAddLine( const char *line );
void CG_DemoClick( int key, qboolean down );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );
void CG_Q3F_Flyby ( void );

void CG_keyOn_f( void );
void CG_keyOff_f( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_ParseSysteminfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);
void CG_LoadHud_f( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );

// Keeger
//
// cg_polybus.c
//
#ifdef API_ET
polyBuffer_t* CG_PB_FindFreePolyBuffer(qhandle_t shader, int numVerts, int numIndicies);
void CG_PB_ClearPolyBuffers( void );
void CG_PB_RenderPolyBuffers( void );
#endif

//
// RR2DO2
//
// cg_q3f_classutil.c
//
void CG_Q3F_PlayClassnameSound( int clsnum, qboolean xian );

//
// cg_q3f_sentry.c
//
void CG_Q3F_RegisterSentry( void );
void CG_Q3F_RegisterSupplyStation( void );
void CG_Q3F_Sentry( centity_t *cent );
void CG_Q3F_Sentry_Explode( centity_t *cent );
void CG_Q3F_Supplystation( centity_t *cent );
void CG_Q3F_Supplystation_Explode( centity_t *cent );

//
// cg_q3f_mapsentry.c
//
void CG_Q3F_RegisterMapSentry( qboolean rocket );
void CG_Q3F_MapSentry( centity_t *cent );

// Keeg  RR2 says i can remove q3f stuff, do it when other code works
//
// cg_q3f_atmospheric.c
//
#ifdef API_Q3
void CG_Q3F_EffectParse( const char *effectstr );
void CG_Q3F_AddAtmosphericEffects();
qboolean CG_Q3F_AtmosphericKludge();
#elif API_ET
void CG_EffectParse( const char *effectstr );
void CG_AddAtmosphericEffects();

// brought in from cg_view and added to atmospheric
void CG_SetupFrustum( void );
qboolean CG_CullPoint( vec3_t pt );
qboolean CG_CullPointAndRadius( const vec3_t pt, vec_t radius);
#endif

//
// cg_q3f_crc.c
//
int CG_Q3F_GetCRC( const char *filename );

//
// cg_q3f_init.c
//
/* Ensiform - Added demoPlayback since apparently bani added this with 2.60 and can be used to check for demo mod version checking ;) */
void CG_Q3F_Init( int serverMessageNum, int serverCommandSequence, int clientNum, qboolean demoPlayback );
void CG_Q3F_InitUpdate(void);
void CG_Q3F_RenderLoadingScreen(void);

//
// cg_q3f_panel.c
//
void CG_Q3F_Panel( centity_t *cent );

//
// cg_q3f_spawn.c
//
void CG_Q3F_ParseEntities();

//
// cg_q3f_mapinfo.c
//
int CG_Q3F_GetMapInfo( const char *mapname, cg_q3f_mapinfo_t mapInfo[], int numItems, int gameIndex );

//
// cg_q3f_flare.c
//
void CG_AddFullFlareToScene( const vec3_t org, float radius, float rotation, qhandle_t shader, float intensity, float r, float g, float b, qboolean isLensFlare, qboolean doScreenBlind );
void CG_AddFlareToScene( const vec3_t org, float intensity, float r, float g, float b, qboolean isLensFlare, qboolean doScreenBlind );
void CG_PositionSunflares( void );
void CG_RenderSunflares( void );
void CG_SetFlareFader( int *timer, float *value );
void CG_SetFlareRenderer( refdef_t *refdef, vec4_t *flareblind );

//
// cg_q3f_waypoint.c
//
void CG_Q3F_Waypoint();
void CG_Q3F_WaypointInit();
void CG_Q3F_WaypointMaintain();
void CG_Q3F_WaypointCommand();

//
// cg_q3f_anticheat.c
//
void CG_MakeTheSpankingStopImOuttaHere( void );
void CG_GhettoShouldSpankMeCauseImACheatingBastard( void );
qboolean CG_GhettoSeesEvilCvars( void );
void CG_CanGhettoSpankMe( void );

//
// cg_q3f_rendereffects.c
//
void CG_Q3F_ShaderBeam( const vec3_t start, const vec3_t end, float width, qhandle_t shader, const vec4_t color );
void CG_Q3F_AddBeamToScene( vec3_t *points, const int numpoints, qhandle_t shader, qboolean tileshader, vec4_t startColor, float *endColor, float scaleStart, float scaleEnd );

void CG_SortScoreboard();

//
// cg_q3f_controllable.c
//
qboolean CG_Q3F_CustomMenuExecKey( int key );
void CG_Q3F_CustomMenuKeyEvent( int key );
void CG_Q3F_CustomMenuShow( const char * filename );
int CG_Q3F_CustomMenuItems( void );
const char * CG_Q3F_CustomMenuGetItem(int index, int column);


//===============================================

typedef enum {
	INITPHASE_NONE = 0,
	INITPHASE_MAPINFO,
	INITPHASE_MAPBSP,
	INITPHASE_MAPRENDER,
	INITPHASE_MAPENTITIES,
	INITPHASE_SPD,
	INITPHASE_SOUND_STATIC,
	INITPHASE_SOUND_FOOTSTEPS,
	INITPHASE_SOUND_ITEMS,
	INITPHASE_SOUND_DYNAMIC,
	INITPHASE_SOUND_VOICECOMMS,
	INITPHASE_GRAPHIC_STATIC,
	INITPHASE_GRAPHIC_ITEMS,
	INITPHASE_GRAPHIC_WORLD,
	INITPHASE_GRAPHIC_MODELS,
	INITPHASE_GRAPHIC_SHADERS,
	INITPHASE_SPIRIT_DYNAMIC,
	INITPHASE_SPIRIT_STATIC,
	INITPHASE_CLASSES,
	INITPHASE_CLIENTS,
	INITPHASE_UISCRIPTING,
	INITPHASE_SHADERREMAP,
	INITPHASE_SNAPSHOT,

	INITPHASE_NUM,
} initPhase_t;

extern vec3_t idteamcolours[Q3F_TEAM_NUM_TEAMS];

//
// system traps
// These functions are how the cgame communicates with the main game system
//
#ifdef API_Q3

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  vec3_t mins, vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
//void		trap_S_ClearLoopingSounds( qboolean killall );
//void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
//void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
sfxHandle_t	trap_S_RealRegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
//void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
void		trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RealRegisterModel( const char *name );		// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re, const centity_t *cent );	// RR2DO2
void		trap_R_RealAddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_RealAddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_RealAddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
//void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
//void		trap_R_RealAddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_RealAddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h, 
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
//int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
//					   float frac, const char *tagName );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean	trap_R_inPVS( const vec3_t p1, const vec3_t p2 );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
//void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );

void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );

int			trap_RealTime(qtime_t *qtime);

typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t;

typedef enum {
	SB_SHOW,
	SB_SHOW_CHAT,
	SB_HIDE,
} sbShow_e;

int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

void trap_SnapVector( float *v );

qboolean	trap_loadCamera(const char *name);
void		trap_startCamera(int time);
qboolean	trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

#endif // API_Q3

#ifdef API_ET

void trap_PumpEventLoop( void );

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );
int			trap_RealTime(qtime_t *qtime);

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void		trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );


// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );
int			trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int			trap_FS_Delete( const char *filename );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

void		trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );
// ydnar: projects a decal onto brush model surfaces
void		trap_R_ProjectDecal( qhandle_t hShader, int numPoints, vec3_t *points, vec4_t projection, vec4_t color, int lifeTime, int fadeTime );
void		trap_R_ClearDecals( void );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
#define		trap_S_StartSound( origin, entityNum, entchannel, sfx ) trap_S_RealStartSound( origin, entityNum, entchannel, sfx, __FILE__, __LINE__ )
void		trap_S_RealStartSound( const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, const char* file, int line );
void		trap_S_StartSoundVControl( const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int volume );
void		trap_S_StartSoundEx( const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags );
void		trap_S_StartSoundExVControl( const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags, int volume );
void		trap_S_StopStreamingSound(int entnum);	// usually AI.  character is talking and needs to be shut up /now/
int			trap_S_GetSoundLength(sfxHandle_t sfx);
int			trap_S_GetCurrentSoundTime( void );	// ydnar

// a local sound is always played full volume
#define		trap_S_StartLocalSound( sfx, channelNum ) trap_S_RealStartLocalSound( sfx, channelNum, __FILE__, __LINE__ )
void		trap_S_RealStartLocalSound( sfxHandle_t sfx, int channelNum, const char* file, int line );
//void		trap_S_ClearLoopingSounds( void );
void		trap_S_ClearSounds( qboolean killmusic );
//void		trap_S_AddLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume, int soundTime );
//void		trap_S_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range, int volume, int soundTime );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// Ridah, talking animations
int			trap_S_GetVoiceAmplitude( int entityNum );
// done.

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
sfxHandle_t	trap_S_RealRegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
//void		trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime );	// empty name stops music
void		trap_S_FadeBackgroundTrack( float targetvol, int time, int num);
void		trap_S_StopBackgroundTrack( void );
int			trap_S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation );
void		trap_S_FadeAllSound(float targetvol, int time, qboolean stopsounds);	

void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RealRegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

qboolean	trap_R_GetSkinModel( qhandle_t skinid, const char *type, char *name );			//----(SA) added
qhandle_t	trap_R_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap );	//----(SA)	added

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re, const centity_t *cent );	// RR2DO2
void		trap_R_RealAddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_RealAddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer );
void		trap_R_RealAddPolyBufferToScene( polyBuffer_t* pPolyBuffer );
// Ridah
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_RealAddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
// done.
//void		trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
//void		trap_R_RealAddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void		trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible);
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h, 
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_DrawRotatedPic( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );		// NERVE - SMF
void		trap_R_DrawStretchPicGradient( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType );
void		trap_R_Add2dPolys( polyVert_t* verts, int numverts, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
//int			trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

// Save out the old render info so we don't kill the LOD system here
void trap_R_SaveViewParms();

// Reset the view parameters
void trap_R_RestoreViewParms();

// Save out the old render info so we don't kill the LOD system here
void trap_R_SaveViewParms();

// Reset the view parameters
void trap_R_RestoreViewParms();

// Set fog
void	trap_R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density);
void	trap_R_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon/holdable select and zoom
//void		trap_SetUserCmdValue( int stateValue, int flags, float sensitivityScale, int mpIdentClient );
void		trap_SetClientLerpOrigin( float x, float y, float z );		// DHM - Nerve

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
void		trap_Key_KeysForBinding( const char* binding, int* key1, int* key2 );
int			trap_Key_GetKey( const char *binding );
qboolean	trap_Key_GetOverstrikeMode( void );
void		trap_Key_SetOverstrikeMode( qboolean state );

// RF
void trap_SendMoveSpeedsToGame( int entnum, char *movespeeds );

//void trap_UI_Popup(const char *arg0);	//----(SA)	added
void trap_UI_Popup( int arg0 );

// NERVE - SMF
qhandle_t getTestShader(void); // JPW NERVE shhh
void trap_UI_ClosePopup( const char *arg0);
void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void trap_Key_SetBinding( int keynum, const char *binding );
void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
// -NERVE - SMF

char* trap_TranslateString( const char *string );		// NERVE - SMF - localization

int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

void trap_SnapVector( float *v );

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );
qboolean	trap_R_inPVS( const vec3_t p1, const vec3_t p2 );
void		trap_GetHunkData( int* hunkused, int* hunkexpected );

// Duffy, camera stuff
#define CAM_PRIMARY 0	// the main camera for cutscenes, etc.
qboolean	trap_loadCamera(int camNum, const char *name);
void		trap_startCamera(int camNum, int time);
void		trap_stopCamera(int camNum);
qboolean	trap_getCameraInfo(int camNum, int time, vec3_t *origin, vec3_t *angles, float *fov);

#endif // API_ET

// shared syscalls
void		trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void		trap_R_RealAddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
int			trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume, int soundTime );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range, int volume, int soundTime );
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime );	// empty name stops music
void		trap_SetUserCmdValue( int stateValue, int flags, float sensitivityScale, int mpIdentClient );

void CG_Q3F_ConcussionEffect2( int* x, int* y );

qboolean CG_CullPointAndRadius( const vec3_t pt, vec_t radius );

#endif	//__CG_LOCAL_H
