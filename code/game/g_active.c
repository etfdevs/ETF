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
#include "bg_q3f_playerclass.h"
#include "g_q3f_mapents.h"
#include "g_q3f_flag.h"
#include "g_q3f_weapon.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_scanner.h"
#include "g_q3f_charge.h"
#include "g_q3f_team.h"
#include "bg_q3f_util.h"
#include "g_q3f_admin.h"

#include "g_bot_interface.h"
/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;
	qboolean pentagram = qfalse;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// player has pentagram, don't send damage effects
	// even though armor still takes a hit
	pentagram = client->ps.powerups[PW_PENTAGRAM] > level.time;

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && !pentagram ) {
		player->pain_debounce_time = level.time + 700;
		// RR2DO2 - players found this too irritating, limit it quite a lot - to low health
		if( client->damage_fromFire && player->health <= 25 )
			G_AddEvent( player, EV_BURN, player->health );
		else
			G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;
	}

	client->ps.damageCount = count;

#ifdef DREVIL_BOT_SUPPORT
	{
		int iAttacker = client->ps.persistant[PERS_ATTACKER];
		BotUserData bud;
		bud.m_DataType = dtEntity;
		bud.udata.m_Entity = (iAttacker != ENTITYNUM_WORLD) ? &g_entities[iAttacker] : 0;
		Bot_Interface_SendEvent(PERCEPT_FEEL_PAIN, player->s.clientNum, 0, 0.0f, &bud);
	}	
#endif

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
	client->damage_fromFire = qfalse;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit, aqualung, pentagram;
	int			waterlevel;

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + HOLDBREATHTIME;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit	= ent->client->ps.powerups[PW_BATTLESUIT] > level.time;
	pentagram	= ent->client->ps.powerups[PW_PENTAGRAM] > level.time;
	aqualung	= ent->client->ps.powerups[PW_Q3F_AQUALUNG] > level.time;

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit || pentagram || aqualung ) {
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				//if (ent->health <= ent->damage) {
				//	G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.wav"));
				//} else {
				if (ent->health > ent->damage) {
					G_AddEvent( ent, EV_GURP, ent->health );
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + HOLDBREATHTIME;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && 
		(ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		if (ent->health > 0
			&& ent->pain_debounce_time <= level.time	) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else if ( pentagram ) {
				G_AddEvent( ent, EV_POWERUP_PENTAGRAM, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}

				if (ent->watertype & CONTENTS_SLIME) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						10*waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		ent->client->ps.loopSound = level.snd_fry;
	} else {
		ent->client->ps.loopSound = 0;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

/*		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}*/

		if ( !other->touch ) {
			continue;
		}

		G_Q3F_TouchEntity( ent, other, &trace );	// Golliwog: more sophisticated checks.
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}
		// ignore most entities if a spectator
		//if ( ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR ) {
		if (Q3F_IsSpectator(ent->client)) {	// RR2DO2
			if ( hit->s.eType != ET_TELEPORT_TRIGGER  /* &&  keeg
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger &&
				hit->touch != SpectatorTouch_DoorTrigger */ ) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			if (	!trap_EntityContact( mins, maxs, hit ) &&
					Q_stricmp( hit->classname, "func_goalinfo" ) )	// Golliwog: Hack - what does trap_EntityContact DO?
				continue;
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			G_Q3F_TouchEntity( ent, hit, &trace );	// Golliwog: more sophisticated checks.
		}

/*		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}*/
	}

	// if we didn't touch a jump pad this pmove frame
	if ( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount ) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}

/*
===============
G_Q3F_InterpolateAngles

Takes two angles, a duration, starttime and time and returns current angle
===============
*/
/*static void G_Q3F_InterpolateAngles( vec3_t src_angle, vec3_t dst_angle, int startTime, int Duration, int atTime, vec3_t result ) {
	float	deltaTime;
	vec3_t	delta;
	float	f;

	if ( atTime > startTime + Duration ) {
		atTime = startTime + Duration;
	}
	
	deltaTime = ( atTime - startTime ) * 0.001;	// milliseconds to seconds
	if ( deltaTime < 0 ) {
		deltaTime = 0;
	}

	VectorSubtract( dst_angle, src_angle, delta );
	f = 1000.0 / Duration;
	VectorScale( delta, f, delta );

	VectorMA( src_angle, deltaTime, delta, result );
}*/

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gclient_t	*client;

	client = ent->client;

	if(	client->sess.spectatorState == SPECTATOR_NOT && (client->ps.eFlags & EF_Q3F_NOSPAWN) &&
		client->respawnTime <= level.time )
	{
		respawn( ent );
		return;
	}

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW &&
		 client->sess.spectatorState != SPECTATOR_CHASE &&
		 client->sess.spectatorState != SPECTATOR_FLYBY ) {
		// RR2DO2
		if (client->sess.adminLevel >= ADMIN_MATCH || client->sess.adminLevel >= ADMIN_FULL)
			client->ps.pm_type = PM_ADMINSPECTATOR;
		else if (g_spectatorMode.integer == 0 && client->sess.sessionTeam == Q3F_TEAM_SPECTATOR)
			client->ps.pm_type = PM_SPECTATOR;
		else
			client->ps.pm_type = PM_LIMITEDSPECTATOR;
		// RR2DO2
		client->ps.speed = 440;  //keeg recon speed

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_TraceNoEnts;
//		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// Golliwog: Prevent firing during ceasefire
		if( level.ceaseFire || client->ps.powerups[PW_Q3F_CEASEFIRE] ) /* Ensiform was PW_NUM_POWERUPS should be PW_Q3F_CEASEFIRE */
			pm.ps->pm_flags |= PMF_CEASEFIRE;
		else
			pm.ps->pm_flags &= ~PMF_CEASEFIRE;
		// Golliwog.

		pm.agentclass = client->agentclass;

		// perform a pmove
		Pmove (&pm);

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent ); 
		trap_UnlinkEntity( ent );
	} else if ( client->sess.spectatorState == SPECTATOR_FLYBY ) {

		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 0;

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = 0; // flyby cam can fly through anything
//		pm.trace = trap_Trace;
		pm.trace = trap_TraceNoEnts;  //keeg don't use forcefield trace 
		pm.pointcontents = trap_PointContents;

		if( level.ceaseFire || client->ps.powerups[PW_Q3F_CEASEFIRE] ) /* Ensiform was PW_NUM_POWERUPS should be PW_Q3F_CEASEFIRE */
			pm.ps->pm_flags |= PMF_CEASEFIRE;
		else
			pm.ps->pm_flags &= ~PMF_CEASEFIRE;

		pm.agentclass = client->agentclass;

		// perform a pmove
		Pmove (&pm);

		if (level.flybyPathIndex >= 0 ) {
			if (client->camtraj.trTime + client->camtraj.trDuration - FRAMETIME > level.time) {
				vec3_t pos;

				// Origin
				BG_Q3F_EvaluateSplineTrajectory( &client->camtraj, NULL, &level.campaths[level.flybyPathIndex].splines[client->currtrajindex], level.time, pos );
				VectorCopy( pos, client->ps.origin );
				BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
				VectorCopy( client->ps.origin, ent->r.currentOrigin );
			}
		}
		trap_UnlinkEntity( ent );
	}
	// RR2DO2: a player with a team but without class can't move either
	else if (client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL) {
		client->ps.speed = 0;	// do not move
	}
	// RR2DO2

	client->oldbuttons = client->buttons;
	client->oldwbuttons = client->wbuttons;
	client->oldcmdflags = client->cmdflags;

	client->buttons = ucmd->buttons;
	client->wbuttons = ucmd->wbuttons;
	client->cmdflags = ucmd->flags;

	// attack button cycles through spectators
	// RR2DO2
	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {
		if ( client->sess.spectatorState == SPECTATOR_FOLLOW  ) {
			Cmd_FollowCycle_f( ent, 1, SPECTATOR_FOLLOW );
		} else if ( client->sess.spectatorState == SPECTATOR_CHASE  ) {
			Cmd_FollowCycle_f( ent, 1, SPECTATOR_CHASE );
		}
	}
	// use or jump goes to free flight
	else if ( (ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_CHASE) &&
		(((client->buttons & BUTTON_USE_HOLDABLE) && !(client->oldbuttons & BUTTON_USE_HOLDABLE)) || ucmd->upmove > 0)) {
		StopFollowing( ent, qfalse );
	}
}

