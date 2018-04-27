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

#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

//#define PRE_RELEASE_DEMO

#ifndef PRE_RELEASE_DEMO
#define Q3_VERSION      "ET 2.60"
#else
#define	Q3_VERSION		"ET 2.32"
#endif // PRE_RELEASE_DEMO
// 2.6x: Enemy Territory - ETPro team maintenance release
// 2.5x: Enemy Territory FINAL
// 2.4x: Enemey Territory RC's
// 2.3x: Enemey Territory TEST
// 2.2+: post SP removal
// 2.1+: post Enemy Territory moved standalone
// 2.x: post Enemy Territory
// 1.x: pre Enemy Territory
////
// 1.3-MP : final for release
// 1.1b - TTimo SP linux release (+ MP updates)
// 1.1b5 - Mac update merge in

#define CONFIG_NAME		"etconfig.cfg"
#define MAX_LATENT_CMDS 64

//#define LOCALIZATION_SUPPORT

#define NEW_ANIMS
#define	MAX_TEAMNAME	32

#if defined _WIN32 && !defined __GNUC__

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable	: 4152)		// nonstandard extension, function/data pointer conversion in expression
#pragma warning(disable : 4201)
#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
//#pragma warning(disable	: 4142)		// benign redefinition
#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable :	4505)		// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters

#if defined (_MSC_VER)
#pragma warning(disable : 4996)		// 'function': was declared deprecated
#endif

#endif

#include "../shared/q_math.h"
#include "../shared/q_bit.h"
#include "../shared/q_color.h"
#include "../shared/q_string.h"
#include "../shared/q_path.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h> // rain
#include <float.h>

//Ignore __attribute__ on non-gcc platforms
#if !defined(__GNUC__) && !defined(__attribute__)
	#define __attribute__(x)
#endif

#if defined(__GNUC__)
	#define UNUSED_VAR __attribute__((unused))
#else
	#define UNUSED_VAR
#endif

#if (defined _MSC_VER)
	#define Q_EXPORT __declspec(dllexport)
#elif (defined __SUNPRO_C)
	#define Q_EXPORT __global
#elif ((__GNUC__ >= 3) && (!__EMX__) && (!sun))
	#define Q_EXPORT __attribute__((visibility("default")))
#else
	#define Q_EXPORT
#endif

#if defined(__GNUC__)
#define NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#endif

// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__) && !defined __sun__  && !defined __LCC__
	#define id386	1
#else
	#define id386	0
#endif

#if (defined(powerc) || defined(powerpc) || defined(ppc) || defined(__ppc) || defined(__ppc__)) && !defined(C_ONLY)
	#define idppc	1
#else
	#define idppc	0
#endif

#include "../shared/q_platform.h"

typedef union fileBuffer_u {
	void *v;
	char *c;
	byte *b;
} fileBuffer_t;

typedef int32_t qhandle_t, thandle_t, fxHandle_t, sfxHandle_t, fileHandle_t, clipHandle_t;

#define NULL_HANDLE ((qhandle_t)0)
#define NULL_SOUND ((sfxHandle_t)0)
#define NULL_SFX ((sfxHandle_t)0)
#define NULL_FILE ((fileHandle_t)0)
#define NULL_CLIP ((clipHandle_t)0)

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))

#ifdef __GNUC__
#define QALIGN(x) __attribute__((aligned(x)))
#else
#define QALIGN(x)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define INT_ID( a, b, c, d ) (uint32_t)((((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((c) & 0xff) << 8) | ((d) & 0xff))

//#define	SND_NORMAL			0x000	// (default) Allow sound to be cut off only by the same sound on this channel
#define		SND_OKTOCUT			0x001	// Allow sound to be cut off by any following sounds on this channel
#define		SND_REQUESTCUT		0x002	// Allow sound to be cut off by following sounds on this channel only for sounds who request cutoff
#define		SND_CUTOFF			0x004	// Cut off sounds on this channel that are marked 'SND_REQUESTCUT'
#define		SND_CUTOFF_ALL		0x008	// Cut off all sounds on this channel
#define		SND_NOCUT			0x010	// Don't cut off.  Always let finish (overridden by SND_CUTOFF_ALL)
#define		SND_NO_ATTENUATION	0x020	// don't attenuate (even though the sound is in voice channel, for example)

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)			(ARRAY_LEN(x) - 1)

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// RF, this is just here so different elements of the engine can be aware of this setting as it changes
#define	MAX_SP_CLIENTS		64		// increasing this will increase memory usage significantly

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	256		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define BIG_INFO_STRING		8192	// used for system info key only
#define BIG_INFO_KEY		8192
#define BIG_INFO_VALUE		8192

