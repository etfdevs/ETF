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

// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef _BG_PUBLIC_H			// JT: Can't believe this wasn't in!
#define _BG_PUBLIC_H

#ifdef DEBUG_CGAME
#include "../cgame/cg_public.h"
#endif
#ifdef DEBUG_GAME
#include "g_public.h"
#endif

#include "bg_q3f_controllable.h"

#ifndef _ANIMATIONS_H
#define _ANIMATIONS_H
#include "../include/animations.h"
#endif

//disable deprecated warnings
#if defined(_MSC_VER)
#pragma warning( disable : 4996 )	 
#endif

//Edit this when there are big changes, so people get a warning 
//when their cgame doesn't match to the server game
#define FORTS_SUB_VERSION	""
//Keeg change game version and name info
#define	GAME_VERSION		"etf"	// RR2DO2: LOWERCASE!!
#define	FORTS_VERSION		"ETF 2.0" FORTS_SUB_VERSION
#define FORTS_SHORTVERSION	"20"	// Slothy: used to version-match in server browser (can't do decimals, whole numbers only)
#define FORTS_VERSIONINT	20		// Ensiform: used to version-match in server browser info for sorting
#define	MAPINFO_TYPE		"etf"  //keeger:  type inside mapinfo file
#define GAME_NAME_CAP		"ETF"  //for where game name should be capitalized

#define GAME_NAME			MAPINFO_TYPE    //keeg same string as mapinfo

//bani
#ifdef __GNUC__
#define _attribute(x) __attribute__(x)
#else
#define _attribute(x)
#endif

#define	DEFAULT_GRAVITY		800
#define	DEFAULT_GRAVITY_STR	"800"

#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.66

#define	MAX_ITEMS			256

#define HOLDBREATHTIME		15000				// time you can swim underwater

#define	RANK_TIED_FLAG		0x4000

#define DEFAULT_SHOTGUN_SPREAD	700		// Original: 0.14 0.08
//#define DEFAULT_SHOTGUN_COUNT	13		etf 1.0
#define DEFAULT_SHOTGUN_COUNT	10

#define DEFAULT_SINGLE_SHOTGUN_SPREAD	300	// Original: 0.04 0.04
//#define DEFAULT_SINGLE_SHOTGUN_COUNT	5	etf 1.0
#define DEFAULT_SINGLE_SHOTGUN_COUNT	4	//etf 1.0

//#define DEFAULT_MINIGUN_PELLET_SPREAD	235//200	// Original: 0.04 0.04
#define DEFAULT_MINIGUN_PELLET_SPREAD	300			// djbob: 2.01c test
#define	EXTRA_MINIGUN_PELLET_SPREAD		1000
#define	DEFAULT_MINIGUN_PELLET_COUNT	5

#define	MAPINFODIR		"maps"
#define	MAPINFOEXT		".mapinfo"
#define	OLDMAPINFOEXT	".mpi"

#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection

#define	LIGHTNING_RANGE		768

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define	MINS_Z				-24
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

#define	Q3F_SENTRY_SPINUP_TIME		400		// Time to 'spin up' before actually firing
#define	Q3F_SENTRY_SPINKEEP_TIME	150		// Time to 'spin down' from ready
#define	Q3F_SENTRY_SPINDOWN_TIME	450		// Time to 'spin down' from ready

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC			2
#define	CS_MESSAGE			3		// from the map worldspawn's message field
#define	CS_MOTD				4		// g_motd string for server message of the day
#define	CS_WARMUP			5		// server time when the match will be restarted
#define CS_VOTE_TIME		6
#define CS_VOTE_STRING		7
#define	CS_VOTE_YES			8
#define	CS_VOTE_NO			9
#define	CS_GAME_VERSION		10
#define	CS_LEVEL_START_TIME	11		// so the timer only shows the current level
#define	CS_INTERMISSION		12		// when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_Q3F_CVARLIMITS	13		// RR2DO2: server set cvar limits overriding the local ones
#define	CS_FORTS_VERSION	14		// Golliwog: Version of the mod, to prevent nasty accidents
#define CS_MATCH_STATE		15		// Canabis: Used to relay information about the state of the match

