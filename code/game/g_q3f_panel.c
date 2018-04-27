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
**	g_q3f_panel.c
**
**	Server-side code for panels.
*/

#include "g_local.h"
#include "bg_public.h"
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"

/******************************************************************************
***** Shared panel code
****/

// origin			- Center point.
// origin2			- Width and height (origin2[2] unused).
// origin2[2]		- Distance panel is 'active' from (Default 0, in which case it matches the (x*x+y*y)/2).
// angles			- Angle of normal, angles[2] for rotation around normal.
// groundEntityNum	- Type of panel.
// legsAnim			- Shader drawn behind dynamic content.
// torsoAnim		- Shader drawn on top of dynamic content.
// weapon			- Power-up/power-down effect.

/*static float AngleDiff( float angle1, float angle2 )
{
	float diff;

	diff = angle1 - angle2;
	if( angle1 > angle2 )
	{
		if( diff > 180.0 )
			diff -= 360.0;
	}
	else {
		if( diff < -180.0 )
			diff += 360.0;
	}
	return( diff );
}*/

#define RADIATEPOINTS 50

qboolean G_Q3F_RadiateToSurface( vec3_t focus, float maxDist, vec3_t traceEnd, vec3_t angles, gentity_t **traceEnt, int contentmask, int ignoreEnt )
{
	int k;
	//float h;
	vec3_t dest, closest, closenormal, vec, dir;
	float closelen, len, theta, phi/*, phi1*/;
	trace_t trace;

	closelen = maxDist + 1;

	// RR2DO2: bleh - be lazy and do it the dumb way. But at least this works :)
	for( k = 0 ; k < NUMVERTEXNORMALS; k++ ) {
		VectorMA( focus, closelen, bytedirs[k], dest );
		trap_Trace( &trace, focus, NULL, NULL, dest, ignoreEnt, contentmask );
		if( trace.fraction < 1 && !trace.startsolid )
		{
			len = Distance( focus, trace.endpos );
			if( len < closelen )
			{
				closelen = len;
				VectorCopy( trace.plane.normal, closenormal );
				VectorCopy( trace.endpos, closest );
				*traceEnt = &g_entities[trace.entityNum];
				if( len < 1 )
					break;
			}
		}
	}

	if( closelen > maxDist )
		return( qfalse );

	// Get the normal from the surface, and calculate the perpendicular intersection
	// from the focus to the traced plane.
	// angle = acos( v1.v2 / |v1||v2| )
	// adjacent = cos( angle ) * hypotenuse
	// adjacent = (v1.v2 / (|v1||v2|)) * hypotenuse

	VectorScale( closenormal, -1, dir );
	VectorSubtract( closest, focus, vec );
	phi = closelen;	// * normal length, which should always be 1.
	theta = DotProduct( dir, vec );
	phi = theta / phi;
//	phi = acos( phi );
	VectorMA( focus, phi * closelen, dir, traceEnd );
	vectoangles( closenormal, angles );
	angles[ROLL] = 0;

	return( qtrue );
}

void G_Q3F_PanelAffix( gentity_t *panel )
{
	// Seat the panel on a surface after everything has been spawned.
	vec3_t newOrigin, angles, up,  right, corner;
	gentity_t *surfaceEnt;
	int index;

		// Check we can locate a surface.
	if( !G_Q3F_RadiateToSurface( panel->s.origin, 64, newOrigin, angles, &surfaceEnt, MASK_SHOT, ENTITYNUM_NONE ) )
	{
		G_Printf( "^3Warning: Unable to locate surface for panel '%d'.^7\n", panel->s.number );
		G_FreeEntity( panel );
		return;
	}

		// Set the surface and normal angles.
	VectorCopy( newOrigin, panel->s.origin );
	panel->s.angles[PITCH]	= angles[PITCH];
	panel->s.angles[YAW]	= angles[YAW];

		// Work out a bounding box (for visibility).
	AngleVectors( panel->s.angles, NULL, up, right );
	VectorSet( panel->r.mins, -1, -1, -1 );
	VectorSet( panel->r.maxs, 1, 1, 1 );
	VectorSet( newOrigin, 0, 0, 0 );
	for( index = 0; index < 4; index++ )
	{
		// Add each corner to the bounds.
		VectorMA( newOrigin, ((index & 1) ? 0.5 : -0.5) * panel->s.origin2[0], right, corner );
		VectorMA( corner, ((index & 2) ? 0.5 : -0.5) * panel->s.origin2[1], up, corner );
		AddPointToBounds( corner, panel->r.mins, panel->r.maxs );
	}

	panel->r.contents = CONTENTS_DETAIL;
	panel->r.svFlags	= SVF_USE_CURRENT_ORIGIN;
	VectorCopy( panel->s.origin, panel->r.currentOrigin );
	trap_LinkEntity( panel );						// Only link if it's visible
}

