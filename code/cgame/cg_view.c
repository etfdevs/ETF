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

// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"
#include "cg_q3f_grenades.h"
#include "cg_q3f_sounddict.h"
#include "cg_q3f_menu.h"
#include "cg_q3f_mapselect.h"
#include "../game/bg_q3f_playerclass.h"
#include "../game/bg_q3f_splines.h"

/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/
void OmnibotRenderDebugLines();
/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

	if ( trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if (! cg.testModelEntity.hModel ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Can't register model\n" );
		return;
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( BOX_PRINT_MODE_CHAT, "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( BOX_PRINT_MODE_CHAT, "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( BOX_PRINT_MODE_CHAT, "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( BOX_PRINT_MODE_CHAT, "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
	int		i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		CG_Printf (BOX_PRINT_MODE_CHAT, "Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
		}
	}

	trap_R_AddRefEntityToScene( &cg.testModelEntity, NULL );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
void CG_CalcVrect (void) {
	int		size;

	// the intermission should allways be full screen
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		size = 100;
	} else {
		// bound normal viewsize
		if (cg_viewsize.integer < 30) {
			trap_Cvar_Set ("cg_viewsize","30");
			size = 30;
		} else if (cg_viewsize.integer > 100) {
			trap_Cvar_Set ("cg_viewsize","100");
			size = 100;
		} else {
			size = cg_viewsize.integer;
		}

	}
	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
	vec3_t		forward, right, up;
	vec3_t		view;
	vec3_t		focusAngles;
	trace_t		trace;
	static vec3_t	mins = { -4, -4, -4 };
	static vec3_t	maxs = { 4, 4, 4 };
	vec3_t		focusPoint;
	float		focusDist;
	float		forwardScale, sideScale;

	cg.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;

	VectorCopy( cg.refdefViewAngles, focusAngles );

	// if dead, look at killer
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
		cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;		// don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

	VectorCopy( cg.refdef.vieworg, view );

	view[2] += 8;

	cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors( cg.refdefViewAngles, forward, right, up );

	forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
	sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
	VectorMA( view, -cg_thirdPersonRange.value * forwardScale, forward, view );
	VectorMA( view, -cg_thirdPersonRange.value * sideScale, right, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	/*if (!cg_cameraMode.integer)*/ {
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

		if ( trace.fraction != 1.0 ) {
			VectorCopy( trace.endpos, view );
			view[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enogh that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
			VectorCopy( trace.endpos, view );
		}
	}


	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
	int		timeDelta;
	
	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < STEP_TIME ) {
		cg.refdef.vieworg[2] -= cg.stepChange 
			* (STEP_TIME - timeDelta) / STEP_TIME;
	}
}

// Golliwog: A pretty effect when you've been concussed. Duh :)
/*
===============
CG_Q3F_ConcussionEffect

===============
*/
/*static void CG_Q3F_ConcussionEffect( void )
{
	vec3_t angleoffset;

	if( cg.snap->ps.powerups[PW_Q3F_CONCUSS] < cg.time )
		return;		// Not concussed
	if( cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL )
		return;		// Don't concuss dead players/spectators/players without a class

	BG_Q3F_ConcussionEffect( cg.snap->ps.generic1, cg.snap->ps.powerups[PW_Q3F_CONCUSS] - cg.time, angleoffset);
	// And update the viewangles
	cg.refdefViewAngles[0] += angleoffset[0];
	cg.refdefViewAngles[1] += angleoffset[1];
}*/
// Golliwog.

void CG_Q3F_ConcussionEffect2( int* x, int* y )
{
	vec3_t angleoffset;float temp;

	*x = 0;*y = 0;
	if( cg.snap->ps.powerups[PW_Q3F_CONCUSS] < cg.time )
		return;		// Not concussed
	if( cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL )
		return;		// Don't concuss dead players/spectators/players without a class

	BG_Q3F_ConcussionEffect( cg.snap->ps.generic1, cg.snap->ps.powerups[PW_Q3F_CONCUSS] - cg.time, angleoffset);

	temp = sin( angleoffset[YAW] * M_PI / 180 );
	*x = - ((( temp * 90.f / cg_fov.integer) * SCREEN_HEIGHT) * cgs.screenXScale) * 0.40f;

	temp = sin( angleoffset[PITCH] * M_PI / 180 );
	*y = ((( temp * 90.f / cg_fov.integer ) * SCREEN_WIDTH) * cgs.screenYScale) * 0.40f;
}

// Golliwog: A (not-very-)pretty effect when you've been gassed
/*
===============
CG_Q3F_GasEffect

===============
*/
static void CG_Q3F_GasEffect( void )
{
	int gastime, index;
	float curr;
	//qboolean flash;

	if( cg.snap->ps.powerups[PW_Q3F_GAS] < cg.time ||
		cg.snap->ps.stats[STAT_HEALTH] <= 0 ||
		cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR ||
		cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL )
	{
		cg.gasEndTime = 0;
		cg.gasCurrColour[3] = 0;
		return;		// Not gassed, dead, or a spectator
	}

	gastime = cg.snap->ps.powerups[PW_Q3F_GAS];

	//flash = qfalse;
	if( cg.time >= cg.gasEndTime )
	{
		// Time to do a new 'random' effect.
		if( !cg.gasEndTime )
		{
			// It's an entirely new gassing

			cg.gasTime = cg.time;
			cg.gasColour[0] = cg.gasColour[1] = cg.gasColour[2] = 0.5;
			cg.gasColour[3] = 0;
		}
		else {
			cg.gasTime = cg.gasEndTime;
			memcpy( cg.gasColour, cg.gasEndColour, sizeof(cg.gasColour) );
		}
		cg.gasEndTime = cg.time + 3450 + Q_flrand(0.0f, 1.0f) * 3450;
		if( (cg.time + 3450) >= gastime || cg.gasEndTime >= gastime )
		{
			// Effect is ending
			cg.gasEndTime = gastime;
			cg.gasEndColour[0] = cg.gasEndColour[1] = cg.gasEndColour[2] = 0.5;
			cg.gasEndColour[3] = 0;

			for( index = 0; index < MAX_CLIENTS; index++ )
			{
				if( !cgs.clientinfo[index].infoValid )
					continue;
				cg.gasPlayerClass[index] = cg.gasPlayerTeam[index] = 0xFF;	// "No effect"
			}
		}
		else {
			cg.gasEndColour[0] = Q_flrand(0.0f, 1.0f);
			cg.gasEndColour[1] = Q_flrand(0.0f, 1.0f);
			cg.gasEndColour[2] = Q_flrand(0.0f, 1.0f);
			cg.gasEndColour[3] = 0.34 + Q_flrand(0.0f, 1.0f) * 0.3 * (((float)(gastime - cg.time)) / 20000);
			if( cg.gasEndColour[3] > 0.7f )
				cg.gasEndColour[3] = 0.7f;

			// Re-randomize all the users
			for( index = 0; index < MAX_CLIENTS; index++ )
			{
				if( !cgs.clientinfo[index].infoValid )
					continue;
				// RR2DO2: don't randomize ourselves
				if ( index == cg.snap->ps.clientNum ) {
					continue;
				}
				// RR2DO2
				while( 1 )
				{
					cg.gasPlayerClass[index] = Q3F_CLASS_RECON + (rand() % (Q3F_CLASS_CIVILIAN-Q3F_CLASS_RECON));
					if( cgs.media.modelcache[cg.gasPlayerClass[index]][0] )
						break;	// This class is cached
				}
				while( 1 )
				{
					cg.gasPlayerTeam[index] = Q3F_TEAM_RED + (int) (Q_flrand(0.0f, 1.0f) * 4);
					//if( cgs.media.skincache[cg.gasPlayerClass[index]][cg.gasPlayerTeam[index]-Q3F_TEAM_RED][0] )
					if( cgs.media.skincache[cg.gasPlayerClass[index]][0] )
						break;	// This skin is cached
				}
			}
		}
		cg.gasFlashTime = cg.time + 100;		// Long enough to assure a 'solid' flash
	}

		// Calculate a smooth transition from start to end
	curr = 0.5 - 0.5 * sin( M_PI * (((float)(cg.gasEndTime - cg.time)) / ((float)(cg.gasEndTime - cg.gasTime)) - 0.5) );
	for( index = 0; index < 4; index++ )		// Screen colour (used later in frame render)
		cg.gasCurrColour[index] = cg.gasColour[index] + curr * (cg.gasEndColour[index] - cg.gasColour[index]);
	if( cg.gasFlashTime > cg.time )
		cg.gasCurrColour[3] = 1;		// "Flash" the screen to opaque this frame
}

// Golliwog: A pretty effect to handle 'vibration'.
/*
===============
CG_Q3F_VibrationEffect

===============
*/
static void CG_Q3F_VibrationEffect( void )
{
	float magnitude, time;

	if( !cg.vibrateTime )
		return;

	time = (float)(cg.time - cg.vibrateTime);
	if( time < 20 )
		time = 20;
	magnitude = ((cg.vibrateMagnitude > 100) ? 100 : cg.vibrateMagnitude) / (time / 20);
	if( magnitude <= 3 )
	{
		// There's not enough vibration to care about
		cg.vibrateTime = 0;
		return;
	}
//	time = 1000 / time;
//	magnitude *= sin( 4 * cg.vibrateOffset + SQRTFAST(time) );
//	magnitude *= sin( cg.vibrateOffset + pow( time * 200, 0.5 ) );

	cg.refdef.vieworg[2] += magnitude * 0.3 * sin( 1.4 * (cg.vibrateOffset + SQRTFAST(time)) );
	cg.refdefViewAngles[ROLL] += magnitude * (0.1 * sin( 0.1 * (cg.vibrateOffset + SQRTFAST(time)) ) - 0.05);
}

// Golliwog.


// Jules: Minigun shudder
/*
===============
CG_Q3F_ShudderEffect

===============
*/
static void CG_Q3F_ShudderEffect( void )
{
	int index = cg.time - cg.minigunLast;
	if ( index < 800) {
		float magnitude = (0.3/800.0) * cg.minigunHeat * (800 - index) ;
		cg.refdefViewAngles[0] += 0.10 * magnitude * sin( cg.time * 0.05 );
		cg.refdefViewAngles[0] += 0.10 * magnitude * cos( cg.time * 0.04 );
		cg.refdefViewAngles[1] += 0.40 * magnitude * cos( cg.time * 0.05 );
		cg.refdefViewAngles[1] += 0.30 * magnitude * sin( cg.time * 0.04 );
	}
}


/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;
	float			bob;
	float			ratio;
	float			delta;
	float			speed;
	float			f;
	vec3_t			predictedVelocity;
	int				timeDelta;
	
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	// add angles based on weapon kick
	VectorAdd (angles, cg.kick_angles, angles);

	// add angles based on damage kick
	if ( cg.damageTime ) {
		ratio = cg.time - cg.damageTime;
		if ( ratio < DAMAGE_DEFLECT_TIME ) {
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		} else {
			ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
			if ( ratio > 0 ) {
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = ( cg.time - cg.landTime) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
	angles[PITCH] += delta * cg_runpitch.value;
	
	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
	angles[ROLL] -= delta * cg_runroll.value;

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

	delta = cg.bobfracsin * cg_bobpitch.value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching
	angles[PITCH] += delta;
	delta = cg.bobfracsin * cg_bobroll.value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching accentuates roll
	if (cg.bobcycle & 1)
		delta = -delta;
	angles[ROLL] += delta;

//===================================

	// add view height
	origin[2] += cg.predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if ( timeDelta < DUCK_TIME) {
		cg.refdef.vieworg[2] -= cg.duckChange 
			* (DUCK_TIME - timeDelta) / DUCK_TIME;
	}

	// add bob height
	bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;
	if (bob > 6) {
		bob = 6;
	}
	if (bob < 0) {
		bob = 0;
	}

	origin[2] += bob;


	// RR2DO2: Some 0lD-Sk00L TF dweeps don't like this...
	if ( cg_fallingBob.value ) {
		// add fall height
		delta = cg.time - cg.landTime;
		if ( delta < LAND_DEFLECT_TIME ) {
			f = delta / LAND_DEFLECT_TIME;
			cg.refdef.vieworg[2] += cg.landChange * f;
		} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
			delta -= LAND_DEFLECT_TIME;
			f = 1.0 - ( delta / LAND_RETURN_TIME );
			cg.refdef.vieworg[2] += cg.landChange * f;
		}
	}
	// RR2DO2

	// add step offset
	CG_StepOffset();

	// add kick offset

	VectorAdd (origin, cg.kick_origin, origin);

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
	vec3_t			forward, up;
 
	cg.refdef.vieworg[2] -= NECK_LENGTH;
	AngleVectors( cg.refdefViewAngles, forward, NULL, up );
	VectorMA( cg.refdef.vieworg, 3, forward, cg.refdef.vieworg );
	VectorMA( cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg );
	}
#endif
}

//======================================================================

void CG_ZoomDown_f( void ) { 
	if ( cg.zoomed ) {
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f( void ) { 
	if ( !cg.zoomed ) {
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
	float	x;
	float	phase;
	float	v;
	int		contents;
	float	fov_x, fov_y;
	float	zoomFov;
	float	f;
	int		inwater;

	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a fixed value
		fov_x = 90;
	} else {
		// user selectable
		if ( cgs.dmflags & DF_FIXED_FOV ) {
			// dmflag to prevent wide fov for all clients
			fov_x = 90;
		} else {
			fov_x = cg_fov.value;
			if ( fov_x < 1 ) {
				fov_x = 1;
			} else if ( fov_x > 160 ) {
				fov_x = 160;
			}
		}

		// account for zooms
		zoomFov = cg_zoomFov.value;
		if ( zoomFov < 1 ) {
			zoomFov = 1;
		} else if ( zoomFov > 160 ) {
			zoomFov = 160;
		}

		if ( cg.zoomed ) {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		// Golliwog: Sniper autozoom
		} else if( cg.autoZoomed ) {
			f = ( cg.time - cg.autoZoomTime ) / (float)AUTOZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		}
		// Golliwog
		else {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = fov_x;
			} else {
				fov_x = zoomFov + f * ( fov_x - zoomFov );
			}
		}
	}

	if ( cg_fovAspectAdjust.integer ) {
		// Based on LordHavoc's code for Darkplaces
		// http://www.quakeworld.nu/forum/topic/53/what-does-your-qw-look-like/page/30
		const float baseAspect = 0.75f; // 3/4
		const float aspect = (float)cgs.glconfig.vidWidth / (float)cgs.glconfig.vidHeight;
		const float desiredFov = fov_x;

		fov_x = atan( tan( desiredFov*M_PI / 360.0f ) * baseAspect*aspect )*360.0f / M_PI;
	}

	x = cg.refdef.width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( cg.refdef.height, x );
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin( phase );
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
		cg.refdef_current->rdflags |= RDF_UNDERWATER;
	}
	else {
		cg.refdef_current->rdflags &= ~RDF_UNDERWATER;
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if ( !cg.zoomed && !cg.autoZoomed ) {
		cg.zoomSensitivity = 1;
	} else {
		cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
	}

	return inwater;
}



/*
===============
CG_DamageBlendBlob

===============
*/
/*static void CG_DamageBlendBlob( void ) {
	int			t;
	int			maxTime;
	refEntity_t		ent;

	if ( !cg.damageValue ) {
		return;
	}*/

/*	if (cg.cameraMode) {
		return;
	}*/

/*	// ragePro systems can't fade blends, so don't obscure the screen
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		return;
	}

	maxTime = DAMAGE_TIME;
	t = cg.time - cg.damageTime;
	if ( t <= 0 || t >= maxTime ) {
		return;
	}


	memset( &ent, 0, sizeof( ent ) );
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_FIRST_PERSON;

	VectorMA( cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin );
	VectorMA( ent.origin, cg.damageX * -8, cg.refdef.viewaxis[1], ent.origin );
	VectorMA( ent.origin, cg.damageY * 8, cg.refdef.viewaxis[2], ent.origin );

	ent.radius = cg.damageValue * 3;
	ent.customShader = cgs.media.viewBloodShader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 200 * ( 1.0 - ((float)t / maxTime) );
	trap_R_AddRefEntityToScene( &ent, NULL );
}*/

// RR2DO2
/*
===============
CG_Q3F_InterpolateAngles

Takes two angles, a duration, starttime and time and returns current angle
===============
*/
static void CG_Q3F_InterpolateAngles( vec3_t src_angle, vec3_t dst_angle, int startTime, int Duration, int atTime, vec3_t result ) {
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
}

/*
===============
CG_Q3F_OffsetFlyBy
===============
*/
static void CG_Q3F_OffsetFlyBy ( void ) {
	// RR2DO2: testcsplinestuff
	if (cgs.flybyPathIndex >= 0 ) {
		if (cgs.campaths[cgs.flybyPathIndex].camtraj.trTime + cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration - cg.frametime < cg.time) {
			vec3_t vec_angle;

			if ( cgs.campaths[cgs.flybyPathIndex].currtrajindex >= 0 )
				trap_SendClientCommand( va( "flyby nexttraj %i", cg.time + (int)cl_timeNudge.value - 2 * cg.frametime )  );

			cgs.campaths[cgs.flybyPathIndex].currtrajindex++;
			if ( cgs.campaths[cgs.flybyPathIndex].currtrajindex >= cgs.campaths[cgs.flybyPathIndex].numsplines )
				cgs.campaths[cgs.flybyPathIndex].currtrajindex = 0;
			VectorCopy( cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].SegmentVtx[0], cgs.campaths[cgs.flybyPathIndex].camtraj.trBase );
			cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration = BG_Q3F_CubicSpline_Length(&cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex])/cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].speed * 1000;
			cgs.campaths[cgs.flybyPathIndex].camtraj.trTime = cg.time;
			cgs.campaths[cgs.flybyPathIndex].camtraj.trType = TR_CUBIC_SPLINE_PATH;

			// Origin
			VectorCopy( cgs.campaths[cgs.flybyPathIndex].camtraj.trBase, cg.refdef.vieworg );

			//CG_Printf("ClientOrigin %f %f %f << FIRST POINTS SPLINE\n", cg.refdef.vieworg[0], cg.refdef.vieworg[1], cg.refdef.vieworg[2] );

			// Angles
			VectorSet( vec_angle, 200000, 200000, 200000 );
			if( !VectorCompare( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].lookat, vec_angle ) ) {
				VectorSubtract( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].lookat, cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[0], vec_angle );
			} else {
				VectorSubtract ( cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[1], cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[0], vec_angle );
			}
			vectoangles( vec_angle, cg.refdefViewAngles );
			cg.refdefViewAngles[ROLL] = cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].roll;
		} else {
			vec3_t pos, vec_angle, vec_lookat, vec_lookat2;

			// Origin
			BG_Q3F_EvaluateSplineTrajectory( &cgs.campaths[cgs.flybyPathIndex].camtraj, NULL, &cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex], cg.time, pos );
			VectorCopy( pos, cg.refdef.vieworg );

			//CG_Printf("ClientOrigin %f %f %f\n", cg.refdef.vieworg[0], cg.refdef.vieworg[1], cg.refdef.vieworg[2] );

			// Angles
			VectorSet( vec_lookat, 200000, 200000, 200000 );
			VectorSet( vec_lookat2, 200000, 200000, 200000 );
			if( !VectorCompare( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].lookat, vec_lookat ) ) {
				VectorSubtract( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].lookat, cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[0], vec_lookat );
				if( !VectorCompare( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].next->lookat, vec_lookat2 ) ) {
					VectorSubtract( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].next->lookat, cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[3], vec_lookat2 ); // CP3 == CP0 of next spline
					CG_Q3F_InterpolateAngles( vec_lookat, vec_lookat2, cgs.campaths[cgs.flybyPathIndex].camtraj.trTime, cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration, cg.time, vec_angle );
				} else {
					VectorSubtract ( cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[3], cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[2], vec_lookat2 );
					CG_Q3F_InterpolateAngles( vec_lookat, vec_lookat2, cgs.campaths[cgs.flybyPathIndex].camtraj.trTime, cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration, cg.time, vec_angle );
				}
			} else if( !VectorCompare( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].next->lookat, vec_lookat2 ) ) {
				VectorSubtract( cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].next->lookat, cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[3], vec_lookat2 ); // CP3 == CP0 of next spline
				VectorSubtract ( cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[1], cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].ControlPoint[0], vec_lookat ); // We already know that this camspline hasn't got a lookat
				CG_Q3F_InterpolateAngles( vec_lookat, vec_lookat2, cgs.campaths[cgs.flybyPathIndex].camtraj.trTime, cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration, cg.time, vec_angle );
			} else {
				BG_Q3F_EvaluateSplineTrajectoryAngle( &cgs.campaths[cgs.flybyPathIndex].camtraj, NULL, &cgs.campaths[cgs.flybyPathIndex].splines[cgs.campaths[cgs.flybyPathIndex].currtrajindex], cg.time, vec_angle );
			}
			vectoangles( vec_angle, cg.refdefViewAngles );
			cg.refdefViewAngles[ROLL] = (float)cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].roll +
										( ((float)cg.time - (float)cgs.campaths[cgs.flybyPathIndex].camtraj.trTime) / (float)cgs.campaths[cgs.flybyPathIndex].camtraj.trDuration *
										AngleNormalize180((float)(cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].next->roll - cgs.campaths[cgs.flybyPathIndex].camsplines[cgs.campaths[cgs.flybyPathIndex].currtrajindex].roll)) );
		}
	}
}
// RR2DO2

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	cg.currentrefdef = &cg.refdef;

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;