#define	CS_FORTS_ATMOSEFFECT	19		// Golliwog: Atmospheric effect, if any.
#define	CS_FORTS_CEASEFIRE	20		// Golliwog: Set to 1 if a ceasefire is active.
#define	CS_FORTS_TEAMPINGS	21		// Golliwog: Average pings for each team.
#define	CS_FORTS_MAPVOTENAMES	22		// Golliwog: Set of maps to vote amongst
#define	CS_FORTS_MAPVOTETALLY	23		// Golliwog: Current votes on map selection
#define	CS_ITEMS			25		// string of 0's and 1's that tell which items are present

#define	CS_CLASSMASK		26		// Canabis, bits of which classes to load

#define CS_SHADERSTATE		27
#define CS_TEAMALLIED		28
#define	CS_MAPLIST			29
#define	CS_TEAMMASK			30
#define	CS_TEAMNAMES		31


#define	CS_MODELS			32
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)
#define	CS_SHADERS			(CS_SOUNDS+MAX_SOUNDS)		// Golliwog: Flag graphics for various things
#define	CS_PLAYERS			(CS_SHADERS+MAX_SHADERS)
#define	CS_SPIRITSCRIPTS	(CS_PLAYERS+MAX_CLIENTS)
#define CS_MAX				(CS_SPIRITSCRIPTS+MAX_SPIRITSCRIPTS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {

	GT_FORTS,				// default Q3F

	GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

#define MAX_ARENAS			1024
#define	MAX_ARENAS_TEXT		8192

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
	PM_NORMAL,				// can accelerate and turn
	PM_NOCLIP,				// noclip movement
	PM_SPECTATOR,			// still run into walls
	PM_LIMITEDSPECTATOR,	// RR2DO2: spawns but can't move
	PM_ADMINSPECTATOR,		// RR2DO2: still run into walls, but can move (flying spectator)
	PM_DEAD,				// no acceleration or turning, but free falling
	PM_INVISIBLE,			// Agent invisibility (no movement/turning etc.)
	PM_FREEZE,				// stuck in place with no control
	PM_INTERMISSION,		// no movement or status bar
	PM_SPINTERMISSION		// no movement or status bar
} pmtype_t;

typedef enum {
	WEAPON_READY, 
	WEAPON_RAISING,
	WEAPON_DROPPING,
	WEAPON_RRAISING,		// JT - RRAISING & RDROPPING - State variables for
	WEAPON_RDROPPING,		// tracking reloads
	WEAPON_RELOADING,
	WEAPON_FIRING,
	WEAPON_AIMING,
	WEAPON_STARTING,		// JT - For MINIGUNNER
	WEAPON_STARTED
} weaponstate_t;

// pmove->pm_flags
#define	PMF_DUCKED			0x0001
#define	PMF_JUMP_HELD		0x0002
#define PMF_LADDER			0x0004
#define	PMF_BACKWARDS_JUMP	0x0008		// go into backwards land
#define	PMF_BACKWARDS_RUN	0x0010		// coast down to backwards run
#define	PMF_TIME_LAND		0x0020		// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	0x0040		// pm_time is an air-accelerate only time
#define	PMF_TIME_WATERJUMP	0x0100		// pm_time is waterjump
#define	PMF_RESPAWNED		0x0200		// clear after attack and jump buttons come up
#define	PMF_USE_ITEM_HELD	0x0400
#define PMF_FOLLOW			0x1000		// spectate following another player
#define PMF_SCOREBOARD		0x2000		// spectate as a scoreboard
#define	PMF_CEASEFIRE		0x4000		// No firing
#define PMF_CHASE			0x8000		// Chasecam
//#define PMF_INVULEXPAND		0x10000		// invulnerability sphere set to full size
#define	PMF_Q3F_CONCLAND	0x20000		// Concussion in effect, they've landed at least once.
#define	PMF_Q3F_CONSPEED	0x40000		// Concussion in effect, they've landed at least once.

#define	PMF_ALL_TIMES	(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define PMRF_SKIP_TAUNT		0x00001		// Done a taunt this pmove
#define PMRF_DONE_TAUNT		0x00002		// Done a taunt this pmove

