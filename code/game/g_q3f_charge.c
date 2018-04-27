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

#include "g_local.h"
#include "g_q3f_charge.h"
#include "bg_q3f_playerclass.h"
#include "bg_q3f_grenades.h"
#include "g_q3f_mapents.h"
#include "g_bot_interface.h"

#define Q3F_CHARGE_DAMAGE		1000		// FIXME - this is a VERY temp value :)
#define	Q3F_CHARGE_LAYTIME		3000		// 3 seconds to lay (?) (was 5)
#define	Q3F_CHARGE_LAYDIST		40			// 40 units in front (?)
#define	Q3F_CHARGE_DISARM_TIME	5000		// 5 seconds to disarm

static void G_Q3F_ChargeFree( gentity_t *ent ) {
	if( ent->activator && ent->activator->inuse && ent->activator->client->chargeEntity == ent)
	{
		ent->activator->client->ps.ammoclip[2] = 0;
		ent->activator->client->ps.ammoclip[3] = 0;
		ent->activator->client->chargeEntity = NULL;
	}
}

static void G_Q3F_ChargeThink( gentity_t *ent )
{
	// Think about what we do. We know what we do, of course... :)

	gentity_t *te, *scan;
	char *str;
	trace_t tr;
	vec3_t src, dest;

	if( ent->count > 0 )
	{
		ent->activator->client->ps.ammoclip[2] = ent->count;
		ent->nextthink = level.time + 1000;
		if( ent->count <= 5 )
		{
			// Send a message to owner
			str = va( "print \"%d...\n\"", ent->count );
			trap_SendServerCommand( ent->activator->s.number, str );
		}

		// play a nice sound
		switch (ent->count) {
		case 5:
			te = G_TempEntity( ent->r.currentOrigin, EV_HE_BEEP );
//			te->s.eventParm = G_SoundIndex( "sound/misc/q3f_det_beep1.wav" );
			te->r.svFlags |= SVF_BROADCAST;
			break;
		/*case 4:
		case 3:
		case 2:*/
		case 1:
			te = G_TempEntity( ent->r.currentOrigin, EV_HE_BEEP2 );
//			te->s.eventParm = G_SoundIndex( "sound/misc/q3f_det_beep2.wav" );
			te->r.svFlags |= SVF_BROADCAST;
			break;
		}
	}
	else if( !ent->count )
	{
		#ifdef BUILD_BOTS
		if(ent->activator)
			Bot_Event_DetpackDetonated(ent->activator);
		#endif
		G_RadiusDamage(	ent->r.currentOrigin, ent, &level.gentities[ent->r.ownerNum], Q3F_CHARGE_DAMAGE, 0, MOD_CHARGE, 0 );
		trap_SendServerCommand( -1, "print \"FIRE IN THE HOLE!\n\"" );
		G_AddEvent( ent, EV_HE_EXPLODE, 0);
		ent->r.svFlags |= SVF_BROADCAST;
		ent->freeAfterEvent = qtrue;

		if( ent->activator && ent->activator->inuse )
		{
			ent->activator->client->ps.ammoclip[2] = 0;
			ent->activator->client->ps.ammoclip[3] = 0;
			ent->activator->client->chargeEntity = NULL;
		}

			// Trigger all chargeable ents
		VectorAdd( ent->r.absmax, ent->r.absmin, src );
		VectorScale( src, 0.5, src );
		for( scan = g_entities; scan < &g_entities[level.num_entities]; scan++ )
		{
			if( scan->inuse && scan->mapdata && scan->mapdata->flags & Q3F_FLAG_CHARGEABLE && ( scan->mapdata->state != Q3F_STATE_INVISIBLE || scan->mapdata->state != Q3F_STATE_DISABLED ) )
			{
				if( scan->r.bmodel )
				{
					VectorAdd( scan->r.absmax, scan->r.absmin, dest );
					VectorScale( dest, 0.5, dest );
				}
				else VectorCopy( scan->r.currentOrigin, dest );
				if( Distance( src, dest ) <= Q3F_CHARGE_DAMAGE )
				{
					trap_Trace( &tr, src, NULL, NULL, dest, ent->s.number, CONTENTS_SOLID );
					if( tr.fraction == 1 || tr.entityNum == scan->s.number )
						G_Q3F_TriggerEntity( scan, ent->activator, Q3F_STATE_ACTIVE, &tr, qfalse );
				}
			}
		}
	}
	else
	{
		if( ent->activator && ent->activator->inuse )
		{
			ent->activator->client->ps.ammoclip[2] = 0;
			ent->activator->client->ps.ammoclip[3] = 0;
		}
		G_FreeEntity( ent );
	}

	ent->count--;
}

