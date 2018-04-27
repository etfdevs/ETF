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

// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_q3f_playerclass.h"
#include "bg_q3f_weapon.h"
#include "bg_q3f_util.h"

pmove_t		*pm;
pml_t		pml;

// movement parameters
float	pm_stopspeed = 100.0f;
/*float	pm_duckScale = 0.25;
float	pm_swimScale = 0.60;
float	pm_swimSurfaceScale = 0.65;		// If we're at the surface, we go faster.
float	pm_wadeScale = 0.70;*/
float	pm_sniperAimScale = 0.287f;
float	pm_minigunSpinScale = 0.35f;
float	pm_duckScale = 0.35f;
float	pm_swimScale = 0.75f;
float	pm_swimSurfaceScale = 0.80f;	// If we're at the surface, we go faster.
float	pm_wadeScale = 0.85f;
float	pm_ladderScale = 0.80f;			// fraction max speed on ladders
float	pm_landConcScale = 10.0f;		// The amount of conc exaggeration when you hit the ground.

float	pm_accelerate = 10.0f;
float	pm_wateraccelerate = 8.0f;
float	pm_flyaccelerate = 8.0f;
float	pm_ladderAccelerate = 3000.0f;  // The acceleration to friction ratio is 1:1

float	pm_airaccelerate = 6.0f;		// Golliwog: Was one, but not 'QW' enough
//float	pm_concairaccelerate = 3.0;		// ETF 0.0
float	pm_concairaccelerate = 3.6;
float	pm_airminspeed = 50.0f; // the "minimum" speed while in air, used to get away from jump pads
#define AMSD_ASYNC 0.35f//2.5
#define AMSD_SYNC 1.5f//9
float	pm_airmaxspeeddiff = AMSD_ASYNC; // this is the actual max acceleration

float	pm_friction = 6.0f;
float	pm_airfriction = 6.0f;			// Air friction, ignores vertical values
float	pm_waterfriction = 2.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;
float	pm_ladderfriction = 3000.0f;  // Friction is high enough so you don't slip down

int		c_pmove = 0;

#ifdef CGAME
			#define BOX_PRINT_MODE_CHAT			0
			void QDECL CG_Printf( int mode, const char *msg, ... );
			void	trap_Cvar_Set( const char *var_name, const char *value );
#endif 


