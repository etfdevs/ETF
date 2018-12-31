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

// cg_playerstate.c -- this file acts on changes in a new playerState_t
// With normal play, this will be done after local prediction, but when
// following another player or playing back a demo, it will be checked
// when the snapshot transitions like all the other entities

#include "cg_local.h"
#include "../game/bg_q3f_weapon.h"
#include "../game/bg_q3f_playerclass.h"
#include "cg_q3f_menu.h"

extern displayContextDef_t cgDC;

/*
==============
CG_CheckAmmo

If the ammo has gone low enough to generate the warning, play a sound
==============
*/
void CG_CheckAmmo( void ) {

	int previous;
//	int weapon;
	bg_q3f_weapon_t *wp;
	int ammotype, ammo;


	//weapon = BG_Q3F_GetClass(&(cg.snap->ps))->weaponslot[cg.snap->ps.weapon];
	wp = BG_Q3F_GetWeapon(cg.snap->ps.weapon);
	ammotype = Q3F_GetAmmoTypeForWeapon(cg.snap->ps.weapon);
	ammo = cg.snap->ps.ammo[ammotype];

	previous = cg.lowAmmoWarning;

	cg.lowAmmoWarning = 0;
	if(ammo < (4000/wp->firetime))			// 4 seconds of weaponry... assuming no realoads.
	{
		cg.lowAmmoWarning = 1;
	}

	if(cg.snap->ps.weapon == WP_NAPALMCANNON)
	{
		if(ammo < 15) // Ammo of 12 == 4 shots +3 = 15
		{
			cg.lowAmmoWarning = 1;
		}
	}

	if(ammo < wp->numammo )
	{
		cg.lowAmmoWarning = 2;				// Can't shoot
	}

	if(ammotype == AMMO_NONE)				// Override it for items without ammo
		cg.lowAmmoWarning = 0;

	// Golliwog: Special 'extra' warning for the minigun only
	if( cg.weaponSelect == WP_MINIGUN )
	{
		if( cg.snap->ps.ammo[AMMO_CELLS] < 4 )
			cg.lowAmmoWarning |= 8;
		else if( cg.snap->ps.ammo[AMMO_CELLS] < 16 )
			cg.lowAmmoWarning |= 4;
	}
	// Golliwog

	// play a sound on transitions
	if ( cg.lowAmmoWarning != previous ) {
		trap_S_StartLocalSound( cgs.media.noAmmoSound, CHAN_LOCAL_SOUND );
	}
}

/*
==============
CG_DamageFeedback
==============
*/
void CG_DamageFeedback( int yawByte, int pitchByte, int damage ) {
	float		left, front, up;
	float		kick;
	int			health;
	float		scale;
	vec3_t		dir;
	vec3_t		angles;
	float		dist;
	float		yaw, pitch;

	// show the attacking player's head and name in corner
	cg.attackerTime = cg.time;
	cg.bleedtime = cg.time;
	cg.bleeddmg = damage;
	CG_Q3F_AddAlertIcon(cg.snap->ps.origin, Q3F_ALERT_PAIN);

	// the lower on health you are, the greater the view kick will be
	health = cg.snap->ps.stats[STAT_HEALTH];
	if ( health < 40 ) {
		scale = 1;
	} else {
		scale = 40.0 / health;
	}
	kick = damage * scale;

	if (kick < 5)
		kick = 5;
	if (kick > 10)
		kick = 10;

	// if yaw and pitch are both 255, make the damage always centered (falling, etc)
	if ( yawByte == 255 && pitchByte == 255 ) {
		cg.damageX = 0;
		cg.damageY = 0;
		cg.v_dmg_roll = 0;
		cg.v_dmg_pitch = -kick;
	} else {
		// positional
		pitch = pitchByte / 255.0 * 360;
		yaw = yawByte / 255.0 * 360;

		angles[PITCH] = pitch;
		angles[YAW] = yaw;
		angles[ROLL] = 0;

		AngleVectors( angles, dir, NULL, NULL );
		VectorSubtract( vec3_origin, dir, dir );

		front = DotProduct (dir, cg.refdef.viewaxis[0] );
		left = DotProduct (dir, cg.refdef.viewaxis[1] );
		up = DotProduct (dir, cg.refdef.viewaxis[2] );

		dir[0] = front;
		dir[1] = left;
		dir[2] = 0;
		dist = VectorLength( dir );
		if ( dist < 0.1 ) {
			dist = 0.1f;
		}

		cg.v_dmg_roll = kick * left;
		
		cg.v_dmg_pitch = -kick * front;

		if ( front <= 0.1 ) {
			front = 0.1f;
		}
		cg.damageX = -left / front;
		cg.damageY = up / dist;
	}

	// clamp the position
	if ( cg.damageX > 1.0 ) {
		cg.damageX = 1.0;
	}
	if ( cg.damageX < - 1.0 ) {
		cg.damageX = -1.0;
	}

	if ( cg.damageY > 1.0 ) {
		cg.damageY = 1.0;
	}
	if ( cg.damageY < - 1.0 ) {
		cg.damageY = -1.0;
	}

	// don't let the screen flashes vary as much
	if ( kick > 10 ) {
		kick = 10;
	}
	cg.damageValue = kick;
	cg.v_dmg_time = cg.time + DAMAGE_TIME;
	cg.damageTime = cg.snap->serverTime;
}