/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gentity_t *ent ) {
	gclient_t *client;

	client = ent->client;
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & BUTTON_ATTACK) ) {
		client->activityTime = level.time;
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			G_Q3F_DropClient( ent, "Dropped due to inactivity" );
//			trap_DropClient( , "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t *client;
	bg_q3f_playerclass_t *cls;
	char userinfo[MAX_INFO_STRING], *dataptr;
	int data;

	client = ent->client;
	cls = BG_Q3F_GetClass( &client->ps );
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// regenerate
		if ( client->ps.powerups[PW_REGEN] ) {
			if ( ent->health < cls->maxhealth) {
				ent->health += 15;
				if ( ent->health > cls->maxhealth * 1.1 ) {
					ent->health = cls->maxhealth * 1.1;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			} else if ( ent->health < cls->maxhealth * 2) {
				ent->health += 5;
				if ( ent->health > cls->maxhealth * 2 ) {
					ent->health = cls->maxhealth * 2;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			}
		} else {
			// count down health when over max
			if ( ent->health > cls->maxhealth ) {
				ent->health--;
			}
		}

		// count down armor when over max
		// FALCON: Modified to suit Q3F's per-class max armour
		if ( client->ps.stats[STAT_ARMOR] > cls->maxarmour ) {
			client->ps.stats[STAT_ARMOR]--;
		// FALCON: END
		}

		// Golliwog: Agent invisible, consumes cells. This should be handled
		// in Pmove, I think.
		if( ent->s.eFlags & EF_Q3F_INVISIBLE )
		{
			if( --client->ps.ammo[AMMO_CELLS] <= 0 )
			{
				client->ps.ammo[AMMO_CELLS] = 0;
				G_Q3F_StopAgentInvisible( ent );
				trap_SendServerCommand( ent->s.number, "print \"Invisibility failed for lack of cells!\n\"" );
			}
			else if( VectorLength( ent->client->ps.velocity ) > 20 )
			{
				G_Q3F_StopAgentInvisible( ent );
				trap_SendServerCommand( ent->s.number, "print \"Invisibility failed because of movement!\n\"" );
			}
		}
		// Golliwog.

		// Golliwog: Do a minimum rate check.
		if( g_minRate.integer && g_banRules.value > 1 )
		{
			trap_GetUserinfo( ent->s.number, userinfo, sizeof(userinfo) );
			dataptr = Info_ValueForKey( userinfo, "rate" );
			data = 0;
			if( !dataptr || (data = atoi( dataptr )) < g_minRate.integer ) {
				G_Q3F_AdminTempBan( ent, va( "Rate set to %d (sv_MinRate is %d)", data, g_minRate.integer ), Q3F_ADMIN_TEMPBAN_TIME );
			}
		}
		// Golliwog.
		// RR2DO2: Do a minimum snaps check.
		if( g_minSnaps.integer && g_banRules.value > 1 )
		{
			trap_GetUserinfo( ent->s.number, userinfo, sizeof(userinfo) );
			dataptr = Info_ValueForKey( userinfo, "snaps" );
			data = 0;
			if( !dataptr || (data = atoi( dataptr )) < g_minSnaps.integer )
				G_Q3F_AdminTempBan( ent, va( "Snaps set to %d (sv_MinSnaps is %d)", data, g_minSnaps.integer ), Q3F_ADMIN_TEMPBAN_TIME );
		}

		// Check for valid cg_adjustAgentSpeed
		trap_GetUserinfo( ent->s.number, userinfo, sizeof(userinfo) );
		dataptr = Info_ValueForKey( userinfo, "cg_adjustAgentSpeed" );
		data = 0;
		if( dataptr && ent->client )
			ent->client->sess.adjustAgentSpeed = atoi( dataptr );
		// RR2DO2
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->oldwbuttons = client->wbuttons;
	client->oldcmdflags = client->cmdflags;
	client->buttons = client->pers.cmd.buttons;
	client->wbuttons = client->pers.cmd.wbuttons;
	client->cmdflags = client->pers.cmd.flags;

	if ( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int		i, j;
	int		event;
	gclient_t *client;
	int		damage = 0;
	vec3_t	dir;
	vec3_t	origin, angles;
//	qboolean	fired;
	gitem_t *item;
	gentity_t *drop;
	bg_q3f_playerclass_t *cls;

	client = ent->client;
	cls = BG_Q3F_GetClass( &client->ps );

	if ( oldEventSequence < client->ps.eventSequence - MAX_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_EVENTS-1) ];

#ifdef _DEBUG
	{
		extern char *eventnames[];
		char buf[256];
		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
		if ( atof(buf) != 0 ) {
			G_Printf("Game event %20s\n", eventnames[event]);
		}
	}
#endif

		switch ( event ) {
		/*case EV_FALL_MEDIUM:
		case EV_FALL_FAR:
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_FAR && client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_RECON ) {
				damage = 10;
			} else {
				damage = 5;
			}
			VectorSet (dir, 0, 0, 1);
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
			break;*/
		case EV_FALL_D31:
		case EV_FALL_D29:
		case EV_FALL_D27:
		case EV_FALL_D25:
		case EV_FALL_D23:
		case EV_FALL_D21:
		case EV_FALL_D19:
		case EV_FALL_D17:
		case EV_FALL_D15:
		case EV_FALL_D13:
		case EV_FALL_D11:
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( (g_dmflags.integer & DF_NO_FALLING) || level.nofallingdmg ) {
				break;
			}
			switch ( event ) {
				case EV_FALL_D31: damage = 31; break;
				case EV_FALL_D29: damage = 29; break;
				case EV_FALL_D27: damage = 27; break;
				case EV_FALL_D25: damage = 25; break;
				case EV_FALL_D23: damage = 23; break;
				case EV_FALL_D21: damage = 21; break;
				case EV_FALL_D19: damage = 19; break;
				case EV_FALL_D17: damage = 17; break;
				case EV_FALL_D15: damage = 15; break;
				case EV_FALL_D13: damage = 13; break;
				case EV_FALL_D11: damage = 11; break;
			}
			if ( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_RECON ) {
				damage *= 0.5;
			}
			VectorSet (dir, 0, 0, 1);
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, DAMAGE_NO_ARMOR, MOD_FALLING);
			break;

		case EV_FIRE_WEAPON:
			FireWeapon( ent );
			break;

		case EV_PLACE_BUILDING:
			{
				if(ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_MOVING))
				{
					
				}
			}
			break;

		case EV_CHANGE_WEAPON:
			ent->flags &= ~FL_NO_KNOCKBACK;
			ent->client->ps.eFlags &= ~ EF_Q3F_AIMING;
			break;

		case EV_USE_ITEM1:		// teleporter
			// drop flags in CTF
			item = NULL;
			j = 0;

			if ( item ) {
				drop = Drop_Item( ent, item, 0 );
				// decide how many seconds it has left
				drop->count = ( ent->client->ps.powerups[ j ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}

				ent->client->ps.powerups[ j ] = 0;
			}

			SelectSpawnPoint( ent->client->ps.origin, origin, angles, ent );
			TeleportPlayer( ent, origin, angles );
			break;

		case EV_USE_ITEM2:		// medkit
			//ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			ent->health = cls->maxhealth;		// Golliwog: This is the most they're getting :)
			break;

		default:
			break;
		}
	}

}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
#if 0
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {

		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
#endif
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;
	bg_q3f_playerclass_t *cls, *agentcls;
	gentity_t *other;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	// frameOffset should be about the number of milliseconds into a frame 
	// this command packet was received, depending on how fast the server
	// does a G_RunFrame()
	client->frameOffset = trap_Milliseconds() - level.frameStartTime;

	// save the estimated ping in a queue for averaging later
	// we use level.previousTime to account for 50ms lag correction
	// besides, this will turn out numbers more like what players are used to
	client->pers.pingsamples[client->pers.samplehead] = level.previousTime + client->frameOffset - ucmd->serverTime;
	client->pers.samplehead++;
	if ( client->pers.samplehead >= NUM_PING_SAMPLES ) {
		client->pers.samplehead -= NUM_PING_SAMPLES;
	}

	// initialize the real ping
	if ( g_truePing.integer ) {
		int i, sum = 0;

		// get an average of the samples we saved up
		for ( i = 0; i < NUM_PING_SAMPLES; i++ ) {
			sum += client->pers.pingsamples[i];
		}

		client->pers.realPing = sum / NUM_PING_SAMPLES;
	}
	else {
		// if g_truePing is off, use the normal ping
		client->pers.realPing = client->ps.ping;
	}

//unlagged - lag simulation #2
	// keep a queue of past commands
	client->pers.cmdqueue[client->pers.cmdhead] = client->pers.cmd;
	client->pers.cmdhead++;
	if ( client->pers.cmdhead >= MAX_LATENT_CMDS ) {
		client->pers.cmdhead -= MAX_LATENT_CMDS;
	}

	// if the client wants latency in commands (client-to-server latency)
	if ( client->pers.latentCmds ) {
		// save the actual command time
		int time = ucmd->serverTime;

		// find out which index in the queue we want
		int cmdindex = client->pers.cmdhead - client->pers.latentCmds - 1;
		while ( cmdindex < 0 ) {
			cmdindex += MAX_LATENT_CMDS;
		}

		// read in the old command
		client->pers.cmd = client->pers.cmdqueue[cmdindex];

		// adjust the real ping to reflect the new latency
		client->pers.realPing += time - ucmd->serverTime;
	}
//unlagged - lag simulation #2


//unlagged - backward reconciliation #4
	// save the command time *before* pmove_fixed messes with the serverTime,
	// and *after* lag simulation messes with it :)
	// attackTime will be used for backward reconciliation later (time shift)
	client->attackTime = ucmd->serverTime;
//unlagged - backward reconciliation #4

//unlagged - smooth clients #1
	// keep track of this for later - we'll use this to decide whether or not
	// to send extrapolated positions for this client
	client->lastUpdateFrame = level.framenum;
//unlagged - smooth clients #1


//unlagged - lag simulation #1
	// if the client is adding latency to received snapshots (server-to-client latency)
	if ( client->pers.latentSnaps ) {
		// adjust the real ping
		client->pers.realPing += client->pers.latentSnaps * (1000 / sv_fps.integer);
		// adjust the attack time so backward reconciliation will work
		client->attackTime -= client->pers.latentSnaps * (1000 / sv_fps.integer);
	}
//unlagged - lag simulation #1


//unlagged - true ping
	// make sure the true ping is over 0 - with cl_timenudge it can be less
	if ( client->pers.realPing < 0 ) {
		client->pers.realPing = 0;
	}
//unlagged - true ping

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW && client->sess.spectatorState != SPECTATOR_CHASE ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	//if ( client->sess.sessionTeam == Q3F_TEAM_SPECTATOR) {}
	if (Q3F_IsSpectator(ent->client)) {	//RR2DO2
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( ent ) ) {
		return;
	}

	// clear the rewards if time
	if ( level.time > client->rewardTime ) {
		client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT );
	}

	// clear drop anim if time
	if( !(client->ps.extFlags & EXTF_ANI_THROWING) &&
		!(client->ps.extFlags & EXTF_ANI_OPERATING)) {
		client->torsoanimEndTime = 0;
	} else if( client->torsoanimEndTime < level.time ) {
		client->ps.extFlags &= ~EXTF_ANI_THROWING;
		client->ps.extFlags &= ~EXTF_ANI_OPERATING;
	}

	if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else if( client->ps.eFlags & EF_Q3F_INVISIBLE ) {
		client->ps.pm_type = PM_INVISIBLE;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	//	JT: Pay attention to STAT_Q3F_FLAGS

	client->ps.gravity = g_gravity.value;

	// set speed
	// Golliwog: Altered for class speeds, and max health
	if( (cls = BG_Q3F_GetClass( &client->ps )) != NULL )
		client->ps.speed = cls->maxspeed;
	else
		client->ps.speed = g_speed.value;
	// Golliwog.

	// Golliwog: Have agents slow down to apparent class speed
	if( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT && client->agentclass  && client->sess.adjustAgentSpeed )
	{
		agentcls = bg_q3f_classlist[client->agentclass];
		if( client->ps.speed > agentcls->maxspeed )
			client->ps.speed = agentcls->maxspeed;
	}
	// Golliwog.

	// Tranquilised
	if( client->tranqTime ) 	{
		if( level.time >= client->tranqTime ) {
			trap_SendServerCommand( ent->s.number, "print \"You feel better now.\n\"" );
			client->tranqTime = 0;
			client->tranqEnt = NULL;
			client->ps.stats[STAT_Q3F_FLAGS] &= ~(1<<FL_Q3F_TRANQ);
		 } else {
			client->ps.speed /= 2;
			client->ps.stats[STAT_Q3F_FLAGS] |= (1<<FL_Q3F_TRANQ);
		}
	} else {
		client->ps.stats[STAT_Q3F_FLAGS] &= ~(1<<FL_Q3F_TRANQ);
	}
	//Canabis, am i burning?
	if (client->flames > 0 && ent->health > 0) {
		client->ps.extFlags |= EXTF_BURNING;
	} else {
		client->ps.extFlags &= ~EXTF_BURNING;
	}
	//Canabis, am i tranqed?
	if (client->tranqTime >= level.time && ent->health > 0) {
		client->ps.extFlags |= EXTF_TRANQED;
	} else {
		client->ps.extFlags &= ~EXTF_TRANQED;
	}
	//Ensiform, am i legshot?
	if (client->legwounds && ent->health > 0) {
		if( client->legwounds > 6 )
			client->legwounds = 6;			// Cap the wounds to something sensible.
		client->ps.extFlags |= EXTF_LEGWOUNDS;
	} else {
		client->ps.extFlags &= ~EXTF_LEGWOUNDS;
	}
	// Agent invisible
	if( client->ps.eFlags & EF_Q3F_INVISIBLE )
		client->ps.speed = 0;		// Can't move if invisible.

	// Laying a charge.
	if( client->chargeTime ) {
		if( client->chargeTime <= level.time ) {
			client->ps.ammoclip[3] = client->chargeEntity->soundPos1;
			trap_SendServerCommand( ent->s.number, "print \"Charge set.\n\"" );
			if( client->ps.ammo[AMMO_CHARGE] > 0 )
				client->ps.ammo[AMMO_CHARGE]--;
			client->chargeTime = 0;		// Allow movement again
			client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_LAYCHARGE);
			client->chargeEntity->s.legsAnim = 1;
			#ifdef BUILD_BOTS
			Bot_Event_DetpackBuilt(ent, client->chargeEntity);
			#endif
			ent->client->pers.stats.data[STATS_GREN + Q3F_GREN_CHARGE].shots++;
		} else {
			other = client->chargeEntity;
			if( Distance( other->r.currentOrigin, ent->r.currentOrigin ) > 100 ) {
				// They've moved too far, cancel the lay.
				if( other->inuse &&	(other->s.eType == ET_Q3F_GRENADE && other->s.weapon == Q3F_GREN_CHARGE))
					G_FreeEntity( other );
				else G_Printf( "Attempted to free '%s' as charge.\n", other->classname );
				client->chargeEntity = NULL;
				client->chargeTime = 0;
				client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_LAYCHARGE);
			}
			else client->ps.speed = 0;		// Stop all movement.
		}
	}
	// Disarming a charge
	if( client->chargeDisarmTime )
	{
		if( !client->chargeDisarmEnt || !client->chargeDisarmEnt->inuse )
		{
			// Lost our disarm ent?

			client->chargeDisarmEnt = NULL;
			client->chargeDisarmTime = 0;
		}
		else if(	!(client->ps.pm_flags & PMF_DUCKED) ||
					!trap_EntityContact( ent->r.absmin, ent->r.absmax, client->chargeDisarmEnt ) )
		{
			// We've lost contact.

			if( client->chargeDisarmEnt->count >= 5 && !(rand() % 10) )
			{
				// They've flumped the disarming process :)

				trap_SendServerCommand( ent->s.number, "print \"HE Charge Triggered! RUN!\n\"" );
				client->chargeDisarmEnt->count = 5;
				client->chargeDisarmEnt->nextthink = level.time;
			}
			client->chargeDisarmEnt = NULL;
			client->chargeDisarmTime = 0;
		}
		else if( client->chargeDisarmTime <= level.time )
		{
			// We've finished, clean this ent up.

			trap_SendServerCommand( ent->s.number, "print \"HE Charge Disarmed.\n\"" );
			if( client->chargeDisarmEnt->activator && client->chargeDisarmEnt->activator->inuse )
			{
				client->chargeDisarmEnt->activator->client->chargeEntity = NULL;
				client->chargeDisarmEnt->activator->client->chargeTime = 0;
				client->chargeDisarmEnt->activator->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_LAYCHARGE);
			}
			G_FreeEntity( client->chargeDisarmEnt );
			client->chargeDisarmEnt = NULL;
			client->chargeDisarmTime = 0;
		} else client->ps.speed = 0;		// Stop all movement.
	}

	if( client->buildTime ) {
		if( client->buildTime <= level.time ) {
			client->buildTime = 0;		// Allow movement again
			client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_BUILDING);
		} else {
			if(	(client->sentry && !client->sentry->s.legsAnim &&
				Distance( client->sentry->r.currentOrigin, ent->r.currentOrigin ) > 100) ||
				(client->supplystation && !client->supplystation->s.legsAnim &&
				Distance( client->supplystation->r.currentOrigin, ent->r.currentOrigin ) > 100) )
				{
				// They've moved too far, cancel the build.
				if( client->sentry )
					G_Q3F_SentryCancel( client->sentry );
				if( client->supplystation )
					G_Q3F_SupplyStationCancel( client->supplystation );
				client->buildTime = 0;
				client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_BUILDING);
			} else 
				client->ps.speed = 0;		// Stop all movement.
		}
	}
	if( client->repairEnt && client->repairEnt->inuse ) {
		if( client->repairEnt->s.eType == ET_Q3F_SENTRY && !G_Q3F_CheckSentryUpgradeable( ent, client->repairEnt->s.number, qtrue ) ) {
			trap_SendServerCommand( ent->s.number, "menu cancel upgradeautosentry" );
			client->repairEnt = NULL;
		} else if ( client->repairEnt->s.eType == ET_Q3F_SUPPLYSTATION && !G_Q3F_CheckSupplyStation( ent, client->repairEnt->s.number ) ) {
			trap_SendServerCommand( ent->s.number, "menu cancel upgradesupplystation" );
			client->repairEnt = NULL;
		}
	}

	if( client->speedscale )				// Goalitem speed scale
		client->ps.speed *= client->speedscale;

	if( client->callTime && client->callTime <= level.time ) {
		// Golliwog: Reset call flags (should use a PlayerStateToEntityState call)
		client->ps.eFlags	&= ~EF_Q3F_MASKME;
		ent->s.eFlags		&= ~EF_Q3F_MASKME;
		client->callTime	= 0;
	}
	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}
	if ( client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_MOVING)) {
		client->ps.speed *= 0.5;
	}
	// Sniper shots to legs.
	if(client->legwounds) {
		client->ps.speed -= 0.1 * client->ps.speed * client->legwounds;
	}

	if( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_GRENADIER )
		G_Q3F_CheckPipesForPlayer(ent);

	G_Q3F_Check_Maladies(ent);

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));
	pm.ps = &client->ps;
	pm.cmd = *ucmd;

	pm.trace = G_Q3F_ForceFieldTrace;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		pm.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	} else if ( pm.ps->pm_type == PM_SPECTATOR || pm.ps->pm_type == PM_ADMINSPECTATOR ) {
		pm.trace = trap_TraceNoEnts;
	} else {
		pm.tracemask = MASK_PLAYERSOLID;
	}

	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy( client->ps.origin, client->oldOrigin );

	// Golliwog: Prevent firing during ceasefire
	if( level.ceaseFire || client->ps.powerups[PW_Q3F_CEASEFIRE] )
		pm.ps->pm_flags |= PMF_CEASEFIRE;
	else pm.ps->pm_flags &= ~PMF_CEASEFIRE;
	// Golliwog.
		
	// Golliwog: Use any held goalitems || RR2DO2: doing this through 'useflag' command now, to prevent nasty things
	// RR2DO2: fix, only add it when it's moving
	if( pm.ps->groundEntityNum >= 0 && pm.ps->groundEntityNum != ENTITYNUM_NONE &&
		( g_entities[pm.ps->groundEntityNum].s.pos.trType == TR_LINEAR ||
		  g_entities[pm.ps->groundEntityNum].s.pos.trType == TR_ROTATING ||
		  ( g_entities[pm.ps->groundEntityNum].s.pos.trType == TR_LINEAR_STOP &&
		    level.time >= g_entities[pm.ps->groundEntityNum].s.pos.trTime + g_entities[pm.ps->groundEntityNum].s.pos.trDuration) ) ) {
		VectorCopy( g_entities[pm.ps->groundEntityNum].s.pos.trDelta, pm.groundVelocity );
	} else {
		VectorSet( pm.groundVelocity, 0, 0, 0 );
	}
	// Golliwog.

	pm.airleft = ent->client->airOutTime - level.time;
	pm.agentclass = client->agentclass;
	pm.retflags = (client->tauntTime > 5000 || client->tauntHeld) ? PMRF_SKIP_TAUNT : 0;
	// perform a pmove
	Pmove (&pm);

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	
	/* Check for changes after the pmove */
	if (client->ps.eFlags & EF_Q3F_AIMING ) {
		if (!client->sniperdot && client->ps.weapon == WP_SNIPER_RIFLE) {
			client->aimtime = client->attackTime;
			G_Q3F_SniperDot(ent, qtrue);
		} else if ( !( ent->flags & FL_NO_KNOCKBACK ) && client->ps.weapon == WP_MINIGUN ) {
			client->aimtime = client->attackTime;
			ent->flags |= FL_NO_KNOCKBACK;
		}
	} else {
		ent->flags &= ~FL_NO_KNOCKBACK;
		if ( client->sniperdot ) 
			G_Q3F_SniperDot(ent, qfalse);
	}

	if ( pm.retflags & PMRF_DONE_TAUNT ) {
		client->tauntHeld = qtrue;
		client->tauntTime *= 5;
		if (client->tauntTime < 5000)
			client->tauntTime = 5000;
		else if (client->tauntTime > 60000)
			client->tauntTime = 60000;
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);

	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->oldwbuttons = client->wbuttons;
	client->oldcmdflags = client->cmdflags;

	client->buttons = ucmd->buttons;
	client->wbuttons = ucmd->wbuttons;
	client->cmdflags = ucmd->flags;

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ( client->respawnForce ||
				(g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 ) ) {
				respawn( ent );
				return;
			}
		
			// pressing attack or use is the normal respawn method
			if( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ||
				ucmd->upmove > 10 ) {
				respawn( ent );
			}
		}
		return;
	}

	// Reset flamer chain
	if( !(ucmd->buttons & BUTTON_ATTACK) )
		client->lastflame = 0;

	// canabis: Check for automatic reload
	if( client->pers.autoReload == 2 && client->ps.weaponstate == WEAPON_READY )
	{
		if( Q3F_GetClipValue( client->ps.weapon, &client->ps ) < BG_Q3F_GetWeapon( client->ps.weapon )->numammo )
			BG_Q3F_Request_Reload( &client->ps );
	}

	// RR2DO2: if we carry an item with "revealagent" make sure we stop our disguise as agent, just in case
	if( client->agentdata && ( (client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == 1 || (client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == 4 ) ) {
		gentity_t *goalitem;

		/* canabis, remove disguise if your quadded */
		if ( client->ps.powerups[PW_QUAD] > level.time ) {
			trap_SendServerCommand( ent->s.number, "print \"You've lost your disguise!\n" );
			if( (client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == 1 )
				G_Q3F_StopAgentDisguise( ent );
			else if( (client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == 4 )
				G_Q3F_StopAgentInvisible( ent );
			
		} else 
		for( goalitem = g_entities; goalitem < &g_entities[level.num_entities] ; goalitem++ )
		{
			if( !goalitem->inuse )
				continue;
			if( (goalitem->s.eType == ET_Q3F_GOAL) && (goalitem->activator == ent) &&
				goalitem->mapdata && (goalitem->mapdata->state == Q3F_STATE_CARRIED) &&
				(goalitem->mapdata->flags & Q3F_FLAG_REVEALAGENT) )
			{				
				trap_SendServerCommand( ent->s.number, "print \"You've lost your disguise!\n" );
				if( (client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == 1 )
					G_Q3F_StopAgentDisguise( ent );
				else if( (client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == 4 )
					G_Q3F_StopAgentInvisible( ent );
				break;
			}
		}
	}
	// RR2DO2

	// Canabis, check for special commands
	if ( (client->cmdflags & UCMDF_SPECIAL) && !(client->oldcmdflags & UCMDF_SPECIAL)) {
		switch(ent->client->ps.persistant[PERS_CURRCLASS]) {
		case Q3F_CLASS_RECON:
			{
				if(!level.ceaseFire && !ent->client->ps.powerups[PW_Q3F_CEASEFIRE] && !Q3F_IsSpectator(ent->client))
					G_Q3F_ToggleScanner( ent );
			}
			break;
		case Q3F_CLASS_SOLDIER:
			BG_Q3F_Request_Reload( &client->ps );
			break;
		case Q3F_CLASS_GRENADIER:
			{
				if ( !level.ceaseFire && !ent->client->ps.powerups[PW_Q3F_CEASEFIRE] )
					G_Q3F_DetPipe(ent, ent->client->attackTime );
			}
			break;
		case Q3F_CLASS_AGENT:
			trap_SendServerCommand( ent->s.number, "menu disguise" );
			break;
		case Q3F_CLASS_ENGINEER:
			if( ent->client->sentry && G_Q3F_SentryCancel( ent->client->sentry ) )
				trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
			else if( ent->client->supplystation && G_Q3F_SupplyStationCancel( ent->client->supplystation ) )
				trap_SendServerCommand( ent->s.number, "print \"You stop building.\n\"" );
			else {
				// see if we're able to hit a sentry
				vec3_t		muzzle, forward, end;
				trace_t		tr;
				gentity_t	*traceEnt;
				CalcMuzzlePoint ( ent,  muzzle, forward );
				VectorMA (muzzle, G_Q3F_GAUNTLET_RANGE, forward, end);
				G_UnlaggedTrace( ent, &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );
				if ( tr.surfaceFlags & SURF_NOIMPACT ) {
					trap_SendServerCommand( ent->s.number, "menu build" );
					break;
				}
				traceEnt = &g_entities[ tr.entityNum ];
				if( traceEnt->s.eType == ET_Q3F_SENTRY && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) ) {
					trap_SendServerCommand(	ent->s.number, va( "menu upgradeautosentry %d", traceEnt->s.number ) );
					ent->client->repairEnt = traceEnt;
					ent->client->repairTime = level.time;
				} else if( traceEnt->s.eType == ET_Q3F_SUPPLYSTATION && (ent == traceEnt->parent || G_Q3F_IsAllied( ent, traceEnt->parent )) ) {
					trap_SendServerCommand(	ent->s.number, va( "menu upgradesupplystation %d", traceEnt->s.number ) );
					ent->client->repairEnt = traceEnt;
					ent->client->repairTime = level.time;
				} else {
					trap_SendServerCommand( ent->s.number, "menu build" );
				}
			}
			break;
		}
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunction(va("Client Think: (%i)", clientNum));
#endif

	ent = g_entities + clientNum;
	//unlagged - lag simulation #3
	// if the client wants to simulate outgoing packet loss
	if ( ent->client->pers.plOut ) {
		// see if a random value is below the threshhold
		float thresh = (float)ent->client->pers.plOut / 100.0f;
		if ( Q_flrand(0.0f, 1.0f) < thresh ) {
			usercmd_t	cmd;
			/* Get the command but don't handle it */
			trap_GetUsercmd( clientNum, &cmd );
			return;
		}
	}
	//unlagged - lag simulation #3
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	if ( !g_synchronousClients.integer ) {
		ClientThink_real( ent );
	}

	// Golliwog: Process any delayed name changes.
	if( (ent->client->pers.namechangeTime <= level.time) && ent->client->pers.newnetname[0] )
		ClientUserinfoChanged( ent->s.number, "name change" );
	// Golliwog.

#ifdef PERFLOG
	BG_Q3F_PerformanceMonitor_LogFunctionStop();
#endif
}


void G_RunClient( gentity_t *ent ) {
	if ( /*!(ent->r.svFlags & SVF_BOT) && */!g_synchronousClients.integer ) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ||
		 ent->client->sess.spectatorState == SPECTATOR_CHASE ) {
		int		clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && !Q3F_IsSpectator(cl) ) {	// RR2DO2
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->client->ps = cl->ps;
				switch ( ent->client->sess.spectatorState )
				{
				default: break;
				case SPECTATOR_FOLLOW:	ent->client->ps.pm_flags |= PMF_FOLLOW; break;
				case SPECTATOR_CHASE:	ent->client->ps.pm_flags |= PMF_CHASE; break;
				}
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

void G_VersionCheck_Init() {
	trap_FS_FOpenFile( "version.log", &level.versionLogFile, FS_APPEND );
	if ( !level.versionLogFile ) {
		G_Printf( "WARNING: Couldn't open logfile: version.log\n");
	} else {
		G_VersionLogPrintf("------------------------------------------------------------\n" );
		G_VersionLogPrintf("VersionCheck Init:\n" );
	}
}

void G_VersionCheck_Close() {
	if ( !level.versionLogFile ) {
		return;
	}

	G_VersionLogPrintf("VersionCheck Shutdown:\n");
	G_VersionLogPrintf("------------------------------------------------------------\n" );

	trap_FS_FCloseFile( level.versionLogFile );
}

void G_VersionLogUser( gentity_t *ent ) {
	char		string[1024];
	char		userinfo[BIG_INFO_KEY+BIG_INFO_VALUE+2];
	char		key[BIG_INFO_KEY], value[BIG_INFO_VALUE];
	const char *info;
	int			min, tens, sec;

	if ( !level.versionLogFile ) {
		return;
	}

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i Client Version Check Failed For %s" S_COLOR_WHITE " (%d)\n", min, tens, sec, ent->client->pers.netname, (int)(ent-level.gentities) );

	trap_FS_Write( string, strlen( string ), level.versionLogFile );

	Com_sprintf( string, sizeof(string), "%3i:%i%i Userinfo:\n", min, tens, sec );

	trap_FS_Write( string, strlen( string ), level.versionLogFile );

	trap_GetUserinfo( ent-level.gentities, userinfo, sizeof( userinfo ) );
	info = &userinfo[0];

	// Loop through each key/value pair and write them to file with the timestamp
	while( 1 )
	{
		Info_NextPair( &info, key, value );
		if( !*info )
			return;

		Com_sprintf( string, sizeof(string), "%3i:%i%i %-20s%s\n", min, tens, sec, key, value );
		trap_FS_Write( string, strlen( string ), level.versionLogFile );
	}
}

void QDECL G_VersionLogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;
	int			offset;

	if ( !level.versionLogFile ) {
		return;
	}

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );
	offset = strlen(string);

	va_start( argptr, fmt );
	Q_vsnprintf( string + offset, sizeof(string) - offset, fmt, argptr );
	va_end( argptr );

	trap_FS_Write( string, strlen( string ), level.versionLogFile );
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;
	//clientPersistant_t	*pers;
	int			frames;

	/* Ensiform - FIXME Happens to people randomly for no reason? */
	if (!ent->client->sess.versionOK && ent->client->pers.enterTime + 5000 < level.time && !g_allowAllVersions.integer ) {
		G_VersionLogUser( ent );
		G_Q3F_DropClient( ent, "Client doesn't have version " FORTS_VERSION);
		return;
	}

	if ( ent->client->tauntHeld ) {
		if (!(ent->client->pers.cmd.flags & UCMDF_GESTURE_MASK))
			ent->client->tauntHeld = qfalse;
	} else if (ent->client->tauntTime) {
		ent->client->tauntTime -= level.time - level.previousTime;
		if (ent->client->tauntTime < 0)
			ent->client->tauntTime = 0;
	}

	if (Q3F_IsSpectator(ent->client)) {	// RR2DO2
		SpectatorClientEndFrame( ent );
		return;
	}
	// see how many frames the client has missed
	frames = level.framenum - ent->client->lastUpdateFrame - 1;

	// don't extrapolate more than two frames
	if ( frames > 2 ) {
		// if they missed more than two in a row, show the phone jack
		ent->client->ps.eFlags |= EF_CONNECTION;
		ent->s.eFlags |= EF_CONNECTION;
	}

	// did the client miss any frames?
	if ( frames > 0 && g_smoothClients.integer ) {
		int lastframe = ent->client->lastUpdateFrame;
		/* Missed a frame, force a clienthink, fuck you laggers */
		ent->client->pers.cmd.serverTime += 1000 / sv_fps.integer;
		ClientThink_real( ent );
		ent->client->lastUpdateFrame = lastframe;
		ent->client->ps.persistant[PERS_FLAGS] |= PF_SKIPPEDFRAME;
	} else {
		ent->client->ps.persistant[PERS_FLAGS] &= ~PF_SKIPPEDFRAME;
	}

	//pers = &ent->client->pers;

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ent->client->ps.powerups[ i ] && ent->client->ps.powerups[ i ] < level.time ) {
			switch( i )
			{
				case PW_Q3F_CONCUSS:	
					trap_SendServerCommand( ent->s.number, "print \"You can see straight again.\n\"" );
					break;
				case PW_Q3F_FLASH:
					trap_SendServerCommand( ent->s.number, "print \"Your blindness has cleared.\n\"" );
					break;
				case PW_Q3F_GAS:
					trap_SendServerCommand( ent->s.number, "print \"You feel a little less confused now.\n\"" );
					break;
			}
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

//	SendPendingPredictableEvents( &ent->client->ps );

	// mark as not missing updates initially
	ent->client->ps.eFlags &= ~EF_CONNECTION;

	// store the client's position for backward reconciliation later
	G_StoreHistory( ent );
}


