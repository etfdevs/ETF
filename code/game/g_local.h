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

// g_local.h -- local definitions for game module

#ifndef	__G_LOCAL_H
#define	__G_LOCAL_H

#include "q_shared.h"
#include "bg_public.h"
#include "bg_q3f_flyby.h"
#include "bg_q3f_grenades.h"
#include "g_public.h"
#include "g_q3f_mapdata.h"

#ifdef API_ET
#include "be_aas.h"
#endif

//disable deprecated warnings
#if defined(_MSC_VER)
#pragma warning( disable : 4996 )
#endif

//==================================================================

// the "gameversion" client command will print this plus compile date

#define	Q3F_CHANNEL_MAX				16			// This many chat channels per client
#define Q3F_MAX_AMMOBOXES			3
#define Q3F_AMMOBOX_DELAY			400			// RR2DO2: nr of msecs between throwing of two ammoboxes
#define	Q3F_AUTH_TIME				240000		// RR2DO2: time the client has to auth itself to the server
#define	Q3F_AUTH_NEXT_TIME			260000		// RR2DO2: period that the client is asked for authentication
#define Q3F_MAX_FLAMES_PER_PERSON	3
#define Q3F_THROW_ANIM_DURATION		250			// RR2DO2: throw anim takes 250 msec
#define Q3F_OPERATE_ANIM_DURATION	600			// RR2DO2: operating anim takes 600 msec

#define BODY_QUEUE_SIZE				8

#define	FRAMETIME					100			// msec
#define	CARNAGE_REWARD_TIME			3000
#define REWARD_SPRITE_TIME			2000

#define	INTERMISSION_DELAY_TIME		1000
#define	SP_INTERMISSION_DELAY_TIME	5000

// gentity->flags
#define	FL_GODMODE					0x00000010
#define	FL_NOTARGET					0x00000020
#define	FL_TEAMSLAVE				0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK				0x00000800
#define FL_DROPPED_ITEM				0x00001000
#define FL_NO_BOTS					0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS				0x00004000	// spawn point just for bots
#define	FL_WARMUP_ONLY				0x00008000

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	qboolean	inuse;

	char		*classname;			// set in QuakeEd
	int			spawnflags;			// set in QuakeEd

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed
	
	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	int			eventIndex;
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects, 
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2, pos3;

	char		*message;

	int			timestamp;		// body queue sinking, etc
								// slothy: timestamp used for visual aid in mover
	int			timestamp2;

	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*team;
	gentity_t	*target_ent;
	char		*targetShaderName;
	char		*targetShaderNewName;

// XreaL BEGIN
#ifdef _ETXREAL
	// Doom 3 mapping convention: every entity has a name
	// ETXMap adds autogenerated targetnames for entities which have no targetname
	qboolean		targetnameAutogenerated;
	qboolean		targetnameFixedDuplicated;
#endif
// XreaL END

	float		speed;
	vec3_t		movedir;

	int			nextthink;
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*free)( gentity_t *ent );
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage );
	void		(*drop)(gentity_t *self, gentity_t *attacker);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

	int			health;

	qboolean	takedamage;

	int			damage;
	int			splashDamage;	// quad will increase this without increasing radius
	int			methodOfDeath;
	int			splashMethodOfDeath;
	int			count;

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;

	gitem_t		*item;			// for bonus items

	// Golliwog: Q3F stuff
	struct q3f_mapent_s *mapdata;
	// Golliwog.

	int			nextbeamhittime;
	qboolean	hasbeeninwater;	// RR2DO2: for water splashes

	int			spawnIndex;

	vec3_t		oldOrigin;
};

// Golliwog: Admin levels
typedef enum {
	ADMIN_NONE,
	ADMIN_MATCH,
	ADMIN_FULL,
} adminLevel_t;
// Golliwog.

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_CHASE,
	SPECTATOR_FLYBY,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef struct {
	playerTeamStateState_t	state;

	int			location;

//	int			captures;
//	int			basedefense;
//	int			carrierdefense;
//	int			flagrecovery;
//	int			fragcarrier;
//	int			assists;

//	float		lasthurtcarrier;
//	float		lastreturnedflag;
//	float		flagsince;
//	float		lastfraggedcarrier;
} playerTeamState_t;

// Golliwog: Used for location data
typedef struct g_q3f_location_s {
	vec3_t pos;
	char *str;
} g_q3f_location_t;
typedef struct g_q3f_waypoint_s {
	vec3_t pos;
	char *str;
} g_q3f_waypoint_t;
// Golliwog.

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	//team_t		sessionTeam;
	q3f_team_t	sessionTeam;		// RR2DO2
	int			sessionClass;		// Golliwog
	int			spectatorTime;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode


	// Golliwog: I'm also keeping data here that should stay across team changes
	// and the like - mostly, cheat tracking.
	qboolean	versionOK;		// We've version checked them successfully
	char		*ipStr;
	char		guidStr[33];
	int			adminAttempts;
	int			adminLevel;
	int			lastTeamChangeTime;
	int			lastDismantleTime;
	int			lastTeamKillTime;
	int			teamKillHeat;
	qboolean	teamKillWarn;
	int			enemyKill, allyKill;	// Only used for TK monitoring

	int			lives;					// Number of lives player has

	qboolean	adjustAgentSpeed;	// RR2DO2: use disguised class' speed?

	/* Ensiform - For muted state, from ET */
	qboolean	muted;

	qboolean	shoutcaster;

	/* Ensiform - For /ignore , from ET */
	int ignoreClients[MAX_CLIENTS / ( sizeof( int ) * 8 )];
} clientSession_t;

#define MAX_NETNAME			36
#define	MAX_VOTE_COUNT		3

#define NUM_PING_SAMPLES 64

#define STATS_WP 0
#define STATS_GREN (STATS_WP + WP_NUM_WEAPONS)
#define STATS_SENTRY (STATS_GREN + Q3F_NUM_GRENADES)
#define STATS_SUPPLY (STATS_SENTRY + 1)
#define STATS_OTHER (STATS_SUPPLY + 1)
#define STATS_NUM (STATS_OTHER + 1)

typedef struct {
	struct {
		int		shots, hits;
		int		kills, deaths;
		int		given, taken;
	} data [STATS_NUM];
	gclient_t * lastdamage;
	int			caps, assists, defends;
	int			teamkills;
} clientStats_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	char		newnetname[MAX_NETNAME];	// Golliwog: desired name.
	int			namechangeTime;				// Time name was last changed.
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	int			voteCount;			// to prevent people from constantly calling votes
	int			autoReload;			// CaNaBiS 0,1,2 Different types of weapon reloading
	gentity_t	*AmmoBoxes[Q3F_MAX_AMMOBOXES];			// RR2DO2: ammo boxes in the world
	qboolean	initializing;		// Golliwog: true if the client is initializing.
	int			gender;
	qboolean	isReady;			// RR2DO2: for ready-startup

