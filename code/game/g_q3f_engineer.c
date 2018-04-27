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
**	g_q3f_engineer.c
**
**	Server-side code for engineer stuff - sentry, supplystation
*/

//	s.angles			angles when idle
//	s.angles2			current desired target angles
//	s.otherEntityNum	Shell count
//	s.otherEntityNum2	Rocket count
//	s.legsAnim			Sentry level (0 for building)
//	s.torsoAnim			Sentry health
//	s.frame				Cycling variable for weapon 
//	s.time				Time to start spin-down
//	s.time2				Time to end spin-up
//	s.weapon			Currently firing
//	angle				Yaw angle to aim towards when idling
//	sound1to2			The number of the 'other engineer' trying to dismantle
//	last_move_time		The time the sentry last damaged someone.
//  timestamp			When to start a new check for an idling sound/movement
//  splashDamage			The current timestamp controls an idling movement

#include "g_local.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_team.h"
#include "g_q3f_mapents.h"
#include "bg_q3f_util.h"

#include "g_bot_interface.h"
#ifdef DREVIL_BOT_SUPPORT
static void BotSendSentryStatus(gentity_t *sentry);
static void BotSendDispenserStatus(gentity_t *station);
#endif

#define Q3F_SENTRY_BUILD_DISTANCE	50		// Distance to build from player
#define	Q3F_SENTRY_BUILD_TIME		5000	// Milliseconds to build
#define	Q3F_SENTRY_BUILD_CELLS		130		// Cells required to build
#define	Q3F_SENTRY_POLL_TIME		500		// Poll five times a second
#define	Q3F_SENTRY_POSTLOCK_TIME	1000	// Wait this long after losing a lock before returning to default pos // RR2DO2: was 3000
#define Q3F_SENTRY_IDLE_TIME		1000
#define	Q3F_SENTRY_MAX_RANGE		1200	// Max scan distance
#define Q3F_SENTRY_CLOSE_RANGE		400		// 'Close range' distance (i.e. shoot behind)
//#define Q3F_SENTRY_ROCKET_RANGE		100		// Minimum range rockets are fired at
#define Q3F_SENTRY_ROT_SPEED		350		// Degrees per second
#define Q3F_SENTRY_ROT_IDLE_SPEED	90		// Degrees per second to idle pos // RR2DO2: was 90
#define	Q3F_SENTRY_FIRE_INTERVAL	100		// Time between fires (frametime).
#define	Q3F_SENTRY_AIM_HEIGHT		40		// Height from base it aims from (level w/ player origin)
#define Q3F_SENTRY_MUZZLE_LENGTH	3		// Length of the cannons from origin
#define	Q3F_SENTRY_DAMAGE			9		// Amount of damage per bullet
#define	Q3F_SENTRY_CANCEL_MINTIME	1000	// Can't cancel for this long
#define	Q3F_SENTRY_BOREDOM_TIME		300000	// Sentries become 'bored' after this long without fighting.

#define	Q3F_BUILDING_REPAIR_TIME		10000	// 10 seconds before spanner becomes invalid.
#define Q3F_BUILDING_REPAIR_DISTANCE	50		// 50 units distant from nearest surface.

#define	Q3F_SUPPLYSTATION_BUILD_TIME	2000	// Milliseconds to build
#define	Q3F_SUPPLYSTATION_BUILD_CELLS	100		// Cells required to build
#define	Q3F_SUPPLYSTATION_REGEN_TIME	10000	// Milliseconds between ammo regen

#define Q3F_SUPPLYSTATION_SHELLS		400	
#define Q3F_SUPPLYSTATION_NAILS			600
#define Q3F_SUPPLYSTATION_ROCKETS		300
#define Q3F_SUPPLYSTATION_CELLS			400
#define Q3F_SUPPLYSTATION_ARMOUR		500

vec3_t q3f_sentry_min = {
	-16, -16, 0
};
vec3_t q3f_sentry_max = {
	16, 16, 48
};

vec3_t q3f_supplystation_min = {
	-8, -8, 0
};
vec3_t q3f_supplystation_max = {
	8, 8, 46
};

typedef struct AimBlock_s {
	int			realowner;
	int			mask;
	vec3_t		origin;
	vec3_t		start;
	vec3_t		end;
	vec3_t		angles;
	float		hoffset;
	float		voffset;
} AimBlock_t;

void G_Q3F_UpdateEngineerStats( gentity_t *player )
{
	// Set the relevant ammo fields to allow engineer data to be displayed

	unsigned int d1, d2;
	gentity_t *ent;

	if( !player || !player->client )
		return;
	
	d1 = d2 = 0;
	ent = player->client->sentry;
	if( ent )
	{
		d1 |= 0xFF & ((ent->health > 255) ? 255 : ((ent->health < 0) ? 0 : ent->health));
		d2 |= (0xFF & ent->s.otherEntityNum) + ((0xFF & ent->s.otherEntityNum2) << 8);
		#ifdef BUILD_BOTS
		Bot_Event_SendSentryStatsToBot( ent );
		#endif
	}
	ent = player->client->supplystation;
	if( ent )
	{
		d1 |= (0xFF & ((ent->health > 255) ? 255 : ((ent->health < 0) ? 0 : ent->health))) << 8;
		#ifdef BUILD_BOTS
		Bot_Event_SendSupplyStatsToBot( ent );
		#endif
	}

	*((unsigned int *)&player->client->ps.ammo[AMMO_Q3F_ENGDATA1]) = d1;
	*((unsigned int *)&player->client->ps.ammo[AMMO_Q3F_ENGDATA2]) = d2;
}

static qboolean G_Q3F_AttemptDismantle( gentity_t *player, gentity_t *building )
{
	// See if building can be dismantled, and mark it down otherwise.
	// (Lamer prevention suggests a "two-engineer" dismantle policy)

	gentity_t *other;

	if(	!player || !player->client ||
		player->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
		return( qfalse );
	if( building->parent == player )
		return( qtrue );
	// If it's an enemy building, just dismantle
	if( (other = &g_entities[building->sound1to2]) && other->client && !G_Q3F_IsAllied( other, building->parent ) )
		return( qtrue );

	player->client->sess.lastDismantleTime = level.time;	// Keep a record for cheat checking

	if( building->sound1to2 >= 0 && building->sound1to2 != player->s.number &&
		(other = &g_entities[building->sound1to2]) &&
		other->client && other->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER &&
		G_Q3F_IsAllied( other, building->parent ) )
	{
		// Already been 'marked' by another engineer, let's go.
		return( qtrue );
	}
	building->sound1to2 = player->s.number;	// Keep a record for later
	return( qfalse );
}

qboolean G_Q3F_SentryEnemyValid( gentity_t *ent, gentity_t *enemy )
{
	// Returns qtrue if the specified entity is a valid enemy.

	if(	!enemy || !enemy->inuse || enemy->health <= 0 || !enemy->client ||
		Q3F_IsSpectator( enemy->client ) || enemy->client->noclip )
		return( qfalse );

	//if( g_q3f_autoSentryMadDog.integer )
	//	return( qtrue );

//	if ( enemy == ent->parent)
//		return qtrue;

	if ( enemy == ent->parent ||
		G_Q3F_IsAllied( ent->parent, enemy ) ||
		(enemy->s.eFlags & EF_Q3F_INVISIBLE) ||
		(enemy->s.powerups & (1<<PW_INVIS)) ||
		enemy->client->agentteam == ent->parent->client->sess.sessionTeam
		)
		return qfalse;
	return qtrue;
}

void G_DumpClientInfo( gclient_t *cl, char* prefix, char* title) {
	char buf[16];
	
	if(!cl) {
		return;
	}

	Com_sprintf(buf, 16, "%s\t", prefix);

	G_LogPrintf("%s ===== %s =====\n", prefix, title );

	G_LogPrintf("%s Name: %s\n", prefix, cl->pers.netname );

	if(cl->supplystation) {
		G_DumpEntityInfo(cl->supplystation, qfalse, buf, "Supplystation", qfalse);
	}
	
	if(cl->sentry) {
		G_DumpEntityInfo(cl->sentry, qfalse, buf, "Sentry", qfalse);
	}
	
	G_LogPrintf("%s ==================\n", prefix );
}

void G_DumpEntityInfo( gentity_t *ent, qboolean coredump, char* prefix, char* title, qboolean client ) {
	char buf[16];

	if(!ent) {
		return;
	}

	Com_sprintf(buf, 16, "%s\t", prefix);

	G_LogPrintf("%s ===== %s =====\n", prefix, title );

	if(ent->activator && coredump) {
		G_DumpEntityInfo(ent->activator, qfalse, buf, "Activator", qtrue);
	}

	G_LogPrintf("%s Classname: %s\n", prefix, ent->classname ); 

	if(ent->client && client) {
		G_DumpClientInfo(ent->client, buf, "Client");
	}

	if(ent->activator && coredump) {
		G_DumpEntityInfo(ent->enemy, qfalse, buf, "Enemy", qtrue);
	}

	G_LogPrintf("%s Flags: %i\n", prefix, ent->flags );
	G_LogPrintf("%s Health: %i\n", prefix, ent->health );
	G_LogPrintf("%s Model: %s\n", prefix, ent->model );
	G_LogPrintf("%s Model2: %s\n", prefix, ent->model2 );
	G_LogPrintf("%s Origin: (%f %f %f)\n", prefix, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] );
	G_LogPrintf("%s Linked: %i\n", prefix, ent->r.linked );
	G_LogPrintf("%s Inuse: %i\n", prefix, ent->inuse );

	if(ent->parent && coredump) {
		G_DumpEntityInfo(ent->parent, qfalse, buf, "Parent", qtrue);
	}

	G_LogPrintf("%s ==================\n", prefix );
}

void G_Q3F_SentryDie( gentity_t *sentry, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	gentity_t *parent/*, *temp*/;
	float realdamage;
	char *attackerName;

	parent = sentry->parent;

	if( meansOfDeath == MOD_LAVA ) {
		attackerName = "MOD_LAVA";
		trap_SendServerCommand(	-1, va( parent->client->pers.gender == GENDER_MALE ?	"print \"%s ^7sacrificed his autosentry to the mighty lavagod.\n\"" : 
																						"print \"%s ^7sacrificed her autosentry to the mighty lavagod.\n\"", parent->client->pers.netname ) );
	} else if( meansOfDeath == MOD_SLIME ) {
		attackerName = "MOD_SLIME";
		trap_SendServerCommand(	-1, va( "print \"%s^7's autosentry got an acid dip.\n\"", parent->client->pers.netname ) );
	} else if( parent == attacker ) {
		attackerName = parent->client->pers.netname;
		trap_SendServerCommand(	-1, va( parent->client->pers.gender == GENDER_MALE ?	"print \"%s ^7has destroyed his autosentry.\n\"" :
																						"print \"%s ^7has destroyed her autosentry.\n\"", attackerName ) );
	} else if( sentry == attacker ) {
		attackerName = parent->client->pers.netname;
		trap_SendServerCommand(	-1, va( "print \"%s^7's autosentry has blown itself up.\n\"", attackerName ) );
	} else if( attacker ) {
		qboolean isally = G_Q3F_IsAllied( attacker, parent ) ? qtrue : qfalse;
		attackerName = attacker->client ? attacker->client->pers.netname : "somebody";
		trap_SendServerCommand(	-1, va( "print \"%s^7's autosentry has been destroyed by %s%s^7!\n\"",
								parent->client->pers.netname, (isally ? "^sally^7 " : ""), attackerName ) );
	} else {
		attackerName = "Unknown";
		trap_SendServerCommand(	-1, va( "print \"%s^7's autosentry has been destroyed by an unknown entity!\n\"",
								parent->client->pers.netname ) );
	}

#ifdef BUILD_BOTS
	Bot_Event_SentryDestroyed( parent, attacker );
#endif

	G_LogPrintf(	"Destroy: autosentry %i %i: %s^7destroyed %s^7's autosentry.\n", 
					attacker->s.number, parent->s.number, attackerName, parent->client->pers.netname );

	realdamage = sentry->s.otherEntityNum + sentry->s.otherEntityNum2 * 3;
	if( realdamage > 200 )
		realdamage = 200;
	/*temp = */(void)G_TempEntity( sentry->r.currentOrigin, EV_SENTRY_EXPLOSION );

	sentry->takedamage = qfalse;		// Stop infinite damage loops against other sentries :)
	G_RadiusDamage(	sentry->r.currentOrigin, sentry, sentry->parent, realdamage, sentry, MOD_AUTOSENTRY_EXPLODE, 0 );

	if( parent && parent->client && parent->client->sentry == sentry ) {
		parent->client->sentry = NULL;
		G_Q3F_UpdateEngineerStats( parent );
	}

	if( attacker && attacker->client && attacker != parent ) {
		if ( G_Q3F_IsAllied( attacker, parent ) ) {
			AddScore( attacker, attacker->r.currentOrigin, -1 );
			G_Q3F_RegisterTeamKill( attacker, NULL );
		}
		else {
			AddScore( attacker, attacker->r.currentOrigin, 1 );
		}
	}

	// RR2DO2: kill sentrycam
	G_FreeEntity( sentry->target_ent );
	G_FreeEntity( sentry );

#ifdef DREVIL_BOT_SUPPORT
	if(parent)
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = attacker;
		Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_DESTROYED, parent->s.number, 0,0, &data);
	}	
#endif
}

void G_Q3F_SentryPain( gentity_t *ent, gentity_t *attacker, int damage )
{
	// We've been hurt, see if we can return the complement

#ifdef DREVIL_BOT_SUPPORT
	BotSendSentryStatus(ent);
#endif

#ifdef BUILD_BOTS
	Bot_Event_BuildableDamaged( ent->parent, ET_Q3F_SENTRY, ent );
	Bot_Event_SendSentryStatsToBot( ent );
#endif

	if( !ent->enemy && G_Q3F_SentryEnemyValid( ent, attacker) )
	{
		// Check we have line-of-sight and range on the enemy
		if( Distance( ent->r.currentOrigin, attacker->r.currentOrigin ) < Q3F_SENTRY_MAX_RANGE )
		{
			ent->s.time = level.time + 150;
			ent->enemy = attacker;
		}
	}
	G_Q3F_UpdateEngineerStats( ent->parent );
}