typedef struct {
	int id;
	char *name;
} panelTransitionMap_t;
panelTransitionMap_t panelTransitionNames[] = {
	// List of transition names.

	{ Q3F_PANELTRANS_NONE,	"none" },
	{ Q3F_PANELTRANS_FADE,	"fade" },
};

qboolean G_Q3F_PanelBaseSpawn( gentity_t *panel )
{
	// Generic init of panels, data shared between panels.

	char *str;
	int index;

		// Find the transition to use.
	G_SpawnString( "transition", "none", &str );
	panel->s.weapon = -1;
	for( index = 0; index < (sizeof(panelTransitionNames) / sizeof(panelTransitionMap_t)); index++ )
	{
		if( !Q_stricmp( str, panelTransitionNames[index].name ) )
		{
			panel->s.weapon = panelTransitionNames[index].id;
			break;
		}
	}
	if( panel->s.weapon == -1 )
	{
		G_Printf( "Unknown transition type '%s' on entity %d.", str, panel->s.number );
		panel->s.weapon = Q3F_PANELTRANS_NONE;
	}

		// The 'base' shader drawn before anything else.
	G_SpawnString( "backshader", "", &str );
	if( *str )
		panel->s.legsAnim = G_ShaderIndex( str );
		// The 'base' shader drawn after anything else.
	G_SpawnString( "frontshader", "", &str );
	if( *str )
		panel->s.torsoAnim = G_ShaderIndex( str );

	G_SpawnFloat( "width",		"100",	&panel->s.origin2[0] );
	G_SpawnFloat( "height",		"75",	&panel->s.origin2[1] );
	G_SpawnFloat( "distance",	"0",	&panel->s.origin2[2] );
	G_SpawnFloat( "angle", "0", &panel->s.angles[ROLL] );

	if( panel->s.origin2[2] <= 0 )
		panel->s.origin2[2] = (panel->s.origin2[0] * panel->s.origin2[0] + panel->s.origin2[1] * panel->s.origin2[1]) * 0.5;

	panel->s.eType	= ET_Q3F_PANEL;
	panel->s.groundEntityNum = Q3F_PANELTYPE_NAME;

	panel->think = G_Q3F_PanelAffix;
	panel->nextthink = level.time + FRAMETIME;

	return( qtrue );
}

int G_Q3F_ConvertSpawnColour( char *key, char *def )
{
	// Get the specified key and convert into an RGB 24-bit integer.

	vec3_t colour;
	if(	Q_stricmp( "color", key ) && Q_stricmp( "_color", key ) &&
		Q_stricmp( "colour", key ) && Q_stricmp( "_colour", key ) )
		G_SpawnVector( key, def, colour );
	else G_SpawnColor( def, colour );
	return(	(((unsigned char) (colour[0] * 255)) << 16) +
			(((unsigned char) (colour[1] * 255)) << 8) +
			((unsigned char) (colour[2] * 255)) );
}



/******************************************************************************
***** Panel think functions
****/

static qboolean G_Q3F_StoreRadarBlip( int blipIndex, gentity_t *ent, float angle, float range, float intensity, int team, int height )
{
	// Store a 'radar blip' in the entity structure.
	// each blip contains: angle (8 bits) range (8 bits), intensity (3 bits), colour (3 bits), height (2 bits)
	// The blips _have_ to match on the cgame side, for obvious reasons.
	// constantLight/32, generic1/32, powerups/32, modelindex/8, modelindex2/8, frame/8

	unsigned char encoded[3];
	entityState_t *es;

	encoded[0]	= angle * 255.0;
	encoded[1]	= range * 255.0;
	encoded[2]	= (((int) (intensity * 7)) & 7);	// Intensity.
	encoded[2]	|= (team & 7) << 3;
	encoded[2]	|= (height & 3) << 6;

	es = &ent->s;
	switch( blipIndex )
	{
		case 0:		((unsigned char *) &es->constantLight)[0]	= encoded[0];
					((unsigned char *) &es->constantLight)[1]	= encoded[1];
					((unsigned char *) &es->constantLight)[2]	= encoded[2];
					break;
		case 1:		((unsigned char *) &es->constantLight)[3]	= encoded[0];
					((unsigned char *) &es->generic1)[0]		= encoded[1];
					((unsigned char *) &es->generic1)[1]		= encoded[2];
					break;
		case 2:		((unsigned char *) &es->generic1)[2]		= encoded[0];
					((unsigned char *) &es->generic1)[3]		= encoded[1];
					((unsigned char *) &es->powerups)[0]		= encoded[2];
					break;
		case 3:		((unsigned char *) &es->powerups)[1]		= encoded[0];
					((unsigned char *) &es->powerups)[2]		= encoded[1];
					((unsigned char *) &es->powerups)[3]		= encoded[2];
					break;
		case 4:		((unsigned char *) &es->modelindex)[0]		= encoded[0];
					((unsigned char *) &es->modelindex2)[0]		= encoded[1];
					((unsigned char *) &es->frame)[0]			= encoded[2];
					break;
		default:	return( qfalse );
	}
	return( qtrue );
}