/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {
	if ( pm->ps->pm_type != PM_INVISIBLE && pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}

static void PM_StartLegsAnim( int anim ) {
	if ( pm->ps->pm_type != PM_INVISIBLE && pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	pm->ps->legsAnim = ( ( pm->ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}

static void PM_ContinueTorsoAnim( int anim ) {
	if ( ( pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->torsoTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartTorsoAnim( anim );
}

static void PM_ContinueLegsAnim( int anim ) {
	if ( ( pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartLegsAnim( anim );
}

static void PM_ForceTorsoAnim( int anim ) {
	pm->ps->legsTimer = 0;
	PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {
	pm->ps->legsTimer = 0;
	PM_StartLegsAnim( anim );
}

int PM_GetIdleAnim( int weaponNum, int classNum ) {
	if( weaponNum == WP_AXE ) {
		switch( classNum ) {
		case Q3F_CLASS_AGENT:		return ANI_WEAPON_KNIFE_IDLE;
		case Q3F_CLASS_PARAMEDIC:	return ANI_WEAPON_SYRINGE_IDLE;
		case Q3F_CLASS_ENGINEER:	return ANI_WEAPON_WRENCH_IDLE;
		default:					return bg_q3f_weapons[weaponNum]->animNumber[0];
		}
	} else {
		return bg_q3f_weapons[weaponNum]->animNumber[0];
	}
}

int PM_GetAttackAnim( int weaponNum, int classNum ) {
	if( weaponNum == WP_AXE ) {
		switch( classNum ) {
		case Q3F_CLASS_AGENT:		return ANI_WEAPON_KNIFE_ATTACK;
		case Q3F_CLASS_PARAMEDIC:	return ANI_WEAPON_SYRINGE_ATTACK;
		case Q3F_CLASS_ENGINEER:	return ANI_WEAPON_WRENCH_ATTACK;
		default:					return bg_q3f_weapons[weaponNum]->animNumber[1];
		}
	} else {
		return bg_q3f_weapons[weaponNum]->animNumber[1];
	}
}

int PM_GetReloadAnim( int weaponNum, int classNum ) {
	if( weaponNum == WP_AXE ) {
		switch( classNum ) {
		case Q3F_CLASS_AGENT:		return ANI_WEAPON_KNIFE_IDLE;
		case Q3F_CLASS_PARAMEDIC:	return ANI_WEAPON_SYRINGE_IDLE;
		case Q3F_CLASS_ENGINEER:	return ANI_WEAPON_WRENCH_IDLE;
		default:					return bg_q3f_weapons[weaponNum]->animNumber[2];
		}
	} else {
		return bg_q3f_weapons[weaponNum]->animNumber[2];
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	//Canabis stop all movement if you go very slow
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;
		vel[2] = 0;
		return;
	}
	//canabis

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	if ( pml.ladder ) // If they're on a ladder... 
		drop += speed*pm_ladderfriction*pml.frametime;  // Add ladder friction! 

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

	// apply flying friction
	if ( pm->ps->powerups[PW_FLIGHT] ) {
		drop += speed*pm_flightfriction*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR || pm->ps->pm_type == PM_ADMINSPECTATOR ) { // RR2DO2
		drop += speed*pm_spectatorfriction*pml.frametime;
	}

	// Golliwog: air friction.
	if( !pml.ladder && !pml.walking && pm->waterlevel <= 1 && pm->ps->pm_type != PM_SPECTATOR && pm->ps->pm_type != PM_ADMINSPECTATOR && !pm->waterlevel ) // RR2DO2
	{
		drop += speed*pm_airfriction*pml.frametime;
		newspeed = speed - drop;
		if (newspeed < 0) {
			newspeed = 0;
		}
		newspeed /= speed;

		vel[0] = vel[0] * newspeed;
		vel[1] = vel[1] * newspeed;
		return;
	}
	// Golliwog.

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 1
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0) {
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}
	
	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed*wishdir[i];	
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}

static void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel )
{
	float		addspeed, accelspeed;
	float newvel[3], tempvel[3];

	// calculate the acceleration
	accelspeed = accel*pml.frametime*wishspeed;
	// get the current speed
	VectorCopy(pm->ps->velocity, newvel);
	
	// apply the accelereration
	newvel[0] += accelspeed*wishdir[0];
	newvel[1] += accelspeed*wishdir[1];
	newvel[2] += accelspeed*wishdir[2];

	// get some info
	VectorCopy(newvel, tempvel);
	tempvel[2] = 0;
	addspeed = VectorNormalize(tempvel);
	VectorCopy(pm->ps->velocity, tempvel);
	tempvel[2] = 0;
	accelspeed = VectorNormalize(tempvel);

	// only apply the new accelerated speed if the user tries to stop
	// save some (pm_airmaxspeeddiff) so that people can accellerate if they wish
	// FIXME?: If accel is greater than maxspeeddiff, all accelleration is lost

	// also, apply the new speed if the current speed is very low (to get away from jump pads)
	if ((addspeed < (pm_airmaxspeeddiff + accelspeed)) || (accelspeed < pm_airminspeed))
		VectorCopy(newvel, pm->ps->velocity); // decrease speed
}

/*
============
PM_CmdScale

Apply a 'concussion effect' to the player
============
*/
static qboolean PM_Q3F_ApplyConc( usercmd_t *cmd )
{
	int i;
	float scale, distortion, angle;
	qboolean applied;

	if( (i = pm->ps->powerups[PW_Q3F_CONCUSS]) > cmd->serverTime && pm->ps->stats[STAT_HEALTH] > 0 ) {
		if( pml.walking || (pm->ps->pm_flags & PMF_Q3F_CONSPEED) ) {
//			applied = qtrue;
			if( cmd->forwardmove || cmd->rightmove || (!pml.walking && pml.previous_walking) ) {
				// Modify movement based on concussion.
				// Yes, this is horrible. Blame Ghetto :)

				i -= cmd->serverTime;
				scale = 1.0 - ((float) i) / 7000.0;
				scale = 0.6f * ((i > 7000) ? 1 : (1 - scale * scale));
				distortion = 0.001 * (float) (pm->ps->powerups[PW_Q3F_CONCUSS] + cmd->serverTime);	// A random but constantly changing value
					// 2 in radians (sin+sin) is about 65 degrees either side. We scale to about 40 degrees.
				angle = cmd->forwardmove
							? atan2( cmd->rightmove, cmd->forwardmove )
							: (cmd->rightmove > 0 ? 0.5*M_PI : 0.5*-M_PI);
				if( !pml.walking && pml.previous_walking && pm->waterlevel < 2 )
					angle = ((float) (cmd->serverTime & 1023)) / M_PI / 512;	// Lurch on landing.

				distortion = angle + scale * (sin( distortion ) + sin( distortion * 4.27 ));

				// the forward and rightmove need to be 127 or -127 max, and only deduct from that. if you grab a 0-56 range then you will half
				// class speed.
				cmd->forwardmove	= (int) (127.0f * cos(distortion));
				cmd->rightmove		= (int) (127.0f * sin(distortion));
			}
		} 
		
		if( pm->ps->pm_flags & PMF_Q3F_CONCLAND) { 
			applied = qtrue;
		} else {
			applied = qfalse;
		}

			
		if( pml.walking || pm->waterlevel >= 2 ) {
			pm->ps->pm_flags |= PMF_Q3F_CONCLAND;
		}
	}
	else {
		pm->ps->pm_flags &= ~(PMF_Q3F_CONCLAND | PMF_Q3F_CONSPEED);
		applied = qfalse;
	}

	return( applied );
}


/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pml.maxSpeed * max / ( 127.0 * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		} 
	}
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	bg_q3f_playerclass_t *cls;
//	vec3_t temp;
	float product;

	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;		// don't allow jump until all buttons are up
	}

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	// Golliwog: Check to see if we can jump off a ladder, first
	if( pml.ladder )
	{
		// Jump off ladder
		cls = BG_Q3F_GetClass( pm->ps );
		if( (product = DotProduct( pml.forward, pml.ladderNormal )) < 0 )
		{
			product = pml.forward[2]*pm->cmd.forwardmove + pml.right[2]*pm->cmd.rightmove;// + pm->cmd.upmove; 
			if( product < 0 || (pm->ps->pm_flags & PMF_DUCKED) )
			{
				// They want off NOW :)
				VectorScale( pml.forward, cls->maxspeed, pm->ps->velocity );
				pm->ps->velocity[2] = 0;
			}
			else return( qfalse );	// Don't jump if we're facing the ladder and not crouching
		}
		else {
			VectorScale( pml.forward, cls->maxspeed, pm->ps->velocity );
			pm->ps->velocity[2] = JUMP_VELOCITY;
		}
	}
	else {
		pm->ps->velocity[0] += pm->groundVelocity[0];
		pm->ps->velocity[1] += pm->groundVelocity[1];
/*		if(pm->ps->velocity[2] < 0) { // if we can jump, we are grounded.... so...
			pm->ps->velocity[2] = 0;
		}*/
		pm->ps->velocity[2] += pm->groundVelocity[2] * 0.5 + JUMP_VELOCITY;
//		VectorAdd( pm->ps->velocity, pm->groundVelocity, pm->ps->velocity );
/*#ifdef DEBUG_CGAME
		CG_Printf(	"CG jump, velocity %d %d %d\n",
					(int) pm->ps->velocity[0],
					(int) pm->ps->velocity[1],
					(int) pm->ps->velocity[2] );
#endif
#ifdef DEBUG_GAME
		G_Printf(	"G jump, velocity %d %d %d\n",
					(int) pm->ps->velocity[0],
					(int) pm->ps->velocity[1],
					(int) pm->ps->velocity[2] );
#endif*/
	}
	// Golliwog.

	pml.ladder = qfalse;
	pm->ps->pm_flags &= ~PMF_LADDER;
	pml.groundPlane = qfalse;		// jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pml.previous_walking = qtrue;	// We assume this was true, sort of...

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	PM_AddEvent( EV_JUMP );

		// Prevent bunny-hopping escaping the concussion.
	if( pm->ps->powerups[PW_Q3F_CONCUSS] > pm->cmd.serverTime &&
		pm->ps->stats[STAT_HEALTH] > 0 ) {
		pm->ps->pm_flags |= PMF_Q3F_CONCLAND;
	}


	if ( pm->cmd.forwardmove >= 0 ) {
		PM_ForceLegsAnim( ANI_MOVE_JUMP );
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		PM_ForceLegsAnim( ANI_MOVE_JUMPBACK );
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pm->ps->origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( !(cont & CONTENTS_SOLID) ) {
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( cont ) {
			return qfalse;
	}

	// jump out of water
	VectorScale (pml.forward, pml.maxSpeed, pm->ps->velocity);
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
	// waterjump has no control, but falls

	PM_StepSlideMove( qtrue, STEPSIZE );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if (pm->ps->velocity[2] < 0) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
=============
CheckLadder [ ARTHUR TOMLIN ]
=============
*/
void CheckLadder( void )
{
	vec3_t flatforward,spot;
	trace_t trace;
	pml.ladder = qfalse;
	pm->ps->pm_flags &= ~PMF_LADDER;

	if( pm->ps->pm_flags & PMF_JUMP_HELD )		// Can't grab ladder while jumping
		return;

		// check for ladder in front
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);
	VectorMA( pm->ps->origin, 1, flatforward, spot );
	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, MASK_PLAYERSOLID );
	if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
	{
		if( pml.groundPlane && (pml.forward[2]*pm->cmd.forwardmove + pml.right[2]*pm->cmd.rightmove) < 0 )
			return;						// Moving backwards off the ladder, let go
		pml.ladder = qtrue;				// Ladder to the front of us...
	}
	else {
		if( !pml.groundPlane )
		{
			// Only for jumps, or back and sides if NOT on ground (so we can walk off ladder)

			flatforward[0] = -flatforward[0];
			flatforward[1] = -flatforward[1];
			VectorMA( pm->ps->origin, 1, flatforward, spot );
			pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, MASK_PLAYERSOLID );
			if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
				pml.ladder = qtrue;				// Ladder to the rear of us...
			else {
				spot[0] = flatforward[0];
				flatforward[0] = flatforward[1];
				flatforward[1] = spot[0];
				VectorMA( pm->ps->origin, 1, flatforward, spot );
				pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, MASK_PLAYERSOLID );
				if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
					pml.ladder = qtrue;				// Ladder to the, uh, side of us... :)
				else {
					flatforward[0] = -flatforward[0];
					flatforward[1] = -flatforward[1];
					VectorMA( pm->ps->origin, 1, flatforward, spot );
					pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, MASK_PLAYERSOLID );
					if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
						pml.ladder = qtrue;				// Ladder to the other side of us...
				}
			}
		}
	}

	if(pml.ladder)
	{
		pm->ps->pm_flags |= PMF_LADDER;	// set ladder bit
	}
	VectorCopy( trace.plane.normal, pml.ladderNormal );
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;
	usercmd_t cmd;

	if ( PM_CheckWaterJump() ) {
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype == CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if (pm->watertype == CONTENTS_SLIME) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	cmd = pm->cmd;
	PM_Q3F_ApplyConc( &cmd );
	scale = PM_CmdScale( &cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	} else {
		for (i=0 ; i<3 ; i++)
			wishvel[i] = scale * pml.forward[i]*cmd.forwardmove + scale * pml.right[i]*cmd.rightmove;

		wishvel[2] += scale * cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if( pm->waterlevel <= 2 )
	{
		// We go faster if we're at the surface (nasty QW-style physics)
		if ( wishspeed > pml.maxSpeed * pm_swimSurfaceScale ) {
			wishspeed = pml.maxSpeed * pm_swimSurfaceScale;
		}
	}
	else {
		if ( wishspeed > pml.maxSpeed * pm_swimScale ) {
			wishspeed = pml.maxSpeed * pm_swimScale;
		}
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove( qfalse );
}



/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	usercmd_t cmd;

	// normal slowdown
	PM_Friction ();
	cmd = pm->cmd;

//	PM_Q3F_ApplyConc( &cmd );
	scale = PM_CmdScale( &cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for (i=0 ; i<3 ; i++) {
			wishvel[i] = scale * pml.forward[i]*cmd.forwardmove + scale * pml.right[i]*cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove( qfalse, STEPSIZE );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( float maxspeed ) {
	int			i;
	vec3_t		wishvel, checkvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed, checkspeed;
	float		scale;
	qboolean	conced;

	usercmd_t	cmd;

	//PM_Friction();

	if( pm->ps->powerups[PW_Q3F_CONCUSS] > pm->cmd.serverTime && pm->ps->stats[STAT_HEALTH] > 0 && !(pm->ps->pm_flags & PMF_Q3F_CONSPEED)) {
			bg_q3f_playerclass_t* cls = BG_Q3F_GetClass( pm->ps );

		if( VectorLength( pm->ps->velocity ) < cls->maxspeed ) {
			pm->ps->pm_flags |= PMF_Q3F_CONSPEED;
		}
	}

	cmd = pm->cmd;
	conced = PM_Q3F_ApplyConc( &cmd );
	scale = PM_CmdScale( &cmd );

	fmove = cmd.forwardmove;
	smove = cmd.rightmove;

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	if( maxspeed )	// Golliwog: Check max speed isn't exceeded (on jump only).
	{
		VectorCopy( wishvel, checkvel );
		checkvel[2] = 0;
		checkspeed = VectorLength( checkvel );
		if( checkspeed > maxspeed )
		{
			wishvel[0] *= (maxspeed / checkspeed);
			wishvel[1] *= (maxspeed / checkspeed);
		}
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	if(conced) {
		PM_AirAccelerate (wishdir, wishspeed, pm_concairaccelerate );
	} else {
		PM_AirAccelerate (wishdir, wishspeed, pm_airaccelerate );
	}

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );
	}

	PM_StepSlideMove ( qtrue, JUMPSTEPSIZE );
}

/*
===================
PM_LadderMove()
by: Calrathan [Arthur Tomlin]

Right now all I know is that this works for VERTICAL ladders. 
Ladders with angles on them (urban2 for AQ2) haven't been tested.
===================
*/
static void PM_LadderMove( void ) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;
	vec3_t ladderright, up;
//	float vel;
	bg_q3f_playerclass_t *cls;
	vec3_t playerforward;
	float dot;

	if ( PM_CheckJump () ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			cls = BG_Q3F_GetClass( pm->ps );
			PM_AirMove( cls->maxspeed );
		}
		return;
	}

	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );

		// user intentions [what the user is attempting to do]
	if ( !scale ) { 
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	}
	else {   // if they're trying to move... lets calculate it
		for (i=0 ; i<3 ; i++)
		wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove; 
		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	// Golliwog: Change it so that any you simply go up or down, at full speed
	wishspeed = VectorNormalize(wishdir);
	VectorSet( up, 0, 0, 1 );
	CrossProduct( pml.ladderNormal, up, ladderright );
	VectorNormalizeFast( ladderright );
	AngleVectors( pm->ps->viewangles, playerforward, NULL, NULL );
	dot = DotProduct( playerforward, pml.ladderNormal );
	if( wishvel[2] > 5 || (pm->cmd.upmove >= 10) )
	{
		/*VectorCopy( pml.ladderNormal, wishdir );
		wishdir[0] = -wishdir[0];
		wishdir[1] = -wishdir[1];
		wishdir[2] += 7500; // was 5000
		VectorNormalize( wishdir );*/
		wishdir[0] = -wishdir[0];
		wishdir[1] = -wishdir[1];
		wishdir[2] += 500;
		VectorMA( wishdir, pm->cmd.rightmove * (dot < 0 ? -1 : 1), ladderright, wishdir );
		VectorNormalize( wishdir );
	}
	else if( wishvel[2] < -5 )
	{
		VectorCopy( pml.ladderNormal, wishdir );
		wishdir[0] = -wishdir[0];
		wishdir[1] = -wishdir[1];
		wishdir[2] -= 500;
		VectorMA( wishdir, pm->cmd.rightmove * (dot < 0 ? -1 : 1), ladderright, wishdir );
		VectorNormalize( wishdir );
	}

	else {
		wishspeed *= 0.25f;
		VectorMA( wishdir, pm->cmd.rightmove * (dot < 0 ? -1 : 1), ladderright, wishdir );
		VectorNormalize( wishdir );
	}
	// Golliwog

	if ( wishspeed > pml.maxSpeed * pm_ladderScale ) {
		wishspeed = pml.maxSpeed * pm_ladderScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_ladderAccelerate);

/*	// This SHOULD help us with sloped ladders, but it remains untested.
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.ladderNormal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
			// slide along the ground plane [the ladder section under our feet] 
		PM_ClipVelocity (pm->ps->velocity, pml.ladderNormal, 
		pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}*/

	PM_SlideMove( qfalse ); // move without gravity
}

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;
	bg_q3f_playerclass_t *cls;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}

	if ( PM_CheckJump () ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			cls = BG_Q3F_GetClass( pm->ps );
			PM_AirMove( cls->maxspeed );
		}
		return;
	}

	PM_Friction ();

	cmd = pm->cmd;
	PM_Q3F_ApplyConc( &cmd );

	fmove = cmd.forwardmove;
	smove = cmd.rightmove;

	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pml.maxSpeed * pm_duckScale ) {
			wishspeed = pml.maxSpeed * pm_duckScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
		if ( wishspeed > pml.maxSpeed * waterScale ) {
			wishspeed = pml.maxSpeed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove( qfalse, STEPSIZE );

	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}

	if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS ) {
		return EV_FOOTSTEP_METAL;
	}

	if ( pm->ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_MINIGUNNER ) {
		return EV_FOOTSTEP_METAL;
	}

	if( pm->ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT && !(pm->ps->eFlags & EF_Q3F_STEPMASK) )
		return( 0 );		// Golliwog: Agents have no footsteps

	return EV_FOOTSTEP;
}