static void G_Q3F_ChargeTouch( gentity_t *ent, gentity_t *other, trace_t *tr )
{
	// Check for recon disarming techniques.

	gclient_t *client;

	client = other->client;
	if( client->chargeDisarmTime ||
		!client ||
		client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_RECON ||
		!(client->ps.pm_flags & PMF_DUCKED) )
		return;

	// Ensiform : This charge is still being planted, fuck off
	if( ent->activator && ent->activator->client && ent->activator->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) )
		return;

	client->chargeDisarmEnt		= ent;
	client->chargeDisarmTime	= level.time + Q3F_CHARGE_DISARM_TIME;

	trap_SendServerCommand( other->s.number, "print \"Disarming HE charge...\n\"" );
}

static void G_Q3F_ChargePain( gentity_t *self, gentity_t *attacker, int damage )
{
	// If we're on a player, knock it off.

	gentity_t *player;
	vec3_t movevec, currpos;

	self->health = 100000;			// Stop us ever dying.

	if( self->s.groundEntityNum < 0 || self->s.groundEntityNum >= MAX_CLIENTS )
		return;
	player = &g_entities[self->s.groundEntityNum];
	if( !player->inuse || !player->client )
		return;

	VectorCopy( player->client->ps.velocity, movevec );
	movevec[0] += Q_flrand(-1.0f, 1.0f) * damage;
	movevec[1] += Q_flrand(-1.0f, 1.0f) * damage;
	movevec[2] = 10 + Q_flrand(0.0f, 1.0f) * damage * 0.5;

	BG_EvaluateTrajectory( &self->s.pos, level.time, currpos );
	currpos[2] += 4;
	VectorCopy( currpos, self->s.pos.trBase );
	VectorCopy( movevec, self->s.pos.trDelta );
	self->s.pos.trTime = level.time;
	self->s.pos.trType = TR_GRAVITY;
	self->s.groundEntityNum = ENTITYNUM_NONE;
}

qboolean G_Q3F_ChargeCommand( gentity_t *ent )
{
	// We've recieved a grenade command, work out if it's valid and act on it.
	// This is invoked from ClientCommand, so we use trap_Argv to get the parameters.
	int timer;
	char strbuff[16];
	gentity_t *charge;
	vec3_t offset, offset2, mins, maxs;
	trace_t tr;

	if( !ent->client || ent->health <= 0 )
		return( qfalse );

	if( level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )
		return( qfalse );

	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_GRENADIER )
	{
		trap_SendServerCommand( ent->s.number, "print \"Only grenadiers can lay charges.\n\"" );
		return( qfalse );
	}

	trap_Argv( 1, strbuff, 16 );
	if( !Q_stricmp( "cancel", strbuff ) )
	{
		// Cancel charge

		if( ent->client->chargeTime )
		{
			if( ent->client->chargeEntity->inuse && ent->client->chargeEntity->s.eType == ET_Q3F_GRENADE && ent->client->chargeEntity->s.weapon == Q3F_GREN_CHARGE )
				G_FreeEntity( ent->client->chargeEntity );
			else G_Printf( "Attempted to free '%s' as charge.\n", ent->client->chargeEntity->classname );
			ent->client->chargeEntity	= NULL;
			ent->client->chargeTime	= 0;
			ent->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_LAYCHARGE);

			trap_SendServerCommand( ent->s.number, "print \"Charge canceled.\n\"" );
			return( qtrue );
		}
		return( qfalse );
	}
	else if( !Q_stricmp( "menu", strbuff ) )
	{
		trap_SendServerCommand( ent->s.number, "menu charge" );
		return( qtrue );
	}
	else if( (timer = atoi( strbuff )) > 0 )
	{
		int contents;
		// Lay charge

		if( ent->client->chargeEntity )
		{
			trap_SendServerCommand( ent->s.number, "print \"You already have a charge in place.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_AlreadyBuilt(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );			// ... or so slow
		}
		if( ent->s.groundEntityNum == ENTITYNUM_NONE &&
			!(trap_PointContents( ent->r.currentOrigin, ENTITYNUM_NONE ) & CONTENTS_WATER) )
		{
			// We're flying (rather than swimming)
			trap_SendServerCommand( ent->s.number, "print \"You must be on the ground to lay a charge.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_MustBeOnGround(ent, 0);
			#endif
			return( qfalse );
		}
		if( ent->client->ps.ammo[AMMO_CHARGE] <= 0 )
		{
			trap_SendServerCommand( ent->s.number, "print \"You have no HE charges remaining.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_NotEnoughAmmo(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );			// No charges left
		}
		if( ent->client->chargeTime )
		{
			#ifdef BUILD_BOTS
			Bot_Event_Build_AlreadyBuilt(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );			// Already laying a charge
		}
		if( timer < 5 )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot set the timer for less than 5 seconds.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );			// Not so fast
		}
		if( timer > 180 )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot set the timer for more than 3 minutes.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );			// ... or so slow
		}

			// Work out where to drop it
		AngleVectors( ent->client->ps.viewangles, offset, NULL, NULL );
		offset[2] = 0;		// Cancel vertical offset;
		VectorNormalize( offset );
		VectorScale( offset, Q3F_CHARGE_LAYDIST, offset );
		VectorAdd( ent->client->ps.origin, offset, offset );
		VectorCopy( offset, offset2 );
		offset[2] += 8;
		offset2[2] -= 64;