void G_Q3F_PanelRadarThink( gentity_t *ent )
{
	// Every frame, work out the ents visible to the current 'sweep' position.

	int index, blipIndex;
	gentity_t *scan;
	float sweepFront, angle, range, attenuation, speed, attnRange, walls;
	vec3_t offset, direction, tracePos;
	trace_t tr;

	if( ent->takedamage )
	{
		G_Q3F_PanelAffix( ent );
		if( !ent->inuse )
			return;
		ent->takedamage = qfalse;
	}

	sweepFront = ((float) level.time) / ent->speed;
	sweepFront = (sweepFront - floor(sweepFront)) * 2*M_PI;

//	G_Printf( "Sweep %i-%i.\n", (int)(sweepFront - ent->angle), (int)sweepFront );
	
	for( scan = g_entities, index = blipIndex = 0; index < level.maxclients; index++, scan++ )
	{
		if( !scan->inuse || scan->health <= 0 || Q3F_IsSpectator( scan->client ) )
			continue;
		VectorSubtract( scan->client->ps.origin, ent->s.origin, offset );
		angle = (offset[0] ? atan2( offset[1], offset[0] ) : (offset[1] > 0 ? 0.5*M_PI : -0.5*M_PI));
		if( angle < 0 )
			angle += 2*M_PI;
		angle = sweepFront - angle;
		if( angle < 0 )
			angle += 2*M_PI;
		if( angle > ent->angle )	// It's out of range of the sweep.
			continue;

		if( !G_Q3F_CheckCriteria( scan, ent ) )
			continue;

			// Calculate attenuation. It is capped at 1 (for full visibility).
			// Attenuation scales down the visibility.
		range = sqrt( offset[0] * offset[0] + offset[1] * offset[1] );
		if( range > ent->s.angles2[0] )
			continue;
		attenuation = ent->s.angles2[0] / range;
		if( ent->pos2[1] )
		{
			// We want speed 500 to work out at 1 at max range, which means it should be 0.2 of original attn

			speed = 0.0004 * sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0] + ent->client->ps.velocity[1] * ent->client->ps.velocity[1] );
			attenuation *= 1 - (ent->pos2[1] < 0 ? -ent->pos2[1] : ent->pos2[1]) * (1 - speed);
			walls = 0;
			if( ent->pos2[1] < 0 )
			{
				// We also attenuate through walls.

				VectorSubtract( ent->s.origin, scan->client->ps.origin, direction );
				attnRange = VectorNormalize( direction ) - 16;
				memset( &tr, 0, sizeof(tr) );
				VectorCopy( scan->client->ps.origin, tr.endpos );
				while( attnRange > 0 )
				{
					if( tr.contents & CONTENTS_SOLID )
					{
						// Just hop forward a little at a time until we find an open area again.

						walls += 16;
						attnRange -= 16;
						VectorMA( tr.endpos, 16, direction, tracePos );
						tr.contents = trap_PointContents( tracePos, scan->s.number );
					}
					else {
						// Trace forward to the next wall.

						VectorCopy( tr.endpos, tracePos );
						trap_Trace( &tr, tracePos, NULL, NULL, ent->s.origin, scan->s.number, CONTENTS_SOLID );
						if( tr.fraction == 1 )
							attnRange = 0;		// We reached the end point.
						else {
							 attnRange -= Distance( tr.endpos, tracePos );
						}
					}
				}
			}

				// Hardcoded 'opacity level' of walls is 100 units.
			if( walls >= 100 )
				attenuation = 0;
			else attenuation *= 1 - walls * 0.01;
		}

		if( attenuation < 0 )
			continue;
		if( attenuation > 1 )
			attenuation = 1;

		if( !G_Q3F_StoreRadarBlip(	blipIndex++, ent, angle / ent->angle, range / ent->s.angles2[0], attenuation,
									ent->s.time2 ? 0 : scan->client->sess.sessionTeam, 0 ) )
			break;	// We've run out of blip space.
	}

	G_Q3F_StoreRadarBlip(	blipIndex++, ent, 0, 1, 1, 0, 0 );

	ent->s.time = blipIndex ? level.time : 0;		// A zero time means no blips.

	while( G_Q3F_StoreRadarBlip( blipIndex++, ent, 0, 0, 0, 0, 0 ) )
		{}

	ent->nextthink = level.time + FRAMETIME;
}