/*
================
PM_FootprintForSurface

Returns an event number apropriate for the groundsurface
================
*/
static qboolean PM_FootprintForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_FOOTPRINTS ) {
		return( qtrue );
	}

	return( qfalse );
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;

	// decide which landing animation to use
	if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
		PM_ForceLegsAnim( ANI_MOVE_LANDBACK );
	} else {
		PM_ForceLegsAnim( ANI_MOVE_LAND );
	}

	pm->ps->legsTimer = TIMER_LAND;

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) {
		return;
	}
	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta*delta * 0.0001;

	// ducking while falling doubles damage
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		delta *= 2;
	}

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) {
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) {
		delta *= 0.25;
	}
	if ( pm->waterlevel == 1 ) {
		delta *= 0.5;
	}

	if ( delta < 1 ) {
		return;
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  {
		// RR2DO2: grmblgrmblgrml qw had no nice falling damage detection (in 2.5)
		// RR2DO2: it is better in 2.6+
		/*if ( delta > 60 ) {
			PM_AddEvent( EV_FALL_FAR );
		} else if ( delta > 40 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent( EV_FALL_MEDIUM );
			}*/
		if ( delta > 299 ) {
			PM_AddEvent( EV_FALL_D31 );
		} else if ( delta > 272 ) {
			PM_AddEvent( EV_FALL_D29 );
		} else if ( delta > 247 ) {
			PM_AddEvent( EV_FALL_D27 );
		} else if ( delta > 219 ) {
			PM_AddEvent( EV_FALL_D25 );
		} else if ( delta > 193 ) {
			PM_AddEvent( EV_FALL_D23 );
		} else if ( delta > 166 ) {
			PM_AddEvent( EV_FALL_D21 );
		} else if ( delta > 140 ) {
			PM_AddEvent( EV_FALL_D19 );
		} else if ( delta > 113 ) {
			PM_AddEvent( EV_FALL_D17 );
		} else if ( delta > 88 ) {
			PM_AddEvent( EV_FALL_D15 );
		} else if ( delta > 62 ) {
			PM_AddEvent( EV_FALL_D13 );
		} else if ( delta > 36 ) {
			PM_AddEvent( EV_FALL_D11 );
		} else if ( delta > 7 ) {
			PM_AddEvent( EV_FALL_SHORT ); // RR2DO2: falling without damage
		} else {
			if ( PM_FootprintForSurface() ) {
				BG_AddPredictableEventToPlayerstate( PM_FootstepForSurface(), 1, pm->ps ); // RR2DO2: add footprints
			} else {
				PM_AddEvent( PM_FootstepForSurface() );
			}
		}
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
void PM_CheckStuck(void) {
	trace_t trace;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
	if (trace.allsolid) {
		Com_Printf("Hey i seem to be stuck\n");
	}
}


/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
	int			i, j, k;
	vec3_t		point;

	if ( pm->debugLevel ) {
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
//	trace_t		trace;
//	vec3_t		point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
/*		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);*/
		if ( /*trace.fraction*/ pml.groundTrace.fraction == 1.0 ) {
			if ( pm->cmd.forwardmove >= 0 ) {
				PM_ForceLegsAnim( ANI_MOVE_JUMP );
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				PM_ForceLegsAnim( ANI_MOVE_JUMPBACK );
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t		point;
	trace_t		trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 64;//0.001;//0.25; //djbob: more accurate *cross fingers*

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;
	pml.previous_walking = pm->ps->groundEntityNum != ENTITYNUM_NONE;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid(&trace) )
			return;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction * 64 > 0.001 /*trace.fraction != 1.0*/) { //djbob: optimization: single trace for ground/jump anim check
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
		if ( pm->cmd.forwardmove >= 0 ) {
			PM_ForceLegsAnim( ANI_MOVE_JUMP );
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} else {
			PM_ForceLegsAnim( ANI_MOVE_JUMPBACK );
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	
	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf("%i:Land\n", c_pmove);
		}
		
		pml.previous_walking = qfalse;
		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
			}
		}
	}
}