/*	if (cg.cameraMode) {
		vec3_t origin, angles;
		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}*/

	// intermission view
	if ( ps->pm_type == PM_INTERMISSION ) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		return CG_CalcFov();
	}

	cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
	cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
	cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
		ps->velocity[1] * ps->velocity[1] );

	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

	if (cg_cameraOrbit.integer) {
		if (cg.time > cg.nextOrbitTime) {
			cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
			cg_thirdPersonAngle.value += cg_cameraOrbit.value;
		}
	}

	// add error decay
	if ( cg_errorDecay.value > 0 ) {
		int		t;
		float	f;

		t = cg.time - cg.predictedErrorTime;
		f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
		if ( f > 0 && f < 1 ) {
			VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
		} else {
			cg.predictedErrorTime = 0;
		}
	}

	if ( cg.renderingFlyBy ) {
		CG_Q3F_OffsetFlyBy();
	} else if ( cg.renderingThirdPerson ) {
		// back away from character

		CG_OffsetThirdPersonView();
	} else {
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}

	// Calculate concussion effects
//	CG_Q3F_ConcussionEffect();

	// Calculate hallucinogenic effects
	CG_Q3F_GasEffect();

	// Calculate vibration effects
	CG_Q3F_VibrationEffect();

	// Calculate Minigun Shudder
	CG_Q3F_ShudderEffect();

	// position eye reletive to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
	
	if ( cg.hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// Golliwog: Check for sniper rifle autozoom.
	if( cg_sniperAutoZoom.integer && cg.snap->ps.weapon == WP_SNIPER_RIFLE && (cg.snap->ps.eFlags & EF_Q3F_AIMING) )
	{
		if( !cg.autoZoomed )
		{
			cg.autoZoomed = qtrue;
			cg.autoZoomTime = cg.time;
		}
	}
	else if( cg.autoZoomed )
	{
		cg.autoZoomed = qfalse;
		cg.zoomTime = cg.time;	// Not a typo, we use the 'normal' zoom-out
	}
	// Golliwog.
	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds( void ) {
	int		i;
	int		t;

	// powerup timers going away
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		t = cg.snap->ps.powerups[i];
		if ( t <= cg.time ) {
			continue;
		}
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			continue;
		}
		if ( ( t - cg.time ) / POWERUP_BLINK_TIME != ( t - cg.oldTime ) / POWERUP_BLINK_TIME ) {
			if( i != PW_Q3F_CONCUSS && i != PW_Q3F_FLASH && i != PW_Q3F_GAS )
				trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound );
		}
	}
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
	if ( !sfx )
		return;
	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if (cg.soundBufferIn == cg.soundBufferOut) {
		cg.soundBufferOut++;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
	if ( cg.soundTime < cg.time ) {
		if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
			trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		}
	}
}