int G_Q3F_SentryMaxHealth( int sentlevel )
{
	switch( sentlevel )
	{
		case 1:		return( 150 );
		case 2:		return( 180 );
		case 3:		return( 220 );
		case 4:		return( 250 );
		default:	return( 0 );
	}
}

int G_Q3F_SentryMaxShells( int sentlevel )
{
	switch( sentlevel )
	{
		case 1:		return( 100 );
		case 2:		return( 120 );
		case 3:		return( 150 );
		case 4:		return( 180 );
		default:	return( 0 );
	}
}

void G_Q3F_SentryFinishBuild( gentity_t *ent )
{
	// Sentry has finished building.

	ent->parent->client->ps.ammo[AMMO_CELLS] -= Q3F_SENTRY_BUILD_CELLS;
	if( ent->parent->client->ps.ammo[AMMO_CELLS] < 0 )
		ent->parent->client->ps.ammo[AMMO_CELLS] = 0;
	ent->parent->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_BUILDING);

	ent->s.legsAnim = 1;					// Sentry level
	ent->s.otherEntityNum = 50;				// Initial shell count
	ent->s.otherEntityNum2 = 0;				// Inital rocket count
	ent->enemy = NULL;						// Enemy entity
	ent->think = NULL;
	ent->takedamage = qtrue;
	ent->health = G_Q3F_SentryMaxHealth( 1 );
	//ent->nextbeamhittime = level.time;
	G_Q3F_UpdateEngineerStats( ent->parent );

	trap_SendServerCommand( ent->parent->s.number, "print \"Your autosentry is complete.\n\"" );

#ifdef BUILD_BOTS
	Bot_Event_SentryBuilt( ent->parent, ent );
	Bot_Event_SendSentryStatsToBot( ent );
#endif

#ifdef DREVIL_BOT_SUPPORT
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = ent;
		Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_BUILT, ent->parent->s.number, 0, 0, &data);
		BotSendSentryStatus(ent);
	}	
#endif

	// RR2DO2: spawn sentrycam
	ent->target_ent = G_Spawn();
	ent->target_ent->classname	= "sentrycam";
	ent->target_ent->s.eType	= ET_Q3F_SENTRYCAM;
	ent->target_ent->r.svFlags	= (SVF_SINGLECLIENT|SVF_PORTAL);
	ent->target_ent->r.singleClient = ent->s.clientNum;
//	ent->target_ent->nextthink = 0;
	VectorCopy( ent->r.currentOrigin, ent->target_ent->s.origin2 );
}

void G_Q3F_SentryBuild( gentity_t *ent )
{
	// Check a sentry can be built, then start building.

	vec3_t origin, tracestart, traceend, sentrymin;
	trace_t trace;
	int traceoffset, index;
	gentity_t *sentry;
	bg_q3f_playerclass_t *cls;

	if( !ent->client || ent->client->ps.stats[STAT_HEALTH] <= 0 )
		return;
	if( level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )
		return;
	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
	{
		trap_SendServerCommand( ent->s.number, "print \"Only engineers can build autosentries.\n\"" );
		return;
	}
	if( ent->client->sentry && ent->client->sentry->s.eType == ET_Q3F_SENTRY && ent->client->sentry->parent == ent )
	{
		if( ent->client->sentry->s.legsAnim ) {
			#ifdef BUILD_BOTS
			Bot_Event_Build_AlreadyBuilt( ent, ET_Q3F_SENTRY );
			#endif
			trap_SendServerCommand( ent->s.number, "print \"You can only build one autosentry at a time.\n\"" );
		}
		else {
			#ifdef BUILD_BOTS
			Bot_Event_BuildCancelled( ent, ET_Q3F_SENTRY );
			#endif
			trap_SendServerCommand( ent->s.number, "print \"You have stopped building.\n\"" );
			ent->client->ps.ammo[AMMO_CELLS] += Q3F_SENTRY_BUILD_CELLS;
			cls = BG_Q3F_GetClass( &ent->client->ps );
			if( ent->client->ps.ammo[AMMO_CELLS] > cls->maxammo_cells )
				ent->client->ps.ammo[AMMO_CELLS] = cls->maxammo_cells;
		}
		return;
	}
	if( ent->client->ps.ammo[AMMO_CELLS] < Q3F_SENTRY_BUILD_CELLS )
	{
		trap_SendServerCommand( ent->s.number, va( "print \"You need at least %d cells to build an autosentry.\n\"", Q3F_SENTRY_BUILD_CELLS ) );
		#ifdef BUILD_BOTS
		Bot_Event_Build_NotEnoughAmmo( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	// Get the sentry origin
	VectorClear( sentrymin );
	sentrymin[YAW] = ent->client->ps.viewangles[YAW];
	AngleVectors( sentrymin, origin, NULL, NULL );
	VectorScale( origin, Q3F_SENTRY_BUILD_DISTANCE, origin );
	VectorAdd( ent->r.currentOrigin, origin, origin );
	origin[2] += Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

	// Try to check the engineer can actually 'reach' that area.
	G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
	if( trace.startsolid || trace.fraction < 1 )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;						// Something got in the way
	}
	origin[2] -= Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

		// Work out the start and end points (so we can set the sentry on the ground)
	for( index = 0; index < 3; index++ )
	{
		switch( index )
		{
			case 0: traceoffset = 32;	break;
			case 1: traceoffset = 0.25;	break;
			case 2:	traceoffset	= 48;	break;
		}
		VectorCopy( origin, tracestart );
		tracestart[2] += traceoffset + q3f_sentry_min[2];
		VectorCopy( origin, traceend );
		traceend[2] += traceoffset - 64 + q3f_sentry_min[2];

		G_Q3F_ForceFieldTrace( &trace, tracestart, q3f_sentry_min, q3f_sentry_max, traceend, ent->s.number, MASK_PLAYERSOLID );
		if( trace.fraction == 1 || /*trace.startsolid ||*/ trace.entityNum < MAX_CLIENTS )//|| trace.startsolid )	// We never hit land, got wedged into something, or landed on a player
			continue;

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldExtTrace( &trace, origin, q3f_sentry_min, q3f_sentry_max, origin, ENTITYNUM_NONE, ent->s.number, MASK_PLAYERSOLID );
		if( trace.startsolid )				// We're embedded in something
			continue;

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
		if( trace.startsolid || trace.fraction < 1 )				// We're no longer visible to the engineer
			continue;

		break;
	}
	if( index >= 3 )
	{
		// We failed. Give up.
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	VectorAdd( trace.endpos, q3f_sentry_min, tracestart );
	VectorAdd( trace.endpos, q3f_sentry_max, traceend );
	if( G_Q3F_NoBuildCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOBUILD_AUTOSENTRY ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOANNOY_BUILDING ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	// Now we have a position, initialise it.
	sentry = G_Spawn();
	sentry->classname	= "autosentry";
	sentry->s.eType		= ET_Q3F_SENTRY;		// Sentry has custom rendering
	sentry->s.legsAnim	= 0;					// Sentry level (0 means building)
	sentry->s.time		= level.time;			// Time building started.
	// Set the angles.
	sentry->s.angles[YAW]	= AngleNormalize360( ent->client->ps.viewangles[YAW] - 180 );
	sentry->s.angles[PITCH]	= 0;
	sentry->s.angles[ROLL]	= 0;
	SnapVector( sentry->s.angles );
	// And the current angles.
	VectorCopy( sentry->s.angles, sentry->s.apos.trBase );
	sentry->s.apos.trType = TR_INTERPOLATE;
	// And the think.
	sentry->die			= G_Q3F_SentryDie;
	sentry->pain		= G_Q3F_SentryPain;
	sentry->think		= G_Q3F_SentryFinishBuild;
	sentry->nextthink	= level.time + Q3F_SENTRY_BUILD_TIME;
	sentry->last_move_time = level.time;

	// And position it
	VectorCopy( trace.endpos, origin );
	origin[2] -= q3f_sentry_min[2];
	G_SetOrigin( sentry, origin );
	sentry->s.pos.trType = TR_GRAVITY;
	sentry->s.pos.trTime = level.time;
	sentry->s.groundEntityNum = ENTITYNUM_NONE;

	sentry->parent = ent;
	sentry->r.ownerNum = ENTITYNUM_NONE;	// Nobody 'owns' this as far as physics are concerned
	sentry->s.clientNum = ent->s.number;	// But we still need to know for ID purposes
	VectorCopy( q3f_sentry_min, sentry->r.mins );
	VectorCopy( q3f_sentry_max, sentry->r.maxs );
	sentry->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	sentry->r.contents = CONTENTS_BODY | CONTENTS_PLAYERCLIP;
	sentry->physicsObject = qtrue;
	trap_LinkEntity( sentry );

	ent->client->sentry = sentry;
	ent->client->ps.stats[STAT_Q3F_FLAGS] |= (1 << FL_Q3F_BUILDING);
	ent->client->buildTime = level.time + Q3F_SENTRY_BUILD_TIME;

	// We set these NOW so that ID can't come up wrong when a sentry is first build
	sentry->s.otherEntityNum = 50;				// Initial shell count
	sentry->s.otherEntityNum2 = 0;				// Inital rocket count
	sentry->health = G_Q3F_SentryMaxHealth( 1 );
	sentry->sound1to2 = -1;						// Engineers wishing to dismantle

	G_AddEvent( sentry, EV_SENTRY_BUILD, 0);

#ifdef BUILD_BOTS
	Bot_Event_SentryBuilding( ent, sentry );
#endif

#ifdef DREVIL_BOT_SUPPORT
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = sentry;
		Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_BUILDING, ent->s.number, 0, 0, &data);
	}
#endif

	trap_SendServerCommand( ent->s.number, "print \"Building autosentry...\n\"" );
}

qboolean G_Q3F_SentryCancel( gentity_t *sentry )
{
	if( sentry && !sentry->s.legsAnim && sentry->s.eType == ET_Q3F_SENTRY && sentry->parent->client->sentry == sentry )
	{
		#ifdef BUILD_BOTS
		Bot_Event_BuildCancelled( sentry->parent, ET_Q3F_SENTRY );
		#endif
		sentry->parent->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_BUILDING);
		sentry->parent->client->buildTime = 0;
		sentry->parent->client->buildDelayTime = level.time + Q3F_SENTRY_CANCEL_MINTIME;

		sentry->parent->client->sentry = NULL;

		G_Q3F_UpdateEngineerStats( sentry->parent );

		G_FreeEntity( sentry );

		return( qtrue );
	}
	return( qfalse );
}

gentity_t *G_Q3F_CheckSentryUpgradeable( gentity_t *player, int sentrynum, qboolean checkforRepairent )
{
	// Check the sentry is valid, or pick a valid sentry in range

	gentity_t *sentry;
	vec3_t vec;

	sentry = sentrynum ? &g_entities[sentrynum] : player->client->repairEnt;
	if( !sentry || !sentry->inuse || sentry->s.eType != ET_Q3F_SENTRY || sentry->health <= 0 )
		return( NULL );		// Not a sentry
	if( sentry->parent != player && !G_Q3F_IsAllied( player, sentry->parent ) )
		return( NULL );		// Not upgradeable
	if( !sentry->s.legsAnim )
		return( NULL );		// Not yet built
	if( checkforRepairent && (sentry != player->client->repairEnt || level.time > player->client->repairTime + Q3F_BUILDING_REPAIR_TIME) )
		return( NULL );
	VectorCopy( player->client->ps.origin, vec );
	vec[2] += player->client->ps.viewheight;
	VectorSubtract( vec, sentry->r.currentOrigin, vec );
	if( SQRTFAST( vec[0]*vec[0] + vec[1]*vec[1] ) > Q3F_BUILDING_REPAIR_DISTANCE ||
		player->r.absmin[2] - sentry->r.absmax[2] > 0.5 * Q3F_BUILDING_REPAIR_DISTANCE || 
		sentry->r.absmin[2] - player->r.absmax[2] > 0.5 * Q3F_BUILDING_REPAIR_DISTANCE )
		return( NULL );

	return( sentry );
}

void G_Q3F_SentryUpgrade( gentity_t *player, int sentrynum )
{
	// Upgrade the specified sentry (or else pick one in range)

	gentity_t *sentry;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qfalse );
	if( !sentry || !sentry->s.legsAnim || sentry->s.legsAnim >= 3 )
		return;

	if( player->client->ps.ammo[AMMO_CELLS] < Q3F_SENTRY_BUILD_CELLS )
	{
		trap_SendServerCommand( player->s.number, va( "print \"You need %d cells to upgrade your autosentry.\n\"", Q3F_SENTRY_BUILD_CELLS ) );
		return;
	}

	sentry->s.legsAnim++;
	player->client->ps.ammo[AMMO_CELLS] -= Q3F_SENTRY_BUILD_CELLS;
	
	sentry->health = G_Q3F_SentryMaxHealth( sentry->s.legsAnim );

#ifdef BUILD_BOTS
	Bot_Event_SentryUpgraded( sentry->parent, sentry->s.legsAnim );
#endif

	//G_AddEvent( sentry, EV_GENERAL_SOUND, sentry->soundPos2 );

	if( sentry->parent == player )
		trap_SendServerCommand( player->s.number, va( "print \"You have upgraded your autosentry to level %d\n\"", sentry->s.legsAnim ) );
	else if( sentry->parent && sentry->parent->client )
		trap_SendServerCommand( player->s.number, va( "print \"You have upgraded %s^7's autosentry to level %d\n\"", sentry->parent->client->pers.netname, sentry->s.legsAnim ) );
	G_Q3F_UpdateEngineerStats( sentry->parent );

	player->client->repairEnt = NULL;
}