#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			256		// max length of a filesystem pathname

// rain - increased to 36 to match MAX_NETNAME, fixes #13 - UI stuff breaks
// with very long names
#define MAX_NAME_LENGTH     36		// max length of a client name

#define MAX_SAY_TEXT		150

#define	MAX_OBITS			4
#define SHOW_OBIT			5000		// 5 seconds

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;

#ifdef	ERR_FATAL
#undef	ERR_FATAL				// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_VID_FATAL,				// exit the entire game with a popup window and doesn't delete profile.pid
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD,				// pop up the need-cd dialog
	ERR_AUTOUPDATE
} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			3
#define PROP_SPACE_WIDTH		8
#define PROP_HEIGHT				27
#define PROP_SMALL_SIZE_SCALE	0.75

#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			75

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000
// JOSEPH 10-24-99
#define UI_MENULEFT		0x00008000
#define UI_MENURIGHT	0x00010000
#define UI_EXSMALLFONT	0x00020000
#define UI_MENUFULL		0x00080000
// END JOSEPH

#define UI_SMALLFONT75	0x00100000

#if defined(_DEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#ifdef __linux__
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=371
// custom Snd_Memset implementation for glibc memset bug workaround
void Snd_Memset (void* dest, const int val, const size_t count);
#else
#define Snd_Memset Com_Memset
#endif

void Com_Memset (void* dest, const int val, const size_t count);
void Com_Memcpy (void* dest, const void* src, const size_t count);

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT)

#define MINICHAR_WIDTH		8
#define MINICHAR_HEIGHT		12

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

// ----------------------------------------------------

#define GAME_INIT_FRAMES	6
#define	FRAMETIME			100					// msec


void	COM_BeginParseSession( const char *name );
void	COM_RestoreParseSession( char **data_p );
void	COM_SetCurrentParseLine( int line );
int		COM_GetCurrentParseLine( void );
char	*COM_Parse( char **data_p );
char	*COM_ParseExt( char **data_p, qboolean allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( char *format, ... ) __attribute__( ( format( printf,1,2 ) ) );
void	COM_ParseWarning( char *format, ... ) __attribute__( ( format( printf,1,2 ) ) );
int		Com_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
	int line;
	int linescrossed;
} pc_token_t;

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( char**buf_p, char *match );

void SkipBracedSection (char **program);
void SkipBracedSection_Depth (char **program, int depth); // start at given depth if already 
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);

void	QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__( ( format( printf,3,4 ) ) );


// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

char	* QDECL va(char *format, ...) __attribute__( ( format( printf,1,2 ) ) );
//float	*tv( float x, float y, float z );

//=============================================

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link
void	QDECL Com_Error( int level, const char *error, ... ) __attribute__( ( format( printf,2,3 ) ) );
void	QDECL Com_Printf( const char *msg, ... ) __attribute__( ( format( printf,1,2 ) ) );

/*
==========================================================

  RELOAD STATES

==========================================================
*/

#define	RELOAD_SAVEGAME			0x01
#define	RELOAD_NEXTMAP			0x02
#define	RELOAD_NEXTMAP_WAITING	0x04
#define	RELOAD_FAILED			0x08
#define	RELOAD_ENDGAME			0x10


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE		1	// set to cause it to be saved to vars.rc
								// used for system variables, not for player
								// specific configurations
#define	CVAR_USERINFO		2	// sent to server on connect or change
#define	CVAR_SERVERINFO		4	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		8	// these cvars will be duplicated on all clients
#define	CVAR_INIT			16	// don't allow change from console at all,
								// but can be set from the command line
