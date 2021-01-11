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

// g_misc.c

#include "g_local.h"
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}

/*QUAKED path_spline (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the chilli, for the bspline paths.
*/
void SP_path_spline( gentity_t *self ) {
	G_FreeEntity( self );
}

/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	qboolean lightSet, colorSet, needlink;
	vec3_t color;
	float light;
	int r, g, b, i;

	needlink = qfalse;
	G_SetOrigin( self, self->s.origin );

		// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnColor( "1 1 1", color );
	if ( lightSet || colorSet ) {
		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		self->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
		needlink = qtrue;
	}
	//if( !self->wait )
	//	self->wait = 30;
	if( self->wait )
		self->wait *= 1000;		// Convert into milliseconds

	if( needlink )		// Only link if necessary (e.g. lighting)
		trap_LinkEntity( self );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) nonlinear angle - - q3map_non-dynamic
Non-displayed light.
"light" overrides the default 300 intensity.
Nonlinear checkbox gives inverse square falloff instead of linear
Angle adds light:surface angle calculations (only valid for "Linear" lights) (wolf)
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
"fade" falloff/radius adjustment value. multiply the run of the slope by "fade" (1.0f default) (only valid for "Linear" lights) (wolf)
"q3map_non-dynamic" specifies that this light should not contribute to the world's 'light grid' and therefore will not light dynamic models in the game.(wolf)
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}

/*QUAKED lightJunior (0 0.7 0.3) (-8 -8 -8) (8 8 8) nonlinear angle negative_spot negative_point
Non-displayed light that only affects dynamic game models, but does not contribute to lightmaps
"light" overrides the default 300 intensity.
Nonlinear checkbox gives inverse square falloff instead of linear
Angle adds light:surface angle calculations (only valid for "Linear" lights) (wolf)
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
"fade" falloff/radius adjustment value. multiply the run of the slope by "fade" (1.0f default) (only valid for "Linear" lights) (wolf)
*/
void SP_lightJunior( gentity_t *self ) {
	G_FreeEntity( self );
}