#define	MAXTOUCH	32
typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server
	vec3_t		groundVelocity;		// Golliwog: Allow 'relative to ground' jumps
	entityState_t *cs;				// Golliwog: state of controlled entity.
	controllabledata_t *cdata;		// Golliwog: Information on the obejct being controlled.

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;

	// for fixed msec Pmove
	int			pmove_fixed;
	int			pmove_msec;

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );

	// djbob: for airgasp across cgame/game
	int			airleft;

	// rr2do2: agentclass
	int			agentclass;

	// canabis: return flags for some stuff
	int			retflags;
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum {
	STAT_HEALTH,
	STAT_HOLDABLE_ITEM,
	STAT_WEAPONS,					// 16 bit fields
	STAT_ARMOR,				
	STAT_DEAD_YAW,					// look this direction when dead (FIXME: get rid of?)
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)
	STAT_Q3F_FLAGS,					// JT - General flags.
	STAT_ARMORTYPE,					// FALCON - Armour type
	STAT_Q3F_ARMOURCLASS,			// Golliwog. Armour 'class' (shock/explosive) Etc. (I think type and class are backwards, myself...)
} statIndex_t;

typedef enum {
	Q3F_WEAPON_RELOAD,				// Set when a reload is requested
	Q3F_WEAPON_NORELOAD,			// Set for clients that don't want auto reload
	FL_Q3F_SCANNER,					// Scanner enabled?
	FL_Q3F_LAYCHARGE,				// Laying a charge (no shooting/moving)
	FL_Q3F_BUILDING,				// Building (no shooting/moving)
	FL_Q3F_TRANQ,					// Tranqued? (slows fire reload rates)
	FL_Q3F_AIMING,
	FL_Q3F_STARTING,
	FL_Q3F_MOVING,
} stat_q3f_flags_t;

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events

	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_ATTACKEE_ARMOR,			// health/armor of last person we attacked
	PERS_KILLED,					// count of the number of times you died
	// player awards tracking
	PERS_EXCELLENT_COUNT,			// two successive kills in a short amount of time
	PERS_GAUNTLET_FRAG_COUNT,		// kills with the guantlet
	PERS_CAPTURES,					// captures
	PERS_DUMMY,						// Arnout - Q3F Port: if this is not-0, the usercmd forwardmove, rightmove and upmove get set to 0
	PERS_CURRCLASS,					// This has to remain across respawns
	PERS_FLAGS,						// Extra 16 bits of flags to be used.
	PERS_DEFEND_COUNT,				// defend awards
} persEnum_t;


// entityState_t->eFlags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for sniper rifle
#define EF_Q3F_UNPREDICTABLE 0x00000200		// Golliwog: Prevent client prediction of this ent
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite

#define	EF_Q3F_DISGUISE		0x00000002		// Disguised Agent
#define	EF_Q3F_SHOWCARRY	EF_Q3F_DISGUISE	// Show while carried (on goalitems)
#define	EF_Q3F_INVISIBLE	0x00000010		// Invisible Agent ('bounce')
#define	EF_Q3F_ROTATING		0x00000020		// Rotating goalinfo/item ('bounce half')
#define EF_Q3F_DISEASED		EF_Q3F_ROTATING	// Is player diseased?
#define EF_Q3F_AIMING		0x00000040
#define	EF_Q3F_NOSPAWN		EF_NODRAW
#define EF_Q3F_SAVEME		0x00000400		// "Save me" icon above player head ('mover stop')
#define EF_Q3F_ARMORME		0x00000800		// "Armour me" icon above player head
#define	EF_Q3F_MASKME		0x00000C00		// A third call for help possible here

#define	EF_Q3F_SENTRYBORED	EF_Q3F_DISGUISE	// Sentry is 'bored' (hasn't shot for a while).

#define	EF_Q3F_FOOTSTEPS	0x00010000		// Force footsteps (for agents)
#define	EF_Q3F_METALSTEPS	0x00020000		// Force metal footsteps (for agents)
#define	EF_Q3F_STEPMASK		0x00030000		// Mask for footsteps (a third step type possible here)

#define EF_TEAMVOTED		0x00200000		// already cast a team vote
#define EF_AWARD_CAP		0x00800000		// draw the capture sprite

#define	EF_Q3F_FAILDIRECTION EF_Q3F_DISGUISE	// Forcefield direction applies to players failing the criteria.

#define EF_Q3F_REVERSECRITERIA EF_Q3F_SAVEME	// Forcefield has criteriareversed
#define EF_Q3F_DISGUISECRITERIA EF_Q3F_ARMORME	// Forcefield has disguisecriteria