#define	CVAR_LATCH			32	// will only change when C code next does
								// a Cvar_Get(), so it can't be changed
								// without proper initialization.  modified
								// will be set, even though the value hasn't
								// changed yet
#define	CVAR_ROM			64	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	128	// created by a set command
#define	CVAR_TEMP			256	// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			512	// can not be changed if cheats are disabled
#define CVAR_NORESTART		1024	// do not clear when a cvar_restart is issued
#define CVAR_WOLFINFO		2048	// DHM - NERVE :: Like userinfo, but for wolf multiplayer info

#define CVAR_UNSAFE			4096	// ydnar: unsafe system cvars (renderer, sound settings, anything that might cause a crash)
#define	CVAR_SERVERINFO_NOUPDATE		8192	// gordon: WONT automatically send this to clients, but server browsers will see it

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// a trace is returned when a box is swept through the world
typedef struct {
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by CM_MarkFragments()
typedef struct {
	int		firstPoint;
	int		numPoints;
} markFragment_t;



typedef struct {
	vec3_t		origin;
	vec3_t		axis[3];
} orientation_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI				0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER,		// announcer voices, etc
	CHAN_VOICE_BG,	// xkan - background sound for voice (radio static, etc.)
} soundChannel_t;


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/
#define	ANIM_BITS		10

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected


#define	SNAPFLAG_NOUSERCMD		( 1 << 24)

//
// per-level limits
//
#define	MAX_CLIENTS			64 // JPW NERVE back to q3ta default was 128		// absolute limit

#define	GENTITYNUM_BITS		10	// JPW NERVE put q3ta default back for testing	// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)


#define	MAX_MODELS			256		// these are sent over the net as 8 bits (Gordon: upped to 9 bits, erm actually it was already at 9 bits, wtf? NEVAR TRUST GAMECODE COMMENTS, comments are evil :E, lets hope it doesnt horribly break anything....)
#define	MAX_SOUNDS			256		// so they cannot be blindly increased
#define MAX_CS_SKINS		64
#define MAX_CSSTRINGS		32

// Arnout: Q3F Port - changed cs_shaders to shaders and gave it the q3 value for now.
// Would be nice to move over to the ET buffered remap shader code
//#define MAX_CS_SHADERS		32
#define	MAX_SHADERS			256
#define	MAX_SPIRITSCRIPTS	64

#define MAX_SERVER_TAGS		256
#define MAX_TAG_FILES		64

#define MAX_MULTI_SPAWNTARGETS	16 // JPW NERVE

#define	MAX_CONFIGSTRINGS	1024

#define MAX_DLIGHT_CONFIGSTRINGS	16
#define MAX_SPLINE_CONFIGSTRINGS	8

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

// xkan, 1/10/2003 - adapted from original SP
typedef enum
{
	AISTATE_RELAXED,
	AISTATE_QUERY,
	AISTATE_ALERT,
	AISTATE_COMBAT,

	MAX_AISTATES
} aistateEnum_t;

#define	REF_FORCE_DLIGHT	(1<<31)	// RF, passed in through overdraw parameter, force this dlight under all conditions
#define	REF_JUNIOR_DLIGHT	(1<<30)	// (SA) this dlight does not light surfaces.  it only affects dynamic light grid
#define REF_DIRECTED_DLIGHT	(1<<29)	// ydnar: global directional light, origin should be interpreted as a normal vector

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				64	// (SA) and yet more!

// Ridah, increased this
//#define	MAX_PS_EVENTS			2
// ACK: I'd really like to make this 4, but that seems to cause network problems
#define	MAX_EVENTS				4	// max events per frame before we drop events
//#define	MAX_PS_EVENTS				2	// max events per frame before we drop events