void G_Q3F_SentryRepair( gentity_t *player, int sentrynum )
{
	// Repair the specified sentry (or else pick one in range)

	gentity_t *sentry;
	int cells;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qfalse );
	if( !sentry || !sentry->s.legsAnim )
		return;

	cells = (G_Q3F_SentryMaxHealth( sentry->s.legsAnim ) + 4 - sentry->health) / 5;
	if( cells > player->client->ps.ammo[AMMO_CELLS] )
		cells = player->client->ps.ammo[AMMO_CELLS];
	player->client->ps.ammo[AMMO_CELLS] -= cells;
	sentry->health += cells * 5;
	if( sentry->health > G_Q3F_SentryMaxHealth( sentry->s.legsAnim ) )
		sentry->health = G_Q3F_SentryMaxHealth( sentry->s.legsAnim );

	G_Q3F_UpdateEngineerStats( sentry->parent );
	player->client->repairEnt = NULL;
}

void G_Q3F_SentryRefill( gentity_t *player, int sentrynum )
{
	// Refill the specified sentry (or else pick one in range)
	// We give 40 shells / 20 rockets refill

	gentity_t *sentry;
	int ammo, available, max;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qfalse );
	if( !sentry || !sentry->s.legsAnim )
		return;

	available = player->client->ps.ammo[AMMO_SHELLS] - Q3F_GetClipValue( WP_SUPERSHOTGUN, &player->client->ps );
	ammo = (available >= 40) ? 40 : available;
	if( (ammo + sentry->s.otherEntityNum) > G_Q3F_SentryMaxShells( sentry->s.legsAnim ) )
		ammo = G_Q3F_SentryMaxShells( sentry->s.legsAnim ) - sentry->s.otherEntityNum;
	player->client->ps.ammo[AMMO_SHELLS] -= ammo;
	sentry->s.otherEntityNum += ammo;

	if( sentry->s.legsAnim >= 3 )
	{
		available = player->client->ps.ammo[AMMO_ROCKETS];
		max = 20;
		ammo = (available >= max) ? max: available;
		if( (ammo + sentry->s.otherEntityNum2) > max )
			ammo = max- sentry->s.otherEntityNum2;
		player->client->ps.ammo[AMMO_ROCKETS] -= ammo;
		sentry->s.otherEntityNum2 += ammo;
	}
	G_Q3F_UpdateEngineerStats( sentry->parent );
	player->client->repairEnt = NULL;
}

void G_Q3F_SentryRotate( gentity_t *player, int sentrynum, int angle )
{
	// Repair the specified sentry (or else pick one in range)

	gentity_t *sentry;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qtrue );
	if( !sentry || !sentry->s.legsAnim )
		return;

	sentry->s.angles[YAW] = AngleNormalize360( sentry->s.angles[YAW] + angle );
#ifdef BUILD_BOTS
	Bot_Event_SentryAimed( sentry->parent, sentry, sentry->s.angles );
#endif
	//Give it slightly longer timestamp so it doesn't go idle around too fast
	sentry->timestamp = level.time + Q3F_SENTRY_IDLE_TIME*2;
	VectorCopy( sentry->s.angles, sentry->movedir );
	sentry->damage = 0;

	player->client->repairEnt = NULL;
}

void G_Q3F_SentryPlrMove( gentity_t *player, int sentrynum )
{
	// Repair the specified sentry (or else pick one in range)

	gentity_t *sentry;
	vec3_t sentrymin, origin;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qtrue );
	if( !sentry || !sentry->s.legsAnim /*|| sentry->sound2to1 || player != sentry->parent*/ )
		return;

	// RR2DO2: kill sentrycam
	G_FreeEntity( sentry->target_ent );

	player->client->ps.stats[STAT_Q3F_FLAGS] |= (1 << FL_Q3F_MOVING);
	sentry->takedamage = qfalse;
	sentry->clipmask = 0;//CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	sentry->r.contents = 0;//CONTENTS_BODY | CONTENTS_PLAYERCLIP;
	
	sentry->sound2to1 = 1;
	
	// Set the angles.
	sentry->s.angles[YAW]	= AngleNormalize360( player->client->ps.viewangles[YAW] );
	sentry->s.angles[PITCH]	= 0;
	sentry->s.angles[ROLL]	= 0;
	SnapVector( sentry->s.angles );
	// And the current angles.
	VectorCopy( sentry->s.angles, sentry->s.apos.trBase );
	sentry->s.apos.trType = TR_INTERPOLATE;
	
	// Get the sentry origin
	VectorClear( sentrymin );
	sentrymin[YAW] = player->client->ps.viewangles[YAW];
	AngleVectors( sentrymin, origin, NULL, NULL );
	VectorScale( origin, Q3F_SENTRY_BUILD_DISTANCE, origin );
	VectorAdd( player->r.currentOrigin, origin, origin );
	//origin[2] += Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

	// And position it
	//VectorCopy( trace.endpos, origin );
	//origin[2] -= q3f_sentry_min[2];
	G_SetOrigin( sentry, origin );
	trap_LinkEntity(sentry);

	/*sentry->s.angles[YAW] = AngleNormalize360( sentry->s.angles[YAW] + angle );
#ifdef BUILD_BOTS
	Bot_Event_SentryAimed( sentry->parent, sentry, sentry->s.angles );
#endif
	//Give it slightly longer timestamp so it doesn't go idle around too fast
	sentry->timestamp = level.time + Q3F_SENTRY_IDLE_TIME*2;
	VectorCopy( sentry->s.angles, sentry->movedir );
	sentry->damage = 0;

	player->client->repairEnt = NULL;*/
}

#if 0
void G_Q3F_SentryAttemptPlace( gentity_t *player )
{
	// Get the sentry origin
	VectorClear( sentrymin );
	sentrymin[YAW] = ent->client->ps.viewangles[YAW];
	AngleVectors( sentrymin, origin, NULL, NULL );
	VectorScale( origin, Q3F_SENTRY_BUILD_DISTANCE, origin );
	VectorAdd( ent->r.currentOrigin, origin, origin );
	origin[2] += Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

	// Try to check the engineer can actually 'reach' that area.
	G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
	if( trace.startsolid || trace.fraction < 1 )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot place an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;						// Something got in the way
	}
	origin[2] -= Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

		// Work out the start and end points (so we can set the sentry on the ground)
	for( index = 0; index < 3; index++ )
	{
		switch( index )
		{
			case 0: traceoffset = 32;	break;
			case 1: traceoffset = 0.25;	break;
			case 2:	traceoffset	= 48;	break;
		}
		VectorCopy( origin, tracestart );
		tracestart[2] += traceoffset + q3f_sentry_min[2];
		VectorCopy( origin, traceend );
		traceend[2] += traceoffset - 64 + q3f_sentry_min[2];

		G_Q3F_ForceFieldTrace( &trace, tracestart, q3f_sentry_min, q3f_sentry_max, traceend, ent->s.number, MASK_PLAYERSOLID );
		if( trace.fraction == 1 || /*trace.startsolid ||*/ trace.entityNum < MAX_CLIENTS )//|| trace.startsolid )	// We never hit land, got wedged into something, or landed on a player
			continue;

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldExtTrace( &trace, origin, q3f_sentry_min, q3f_sentry_max, origin, ENTITYNUM_NONE, ent->s.number, MASK_PLAYERSOLID );
		if( trace.startsolid )				// We're embedded in something
			continue;

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
		if( trace.startsolid || trace.fraction < 1 )				// We're no longer visible to the engineer
			continue;

		break;
	}
	if( index >= 3 )
	{
		// We failed. Give up.
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	VectorAdd( trace.endpos, q3f_sentry_min, tracestart );
	VectorAdd( trace.endpos, q3f_sentry_max, traceend );
	if( G_Q3F_NoBuildCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOBUILD_AUTOSENTRY ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOANNOY_BUILDING ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an autosentry here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SENTRY );
		#endif
		return;
	}

	// Now we have a position, initialise it.
	sentry = G_Spawn();
	sentry->classname	= "autosentry";
	sentry->s.eType		= ET_Q3F_SENTRY;		// Sentry has custom rendering
	sentry->s.legsAnim	= 0;					// Sentry level (0 means building)
	sentry->s.time		= level.time;			// Time building started.
	// Set the angles.
	sentry->s.angles[YAW]	= AngleNormalize360( ent->client->ps.viewangles[YAW] - 180 );
	sentry->s.angles[PITCH]	= 0;
	sentry->s.angles[ROLL]	= 0;
	SnapVector( sentry->s.angles );
	// And the current angles.
	VectorCopy( sentry->s.angles, sentry->s.apos.trBase );
	sentry->s.apos.trType = TR_INTERPOLATE;
	// And the think.
	sentry->die			= G_Q3F_SentryDie;
	sentry->pain		= G_Q3F_SentryPain;
	sentry->think		= G_Q3F_SentryFinishBuild;
	sentry->nextthink	= level.time + Q3F_SENTRY_BUILD_TIME;
	sentry->last_move_time = level.time;

	// And position it
	VectorCopy( trace.endpos, origin );
	origin[2] -= q3f_sentry_min[2];
	G_SetOrigin( sentry, origin );
	sentry->s.pos.trType = TR_GRAVITY;
	sentry->s.pos.trTime = level.time;
	sentry->s.groundEntityNum = ENTITYNUM_NONE;

	sentry->parent = ent;
	sentry->r.ownerNum = ENTITYNUM_NONE;	// Nobody 'owns' this as far as physics are concerned
	sentry->s.clientNum = ent->s.number;	// But we still need to know for ID purposes
	VectorCopy( q3f_sentry_min, sentry->r.mins );
	VectorCopy( q3f_sentry_max, sentry->r.maxs );
	sentry->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	sentry->r.contents = CONTENTS_BODY | CONTENTS_PLAYERCLIP;
	sentry->physicsObject = qtrue;
	trap_LinkEntity( sentry );

	ent->client->sentry = sentry;
	ent->client->ps.stats[STAT_Q3F_FLAGS] |= (1 << FL_Q3F_BUILDING);
	ent->client->buildTime = level.time + Q3F_SENTRY_BUILD_TIME;

	// We set these NOW so that ID can't come up wrong when a sentry is first build
	sentry->s.otherEntityNum = 50;				// Initial shell count
	sentry->s.otherEntityNum2 = 0;				// Inital rocket count
	sentry->health = G_Q3F_SentryMaxHealth( 1 );
	sentry->sound1to2 = -1;						// Engineers wishing to dismantle

	G_AddEvent( sentry, EV_SENTRY_BUILD, 0);

/*#ifdef BUILD_BOTS
	Bot_Event_SentryBuilding( ent, sentry );
#endif

#ifdef DREVIL_BOT_SUPPORT
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = sentry;
		Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_BUILDING, ent->s.number, 0, 0, &data);
	}
#endif*/

	trap_SendServerCommand( ent->s.number, "print \"Placing autosentry...\n\"" );
}
#endif

void G_Q3F_SentryDismantle( gentity_t *player, int sentrynum )
{
	// Dismantle the specified sentry and reclaim cells

	gentity_t *sentry;
	int cells;
	bg_q3f_playerclass_t *cls;

	sentry = G_Q3F_CheckSentryUpgradeable( player, sentrynum, qtrue );
	if( !sentry || !sentry->s.legsAnim )
		return;
	if( !G_Q3F_AttemptDismantle( player, sentry ) )
	{
		trap_SendServerCommand( player->s.number, "print \"Two engineers are required to dismantle this.\n\"" );
		return;
	}

	cls = BG_Q3F_GetClass( &player->client->ps );

	cells = Q3F_SENTRY_BUILD_CELLS / 2;
	if( player->client->ps.ammo[AMMO_CELLS] > cls->maxammo_cells )	// Ignore if they're 'overcelled'
		cells = 0;
	else if( (player->client->ps.ammo[AMMO_CELLS] + cells) > cls->maxammo_cells )
		cells = cls->maxammo_cells - player->client->ps.ammo[AMMO_CELLS];

#ifdef BUILD_BOTS
	Bot_Event_SentryDismantled( sentry->parent );
#endif

	if( sentry->parent == player )
		trap_SendServerCommand( player->s.number, va( "print \"You have dismantled your autosentry, and recovered %d cells.\n\"", cells ) );
	else {
		trap_SendServerCommand( player->s.number, va( "print \"You have dismantled %s^7's autosentry, and recovered %d cells.\n\"", sentry->parent->client->pers.netname, cells ) );
		trap_SendServerCommand( player->s.number, va( "print \"%s^7 has dismantled your autosentry!\n\"", player->client->pers.netname ) );
	}
	player->client->ps.ammo[AMMO_CELLS] += cells;

	sentry->parent->client->sentry = NULL;
	G_Q3F_UpdateEngineerStats( sentry->parent );

	G_LogPrintf(	"Dismantle: autosentry %d %d: %s dismantled %s's autosentry.\n",
					sentry->parent->s.number, player->s.number,
					player->client->pers.netname,
					sentry->parent->client->pers.netname );

	// RR2DO2: kill sentrycam
	G_FreeEntity( sentry->target_ent );

	G_FreeEntity( sentry );
	player->client->repairEnt = NULL;
}


/*
**	The run function for the sentry.
**
**	The fun bits go in here... it shoots at enemies, basically :)
*/