/*
================
CG_Respawn

A respawn happened this snapshot
================
*/
void CG_Respawn( void ) {
	gitem_t* weaponitem;
	char pclass[25];

	cg.serverRespawning = qfalse;   // Arnout: just in case

	// no error decay on player movement
	cg.thisFrameTeleport = qtrue;

	// display weapons available
	cg.weaponSelectTime = cg.time;

	// select the weapon the server says we are using
	cg.weaponSelect = cg.snap->ps.weapon;

	cg.grenade1latch = cg.grenade2latch = qfalse;		// No grenade latch, we can't still be holding it.

	if(cg.weaponSelect) {
		weaponitem = BG_FindItemForWeapon( cg.weaponSelect );
		trap_Cvar_Set("ui_currentweapon", weaponitem->pickup_name);
	}

	pclass[0] = 0;
	cgDC.playerClass = 0;
	if(cgs.clientinfo[cg.snap->ps.clientNum].team == Q3F_TEAM_SPECTATOR || (cg.snap->ps.eFlags & EF_Q3F_NOSPAWN)) {
		cgDC.playerClass = CLASS_SPECTATOR;
		strcpy(pclass, "spectator"); 
	} else {
		switch(cgs.clientinfo[cg.snap->ps.clientNum].cls)
		{
			case Q3F_CLASS_RECON : cgDC.playerClass = CLASS_RECON; strcpy(pclass, "recon"); break;
			case Q3F_CLASS_SNIPER : cgDC.playerClass = CLASS_SNIPER; strcpy(pclass, "sniper"); break;
			case Q3F_CLASS_SOLDIER : cgDC.playerClass = CLASS_SOLDIER; strcpy(pclass, "soldier"); break;
			case Q3F_CLASS_GRENADIER : cgDC.playerClass = CLASS_GRENADIER; strcpy(pclass, "grenadier"); break;
			case Q3F_CLASS_PARAMEDIC : cgDC.playerClass = CLASS_PARAMEDIC; strcpy(pclass, "paramedic"); break;
			case Q3F_CLASS_MINIGUNNER : cgDC.playerClass = CLASS_MINIGUNNER; strcpy(pclass, "minigunner"); break;
			case Q3F_CLASS_FLAMETROOPER : cgDC.playerClass = CLASS_FLAMETROOPER; strcpy(pclass, "flametrooper"); break;
			case Q3F_CLASS_AGENT : cgDC.playerClass = CLASS_AGENT; strcpy(pclass, "agent"); break;
			case Q3F_CLASS_ENGINEER : cgDC.playerClass = CLASS_ENGINEER; strcpy(pclass, "engineer"); break;
			case Q3F_CLASS_CIVILIAN : cgDC.playerClass = CLASS_CIVILIAN; strcpy(pclass, "civilian"); break;
			case Q3F_CLASS_NULL : cgDC.playerClass = CLASS_SPECTATOR; strcpy(pclass, "spectator"); break;
		}
	}
	trap_Cvar_Set("ui_currentclass", pclass);

	cgDC.weapon = 0;  // cg.snap->ps.weapon;
}