// The extFlags field is an 8-bit (presently) space for extra flags.
// Ensiform: This is actually 32-bit in ETF.
#define	EXTF_BURNING		0x04			// Player is burning
#define	EXTF_TRANQED		0x08			// Player is tranqed
#define	EXTF_CONTROL		0x10			// Player is controlling something instead of running around.

#define EXTF_ANI_THROWING	0x20			// Player is in a throwing animation
#define EXTF_ANI_SPECIAL	0x40			// Player is in a special anim (one per class)
#define EXTF_ANI_OPERATING	0x80			// Player is operating with a supplystation

#define EXTF_LEGWOUNDS		0x100			// Player has been legshot


// NOTE: may not have more than 16
typedef enum {
	PW_NONE,

	PW_QUAD,
	PW_BATTLESUIT,
	PW_HASTE,
	PW_INVIS,
	PW_REGEN,
	PW_FLIGHT,
	PW_PENTAGRAM,

	PW_Q3F_CONCUSS,
	PW_Q3F_FLASH,
	PW_Q3F_GAS,

	PW_Q3F_INVULN,
	PW_Q3F_AQUALUNG,
	PW_Q3F_CEASEFIRE,

	PW_NUM_POWERUPS
} powerup_t;

typedef enum {
	HI_NONE,

	HI_TELEPORTER,
	HI_MEDKIT,

	HI_NUM_HOLDABLE
} holdable_t;

typedef enum {
	WP_INVALID=-1,
	WP_NONE=0,

	WP_AXE,
	WP_SHOTGUN,
	WP_SUPERSHOTGUN,
	WP_NAILGUN,
	WP_SUPERNAILGUN,
	WP_GRENADE_LAUNCHER,
	WP_ROCKET_LAUNCHER,
	WP_SNIPER_RIFLE,
	WP_RAILGUN,
	WP_FLAMETHROWER,
	WP_MINIGUN,
	WP_ASSAULTRIFLE,
	WP_DARTGUN,
	WP_PIPELAUNCHER,
	WP_NAPALMCANNON,

	WP_NUM_WEAPONS
} weapon_t;

typedef enum {
	AMMO_SHELLS,
	AMMO_NAILS,
	AMMO_ROCKETS,
	AMMO_CELLS,
	AMMO_MEDIKIT,
	AMMO_CHARGE,
	AMMO_GRENADES,
	AMMO_CLIP1,						// JT: 2 stats - hold all the clip values for clippable weapons.
	AMMO_CLIP2,						

	AMMO_Q3F_ENGDATA1,				// Stores assorted information on sentry/supply 
	AMMO_Q3F_ENGDATA2,

	AMMO_NONE,

} q3f_ammo_t;

// RR2DO2
#define Q3F_BEAM_STRAIGHT		0x1
#define Q3F_BEAM_NO_EFFECT		0x2
#define Q3F_BEAM_WAVE_EFFECT	0x4
#define Q3F_BEAM_WAVE_EFFECT_3D	0x8
#define Q3F_BEAM_TILESHADER		0x10
#define Q3F_BEAM_FADE			0x20
// RR2DO2

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

//Extra persistant flags (stored in ps->persistant[PERS_FLAGS])
#define PF_AUTORELOAD					0x0001
#define PF_SKIPPEDFRAME					0x0002
#define PF_JOINEDTEAM					0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
//#define	EV_EVENT_BIT1		0x00000100
//#define	EV_EVENT_BIT2		0x00000200
//#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

#define	EVENT_VALID_MSEC	300