//		VectorSet( mins, -8, -8, -8 );	// Fix this?
//		VectorSet( maxs, 8, 8, 8 );
		VectorSet( mins, -6, 6, 0 );	// Fix this?
		VectorSet( maxs, 6, 6, 6 );
//		trap_Trace( &tr, offset, mins, maxs, offset2, ent->s.number, MASK_PLAYERSOLID );
		G_Q3F_ForceFieldTrace( &tr, offset, mins, maxs, offset2, ent->s.number, MASK_PLAYERSOLID );
		if( tr.fraction == 1 || tr.startsolid )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot lay a charge here.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );
		}
//		trap_Trace( &tr, offset, mins, maxs, ent->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID );
		G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, mins, maxs, offset, ent->s.number, MASK_PLAYERSOLID );
		if( tr.fraction < 1 && (tr.startsolid || tr.entityNum != ent->s.number) )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot lay a charge here.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );
		}
		VectorAdd( tr.endpos, mins, offset );
		VectorAdd( tr.endpos, maxs, offset2 );
		if( G_Q3F_NoBuildCheck( offset, offset2, ent->client->sess.sessionTeam, Q3F_NOBUILD_CHARGE ) )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot lay a charge here.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );
		}

		if( G_Q3F_NoAnnoyCheck( offset, offset2, ent->client->sess.sessionTeam, Q3F_NOANNOY_CHARGES ) )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot lay a charge here.\n\"" );
			#ifdef BUILD_BOTS
			Bot_Event_Build_CantBuild(ent, Q3F_GREN_CHARGE);
			#endif
			return( qfalse );
		}

		ent->client->chargeEntity = charge = G_Spawn();
		ent->client->chargeTime = level.time + Q3F_CHARGE_LAYTIME;	// Time till player release
		ent->client->ps.stats[STAT_Q3F_FLAGS] |= 1 << FL_Q3F_LAYCHARGE;

		trap_SendServerCommand( ent->s.number, va( "print \"Laying charge, %d seconds...\n\"", timer ) );

#ifdef BUILD_BOTS
		Bot_Event_DetpackBuilding(ent, charge);
#endif

			// Initialise grenade
		charge->s.eType		= ET_Q3F_GRENADE;			// Not really a grenade, of course...
		charge->s.eFlags	= 0;//EF_BOUNCE_HALF;
		charge->classname	= "charge";
		VectorCopy( mins, charge->r.mins );
		VectorCopy( maxs, charge->r.maxs );
		charge->touch		= 0;
		charge->s.time		= level.time + Q3F_CHARGE_LAYTIME + timer * 1000;
		charge->s.legsAnim	= 0;
		charge->r.ownerNum	= ent->s.number;
		charge->activator	= ent;
		charge->count		= timer;//5
		charge->soundPos1	= timer;
		charge->damage		= ent->client->sess.sessionTeam;
		charge->s.weapon	= Q3F_GREN_CHARGE;
		charge->think		= G_Q3F_ChargeThink;
		charge->nextthink = level.time + Q3F_CHARGE_LAYTIME;
		//charge->nextthink	= charge->s.time - 5000;		// Time to start countdown
		charge->touch		= G_Q3F_ChargeTouch;
		charge->r.contents	= CONTENTS_TRIGGER;
		charge->health		= 100000;
		charge->takedamage	= qtrue;
		charge->pain		= G_Q3F_ChargePain;
		charge->free		= G_Q3F_ChargeFree;

			// Drop it and set the motion
		G_SetOrigin( charge, offset );
		charge->s.groundEntityNum = ENTITYNUM_NONE;
		charge->s.pos.trDelta[0] = 0;
		charge->s.pos.trDelta[1] = 0;
		charge->s.pos.trDelta[2] = 0;
		charge->s.pos.trTime = level.time;
		charge->s.pos.trType = TR_GRAVITY;

		// Make it face the player
		charge->s.apos.trBase[PITCH] = 0.f;
		charge->s.apos.trBase[YAW] = 180.f + ent->client->ps.viewangles[YAW];
		charge->s.apos.trBase[ROLL] = 0.f;
		ent->s.apos.trType = TR_STATIONARY;

		contents = trap_PointContents( charge->s.pos.trBase, -1 );
		if ( contents & CONTENTS_WATER ) {
			charge->hasbeeninwater = qtrue;
		}

		return( qtrue );
	}
	else {
		// Display a usage menu

		trap_SendServerCommand( ent->s.number, "print \"usage: charge <seconds>|menu|cancel\n\"" );

		return( qfalse );
	}
}