extern char *eventnames[];

/*
==============
CG_CheckPlayerstateEvents

==============
*/
void CG_CheckPlayerstateEvents( playerState_t *ps, playerState_t *ops ) {
	int			i;
	int			event;
	centity_t	*cent;

	cent = &cg.predictedPlayerEntity; // cg_entities[ ps->clientNum ];
	// go through the predictable events buffer
	for ( i = ps->eventSequence - MAX_EVENTS ; i < ps->eventSequence ; i++ ) {
		// if we have a new predictable event
		if ( i >= ops->eventSequence
			// or the server told us to play another event instead of a predicted event we already issued
			// or something the server told us changed our prediction causing a different event
			|| (i > ops->eventSequence - MAX_EVENTS && ps->events[i & (MAX_EVENTS-1)] != ops->events[i & (MAX_EVENTS-1)]) ) {

			event = ps->events[ i & (MAX_EVENTS-1) ];
			cent->currentState.event = event;
			cent->currentState.eventParm = ps->eventParms[ i & (MAX_EVENTS-1) ];
			CG_EntityEvent( cent, cent->lerpOrigin );

			cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

			cg.eventSequence++;
		}
	}
}

/*
==================
CG_CheckChangedPredictableEvents
==================
*/
void CG_CheckChangedPredictableEvents( playerState_t *ps ) {
	int i;
	int event;
	centity_t	*cent;

	cent = &cg.predictedPlayerEntity;
	for ( i = ps->eventSequence - MAX_EVENTS ; i < ps->eventSequence ; i++ ) {
		//
		if (i >= cg.eventSequence) {
			continue;
		}
		// if this event is not further back in than the maximum predictable events we remember
		if (i > cg.eventSequence - MAX_PREDICTED_EVENTS) {
			// if the new playerstate event is different from a previously predicted one
			if ( ps->events[i & (MAX_EVENTS-1)] != cg.predictableEvents[i & (MAX_PREDICTED_EVENTS-1) ] ) {

				event = ps->events[ i & (MAX_EVENTS-1) ];
				cent->currentState.event = event;
				cent->currentState.eventParm = ps->eventParms[ i & (MAX_EVENTS-1) ];
				CG_EntityEvent( cent, cent->lerpOrigin );

				cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

				if ( cg_showmiss.integer ) {
					CG_Printf(BOX_PRINT_MODE_CHAT, "WARNING: changed predicted event\n");
				}
			}
		}
	}
}

/*
==================
pushReward
==================
*/
static void pushReward(sfxHandle_t sfx, qhandle_t shader, int rewardCount) {
	if (cg.rewardStack < (MAX_REWARDSTACK-1)) {
		cg.rewardStack++;
		cg.rewardSound[cg.rewardStack] = sfx;
		cg.rewardShader[cg.rewardStack] = shader;
		cg.rewardCount[cg.rewardStack] = rewardCount;
	}
}