static void G_Q3F_Sentry_Fire( gentity_t *ent, AimBlock_t *aim, int damage )
{
	// Modified version of the machine gun Bullet_Fire() function.
	// Takes muzzle pos and direction as arguments, along with the offset along
	// the left-right plane so we can have parallel shots etc (assuming roll is 0).

	trace_t		tr;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	gclient_t	*client = ent->parent->client;


	if( ent->s.otherEntityNum <= 0 )
		return;

	if(level.intermissiontime)
		return;

	ent->s.otherEntityNum--;

	ent->s.weapon = 1;
	client->pers.stats.data[STATS_SENTRY].shots++;
	G_Q3F_ForceFieldTrace( &tr, aim->start, NULL, NULL, aim->end, ent->s.number, MASK_SHOT );

	if( g_debugBullets.integer & 1 )
		G_Q3F_DebugTrace( aim->start, &tr );

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];
	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, aim->start );
	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
		tent->s.eventParm = traceEnt->s.number;
		ent->last_move_time = level.time;
	} else {
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}
	tent->s.otherEntityNum = ent->s.number;

	if ( traceEnt->takedamage ) {
		vec3_t dir;
		int given;
		VectorSubtract( tr.endpos, aim->start, dir );

		given = client->pers.stats.data[STATS_SENTRY].given;

		if( traceEnt->client )
			G_Damage( traceEnt, ent, ent->parent, dir , tr.endpos,
				damage * (1.0f + 0.005f * (float)traceEnt->client->ps.stats[STAT_ARMORTYPE]), DAMAGE_Q3F_SHELL, MOD_AUTOSENTRY_BULLET );
		else
			G_Damage( traceEnt, ent, ent->parent, dir , tr.endpos,
				damage, DAMAGE_Q3F_SHELL, MOD_AUTOSENTRY_BULLET );

		if (given < client->pers.stats.data[STATS_SENTRY].given )
			client->pers.stats.data[STATS_SENTRY].hits++;
	}
}

void G_ExplodeMissile( gentity_t *ent );
#define	MISSILE_PRESTEP_TIME	50
static void G_Q3F_Sentry_RocketFire( gentity_t *ent, AimBlock_t *aim ) {
	gentity_t	*bolt;
	vec3_t forward, start;

	if( ent->s.otherEntityNum2 <= 0 )
		return;

	if(level.intermissiontime)
		return;

	ent->s.otherEntityNum2--;

	ent->parent->client->pers.stats.data[STATS_SENTRY].shots++;

	VectorSubtract( aim->end, aim->start, forward );
	VectorNormalize( forward );
	VectorMA( aim->start, Q3F_SENTRY_MUZZLE_LENGTH , forward, start );

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = ent->s.number;			// Owned by sentry, will hit sentry owner
	bolt->parent = ent->parent;
	bolt->damage = 92+Q_flrand(0.0f, 1.0f)*20;
	bolt->splashDamage = 92;
	bolt->methodOfDeath = MOD_AUTOSENTRY_ROCKET;
	bolt->splashMethodOfDeath = MOD_AUTOSENTRY_ROCKET;
	bolt->clipmask = MASK_SHOT;
	bolt->s.powerups = ent->parent->s.powerups & (1<<PW_QUAD);

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( forward, 900, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->r.currentOrigin );

	trap_LinkEntity( bolt );
}


static qboolean G_Q3F_FindLock( AimBlock_t * aim, gentity_t *target ) {
	/* Aim at the normal origin */
	trace_t tr;
	float aimz;
	int targno = target->s.number;

	aimz = target->r.currentOrigin[2];
	/* Try the center */
	VectorCopy( target->r.currentOrigin, aim->end );
	if ( aim->mask == (trap_PointContents( aim->end, targno ) & MASK_WATER)) {
		G_Q3F_ForceFieldExtTrace(&tr, aim->origin, NULL, NULL, aim->end, ENTITYNUM_NONE, aim->realowner, MASK_SOLID );
		if (tr.fraction == 1 ) 
			return qtrue;
	}
	/* Try the bottom */
	aim->end[2] = aimz + target->r.mins[2] * 0.75;
	if ( aim->mask == (trap_PointContents( aim->end, targno ) & MASK_WATER)) {
		G_Q3F_ForceFieldExtTrace(&tr, aim->origin, NULL, NULL, aim->end, ENTITYNUM_NONE, aim->realowner, MASK_SOLID );
		if (tr.fraction == 1 ) 
			return qtrue;
	}
	/* Try the top */
	aim->end[2] = aimz + target->r.maxs[2] * 0.90;
	if ( aim->mask == (trap_PointContents( aim->end, targno ) & MASK_WATER)) {
		G_Q3F_ForceFieldExtTrace(&tr, aim->origin, NULL, NULL, aim->end, ENTITYNUM_NONE, aim->realowner, MASK_SOLID );
		if (tr.fraction == 1 ) 
			return qtrue;
	}
	return qfalse;
}

static qboolean G_Q3F_SentryAimTrace( AimBlock_t * aim) {
	trace_t tr;
	float len, forward;
	float sy, cy, sp, cp;
	vec3_t lenbase;

	VectorSubtract( aim->end, aim->origin, lenbase);
	/* Determine yaw, left right */
	len = VectorLength( lenbase );
	/* Set the center angle of the turret */
	aim->angles[YAW] = atan2( lenbase[1], lenbase[0] );
	/* Add Angle between center and the barrelpoint */
	aim->angles[YAW] += asin( aim->hoffset / len );
	/* Determine pitch, up down */
	/* Add the angle of the turret if the center would be facing the point */
	forward = sqrt ( lenbase[0]*lenbase[0] + lenbase[1]*lenbase[1] );
	aim->angles[PITCH] = -atan2( lenbase[2], forward  );
	/* Angle between center and the barrelpoint */
	aim->angles[PITCH] += asin( aim->voffset / len );

	sy = sin(aim->angles[YAW]);
	cy = cos(aim->angles[YAW]);
	sp = sin(aim->angles[PITCH]);
	cp = cos(aim->angles[PITCH]);

	/* Determine start position */
	aim->start[0] = (sy * aim->hoffset) + (sp*cy * aim->voffset);
	aim->start[1] = (-cy * aim->hoffset) + (sp*sy * aim->voffset);
	aim->start[2] = cp * aim->voffset;

	VectorAdd( aim->start, aim->origin, aim->start);

	/* Save the angles */
	aim->angles[YAW] = Q_ftol(aim->angles[YAW]*(180/M_PI));
	aim->angles[PITCH] = Q_ftol(aim->angles[PITCH]*(180/M_PI));
	aim->angles[ROLL] = 0;

	G_Q3F_ForceFieldExtTrace(&tr, aim->start, NULL, NULL, aim->end, ENTITYNUM_NONE, aim->realowner, MASK_SOLID );
	return (tr.fraction == 1 ); 
}

static qboolean G_Q3F_SentryAimLock( AimBlock_t * aim, gentity_t *target ) {
	/* Aim at the normal origin */
	float aimz;
	//int targno = target->s.number;

	aimz = target->r.currentOrigin[2];
	/* Try the center */
	VectorCopy( target->r.currentOrigin, aim->end );
	if (G_Q3F_SentryAimTrace( aim )) 
			return qtrue;
	/* Try the bottom */
	aim->end[2] = aimz + target->r.mins[2] * 0.75;
	if (G_Q3F_SentryAimTrace( aim )) 
		return qtrue;
	/* Try the top */
	aim->end[2] = aimz + target->r.maxs[2] * 0.90;
	if (G_Q3F_SentryAimTrace( aim )) 
		return qtrue;
	/* Try left of origin */
	aim->end[0] = target->r.currentOrigin[0];
	aim->end[1] = target->r.currentOrigin[1] + 0.8 * target->r.mins[1];
	aim->end[2] = target->r.currentOrigin[2];
	if (G_Q3F_SentryAimTrace( aim )) 
		return qtrue;
	/* Try right of origin */
	aim->end[0] = target->r.currentOrigin[0];
	aim->end[1] = target->r.currentOrigin[1] + 0.8 * target->r.maxs[1];
	aim->end[2] = target->r.currentOrigin[2];
	if (G_Q3F_SentryAimTrace( aim )) 
		return qtrue;
	return qfalse;
}

void G_Q3F_SentryMove( gentity_t *ent )
{	trace_t tr;
	vec3_t origin;
	gentity_t *ground = &g_entities[ent->s.groundEntityNum];
	//Are we falling
	if ( ent->s.groundEntityNum == ENTITYNUM_NONE ) {
		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	} else if( ground->s.eType == ET_PLAYER ) {
		// RR2DO2 crush any players under the sentry
		G_Damage( &g_entities[ent->s.groundEntityNum], ent->parent, ent->parent, NULL, NULL, 99999, DAMAGE_NO_PROTECTION, 
			ent->s.eType == ET_Q3F_SENTRY ? MOD_CRUSHEDBYSENTRY : MOD_CRUSHEDBYSUPPLYSTATION );
		//Stay in this place for, can fall next frame
		VectorCopy( ent->r.currentOrigin, origin );
		origin[2]--;
	} else if (ground->s.eType == ET_MOVER && ground->inuse ) {
		BG_EvaluateTrajectory( &g_entities[ent->s.groundEntityNum].s.pos, level.time, origin );
		VectorAdd( origin, ent->pos1, origin );
		G_SetOrigin( ent, origin );
		origin[2]--;
	} else {
		/* We just sit still */
		G_SetOrigin( ent, ent->r.currentOrigin );
		VectorCopy( ent->r.currentOrigin, origin );
		origin[2]--;
	}
	G_Q3F_ForceFieldExtTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, ent->parent->s.number, MASK_PLAYERSOLID );
	VectorCopy( tr.endpos, ent->r.currentOrigin );
	if ( tr.startsolid ) {
		//Stop all movement and go down slowly
		ent->r.currentOrigin[2]--;
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		VectorClear( ent->s.pos.trDelta );
		ent->s.pos.trTime = level.time;
		ent->s.groundEntityNum = ENTITYNUM_NONE;
	} else if (tr.fraction != 1 ) {
		/* We've hit something, crap, determine position between us and what we hit */
		ground = &g_entities[tr.entityNum];
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		VectorClear( ent->s.pos.trDelta );
		ent->s.pos.trTime = level.time;
		if ( ground->s.eType == ET_MOVER ) {
			BG_EvaluateTrajectory( &ground->s.pos, level.time, ent->pos1 );
			VectorSubtract( ent->r.currentOrigin, ent->pos1, ent->pos1 );
		}
		ent->s.groundEntityNum = tr.entityNum;
	} else {
		/* We seem to have nothing below us */
		ent->s.groundEntityNum = ENTITYNUM_NONE;
		/* Ensure we use gravity */
		if (ent->s.pos.trType != TR_GRAVITY) {
			VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
			VectorClear( ent->s.pos.trDelta );
			ent->s.pos.trTime = level.time;
			ent->s.pos.trType = TR_GRAVITY;
		}
	}
	trap_LinkEntity( ent );

	// RR2DO2: liquid calculations
	ent->watertype = trap_PointContents( ent->r.currentOrigin, ent->s.number ); 
	ent->waterlevel = (ent->watertype & MASK_WATER) ? 3 : 0;

	if( ent->waterlevel ) {
		if (ent->watertype & CONTENTS_LAVA) {
			G_Damage (ent, NULL, NULL, NULL, NULL, 60, 0, MOD_LAVA);
		}

		if (ent->watertype & CONTENTS_SLIME) {
			G_Damage (ent, NULL, NULL, NULL, NULL, 20, 0, MOD_SLIME);
		}		
	}

	// Golliwog: Check for nodrop areas.
	if( ent->watertype & CONTENTS_NODROP )
		G_Damage( ent, NULL, NULL, NULL, NULL, 10000, DAMAGE_NO_PROTECTION, MOD_NODROP );
}

static qboolean G_Q3F_SentrySetAngle( gentity_t *ent, const vec3_t angles, float speed ) {
	vec3_t delta;
	float length, canreach;
	/* Check if we can reach this angle this frame */
	delta[YAW] = AngleNormalize180( angles[YAW] -  ent->s.apos.trBase[YAW] );
	delta[PITCH] = AngleNormalize180( angles[PITCH] - ent->s.apos.trBase[PITCH] );
	delta[ROLL] = AngleNormalize180( angles[ROLL] -  ent->s.apos.trBase[ROLL] );
	length = VectorLength(delta);
	canreach = 0.001 * speed * (level.time - level.previousTime);
	if ( length <= canreach ) {
		VectorCopy( angles, ent->s.apos.trBase );
		return qtrue;
	} else {
		canreach /= length;
		ent->s.apos.trBase[YAW] = LerpAngle( ent->s.apos.trBase[YAW], angles[YAW], canreach );
		ent->s.apos.trBase[PITCH] = LerpAngle( ent->s.apos.trBase[PITCH], angles[PITCH], canreach );
//		ent->s.apos.trBase[ROLL] = LerpAngle( ent->s.apos.trBase[ROLL], angles[ROLL], canreach );
		ent->s.apos.trBase[ROLL] = 0;
	}
	return qfalse;
}

