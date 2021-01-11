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
**	cg_q3f_waypoint.c
**
**	Client-side functions for handling waypoints
*/

#include "cg_local.h"

extern displayContextDef_t cgDC;

typedef struct cg_q3f_waypoint_s {
	int time, highlightStart, highlightEnd;
	vec3_t pos;
	float distance;		// Updated every call to CG_Q3F_Waypoint().
	char *loc;
	char message[256];
	float locwidth, messagewidth;
} cg_q3f_waypoint_t;

#define	MAXWAYPOINTAGE			300000		// 5 minutes.
#define	MAXWAYPOINTS			10			// No more than 10 waypoints at any time.
#define	WAYPOINTMAINTAINTIME	500			// Twice a second.
#define	WAYPOINTREACHDISTANCE	200			// Within 200 units to reach waypoint.
#define	WAYPOINTREACHTRACE		1			// Must manage a trace to be reached.
#define	WAYPOINTHIGHLIGHTANGLE	3			// Must be within 3 degrees either side to be highlighted.
#define	WAYPOINTHIGHLIGHTTIME	500			// Hightlight/unhighlight takes this long to complete.
#define	WAYPOINTGUIDESCALE		0.1			// Pixels per unit.
#define	WAYPOINTGUIDEXMAX		2000		// Largest X guide size (in units).
#define	WAYPOINTGUIDEYMAX		2000		// Largest Y guide size (in units).


static cg_q3f_waypoint_t waypoints[MAXWAYPOINTS];


/******************************************************************************
***** Waypoint rendering functions
****/