//unlagged - these correspond with variables in the userinfo string
	int			unlagged;
	int			debugDelag;
	int			cmdTimeNudge;
//unlagged - client options
	int			latentSnaps;
	int			latentCmds;
	int			plOut;
	usercmd_t	cmdqueue[MAX_LATENT_CMDS];
	int			cmdhead;
//unlagged - true ping
	int			realPing;
	int			pingsamples[NUM_PING_SAMPLES];
	int			samplehead;
//unlagged - true ping

	// Canabis, stats block
	clientStats_t stats;
} clientPersistant_t;

#define NUM_CLIENT_HISTORY 17

// everything we need to know to backward reconcile
typedef struct {
	vec3_t		mins, maxs;
	vec3_t		currentOrigin;
	int			leveltime;
} clientHistory_t;

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	char		buttons;
	char		oldbuttons;
	char		wbuttons;
	char		oldwbuttons;
	char		cmdflags;
	char		oldcmdflags;

	vec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector
	qboolean	damage_fromFire;	// if true, we're on fire

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	qboolean	respawnForce;		// Golliwog: Force respawn (e.g. they clicked but there were no free spawn points)
	int			activityTime;		// Golliwog: Last activity noted.
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	int			switchTeamTime;		// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	char		*areabits;

	int			aimtime;			// JT (used by sniper rifle)
	int			legwounds;			// JT - Used for sniper leg-shots.

	// Golliwog: Custom additions
	int			lastgrenTime;			// Time last grenade was thrown
	q3f_array_t	*chatchannels;			// List of chat channels user is on.
	int			agentclass;				// Class agent is pretending to be.
	int			agentteam;				// Team agent is pretending to be.
	gentity_t	*agentdata;				// Agent 'overlay' entity.
	g_q3f_location_t *reportLoc, *deathLoc;	// Report and death locations.
	gentity_t	*sentry, *supplystation;	// Sentry and supplystation
	int			buildTime;				// Time till next build is finished.
	int			buildDelayTime;			// Time before we can build again.
	int			callTime;				// Time before we allow saveme/armorme commands
	int			chatTime;				// Flood protection timer
	qboolean	chatFloodWarning;		// Have they been warned they're flooding?
	int			spectatorTeam;			// Team we're spectating on.
	// Golliwog.

	// Canabis, more custom additions
	int			lastgasTime;			// Time player was last gassed
	// JT: Sniper Stuff
	struct gentity_s *sniperdot;
	// JT: Sundry Stuff
	int			pipecount;			// Number of pipes in use.
	int			tranqTime;		// When will tranq where off?
	gentity_t	*tranqEnt;		// Who did it?
	int			flames;			// How many flames are on me?
	int			diseaseTime;	// When am I next due to be hurt by a disease?
	int			killDelayTime;	// How long 'til I'm allowed to restart?
	gentity_t	*diseaseEnt;	// Who infected me in the first place?
	int			next_scan;		// When am I next due to update scanner info?
	int			last_scanned_client;	// Which is the last client I looked at?
	int			paramedicregenTime;
	int			lastflame;		// What was the number of the last flame I fired?
	gentity_t	*scanner_ent;	// Where's my scanner?
	float		speedscale;		// Alter players speed based on held goalitems
	int			minigunFireTime;	// When did they start shooting
	int			minigunLastFireTime;	// When did they last fire?

	int			torsoanimEndTime;	// when does the dropanim end

	// Canabis: Taunt limits
	int			tauntTime;
	qboolean	tauntHeld;

	// Slothy: player being punished
	int			punishTime;

	// Falcon: Charge
	int			chargeTime;
	gentity_t	*chargeEntity;
	int			chargeDisarmTime;
	gentity_t	*chargeDisarmEnt;
	// Falcon.

	int			mapVote;		// Golliwog: Number of map voted for
	int			NextAmmoBoxDropTime;	// RR2DO2: time last ammobox was dropped

	// RR2DO2: flyby cam data
	trajectory_t	camtraj;
	int				currtrajindex;
	//vec3_t			oldCamPos;
	qboolean		inFlyBy;
	gentity_t		*teleportTransitionEnt;	// Golliwog: Used for respawn 'transition' effects.
	gentity_t		*controllable;		// Golliwog: Controllable entity.
	gentity_t		*repairEnt;			// Golliwog: Entity hit with spanner.
	int				repairTime;			// Golliwog: Time entity remains 'hit' with spanner.

	// supplystation
	int				lastSupplystationTouchTime;
	int				topMarker;
	//Unlagged data
	// the serverTime the button was pressed
	// (stored before pmove_fixed changes serverTime)
	int			attackTime;
	// the head of the history queue
	int			historyHead;
	// the history queue
	clientHistory_t	history[NUM_CLIENT_HISTORY];
	// the client's saved position
	clientHistory_t	saved;			// used to restore after time shift
	// an approximation of the actual server time we received this
	// command (not in 50ms increments)
	int			frameOffset;
	// the last frame number we got an update from this client
	int			lastUpdateFrame;
};

//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	4096