/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
	trace_t	trace;
	bg_q3f_playerclass_t *cls;

	cls = BG_Q3F_GetClass(pm->ps);

	pm->mins[0] = cls->mins[0];
	pm->mins[1] = cls->mins[1];

	pm->maxs[0] = cls->maxs[0];
	pm->maxs[1] = cls->maxs[1];

	pm->mins[2] = MINS_Z;

	if (pm->ps->pm_type == PM_DEAD )
	{
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	if (pm->cmd.upmove < 0)
	{	// duck
		pm->ps->pm_flags |= PMF_DUCKED;
	} else {	// stand up if possible
		if (pm->ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->maxs[2] = cls->maxs[2];
			pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );

			if (!trace.allsolid)
				pm->ps->pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		// DrEvil: make the crouch box actually contain the mini, and not have him hang out of it
		switch(pm->ps->persistant[PERS_CURRCLASS])
		{
		case Q3F_CLASS_RECON:
		case Q3F_CLASS_SNIPER:
			pm->maxs[2] = cls->maxs[2] * 0.7;
			break;
		case Q3F_CLASS_MINIGUNNER:
			pm->maxs[2] = cls->maxs[2] * 0.85;
			break;
		default:
			pm->maxs[2] = cls->maxs[2] * 0.75;		
		}
			
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else
	{
		pm->maxs[2] = cls->maxs[2];
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}

//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
	float		bobmove;
	int			old;
	qboolean	footstep;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	// special animations
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) ||
		pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) ) {
		pm->ps->bobCycle = 0;	// start at beginning of cycle again
		PM_ContinueLegsAnim( ANI_SPECIAL );
		return;
	}

	if( !pml.ladder && pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		/*if ( pm->ps->powerups[PW_INVULNERABILITY] ) {
			PM_ContinueLegsAnim( LEGS_IDLECR );
		}*/
		// airborne leaves position in cycle intact, but doesn't advance
		if ( pm->waterlevel > 1 ) {
			PM_ContinueLegsAnim( ANI_MOVE_SWIM );
		}
		return;
	}

	// if not trying to move
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		if (  pm->xyspeed < 5 ) {
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm->ps->pm_flags & PMF_DUCKED ) {
				PM_ContinueLegsAnim( ANI_MOVE_IDLECROUCH );
			} else {
				PM_ContinueLegsAnim( ANI_MOVE_IDLESTAND );
			}
		}
		return;
	}
	

	footstep = qfalse;

	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		bobmove = 0.5;	// ducked characters bob much faster
		//if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
		//	PM_ContinueLegsAnim( LEGS_BACKCR );
		//}
		//else {
			PM_ContinueLegsAnim( ANI_MOVE_WALKCROUCH );
		//}
		// ducked characters never play footsteps
	/*
	} else 	if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4;	// faster speeds bob faster
			footstep = qtrue;
		} else {
			bobmove = 0.3;
		}
		PM_ContinueLegsAnim( LEGS_BACK );
	*/
	} else {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4f;	// faster speeds bob faster
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
				PM_ContinueLegsAnim( ANI_MOVE_WALKBACK );
			}
			else {
				PM_ContinueLegsAnim( ANI_MOVE_RUN );
			}
			footstep = qtrue;
		} else {
			bobmove = 0.3f;	// walking bobs slow
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
				PM_ContinueLegsAnim( ANI_MOVE_WALKBACK );
			}
			else {
				PM_ContinueLegsAnim( ANI_MOVE_WALK );
			}	
		}
	}

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
		if ( pm->waterlevel == 0 ) {
			if ( PM_FootprintForSurface() ) {
				if ( footstep && !pm->noFootsteps ) {
					BG_AddPredictableEventToPlayerstate( PM_FootstepForSurface(), 1, pm->ps ); // RR2DO2: add footprints
				} else{
					BG_AddPredictableEventToPlayerstate( EV_FOOTSTEP, 2, pm->ps ); // RR2DO2: add footprints but no sound
				}
			} else {
				// on ground will only play sounds if running
				if ( footstep && !pm->noFootsteps ) {
					PM_AddEvent( PM_FootstepForSurface() );
				}
			}
		} else if ( pm->waterlevel == 1 ) {
			// splashing
			PM_AddEvent( EV_FOOTSPLASH );
		} else if ( pm->waterlevel == 2 ) {
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
		} else if ( pm->waterlevel == 3 ) {
			// no sound when completely underwater

		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent( EV_WATER_LEAVE );
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		if(pm->airleft < 6000) {
			PM_AddEvent( EV_WATER_CLEAR );
		}
	}
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
	if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		return;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		return;
	}
	
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		return;
	}

	PM_AddEvent( EV_CHANGE_WEAPON );
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += 90;
	PM_StartTorsoAnim( ANI_INTERACT_WPCHANGE_START );
}