void G_Q3F_PanelMessageThink( gentity_t *ent )
{
	// Every frame, work out the the current state.

	if( ent->takedamage )
	{
		G_Q3F_PanelAffix( ent );
		if( !ent->inuse )
			return;
		ent->takedamage = qfalse;
	}

	if( !ent->mapdata )
		return;
	ent->s.otherEntityNum	= ent->mapdata->lastTriggerer ? ent->mapdata->lastTriggerer->s.number : ENTITYNUM_NONE;
	ent->s.frame			= ent->mapdata->state;
	ent->nextthink = level.time + FRAMETIME;
}




/******************************************************************************
***** Panel control functions (for interactive panels)
****/

/******************************************************************************
***** Panel spawn functions
****/

void SP_Q3F_Panel_Name( gentity_t *self )
{
	// Display the player's name.

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

	self->s.groundEntityNum	= Q3F_PANELTYPE_NAME;
}

void SP_Q3F_Panel_ScoreSummary( gentity_t *self )
{
	// Display the team scores.

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

	self->s.groundEntityNum	= Q3F_PANELTYPE_SCORESUMMARY;
}


void SP_Q3F_Panel_Location( gentity_t *self )
{
	// Display the closest location ent.

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

	self->s.groundEntityNum	= Q3F_PANELTYPE_LOCATION;
	G_SpawnFloat( "xborder", "32", &self->s.angles2[0] );
	G_SpawnFloat( "yborder", "32", &self->s.angles2[1] );
	self->s.time2 = G_Q3F_ConvertSpawnColour( "color", "1.0 1.0 1.0" );
}

void SP_Q3F_Panel_Timer( gentity_t *self )
{
	// Display a digital clock/timer.

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

	self->s.time2 = G_Q3F_ConvertSpawnColour( "color", "1.0 0.0 0.0" );
	G_SpawnInt( "type", "1", &self->s.otherEntityNum );

	self->s.groundEntityNum	= Q3F_PANELTYPE_TIMER;
}

void SP_Q3F_Panel_Radar( gentity_t *self )
{
	// Display a spinning radar display.

	// RR2DO2: disabled for now as it looks crap
	if( 1 ) {
		G_FreeEntity( self );
		return;
	}

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

//	self->s.angles2[0] = self->speed;
	self->s.time2 = G_Q3F_ConvertSpawnColour( "color", "0 0 0" );
	G_SpawnFloat( "range",			"100",	&self->s.angles2[0] ); 
	G_SpawnFloat( "attenuation",	"0",	&self->pos2[1] ); 
	self->speed = self->speed ? 1000 * self->speed : 2000;
	self->s.angles2[1] = self->speed;

	self->angle = ((float) FRAMETIME) / self->speed;
	self->angle = (self->angle - floor(self->angle)) * 2*M_PI;

	self->nextthink = level.time + FRAMETIME;
	self->think = G_Q3F_PanelRadarThink;
	self->takedamage = qtrue;

	self->s.groundEntityNum	= Q3F_PANELTYPE_RADAR;
}

void SP_Q3F_Panel_Message( gentity_t *self )
{
	// Display an arbitrary message based on the board state.

	if( !G_Q3F_PanelBaseSpawn( self ) )
		return;

	self->s.powerups = level.spawnIndex;

	self->nextthink = level.time + FRAMETIME;
	self->think = G_Q3F_PanelMessageThink;
	self->takedamage = qtrue;

	self->s.groundEntityNum	= Q3F_PANELTYPE_MESSAGE;
}