void G_Q3F_RunSentry( gentity_t *ent )
{
	AimBlock_t aim;
	int activegun, spinleft;
	
	if(ent->sound2to1 && ent->parent)
	{
		vec3_t sentrymin, origin;
		// Set the angles.
		ent->s.angles[YAW]	= AngleNormalize360( ent->parent->client->ps.viewangles[YAW] );
		ent->s.angles[PITCH]	= 0;
		ent->s.angles[ROLL]	= 0;
		SnapVector( ent->s.angles );
		// And the current angles.
		VectorCopy( ent->s.angles, ent->s.apos.trBase );
		ent->s.apos.trType = TR_INTERPOLATE;

		// Get the sentry origin
		VectorClear( sentrymin );
		sentrymin[YAW] = ent->parent->client->ps.viewangles[YAW];
		AngleVectors( sentrymin, origin, NULL, NULL );
		VectorScale( origin, Q3F_SENTRY_BUILD_DISTANCE, origin );
		VectorAdd( ent->parent->r.currentOrigin, origin, origin );
		//origin[2] += Q3F_SENTRY_AIM_HEIGHT + bg_q3f_classlist[Q3F_CLASS_ENGINEER]->mins[2];

		// And position it
		//VectorCopy( trace.endpos, origin );
		//origin[2] -= q3f_sentry_min[2];
		G_SetOrigin( ent, origin );
		trap_LinkEntity( ent );
		return;
	}
	// If we have an enemy, and we want to fire, fire off the correct weapon.
	G_Q3F_SentryMove( ent);

	// bogus: sentries can be destroyed in move (eg drop in lava/slime)
	if(!ent->inuse)
		return;

	ent->s.torsoAnim = ent->health;		// Client-side copy for ID purposes
	ent->s.weapon = 0;					// Disable firing flag
	ent->s.powerups = ent->parent->s.powerups & (1 << PW_QUAD); 

	if( ent->last_move_time + Q3F_SENTRY_BOREDOM_TIME < level.time )
		ent->s.eFlags |= EF_Q3F_SENTRYBORED;
	else ent->s.eFlags &= ~EF_Q3F_SENTRYBORED;

	/* We still have a think, wait fill finished building */
	if ( ent->think ) {
		G_RunThink( ent );
		return;
	}
	/* Prepare aiming */
	VectorCopy ( ent->r.currentOrigin, aim.origin );
	aim.origin[2] += Q3F_SENTRY_AIM_HEIGHT;
	if (level.ceaseFire) 
		goto ceasefire_target;

	/* Do we have an enemy */
	if (ent->enemy ) {
		ent->last_move_time = level.time;
		/* Is it time for another shot */
		if ( level.time > ent->s.time2 + Q3F_SENTRY_SPINKEEP_TIME) {
			spinleft = level.time - ent->s.time2 - Q3F_SENTRY_SPINKEEP_TIME;
			if (spinleft > Q3F_SENTRY_SPINDOWN_TIME) {
				ent->s.time2 = level.time + Q3F_SENTRY_SPINUP_TIME;
				G_AddEvent( ent, EV_SENTRY_SPINUP, 0);
			} else {
				spinleft = (spinleft * Q3F_SENTRY_SPINUP_TIME) / Q3F_SENTRY_SPINDOWN_TIME;
				ent->s.time2 = level.time + spinleft;
			}
			if (ent->s.time < level.time)
				ent->s.time = level.time;			//Be Ready as soon as it's spun up
		} else if (level.time < ent->s.time2) {
			spinleft = ent->s.time2 - level.time;
			if (spinleft > Q3F_SENTRY_SPINUP_TIME) {
				ent->s.time2 = level.time + Q3F_SENTRY_SPINUP_TIME;
			}
			if (ent->s.time < level.time)
				ent->s.time = level.time;			//Be Ready as soon as it's spun up
		/* We're spun up, let's KILL! */
		} else {
			ent->s.time2 = level.time;
			if (!ent->damage && ent->s.time <= level.time ) {
				if ( ent->s.legsAnim == 1  ) {
					if (ent->s.otherEntityNum && !(ent->s.frame & 1)) {
						ent->damage = 1;
					} else {
						ent->s.time = level.time + Q3F_SENTRY_FIRE_INTERVAL;
						ent->s.frame = ( ent->s.frame + 1 ) & 63;
					}
				} else if (ent->s.legsAnim == 2) {
					if (ent->s.otherEntityNum ) {
						ent->damage = 2 + ( ent->s.frame & 1 );
					}
				} else if (ent->s.legsAnim == 3) {
					if ( ent->s.otherEntityNum && ( ent->s.frame & 31 ) ) 
						ent->damage = 4 + ( ent->s.frame & 1 );
					 else if ( ent->s.otherEntityNum2 && !(ent->s.frame & 31 ) ) {
						ent->damage = 6 + ( ent->s.frame >> 5 );
					} else {
						ent->s.time = level.time + Q3F_SENTRY_FIRE_INTERVAL;
						ent->s.frame = ( ent->s.frame + 1 ) & 63;
					}
				}
			}
		}
		switch ( ent->damage ) {
		case 0:	activegun = 0;aim.voffset =  0;aim.hoffset =  0;break;
		case 1:	activegun = 1;aim.voffset =  0;aim.hoffset =  0;break;
		case 2:	activegun = 1;aim.voffset = -4;aim.hoffset =  0;break;
		case 3:	activegun = 1;aim.voffset =  4;aim.hoffset =  0;break;
		case 4:	activegun = 1;aim.voffset = -4;aim.hoffset = -4;break;
		case 5:	activegun = 1;aim.voffset = -4;aim.hoffset =  4;break;
		case 6:	activegun = 2;aim.voffset =  4;aim.hoffset = -4;break;
		case 7:	activegun = 2;aim.voffset =  4;aim.hoffset =  4;break;
		}
		if (!G_Q3F_SentryEnemyValid( ent, ent->enemy ) 
			|| DistanceSquared( aim.origin, ent->enemy->r.currentOrigin) > (Q3F_SENTRY_MAX_RANGE * Q3F_SENTRY_MAX_RANGE)
			|| !G_Q3F_SentryAimLock( &aim, ent->enemy )) 
		{
			ent->enemy = NULL;
			ent->timestamp = level.time + Q3F_SENTRY_POSTLOCK_TIME;
			VectorCopy(ent->s.apos.trBase, ent->movedir);
			ent->splashDamage = 0;		//No idle sound
		} else if ( activegun == 1) {
			if (G_Q3F_SentrySetAngle( ent, aim.angles, Q3F_SENTRY_ROT_SPEED )) {
				G_Q3F_Sentry_Fire( ent, &aim, Q3F_SENTRY_DAMAGE );
				ent->damage = 0;
				ent->s.time = level.time + Q3F_SENTRY_FIRE_INTERVAL;
				ent->s.frame = ( ent->s.frame + 1 ) & 63;
			}
		} else if (activegun == 2) {
			if (G_Q3F_SentrySetAngle( ent, aim.angles, Q3F_SENTRY_ROT_SPEED )) {
				G_Q3F_Sentry_RocketFire( ent, &aim );
				ent->damage = 0;
			}
			ent->s.time = level.time + Q3F_SENTRY_FIRE_INTERVAL;
			ent->s.frame = ( ent->s.frame + 1 ) & 63;
		} else {
			G_Q3F_SentrySetAngle( ent, aim.angles, /* g_sentryRotSpeed.value */ 350 );
		}
		G_Q3F_UpdateEngineerStats( ent->parent );
	}

	/* Sentries can kill themselves because of their shooting */
	if(!ent->inuse)
		return;

	/* Try and find a new enemy */
	if (!ent->enemy ) {
		vec3_t dist;
		gentity_t *scan;
		float range, angle, yaw;
		float closelen = 999999;
		gentity_t *closest	= NULL;
		
		aim.mask = trap_PointContents( aim.origin, ent->s.number ) & MASK_WATER;

//		ent->s.frame = 0;			//Reset firing combination
		ent->damage = 0;
		activegun = 0;				//No firing this frame for spin up

		for( scan = level.gentities; scan < &level.gentities[level.maxclients]; scan++ ) {
		// Test each entity for 'targetability'.
			if (!G_Q3F_SentryEnemyValid( ent, scan ) )
				continue;
			VectorCopy( scan->r.currentOrigin, dist );
			VectorSubtract( dist, aim.origin, dist );
			range = VectorLength( dist );
			if( range > closelen || range > Q3F_SENTRY_MAX_RANGE )
				continue;
			// Work out the scan range - there's a falloff from MAX_RANGE directly ahead to
			// CLOSE_RANGE directly behind.
			yaw = vectoyaw( dist );
			angle = fabs( AngleSubtract( ent->s.apos.trBase[YAW], yaw ) );
			if( range > (Q3F_SENTRY_CLOSE_RANGE + (1.0f - angle / 180.0f) * (Q3F_SENTRY_MAX_RANGE - Q3F_SENTRY_CLOSE_RANGE)) )
				continue;	// Outside our scan range, keep searching.
			if( !G_Q3F_FindLock( &aim, scan ))
				continue;	// Outside PVS, keep searching
			closest = scan;
			closelen = range;
		}
		if ( closest ) {
			ent->enemy = closest;
			if (ent->s.time < level.time)
				ent->s.time = level.time;
		}
	}
	/* Still no enemy do some idling */
	if (!ent->enemy ) {
ceasefire_target:
		if ( ent->timestamp > level.time ) {
            /* Try to get closer to the correct angle */
			if (G_Q3F_SentrySetAngle( ent, ent->movedir, Q3F_SENTRY_ROT_IDLE_SPEED ) && ent->splashDamage) {
				ent->splashDamage = 0;		//No new idle sounds
				G_AddEvent( ent, EV_SENTRY_IDLESTOP, 0 );
			}
		} else {
			ent->timestamp = level.time + Q3F_SENTRY_IDLE_TIME;
			if (!(rand() & 7)) {
				ent->splashDamage = 1;
				G_AddEvent( ent, EV_SENTRY_IDLESTART, 0 );
				ent->movedir[YAW] = ent->s.angles[YAW] + Q_flrand(-1.0f, 1.0f) * 45;
				ent->movedir[PITCH] = ent->s.angles[PITCH] + Q_flrand(-1.0f, 1.0f) * 15;
			} else {
				ent->splashDamage = 0;
				VectorCopy( ent->s.angles, ent->movedir );
			}
		}
	}
	//Update angles like they are now
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );
	// RR2DO2: update sentrycam
	if( ent->target_ent ) {
		VectorCopy( g_entities[ent->s.clientNum].r.currentOrigin, ent->target_ent->r.currentOrigin );
		VectorCopy( ent->r.currentOrigin, ent->target_ent->s.origin2 );
		VectorCopy( ent->s.pos.trBase, ent->target_ent->s.pos.trBase );
		VectorCopy( ent->s.pos.trDelta, ent->target_ent->s.pos.trDelta );
		ent->target_ent->s.pos.trTime = ent->s.pos.trTime;
		ent->target_ent->s.pos.trDuration = ent->s.pos.trDuration;
		ent->target_ent->s.pos.trType = ent->s.pos.trType;
		VectorCopy( ent->s.apos.trBase, ent->target_ent->s.apos.trBase );
		VectorCopy( ent->s.apos.trDelta, ent->target_ent->s.apos.trDelta );
		ent->target_ent->s.apos.trTime = ent->s.apos.trTime;
		ent->target_ent->s.apos.trDuration = ent->s.apos.trDuration;
		ent->target_ent->s.apos.trType = ent->s.apos.trType;
		trap_LinkEntity( ent->target_ent );
	}
	return;
}


/*
**	SupplyStation handling
*/

int G_Q3F_SupplyStationMaxHealth( int supplevel )
{
	switch( supplevel )
	{
		case 1:		return( 150 );
		case 2:		return( 180 );
		case 3:		return( 220 );
		case 4:		return( 250 );
		default:	return( 0 );
	}
}

void G_Q3F_RunSupplystation( gentity_t *ent )
{
	G_Q3F_SentryMove( ent );

	ent->s.torsoAnim = ent->health;	// Client-side copy for ID purposes

	if(!ent->inuse)
		return;

	G_RunThink( ent );
}

void G_Q3F_SupplyStationDie( gentity_t *supplystation, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	// Blow it up (handy as a booby trap)

	gentity_t *parent;//, *temp;
	float explosion;
	char *attackerName;

	if( !supplystation )
		return;

	parent = supplystation->parent;

#ifdef DREVIL_BOT_SUPPORT
	if(attacker && parent)
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = attacker;
		Bot_Interface_SendEvent(ETF_MESSAGE_SS_DESTROYED, parent->s.clientNum, 0, 0, &data);
	} else
		Bot_Interface_SendEvent(ETF_MESSAGE_SS_DESTROYED, parent->s.clientNum, 0, 0, 0);
#endif

	if(parent && parent->client && parent->client->supplystation != supplystation) {
		trap_SendServerCommand(	-1, va( "print \"%s Eronious supply station. Dumping into to server log\n\"", parent->client->pers.netname ) );
	}

	if( meansOfDeath == MOD_LAVA ) {
		attackerName = "MOD_LAVA";
		trap_SendServerCommand(	-1, va( parent->client->pers.gender == GENDER_MALE ?	"print \"%s ^7sacrificed his supply station to the mighty lavagod.\n\"" : 
																						"print \"%s ^7sacrificed her supply station to the mighty lavagod.\n\"", parent->client->pers.netname ) );
	} else if( meansOfDeath == MOD_SLIME ) {
		attackerName = "MOD_SLIME";
		trap_SendServerCommand(	-1, va( "print \"%s^7's supply station got an acid dip.\n\"", parent->client->pers.netname ) );
	}
	else if( parent == attacker ) {
		attackerName = parent->client->pers.netname;
		trap_SendServerCommand(	-1, va( parent->client->pers.gender == GENDER_MALE ?	"print \"%s ^7has destroyed his supply station.\n\"" :
																						"print \"%s ^7has destroyed her supply station.\n\"", attackerName ) );
	}
	else if( attacker ) {
		qboolean isally = G_Q3F_IsAllied( attacker, parent ) ? qtrue : qfalse;
		attackerName = attacker->client ? attacker->client->pers.netname : "somebody";
		trap_SendServerCommand(	-1, va( "print \"%s^7's supply station has been destroyed by %s%s^7!\n\"", parent->client->pers.netname, (isally ? "^sally^7 " : ""), attackerName ) );
	} else {
		attackerName = "Unknown";
		trap_SendServerCommand(	-1, va( "print \"%s^7's supply station has been destroyed by an unknown entity!\n\"",
								parent->client->pers.netname ) );
	}

#ifdef BUILD_BOTS
	Bot_Event_DispenserDestroyed( parent, attacker );