/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
	int		weapon;
#ifdef CGAME
	gitem_t* weaponitem;
#endif

	weapon = pm->cmd.weapon;
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		weapon = WP_NONE;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 120;
	PM_StartTorsoAnim( ANI_INTERACT_WPCHANGE_END );

#ifdef CGAME
	weaponitem = BG_FindItemForWeapon( weapon );
	trap_Cvar_Set("ui_currentweapon", weaponitem->pickup_name);
#endif
}

/* JT */
/*
===================
PM_BeginWeaponReload
===================
*/
static void PM_BeginWeaponReload( void ) {
	PM_AddEvent( EV_COCK_WEAPON );
	pm->ps->weaponstate = WEAPON_RDROPPING;
	pm->ps->weaponTime += 200;
	pm->ps->stats[STAT_Q3F_FLAGS] &= ~(1<<Q3F_WEAPON_RELOAD);	// Reset Flag
	PM_ForceTorsoAnim( PM_GetReloadAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );
	pm->ps->torsoTimer = 200;
}
/* JT */

/*
===============
PM_FinishWeaponReload		-- JT
===============
*/
#ifdef CGAME
// RR2DO2 - ugly
extern void CG_Q3F_EndReload();
#endif

static void PM_FinishWeaponReload( void ) {
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 250;
	PM_StartTorsoAnim( PM_GetIdleAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );
	pm->ps->torsoTimer = 0;

#ifdef CGAME
// RR2DO2 - ugly
	CG_Q3F_EndReload();
#endif
}