typedef struct {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;
	fileHandle_t	memLogFile;
	fileHandle_t	versionLogFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked
	int			frameTime;				// Gordon: time the frame started, for antilag stuff

	int			startTime;				// level.time the map was started

	int			teamScores[Q3F_TEAM_NUM_TEAMS];
	int			teamFrags[Q3F_TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

//	qboolean	restarted;				// waiting for a map_restart to fire
//	qboolean	preserveSession;		// Preserve the player session if they're rejoining on map_restart

	int			numConnectedClients;
	int			numNonSpectatorClients;	// includes connecting clients
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			lastteamplayersupdate;

	int			snd_fry;				// sound index for standing in lava

	int			warmupModificationCount;	// for detecting if g_warmup is changed

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			spawnIndex;				// The index of the spawned entity.
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	vec3_t		intermission_origin;	// also used for spectator spawns
	vec3_t		intermission_angle;

//	qboolean	locationLinked;			// target_locations get linked
//	gentity_t	*locationHead;			// head of the location list
	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];
	gentity_t	*bodyQueAgentData[BODY_QUEUE_SIZE];	// Lock the class/team

	q3f_array_t	*locationarray;			// Golliwog: List of locations.
	q3f_array_t	*waypointarray;			// Golliwog: List of waypoints.
	q3f_keypairarray_t *targetnameArray;	// Golliwog: Entity name lookup array

	gentity_t	*hudHead;				// Golliwog: head of the HUD list
	gentity_t	*onKillHead;			// Golliwog: head of the onkill list

	qboolean	ctfcompat;				// Golliwog: Set if we're running in CTF compatability mode
	qboolean	wfacompat;				// Golliwog: Set if we're running WFA compatability mode
	qboolean	ceaseFire;				// Golliwog: Set if ceasefire mode is on

	int			friendlyFire;			// Golliwog: Type of FF active.
	int			friendlyFireCount;		// Golliwog: Last modification count for g_friendlyFire

	int			teamPingTime;				// Golliwog: Time to next calculate teampings
	int			teamPreviousBalanceWarning;	// Golliwog: Last balance warning sent to clients

	int			gravityModificationCount;	// Golliwog: Last modification to gravity

	int			minRateModificationCount;	// RR2DO2: Last modification to g_minsnaps
	int			minSnapsModificationCount;	// RR2DO2: Last modification to g_minrate

	int			mapSelectState;				// Golliwog: State of map voting system
	int			nextMapTime;				// Golliwog: Set if the map is supposed to change

	int			mapGameIndices;			// RR2DO2: list of allowed mapindices for this map

	// RR2DO2: flyby cam data
	campath_t	campaths[Q3F_MAX_PATHS];
	int			camNumPaths;
	int			flybyPathIndex;

	int			validatedEntityIndex;	// Golliwog: Last entity validated in by G_Q3F_ValidateEntities.

	q3f_keypairarray_t *mapInfo;		// Golliwog: The loaded .mpi/.mapinfo data (only during init).

//	char		connectedClients[MAX_CLIENTS];	// A set of booleans indicating which clients are connected.

	int			redTeamModificationCount;
	int			blueTeamModificationCount;
	int			greenTeamModificationCount;
	int			yellowTeamModificationCount;

	char		cheatcmd[9];	// command clients send during this map when they are cheating

	char		tinfo[Q3F_TEAM_NUM_TEAMS][1400];

	// actual time this server frame started
	int			frameStartTime;

	// slothy
	char		awards[1400];

	qboolean	nofallingdmg;
	qboolean	noselfdmg;

} level_locals_t;


//
//	Golliwog: Types of friendly fire
//

#define FF_Q3F_NONE		0x00		// Should be zero, to match Q3A version
#define	FF_Q3F_FULL		0x01		// Should be one, to match Q3A verision
#define	FF_Q3F_HALF		0x02
#define	FF_Q3F_ARMOUR	0x03
#define	FF_Q3F_MASK		FF_Q3F_ARMOUR

#define	FF_Q3F_MIRROR		0x00				// Equivalent to NONE
#define	FF_Q3F_MIRRORFULL	0x08
#define	FF_Q3F_MIRRORHALF	0x10
#define	FF_Q3F_MIRRORARMOUR	0x18
#define	FF_Q3F_MIRRORMASK	FF_Q3F_MIRRORARMOUR		


//
// g_spawn.c
//

//
// fields are needed for spawning from the entity string
//
typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_ENTITY,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	const char *name;
	size_t		ofs;
	fieldtype_t	type;
	//int		flags;
} field_t;

//keeg for tracemap support -- could technically change this to older style, but the FILE, LINE is nice
#define		G_SpawnString(		key, def, out ) G_SpawnStringExt	( key, def, out, __FILE__, __LINE__ )
#define		G_SpawnVector2D(	key, def, out ) G_SpawnVector2DExt	( key, def, out, __FILE__, __LINE__ )
qboolean	G_SpawnStringExt( const char *key, const char *defaultString, char **out, const char* file, int line );

// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnBoolean( const char *key, const char *defaultString, qboolean *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnColor( const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );
qboolean	G_ParseSpawnVars( void );
void		G_SpawnGEntityFromSpawnVars( qboolean fromBSP, gentity_t *usethisent );

//keeg for tracemap support
qboolean	G_SpawnVector2DExt( const char *key, const char *defaultString, float *out, const char* file, int line );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
qboolean CheatsOk( gentity_t *ent, qboolean silent );
void StopFollowing( gentity_t *ent, qboolean resetclient );
void BroadcastTeamChange( gclient_t *client, int oldTeam );
qboolean SetTeam( gentity_t *ent, char *s );
void Cmd_FollowCycle_f( gentity_t *ent, int dir, spectatorState_t state );

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

#if 0
void UseHoldableItem( gentity_t *ent );
void PrecacheItem (gitem_t *it);
#endif
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle );
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity );
#if 0
void SetRespawn (gentity_t *ent, float delay);
#endif
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
#if 0
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
#endif
void	Add_Ammo (gentity_t *ent, int weapon, int count);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

void G_Q3F_FreeAmmoBox( gentity_t *ent ); // RR2DO2

//
// g_utils.c
//

void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
void G_ResetRemappedShaders();
const char *BuildShaderStateConfig();

int G_ModelIndex( const char *name );
int	G_SoundIndex( const char *name );
int	G_ShaderIndex( const char *name );		// Much like modelindex
int G_SpiritScriptIndex( const char *name );

void	G_TeamCommand( q3f_team_t team, char *cmd );	//RR2DO2
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
gentity_t *G_PickTarget (char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( const vec3_t origin, int event );
void	G_Sound( gentity_t *ent, int channel, int soundIndex );
void	G_FreeEntity( gentity_t *e );
qboolean	G_EntitiesFree( void );

void	G_TouchTriggers (gentity_t *ent);
//void	G_TouchSolids (gentity_t *ent); //djbob: not implemented ????

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

float vectoyaw( const vec3_t vec );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, const vec3_t origin );

// Falcon: bot stuff
//qboolean G_Q3F_Infront (gentity_t *self, gentity_t *other);
//qboolean G_Q3F_Visible(gentity_t *self, gentity_t *other);
// Falcon.

//
// g_combat.c
//
void G_DamageStats(gclient_t *target, gclient_t *attacker, int damage, int mod );
void G_DeathStats(gclient_t *target, gclient_t *attacker, int mod );
int G_StatsModIndex( int mod );