/*
=================
CG_FirstFrame

Called once on first rendered frame
=================
*/
static void CG_FirstFrame( void )
{
	CG_SetConfigValues();

	cgs.voteTime = atoi( CG_ConfigString( CS_VOTE_TIME ) );
	cgs.voteYes = atoi( CG_ConfigString( CS_VOTE_YES ) );
	cgs.voteNo = atoi( CG_ConfigString( CS_VOTE_NO ) );
	Q_strncpyz( cgs.voteString, CG_ConfigString( CS_VOTE_STRING ), sizeof( cgs.voteString ) );

	if ( cgs.voteTime )
		cgs.voteModified = qtrue;
	else
		cgs.voteModified = qfalse;
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
extern displayContextDef_t cgDC; // RR2DO2

//#define DEBUGTIME_ENABLED
#ifdef DEBUGTIME_ENABLED
//#define DEBUGTIME Com_Printf("t%i:%i ", dbgCnt++, elapsed = (trap_Milliseconds()-dbgTime) ); dbgTime+=elapsed;
#define DEBUGTIME if(cg_debugTime.integer) { Com_Printf("%i, ", elapsed = (trap_Milliseconds()-dbgTime) ); dbgCnt++; dbgTime+=elapsed; }
#else
#define DEBUGTIME
#endif

extern int remapSkyPortalCount;

#define	CG_FPS_FRAMES	20
void CG_Q3F_RenderCustomShaders();

#ifdef Q3F_WATER
void CG_Q3F_RenderWater();
#endif

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;

	static int index;
	static int	previousTimes[CG_FPS_FRAMES];

#ifdef DEBUGTIME_ENABLED
	char buffer[256];
	int dbgTime=trap_Milliseconds(),elapsed;
	int dbgCnt=0;
#endif

	cgDC.frameTime = serverTime - cgDC.realTime;
	cgDC.realTime = serverTime;

	previousTimes[index % CG_FPS_FRAMES] = cgDC.frameTime;
	index++;
	if ( index > CG_FPS_FRAMES ) {
		int i, total;
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < CG_FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		cgDC.FPS = 1000 * CG_FPS_FRAMES / total;
	}

	cg.time = serverTime;
	// adjust the clock to reflect latent snaps
	cg.time -= cg_latentSnaps.integer * (1000 / cgs.sv_fps);

	cg.demoPlayback = demoPlayback;

	// RR2DO2
	cgDC.frameTime = cg.time - cgDC.realTime;
	cgDC.realTime = cg.time;
	// RR2DO2

	// Allow for drawing unless stated otherwise
	cg.drawFilter = qfalse;

	// RR2DO2: set flare rendering to main refdef
	CG_SetFlareRenderer( &cg.refdef, &cg.lensflare_blinded );

	// update cvars
	CG_UpdateCvars();

   //Keeger polybus hook
   CG_PB_ClearPolyBuffers();

	//Prepare the amount of particles to spawn for all systems.
	Spirit_PrepareFrame();

#ifdef DEBUGTIME_ENABLED
	Com_Printf("\n");
#endif
	DEBUGTIME

	// RR2DO2: check for latched cvars
/*	if ( !cg.r_gammaCanChange ) {
		float testvar;

		testvar = r_gamma.value - cg.r_gammaOldValue;

		if ( testvar < 0 )
			testvar = -testvar;

		if ( testvar > 0.05 ) {	
			cg.r_gammaLatched = r_gamma.value;
			if ( cg.r_gammaLatched < 0.5 )
				cg.r_gammaLatched = 0.5;
			else if ( cg.r_gammaLatched > 3 )
				cg.r_gammaLatched = 3;
			CG_Printf(BOX_PRINT_MODE_CHAT, "Can't change r_gamma, latched to %f\n", cg.r_gammaLatched);			
			//CG_Printf("Can't change r_gamma, latched to %f %f %f %f %f %f %f %f %f %f %f\n", .0f, .1f, .2f, .3f, .4f, .5f, .6f, .7f, .8f, .9f, 1.f);
			trap_Cvar_Set ( "r_gamma", va("%f",cg.r_gammaOldValue) );
		}

		cg.r_gammaOldValue = cg.r_gammaOldValue + ( (cg.clientFrame & 1) ? 0.01 : -0.01 );
		trap_Cvar_Set( "r_gamma", va("%f",cg.r_gammaOldValue ) );
	}*/
	// RR2DO2

	DEBUGTIME

	// RR2DO2: reset lensflare blinding
	VectorSet( cg.lensflare_blinded, 0.f, 0.f, 0.f );
	cg.lensflare_blinded[3] = 0.f;

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if( cgs.initPhase || cg.infoScreenText[0] != 0 ) {
		if( !cgs.initDemoFrameRender )
			CG_Q3F_InitUpdate();
		return;
	}

	DEBUGTIME

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	DEBUGTIME

	// clear all the render lists
	trap_R_ClearScene();

	DEBUGTIME

	cg.sniperWashColour[3]	= 0;		// Clear 'sniper wash' for this frame.
	cg.sniperDotEnt			= NULL;		// And our dot.

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	DEBUGTIME

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		CG_Q3F_RenderLoadingScreen();
		return;
	}

	DEBUGTIME

	// let the client system know what our weapon and zoom settings are
	trap_SetUserCmdValue( cg.weaponSelect, cg.ucmd_flags, cg.zoomSensitivity, 0 );

	if ( cg.clientFrame == 0 )
		CG_FirstFrame();

	DEBUGTIME

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
//#ifdef PERFLOG
//	BG_Q3F_PerformanceMonitor_LogFunction("CG_PredictPlayerState");
//#endif
	CG_PredictPlayerState();