static void PM_DoWeaponReload(void)
{
	int ammo;
	int curammo;
	float reloadtime;
	bg_q3f_weapon_t *wp;
	wp = BG_Q3F_GetWeapon(pm->ps->weapon);

	ammo = wp->clipsize;
	
	if(ammo > pm->ps->ammo[wp->ammotype]) {
		ammo = pm->ps->ammo[wp->ammotype];
	}
	
	// JT - Work out how many clips we're putting back in, and hence how long the delay must be.
	curammo = Q3F_GetClipValue(pm->ps->weapon, pm->ps);
	reloadtime = (wp->reloadtime)*((float)(ammo-curammo)/(float)ammo);
	pm->ps->weaponTime += reloadtime;
	pm->ps->weaponstate = WEAPON_RELOADING;
	Q3F_SetClipValue(pm->ps->weapon, ammo, pm->ps);
	pm->ps->torsoTimer += reloadtime;
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation( void ) {
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) ||
		pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) ) {
		PM_ContinueTorsoAnim( ANI_SPECIAL );
	} else if( pm->ps->extFlags & EXTF_ANI_OPERATING ) {	// operating has priority over throwing
		PM_ContinueTorsoAnim( ANI_INTERACT_OPERATING );
	} else if( pm->ps->extFlags & EXTF_ANI_THROWING ) {
		PM_ContinueTorsoAnim( ANI_INTERACT_THROW );
	} else if ( pm->ps->weaponstate == WEAPON_READY ) {
		PM_ContinueTorsoAnim( PM_GetIdleAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );
		return;
	}
}


/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) {
	int		addTime;
	int q3f_ammo_type;
	bg_q3f_playerclass_t *cls;
	bg_q3f_weapon_t *wp, *wp2;
	vec3_t testvel;
	qboolean inwater;
	//vec3_t point;

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
 		return;
	}
	// ignore if spectator
	if ( pm->ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->ps->weapon = WP_NONE;
		return;
	}

	// check for item using
	// Golliwog: Limit Medikit against class health
	cls = BG_Q3F_GetClass( pm->ps );
	wp = BG_Q3F_GetWeapon(pm->ps->weapon);
	q3f_ammo_type = wp->ammotype;

	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
		if ( ! ( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
			if ( bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
				&& pm->ps->stats[STAT_HEALTH] >= cls->maxhealth ) {
				// don't use medkit if at max health
			} else {
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent( EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag );
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}
			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}
	// Golliwog.


	// JT: Make _ABSOLUTELY_ sure that the aiming flag is only set if needed:

	if(pm->ps->weapon != WP_MINIGUN && pm->ps->weapon != WP_SNIPER_RIFLE)
		pm->ps->eFlags &= ~EF_Q3F_AIMING;

	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
	}

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	if (  (pm->ps->weaponTime <= 0 && pm->ps->weaponstate != WEAPON_AIMING) || pm->ps->weaponstate == WEAPON_RAISING ||
		pm->ps->weaponstate == WEAPON_DROPPING || pm->ps->weaponstate == WEAPON_READY) {
		//wp2 = BG_Q3F_GetWeapon( cls->weaponslot[pm->cmd.weapon] );
		wp2= BG_Q3F_GetWeapon( pm->cmd.weapon );
		if( pm->ps->weapon != pm->cmd.weapon &&
			(pm->ps->ammo[wp2->ammotype] >= wp2->numammo || !wp2->numammo ))
		{
			PM_BeginWeaponChange( pm->cmd.weapon );
		} 
	}
	
	if ( pm->cmd.wbuttons & WBUTTON_RELOAD)
		BG_Q3F_Request_Reload( pm->ps );

// JT ->
	if( pm->ps->weaponstate == WEAPON_READY && 
		(pm->ps->stats[STAT_Q3F_FLAGS] & (1 << Q3F_WEAPON_RELOAD))
		&& wp->clipsize && pm->ps->ammo[wp->ammotype] >= wp->numammo )
	{
		PM_BeginWeaponReload();
		return;
	}
// <- JT

	if ( pm->ps->weaponTime > 0 ) {
		return;
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		PM_FinishWeaponChange();
		return;
	}

// JT ->
	// If we're reloading, start the reload timer.
	if(pm->ps->weaponstate == WEAPON_RDROPPING) {
		PM_DoWeaponReload();
		return;
	}

	if(pm->ps->weaponstate == WEAPON_RELOADING) {
		PM_FinishWeaponReload();	
		return;
	}