qboolean CanDamage (gentity_t *targ, vec3_t origin, gentity_t *attacker );
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
void G_RadiusDamage (vec3_t origin, gentity_t *inflictor, gentity_t *attacker, float damage, gentity_t *ignore, int mod, int dflags );
void G_NapalmRadiusDamage (vec3_t origin, gentity_t *attacker, float damage, gentity_t *ignore, int mod);
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
void G_Q3F_RegisterTeamKill( gentity_t *attacker, gentity_t *obituary );


// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // armor, shields, invulnerability, and godmode have no effect
#define	DAMAGE_NO_GIB				0x00000010	// Golliwog: Don't gib the entity
//#define	DAMAGE_HIGH_KNOCKBACK		0x00000020	// RR2DO2: 160% knockback on hit
//#define	DAMAGE_EXTRA_KNOCKBACK		0x00000800	// Golliwog: 15% extra knockback on hit
#define	DAMAGE_EXTRA_SELF			0x00001000	// Golliwog: 80% self damage.
#define	DAMAGE_MIRROR				0x00002000	// Golliwog: Mirror damage rather than normal damage
#define DAMAGE_GRENADERADIUS		0x00004000	// RR2DO2: Grenade radius damage, smaller than normal radiusdamage

#define	DAMAGE_Q3F_SHELL			0x00000040	// Shell damage
#define DAMAGE_Q3F_NAIL				0x00000080	// Nail damage
#define DAMAGE_Q3F_EXPLOSION		0x00000100	// Explosive damage
#define DAMAGE_Q3F_SHOCK			0x00000200	// Electrical damage
#define	DAMAGE_Q3F_FIRE				0x00000400	// Fire damage
#define	DAMAGE_Q3F_MASK				(DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_EXPLOSION|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE)

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );

gentity_t *fire_flame (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_mapflame (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_pipe (gentity_t *self, vec3_t start, vec3_t aimdir);	// JT
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_nail (gentity_t *self, vec3_t start, vec3_t dir, int damage, int mod); // RR2DO2


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );
void SpectatorTouch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );

//
// g_trigger.c
//
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace );

//
// g_target.c
//
void G_Q3F_LocationSort();

//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
void G_RunPortal( gentity_t *portal );

//
// g_weapon.c
//
void CalcMuzzlePoint ( const gentity_t *ent, vec3_t muzzle, vec3_t forward );
void TraceMuzzlePoint ( const gentity_t *ent, vec3_t muzzle, vec3_t forward, const vec3_t mins, const vec3_t maxs );

//g_unlagged.c
void G_ResetHistory( gentity_t *ent );
void G_StoreHistory( gentity_t *ent );
void G_TimeShiftAllClients( int time, gentity_t *skip );
void G_UnTimeShiftAllClients( gentity_t *skip );
void G_DoTimeShiftFor( gentity_t *ent );
void G_UndoTimeShiftFor( gentity_t *ent );
void G_UnTimeShiftClient( gentity_t *client );
void G_UnlaggedTrace( gentity_t *ent, trace_t *results, const vec3_t start,
							const vec3_t mins, const vec3_t maxs,
							const vec3_t end, int passEntityNum, int contentmask );

// g_client.c
int TeamCount( int ignoreClientNum, q3f_team_t team );	// RR2DO2
#if 0
int TeamLeader( int team );
#endif
q3f_team_t PickTeam( int ignoreClientNum );				// RR2DO2
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, gentity_t *ent );
void CopyToBodyQue( gentity_t *ent );
void respawn (gentity_t *ent);
void BeginIntermission (void);
#if 0
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
#endif
void InitBodyQue (void);
qboolean ClientSpawn( gentity_t *ent );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, vec3_t origin, int score );
void CalculateRanks( void );
qboolean SpotWouldTelefrag( gentity_t *spot );

//
// g_q3f_playerclass.c
//
void G_Q3F_DropClient( gentity_t *ent, const char *reason );
void G_Q3F_MuteClient( gentity_t *ent, qboolean mute );


//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );
//void G_ProcessIPBans(void);
//qboolean G_FilterPacket (char *from);

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent );

//
// p_hud.c
//
void MoveClientToIntermission (gentity_t *client);
#if 0
void G_SetStats (gentity_t *ent);
#endif
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_cmds.c
//
//static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message );

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
#if 0
void SetLeader(int team, int client);
void CheckTeamLeader( int team );
#endif
void G_RunThink (gentity_t *ent);
void QDECL G_LogPrintf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
void QDECL G_Error( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );
void LogExit( const char *string );
void G_Q3F_CeaseFire(qboolean state);
void G_Q3F_RestartMap(void);
void G_SetMatchState(int state);

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum, char *reason );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

void G_RunTeleportTransition( gentity_t *ent );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
//void Team_CheckDroppedItem( gentity_t *dropped );
//qboolean CheckObeliskAttack( gentity_t *obelisk, gentity_t *attacker );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_Free( void *ptr );		// Golliwog: Counterpart to G_Alloc
void G_InitMemory( void );
void G_DefragmentMemory( void );	// Golliwog: For those big jobs :)
void Svcmd_GameMem_f( void );
const char *String_Alloc(const char *p);

#ifdef DEBUG_MEM
void G_MemDebug_Init();				// djbob: debugging
void G_MemDebug_Close();
#endif

void G_VersionCheck_Init();
void G_VersionCheck_Close();
void G_VersionLogUser( gentity_t *ent );
void QDECL G_VersionLogPrintf( const char *fmt, ... ) _attribute( ( format( printf,1,2 ) ) );

// Golliwog: String handling commands
// g_q3f_string.c
//
qboolean G_Q3F_AddString( char **target, char *str );
void G_Q3F_RemString( char **target );
char *G_Q3F_GetString( char *str );
void Q3F_Svcmd_GameStrings_f( void );
void G_Q3F_InitStrings();
// Golliwog.

// Golliwog: Agent commands.
qboolean G_Q3F_IsDisguised( gentity_t *ent );
void G_Q3F_StartAgentDisguise( gentity_t *ent, int agentclass, int agentteam, disguising_t dt );
void G_Q3F_StopAgentDisguise( gentity_t *ent );
void G_Q3F_StartAgentInvisible( gentity_t *ent );
void G_Q3F_StopAgentInvisible( gentity_t *ent );
void G_Q3F_DisguiseCommand( gentity_t *ent );
void G_Q3F_RunAgentData( gentity_t *ent );
//void G_Q3F_FeignDeathCommand( gentity_t *ent );
void G_Q3F_InvisibleCommand( gentity_t *ent );
// Golliwog.