#endif

	G_LogPrintf(	"Destroy: supplystation %i %i: %s^7 destroyed %s^7's supplystation.\n", 
					attacker->s.number, parent->s.number, attackerName, parent->client->pers.netname );

	explosion =	supplystation->s.origin2[0] / 2 + 
				supplystation->s.origin2[1] / 3 +
				supplystation->s.origin2[2] +
				supplystation->s.angles2[0] / 3;
	if( explosion > 260 )
		explosion = 260; // was 300
	/*if( explosion > g_supplyStationMaxDamage.value )
		explosion = g_supplyStationMaxDamage.value;*/

	/*temp = */(void)G_TempEntity( supplystation->r.currentOrigin, EV_ETF_SUPPLYSTATION_EXPLOSION );
	supplystation->takedamage = qfalse;		// Stop infinite damage loops against other supplystations :)
	G_RadiusDamage(	supplystation->r.currentOrigin, supplystation, supplystation->parent,
					explosion, supplystation, MOD_SUPPLYSTATION_EXPLODE, 0 );

	if( parent && parent->client && parent->client->supplystation == supplystation )
	{
		parent->client->supplystation = NULL;
		G_Q3F_UpdateEngineerStats( parent );
	}

	G_FreeEntity( supplystation );
}

void G_Q3F_SupplyStationPain( gentity_t *ent, gentity_t *attacker, int damage )
{
	// Simply update stats

	G_Q3F_UpdateEngineerStats( ent->parent );

	// set health
	ent->s.otherEntityNum = ent->health;

#ifdef BUILD_BOTS
	Bot_Event_BuildableDamaged( ent->parent, ET_Q3F_SUPPLYSTATION, ent );
	Bot_Event_SendSupplyStatsToBot( ent );
#endif
	//BotSendDispenserStatus(ent);
}