typedef enum {
	EV_NONE,

	EV_FOOTSTEP,
	EV_FOOTSTEP_METAL,
	EV_FOOTSPLASH,
	EV_FOOTWADE,
	EV_SWIM,

	EV_STEP_4,
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,

	EV_FALL_SHORT,

	EV_FALL_D11,
	EV_FALL_D13,
	EV_FALL_D15,
	EV_FALL_D17,
	EV_FALL_D19,
	EV_FALL_D21,
	EV_FALL_D23,
	EV_FALL_D25,
	EV_FALL_D27,
	EV_FALL_D29,
	EV_FALL_D31,

	EV_JUMP_PAD,			// boing sound at origin, jump sound on player

	EV_JUMP,
	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LEAVE,	// foot leaves
	EV_WATER_UNDER,	// head touches
	EV_WATER_CLEAR,	// head leaves

	EV_ITEM_PICKUP,			// normal item pickups are predictable
	EV_GLOBAL_ITEM_PICKUP,	// powerup / team sounds are broadcast to everyone

	EV_NOAMMO,
	EV_CHANGE_WEAPON,
	EV_COCK_WEAPON,
	EV_FIRE_WEAPON,

	EV_USE_ITEM0,
	EV_USE_ITEM1,
	EV_USE_ITEM2,
	EV_USE_ITEM3,
	EV_USE_ITEM4,
	EV_USE_ITEM5,
	EV_USE_ITEM6,
	EV_USE_ITEM7,
	EV_USE_ITEM8,
	EV_USE_ITEM9,
	EV_USE_ITEM10,
	EV_USE_ITEM11,
	EV_USE_ITEM12,
	EV_USE_ITEM13,
	EV_USE_ITEM14,
	EV_USE_ITEM15,

	EV_ITEM_RESPAWN,
	EV_ITEM_POP,
	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,

	EV_GRENADE_BOUNCE,		// eventParm will be the soundindex

	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// no attenuation
	EV_GLOBAL_TEAM_SOUND,

	EV_BULLET_HIT_FLESH,
	EV_BULLET_HIT_WALL,

	EV_SNIPER_HIT_FLESH,
	EV_SNIPER_HIT_WALL,

	EV_MISSILE_HIT,
	EV_MISSILE_MISS,
	EV_MISSILE_MISS_METAL,
	EV_RAILTRAIL,
	EV_SHOTGUN,
	EV_SINGLESHOTGUN,
	EV_MINIGUN,
	EV_NAIL,				// otherEntity is the shooter

	EV_PAIN,
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,
	EV_OBITUARY,

	EV_POWERUP_QUAD,
	EV_POWERUP_BATTLESUIT,
	EV_POWERUP_REGEN,

	EV_GIB_PLAYER,			// gib a previously living player
	EV_SCOREPLUM,			// score plum

	EV_DEBUG_LINE,

	EV_STOPLOOPINGSOUND,

	EV_TAUNT,
	/*EV_TAUNT_YES,
	EV_TAUNT_NO,
	EV_TAUNT_FOLLOWME,
	EV_TAUNT_GETFLAG,
	EV_TAUNT_GUARDBASE,
	EV_TAUNT_PATROL,*/

	EV_WEAPON_AIM,
	EV_WEAPON_START,
	EV_WEAPON_END,
//	EV_SCANNER_UPDATE,
	EV_DISEASE,

	EV_SENTRY_IDLESTART,	// Golliwog: Idle start and stop sounds
	EV_SENTRY_IDLESTOP,

	EV_MUZZLEFLASH,				// Golliwog: The old classic

	EV_ALLYOBITUARY,				// Just killed a teammate.

	EV_ETF_EXPLOSION,				// Ent-driven explosion effect
	EV_ETF_GRENADE_EXPLOSION,		// Ent-driven explosion effect for grenades

	EV_ETF_DISCARD_AMMO,
	EV_ETF_USE_ITEM_FAILED,

	EV_ETF_GASEXPLOSION,

	EV_ETF_WATERSPLASH,

	EV_VISUAL_TRIGGER,
	EV_SENTRY_BUILD,

	EV_ETF_MINIGUN_START,

	EV_VISUAL_NAILFIRE,

	EV_DEBUG_DATA,

	EV_SENTRY_EXPLOSION,
	EV_ETF_SUPPLYSTATION_EXPLOSION,

	EV_GURP,
	EV_DROWN,
	EV_BURN,
	EV_BURNTODEATH,

	EV_HE_BEEP,
	EV_HE_BEEP2,
	EV_HE_EXPLODE,

	EV_RELOAD_WEAPON,

	EV_DOOR,			// slothy
	EV_LIFT,			// slothy
	EV_BOT_DEBUG_LINE,		// drevil
	EV_BOT_DEBUG_RADIUS,	// drevil
	EV_HEAL_PERSON,

	EV_SENTRY_SPINUP, 

	EV_DISCONNECT,

	EV_ETF_FLAMETHROWER_EFFECT,

	EV_SUPPLY_BUILD,
	EV_PLACE_BUILDING,
	EV_POWERUP_PENTAGRAM
} entity_event_t;