// Golliwog: Engineer commands
void G_Q3F_SentryBuild( gentity_t *ent );
qboolean G_Q3F_SentryCancel( gentity_t *sentry );
qboolean G_Q3F_SupplyStationCancel( gentity_t *supplystation );
void G_Q3F_EngineerBuild_Command( gentity_t *ent );
void G_Q3F_EngineerDestroy_Command( gentity_t *ent );
void G_Q3F_RunSentry( gentity_t *sentry );
void G_Q3F_RunSupplystation( gentity_t *supplystation );
void G_Q3F_SentryDie( gentity_t *sentry, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void G_Q3F_SupplyStationDie( gentity_t *sentry, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void G_Q3F_Supply_Command( gentity_t *player );
void G_Q3F_UpdateEngineerStats( gentity_t *player );
gentity_t *G_Q3F_CheckSentryUpgradeable( gentity_t *player, int sentrynum, qboolean checkforRepairent );
gentity_t *G_Q3F_CheckSupplyStation( gentity_t *player, int suppnum );

void G_Q3F_SentryUpgrade( gentity_t *player, int sentrynum );
void G_Q3F_SentryRepair( gentity_t *player, int sentrynum );
void G_Q3F_SentryRefill( gentity_t *player, int sentrynum );
void G_Q3F_SupplyStationUpgrade( gentity_t *player, int suppnum );
void G_Q3F_SupplyStationRepair( gentity_t *player, int suppnum );
void G_Q3F_SupplyStationRefill( gentity_t *player, int suppnum );
// Golliwog.

void G_DumpEntityInfo( gentity_t *ent, qboolean coredump, char* prefix, char* title, qboolean client );

// Golliwog: Panel commands.
qboolean G_Q3F_RadiateToSurface( vec3_t focus, float maxDist, vec3_t traceEnd, vec3_t angles, gentity_t **traceEnt, int contentmask, int ignoreEnt );
// Golliwog.

// RR2DO2: Map sentry
void G_Q3F_RunMapSentry( gentity_t *ent );
// RR2DO2

void G_Q3F_ValidateEntities();

// Golliwog: Server configuration commands commands
void G_Q3F_LoadServerConfiguration( qboolean testMode );
void G_Q3F_ExecuteSetting( char *mapexec, int gameindex );
void G_Q3F_UnloadServerConfiguration();
void G_Q3F_TestServerConfiguration();
// Golliwog.

// Golliwog: Map info commands
q3f_keypairarray_t *G_Q3F_LoadMapInfo( char *mapname );
char *G_Q3F_GetMapInfoEntry( q3f_keypairarray_t *mpi, char *key, int gameindex, char *defstr );
void G_Q3F_CheckGameIndex();
q3f_keypairarray_t *G_Q3F_LoadMapHistory();
void G_Q3F_UpdateMapHistory( char *mapname );
q3f_array_t *G_Q3F_GetAvailableMaps();

//
// g_q3f_controllable.c
//

qboolean G_Q3F_Control( gentity_t *ent );

//
// g_q3f_waypoint.c
//
void G_Q3F_WaypointBuildArray();
void G_Q3F_WaypointCommand( gentity_t *ent );

//
// g_session.c
//
void G_ReadClientSessionData( gclient_t *client );
void G_InitClientSessionData( gclient_t *client, char *userinfo );

void G_ReadSessionData( void );
void G_WriteSessionData( void );

//
// g_arenas.c
//


// RR2DO2: crc stuff
// g_q3f_crc.c
//
int G_Q3F_GetCRC( const char *filename );

//
// g_q3f_soundscript.c
//
qboolean G_Q3F_SSCR_ParseSoundScript( char *mapname );

//
// g_q3f_weapon.c
//
void G_Q3F_Burn_Person(struct gentity_s *target, struct gentity_s *attacker/*, int damage*/);
void G_Q3F_DebugLine( const vec3_t start, const vec3_t end, const vec4_t color);
void G_Q3F_DebugTrace( const vec3_t start, const trace_t * tr );
void G_Q3F_DebugBox( const vec3_t origin, const vec3_t mins, const vec3_t maxs, const vec4_t color );
void G_Q3F_MuzzleTraceBox( gentity_t *ent, const vec3_t muzzle, const vec3_t forward );

//
// g_q3f_ready.c
//
qboolean CheckEveryoneReady( void );
void NotReadyNames( char *names, size_t size );
void WarnNotReadyClients( void );
void Cmd_Ready_f( gentity_t *ent );
void Cmd_UnReady_f( gentity_t *ent );

//
// g_demos.c
//
//void G_ServerDemoCommand(void);

#ifdef id386
void G_PatchEngine(void);
uintptr_t G_GetSyscall(void);
#endif

#include "g_team.h" // teamplay specific stuff

extern	level_locals_t	level;
extern	gentity_t		g_entities[MAX_GENTITIES];

#define	FOFS(x) offsetof(gentity_t, x)

extern	vmCvar_t	g_OmniBotPath;
extern	vmCvar_t	g_OmniBotEnable;
extern	vmCvar_t	g_OmniBotFlags;
extern	vmCvar_t	g_OmniBotPlaying;

extern	vmCvar_t	g_gametype;
extern	vmCvar_t	g_gameindex;
extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_restarted;
extern	vmCvar_t	g_teamFrags;			// Slothy - team frags


extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_password;
extern	vmCvar_t	g_needpass;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_quadfactor;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_synchronousClients;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_blood;
extern	vmCvar_t	g_allowVote;
extern	vmCvar_t	g_teamAutoJoin;
//extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_banIPs;
extern	vmCvar_t	g_filterBan;
//extern	vmCvar_t	g_obeliskHealth;
//extern	vmCvar_t	g_obeliskRegenPeriod;
//extern	vmCvar_t	g_obeliskRegenAmount;
//extern	vmCvar_t	g_obeliskRespawnDelay;
//extern	vmCvar_t	g_cubeTimeout;
extern	vmCvar_t	g_redteam;
extern	vmCvar_t	g_blueteam;
extern	vmCvar_t	g_yellowteam;
extern	vmCvar_t	g_greenteam;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;
extern	vmCvar_t	g_rankings;
extern	vmCvar_t	g_singlePlayer;
//extern	vmCvar_t	g_proxMineTimeout;
extern	vmCvar_t	g_suicideDelay;
extern	vmCvar_t	g_teamChatSounds;
//extern	vmCvar_t	g_grenadeScale;
//extern	vmCvar_t	g_grenadeConcBlast;
//extern	vmCvar_t	g_grenadeThrow;
//extern	vmCvar_t	g_grenadeZorg;
//extern	vmCvar_t	g_grenadeZvel;
//extern	vmCvar_t	g_grenadeDropZVel;
//extern	vmCvar_t	g_grenadeDropDelay;
//extern	vmCvar_t	g_grenadeDropZ;
//extern	vmCvar_t	g_grenadeBounce;
//extern	vmCvar_t	g_grenadeStick;
//extern	vmCvar_t	g_grenadePJdelay;
//extern	vmCvar_t	g_grenadeConcHBlast;
//extern	vmCvar_t	g_grenadeConcVBlast;
//extern	vmCvar_t	g_grenadeHBlast;
//extern	vmCvar_t	g_grenadeVBlast;
//extern	vmCvar_t	g_pipedetdelay;
//extern	vmCvar_t	g_pipeLauncherVel;
//extern	vmCvar_t	g_radiusDamageScale;
//extern	vmCvar_t	g_supplyStationMaxDamage;
//extern	vmCvar_t	g_napalmRocketVel;
//extern	vmCvar_t	g_napalmKnockBack;
//extern	vmCvar_t	g_MissileImpactKnockback;
extern	vmCvar_t	g_mapentDebug;
//extern	vmCvar_t	g_nailGrenUpdateInterval;
//extern		vmCvar_t	g_sentryRotSpeed;
//extern		vmCvar_t	g_sentryBulletDamage;
//extern		vmCvar_t	g_sentryLockDelay;

extern	vmCvar_t	g_classReconLimit;
extern	vmCvar_t	g_classSniperLimit;
extern	vmCvar_t	g_classSoldierLimit;
extern	vmCvar_t	g_classGrenadierLimit;
extern	vmCvar_t	g_classParamedicLimit;
extern	vmCvar_t	g_classMinigunnerLimit;
extern	vmCvar_t	g_classFlametrooperLimit;
extern	vmCvar_t	g_classAgentLimit;
extern	vmCvar_t	g_classEngineerLimit;
extern	vmCvar_t	g_adminPassword;
extern	vmCvar_t	g_matchPassword;
extern	vmCvar_t	g_matchState;
extern	vmCvar_t	g_matchMode;
extern	vmCvar_t	g_matchPlayers;
extern	vmCvar_t	g_warmup;
extern	vmCvar_t	g_spectatorMode;	// RR2DO2
extern	vmCvar_t	g_showTeamBalanceWarning;	// RR2DO2
extern	vmCvar_t	g_execMapConfigs;	// RR2DO2
extern	vmCvar_t	g_mapVote;
extern	vmCvar_t	g_minRate;
extern	vmCvar_t	g_minSnaps;
extern	vmCvar_t	g_teamKillRules;
extern	vmCvar_t	g_banRules;
extern	vmCvar_t	score_red;
extern	vmCvar_t	score_blue;
extern	vmCvar_t	score_yellow;
extern	vmCvar_t	score_green;
//extern	vmCvar_t	sv_warmup;
//extern	vmCvar_t	sv_time;
//extern	vmCvar_t	sv_levelStartTime;
extern	vmCvar_t	players_red;
extern	vmCvar_t	players_blue;
extern	vmCvar_t	players_yellow;
extern	vmCvar_t	players_green;
extern	vmCvar_t	g_agentHitBeep;
extern	vmCvar_t	g_serverConfigTime;
extern	vmCvar_t	sv_floodProtect;
//extern	vmCvar_t	g_q3f_autoSentrySpinup;
//extern	vmCvar_t	g_q3f_autoSentryPostLock;
//extern	vmCvar_t	g_q3f_autoSentryMadDog;
extern	vmCvar_t	g_debugBullets;
//Unlagged
extern	vmCvar_t	g_unlagged;
extern	vmCvar_t	g_unlaggedVersion;
extern	vmCvar_t	g_truePing;
extern	vmCvar_t	sv_fps;

extern	vmCvar_t	g_allowAllVersions;

extern	vmCvar_t	g_adminFloodImmunity;

extern	vmCvar_t	sv_maxConnections;
extern	vmCvar_t	sv_maxConnectionTime;
extern	vmCvar_t	sv_maxConnectionBan;
extern	vmCvar_t	sv_maxConnectionKick;

extern	vmCvar_t	g_shoutcastPassword;

#ifdef BUILD_LUA
extern	vmCvar_t	lua_modules;
extern	vmCvar_t	lua_allowedModules;
#endif

extern	vmCvar_t	g_spawnFullStats;

#ifdef API_Q3
void	trap_Printf( const char *fmt );
void	trap_Error( const char *fmt );
int		trap_Milliseconds( void );
int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void	trap_Args( char *buffer, int bufferLength );
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );
int		trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
void	trap_SendConsoleCommand( int exec_when, const char *text );
void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	trap_Cvar_Update( vmCvar_t *cvar );
void	trap_Cvar_Set( const char *var_name, const char *value );
int		trap_Cvar_VariableIntegerValue( const char *var_name );
float	trap_Cvar_VariableValue( const char *var_name );
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
//void	trap_DropClient( int clientNum, const char *reason );
void	trap_SendServerCommand( int clientNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
void	trap_SetBrushModel( gentity_t *ent, const char *name );
void	trap_Trace( trace_t *results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( void );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);
int		trap_RealTime( qtime_t *qtime );

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibVarGet(char *var_name, char *value, int size);
int		trap_BotLibDefine(char *string);
int		trap_PC_LoadSource( const char *filename );
int		trap_PC_FreeSource( int handle );
int		trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int		trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
void	trap_BotUserCommand(int client, usercmd_t *ucmd);

int		trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
int		trap_AAS_AreaInfo( int areanum, void /* struct aas_areainfo_s */ *info );
void	trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int		trap_AAS_Initialized(void);
void	trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float	trap_AAS_Time(void);

int		trap_AAS_PointAreaNum(vec3_t point);
int		trap_AAS_PointReachabilityAreaIndex(vec3_t point);
int		trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);

int		trap_AAS_PointContents(vec3_t point);
int		trap_AAS_NextBSPEntity(int ent);
int		trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int		trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int		trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int		trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int		trap_AAS_AreaReachability(int areanum);

int		trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
int		trap_AAS_EnableRoutingArea( int areanum, int enable );
int		trap_AAS_PredictRoute(void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);

int		trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
										int type);