// <- JT

	if((pm->ps->weapon == 0) && (pm->ps->weaponstate != 0))
	{
		Com_Printf("DrEvil, your bot has no weapon!!??\n");
		pm->ps->weapon = WP_AXE;
		//return;
	}

	if ( pm->ps->weaponstate == WEAPON_RAISING ||
		pm->ps->weaponstate == WEAPON_RRAISING) {
		pm->ps->weaponstate = WEAPON_READY;
		PM_StartTorsoAnim( PM_GetIdleAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );
		return;
	}
	
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) )
		return;		// Golliwog: Laying a charge, can't fire
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) )
		return;		// Golliwog: Building something
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_MOVING) )
	{
		PM_AddEvent( EV_PLACE_BUILDING );
		return;		// Ensiform: Moving something
	}
	if( pm->ps->pm_flags & PMF_CEASEFIRE )
		return;		// No weapons activity if in ceasefire mode.
	if((pm->cmd.buttons & BUTTON_ATTACK) && wp->fire_on_release && pm->ps->ammo[q3f_ammo_type] >= wp->numammo)		// JT - Don't do it if we're out of ammo.
	{
		if(pm->ps->weaponstate != WEAPON_AIMING && pm->ps->groundEntityNum != ENTITYNUM_NONE )
		{
			VectorCopy( pm->ps->velocity, testvel );
			testvel[2] = 0;
            if( VectorLength( testvel ) < 100 )
			{
				pm->ps->weaponstate = WEAPON_AIMING;
				pm->ps->eFlags |= EF_Q3F_AIMING;
			}
		}
		return;
	}

	// RR2DO2: Check for water
	//VectorCopy( pm->ps->origin, point );
	//point[2] += 3;	// lift some up due to swimming bob up/down
	//inwater = ( pm->pointcontents( point, pm->ps->clientNum ) & MASK_WATER );
	inwater = ( pm->waterlevel == 3 ) ? qtrue : qfalse;

	// JT: WEAPON_STARTING/STARTED support for weapons
	if((pm->cmd.buttons & BUTTON_ATTACK) && wp->inform_on_start && pm->ps->ammo[q3f_ammo_type] >= wp->numammo)	// JT - Don't do it if we're out of ammo
	{
		if(pm->ps->weaponstate != WEAPON_STARTING)
		{
			if(pm->ps->weaponstate != WEAPON_STARTED)
			{
				if( pm->ps->weapon == WP_MINIGUN ) {
					if( pm->ps->ammo[AMMO_CELLS] > 3 )
					{
						pm->ps->weaponstate = WEAPON_STARTING;
						pm->ps->weaponTime = Q3F_MINIGUN_WARMUP_TIME;
//						PM_AddEvent( EV_Q3F_MINIGUN_START );
						pm->ps->eFlags |= EF_Q3F_AIMING;
						PM_StartTorsoAnim( PM_GetAttackAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );
						pm->ps->ammo[AMMO_CELLS] -=4;
						return;
					}
					else
					{
						pm->ps->weaponstate = WEAPON_READY;
						pm->ps->weaponTime = 500;
						return;
					}
				} else if ( pm->ps->weapon == WP_FLAMETHROWER ) {
					if( !inwater ) {
						pm->ps->weaponstate = WEAPON_STARTED;
					}
					return;
				}
			}

			// RR2DO2: Can only fire when standing on ground
			if( pm->ps->weapon == WP_MINIGUN && pm->ps->groundEntityNum == ENTITYNUM_NONE )
			{
				return;
				/*pm->ps->weaponTime = Q3F_MINIGUN_WARMUP_TIME;
				pm->ps->weaponstate = WEAPON_STARTING;	// Golliwog: Can't fire in water */
			}
		}
		else
		{
			if( pm->ps->weapon == WP_MINIGUN ) {
				if( pm->ps->groundEntityNum != ENTITYNUM_NONE && pm->ps->weaponTime <= 0 )	// Golliwog: Can't fire in water
					pm->ps->weaponstate = WEAPON_STARTED;
				else
					return;
			}
		}
	}


	// check for fire
	if ( !(pm->cmd.buttons & BUTTON_ATTACK)) {
		if(pm->ps->weaponstate != WEAPON_AIMING)
		{
			pm->ps->eFlags &= ~ EF_Q3F_AIMING;
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		}
		else
		{
			/* Goint from WEAPON_AIMING to ready, remove aiming flag */
			pm->ps->eFlags &= ~EF_Q3F_AIMING;
			pm->ps->weaponstate = WEAPON_READY;
		}
	}

// JT - At this point... we want to fire.

	// RR2DO2 sniperrifle
	if( pm->ps->groundEntityNum == ENTITYNUM_NONE && pm->ps->weapon == WP_SNIPER_RIFLE) {
		if(pm->ps->weaponstate != WEAPON_STARTED)	// JT - Don't override 'started' as a state.
			pm->ps->weaponstate = WEAPON_FIRING;
		addTime = wp->firetime;
		if(pm->ps->stats[STAT_Q3F_FLAGS] & (1<<FL_Q3F_TRANQ))
			addTime *=2;
		if ( pm->ps->powerups[PW_HASTE] ) {
			addTime /= 1.3;
		}
		pm->ps->weaponTime = addTime;
		PM_AddEvent( EV_FIRE_WEAPON ); // RR2DO2: we try to... but...
		return;		// RR2DO2: Can't fire rifle when not on ground
	}

// JT ->
	//canabis, simplify this one, this only checks if someone started a reload request
	if( pm->ps->stats[STAT_Q3F_FLAGS] & (1<< Q3F_WEAPON_RELOAD)
		&& wp->clipsize != 0 && pm->ps->ammo[wp->ammotype] >= wp->numammo)
	{
		PM_BeginWeaponReload();
		return;
	}
// <- JT

	// start the animation even if out of ammo
	if(pm->ps->weapon == WP_FLAMETHROWER)
	{
		if( inwater )
		{
			pm->ps->weaponTime +=500;
			if( pm->ps->weaponstate != WEAPON_READY ) {
				pm->ps->weaponstate = WEAPON_READY;
			}
			//PM_AddEvent(EV_GUNCHOKED);
			return;
		}
	}

	// start the animation
	PM_StartTorsoAnim( PM_GetAttackAnim( pm->ps->weapon, pm->ps->persistant[PERS_CURRCLASS] ) );

	if(pm->ps->weaponstate != WEAPON_STARTED)	// JT - Don't override 'started' as a state.
		pm->ps->weaponstate = WEAPON_FIRING;

	// check for out of ammo

	if ( pm->ps->ammo[ q3f_ammo_type] < wp->numammo && wp->ammotype != AMMO_NONE)
	{
		PM_AddEvent( EV_NOAMMO );
		pm->ps->weaponTime += 500;
		return;
	}

	/* Check for auto reloading with clipped weapons */
	if( !Q3F_GetClipValue( pm->ps->weapon, pm->ps ) &&  wp->clipsize )		// JT - If we're out of clip... do the nasty thing.
	{
		//canabis, check if we have reload on firing enabled
		if (pm->ps->persistant[PERS_FLAGS]&PF_AUTORELOAD) {
			PM_BeginWeaponReload();
		} else {
			pm->ps->weaponTime+=300;			//Slight delay for the next check
			PM_AddEvent(EV_COCK_WEAPON);
		}
		return;
	}
	
	// JT: Don't allow the fire anim if you're trying to move.
	if((pm->cmd.forwardmove || pm->cmd.rightmove) && pm->ps->weapon == WP_MINIGUN )//wp->inform_on_start)
	{
		return;
	}
	// take ammo away

	pm->ps->ammo[ q3f_ammo_type ] -= wp->numammo;

	// take a clip away, too
	if(wp->clipsize)
		Q3F_SetClipValue(pm->ps->weapon,Q3F_GetClipValue(pm->ps->weapon,pm->ps)-wp->numammo,
					pm->ps);

	// fire weapon
	PM_AddEvent( EV_FIRE_WEAPON );


	// JT These delays are be fetched from the weapon structure instead.

	addTime = wp->firetime;

	if(pm->ps->stats[STAT_Q3F_FLAGS] & (1<<FL_Q3F_TRANQ))
		addTime *=2;

	if ( pm->ps->powerups[PW_HASTE] ) {
		addTime /= 1.3;
	}

	pm->ps->weaponTime = addTime;
}