typedef struct animation_s {
	int		animNumber;			// RR2DO2: animation number (as defined in animations.h)
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
	int		reversed;			// true if animation is reversed
	int		flipflop;			// true if animation should flipflop back to
} animation_t;

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		128


// RR2DO2: not used anymore...
/*typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS
} team_t;*/

// RR2DO2: use this instead
typedef enum {
	Q3F_TEAM_FREE = 0,			// This is a dummy ID (so we know if they have no team)

	Q3F_TEAM_RED,				// Our list of teams
	Q3F_TEAM_BLUE,
	Q3F_TEAM_YELLOW,
	Q3F_TEAM_GREEN,

	Q3F_TEAM_SPECTATOR,			// The spectatores have their own team

	Q3F_TEAM_NUM_TEAMS			// A placeholder for the number of teams
} q3f_team_t;
// RR2DO2

typedef enum {
	DISGUISING_NOT,
	DISGUISING_CLASS,
	DISGUISING_TEAM
} disguising_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME		1500

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

// means of death
typedef enum {
	MOD_UNKNOWN,
	MOD_SHOTGUN,
	MOD_AXE,
	MOD_NAILGUN,
	MOD_GRENADE,
	MOD_GRENADE_SPLASH,
	MOD_PIPE,
	MOD_ROCKET,
	MOD_ROCKET_SPLASH,
	MOD_FLAME,
	MOD_FLAME_SPLASH,
	MOD_RAILGUN,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	MOD_TARGET_LASER,
	MOD_TRIGGER_HURT,
	MOD_SNIPER_RIFLE,
	MOD_SNIPER_RIFLE_HEAD,
	MOD_SNIPER_RIFLE_FEET,
	MOD_RIFLE_ASSAULT,
	MOD_DARTGUN,
	MOD_KNIFE,
	MOD_DISEASE,
	MOD_FAILED_OPERATION,
	MOD_WRENCH,
	MOD_HANDGREN,
	MOD_FLASHGREN,
	MOD_NAILGREN,
	MOD_CLUSTERGREN,
	MOD_NAPALMGREN,
	MOD_GASGREN,
	MOD_PULSEGREN,
	MOD_CHARGE,
	MOD_AUTOSENTRY_BULLET,
	MOD_AUTOSENTRY_ROCKET,
	MOD_AUTOSENTRY_EXPLODE,
	MOD_SUPPLYSTATION_EXPLODE,
	MOD_SINGLESHOTGUN,
	MOD_MINIGUN,
	MOD_CUSTOM,
	MOD_MIRROR,
	MOD_BEAM,
	MOD_MAPSENTRY,
	MOD_GASEXPLOSION,
	MOD_CRUSHEDBYSENTRY,
	MOD_MAPSENTRY_BULLET,
	MOD_MAPSENTRY_ROCKET,
	MOD_NODROP,
	MOD_SUPERNAILGUN,
	MOD_CRUSHEDBYSUPPLYSTATION,
	MOD_NEEDLE_PRICK,
	MOD_SWITCHTEAM,
	MOD_DISCONNECT,
	MOD_LASTONE
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
	IT_PERSISTANT_POWERUP,
							// EFX: rotate + bob
	IT_Q3F_BACKPACK,		// EFX: rotate + bob
	IT_Q3F_AMMOBOX,
//	IT_TEAM,

	// FALCON : START : Quake1 style armour types
	IT_GREEN_ARMOUR,
	IT_YELLOW_ARMOUR,
	IT_RED_ARMOUR
	// FALCON : END
} itemType_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];

	char		*icon;
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	gitem_t bg_extendeditemlist[];
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
gitem_t	*BG_FindItemForWeapon( weapon_t weapon );
gitem_t	*BG_FindItemForPowerup( powerup_t pw );
gitem_t	*BG_FindItemForHoldable( holdable_t pw );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps, int time );

// g_dmflags->integer flags
#define	DF_NO_FALLING			8
#define DF_FIXED_FOV			16
#define	DF_NO_FOOTSTEPS			32

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define MASK_PARTICLESOLID		(CONTENTS_SOLID|CONTENTS_PARTICLECLIP)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_FORCEFIELD)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_FORCEFIELD)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_FORCEFIELD)