#define PS_PMOVEFRAMECOUNTBITS	6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c 
// (Gordon: unless it doesnt need transmitted over the network, in which case it should prolly go in the new pmext struct anyway)

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
//
// NOTE: all fields in here must be 32 bits (or those within sub-structures)
typedef struct playerState_s {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;
	int			weaponTime;
	int			weaponDelay;	// for weapons that don't fire immediately when 'fire' is hit (grenades, venom, ...)
//	int			grenadeTimeLeft;	// for delayed grenade throwing.  this is set to a #define for grenade
									// lifetime when the attack button goes down, then when attack is released
									// this is the amount of time left before the grenade goes off (or if it
									// gets to 0 while in players hand, it explodes)
	int			generic1;

	int			gravity;
	float		leanf;			// amount of 'lean' when player is looking around corner //----(SA)	added
	
	int			speed;
	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum;// ENTITYNUM_NONE = in air

	int			legsTimer;		// don't change low priority animations until this runs out
	int			legsAnim;		// mask off ANIM_TOGGLEBIT

	int			torsoTimer;		// don't change low priority animations until this runs out
	int			torsoAnim;		// mask off ANIM_TOGGLEBIT

	int			movementDir;	// a number 0 to 7 that represents the reletive angle
								// of movement to the view angle (axial and diagonals)
								// when at rest, the value will remain unchanged
								// used to twist the legs during strafing



	int			eFlags;			// copied to entityState_t->eFlags

	int			eventSequence;	// pmove generated events
	int			events[MAX_EVENTS];
	int			eventParms[MAX_EVENTS];
	int			oldEventSequence;	// so we can see which events have been added since we last converted to entityState_t

	int			_externalEvent;	// events set on player from another source
	int			_externalEventParm;
	int			_externalEventTime;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1

	// weapon info
	int			weapon;			// copied to entityState_t->weapon
	int			weaponstate;

	// item info
//	int			item;
// Arnout - Q3F Port: replaced item with the value as used in Q3(F2)
// Q3: jumppad_ent, 10 bits, ET, item: 10 bits
	int			jumppad_ent;

	vec3_t		viewangles;		// for fixed views
	int			viewheight;

	// damage feedback
	int			damageEvent;	// when it changes, latch the other parms
	int			damageYaw;
	int			damagePitch;
	int			damageCount;

	int			stats[MAX_STATS];
	int			persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
	int			powerups[MAX_POWERUPS];	// level.time that the powerup runs out
	int			ammo[MAX_WEAPONS];		// total amount of ammo
	int			ammoclip[MAX_WEAPONS];	// ammo in clip
	int			holdable[16];
	int			holding;				// the current item in holdable[] that is selected (held)
	int			weapons[MAX_WEAPONS/(sizeof(int)*8)];	// 64 bits for weapons held

	// Ridah, allow for individual bounding boxes
	vec3_t		mins, maxs;
	float		crouchMaxZ;
	//float		crouchViewHeight, standViewHeight, deadViewHeight;
// Arnout - Q3F Port: replaced these 3 with teh value as used in Q3(F2)
// Q3: grapplePoint, 3 floats, 0 bits; ET: crouchViewHeight, standViewHeight, deadViewHeight, 3 floats, 0 bits
	vec3_t		grapplePoint;	// location of grapple to pull towards if PMF_GRAPPLE_PULL

	// variable movement speed
	float		runSpeedScale, sprintSpeedScale, crouchSpeedScale;
	// done.

	// Ridah, view locking for mg42
	int			viewlocked;
//	int			viewlocked_entNum;
// Arnout - Q3F Port: replaced viewlocked_entNum with the value as used in Q3(F2)
// Q3: loopSound, 16 bits, ET, viewlocked_entNum: 16 bits
	int			loopSound;

	float		friction;

	int			nextWeapon;
	int			teamNum;				// Arnout: doesn't seem to be communicated over the net

	// Rafael 
	//int			gunfx;

	// RF, burning effect is required for view blending effect
	//int			onFireStart;
// Arnout - Q3F Port: replaced onFireStart with the values as used in Q3(F2)
// Q3: generic1, 8 bits, ET, onFireStart: 32 bits
// (so now all generic1 bits can actually be used)
	int			extFlags;
//	char		generic1[3];

	int			serverCursorHint;		// what type of cursor hint the server is dictating
	int			serverCursorHintVal;	// a value (0-255) associated with the above

	trace_t		serverCursorHintTrace;	// not communicated over net, but used to store the current server-side cursorhint trace

	// ----------------------------------------------------------------------
	// So to use persistent variables here, which don't need to come from the server,
	// we could use a marker variable, and use that to store everything after it
	// before we read in the new values for the predictedPlayerState, then restore them
	// after copying the structure recieved from the server.

	// Arnout: use the pmoveExt_t structure in bg_public.h to store this kind of data now (presistant on client, not network transmitted)

	int			ping;			// server to game info for scoreboard
	int			pmove_framecount;
	int			_entityEventSequence;

//	int			sprintExertTime;
	int			jumppad_frame;		// Arnout - Q3F Port: From Q3

	// JPW NERVE -- value for all multiplayer classes with regenerating "class weapons" -- ie LT artillery, medic medpack, engineer build points, etc
	int			classWeaponTime;		// Arnout : DOES get send over the network
	int			jumpTime;			// used in MP to prevent jump accel
	// jpw

	int			weapAnim;			// mask off ANIM_TOGGLEBIT										//----(SA)	added		// Arnout : DOES get send over the network

	qboolean	releasedFire;

	float		aimSpreadScaleFloat;	// (SA) the server-side aimspreadscale that lets it track finer changes but still only
										// transmit the 8bit int to the client
	int			aimSpreadScale;			// 0 - 255 increases with angular movement		// Arnout : DOES get send over the network
	int			lastFireTime;			// used by server to hold last firing frame briefly when randomly releasing trigger (AI)

	int			quickGrenTime;

	int			leanStopDebounceTime;

//----(SA)	added

	// seems like heat and aimspread could be tied together somehow, however, they (appear to) change at different rates and
	// I can't currently see how to optimize this to one server->client transmission "weapstatus" value.
	int			weapHeat[MAX_WEAPONS];	// some weapons can overheat.  this tracks (server-side) how hot each weapon currently is.
	int			curWeapHeat;			// value for the currently selected weapon (for transmission to client)		// Arnout : DOES get send over the network
	int			identifyClient;			// NERVE - SMF
	int			identifyClientHealth;

	aistateEnum_t	aiState;		// xkan, 1/10/2003	
} playerState_t;