//#ifdef PERFLOG
//	BG_Q3F_PerformanceMonitor_LogFunctionStop();
//#endif

	DEBUGTIME

	// decide on third person view
	cg.renderingThirdPerson = (cg_thirdPerson.integer /*&& cg.concussTime < cg.time*/) /*|| cg.snap->ps.stats[STAT_HEALTH] <= 0*/ || (cg.snap->ps.pm_flags & PMF_CHASE);// || (cg.snap->ps.eFlags & EF_Q3F_INVISIBLE);

	// build cg.refdef
	inwater = CG_CalcViewValues();

	DEBUGTIME

	/* RR2: disabled as we do a red screen wipe now
	// first person blend blobs, done after AnglesToAxis
	if ( !cg.renderingThirdPerson && !cg.renderingFlyBy && !cg.rendering2ndRefDef ) {
		CG_DamageBlendBlob();
	}*/

	// RR2DO2: draw sky portal
	if( cg_drawSkyPortal.integer && !r_fastSky.integer && cgs.skyportal.hasportal) {
		CG_ETF_DrawSkyPortal( &cg.refdef, &cg.lensflare_blinded, stereoView, cgs.skyportal.origin );
	}

	DEBUGTIME

	// Clear the HUD slots before processing entities
	memset( cg.hudslots, 0, sizeof( cg.hudslots ) );

	// build the render lists
	if ( !cg.hyperspace ) {
		DEBUGTIME
		CG_AddPacketEntities();						// alter calcViewValues, so predicted player state is correct
		DEBUGTIME
		CG_AddMarks();
		DEBUGTIME
		CG_AddLocalEntities();
		DEBUGTIME
		CG_AddAtmosphericEffects();                  // Keeger:  ET atmospherics
		DEBUGTIME
		CG_Q3F_RenderCustomShaders();
		DEBUGTIME
#ifdef Q3F_WATER
		CG_Q3F_RenderWater();
		DEBUGTIME
#endif // Q3F_WATER
#ifdef BUILD_BOTS
		if ( cgs.localServer ) { 
			OmnibotRenderDebugLines();
		}
#endif
	}

	DEBUGTIME

	CG_AddViewWeapon( &cg.predictedPlayerState );

	DEBUGTIME

	if ( !cg.hyperspace ) {
		CG_AddFlameChunks ();  //keeg flamethrower, this comes first i believe  FT_NEW
		DEBUGTIME
		CG_AddTrails ();		// this must come last, so the trails dropped this frame get drawn
		DEBUGTIME
		Spirit_AddParticles();
		DEBUGTIME
		CG_AddSmokeSprites();
		DEBUGTIME

	}

	DEBUGTIME

	// add buffered sounds
	CG_PlayBufferedSounds();

	DEBUGTIME

	// play delayed server sounds
	CG_PlayDelayedSounds();

	DEBUGTIME

	// finish up the rest of the refdef
	if ( cg.testModelEntity.hModel ) {
		CG_AddTestModel();
	}
	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	DEBUGTIME

	// warning sounds when powerup is wearing off
	CG_PowerupTimerSounds();

	DEBUGTIME

	// update audio positions
	trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

	DEBUGTIME

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	}

	DEBUGTIME

	if (cg_timescale.value != cg_timescaleFadeEnd.value) {
		if (cg_timescale.value < cg_timescaleFadeEnd.value) {
			cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value > cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		else {
			cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value < cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		if (cg_timescaleFadeSpeed.value) {
			trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
		}
	}

	DEBUGTIME

	// Golliwog: Hold a menu up if we're not in the game
	CG_Q3F_AutoTeamMenu();
	// Golliwog

	DEBUGTIME

	// actually issue the rendering calls
	CG_DrawActive( stereoView );

	DEBUGTIME

	// Golliwog: Draw the map selection menu if required
//	if( cg.mapSelectState != Q3F_MAPSELECT_NONE )
//		CG_Q3F_MapSelectVote();
	// Golliwog:

	// Golliwog: Expire any old or reached waypoints.
	CG_Q3F_WaypointMaintain();
	// Golliwog.

	DEBUGTIME

	if ( cg_stats.integer ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "cg.clientFrame:%i\n", cg.clientFrame );
	}

	// Golliwog: Throw the grenade if we're dead, held it too long,
	// or failed to get an ack back from the server.
	if(	cg.grenadeprimeTime &&
		((cg.grenadeprimeTime + Q3F_GRENADE_PRIME_TIME - 60) <= cg.time ||
		cg.snap->ps.stats[STAT_HEALTH] <= 0 ||
		(cg.grenadethrowTime && cg.grenadethrowTime <= cg.time)) )
		CG_Q3F_GrenThrow();
	// Golliwog.

	DEBUGTIME

	// Golliwog: Check for changed weapons
	if( cg.weaponSelect != cg.currWeapon )
	{
		cg.lastWeapon = cg.currWeapon;
		cg.currWeapon = cg.weaponSelect;
	}

	DEBUGTIME

	CG_Q3F_PlaySoundDict();		// Golliwog: Ensure sounddict sounds keep playing

	DEBUGTIME

//	CG_Q3F_DrawCamPaths();

}