//
// entityState_t->eType
//
typedef enum {
	ET_GENERAL,
	ET_ITEM,
	ET_PLAYER,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_Q3F_GRENADE,			// handgrenade with custom processing
	ET_Q3F_GOAL,			// Goalitem you can pickup
	ET_Q3F_HUD,				// HUD entity
	ET_Q3F_AGENTDATA,		// Non-rendered Agentdata, which follows the player around
	ET_Q3F_SCANNERDATA,		// Non-rendered scannerdata, which follows the player around
	ET_SNIPER_DOT,	
	ET_FLAME,				// JT - Flame
	ET_Q3F_SENTRY,			// Autosentry gun
	ET_Q3F_SUPPLYSTATION,	// Supplystation
	ET_Q3F_BEAM,			// RR2DO2: electrical beam
	ET_Q3F_MAPSENTRY,		// RR2DO2: mapsentry
	ET_Q3F_PANEL,			// Golliwog: A dynamic flat panel.
	ET_Q3F_FORCEFIELD,		// Golliwog: A team-specific forcefield.
	ET_Q3F_SENTRYCAM,		// RR2DO2: sentrycam
	ET_Q3F_TELEPORTTRANSITION,	// Golliwog: A portal entity to display transition effect during respawn.

//Remove?
	ET_Q3F_MAGICMIRROR,		// Golliwog: A magic mirror that flips players into the third dimension.
	ET_Q3F_SKYPORTAL,		// RR2DO2 sky portal

	ET_Q3F_FLAMER,

	ET_Q3F_VISIBILITY,		// Ensiform : Simple Criteria and/or range based rendered brush (should be non-solid)

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;


#define ETF_GRENDADE_EXPLOSION_NORMAL		0
#define ETF_GRENDADE_EXPLOSION_FLASH		1
#define ETF_GRENDADE_EXPLOSION_CONCUSSION	2

#define SCORE_FLAG_READY		0x0001
#define SCORE_FLAG_ADMIN		0x0002
#define SCORE_FLAG_DEV			0x0004
#define SCORE_FLAG_SPECCHASE	0x0008
#define SCORE_FLAG_SPECFOLLOW	0x0010
#define SCORE_FLAG_SPECFREE		0x0020
#define SCORE_BOT				0x0040
#define SCORE_FLAG_SHOUTCAST	0x0080
#define SCORE_FLAG_SPECFLYBY	0x0100

extern float bg_evaluategravity;
void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void	BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );

float BG_JulianDay( int year, int month, int day );
int BG_ApproxDaysSinceCompile( qtime_t time );

// RR2DO2: used in cgame
int PM_GetIdleAnim( int weaponNum, int classNum );
int PM_GetAttackAnim( int weaponNum, int classNum );
int PM_GetReloadAnim( int weaponNum, int classNum );

//	#define PERFLOG

#ifdef PERFLOG
void BG_Q3F_PerformanceMonitorInit(char* filename);
void BG_Q3F_PerformanceMonitorShutdown();
void BG_Q3F_PerformanceMonitor_LogFunction(const char* funcName);
void BG_Q3F_PerformanceMonitor_LogFunctionStop();
void BG_Q3F_PerformanceMonitor_LogFunctionUpdate();
void BG_Q3F_LogTrace();
void BG_Q3F_FlushTraceBuffer();
#endif

#define MAX_MAPINFOS		1024
#define	MAX_MAPINFOS_TEXT	8192

#define MAX_BOTS			1024
#define MAX_BOTS_TEXT		8192

#define	Q3F_GRENADE_PRIME_TIME	4000	// Time between prime and detonate on handgrenades

#define	Q3F_NUM_NAILGRENADES	10		// Allow 10 nail grenades at once
#define	Q3F_NUM_NAILGRENNAILS	200		// Ensure it's divisible by 16 (max 200)
#define	Q3F_NAILGRENADETIME		10000	// Lasts 10000 milliseconds
#define	Q3F_NAILSPEED			900		// Units per second (not millisecond)
#define	Q3F_NAILGRENINTERVAL	50		// Milliseconds between nails
#define	Q3F_NAILGRENANGLE		31		// Angle offset for each succeeding nail
#define	Q3F_NAILGREN_PACKINTERVAL 500	// Time between telling client about 'impacted' nails.

#define	Q3F_SLOT_MAX			10		// Number of HUD slots available

#define	Q3F_GOAL_SHRINKTIME		500		// Time for flags to shrink in/out