//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		1
#define	BUTTON_TALK			2			// displays talk balloon and disables actions
#define	BUTTON_USE_HOLDABLE	4			// Arnout - Q3F Port: reenabled
//#define	BUTTON_GESTURE		8
#define	BUTTON_WALKING		16			// walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps
//----(SA)	added
#define BUTTON_SPRINT		32
#define BUTTON_ACTIVATE		64
//----(SA)	end

#define	BUTTON_ANY			128			// any key whatsoever


//----(SA) wolf buttons
#define	WBUTTON_ATTACK2		1
#define	WBUTTON_ZOOM		2
#define	WBUTTON_RELOAD		8
#define	WBUTTON_LEANLEFT	16
#define	WBUTTON_LEANRIGHT	32
#define	WBUTTON_DROP		64 // JPW NERVE
#define WBUTTON_PRONE		128 // Arnout: wbutton now*/
//----(SA) end

#define	MOVE_RUN			120			// if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

#define GESTURE_LOOK		0x01
#define GESTURE_BECON		0x02
#define GESTURE_STOP		0x03
#define GESTURE_POINT		0x04
#define GESTURE_WAVENO		0x05
#define	GESTURE_WAVEYES		0x06
#define	GESTURE_TAUNT		0x07

#define UCMDF_GESTURE_MASK	0x07

#define UCMDF_SPECIAL		0x10
#define UCMDF_SPECIAL2		0x20

// Arnout: doubleTap buttons - DT_NUM can be max 8
typedef enum {
	DT_NONE,
	DT_MOVELEFT,
	DT_MOVERIGHT,
	DT_FORWARD,
	DT_BACK,
	DT_LEANLEFT,
	DT_LEANRIGHT,
	DT_UP,
	DT_NUM
} dtType_t;

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
	int		serverTime;
	byte	buttons;
	byte	wbuttons;
	byte	weapon;
	byte	flags;
	int		angles[3];

	signed char	forwardmove, rightmove, upmove;
	byte	doubleTap;			// Arnout: only 3 bits used

	char	identClient;		// NERVE - SMF
} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	/*
	TR_LINEAR_STOP_BACK,		//----(SA)	added.  so reverse movement can be different than forward
	*/
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	/*
	// Ridah
	TR_GRAVITY_LOW,
	TR_GRAVITY_FLOAT,			// super low grav with no gravity acceleration (floating feathers/fabric/leaves/...)
	TR_GRAVITY_PAUSED,			//----(SA)	has stopped, but will still do a short trace to see if it should be switched back to TR_GRAVITY
	TR_ACCELERATE,
	TR_DECCELERATE,
	// Gordon
	TR_SPLINE,
	TR_LINEAR_PATH,
	*/