void CG_Q3F_WaypointDrawLine( float x1, float y1, float x2, float y2, float width, qhandle_t shader, vec4_t rgba )
{
	// Draws a 3d line and then renders the scene.

	unsigned char crgba[4];
	polyVert_t verts[4];
	vec2_t dir, perpendicular;
	float length, xoff, yoff;
	refdef_t refdef;
//	qhandle_t shader;

//	CG_AdjustFrom640( &x1, &y1, &x2, &y2 );

	memset( &refdef, 0, sizeof(refdef) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	refdef.width = cgs.glconfig.vidWidth;
	refdef.width &= ~1;
	refdef.height = cgs.glconfig.vidHeight;
	refdef.height &= ~1;
	refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
	refdef.time = cg.time;

	// The field of view should allow 640x480 for graphics projected at 100 units away.
	refdef.fov_x = 2 * atan2( 320, 100 ) * (180 / M_PI);// * 0.5;
	refdef.fov_y = 2 * atan2( 240, 100 ) * (180 / M_PI);// * 0.5;

	// Find direction and perpendicular (90 degrees anticlockwise)
	dir[0] = x2 - x1;
	dir[1] = y2 - y1;
	//length = Q_rsqrt( dir[0] * dir[0] + dir[1] * dir[1] );
	length = SQRTFAST( dir[0] * dir[0] + dir[1] * dir[1] );
	dir[0] *= length;
	dir[1] *= length;
	perpendicular[0] = -dir[1];
	perpendicular[1] = dir[0];

	xoff = width * 0.5 * perpendicular[0];
	yoff = width * 0.5 * perpendicular[1];
	x1 = 320 - x1;
	x2 = 320 - x2;
	y1 = 240 - y1;
	y2 = 240 - y2;

	// Create 2 vertices for each endpoint, offset by the +- perpendicular
	// Forward is the X axis, up is the Z axis, and left-right is the Y axis.
	memset( verts, 0, sizeof(verts) );

	VectorSet( verts[0].xyz, 100, x1 - xoff, y1 - yoff );
	VectorSet( verts[1].xyz, 100, x1 + xoff, y1 + yoff );

	VectorSet( verts[2].xyz, 100, x2 + xoff, y2 + yoff );
	VectorSet( verts[3].xyz, 100, x2 - xoff, y2 - yoff );

	verts[0].st[0] = verts[0].st[1] = verts[1].st[1] = verts[3].st[0] = 0;
	verts[1].st[0] = verts[2].st[0] = verts[2].st[1] = verts[3].st[1] = 1;

	crgba[0] = (unsigned char) 255 * rgba[0];
	crgba[1] = (unsigned char) 255 * rgba[1];
	crgba[2] = (unsigned char) 255 * rgba[2];
	crgba[3] = (unsigned char) 255 * rgba[3];
	*(int *)verts[0].modulate = *(int *)verts[1].modulate = *(int *)verts[2].modulate = *(int *)verts[3].modulate = *(int *)crgba;

//	shader = trap_R_RegisterShader( "nocullwhite" );
	trap_R_ClearScene();
	trap_R_AddPolyToScene( shader, 4, verts );
	trap_R_RenderScene( &refdef );
}

qboolean CG_Q3F_WaypointDetail( float x, float y, cg_q3f_waypoint_t *wp, vec3_t vec )
{
	// Draw detailed information on the specified waypoint.
	// Assumes that the waypoint is near the center of the screen.

	float frac, xdist, ydist, width, height, rightmargin, locy, msgy;
	vec4_t colour;
	int vdist;
	char buff[10];

		// Calculate the rollout fraction completed.
	frac = ((float) (cg.time - (wp->highlightStart ? wp->highlightStart : wp->highlightEnd))) / WAYPOINTHIGHLIGHTTIME; 
	if( frac >= 1 )
	{
		if( wp->highlightEnd )
			return( qfalse );		// This shouldn't happen, but just in case.
		frac = 1;
	}
	if( wp->highlightEnd )
		frac = 1 - frac;

	xdist		= sqrt( vec[0] * vec[0] + vec[1] * vec[1] );
	ydist		= -vec[2];
	if( xdist > WAYPOINTGUIDEXMAX )			xdist = WAYPOINTGUIDEXMAX;
	if( ydist < -WAYPOINTGUIDEYMAX )		ydist = -WAYPOINTGUIDEYMAX;
	else if( ydist > WAYPOINTGUIDEYMAX )	ydist = WAYPOINTGUIDEYMAX;

	width		= xdist * WAYPOINTGUIDESCALE * frac;
	height		= ydist * WAYPOINTGUIDESCALE * frac;

	rightmargin	= x + width - 4;
	if( height < 0 )
	{
		locy = y - 4 - TINYCHAR_HEIGHT;
		msgy = y - 4 - TINYCHAR_HEIGHT * 3;
	}
	else {
		locy = y + 4 + TINYCHAR_HEIGHT;
		msgy = y + 4 + 3 * TINYCHAR_HEIGHT;
	}

	colour[0] = colour[3] = 1.0f;
	colour[1] = colour[2] = 0.0f;
	trap_R_SetColor( colour );
	CG_DrawPic( x, y, width, 1, cgs.media.whiteShader );
	CG_DrawPic( x + width, height < 0 ? (y + height) : y, 1, fabs( height ), cgs.media.whiteShader );
	trap_R_SetColor( NULL );

	colour[0] = colour[1] = colour[2] = 1.0f;
	colour[3] = frac;
//	CG_DrawStringExt( rightmargin - wp->locwidth, locy, wp->loc, colour, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 256 );
//	CG_DrawStringExt( rightmargin - wp->messagewidth, msgy, wp->message, colour, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 256 );
	CG_Text_Paint( rightmargin - wp->locwidth, locy, 0.2f, colour, wp->loc, 0, 0, 0, &cgDC.Assets.font, ITEM_ALIGN_LEFT );
	CG_Text_Paint( rightmargin - wp->messagewidth, msgy, 0.2f, colour, wp->message, 0, 0, 0, &cgDC.Assets.font, ITEM_ALIGN_LEFT );

	vdist = fabs( ydist );
	Com_sprintf(	buff, sizeof(buff), "%s%d.%d",
					(ydist > 0) ? "-" : "+", (int) (vdist * 0.01), ((int) (vdist * 0.1)) % 10 );
//	CG_DrawStringExt( rightmargin + 8, y + height, buff, colour, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 10 );
	CG_Text_Paint( rightmargin + 8, y + height, 0.2f, colour, buff, 0, 0, 0, &cgDC.Assets.font, ITEM_ALIGN_LEFT );

	return( qtrue );
}

float CG_Q3F_AngleToCoord( float angle, int coordRange, float fov, qboolean reverseCoord )
{
	// Take an angle and return the coord it maps to (may be out of visible range)
	// The conversion is: correct fov to degrees: (90.0 / fov), convert to radians: M_PI / 180,
	// obtain tangent (results in -1 - 1, at least in visible coords). The first two steps can
	// be combined, hence the M_PI * 0.5f / fov.

	angle = tan( angle * M_PI * 0.5f / fov );
	angle = coordRange * (reverseCoord ? (1.0f - angle) : (1.0f + angle));
	return( angle );
}


void CG_Q3F_Waypoint()
{
	// Draw each waypoint on screen as a 2d sprite.

	vec3_t angles, vec, normal;
	float x, y, roll, txtx, txty, dist;
	int index, distsize;
	qboolean highlighted;
	cg_q3f_waypoint_t *wp;
	char buff[6];

	for( index = highlighted = 0; index < MAXWAYPOINTS; index++ )
	{
		wp = &waypoints[index];
		if( !wp->time )
			continue;
		VectorSubtract( wp->pos, cg.refdef.vieworg, vec );
		wp->distance = VectorNormalize2( vec, normal );

			// Find the 2d coordinates for this angle.
		vectoangles( vec, angles );
		angles[PITCH]	= AngleNormalize180( angles[PITCH]	- cg.refdefViewAngles[PITCH] );
		angles[YAW]		= AngleNormalize180( angles[YAW]	- cg.refdefViewAngles[YAW] );
		angles[ROLL]	= AngleNormalize180( angles[ROLL]	- cg.refdefViewAngles[ROLL] );
		x = CG_Q3F_AngleToCoord( angles[YAW],	320, cg.refdef.fov_x, qtrue );
		y = CG_Q3F_AngleToCoord( angles[PITCH],	240, cg.refdef.fov_y, qfalse );

			// Ensure that waypoints behind us aren't mistaken for in front (the 3d->2d
			// converstion can't tell). We check each dimension individually to make sure.
		if( /*x >= 0 && x < 640 && */fabs( angles[YAW] ) > cg.refdef.fov_x )
			x = angles[YAW] < 0 ? 1280 : -640;
		if( /*y >= 0 && y < 640 && */fabs( angles[PITCH] ) > cg.refdef.fov_y )
			y = angles[PITCH] > 0 ? 960 : -480;

			// Convert for view roll, if present.
		if( angles[ROLL] )
		{
			roll = angles[ROLL] * M_PI/180;
			x = x * cos( roll ) - y * sin( roll );
			y = y * cos( roll ) + x * sin( roll );
		}

		dist = (int) (0.1 * wp->distance);
		Com_sprintf( buff, sizeof(buff), "%d.%d", (int) (0.1 * dist), ((int) dist) % 10 );
		distsize = strlen( buff );
		txtx = x - 0.5 * distsize * TINYCHAR_WIDTH;
		txty = y + 4;

		if( txtx < 0 )										txtx = 0;
		else if( txtx > 640 - distsize * TINYCHAR_WIDTH )	txtx = 640 - distsize * TINYCHAR_WIDTH;
		if( txty < 4 )										txty = 4;
		else if( txty > 476 - TINYCHAR_HEIGHT )				txty = 476 - TINYCHAR_HEIGHT;
		if( x < 0 )											x = 0;
		else if( x > 638 )									x = 638;
		if( y < 0 )											y = 4;
		else if( y > 488 )									y = 488;

//		CG_DrawStringExt( txtx, txty, buff, NULL, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, distsize );
		CG_Text_Paint( txtx, txty, 0.2f, colorWhite, buff, 0, 0, 0, &cgDC.Assets.font, ITEM_ALIGN_LEFT );
		CG_DrawPic( x, y, 2, 2, cgs.media.whiteShader );

			// Draw any highlighting desired.
		if(	!highlighted &&
			fabs( angles[PITCH] ) <= WAYPOINTHIGHLIGHTANGLE &&
			fabs( angles[YAW] ) <= WAYPOINTHIGHLIGHTANGLE )
		{
			highlighted = qtrue;
			if( !wp->highlightStart )
			{
				dist = cg.time - wp->highlightEnd;
				wp->highlightStart = dist < WAYPOINTHIGHLIGHTTIME ? (cg.time - WAYPOINTHIGHLIGHTTIME + dist) : cg.time;
				wp->highlightEnd = 0;
			}
		}
		else if( wp->highlightStart )
		{
			dist = cg.time - wp->highlightStart;
			wp->highlightEnd = dist < WAYPOINTHIGHLIGHTTIME ? (cg.time - WAYPOINTHIGHLIGHTTIME + dist) : cg.time;
			wp->highlightStart = 0;
		}
		if( wp->highlightStart || wp->highlightEnd )
		{
			if( !CG_Q3F_WaypointDetail( x, y, wp, vec ) )
				wp->highlightStart = wp->highlightEnd = 0;
		}

		// 'psuedo-project' line to an abitrary clipping plane -
		// we already know the angle, so we can work out an amount based on
		// the distance from the player.
/*		dist = wp->distance / 5000.0f;
		txtx = 320 + (x - 320) * (dist < 1.0f ? dist : 1.0f);
		txty = 240 + (y - 240) * (dist < 1.0f ? dist : 1.0f);

		colour[0] = colour[1] = 0;
		colour[2] = 1.0f;
		colour[3] = 0.5;
		
		CG_Q3F_WaypointDrawLine( x, y, txtx, txty, 1, cgs.media.whiteShader, colour );*/
	}
}

/******************************************************************************
***** Waypoint handling functions
****/

void CG_Q3F_WaypointInit()
{
	// Clear out the waypoint table before use.

	memset( waypoints, 0, sizeof(waypoints) );
}

void CG_Q3F_WaypointExpire( int index )
{
	// Remove the specified waypoint.

	cg_q3f_waypoint_t *wp;

	wp = &waypoints[index];
	wp->time = 0;
	CG_Printf( BOX_PRINT_MODE_CHAT, "Waypoint \"%s^7\" at %s^7 expired.\n", wp->message, wp->loc );
}

static int wpnexttime;
static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};
void CG_Q3F_WaypointMaintain()
{
	// Expire and delete reached waypoints.

	int index, minTime;
	cg_q3f_waypoint_t *wp, *reached;
#if WAYPOINTREACHTRACE
	trace_t tr;
#endif

	if( cg.time < wpnexttime )
		return;
	wpnexttime = cg.time + WAYPOINTMAINTAINTIME;
	minTime = cg.time - MAXWAYPOINTAGE;

	reached = NULL;
	for( index = 0; index < MAXWAYPOINTS; index++ )
	{
		wp = &waypoints[index];

		if( !wp->time )
			continue;
		if( wp->time < minTime )
			CG_Q3F_WaypointExpire( index );
		else if( !reached && wp->distance < WAYPOINTREACHDISTANCE )
		{
#if WAYPOINTREACHTRACE
			CG_Trace(	&tr, wp->pos, playerMins, playerMaxs, cg.predictedPlayerState.origin,
						cg.predictedPlayerState.clientNum, MASK_PLAYERSOLID );
			if( tr.fraction == 1 && !tr.startsolid )
				reached = wp;
			else if( tr.startsolid )
			{
				CG_Trace(	&tr, wp->pos, NULL, NULL, cg.predictedPlayerState.origin,
							cg.predictedPlayerState.clientNum, MASK_PLAYERSOLID );
				if( tr.startsolid || tr.fraction == 1 )
					reached = wp;
			}
#else
			reached = wp;
#endif
		}
	}

	if( reached )
	{
		// Play a sound, centerprint?

		CG_Printf( BOX_PRINT_MODE_CHAT, "Reached waypoint \"%s^7\" at %s^7.\n", reached->message, reached->loc );
		reached->time = 0;
	}
}