/*
==================
CG_CheckLocalSounds
==================
*/
void CG_CheckLocalSounds( playerState_t *ps, playerState_t *ops ) {
	int			/*highScore,*/ health, armor, reward;
	sfxHandle_t sfx;

	// don't play the sounds if the player just changed teams
	if ( ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM] ) {
		return;
	}

	// hit changes
	if ( ps->persistant[PERS_HITS] > ops->persistant[PERS_HITS] ) {
		armor  = ps->persistant[PERS_ATTACKEE_ARMOR] & 0xff;
		health = ps->persistant[PERS_ATTACKEE_ARMOR] >> 8;
		trap_S_StartLocalSound( cgs.media.hitSound, CHAN_LOCAL_SOUND );
	} else if ( ps->persistant[PERS_HITS] < ops->persistant[PERS_HITS] ) {
		trap_S_StartLocalSound( cgs.media.hitTeamSound, CHAN_LOCAL_SOUND );
	}

	// health changes of more than -1 should make pain sounds
	if ( ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH] - 1 ) {
		if ( ps->stats[STAT_HEALTH] > 0 ) {
			CG_PainEvent( &cg.predictedPlayerEntity, ps->stats[STAT_HEALTH] );
		}
	}


	// if we are going into the intermission, don't start any voices
	if ( cg.intermissionStarted ) {
		return;
	}

	// reward sounds
	reward = qfalse;
	if (ps->persistant[PERS_EXCELLENT_COUNT] != ops->persistant[PERS_EXCELLENT_COUNT]) {
		sfx = cgs.media.excellentSound;
		pushReward(sfx, cgs.media.medalExcellent, ps->persistant[PERS_EXCELLENT_COUNT]);
		reward = qtrue;
		//Com_Printf("excellent\n");
	}
	if (ps->persistant[PERS_GAUNTLET_FRAG_COUNT] != ops->persistant[PERS_GAUNTLET_FRAG_COUNT]) {
		sfx = cgs.media.humiliationSound;
		pushReward(sfx, cgs.media.medalGauntlet, ps->persistant[PERS_GAUNTLET_FRAG_COUNT]);
		reward = qtrue;
		//Com_Printf("guantlet frag\n");
	}
	// if any of the player event bits changed
	if (ps->persistant[PERS_PLAYEREVENTS] != ops->persistant[PERS_PLAYEREVENTS]) {
		if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_DENIEDREWARD) !=
				(ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_DENIEDREWARD)) {
			trap_S_StartLocalSound( cgs.media.deniedSound, CHAN_ANNOUNCER );
		}
		else if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_GAUNTLETREWARD) !=
				(ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_GAUNTLETREWARD)) {
			trap_S_StartLocalSound( cgs.media.humiliationSound, CHAN_ANNOUNCER );
		}
		else if ((ps->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_HOLYSHIT) !=
				(ops->persistant[PERS_PLAYEREVENTS] & PLAYEREVENT_HOLYSHIT)) {
			trap_S_StartLocalSound( cgs.media.holyShitSound, CHAN_ANNOUNCER );
		}
		reward = qtrue;
	}

	// timelimit warnings
	if ( cgs.timelimit > 0 && cg.matchState <= MATCH_STATE_PLAYING ) {
		int		msec;

		msec = cg.time - cgs.levelStartTime;

		if ( !( cg.timelimitWarnings & 4 ) && msec > ( cgs.timelimit * 60 + 2 ) * 1000 ) {
			cg.timelimitWarnings |= 1 | 2 | 4;
			trap_S_StartLocalSound( cgs.media.suddenDeathSound, CHAN_ANNOUNCER );
		}
		else if ( !( cg.timelimitWarnings & 2 ) && msec > (cgs.timelimit - 1) * 60 * 1000 ) {
			cg.timelimitWarnings |= 1 | 2;
			trap_S_StartLocalSound( cgs.media.oneMinuteSound, CHAN_ANNOUNCER );
		}
		else if ( cgs.timelimit > 5 && !( cg.timelimitWarnings & 1 ) && msec > (cgs.timelimit - 5) * 60 * 1000 ) {
			cg.timelimitWarnings |= 1;
			trap_S_StartLocalSound( cgs.media.fiveMinuteSound, CHAN_ANNOUNCER );
		}
	}
}