int		trap_AAS_Swimming(vec3_t origin);
int		trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);

void	trap_EA_Say(int client, char *str);
void	trap_EA_SayTeam(int client, char *str);
void	trap_EA_Command(int client, char *command);

void	trap_EA_Action(int client, int action);
void	trap_EA_Gesture(int client);
void	trap_EA_Talk(int client);
void	trap_EA_Attack(int client);
void	trap_EA_Use(int client);
void	trap_EA_Respawn(int client);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Crouch(int client);
void	trap_EA_MoveUp(int client);
void	trap_EA_MoveDown(int client);
void	trap_EA_MoveForward(int client);
void	trap_EA_MoveBack(int client);
void	trap_EA_MoveLeft(int client);
void	trap_EA_MoveRight(int client);
void	trap_EA_SelectWeapon(int client, int weapon);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Move(int client, vec3_t dir, float speed);
void	trap_EA_View(int client, vec3_t viewangles);

void	trap_EA_EndRegular(int client, float thinktime);
void	trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void	trap_EA_ResetInput(int client);


int		trap_BotLoadCharacter(char *charfile, float skill);
void	trap_BotFreeCharacter(int character);
float	trap_Characteristic_Float(int character, int index);
float	trap_Characteristic_BFloat(int character, int index, float min, float max);
int		trap_Characteristic_Integer(int character, int index);
int		trap_Characteristic_BInteger(int character, int index, int min, int max);
void	trap_Characteristic_String(int character, int index, char *buf, int size);

