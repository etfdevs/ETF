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
**	g_q3f_mapents.h
**
**	Structures and definitions for the extended map entity stuff.
**
*/

#ifndef	__G_Q3F_MAPENTS_H
#define	__G_Q3F_MAPENTS_H

#include "g_local.h"
#include "q_shared.h"
#include "g_q3f_mapdata.h"

#define	Q3F_NOBUILD_AUTOSENTRY		0x01
#define	Q3F_NOBUILD_SUPPLYSTATION	0x02
#define	Q3F_NOBUILD_CHARGE			0x04

#define	Q3F_NOANNOY_BUILDING		0x01
#define	Q3F_NOANNOY_GRENS			0x02
#define Q3F_NOANNOY_GASGREN			0x04
#define Q3F_NOANNOY_NAPALMGREN		0x08
#define Q3F_NOANNOY_NAILGREN		0x10
#define	Q3F_NOANNOY_PROJECTILES		0x20
#define Q3F_NOANNOY_CHARGES			0x40
#define Q3F_NOANNOY_BACKPACKS		0x80

#define	Q3F_VFLAG_GIVE_ADD		0x01	// Add value to existing value
#define Q3F_VFLAG_CLSTAT_EQ		0x02	// Value matches client value
#define Q3F_VFLAG_CLSTAT_LT		0x04	// Value is lower than client value
#define Q3F_VFLAG_CLSTAT_GT		0x08	// Value is higher than client value
#define Q3F_VFLAG_CLSTAT_CM		0x40	// Take classmaximum as value
#define	Q3F_VFLAG_FORCE			0x80	// Override maximum value

#define	Q3F_FLAG_CARRYABLE			0x00000001	// Can ent be carried?
#define Q3F_FLAG_HIDEACTIVE			0x00000002	// Does it 'vanish' while active?
#define	Q3F_FLAG_AFFECTTEAM			0x00000004	// All teammembers are affected
#define	Q3F_FLAG_AFFECTNONTEAM		0x00000008	// All non-teammembers are affected
#define	Q3F_FLAG_EFFECTDROPOFF		0x00000010	// Effect drops off with distance
#define	Q3F_FLAG_LINEOFSIGHT		0x00000020	// Only affects players visible
#define	Q3F_FLAG_ENVIRONMENT		0x00000040	// Only affects players in same environment
#define	Q3F_FLAG_SHOOTABLE			0x00000080	// Allow door/plat to be shot
#define	Q3F_FLAG_REVERSECRITERIA	0x00000100	// Reverse criteria
#define	Q3F_FLAG_REVEALAGENT		0x00000200	// Reveal an agent when carried/active
#define Q3F_FLAG_SHOWCARRY			0x00000400	// Show item while carried
#define	Q3F_FLAG_CHARGEABLE			0x00000800	// Allow charges to trigger this item
#define	Q3F_FLAG_ROTATING			0x00001000	// Model rotates/bobs like a normal item.
#define	Q3F_FLAG_FLAGINFO			0x00002000	// (Probably) has flaginfo keys
#define	Q3F_FLAG_RETOUCH			0x00004000	// Can be retouched without triggering (i.e. doors)
#define	Q3F_FLAG_SPEEDSCALE			0x00008000	// Goalitem alters player speed when carried
#define	Q3F_FLAG_NOSHRINK			0x00010000	// Goal does not grow or shrink when appearing/disappearing
#define	Q3F_FLAG_NODROP				0x00020000	// Flag cannot be 'dropped' manually
#define	Q3F_FLAG_KILLMSG			0x00040000	// (Probably) has kill_<team>_message keys
#define	Q3F_FLAG_ALLOWDEAD			0x00080000	// Allow dead players to be selected in target_cycle use
#define	Q3F_FLAG_ALLOWSAME			0x00100000	// Allow same player to be selected in target_cycle use
#define	Q3F_FLAG_KEEPONDEATH		0x00200000	// Don't drop goalitem when client dies.
#define	Q3F_FLAG_USEGAUNTLET		0x00400000	// Entity must be gauntleted instead of touched
#define Q3F_FLAG_SAMECLASS			0x00800000	// Allow players of the same class to trigger
#define Q3F_FLAG_SAMETEAM			0x01000000	// Allow players of the same team to trigger
#define	Q3F_FLAG_FAILDIRECTION		0x02000000	// Forcefield direction field applies to those _failing_ the criteria
#define	Q3F_FLAG_ALLOWSENTRYLOCK	0x04000000	// Sentry is allowed to lock on through forcefields (bad idea :)
#define	Q3F_FLAG_DISGUISECRITERIA	0x08000000	// Criteria work on agent's apparent team/class rather than real.
//#define	Q3F_FLAG_FLASHPROTECT		0x08000000	// Forcefields protect against flash grenades too
//#define Q3F_FLAG_THROWABLE			0x10000000	// Ent is throwable by using the 'useflag' command
#define Q3F_FLAG_ORCLIENTSTATS		0x10000000	// The clientstats are usually && for all bits, setting this flag makes them ||
#define Q3F_FLAG_RESETABLE			0x20000000	// This ent is resetable by a target_reset
//#define Q3F_FLAG_KEEPONTEAMCNG		0x40000000	// Don't drop goalitem when client changes teams.
#define Q3F_FLAG_DETPIPES			0x40000000	// Grenadier players passing through this forcefield will have their pipes det'd