void G_Q3F_SupplyStationTouch( gentity_t *supplystation, gentity_t *other, trace_t *tr )
{
	// Someone is touching the supplystation, supply something.

	if( other->health <= 0 || !other->client )
		return;

	supplystation->s.frame = qtrue;	// RR2: it really is an int but let's just hijack it :)

	if( other->client->lastSupplystationTouchTime < level.time )
	{
		/*trap_SendServerCommand( other->s.number, va( "menu supply %d %d %d %d %d %d",
								supplystation->s.number, (int) supplystation->s.origin2[0],
								(int) supplystation->s.origin2[1], (int) supplystation->s.origin2[2],
								(int) supplystation->s.angles2[0], (int) supplystation->s.angles2[1] ) );*/
		trap_SendServerCommand( other->s.number, va( "menu supply %d", supplystation->s.number ) );

		if( !G_Q3F_IsAllied( supplystation->parent, other ) ) {
			trap_SendServerCommand( supplystation->parent->s.number, "enesup \"Enemies are using your supply station!\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_DispenserEnemyUsed( supplystation->parent, other );
			Bot_Event_SendSupplyStatsToBot( supplystation );
			#endif
#ifdef DREVIL_BOT_SUPPORT
			if(supplystation->parent)
			{
				BotUserData data;
				data.m_DataType = dtEntity;
				data.udata.m_Entity = other;
				Bot_Interface_SendEvent(ETF_MESSAGE_SS_TOUCHED, supplystation->parent->s.number, 0, 0, &data);
				BotSendDispenserStatus(supplystation);
			}			
#endif
		} else {
			trap_SendServerCommand( supplystation->parent->s.number, "frisup" );
		}

		other->client->lastSupplystationTouchTime = level.time + 500;

		// operating animation
		if( !(other->client->ps.extFlags & EXTF_ANI_OPERATING) ) {
			other->client->ps.extFlags |= EXTF_ANI_OPERATING;
			other->client->torsoanimEndTime = level.time + Q3F_OPERATE_ANIM_DURATION;
		}
	}
}

static QINLINE int G_Q3F_SupplyRegenDivisor(int supplevel)
{
	return (supplevel != 99 && supplevel >= 2) ? 10 : 20;
}

static QINLINE int G_Q3F_SupplyRegenTime(int supplevel)
{
	return supplevel == 3 ? 6000 : Q3F_SUPPLYSTATION_REGEN_TIME;
}

void G_Q3F_SupplyStationRegen( gentity_t *supplystation )
{
	// Time to increase ammo?
	if( supplystation->timestamp < level.time ) {

		// Increase the amount of ammo inside
		int divisor = G_Q3F_SupplyRegenDivisor(supplystation->s.legsAnim);

		supplystation->s.origin2[0]	+= Q3F_SUPPLYSTATION_SHELLS / divisor;
		supplystation->s.origin2[1]	+= Q3F_SUPPLYSTATION_NAILS / divisor;
		supplystation->s.origin2[2]	+= Q3F_SUPPLYSTATION_ROCKETS / divisor;
		supplystation->s.angles2[0]	+= Q3F_SUPPLYSTATION_CELLS / divisor;
		supplystation->s.angles2[1]	+= Q3F_SUPPLYSTATION_ARMOUR / divisor;

		if( supplystation->s.origin2[0] > Q3F_SUPPLYSTATION_SHELLS )
			supplystation->s.origin2[0] = Q3F_SUPPLYSTATION_SHELLS;
		if( supplystation->s.origin2[1] > Q3F_SUPPLYSTATION_NAILS )
			supplystation->s.origin2[1] = Q3F_SUPPLYSTATION_NAILS;
		if( supplystation->s.origin2[2] > Q3F_SUPPLYSTATION_ROCKETS )
			supplystation->s.origin2[2] = Q3F_SUPPLYSTATION_ROCKETS;
		if( supplystation->s.angles2[0] > Q3F_SUPPLYSTATION_CELLS )
			supplystation->s.angles2[0] = Q3F_SUPPLYSTATION_CELLS;
		if( supplystation->s.angles2[1] > Q3F_SUPPLYSTATION_ARMOUR )
			supplystation->s.angles2[1] = Q3F_SUPPLYSTATION_ARMOUR;

		supplystation->timestamp = level.time + G_Q3F_SupplyRegenTime(supplystation->s.legsAnim); //Q3F_SUPPLYSTATION_REGEN_TIME;

#ifdef BUILD_BOTS
		Bot_Event_SendSupplyStatsToBot( supplystation );
#endif
#ifdef DREVIL_BOT_SUPPORT
		BotSendDispenserStatus(supplystation);
#endif
	}

	if( supplystation->s.legsAnim != 3)
		return;

	if( supplystation->timestamp2 < level.time ) {
		supplystation->s.angles2[2] += 1;

		if( supplystation->s.angles2[2] > 2 )
			supplystation->s.angles2[2] = 2;

		supplystation->timestamp2 = level.time + (60000); // 1 minute
	}
}

void G_Q3F_SupplyStationThink( gentity_t *supplystation )
{
	// see if we should retract our screen
	if( supplystation->s.frame ) {
		// scan for players within 80 units around us
		int i, num;
		int touch[MAX_GENTITIES];
		vec3_t mins, maxs;
		gentity_t *check;

		VectorSet( mins, -46, -46, -46 );
		VectorSet( maxs, 46, 46, 46 );		

		VectorAdd( supplystation->r.currentOrigin, mins, mins );
		VectorAdd( supplystation->r.currentOrigin, maxs, maxs );
		num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		for( i = 0; i < num; i++ ) {
			check = &g_entities[touch[i]];

			if ( check->client) {
				break;
			}
		}

		// nobody found, retract screen
		if( i == num ) {
			supplystation->s.frame = qfalse;
		}
	}

	// Finalize building?
	if( !supplystation->s.legsAnim )
	{
		supplystation->s.legsAnim	= 1;
		supplystation->health		= G_Q3F_SupplyStationMaxHealth( 1 );
		supplystation->touch		= G_Q3F_SupplyStationTouch;
		supplystation->takedamage	= qtrue;
		G_Q3F_UpdateEngineerStats( supplystation->parent );
		supplystation->parent->client->ps.ammo[AMMO_CELLS] -= Q3F_SUPPLYSTATION_BUILD_CELLS;
		if( supplystation->parent->client->ps.ammo[AMMO_CELLS] < 0 )
			supplystation->parent->client->ps.ammo[AMMO_CELLS] = 0;

		// set health
		supplystation->s.otherEntityNum = supplystation->health;

#ifdef BUILD_BOTS
		Bot_Event_DispenserBuilt( supplystation->parent, supplystation );
		Bot_Event_SendSupplyStatsToBot( supplystation );
#endif
#ifdef DREVIL_BOT_SUPPORT
		if(supplystation->parent)
		{
			BotUserData data;
			data.m_DataType = dtEntity;
			data.udata.m_Entity = supplystation;
			Bot_Interface_SendEvent(ETF_MESSAGE_SS_BUILT, supplystation->parent->s.number, 0, 0, &data);
			BotSendDispenserStatus(supplystation);
		}		
#endif
	}

	G_Q3F_SupplyStationRegen( supplystation );

	supplystation->nextthink = level.time + FRAMETIME;
}

gentity_t *G_Q3F_CheckSupplyStation( gentity_t *player, int suppnum )
{
	gentity_t *supplystation;
	vec3_t vec;

	supplystation = suppnum ? &g_entities[suppnum] : player->client->repairEnt;
	if( !supplystation || !supplystation->inuse || supplystation->s.eType != ET_Q3F_SUPPLYSTATION || supplystation->health <= 0 )
		return( NULL );		// Not a supplystation
	if( !supplystation->s.legsAnim )
		return( NULL );		// Not yet built
//	if( supplystation != player->client->repairEnt || player->client->repairTime < level.time + Q3F_BUILDING_REPAIR_TIME )
//		return( NULL );
	VectorCopy( player->client->ps.origin, vec );
	vec[2] += player->client->ps.viewheight;
	VectorSubtract( vec, supplystation->r.currentOrigin, vec );
	if( sqrt( vec[0]*vec[0] + vec[1]*vec[1] ) > Q3F_BUILDING_REPAIR_DISTANCE ||
		player->r.absmin[2] - supplystation->r.absmax[2] > 0.5 * Q3F_BUILDING_REPAIR_DISTANCE || 
		supplystation->r.absmin[2] - player->r.absmax[2] > 0.5 * Q3F_BUILDING_REPAIR_DISTANCE )
		return( NULL );
	return( supplystation );

	/*	if( suppnum )
	{
		supplystation = &g_entities[suppnum];
		if( !supplystation->inuse || strcmp( supplystation->classname, "supplystation" ) || supplystation->health <= 0 )
			return( NULL );		// Not a supplystation
		if( !supplystation->s.legsAnim )
			return( NULL );		// Not yet built
		if( Distance( player->r.currentOrigin, supplystation->r.currentOrigin ) > 1.5 * Q3F_SENTRY_BUILD_DISTANCE )
			return( NULL );		// Out of range
		return( supplystation );
	}
	else {
		for( supplystation = &g_entities[MAX_CLIENTS]; supplystation < (g_entities + level.num_entities); supplystation++ )
		{
			if( !supplystation->inuse || strcmp( supplystation->classname, "supplystation" ) || supplystation->health <= 0 )
				continue;		// Not a supplystation
			if( !supplystation->s.legsAnim )
				continue;		// Not yet built
			if( Distance( player->r.currentOrigin, supplystation->r.currentOrigin ) > 1.5 * Q3F_SENTRY_BUILD_DISTANCE )
				continue;		// Out of range
			return( supplystation );
		}
		return( NULL );
	}*/
}

void G_Q3F_Supply_Command( gentity_t *player )
{
	// User has asked for something from a supplystation. If they didn't
	// specify a supplystation, find the nearest

	char buff[16];
	int suppnum, count;
	gentity_t *supplystation;
	bg_q3f_playerclass_t *cls;
#ifdef BUILD_BOTS
	qboolean gotammo = qfalse;
#endif

	trap_Argv( 2, buff, sizeof(buff) );
	suppnum = atoi( buff );

	supplystation = G_Q3F_CheckSupplyStation( player, suppnum );
	if( !supplystation )
		return;

	trap_Argv( 1, buff, sizeof(buff) );

	cls = BG_Q3F_GetClass( &player->client->ps );
	if( !Q_stricmp( buff, "ammo" ) )
	{
		if ( player->client->ps.ammo[AMMO_SHELLS] < cls->maxammo_shells)
		{
			count = cls->maxammo_shells - player->client->ps.ammo[AMMO_SHELLS];
			if( count > supplystation->s.origin2[0] )
				count = (int) supplystation->s.origin2[0];
			player->client->ps.ammo[AMMO_SHELLS] += count;
			supplystation->s.origin2[0] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		if ( player->client->ps.ammo[AMMO_NAILS] < cls->maxammo_nails)
		{
			count = cls->maxammo_nails - player->client->ps.ammo[AMMO_NAILS];
			if( count > supplystation->s.origin2[1] )
				count = (int) supplystation->s.origin2[1];
			player->client->ps.ammo[AMMO_NAILS] += count;
			supplystation->s.origin2[1] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		if ( player->client->ps.ammo[AMMO_ROCKETS] < cls->maxammo_rockets)
		{
			count = cls->maxammo_rockets - player->client->ps.ammo[AMMO_ROCKETS];
			if( count > supplystation->s.origin2[2] )
				count = (int) supplystation->s.origin2[2];
			player->client->ps.ammo[AMMO_ROCKETS] += count;
			supplystation->s.origin2[2] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		if ( player->client->ps.ammo[AMMO_CELLS] < cls->maxammo_cells)
		{
			count = cls->maxammo_cells - player->client->ps.ammo[AMMO_CELLS];
			if( count > supplystation->s.angles2[0] )
				count = (int) supplystation->s.angles2[0];
			player->client->ps.ammo[AMMO_CELLS] += count;
			supplystation->s.angles2[0] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		player->client->repairEnt = NULL;

#ifdef BUILD_BOTS
		if(gotammo)
			Bot_Event_GotDispenserAmmo( player );
#endif
	}
	else if( !Q_stricmp( buff, "armor" ) )
	{
		if ( player->client->ps.stats[STAT_ARMOR] < cls->maxarmour)
		{
			count = cls->maxarmour - player->client->ps.stats[STAT_ARMOR];
			if( count > supplystation->s.angles2[1] )
				count = (int) supplystation->s.angles2[1];
			player->client->ps.stats[STAT_ARMOR] += count;
			supplystation->s.angles2[1] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		player->client->repairEnt = NULL;

#ifdef BUILD_BOTS
		if(gotammo)
			Bot_Event_GotDispenserAmmo( player );
#endif
	}
	else if( !Q_stricmp( buff, "grenade" ) )
	{
		if ( (player->client->ps.ammo[AMMO_GRENADES] & 0xFF) < cls->gren1max)
		{
			count = 1;
			if( count > supplystation->s.angles2[2] )
				count = (int) supplystation->s.angles2[2];
			player->client->ps.ammo[AMMO_GRENADES] = 
				(player->client->ps.ammo[AMMO_GRENADES] & 0xFF00) + 
				(player->client->ps.ammo[AMMO_GRENADES] & 0xFF) + count;
			//player->client->ps.stats[STAT_ARMOR] += count;
			supplystation->s.angles2[2] -= count;
		}
		else
			count = 0;

#ifdef BUILD_BOTS
		if( count > 0 )
			gotammo = qtrue;
#endif

		player->client->repairEnt = NULL;

#ifdef BUILD_BOTS
		if(gotammo)
			Bot_Event_GotDispenserAmmo( player );
#endif
	}
	else {
		trap_SendServerCommand( player->s.number, "print \"Usage: supply ammo|armor|grenade\n\"" );
	}
}

void G_Q3F_SupplyStationUpgrade( gentity_t *player, int suppnum )
{
	// Upgrade the specified supplystation (or else pick one in range)

	gentity_t *supplystation;

	supplystation = G_Q3F_CheckSupplyStation( player, suppnum );
	if( !supplystation || !supplystation->s.legsAnim || supplystation->s.legsAnim >= 3 )
		return;

	if( player->client->ps.ammo[AMMO_CELLS] < Q3F_SUPPLYSTATION_BUILD_CELLS )
	{
		trap_SendServerCommand( player->s.number, va( "print \"You need %d cells to upgrade your supply station.\n\"", Q3F_SUPPLYSTATION_BUILD_CELLS ) );
		return;
	}

	supplystation->s.legsAnim++;
	supplystation->timestamp = level.time + G_Q3F_SupplyRegenTime(supplystation->s.legsAnim);
	if(supplystation->s.legsAnim == 3)
		supplystation->timestamp2 = level.time + 60000;
	player->client->ps.ammo[AMMO_CELLS] -= Q3F_SUPPLYSTATION_BUILD_CELLS;
	
	supplystation->health = G_Q3F_SupplyStationMaxHealth( supplystation->s.legsAnim );

#ifdef BUILD_BOTS
	Bot_Event_SendSupplyStatsToBot( supplystation );
	//Bot_Event_SupplyStationUpgraded( sentry->parent, sentry->s.legsAnim );
#endif

	//G_AddEvent( sentry, EV_GENERAL_SOUND, sentry->soundPos2 );

	if( supplystation->parent == player )
		trap_SendServerCommand( player->s.number, va( "print \"You have upgraded your supply station to level %d\n\"", supplystation->s.legsAnim ) );
	else if( supplystation->parent && supplystation->parent->client )
		trap_SendServerCommand( player->s.number, va( "print \"You have upgraded %s^7's supply station to level %d\n\"", supplystation->parent->client->pers.netname, supplystation->s.legsAnim ) );
	G_Q3F_UpdateEngineerStats( supplystation->parent );

	player->client->repairEnt = NULL;
}

void G_Q3F_SupplyStationRepair( gentity_t *player, int suppnum )
{
	// Repair the specified supplystation (or else pick one in range)

	gentity_t *supplystation;
	int cells;

	supplystation = G_Q3F_CheckSupplyStation( player, suppnum );
	if( !supplystation || !supplystation->s.legsAnim )
		return;

	cells = (G_Q3F_SupplyStationMaxHealth( supplystation->s.legsAnim ) + 4 - supplystation->health) / 5;
	if( cells > player->client->ps.ammo[AMMO_CELLS] )
		cells = player->client->ps.ammo[AMMO_CELLS];
	player->client->ps.ammo[AMMO_CELLS] -= cells;
	supplystation->health += cells * 5;
	if( supplystation->health > G_Q3F_SupplyStationMaxHealth( supplystation->s.legsAnim ) )
		supplystation->health = G_Q3F_SupplyStationMaxHealth( supplystation->s.legsAnim );

	G_Q3F_UpdateEngineerStats( supplystation->parent );
	player->client->repairEnt = NULL;

	// set health
	supplystation->s.otherEntityNum = supplystation->health;

#ifdef BUILD_BOTS
	Bot_Event_SendSupplyStatsToBot( supplystation );
#endif
#ifdef DREVIL_BOT_SUPPORT
	if(supplystation->parent)
	{		
		BotSendDispenserStatus(supplystation);
	}	
#endif
}

void G_Q3F_SupplyStationRefill( gentity_t *player, int suppnum )
{
	// Refill the specified supplystation (or else pick one in range)
	// We give 40 shells / 20 rockets refill

	gentity_t *supplystation;
	int ammo, available;

	supplystation = G_Q3F_CheckSupplyStation( player, suppnum );
	if( !supplystation || !supplystation->s.legsAnim )
		return;

	ammo = Q3F_SUPPLYSTATION_SHELLS - supplystation->s.origin2[0];
	available = player->client->ps.ammo[AMMO_SHELLS] - Q3F_GetClipValue( WP_SUPERSHOTGUN, &player->client->ps );
	if( ammo > available )
		ammo = available;
	if( ammo > Q3F_SUPPLYSTATION_SHELLS )
		ammo = Q3F_SUPPLYSTATION_SHELLS;
	supplystation->s.origin2[0] += ammo;
	if( supplystation->s.origin2[0] > Q3F_SUPPLYSTATION_SHELLS )
		supplystation->s.origin2[0] = Q3F_SUPPLYSTATION_SHELLS;
	player->client->ps.ammo[AMMO_SHELLS] -= ammo;

	ammo = Q3F_SUPPLYSTATION_NAILS - supplystation->s.origin2[1];
	available = player->client->ps.ammo[AMMO_NAILS];
	if( ammo > available )
		ammo = available;
	if( ammo > Q3F_SUPPLYSTATION_NAILS )
		ammo = Q3F_SUPPLYSTATION_NAILS;
	supplystation->s.origin2[1] += ammo;
	if( supplystation->s.origin2[1] > Q3F_SUPPLYSTATION_NAILS )
		supplystation->s.origin2[1] = Q3F_SUPPLYSTATION_NAILS;
	player->client->ps.ammo[AMMO_NAILS] -= ammo;

	ammo = Q3F_SUPPLYSTATION_ROCKETS - supplystation->s.origin2[2];
	if( ammo > player->client->ps.ammo[AMMO_ROCKETS] )
		ammo = player->client->ps.ammo[AMMO_ROCKETS];
	if( ammo > Q3F_SUPPLYSTATION_ROCKETS )
		ammo = Q3F_SUPPLYSTATION_ROCKETS;
	supplystation->s.origin2[2] += ammo;
	if( supplystation->s.origin2[2] > Q3F_SUPPLYSTATION_ROCKETS )
		supplystation->s.origin2[2] = Q3F_SUPPLYSTATION_ROCKETS;
	player->client->ps.ammo[AMMO_ROCKETS] -= ammo;

	ammo = Q3F_SUPPLYSTATION_CELLS - supplystation->s.angles2[0];
	if( ammo > player->client->ps.ammo[AMMO_CELLS] )
		ammo = player->client->ps.ammo[AMMO_CELLS];
	if( ammo > Q3F_SUPPLYSTATION_CELLS )
		ammo = Q3F_SUPPLYSTATION_CELLS;
	supplystation->s.angles2[0] += ammo;
	if( supplystation->s.angles2[0] > Q3F_SUPPLYSTATION_CELLS )
		supplystation->s.angles2[0] = Q3F_SUPPLYSTATION_CELLS;
	player->client->ps.ammo[AMMO_CELLS] -= ammo;

#ifdef BUILD_BOTS
	Bot_Event_SendSupplyStatsToBot( supplystation );
#endif
#ifdef DREVIL_BOT_SUPPORT
	if(supplystation->parent)
	{
		BotSendDispenserStatus(supplystation);
	}
#endif
}

void G_Q3F_SupplyStationBuild( gentity_t *ent )
{
	// Check a supplystation can be built, then start building.

	vec3_t origin, tracestart, traceend, supplystationmin;
	trace_t trace;
	int traceoffset, index;
	gentity_t *supplystation;
	bg_q3f_playerclass_t *cls;

	if( !ent->client || ent->client->ps.stats[STAT_HEALTH] <= 0 )
		return;
	if( level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )
		return;
	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
	{
		trap_SendServerCommand( ent->s.number, "print \"Only engineers can build supply stations.\n\"" );
		return;
	}
	if( ent->client->supplystation )
	{
		if( ent->client->supplystation->s.legsAnim ) {
			trap_SendServerCommand( ent->s.number, "print \"You can only build one supply station at a time.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_AlreadyBuilt( ent, ET_Q3F_SUPPLYSTATION );
			#endif
		}
		else {
			trap_SendServerCommand( ent->s.number, "print \"You have stopped building.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_BuildCancelled( ent, ET_Q3F_SUPPLYSTATION );
			#endif
			ent->client->ps.ammo[AMMO_CELLS] += Q3F_SUPPLYSTATION_BUILD_CELLS;
			cls = BG_Q3F_GetClass( &ent->client->ps );
			if( ent->client->ps.ammo[AMMO_CELLS] > cls->maxammo_cells )
				ent->client->ps.ammo[AMMO_CELLS] = cls->maxammo_cells;
		}
		return;
	}
	if( ent->client->ps.ammo[AMMO_CELLS] < Q3F_SUPPLYSTATION_BUILD_CELLS )
	{
		trap_SendServerCommand( ent->s.number, va( "print \"You need at least %d cells to build a supply station.\n\"", Q3F_SUPPLYSTATION_BUILD_CELLS ) );
		#ifdef BUILD_BOTS
		Bot_Event_Build_NotEnoughAmmo( ent, ET_Q3F_SUPPLYSTATION );
		#endif
		return;
	}

		// Get the supplystation origin
	memset( supplystationmin, 0, sizeof(vec3_t) );
	supplystationmin[YAW] = ent->client->ps.viewangles[YAW];
	AngleVectors( supplystationmin, origin, NULL, NULL );
	VectorScale( origin, Q3F_SENTRY_BUILD_DISTANCE, origin );
	VectorAdd( ent->r.currentOrigin, origin, origin );
	origin[2] += (q3f_supplystation_max[2] + q3f_supplystation_min[2]) / 2;

	// Try to check the engineer can actually 'reach' that area.
	G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
	if( trace.startsolid || trace.fraction < 1 )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build a supply station here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SUPPLYSTATION );
		#endif
		return;						// Something got in the way
	}
	origin[2] -= (q3f_supplystation_max[2] + q3f_supplystation_min[2]) / 2;

		// Work out the start and end points (so we can set the sentry on the ground)
	for( index = 0; index < 3; index++ )
	{
		switch( index )
		{
			case 0: traceoffset = 32;	break;
			case 1: traceoffset = 0.25;	break;
			case 2:	traceoffset	= 48;	break;
		}
		VectorCopy( origin, tracestart );
		tracestart[2] += traceoffset + q3f_supplystation_min[2];
		VectorCopy( origin, traceend );
		traceend[2] += traceoffset - 64 + q3f_supplystation_min[2];

		G_Q3F_ForceFieldTrace( &trace, tracestart, q3f_supplystation_min, q3f_supplystation_max, traceend, ent->s.number, MASK_PLAYERSOLID );
		if( trace.fraction == 1 || (trace.entityNum < MAX_CLIENTS ))	// We never hit land, or landed on a player
			continue;

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldTrace( &trace, origin, q3f_supplystation_min, q3f_supplystation_max, origin, ENTITYNUM_NONE, MASK_PLAYERSOLID );
		if( ( trace.startsolid || trace.fraction < 1 ) && trace.entityNum != ent->s.number )
			continue;						// Something got in the way

		VectorCopy( trace.endpos, origin );
		G_Q3F_ForceFieldTrace( &trace, ent->r.currentOrigin, NULL, NULL, origin, ent->s.number, MASK_PLAYERSOLID );
		if( trace.startsolid || trace.fraction < 1 )				// We're no longer visible to the engineer
			continue;

		break;
	}
	if( index > 2 )
	{
		// We failed. Give up.
		trap_SendServerCommand( ent->s.number, "print \"You cannot build a supply station here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SUPPLYSTATION );
		#endif
		return;
	}
	VectorAdd( trace.endpos, q3f_supplystation_min, tracestart );
	VectorAdd( trace.endpos, q3f_supplystation_max, traceend );
	if( G_Q3F_NoBuildCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOBUILD_SUPPLYSTATION ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build a supply station here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SUPPLYSTATION );
		#endif
		return;
	}

	if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->client->sess.sessionTeam, Q3F_NOANNOY_BUILDING ) )
	{
		trap_SendServerCommand( ent->s.number, "print \"You cannot build an supply station here.\n\"" );
		#ifdef BUILD_BOTS
		Bot_Event_Build_CantBuild( ent, ET_Q3F_SUPPLYSTATION );
		#endif
		return;
	}

	// Now we have a position, initialise it.
	supplystation = G_Spawn();
	supplystation->classname	= "supplystation";
	supplystation->s.eType		= ET_Q3F_SUPPLYSTATION;
	supplystation->s.legsAnim	= 0;					// Sentry level (0 means building)
	supplystation->s.time		= level.time;			// Time building started.
	// Set the angles.
	supplystation->s.angles[YAW]	= AngleNormalize360( ent->client->ps.viewangles[YAW] - 180 );
	supplystation->s.angles[PITCH]	= 0;
	supplystation->s.angles[ROLL]	= 0;
	SnapVector( supplystation->s.angles );
	// And the current angles.
	VectorCopy( supplystation->s.angles, supplystation->s.apos.trBase );
	supplystation->s.apos.trType = TR_STATIONARY;
	// And the think.
	supplystation->die			= G_Q3F_SupplyStationDie;
	supplystation->pain			= G_Q3F_SupplyStationPain;
	supplystation->think		= G_Q3F_SupplyStationThink;
	supplystation->nextthink	= level.time + Q3F_SUPPLYSTATION_BUILD_TIME;
	supplystation->timestamp	= level.time + G_Q3F_SupplyRegenTime(1);
	supplystation->timestamp2	= level.time + 60000;

	// And position it
	VectorCopy( trace.endpos, origin );
	origin[2] -= q3f_supplystation_min[2];
	G_SetOrigin( supplystation, origin );
	supplystation->s.pos.trType = TR_GRAVITY;
	supplystation->s.pos.trTime = level.time;
	supplystation->s.groundEntityNum = ENTITYNUM_NONE;

	supplystation->parent = ent;
	supplystation->r.ownerNum = ENTITYNUM_NONE;	// Nobody 'owns' this as far as physics are concerned
	supplystation->s.clientNum = ent->s.number;	// But we still need to know for ID purposes
	VectorCopy( q3f_supplystation_min, supplystation->r.mins );
	VectorCopy( q3f_supplystation_max, supplystation->r.maxs );
	supplystation->clipmask = CONTENTS_SOLID|CONTENTS_PLAYERCLIP;//|CONTENTS_FORCEFIELD;
	supplystation->r.contents = CONTENTS_BODY|CONTENTS_PLAYERCLIP;//|CONTENTS_FORCEFIELD;
	supplystation->physicsObject = qtrue;
	trap_LinkEntity( supplystation );

	supplystation->sound1to2 = -1;						// Engineers wishing to dismantle

	supplystation->s.frame = qfalse;	// not in use atm

	ent->client->supplystation = supplystation;
	ent->client->ps.stats[STAT_Q3F_FLAGS] |= (1 << FL_Q3F_BUILDING);
	ent->client->buildTime = level.time + Q3F_SUPPLYSTATION_BUILD_TIME;

	G_AddEvent( supplystation, EV_SUPPLY_BUILD, 0);

#ifdef BUILD_BOTS
	Bot_Event_DispenserBuilding( ent, supplystation );
#endif

#ifdef DREVIL_BOT_SUPPORT
	if(supplystation->parent)
	{
		BotUserData data;
		data.m_DataType = dtEntity;
		data.udata.m_Entity = supplystation;
		Bot_Interface_SendEvent(ETF_MESSAGE_SS_BUILDING, supplystation->parent->s.number, 0, 0, &data);
	}
#endif

	trap_SendServerCommand( ent->s.number, "print \"Building supply station...\n\"" );
}

qboolean G_Q3F_SupplyStationCancel( gentity_t *supplystation )
{
	if(	supplystation && !supplystation->s.legsAnim && supplystation->s.eType == ET_Q3F_SUPPLYSTATION &&
		supplystation->parent->client->supplystation == supplystation )
	{
		#ifdef BUILD_BOTS
		Bot_Event_BuildCancelled( supplystation->parent, ET_Q3F_SUPPLYSTATION );
		#endif
		supplystation->parent->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_BUILDING);
		supplystation->parent->client->buildTime = 0;
		supplystation->parent->client->buildDelayTime = level.time + Q3F_SENTRY_CANCEL_MINTIME;

		supplystation->parent->client->supplystation = NULL;

		G_Q3F_UpdateEngineerStats( supplystation->parent );
		supplystation->parent->client->repairEnt = NULL;

		G_FreeEntity(supplystation);

		return( qtrue );

	}
	return( qfalse );
}

void G_Q3F_SupplyStationDismantle( gentity_t *player, int suppnum )
{
	// Dismantle the specified supplystation and reclaim cells

	gentity_t *supplystation;
	int cells;
	bg_q3f_playerclass_t *cls;

	supplystation = G_Q3F_CheckSupplyStation( player, suppnum );
	if( !supplystation || !supplystation->s.legsAnim )
		return;
	if( !G_Q3F_AttemptDismantle( player, supplystation ) )
	{
		trap_SendServerCommand( player->s.number, "print \"Two engineers are required to dismantle this.\n\"" );
		return;
	}

	cls = BG_Q3F_GetClass( &player->client->ps );

	cells = Q3F_SUPPLYSTATION_BUILD_CELLS / 2;
	if( player->client->ps.ammo[AMMO_CELLS] > cls->maxammo_cells )	// Ignore if they're 'overcelled'
		cells = 0;
	else if( (player->client->ps.ammo[AMMO_CELLS] + cells) > cls->maxammo_cells )
		cells = cls->maxammo_cells - player->client->ps.ammo[AMMO_CELLS];

#ifdef BUILD_BOTS
	Bot_Event_DispenserDismantled( supplystation->parent );
#endif

	if( supplystation->parent == player )
		trap_SendServerCommand( player->s.number, va( "print \"You have dismantled your supply station, and recovered %d cells.\n\"", cells ) );
	else {
		trap_SendServerCommand( player->s.number, va( "print \"You have dismantled %s^7's supply station, and recovered %d cells.\n\"", supplystation->parent->client->pers.netname, cells ) );
		trap_SendServerCommand( player->s.number, va( "print \"%s^7 has dismantled your supply station!\n\"", player->client->pers.netname ) );
	}
	player->client->ps.ammo[AMMO_CELLS] += cells;

	supplystation->parent->client->supplystation = NULL;
	G_Q3F_UpdateEngineerStats( supplystation->parent );
	G_LogPrintf(	"Dismantle: supplystation %d %d: %s dismantled %s's supplystation.\n",
					supplystation->parent->s.number, player->s.number,
					player->client->pers.netname,
					supplystation->parent->client->pers.netname );
	G_FreeEntity( supplystation );
}

/*
**	The engineer commands
*/


void G_Q3F_EngineerBuild_Command( gentity_t *ent )
{
	// Build a sentry or supplystation, or cancel a build

	char cmdbuff[64], anglebuff[8];
	int id;
	float angle;

	if( !ent->client || ent->health <= 0 )
		return;

	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
	{
		trap_SendServerCommand( ent->s.number, "print \"Only engineers can build autosentries and supply stations.\n\"" );
		return;
	}
//	trap_SendServerCommand( ent->s.number, "menu cancel build" );

	trap_Argv( 1, cmdbuff, sizeof(cmdbuff) );
	if( !Q_stricmp( "autosentry", cmdbuff ) )
	{
		if( !ent->client->buildTime && ent->client->buildDelayTime <= level.time )
			G_Q3F_SentryBuild( ent );
		return;
	}
	if( !Q_stricmp( "supplystation", cmdbuff ) )
	{
		if( !ent->client->buildTime && ent->client->buildDelayTime <= level.time )
			G_Q3F_SupplyStationBuild( ent );
		return;
	}
	if( !Q_stricmp( "cancel", cmdbuff ) )
	{
		if( ent->client->sentry && G_Q3F_SentryCancel( ent->client->sentry ) )
			trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
		else if( ent->client->supplystation && G_Q3F_SupplyStationCancel( ent->client->supplystation ) )
			trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
		return;
	}
	if( !Q_stricmp( "menu", cmdbuff ) )
	{
		G_Q3F_UpdateEngineerStats( ent );		// In case it's become inaccurate during respawn
		trap_SendServerCommand( ent->s.number, "menu build" );
		return;
	}
	/*if( !Q_stricmp( "upgrade", cmdbuff ) )
	{
		if( ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
			return;
		trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryUpgrade( ent, id );
		return;
	}*/
	/*if( !Q_stricmp( "repair", cmdbuff ) )
	{
		if( ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
			return;
		trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryRepair( ent, id );
		G_Q3F_SupplyStationRepair( ent, id );
		return;
	}*/
	/*if( !Q_stricmp( "refill", cmdbuff ) )
	{
		if( ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
			return;
		trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryRefill( ent, id );
		G_Q3F_SupplyStationRefill( ent, id );
		return;
	}*/
	if( !Q_stricmp( "rotate", cmdbuff ) )
	{
		trap_Argv( 2, anglebuff, sizeof(angle) );
		angle = atof( anglebuff );
		trap_Argv( 3, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryRotate( ent, id, angle );
		return;
	}
	if( !Q_stricmp( "dismantle", cmdbuff ) )
	{
		trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryDismantle( ent, id );
		G_Q3F_SupplyStationDismantle( ent, id );
		return;
	}
	if( !Q_stricmp( "move", cmdbuff ) )
	{
		trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );
		id = atoi( cmdbuff );
		G_Q3F_SentryPlrMove( ent, id );
		//G_Q3F_SupplyStationDismantle( ent, id );
		return;
	}

	trap_SendServerCommand( ent->s.number, "print \"Usage: build autosentry|supplystation|cancel|menu\n\""); /// slothy |upgrade|repair|refill
}

void G_Q3F_EngineerDestroy_Command( gentity_t *ent )
{
	// Destroy a sentry or supplystation

	char cmdbuff[64];

	if( !ent->client || ent->health <= 0 )
		return;

	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER )
	{
		trap_SendServerCommand( ent->s.number, "print \"Only engineers can remotely destroy autosentries and supply stations.\n\"" );
		return;
	}

	trap_SendServerCommand( ent->s.number, "menu cancel build" );
	trap_Argv( 1, cmdbuff, sizeof(cmdbuff) );
	if( !Q_stricmp( cmdbuff, "autosentry" ) )
	{
		if( ent->client->sentry )
			if( !G_Q3F_SentryCancel( ent->client->sentry ) )
			{
				#ifdef BUILD_BOTS
				Bot_Event_SentryDetonated( ent );
				#endif
				G_Q3F_SentryDie( ent->client->sentry, ent, ent, 0, 0 );
			}
	}
	else if( !Q_stricmp( cmdbuff, "supplystation" ) )
	{
		if( ent->client->supplystation )
			if( !G_Q3F_SupplyStationCancel( ent->client->supplystation ) )
			{
				#ifdef BUILD_BOTS
				Bot_Event_DispenserDetonated( ent );
				#endif
				G_Q3F_SupplyStationDie( ent->client->supplystation, ent, ent, 0, 0 );
			}
	}
	else {
		trap_SendServerCommand( ent->s.number, "print \"Usage: destroy autosentry|supplystation\n\"" );
	}
}

#ifdef DREVIL_BOT_SUPPORT
static void BotSendSentryStatus(gentity_t *sentry)
{
	BotUserData data;

	if(!sentry->parent)
		return;

	// Send the sentry entity
	data.m_DataType = dtEntity;
	data.udata.m_Entity = sentry;
	Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_ENTITY, sentry->parent->s.number, 0, 0, &data);

	data.m_DataType = dt6_2byteFlags;
	data.udata.m_2ByteFlags[0] = sentry->health; // Current Health
	data.udata.m_2ByteFlags[1] = G_Q3F_SentryMaxHealth(sentry->s.legsAnim); // Max Health
	data.udata.m_2ByteFlags[2] = sentry->s.otherEntityNum; // Current Shells
	data.udata.m_2ByteFlags[3] = G_Q3F_SentryMaxShells(sentry->s.legsAnim); // Max Shells
	data.udata.m_2ByteFlags[4] = (sentry->s.otherEntityNum2 & 0xFF) | (sentry->s.otherEntityNum2 << 8); // Rockets
	data.udata.m_2ByteFlags[5] = sentry->s.legsAnim; // Level
	Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_STATS, sentry->parent->s.number, 0, 0, &data);

	// Send the sentry position.
	data.m_DataType = dtVector;
	data.udata.m_Vector[0] = sentry->r.currentOrigin[0];
	data.udata.m_Vector[1] = sentry->r.currentOrigin[1];
	data.udata.m_Vector[2] = sentry->r.currentOrigin[2];
	Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_POSITION, sentry->parent->s.number, 0, 0, &data);

	// Send the sentry facing.
	data.udata.m_Vector[0] = sentry->s.angles[0];
	data.udata.m_Vector[1] = sentry->s.angles[1];
	data.udata.m_Vector[2] = sentry->s.angles[2];
	Bot_Interface_SendEvent(ETF_MESSAGE_SENTRY_FACING, sentry->parent->s.number, 0, 0, &data);	
}