/*
===============
CG_TransitionPlayerState

===============
*/
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops ) {
	bg_q3f_playerclass_t *cls;
	fileHandle_t fh;
	char buf[32];
	int execClassConfig = 0;
	static qboolean doneFirstSpawn = qfalse;

	// check for changing follow mode
	if ( ps->clientNum != ops->clientNum ) {
		cg.thisFrameTeleport = qtrue;
		// make sure we don't get any unwanted transition effects
		*ops = *ps;
	}

	// damage events (player is getting wounded)
	if ( ps->damageEvent != ops->damageEvent && ps->damageCount ) {
		CG_DamageFeedback( ps->damageYaw, ps->damagePitch, ps->damageCount );
	}

	// respawning
	if ( ps->persistant[PERS_SPAWN_COUNT] != ops->persistant[PERS_SPAWN_COUNT] ) {
		doneFirstSpawn = qtrue;

		CG_Respawn();

		if(cg_execClassConfigs.integer) {
			execClassConfig = cg_execClassConfigs.integer % 3;
		} else {
			cls = BG_Q3F_GetClass( ps );
			if( cls->commandstring && *cls->commandstring ) {
				trap_Cvar_VariableStringBuffer(va("cg_execClass%sConfig", cls->commandstring), buf, 32);
				execClassConfig = atoi(buf);
			}
		}

		// Golliwog: If they've changed class, play the sound and exec the class config
		if( ps->persistant[PERS_CURRCLASS] != ops->persistant[PERS_CURRCLASS] ||
			ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM] )
		{
			if( ( ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) && execClassConfig ) {
				if( trap_FS_FOpenFile( "spectator.cfg", &fh, FS_READ ) >= 0 )
				{
					trap_FS_FCloseFile( fh );
					trap_SendConsoleCommand( "exec spectator.cfg\n" );
				}
				else if( trap_FS_FOpenFile( "classconfigs/spectator.cfg", &fh, FS_READ ) >= 0 )
				{
					trap_FS_FCloseFile( fh );
					trap_SendConsoleCommand( "exec classconfigs/spectator.cfg\n" );
				}
			} else {
				if( cg_playClassSound.integer )
					CG_Q3F_PlayClassnameSound( ps->persistant[PERS_CURRCLASS], (cg_playClassSound.integer == 2) ? qtrue : qfalse );
				if( execClassConfig )
				{
					cls = BG_Q3F_GetClass( ps );
					if( cls->commandstring && *cls->commandstring )
					{
						if( trap_FS_FOpenFile( va( "%s.cfg", cls->commandstring ), &fh, FS_READ ) >= 0 )
						{
							trap_FS_FCloseFile( fh );
							trap_SendConsoleCommand( va( "exec %s.cfg\n", cls->commandstring ) );
						}
						else if( trap_FS_FOpenFile( va( "classconfigs/%s.cfg", cls->commandstring ), &fh, FS_READ ) >= 0 )
						{
							trap_FS_FCloseFile( fh );
							trap_SendConsoleCommand( va( "exec classconfigs/%s.cfg\n", cls->commandstring ) );
						}
					}
				}
			}
		}
		else if( execClassConfig == 2 ) {
			if( ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) {
				if( trap_FS_FOpenFile( "spectator.cfg", &fh, FS_READ ) >= 0 )
				{
					trap_FS_FCloseFile( fh );
					trap_SendConsoleCommand( "exec spectator.cfg\n" );
				}
				else if( trap_FS_FOpenFile( "classconfigs/spectator.cfg", &fh, FS_READ ) >= 0 )
				{
					trap_FS_FCloseFile( fh );
					trap_SendConsoleCommand( "exec classconfigs/spectator.cfg\n" );
				}

			} else {
				cls = BG_Q3F_GetClass( ps );
				if( cls->commandstring && *cls->commandstring )
				{
					if( trap_FS_FOpenFile( va( "%s.cfg", cls->commandstring ), &fh, FS_READ ) >= 0 )
					{
						trap_FS_FCloseFile( fh );
						trap_SendConsoleCommand( va( "exec %s.cfg\n", cls->commandstring ) );
					}
					else if( trap_FS_FOpenFile( va( "classconfigs/%s.cfg", cls->commandstring ), &fh, FS_READ ) >= 0 )
					{
						trap_FS_FCloseFile( fh );
						trap_SendConsoleCommand( va( "exec classconfigs/%s.cfg\n", cls->commandstring ) );
					}

				}
			}
		}
		// Golliwog.
	}

	if ( cg.mapRestart ) {
		CG_Respawn();
		cg.mapRestart = qfalse;
	}

	// check for going low on ammo
	CG_CheckAmmo();

	// run events
	CG_CheckPlayerstateEvents( ps, ops );

	if ( cg.snap->ps.pm_type != PM_INTERMISSION 
		&& ps->persistant[PERS_TEAM] != Q3F_TEAM_SPECTATOR ) {
		CG_CheckLocalSounds( ps, ops );
	}

	// smooth the ducking viewheight change
	if ( ps->viewheight != ops->viewheight ) {
		cg.duckChange = ps->viewheight - ops->viewheight;
		cg.duckTime = cg.time;
	}
}