/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	//if ( player->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
	if ( !Q3F_IsSpectator(player->client) ) {	// RR2DO2
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
	player->client->ps.pm_time = 160;		// hold time
	player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// we don't want players being backward-reconciled back through teleporters
	G_ResetHistory( player );

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	//if ( player->client->sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
	if ( !Q3F_IsSpectator(player->client) ) {	// RR2DO2
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( !Q3F_IsSpectator(player->client) ) {	// RR2DO2
		trap_LinkEntity (player);
	}
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	// Golliwog: Don't kill it if it's switchable
// XreaL BEGIN
#ifdef _ETXREAL
	if( (!ent->targetname || ent->targetnameAutogenerated) && !(ent->mapdata && ent->mapdata->groupname) )
#else
	if( !ent->targetname && !(ent->mapdata && ent->mapdata->groupname) )
#endif
// XreaL END
		G_FreeEntity( ent );
	// Golliwog.
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) {
	// Golliwog: RunPortal now does all the work

	G_RunPortal( ent );

/*	vec3_t		dir;
	gentity_t	*target;
	gentity_t	*owner;

	owner = G_PickTarget( ent->target );
	if ( !owner ) {
		G_Printf( "Couldn't find target for misc_partal_surface\n" );
		G_FreeEntity( ent );
		return;
	}
	ent->r.ownerNum = owner->s.number;

	// frame holds the rotate speed
	if ( owner->spawnflags & 1 ) {
		ent->s.frame = 25;
	} else if ( owner->spawnflags & 2 ) {
		ent->s.frame = 75;
	}

	// swing camera ?
	if ( owner->spawnflags & 4 ) {
		// set to 0 for no rotation at all
		ent->s.powerups = 0;
	}
	else {
		ent->s.powerups = 1;
	}

	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy( owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	target = G_PickTarget( owner->target );
	if ( target ) {
		VectorSubtract( target->s.origin, owner->s.origin, dir );
		VectorNormalize( dir );
	} else {
		G_SetMovedir( owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );*/
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void locatePath(gentity_t *ent) {
	if ( ent->s.legsAnim == -1 )
		ent->s.legsAnim = BG_Q3F_GetPathIndex( ent->message, (void *)&level.campaths );

	if ( ent->s.legsAnim < 0 || ent->s.legsAnim > Q3F_MAX_PATHS ) 
		return;

	if (level.campaths[ent->s.legsAnim].camtraj.trTime + level.campaths[ent->s.legsAnim].camtraj.trDuration < level.time) {
		level.campaths[ent->s.legsAnim].currtrajindex++;
		if ( level.campaths[ent->s.legsAnim].currtrajindex >= level.campaths[ent->s.legsAnim].numsplines )
			level.campaths[ent->s.legsAnim].currtrajindex = 0;
		VectorCopy( level.campaths[ent->s.legsAnim].splines[level.campaths[ent->s.legsAnim].currtrajindex].SegmentVtx[0], level.campaths[ent->s.legsAnim].camtraj.trBase );
		level.campaths[ent->s.legsAnim].camtraj.trDuration = BG_Q3F_CubicSpline_Length(&level.campaths[ent->s.legsAnim].splines[level.campaths[ent->s.legsAnim].currtrajindex])/level.campaths[ent->s.legsAnim].camsplines[level.campaths[ent->s.legsAnim].currtrajindex].speed * 1000;
		level.campaths[ent->s.legsAnim].camtraj.trTime = level.time;
		level.campaths[ent->s.legsAnim].camtraj.trType = TR_CUBIC_SPLINE_PATH;
		VectorCopy( level.campaths[ent->s.legsAnim].camtraj.trBase, ent->s.origin2 );
	} else {
		BG_Q3F_EvaluateSplineTrajectory( &level.campaths[ent->s.legsAnim].camtraj, NULL, &level.campaths[ent->s.legsAnim].splines[level.campaths[ent->s.legsAnim].currtrajindex], level.time, ent->s.origin2 );
	}
	ent->nextthink = level.time + FRAMETIME;

	// set to 0 for no rotation at all
	ent->s.powerups = 1;
}

void SP_misc_portal_surface(gentity_t *ent) {
	char *str;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	VectorCopy( ent->s.origin, ent->s.origin2 );
	trap_LinkEntity (ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	// RR2DO2: add support for spline cameras
	G_SpawnString( "camerapath", "", &str );
	if( *str ) {
		ent->message = G_NewString( str );
	}
	
	ent->s.legsAnim = -1;

	if ( ent->message ) {
		ent->think = locatePath;
		ent->nextthink = level.time + 100;
	} else if ( !ent->target ) {  
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} else {
		ent->think = locateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t		dir;
	float		deg;
	vec3_t		up, right;

	// see if we have a target
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
	} else {
		VectorCopy( ent->movedir, dir );
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = Q_flrand(-1.0f, 1.0f) * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = Q_flrand(-1.0f, 1.0f) * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) {
	case WP_GRENADE_LAUNCHER:
		fire_grenade( ent, ent->s.origin, dir );
		break;
	case WP_ROCKET_LAUNCHER:
		fire_rocket( ent, ent->s.origin, dir );
		break;
/* JT
	case WP_PLASMAGUN:
		fire_plasma( ent, ent->s.origin, dir );
		break;
	JT */
	}

	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
	ent->enemy = G_PickTarget( ent->target );
	ent->think = 0;
	ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
	ent->use = Use_Shooter;
	ent->s.weapon = weapon;

	RegisterItem( BG_FindItemForWeapon( weapon ) );

	G_SetMovedir( ent->s.angles, ent->movedir );

	if ( !ent->random ) {
		ent->random = 1.0;
	}
	ent->random = sin( M_PI * ent->random / 180 );
	// target might be a moving object, so we can't set movedir for it
	if ( ent->target ) {
		ent->think = InitShooter_Finish;
		ent->nextthink = level.time + 500;
	}
	trap_LinkEntity( ent );
}

/*QUAKED shooter_rocket (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_rocket( gentity_t *ent ) {
	InitShooter( ent, WP_ROCKET_LAUNCHER );
}

/*QUAKED shooter_plasma (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_plasma( gentity_t *ent ) {
// JT	InitShooter( ent, WP_PLASMAGUN);
}

/*QUAKED shooter_grenade (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_grenade( gentity_t *ent ) {
	InitShooter( ent, WP_GRENADE_LAUNCHER);
}


/*
**	Portal 'run' function (checks for cameras, etc.)
*/

void G_RunPortal( gentity_t *portal )
{
	// Golliwog: If targeted, look for usable cameras (rather than simply existing cameras,
	// as in Q3A).

	gentity_t *camera, *target;
	vec3_t dir;

	if( !portal->target )
		return;
	camera = portal->activator;
	if( camera )
	{
		// Check this camera is still valid
		if( !camera->mapdata )
			return;
		if( camera->mapdata->state == Q3F_STATE_INACTIVE || camera->mapdata->state == Q3F_STATE_ACTIVE )
			return;
	}

	camera = NULL;
	while( 1 )
	{
		camera = G_Find( camera, FOFS(targetname), portal->target );
		if( !camera )
		{
			// No cameras left to try - turn it into a mirror
			if( portal->activator )
			{
				VectorCopy( portal->s.origin, portal->s.origin2 );
				portal->r.ownerNum = 0;
				portal->s.clientNum = 0;
				portal->s.eventParm = 0;
				portal->activator = NULL;
				trap_LinkEntity( portal );
			}
			return;
		}
		if( camera->mapdata && (camera->mapdata->state == Q3F_STATE_DISABLED || camera->mapdata->state == Q3F_STATE_INVISIBLE) ) 
			continue;		// This camera isn't usable, keep looking

		// We've found a usable camera, set it up

		// frame holds the rotate speed
		if ( camera->spawnflags & 1 ) {
			portal->s.frame = 25;
		} else if ( camera->spawnflags & 2 ) {
			portal->s.frame = 75;
		}
		else portal->s.frame = 0;

		// swing camera ?
		if ( camera->spawnflags & 4 ) {
			// set to 0 for no rotation at all
			portal->s.powerups = 0;
		}
		else {
			portal->s.powerups = 1;
		}

		// clientNum holds the rotate offset
		portal->r.ownerNum = camera->s.number;
		portal->s.clientNum = camera->s.clientNum;

		VectorCopy( camera->s.origin, portal->s.origin2 );

		// see if the portal_camera has a target
		target = G_PickTarget( camera->target );
		if ( target ) {
			VectorSubtract( target->s.origin, camera->s.origin, dir );
			VectorNormalize( dir );
		} else {
			G_SetMovedir( camera->s.angles, dir );
		}

		portal->s.eventParm = DirToByte( dir );
		portal->activator = camera;
		trap_LinkEntity( portal );
		return;
	}
}

#if 0

/*QUAKED corona (0 1 0) (-4 -4 -4) (4 4 4) START_OFF
Use color picker to set color or key "color".  values are 0.0-1.0 for each color (rgb).
"scale" will designate a multiplier to the default size.  (so 2.0 is 2xdefault size, 0.5 is half)
*/

/*
==============
use_corona
	so level designers can toggle them on/off
==============
*/
void use_corona( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	} else
	{
		ent->active = 0;
		trap_LinkEntity( ent );
	}
}


/*
==============
SP_corona
==============
*/
void SP_corona( gentity_t *ent ) {
	float scale;
	vec3_t color;

	ent->s.eType        = ET_CORONA;

	G_SpawnColor( "1 1 1", color );

	if ( color[0] <= 0 &&                // if it's black or has no color assigned
		 color[1] <= 0 &&
		 color[2] <= 0 ) {
		color[0] = color[1] = color[2] = 1; // set white

	}
	color[0] = color[0] * 255;
	color[1] = color[1] * 255;
	color[2] = color[2] * 255;

	ent->s.dl_intensity = (int)color[0] | ( (int)color[1] << 8 ) | ( (int)color[2] << 16 );

	G_SpawnFloat( "scale", "1", &scale );
	ent->s.density = (int)( scale * 255 );

	ent->use = use_corona;

	if ( !( ent->spawnflags & 1 ) ) {
		trap_LinkEntity( ent );
	}
}

#endif