// RR2DO2: these functions get the duration of taunt/gesture/signal animations - hardcoded, so update when f2r's change
// FIXME: disguised agent taunts

#define ANI_SIGNAL_DURATION(numframes,fps) ((int)((1000.f/fps)*numframes+50.f))
static int PM_GetTauntDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(31.f,12.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(20.f,10.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(20.f,12.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(10.f,16.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(18.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(24.f,22.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(20.f,10.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(11.f,12.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(20.f,10.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(40.f,20.f));
	}
	return 0;
}

static int PM_GetPointDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(4.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(9.f,16.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(4.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(4.f,14.f));
	}
	return 0;
}

static int PM_GetStopDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(4.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(9.f,16.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(4.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(4.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(4.f,14.f));
	}
	return 0;
}

static int PM_GetBeconDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(11.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(12.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(12.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(12.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(19.f,26.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(12.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(12.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(12.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(12.f,14.f));
	}
	return 0;
}

static int PM_GetLookDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(10.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(18.f,16.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(10.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(10.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(10.f,14.f));
	}
	return 0;
}

static int PM_GetWaveYesDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(6.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(12.f,16.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(7.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(7.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(6.f,14.f));
	}
	return 0;
}

static int PM_GetWaveNoDuration( void ) {
	int classNum = pm->agentclass ? pm->agentclass : pm->ps->persistant[PERS_CURRCLASS];
	switch( classNum ) {
	case Q3F_CLASS_RECON:			return(ANI_SIGNAL_DURATION(11.f,14.f));
	case Q3F_CLASS_SNIPER:			return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_SOLDIER:			return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_GRENADIER:		return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_PARAMEDIC:		return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_MINIGUNNER:		return(ANI_SIGNAL_DURATION(17.f,16.f));
	case Q3F_CLASS_FLAMETROOPER:	return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_AGENT:			return(ANI_SIGNAL_DURATION(11.f,15.f));
	case Q3F_CLASS_ENGINEER:		return(ANI_SIGNAL_DURATION(11.f,12.f));
	case Q3F_CLASS_CIVILIAN:		return(ANI_SIGNAL_DURATION(10.f,14.f));
	}
	return 0;
}

static void PM_Animate( void ) {
	/* Don't start a new animation if one is still playing */
	int gesture;
	if ( pm->ps->torsoTimer )
		return;
	if (pm->retflags & PMRF_SKIP_TAUNT) {
		return;
	}
	gesture = pm->cmd.flags & UCMDF_GESTURE_MASK;
	if (!gesture) 
		return;
	pm->retflags |= PMRF_DONE_TAUNT;
	switch ( gesture ) {
	case GESTURE_TAUNT:
		PM_StartTorsoAnim( ANI_SIGNAL_TAUNT );
		pm->ps->torsoTimer = PM_GetTauntDuration();
		PM_AddEvent( EV_TAUNT );
		break;
	case GESTURE_WAVENO:
		PM_StartTorsoAnim( ANI_SIGNAL_WAVENO );
		pm->ps->torsoTimer = PM_GetWaveNoDuration();
		break;
	case GESTURE_WAVEYES:
		PM_StartTorsoAnim( ANI_SIGNAL_WAVEYES );
		pm->ps->torsoTimer = PM_GetWaveYesDuration();
		break;
	case GESTURE_LOOK:
		PM_StartTorsoAnim( ANI_SIGNAL_LOOK );
		pm->ps->torsoTimer = PM_GetLookDuration();
		break;
	case GESTURE_BECON:
		PM_StartTorsoAnim( ANI_SIGNAL_BECON );
		pm->ps->torsoTimer = PM_GetBeconDuration();
		break;
	case GESTURE_STOP:
		PM_StartTorsoAnim( ANI_SIGNAL_STOP );
		pm->ps->torsoTimer = PM_GetStopDuration();
		break;
	case GESTURE_POINT:
		PM_StartTorsoAnim( ANI_SIGNAL_POINT );
		pm->ps->torsoTimer = PM_GetPointDuration();
		break;
	}
}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
	short		temp;
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->pm_type != PM_ADMINSPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;		// no view changes at all
	}

//	if( ps->pm_type == PM_INVISIBLE )
//	{
//		// Golliwog: Stop movement AND stop it tracking movement.
//		return;
//	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}

}


/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove) {
	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}

	pm->ps->eFlags &= ~EF_FIRING;

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && 
		!( pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) && pm->cmd.upmove < 10 ) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	// Set the maximum speed
	pml.maxSpeed = pm->ps->speed;
	if ( pm->ps->eFlags & EF_Q3F_AIMING ) {
		if (pm->ps->weapon == WP_SNIPER_RIFLE ) 
			pml.maxSpeed *= pm_sniperAimScale;
		else if (pm->ps->weapon == WP_MINIGUN ) 
			pml.maxSpeed *= pm_minigunSpinScale;
	} 

	// RR2DO2 extended spectator
	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_CheckDuck ();
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}
	else if ( pm->ps->pm_type == PM_LIMITEDSPECTATOR ) {
		PM_CheckDuck ();
		//PM_FlyMove ();		// Golliwog: Prevent spectators moving (i.e. studying enemy positions)
		PM_DropTimers ();
		return;
	}
	else if ( pm->ps->pm_type == PM_ADMINSPECTATOR ) {
		PM_CheckDuck ();
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}
	// RR2DO2

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	// set groundentity
	PM_GroundTrace();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	PM_DropTimers();
	CheckLadder();  // ARTHUR TOMLIN check and see if they're on a ladder

	if ( pm->ps->powerups[PW_FLIGHT] ) {
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
	} else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
		PM_WaterJumpMove();
	} else if (pml.ladder) {	
		PM_LadderMove();
	} else if ( pm->waterlevel > 1 ) {
		// swimming
		PM_WaterMove();
	} else if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove( 0 );
	}

	PM_Animate();

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();
	if (pm->ps->weaponstate >= WEAPON_FIRING)
		pm->ps->eFlags |= EF_FIRING;

	// torso animation
	PM_TorsoAnimation();

	// footstep events / legs animations
	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}

	//PM_CheckStuck();

}