#define	Q3F_BHT_AUTOSENTRY		0x01
#define	Q3F_BHT_SUPPLYSTATION	0x02
#define	Q3F_BHT_PROJECTILES		0x04
#define	Q3F_BHT_CHARGE			0x08
#define	Q3F_BHT_BACKPACKS		0x10
#define	Q3F_BHT_GRENADES		0x20

enum {
	Q3F_BROADCAST_TEXT,				// Different types of broadcast
	Q3F_BROADCAST_SOUND,
	Q3F_BROADCAST_DICT,
};

typedef struct q3f_mapent_s {
	int state, statechangetime, statechangecount;
	int waittime, origetype;
	int team, ownerteam, flags, classes, gameindex;

	gentity_t *lastTriggerer;

	q3f_array_t *groupname, *holding, *notholding;
	q3f_keypairarray_t *activetarget, *inactivetarget, *carrytarget;
	q3f_keypairarray_t *disabletarget, *invisibletarget, *failtarget;

	q3f_keypairarray_t *give, *checkstate, *clientstats;

	q3f_keypairarray_t *other;

	int (*statethink)( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace );

	q3f_keypairarray_t	*spawVars;	// RR2DO2: spawnvars of the entity, set if resetable
} q3f_mapent_t;

extern char *q3f_statestrings[Q3F_NUM_STATES];
extern const char* team_suffixes[5];
extern const int team_for_suffix[5];

	// String parsing functions for map ents
q3f_keypairarray_t *G_Q3F_ProcessStateString( const char *value );
int G_Q3F_ProcessInitialStateString( const char *value );
q3f_array_t *G_Q3F_ProcessStrings( const char *value );
int G_Q3F_ProcessFlagString( const char *value );
int G_Q3F_ProcessTeamString( const char *value );
int G_Q3F_ProcessBlackHoleTypeString( const char *value );
int G_Q3F_ProcessWeaponString( const char *value );
int G_Q3F_ProcessClassString( const char *value );
int G_Q3F_ProcessGameIndexString( const char *value );
q3f_keypairarray_t *G_Q3F_ProcessGiveString( const char *value );
q3f_keypairarray_t *G_Q3F_ProcessClientStatsString( const char *value );


	// Process a map field during init.
void G_Q3F_ProcessMapField( const char *key, const char *value, gentity_t *ent );
	// Remove data at entity destruction
void G_Q3F_FreeMapData( gentity_t *ent );
	// Copy all extended map data from src to dest
void G_Q3F_CopyMapData( gentity_t *src, gentity_t *dest );

	// Process entity 'touches'
qboolean G_Q3F_TouchEntity( gentity_t *ent, gentity_t *other, trace_t *trace );
	// Process entity 'uses'
qboolean G_Q3F_UseEntity( gentity_t *ent, gentity_t *other, gentity_t *attacker );

	// Process entity 'triggers'
qboolean G_Q3F_TriggerEntity( gentity_t *ent, gentity_t *activator, int state, trace_t *trace, int force );
void G_Q3F_PropogateTrigger( q3f_keypairarray_t *propogator, gentity_t *activator, trace_t *trace );

	// Process a message string for %% commands.
char *G_Q3F_MessageString( char *srcptr, gentity_t *activator, gentity_t *queryent, int colour );

	// Process 'give' commands for an entity.
void G_Q3F_MapGive( gentity_t *activator, gentity_t *entity );

	// Print off a message when an entity is triggered.
void G_Q3F_StateMessage( gentity_t *ent, gentity_t *activator );
	// Print off a message when a player is killed
void G_Q3F_KillMessage( gentity_t *victim, gentity_t *inflictor, gentity_t *attacker );
	// General message broadcast
//void G_Q3F_StateBroadcast( gentity_t *ent, gentity_t *activator, gentity_t *queryent, char *messagesuffix, char *strarray[5][4], int type, char *prefix );
void G_Q3F_StateBroadcast( gentity_t *ent, gentity_t *activator, gentity_t *queryent, char *messagesuffix, char *strarray[5][8], int type, char *prefix );
//void G_Q3F_StateBroadcast_TeamedNoActivator( gentity_t *ent, char *messagesuffix, int type, char *prefix );
void G_Q3F_EntityMessage(const char * format, ... );


	// Check that all the named groups have at least on entity in the specified state
qboolean G_Q3F_CheckStates( q3f_keypairarray_t *array );
	// Check that the activator client has all the needed stats