// Arnout - Q3F Port
	TR_QUAD_SPLINE_PATH,
	TR_CUBIC_SPLINE_PATH,
	TR_ROTATING					// Ensiform: To allow stopping func_rotating
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
//----(SA)	removed
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
//----(SA)	removed
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large
//
// NOTE: all fields in here must be 32 bits (or those within sub-structures)

/*
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_CONCUSSIVE_TRIGGER,	// JPW NERVE trigger for concussive dust particles
	ET_OID_TRIGGER,			// DHM - Nerve :: Objective Info Display
	ET_EXPLOSIVE_INDICATOR,	// NERVE - SMF

	//---- (SA) Wolf
	ET_EXPLOSIVE,			// brush that will break into smaller bits when damaged
	ET_EF_SPOTLIGHT,
	ET_ALARMBOX,
	ET_CORONA,
	ET_TRAP,

	ET_GAMEMODEL,			// misc_gamemodel.  similar to misc_model, but it's a dynamic model so we have LOD
	ET_FOOTLOCKER,	//----(SA)	added
	//---- end

	ET_FLAMEBARREL,
	ET_FP_PARTS,

	// FIRE PROPS
	ET_FIRE_COLUMN,
	ET_FIRE_COLUMN_SMOKE,
	ET_RAMJET,	

	ET_FLAMETHROWER_CHUNK,		// DHM - NERVE :: Used in server side collision detection for flamethrower

	ET_EXPLO_PART,

	ET_PROP,

	ET_AI_EFFECT,

	ET_CAMERA,
	ET_MOVERSCALED,

	ET_CONSTRUCTIBLE_INDICATOR,
	ET_CONSTRUCTIBLE,
	ET_CONSTRUCTIBLE_MARKER,
	ET_BOMB,
	ET_WAYPOINT,
	ET_BEAM_2,
	ET_TANK_INDICATOR,
	ET_TANK_INDICATOR_DEAD,
	// Start - TAT - 8/29/2002
	// An indicator object created by the bot code to show where the bots are moving to
	ET_BOTGOAL_INDICATOR,
	// End - TA - 8/29/2002
	ET_CORPSE,				// Arnout: dead player
	ET_SMOKER,				// Arnout: target_smoke entity

	ET_TEMPHEAD,			// Gordon: temporary head for clients for bullet traces
	ET_MG42_BARREL,			// Arnout: MG42 barrel
	ET_TEMPLEGS,			// Arnout: temporary leg for clients for bullet traces
	ET_TRIGGER_MULTIPLE,
	ET_TRIGGER_FLAGONLY,
	ET_TRIGGER_FLAGONLY_MULTIPLE,
	ET_GAMEMANAGER,
	ET_AAGUN,
	ET_CABINET_H,
	ET_CABINET_A,
	ET_HEALER,
	ET_SUPPLIER,

	ET_LANDMINE_HINT,		// Gordon: landmine hint for botsetgoalstate filter
	ET_ATTRACTOR_HINT,		// Gordon: attractor hint for botsetgoalstate filter
	ET_SNIPER_HINT,			// Gordon: sniper hint for botsetgoalstate filter
	ET_LANDMINESPOT_HINT,	// Gordon: landminespot hint for botsetgoalstate filter

	ET_COMMANDMAP_MARKER,

	ET_WOLF_OBJECTIVE,

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;
*/