#define	Q3F_AGENT_MORPH_TIME			4000
#define	Q3F_AGENT_UNMORPH_TIME			500
#define	Q3F_AGENT_INVISIBLE_TIME		1000	// RR2DO2: was 4000
#define	Q3F_AGENT_VISIBLE_TIME			500
#define	Q3F_AGENT_VISIBLEONDEATH_TIME	700
#define	Q3F_AGENT_EXCESS_TIME			500	// Time for data entity to remain after effect
#define	Q3F_AGENT_DISGUISE				1
#define	Q3F_AGENT_DISGUISEEND			2
#define	Q3F_AGENT_DISGUISEMASK			3
#define	Q3F_AGENT_INVIS					4
#define	Q3F_AGENT_INVISEND				8
#define	Q3F_AGENT_INVISMASK				12

#define	Q3F_MESSAGE_BLANK		"<No message>"

typedef enum {
	Q3F_PANELTYPE_NAME			= 0,			// Displays the client's name.
	Q3F_PANELTYPE_SCORESUMMARY,					// Displays team scores.
	Q3F_PANELTYPE_LOCATION,						// Displays the current location.
	Q3F_PANELTYPE_TIMER,						// Displays a timer or clock.
	Q3F_PANELTYPE_RADAR,						// Displays a radar sweep.
	Q3F_PANELTYPE_MESSAGE,						// Displays an arbitrary message.

	Q3F_NUM_PANELTYPES
} bg_q3f_panelType_t;

typedef enum {
	Q3F_PANELTRANS_NONE = 0,					// No transition - on/off, and that's it.
	Q3F_PANELTRANS_FADE,						// Fade in and out.

	Q3F_NUM_PANELTRANSITIONS
} bg_q3f_panelTransitionType_t;

enum {
	Q3F_STATE_INACTIVE,
	Q3F_STATE_ACTIVE,
	Q3F_STATE_DISABLED,
	Q3F_STATE_INVISIBLE,
	Q3F_STATE_CARRIED,

	Q3F_NUM_STATES
};

typedef struct {
	const char *gameType;
	int gtEnum;
} gameTypeInfo;

typedef struct {
	char name[256];
	const char* description;
	int number;
} gameIndexInfo_t;

#define MAX_GAMEINDICIES 8

typedef struct {
	const char *mapName;
	const char *mapLoadName;
	const char *imageName;
	const char *gameIndicies;
	qhandle_t levelShot;
	gameIndexInfo_t gameIndiciesInfo[MAX_GAMEINDICIES];
	int numGameIndicies;
	int cinematic;
} mapInfo;

#define Q3F_SAY_ALL				0
#define Q3F_SAY_TEAM			1
#define Q3F_SAY_ATTACKER		2
#define Q3F_SAY_TARGET			3

#define MAX_CUSTOM_SOUNDS 32

#define MATCH_STATE_NORMAL		0
#define MATCH_STATE_PLAYING		1

#define MATCH_STATE_WAITING		5
#define MATCH_STATE_READYUP		6
#define MATCH_STATE_WARMUP		7

#define MATCH_STATE_PREPARE		10

#define MATCH_MODE_ACTIVE		1
#define MATCH_MODE_NOREADYUP	2
#define MATCH_MODE_DOCEASE		4

//Keeg might come in handy
int BG_cleanName(const char *pszIn, char *pszOut, unsigned int dwMaxLength, qboolean fCRLF);
// Keeg Crosshair support
void BG_setCrosshair(char *colString, float *col, float alpha, char *cvarName);

// Keeger support for atmospherics, tracemap
//
#ifdef API_ET
#define MAX_MAP_SIZE 65536

// Tracemap
#ifdef CGAMEDLL
void CG_GenerateTracemap( void );
#endif // CGAMEDLL

qboolean BG_LoadTraceMap( char *rawmapname, vec2_t world_mins, vec2_t world_maxs );
float BG_GetSkyHeightAtPoint( vec3_t pos );
float BG_GetSkyGroundHeightAtPoint( vec3_t pos );
float BG_GetGroundHeightAtPoint( vec3_t pos );
int BG_GetTracemapGroundFloor( void );
int BG_GetTracemapGroundCeil( void );
#endif  //API_ET

extern	char	*modNames[MOD_LASTONE];
char *_MS_FixColour( char *ptr, int colour );

#endif