int		trap_BotAllocChatState(void);
void	trap_BotFreeChatState(int handle);
void	trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void	trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		trap_BotNumConsoleMessages(int chatstate);
void	trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotNumInitialChats(int chatstate, char *type);
int		trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
void	trap_BotSetChatName(int chatstate, char *name, int client);
void	trap_BotEnterChat(int chatstate, int client, int sendto);
void	trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		trap_StringContains(char *str1, char *str2, int casesensitive);
int		trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	trap_UnifyWhiteSpaces(char *string);
void	trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void	trap_BotSetChatGender(int chatstate, int gender);
void	trap_BotResetGoalState(int goalstate);
void	trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	trap_BotResetAvoidGoals(int goalstate);
void	trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	trap_BotPopGoal(int goalstate);
void	trap_BotEmptyGoalStack(int goalstate);
void	trap_BotDumpAvoidGoals(int goalstate);
void	trap_BotDumpGoalStack(int goalstate);
void	trap_BotGoalName(int number, char *name, int size);
int		trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int		trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
int		trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int		trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
float	trap_BotAvoidGoalTime(int goalstate, int number);
void	trap_BotSetAvoidGoalTime(int goalstate, int number, float avoidtime);
void	trap_BotInitLevelItems(void);
void	trap_BotUpdateEntityItems(void);
int		trap_BotLoadItemWeights(int goalstate, char *filename);
void	trap_BotFreeItemWeights(int goalstate);
void	trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void	trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int		trap_BotAllocGoalState(int state);
void	trap_BotFreeGoalState(int handle);

void	trap_BotResetMoveState(int movestate);
void	trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void	trap_BotResetAvoidReach(int movestate);
void	trap_BotResetLastAvoidReach(int movestate);
int		trap_BotReachabilityArea(vec3_t origin, int testground);
int		trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
int		trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
int		trap_BotAllocMoveState(void);
void	trap_BotFreeMoveState(int handle);
void	trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
void	trap_BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type);

int		trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int		trap_BotAllocWeaponState(void);
void	trap_BotFreeWeaponState(int weaponstate);
void	trap_BotResetWeaponState(int weaponstate);

int		trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);

void	trap_SnapVector( float *v );
#endif // API_Q3

#ifdef API_ET
void	trap_Printf( const char *fmt );
void	trap_Error( const char *fmt );
int		trap_Milliseconds( void );
int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void	trap_Args( char *buffer, int bufferLength );
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
int		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
int		trap_FS_Rename( const char *from, const char *to );
void	trap_FS_FCloseFile( fileHandle_t f );
int		trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
void	trap_SendConsoleCommand( int exec_when, const char *text );
void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void	trap_Cvar_Update( vmCvar_t *cvar );
void	trap_Cvar_Set( const char *var_name, const char *value );
int		trap_Cvar_VariableIntegerValue( const char *var_name );
float	trap_Cvar_VariableValue( const char *var_name );
void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
//void	trap_DropClient( int clientNum, const char *reason, int length );
void	trap_SendServerCommand( int clientNum, const char *text );
void	trap_SetConfigstring( int num, const char *string );
void	trap_GetConfigstring( int num, char *buffer, int bufferSize );
void	trap_GetUserinfo( int num, char *buffer, int bufferSize );
void	trap_SetUserinfo( int num, const char *buffer );
void	trap_GetServerinfo( char *buffer, int bufferSize );
void	trap_SetBrushModel( gentity_t *ent, const char *name );
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_TraceCapsuleNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_TraceNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int		trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void	trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void	trap_LinkEntity( gentity_t *ent );
void	trap_UnlinkEntity( gentity_t *ent );
int		trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int		trap_BotAllocateClient( int clientNum );
void	trap_BotFreeClient( int clientNum );
void	trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean	trap_GetEntityToken( char *buffer, int bufferSize );
qboolean trap_GetTag( int clientNum, int tagFileNumber, char *tagName, orientation_t *ori );
qboolean trap_LoadTag( const char* filename );

int		trap_RealTime( qtime_t *qtime );

int		trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void	trap_DebugPolygonDelete(int id);

int		trap_BotLibSetup( void );
int		trap_BotLibShutdown( void );
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibVarGet(char *var_name, char *value, int size);
int		trap_BotLibDefine(char *string);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);
int		trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);

int		trap_BotGetSnapshotEntity( int clientNum, int sequence );
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
//int		trap_BotGetConsoleMessage(int clientNum, char *message, int size);
void	trap_BotUserCommand(int client, usercmd_t *ucmd);

void		trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);

int			trap_AAS_Initialized(void);
void		trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);
float		trap_AAS_Time(void);

// Ridah
void		trap_AAS_SetCurrentWorld(int index);
// done.

int			trap_AAS_PointAreaNum(vec3_t point);
int			trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);
int			trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
void		trap_AAS_AreaCenter(int areanum, vec3_t center);
qboolean	trap_AAS_AreaWaypoint(int areanum, vec3_t center);

int			trap_AAS_PointContents(vec3_t point);
int			trap_AAS_NextBSPEntity(int ent);
int			trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int			trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int			trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int			trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);

int			trap_AAS_AreaReachability(int areanum);
int			trap_AAS_AreaLadder(int areanum);