static void BotSendDispenserStatus(gentity_t *station)
{
	BotUserData data;

	if(!station->parent)
		return;

	// Send the sentry entity
	data.m_DataType = dtEntity;
	data.udata.m_Entity = station;
	Bot_Interface_SendEvent(ETF_MESSAGE_SS_ENTITY, station->parent->s.number, 0, 0, &data);

	data.m_DataType = dt6_2byteFlags;
	data.udata.m_2ByteFlags[0] = station->health; // Health
	data.udata.m_2ByteFlags[1] = station->s.origin2[0]; // Shells
	data.udata.m_2ByteFlags[2] = station->s.origin2[1]; // Nails
	data.udata.m_2ByteFlags[3] = station->s.origin2[2]; // Rockets
	data.udata.m_2ByteFlags[4] = station->s.angles2[0]; // Cells
	data.udata.m_2ByteFlags[5] = station->s.angles2[1]; // Armor
	Bot_Interface_SendEvent(ETF_MESSAGE_SS_STATS, station->parent->s.number, 0, 0, &data);

	// Send the ss position.
	data.m_DataType = dtVector;
	data.udata.m_Vector[0] = station->r.currentOrigin[0];
	data.udata.m_Vector[1] = station->r.currentOrigin[1];
	data.udata.m_Vector[2] = station->r.currentOrigin[2];
	Bot_Interface_SendEvent(ETF_MESSAGE_SS_POSITION, station->parent->s.number, 0, 0, &data);

	// Send the ss facing.
	data.udata.m_Vector[0] = station->s.angles[0];
	data.udata.m_Vector[1] = station->s.angles[1];
	data.udata.m_Vector[2] = station->s.angles[2];
	Bot_Interface_SendEvent(ETF_MESSAGE_SS_FACING, station->parent->s.number, 0, 0, &data);
}
#endif