qboolean G_Q3F_CheckClientStats( gentity_t *activator, q3f_keypairarray_t *array, qboolean or );
	// Try a criteria check.
qboolean G_Q3F_CheckCriteria( gentity_t *activator, gentity_t *ent );

	// Run the goalinfo/item
void G_Q3F_RunGoal( gentity_t *ent );

	// Spawn a func_goalinfo
void SP_Q3F_func_goalinfo( gentity_t *ent );
	// Spawn a func_commandpoint
void SP_Q3F_func_commandpoint( gentity_t *ent );
	// Spawn a func_hud
void SP_Q3F_func_hud( gentity_t *ent );
#if 0
	// Spawn a func_objectiveicon
void SP_Q3F_func_objectiveicon( gentity_t *ent );
#endif
	// Spawn a target_command
void SP_Q3F_target_command( gentity_t *ent );
	// Spawn a target_reset
void SP_Q3F_target_reset( gentity_t *ent );

	// Update HUD icons on change
void G_Q3F_UpdateHUDIcons();
void G_Q3F_UpdateHUD( gentity_t *ent );

	// No-build functions
void SP_Q3F_func_nobuild( gentity_t *ent );
void G_Q3F_NoBuildFinish();
qboolean G_Q3F_NoBuildCheck( vec3_t mins, vec3_t maxs, int team, int mask );

	// No-annoyances functions
void SP_Q3F_func_noannoyances( gentity_t *ent );
void G_Q3F_NoAnnoyFinish();
extern qboolean levelhasnoannoys;
qboolean G_Q3F_MapHasNoAnnoys();
qboolean G_Q3F_NoAnnoyCheck( vec3_t mins, vec3_t maxs, int team, int mask );

	// target_cycle functions
void SP_Q3F_target_cycle( gentity_t *ent );

	// Target mapping functions
void G_Q3F_AddEntityToTargetArray( gentity_t *ent );
void G_Q3F_RemoveEntityFromTargetArray( gentity_t *ent );

	// Target respawn function
void SP_Q3F_target_respawn( gentity_t *ent );
void SP_Q3F_target_multiport( gentity_t *ent );

	// func_damage ent
void SP_Q3F_func_damage( gentity_t *ent );
void SP_Q3F_target_accumulator( gentity_t *ent );

	// misc_onkill & misc_onprotect
void G_Q3F_CheckOnKill( gentity_t *attacker, gentity_t *victim );
void SP_Q3F_misc_onkill( gentity_t *self );
void SP_Q3F_misc_onprotect( gentity_t *self );
void G_Q3F_LinkOnKillChain();

	// target_semitrigger ent
void SP_Q3F_target_semitrigger( gentity_t *self );

	// func_explosion ent
void SP_Q3F_func_explosion( gentity_t *self );

	// misc_beam ent
void SP_Q3F_misc_beam( gentity_t *self );

	// func_wall ent
void SP_Q3F_func_wall( gentity_t *ent );

	// misc_mapsentry ent
void SP_Q3F_misc_mapsentry( gentity_t *ent );

//BirdDawg: CTF Ammo conversions
void SP_Q3F_CTF_AmmoConversion ( gentity_t * ent );

// WFA compat functions
void SP_Q3F_item_pack( gentity_t *ent );
void SP_Q3F_item_flagreturn_team2( gentity_t *ent );
void SP_Q3F_item_flagreturn_team1( gentity_t *ent );
void G_Q3F_CTFCompatAdjust();

// Force-field aware trace function
void G_Q3F_ForceFieldExtTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int realOwner, int contentmask );
static QINLINE void G_Q3F_ForceFieldTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,	const vec3_t end, int passEntityNum, int contentmask ) {
	G_Q3F_ForceFieldExtTrace( results, start, mins, maxs, end, passEntityNum, passEntityNum, contentmask );
}

void SP_Q3F_func_forcefield( gentity_t *ent );

void SP_Q3F_func_visibility( gentity_t * ent );


	// Panel functions
void SP_Q3F_Panel_Name( gentity_t *ent );
void SP_Q3F_Panel_ScoreSummary( gentity_t *ent );
void SP_Q3F_Panel_Location( gentity_t *ent );
void SP_Q3F_Panel_Timer( gentity_t *ent );
void SP_Q3F_Panel_Radar( gentity_t *self );
void SP_Q3F_Panel_Message( gentity_t *self );

// Spawn functions
qboolean G_Q3F_PositionDynamicSpawn( gentity_t *ent, vec3_t origin, vec3_t angles );
void SP_info_player_targetspawn( gentity_t *ent );

// Q3F timer
void SP_Q3F_misc_matchtimer( gentity_t *ent );
void SP_Q3F_misc_stopwatch( gentity_t *ent );
void SP_Q3F_misc_changeclass( gentity_t *ent );

// entity removal
void SP_Q3F_misc_blackhole( gentity_t *ent );


#endif//__G_Q3F_MAPENTS_H