typedef struct entityState_s {
	int				number;			// entity index
//	entityType_t	eType;			// entityType_t
	int				eType;			// entityType_t
	int				eFlags;

	trajectory_t	pos;	// for calculating position
	trajectory_t	apos;	// for calculating angles

	int		time;
	int		time2;

	vec3_t	origin;
	vec3_t	origin2;

	vec3_t	angles;
	vec3_t	angles2;

	int		otherEntityNum;	// shotgun sources, etc
	int		otherEntityNum2;

	int		groundEntityNum;	// -1 = in air

	int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
	int		dl_intensity;	// used for coronas
	int		loopSound;		// constantly loop this sound

	int		modelindex;
	int		modelindex2;
	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		frame;

	int		solid;			// for client side prediction, trap_linkentity sets this properly

	// old style events, in for compatibility only
	int		event;
	int		eventParm;

	int		eventSequence;	// pmove generated events
	int		events[MAX_EVENTS];
	int		eventParms[MAX_EVENTS];

	// for players
	int		powerups;		// bit flags	// Arnout: used to store entState_t for non-player entities (so we know to draw them translucent clientsided)
	int		weapon;			// determines weapon and flash model, etc
	int		legsAnim;		// mask off ANIM_TOGGLEBIT
	int		torsoAnim;		// mask off ANIM_TOGGLEBIT
//	int		weapAnim;		// mask off ANIM_TOGGLEBIT	//----(SA)	removed (weap anims will be client-side only)

	int		density;		// for particle effects

	int		dmgFlags;		// to pass along additional information for damage effects for players/ Also used for cursorhints for non-player entities

	// Ridah
	//Canabis, replace onfirststart/end for some q3 variables
//	int		onFireStart;
//	int		onFireEnd;
	int		extFlags;
	int		generic1;


	int		nextWeapon;
	int		teamNum;

	int		effect1Time, effect2Time, effect3Time;

	aistateEnum_t	aiState;	// xkan, 1/10/2003
	int		animMovetype;	// clients can't derive movetype of other clients for anim scripting system
} entityState_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key 
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

// font support 

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPH_CHARSTART2		160	// Arnout: moved over from Q3's q_shared.h
#define GLYPH_CHAREND2			255	// Arnout: moved over from Q3's q_shared.h
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
typedef struct {
	int height;       // number of scan lines
	int top;          // top of glyph in buffer
	int bottom;       // bottom of glyph in buffer
	int pitch;        // width for copying
	int xSkip;        // x adjustment
	int imageWidth;   // width of actual image
	int imageHeight;  // height of actual image
	float s;          // x offset in image where glyph starts
	float t;          // y offset in image where glyph starts
	float s2;
	float t2;
	qhandle_t glyph;  // handle to the shader with the glyph
	char shaderName[32];
} glyphInfo_t;

typedef struct {
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
	float glyphScale;
	char name[MAX_QPATH];
} fontInfo_t;

#define Square(x) ((x)*(x))

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources
#define AS_LOCAL		0
#define AS_GLOBAL		1			// NERVE - SMF - modified
#define AS_FAVORITES	2

// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

typedef enum _flag_status {
	FLAG_ATBASE = 0,
	FLAG_TAKEN,			// CTF
	FLAG_TAKEN_RED,		// One Flag CTF
	FLAG_TAKEN_BLUE,	// One Flag CTF
	FLAG_DROPPED
} flagStatus_t;



#define	MAX_GLOBAL_SERVERS			2048
#define	MAX_OTHER_SERVERS			128
#define MAX_PINGREQUESTS			16
#define MAX_SERVERSTATUSREQUESTS	16

#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2

// NERVE - SMF - localization
typedef enum {
#ifndef __MACOS__	//DAJ USA
	LANGUAGE_FRENCH = 0,
	LANGUAGE_GERMAN,
	LANGUAGE_ITALIAN,
	LANGUAGE_SPANISH,
#endif
	MAX_LANGUAGES
} languages_t;

// NERVE - SMF - wolf server/game states
typedef enum {
	GS_INITIALIZE = -1,
	GS_PLAYING,
	GS_WARMUP_COUNTDOWN,
	GS_WARMUP,
	GS_INTERMISSION,
	GS_WAITING_FOR_PLAYERS,
	GS_RESET
} gamestate_t;

#define SQR( a ) ((a)*(a))

typedef int( *cmpFunc_t )(const void *a, const void *b);

void *Q_LinearSearch( const void *key, const void *ptr, size_t count,
	size_t size, cmpFunc_t cmp );

#endif	// __Q_SHARED_H