void CG_Q3F_WaypointCommand()
{
	// The user has been sent a waypoint by the server.
	// Format is: sendernum x y z "message"

	int index, wpIndex, oldIndex, sender, oldTime;
	vec3_t pos;
	char buff[256];
	cg_q3f_waypoint_t *wp;

	trap_Argv( 1, buff, sizeof(buff) );		sender = atoi( buff );
	trap_Argv( 2, buff, sizeof(buff) );		pos[0] = atof( buff );
	trap_Argv( 3, buff, sizeof(buff) );		pos[1] = atof( buff );
	trap_Argv( 4, buff, sizeof(buff) );		pos[2] = atof( buff );
	trap_Argv( 5, buff, sizeof(buff) );

	for(	index = 0, wpIndex = oldIndex = -1, oldTime = cg.time + 1;
			index < MAXWAYPOINTS; index++ )
	{
		wp = &waypoints[index];
		if( VectorCompare( wp->pos, pos ) )
		{
			wpIndex = index;
			break;
		}
		if( !wp->time )
			wpIndex = index;
		else if( oldTime > wp->time )
			oldIndex = index;
	}

	if( wpIndex < 0 )
	{
		CG_Q3F_WaypointExpire( oldIndex );
		wpIndex = oldIndex;
	}

	wp = &waypoints[wpIndex];
	VectorCopy( pos, wp->pos );
	Q_strncpyz( wp->message, buff, sizeof(wp->message) );
	wp->time = cg.time;
	if( !(wp->loc = CG_Q3F_GetLocation( pos, qtrue )) )
		wp->loc = "Unknown Location";
	wp->locwidth		= TINYCHAR_WIDTH * strlen( wp->loc );
	wp->messagewidth	= TINYCHAR_WIDTH * strlen( wp->message );
	wp->highlightEnd = wp->highlightStart = 0;

	CG_Printf(	BOX_PRINT_MODE_CHAT, "Received waypoint from %s^7, \"%s^7\" at %s^7.\n",
				cgs.clientinfo[sender].name, buff, wp->loc );
}