int			trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);

int			trap_AAS_Swimming(vec3_t origin);
int			trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize);

// Ridah, route-tables
void		trap_AAS_RT_ShowRoute( vec3_t srcpos, int	srcnum, int destnum );
int			trap_AAS_NearestHideArea(int srcnum, vec3_t origin, int areanum, int enemynum, vec3_t enemyorigin, int enemyareanum, int travelflags, float maxdist, vec3_t distpos);
int			trap_AAS_ListAreasInRange(vec3_t srcpos, int srcarea, float range, int travelflags, float **outareas, int maxareas);
int			trap_AAS_AvoidDangerArea(vec3_t srcpos, int srcarea, vec3_t dangerpos, int dangerarea, float range, int travelflags);
int			trap_AAS_Retreat
(
	// Locations of the danger spots (AAS area numbers)
	int *dangerSpots,
	// The number of danger spots
	int dangerSpotCount,
	vec3_t srcpos, 
	int srcarea, 
	vec3_t dangerpos, 
	int dangerarea, 
	// Min range from startpos
	float range, 
	// Min range from danger
	float dangerRange,
	int travelflags
);
int			trap_AAS_AlternativeRouteGoals(vec3_t start, vec3_t goal, int travelflags,
										 aas_altroutegoal_t *altroutegoals, int maxaltroutegoals,
										 int color);
void		trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, int blocking );
void		trap_AAS_RecordTeamDeathArea( vec3_t srcpos, int srcarea, int team, int teamCount, int travelflags );
// done.

void	trap_EA_Say(int client, char *str);
void	trap_EA_SayTeam(int client, char *str);
void	trap_EA_UseItem(int client, char *it);
void	trap_EA_DropItem(int client, char *it);
void	trap_EA_UseInv(int client, char *inv);
void	trap_EA_DropInv(int client, char *inv);
void	trap_EA_Gesture(int client);
void	trap_EA_Command(int client, char *command);

void	trap_EA_SelectWeapon(int client, int weapon);
void	trap_EA_Talk(int client);
void	trap_EA_Attack(int client);
void	trap_EA_Reload(int client);
void	trap_EA_Activate(int client);
void	trap_EA_Respawn(int client);
void	trap_EA_Jump(int client);
void	trap_EA_DelayedJump(int client);
void	trap_EA_Crouch(int client);
void	trap_EA_Walk(int client);
void	trap_EA_MoveUp(int client);
void	trap_EA_MoveDown(int client);
void	trap_EA_MoveForward(int client);
void	trap_EA_MoveBack(int client);
void	trap_EA_MoveLeft(int client);
void	trap_EA_MoveRight(int client);
void	trap_EA_Move(int client, vec3_t dir, float speed);
void	trap_EA_View(int client, vec3_t viewangles);
void	trap_EA_Prone(int client);

void	trap_EA_EndRegular(int client, float thinktime);
void	trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void	trap_EA_ResetInput(int client, void *init);


int		trap_BotLoadCharacter(char *charfile, int skill);
void	trap_BotFreeCharacter(int character);
float	trap_Characteristic_Float(int character, int index);
float	trap_Characteristic_BFloat(int character, int index, float min, float max);
int		trap_Characteristic_Integer(int character, int index);
int		trap_Characteristic_BInteger(int character, int index, int min, int max);
void	trap_Characteristic_String(int character, int index, char *buf, int size);

int		trap_BotAllocChatState(void);
void	trap_BotFreeChatState(int handle);
void	trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void	trap_BotRemoveConsoleMessage(int chatstate, int handle);
int		trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int		trap_BotNumConsoleMessages(int chatstate);
void	trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotNumInitialChats(int chatstate, char *type);
int		trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int		trap_BotChatLength(int chatstate);
void	trap_BotEnterChat(int chatstate, int client, int sendto);
void	trap_BotGetChatMessage(int chatstate, char *buf, int size);
int		trap_StringContains(char *str1, char *str2, int casesensitive);
int		trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void	trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void	trap_UnifyWhiteSpaces(char *string);
void	trap_BotReplaceSynonyms(char *string, unsigned long int context);
int		trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void	trap_BotSetChatGender(int chatstate, int gender);
void	trap_BotSetChatName(int chatstate, char *name);
void	trap_BotResetGoalState(int goalstate);
void	trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void	trap_BotResetAvoidGoals(int goalstate);
void	trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void	trap_BotPopGoal(int goalstate);
void	trap_BotEmptyGoalStack(int goalstate);
void	trap_BotDumpAvoidGoals(int goalstate);
void	trap_BotDumpGoalStack(int goalstate);
void	trap_BotGoalName(int number, char *name, int size);
int		trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int		trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int		trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime);
int		trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int		trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int		trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);
int		trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
int		trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);
float	trap_BotAvoidGoalTime(int goalstate, int number);
void	trap_BotInitLevelItems(void);
void	trap_BotUpdateEntityItems(void);
int		trap_BotLoadItemWeights(int goalstate, char *filename);
void	trap_BotFreeItemWeights(int goalstate);
void	trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void	trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void	trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int		trap_BotAllocGoalState(int state);
void	trap_BotFreeGoalState(int handle);

void	trap_BotResetMoveState(int movestate);
void	trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags);
int		trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void	trap_BotResetAvoidReach(int movestate);
void	trap_BotResetLastAvoidReach(int movestate);
int		trap_BotReachabilityArea(vec3_t origin, int testground);
int		trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target);
int		trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target);
int		trap_BotAllocMoveState(void);
void	trap_BotFreeMoveState(int handle);
void	trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
// Ridah
void	trap_BotInitAvoidReach(int handle);
// done.

int		trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void	trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int		trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int		trap_BotAllocWeaponState(void);
void	trap_BotFreeWeaponState(int weaponstate);
void	trap_BotResetWeaponState(int weaponstate);

int		trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);

void	trap_SnapVector( float *v );

void trap_PbStat ( int clientNum , char *category , char *values ) ;

int trap_PC_AddGlobalDefine( char *define );
int trap_PC_LoadSource( const char *filename );
int trap_PC_FreeSource( int handle );
int trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
int	trap_PC_UnReadToken( int handle );
#endif // API_ET

// shared syscalls
void	trap_DropClient( int clientNum, const char *reason, int length );

void QDECL G_DebugLog( const char *fmt, ... );

#ifdef _DEBUG
//#define	DEBUGLOG
#endif

#endif//__G_LOCAL_H
