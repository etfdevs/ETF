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
**	cg_q3f_atmospheric.c
**
**	Add atmospheric effects (e.g. rain, snow etc.) to view.
**
**	Current supported effects are rain and snow.
*/

#ifdef API_Q3

#include "cg_local.h"

// some culling bits
typedef struct plane_s {
	vec3_t	normal;
	float	dist;
} plane_t;

static plane_t frustum[4];

//
//	CG_SetupFrustum
//
static void CG_SetupFrustum( void ) {
	int		i;
	float	xs, xc;
	float	ang;

	ang = cg.refdef.fov_x / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[0].normal );
	VectorMA( frustum[0].normal, xc, cg.refdef.viewaxis[1], frustum[0].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[1].normal );
	VectorMA( frustum[1].normal, -xc, cg.refdef.viewaxis[1], frustum[1].normal );

	ang = cg.refdef.fov_y / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[2].normal );
	VectorMA( frustum[2].normal, xc, cg.refdef.viewaxis[2], frustum[2].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[3].normal );
	VectorMA( frustum[3].normal, -xc, cg.refdef.viewaxis[2], frustum[3].normal );

	for (i=0 ; i<4 ; i++) {
		frustum[i].dist = DotProduct( cg.refdef.vieworg, frustum[i].normal);
	}
}

//
//	CG_CullPointAndRadius - returns true if not culled
//
qboolean CG_CullPointAndRadius( const vec3_t pt, float radius ) {
	int		i;
	float	dist;
	plane_t	*frust;

	// check against frustum planes
	for (i = 0 ; i < 4 ; i++) {
		frust = &frustum[i];

		dist = DotProduct( pt, frust->normal) - frust->dist;
		if ( dist < -radius || dist <= radius )
			return( qfalse );
	}

	return( qtrue );
}

#define	Q3F_MAX_ATMOSPHERIC_PARTICLES		1000	// maximum # of particles
#define	Q3F_MAX_ATMOSPHERIC_DISTANCE		1000	// maximum distance from refdef origin that particles are visible
#define	Q3F_MAX_ATMOSPHERIC_HEIGHT			4096	// maximum world height (FIXME: since 1.27 this should be 65536)
#define	Q3F_MIN_ATMOSPHERIC_HEIGHT			-4096	// minimum world height (FIXME: since 1.27 this should be -65536)
#define Q3F_MAX_ATMOSPHERIC_EFFECTSHADERS	6		// maximum different effectshaders for an atmospheric effect
#define	Q3F_ATMOSPHERIC_DROPDELAY			1000
#define	Q3F_ATMOSPHERIC_CUTHEIGHT			800

#define	Q3F_ATMOSPHERIC_RAIN_SPEED		(1.1f * DEFAULT_GRAVITY)
#define	Q3F_ATMOSPHERIC_RAIN_HEIGHT		150

#define	Q3F_ATMOSPHERIC_SNOW_SPEED		(0.1f * DEFAULT_GRAVITY)
#define	Q3F_ATMOSPHERIC_SNOW_HEIGHT		10

typedef enum {
	ACT_NOT,
	ACT_FALLING,
	ACT_HITGROUND
} active_t;

typedef struct cg_q3f_atmosphericParticle_s {
	vec3_t pos, delta, deltaNormalized, colour, surfacenormal, surfaceimpactpos;
	float height, minz, weight;
	active_t active;
	int contents, surface, nextDropTime;
	qhandle_t *effectshader;
} cg_q3f_atmosphericParticle_t;

typedef struct cg_q3f_atmosphericEffect_s {
	cg_q3f_atmosphericParticle_t particles[Q3F_MAX_ATMOSPHERIC_PARTICLES];
	qhandle_t effectshaders[Q3F_MAX_ATMOSPHERIC_EFFECTSHADERS];
	qhandle_t effectwatershader, effectlandshader;
	int lastRainTime, numDrops;
	int gustStartTime, gustEndTime;
	int baseStartTime, baseEndTime;
	int gustMinTime, gustMaxTime;
	int changeMinTime, changeMaxTime;
	int baseMinTime, baseMaxTime;
	float baseWeight, gustWeight;
	int baseDrops, gustDrops;
	int numEffectShaders;
	qboolean waterSplash, landSplash;
	vec3_t baseVec, gustVec;

	vec3_t viewDir;

	qboolean (*ParticleCheckVisible)( cg_q3f_atmosphericParticle_t *particle );
	qboolean (*ParticleGenerate)( cg_q3f_atmosphericParticle_t *particle, vec3_t currvec, float currweight );
	void (*ParticleRender)( cg_q3f_atmosphericParticle_t *particle );

	int dropsRendered, dropsCreated, dropsSkipped;
} cg_q3f_atmosphericEffect_t;

static cg_q3f_atmosphericEffect_t cg_q3f_atmFx;

/*
**  Render utility functions
*/

void CG_Q3F_EffectMark(	qhandle_t markShader, const vec3_t origin, const vec3_t dir, float alpha, float radius ) {
	// 'quick' version of the CG_ImpactMark function

	vec3_t			axis[3];
	float			texCoordScale;
	vec3_t			originalPoints[4];
	byte			colors[4];
	int				i;//, j;
//	vec3_t			projection;
	polyVert_t		*v;
	polyVert_t		verts[4];

	if ( !cg_addMarks.integer ) {
		return;
	}

	if ( radius <= 0 ) {
		CG_Error( "CG_Q3F_EffectMark called with <= 0 radius" );
	}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	VectorSet( axis[2], 1, 0, 0 );			// This is _wrong_, but the function is for water anyway (i.e. usually flat)
//	RotatePointAroundVector( axis[2], axis[0], axis[1], 0 );		// Should be able to get rid of this?
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	colors[0] = 127;//red * 255;
	colors[1] = 127;//green * 255;
	colors[2] = 127;//blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, v = verts ; i < 4 ; i++, v++ ) {
		vec3_t		delta;

		VectorCopy( originalPoints[i], v->xyz );

		VectorSubtract( v->xyz, origin, delta );
		v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
		v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
		*(int *)v->modulate = *(int *)colors;
	}

	trap_R_AddPolyToScene( markShader, 4, verts );
}

/*
**	Raindrop management functions
*/

static qboolean CG_Q3F_RainParticleCheckVisible( cg_q3f_atmosphericParticle_t *particle )
{
	// Check the raindrop is visible and still going, wrapping if necessary.

	float moved;
	vec3_t distance;
	trace_t tr;

	if( !particle || particle->active == ACT_NOT )
		return( qfalse );

	moved = (cg.time - cg_q3f_atmFx.lastRainTime) * 0.001;	// Units moved since last frame
	VectorMA( particle->pos, moved, particle->delta, particle->pos );
	if( particle->pos[2] + Q3F_ATMOSPHERIC_CUTHEIGHT < particle->minz )
		return( particle->active = qfalse );

	VectorSubtract( particle->pos, cg.refdef.vieworg, distance );
	//if( (moved = Q_rsqrt( distance[0] * distance[0] + distance[1] * distance[1] )) < (1.0f / Q3F_MAX_ATMOSPHERIC_DISTANCE) )
	if( (moved = SQRTFAST( distance[0] * distance[0] + distance[1] * distance[1] )) < (1.0f / Q3F_MAX_ATMOSPHERIC_DISTANCE) )
	{
			// Don't respot particles to front or either side, just back.
		moved = DotProduct( cg_q3f_atmFx.viewDir, distance ) * moved;
		if( moved > -0.2f )// && moved < 0.2f )
			return( particle->active = ACT_NOT );

			// Attempt to respot the particle in front of us instead.
		particle->pos[0] -= 1.5 * distance[0];
		particle->pos[1] -= 1.5 * distance[1];
		VectorMA( particle->pos, -2 * Q3F_MAX_ATMOSPHERIC_HEIGHT, particle->deltaNormalized, distance );

			// Successful trace?
		CG_Trace( &tr, particle->pos, NULL, NULL, distance, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
		if( tr.startsolid || tr.fraction == 1 || !(tr.surfaceFlags & SURF_SKY) )
			return( particle->active = ACT_NOT );

			// Reset values for respotted particle.
		VectorMA( particle->pos, 2 * Q3F_MAX_ATMOSPHERIC_HEIGHT, particle->deltaNormalized, distance );
		CG_Trace( &tr, particle->pos, NULL, NULL, distance, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
		if( tr.startsolid || tr.fraction == 1 )
			return( particle->active = ACT_NOT );
		particle->minz = tr.endpos[2];
		tr.endpos[2]--;
		VectorCopy( tr.plane.normal, particle->surfacenormal );
		particle->surface = tr.surfaceFlags;
		particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );
	}

	return( qtrue );
}

static qboolean CG_Q3F_RainParticleGenerate( cg_q3f_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
	// Attempt to 'spot' a raindrop somewhere below a sky texture.

	float angle, distance, origz;
	vec3_t testpoint, testend;
	trace_t tr;

	cg_q3f_atmFx.dropsCreated++;

	angle = random() * 2*M_PI;
	distance = 20 + Q3F_MAX_ATMOSPHERIC_DISTANCE * random();

	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
	testpoint[2] = origz = cg.refdef.vieworg[2];
	testend[2] = testpoint[2] + Q3F_MAX_ATMOSPHERIC_HEIGHT;

	while( 1 )
	{
		// Perform traces up to the sky, repeating at a higher start height if we start
		// inside a solid.

		if( testpoint[2] >= Q3F_MAX_ATMOSPHERIC_HEIGHT )
			return( qfalse );
		if( testend[2] >= Q3F_MAX_ATMOSPHERIC_HEIGHT )
			testend[2] = Q3F_MAX_ATMOSPHERIC_HEIGHT - 1;
		CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
		if( tr.startsolid )			// Stuck in something, skip over it.
		{
			testpoint[2] += 64;
			testend[2] = testpoint[2] + Q3F_MAX_ATMOSPHERIC_HEIGHT;
		}
		else if( tr.fraction == 1 )		// Didn't hit anything, we're (probably) outside the world
			return( qfalse );
		else if( tr.surfaceFlags & SURF_SKY )	// Hit sky, this is where we start.
			break;
		else return( qfalse );
	}
//	return( qfalse );

	particle->active = ACT_FALLING;
	particle->colour[0] = 0.6 + 0.2 * random();
	particle->colour[1] = 0.6 + 0.2 * random();
	particle->colour[2] = 0.6 + 0.2 * random();
	VectorCopy( tr.endpos, particle->pos );
	VectorCopy( currvec, particle->delta );
	particle->delta[2] += crandom() * 100;
	VectorNormalize2( particle->delta, particle->deltaNormalized );
	particle->height = Q3F_ATMOSPHERIC_RAIN_HEIGHT + crandom() * 100;
	particle->weight = currweight;
	particle->effectshader = &cg_q3f_atmFx.effectshaders[0];

	distance =	((float)(tr.endpos[2] - Q3F_MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
	VectorMA( tr.endpos, distance, particle->delta, testend );
//	testend[2] -= 10240;
//	if( testend[2] <= Q3F_ATMOSPHERIC_MINHEIGHT )
//		testend[2] = Q3F_ATMOSPHERIC_MINHEIGHT + 1;
//	testend[0] += (tr.endpos[2] - testend[2]) * 0.1;
	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
	particle->minz = tr.endpos[2];
	tr.endpos[2]--;
	VectorCopy( tr.plane.normal, particle->surfacenormal );
	particle->surface = tr.surfaceFlags;
	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

	return( qtrue );
}

static void CG_Q3F_RainParticleRender( cg_q3f_atmosphericParticle_t *particle )
{
	// Draw a raindrop

	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec2_t		line;
	float		len, frac, dist;
	vec3_t		start, finish;
	qboolean	inFrustum;

	if( particle->active == ACT_NOT )
		return;

	if( inFrustum = CG_CullPointAndRadius( particle->pos, particle->weight ) ) {
		cg_q3f_atmFx.dropsRendered++;
		line[0] = particle->pos[0] - cg.refdef.vieworg[0];
		line[1] = particle->pos[1] - cg.refdef.vieworg[1];
		dist = sqrt( line[0]*line[0] + line[1]*line[1] );
	} else
		cg_q3f_atmFx.dropsSkipped++;

	VectorCopy( particle->pos, start );
	len = particle->height;
	if( start[2] <= particle->minz && particle->minz < cg.refdef.vieworg[2] )
	{
		// Stop rain going through surfaces.
		len = particle->height - particle->minz + start[2];
		frac = start[2];
		VectorMA( start, len - particle->height, particle->deltaNormalized, start );

		//if( !cg_lowEffects.integer )
		if( inFrustum )
		{
			frac = (Q3F_ATMOSPHERIC_CUTHEIGHT - particle->minz + frac) / (float) Q3F_ATMOSPHERIC_CUTHEIGHT;
			// Splash effects on different surfaces
			if( particle->contents & (CONTENTS_WATER|CONTENTS_SLIME) )
			{
				// Water splash
				if( cg_q3f_atmFx.effectwatershader && frac > 0 && frac < 1 && dist < 196 ) {
					if( particle->active != ACT_HITGROUND ) {
						particle->active = ACT_HITGROUND;
						VectorCopy( start, particle->surfaceimpactpos );
					}
					CG_Q3F_EffectMark( cg_q3f_atmFx.effectwatershader, start, particle->surfacenormal, frac * 0.5, 8 - frac * 8 );
				}
			}
			else if( !(particle->contents & CONTENTS_LAVA) && !(particle->surface & (SURF_NODAMAGE|SURF_NOIMPACT|SURF_NOMARKS|SURF_SKY)) )
			{
				// Solid splash
				if( cg_q3f_atmFx.effectlandshader && frac > 0 && frac < 1 && dist < 196 ) {
//					CG_ImpactMark( cg_atmFx.effectlandshader, start, particle->surfacenormal, 0, 1, 1, 1, frac * 0.5, qfalse, 3 - frac * 2, qtrue );
					if( particle->active != ACT_HITGROUND ) {
						particle->active = ACT_HITGROUND;
						VectorCopy( start, particle->surfaceimpactpos );
					}
					CG_Q3F_EffectMark( cg_q3f_atmFx.effectlandshader, start, particle->surfacenormal, frac * 0.5, 3 - frac * 2 );
				}
			}
		}
	}
	if( len <= 0 || !inFrustum )
		return;

	// fade nearby rain particles
	if( dist <= 128 )
		dist = .25f + .75f * (dist / 128.f);

	VectorCopy( particle->deltaNormalized, forward );
	VectorMA( start, -len, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	// render square
	VectorMA( finish, particle->weight, right, verts[0].xyz );
	verts[0].st[0] = 1;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 100 * dist;

	VectorMA( finish, -particle->weight, right, verts[1].xyz );
	verts[1].st[0] = 0;

	// render triangle
	// VectorCopy( finish, verts[1].xyz );	
	// verts[1].st[0] = 0.5f;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 100 * dist;

	VectorMA( start, -particle->weight, right, verts[2].xyz );
	verts[2].st[0] = 0;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 200 * dist;

	VectorMA( start, particle->weight, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 1;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 200 * dist;

	// render square
	trap_R_AddPolyToScene( *particle->effectshader, 4, verts );
	// render triangle
	//trap_R_AddPolyToScene( *particle->effectshader, 3, verts + 1 );
}


/*
**	Snow management functions
*/

static qboolean CG_Q3F_SnowParticleGenerate( cg_q3f_atmosphericParticle_t *particle, vec3_t currvec, float currweight )
{
	// Attempt to 'spot' a raindrop somewhere below a sky texture.

	float angle, distance, origz;
	vec3_t testpoint, testend;
	trace_t tr;

	angle = random() * 2*M_PI;
	distance = 20 + Q3F_MAX_ATMOSPHERIC_DISTANCE * random();

	testpoint[0] = testend[0] = cg.refdef.vieworg[0] + sin(angle) * distance;
	testpoint[1] = testend[1] = cg.refdef.vieworg[1] + cos(angle) * distance;
	testpoint[2] = origz = cg.refdef.vieworg[2];
	testend[2] = testpoint[2] + Q3F_MAX_ATMOSPHERIC_HEIGHT;

	while( 1 )
	{
		if( testpoint[2] >= Q3F_MAX_ATMOSPHERIC_HEIGHT )
			return( qfalse );
		if( testend[2] >= Q3F_MAX_ATMOSPHERIC_HEIGHT )
			testend[2] = Q3F_MAX_ATMOSPHERIC_HEIGHT - 1;
		CG_Trace( &tr, testpoint, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
		if( tr.startsolid )			// Stuck in something, skip over it.
		{
			testpoint[2] += 64;
			testend[2] = testpoint[2] + Q3F_MAX_ATMOSPHERIC_HEIGHT;
		}
		else if( tr.fraction == 1 )		// Didn't hit anything, we're (probably) outside the world
			return( qfalse );
		else if( tr.surfaceFlags & SURF_SKY )	// Hit sky, this is where we start.
			break;
		else return( qfalse );
	}

	particle->active = ACT_FALLING;
	particle->colour[0] = 0.6 + 0.2 * random();
	particle->colour[1] = 0.6 + 0.2 * random();
	particle->colour[2] = 0.6 + 0.2 * random();
	VectorCopy( tr.endpos, particle->pos );
	VectorCopy( currvec, particle->delta );
	particle->delta[2] += crandom() * 25;
	VectorNormalize2( particle->delta, particle->deltaNormalized );
	particle->height = Q3F_ATMOSPHERIC_SNOW_HEIGHT + crandom() * 8;
	particle->weight = particle->height * 0.5f;
	particle->effectshader = &cg_q3f_atmFx.effectshaders[ (int) (random() * ( cg_q3f_atmFx.numEffectShaders - 1 )) ];

	distance =	((float)(tr.endpos[2] - Q3F_MIN_ATMOSPHERIC_HEIGHT)) / -particle->delta[2];
	VectorMA( tr.endpos, distance, particle->delta, testend );
	CG_Trace( &tr, particle->pos, NULL, NULL, testend, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER );
	particle->minz = tr.endpos[2];
	tr.endpos[2]--;
	VectorCopy( tr.plane.normal, particle->surfacenormal );
	particle->surface = tr.surfaceFlags;
	particle->contents = CG_PointContents( tr.endpos, ENTITYNUM_NONE );

	return( qtrue );
}

static void CG_Q3F_SnowParticleRender( cg_q3f_atmosphericParticle_t *particle )
{
	// Draw a snowflake

	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec2_t		line;
	float		len, frac, sinTumbling, cosTumbling, particleWidth;
	vec3_t		start, finish;

	if( particle->active == ACT_NOT )
		return;

	VectorCopy( particle->pos, start );

	sinTumbling = sin( particle->pos[2] * 0.03125f );
	cosTumbling = cos( ( particle->pos[2] + particle->pos[1] )  * 0.03125f );
	start[0] += 24 * ( 1 - particle->deltaNormalized[2] ) * sinTumbling;
	start[1] += 24 * ( 1 - particle->deltaNormalized[2] ) * cosTumbling;

	len = particle->height;
	if( start[2] <= particle->minz )
	{
		// Stop snow going through surfaces.
		len = particle->height - particle->minz + start[2];
		frac = start[2];
		VectorMA( start, len - particle->height, particle->deltaNormalized, start );
	}
	if( len <= 0 )
		return;

	VectorCopy( particle->deltaNormalized, forward );
	VectorMA( start, -( len * sinTumbling ), forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	particleWidth = cosTumbling * particle->weight;

	VectorMA( finish, particleWidth, right, verts[0].xyz );
	verts[0].st[0] = 1;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -particleWidth, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -particleWidth, right, verts[2].xyz );
	verts[2].st[0] = 0;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, particleWidth, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 1;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( *particle->effectshader, 4, verts );
}

/*
**	Set up gust parameters.
*/

static void CG_Q3F_EffectGust()
{
	// Generate random values for the next gust

	int diff;

	cg_q3f_atmFx.baseEndTime		= cg.time						+ cg_q3f_atmFx.baseMinTime		+ (rand() % (cg_q3f_atmFx.baseMaxTime - cg_q3f_atmFx.baseMinTime));
	diff							= cg_q3f_atmFx.changeMaxTime	- cg_q3f_atmFx.changeMinTime;
	cg_q3f_atmFx.gustStartTime		= cg_q3f_atmFx.baseEndTime		+ cg_q3f_atmFx.changeMinTime	+ (diff ? (rand() % diff) : 0);
	diff							= cg_q3f_atmFx.gustMaxTime		- cg_q3f_atmFx.gustMinTime;
	cg_q3f_atmFx.gustEndTime		= cg_q3f_atmFx.gustStartTime	+ cg_q3f_atmFx.gustMinTime		+ (diff ? (rand() % diff) : 0);
	diff							= cg_q3f_atmFx.changeMaxTime	- cg_q3f_atmFx.changeMinTime;
	cg_q3f_atmFx.baseStartTime		= cg_q3f_atmFx.gustEndTime		+ cg_q3f_atmFx.changeMinTime	+ (diff ? (rand() % diff) : 0);
}

static qboolean CG_Q3F_EffectGustCurrent( vec3_t curr, float *weight, int *num )
{
	// Calculate direction for new drops.

	vec3_t temp;
	float frac;

	if( cg.time < cg_q3f_atmFx.baseEndTime )
	{
		VectorCopy( cg_q3f_atmFx.baseVec, curr );
		*weight = cg_q3f_atmFx.baseWeight;
		*num = cg_q3f_atmFx.baseDrops;
	}
	else {
		VectorSubtract( cg_q3f_atmFx.gustVec, cg_q3f_atmFx.baseVec, temp );
		if( cg.time < cg_q3f_atmFx.gustStartTime )
		{
			frac = ((float)(cg.time - cg_q3f_atmFx.baseEndTime))/((float)(cg_q3f_atmFx.gustStartTime - cg_q3f_atmFx.baseEndTime));
			VectorMA( cg_q3f_atmFx.baseVec, frac, temp, curr );
			*weight = cg_q3f_atmFx.baseWeight + (cg_q3f_atmFx.gustWeight - cg_q3f_atmFx.baseWeight) * frac;
			*num = cg_q3f_atmFx.baseDrops + ((float)(cg_q3f_atmFx.gustDrops - cg_q3f_atmFx.baseDrops)) * frac;
		}
		else if( cg.time < cg_q3f_atmFx.gustEndTime )
		{
			VectorCopy( cg_q3f_atmFx.gustVec, curr );
			*weight = cg_q3f_atmFx.gustWeight;
			*num = cg_q3f_atmFx.gustDrops;
		}
		else
		{
			frac = 1.0 - ((float)(cg.time - cg_q3f_atmFx.gustEndTime))/((float)(cg_q3f_atmFx.baseStartTime - cg_q3f_atmFx.gustEndTime));
			VectorMA( cg_q3f_atmFx.baseVec, frac, temp, curr );
			*weight = cg_q3f_atmFx.baseWeight + (cg_q3f_atmFx.gustWeight - cg_q3f_atmFx.baseWeight) * frac;
			*num = cg_q3f_atmFx.baseDrops + ((float)(cg_q3f_atmFx.gustDrops - cg_q3f_atmFx.baseDrops)) * frac;
			if( cg.time >= cg_q3f_atmFx.baseStartTime )
				return( qtrue );
		}
	}
	return( qfalse );
}

static void CG_Q3F_EP_ParseFloats( char *floatstr, float *f1, float *f2 )
{
	// Parse the float or floats

	char *middleptr;
	char buff[64];

	Q_strncpyz( buff, floatstr, sizeof(buff) );
	for( middleptr = buff; *middleptr && *middleptr != ' '; middleptr++ );
	if( *middleptr )
	{
		*middleptr++ = 0;
		*f1 = atof( floatstr );
		*f2 = atof( middleptr );
	}
	else {
		*f1 = *f2 = atof( floatstr );
	}
}
void CG_Q3F_EffectParse( const char *effectstr )
{
	// Split the string into it's component parts.

	float bmin, bmax, cmin, cmax, gmin, gmax, bdrop, gdrop, wsplash, lsplash;
	int count;
	char *startptr, *eqptr, *endptr, *type;
	char workbuff[128];

	if( CG_Q3F_AtmosphericKludge() )
		return;

		// Set up some default values
	cg_q3f_atmFx.baseVec[0] = cg_q3f_atmFx.baseVec[1] = 0;
	cg_q3f_atmFx.gustVec[0] = cg_q3f_atmFx.gustVec[1] = 100;
	bmin = 5;
	bmax = 10;
	cmin = 1;
	cmax = 1;
	gmin = 0;
	gmax = 2;
	bdrop = gdrop = 300;
	cg_q3f_atmFx.baseWeight = 0.7f;
	cg_q3f_atmFx.gustWeight = 1.5f;
	wsplash = 1;
	lsplash = 1;
	type = NULL;

		// Parse the parameter string
	Q_strncpyz( workbuff, effectstr, sizeof(workbuff) );
	for( startptr = workbuff; *startptr; )
	{
		for( eqptr = startptr; *eqptr && *eqptr != '=' && *eqptr != ','; eqptr++ );
		if( !*eqptr )
			break;			// No more string
		if( *eqptr == ',' )
		{
			startptr = eqptr + 1;	// Bad argument, continue
			continue;
		}
		*eqptr++ = 0;
		for( endptr = eqptr; *endptr && *endptr != ','; endptr++ );
		if( *endptr )
			*endptr++ = 0;

		if( !type )
		{
			if( Q_stricmp( startptr, "T" ) ) {
				cg_q3f_atmFx.numDrops = 0;
				CG_Printf( BOX_PRINT_MODE_CHAT, "Atmospheric effect must start with a type.\n" );
				return;
			}
			if( !Q_stricmp( eqptr, "RAIN" ) ) {
				type = "rain";
				cg_q3f_atmFx.ParticleCheckVisible = &CG_Q3F_RainParticleCheckVisible;
				cg_q3f_atmFx.ParticleGenerate = &CG_Q3F_RainParticleGenerate;
				cg_q3f_atmFx.ParticleRender = &CG_Q3F_RainParticleRender;

				cg_q3f_atmFx.baseVec[2] = cg_q3f_atmFx.gustVec[2] = - Q3F_ATMOSPHERIC_RAIN_SPEED;
			} else if( !Q_stricmp( eqptr, "SNOW" ) ) {
				type = "snow";
				cg_q3f_atmFx.ParticleCheckVisible = &CG_Q3F_RainParticleCheckVisible;
				cg_q3f_atmFx.ParticleGenerate = &CG_Q3F_SnowParticleGenerate;
				cg_q3f_atmFx.ParticleRender = &CG_Q3F_SnowParticleRender;

				cg_q3f_atmFx.baseVec[2] = cg_q3f_atmFx.gustVec[2] = - Q3F_ATMOSPHERIC_SNOW_SPEED;
			} else {
				cg_q3f_atmFx.numDrops = 0;
				CG_Printf( BOX_PRINT_MODE_CHAT, "Only effect type 'rain' and 'snow' are supported.\n" );
				return;
			}
		}
		else {
			if( !Q_stricmp( startptr, "B" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &bmin, &bmax );
			else if( !Q_stricmp( startptr, "C" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &cmin, &cmax );
			else if( !Q_stricmp( startptr, "G" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &gmin, &gmax );
			else if( !Q_stricmp( startptr, "BV" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &cg_q3f_atmFx.baseVec[0], &cg_q3f_atmFx.baseVec[1] );
			else if( !Q_stricmp( startptr, "GV" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &cg_q3f_atmFx.gustVec[0], &cg_q3f_atmFx.gustVec[1] );
			else if( !Q_stricmp( startptr, "W" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &cg_q3f_atmFx.baseWeight, &cg_q3f_atmFx.gustWeight );
			else if( !Q_stricmp( startptr, "S" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &wsplash, &lsplash );
			else if( !Q_stricmp( startptr, "D" ) )
				CG_Q3F_EP_ParseFloats( eqptr, &bdrop, &gdrop );
			else CG_Printf( BOX_PRINT_MODE_CHAT, "Unknown effect key '%s'.\n", startptr );
		}
		startptr = endptr;
	}

	if( !type )
	{
		// No effects

		cg_q3f_atmFx.numDrops = -1;
		return;
	}
		
	cg_q3f_atmFx.baseMinTime = 1000 * bmin;
	cg_q3f_atmFx.baseMaxTime = 1000 * bmax;
	cg_q3f_atmFx.changeMinTime = 1000 * cmin;
	cg_q3f_atmFx.changeMaxTime = 1000 * cmax;
	cg_q3f_atmFx.gustMinTime = 1000 * gmin;
	cg_q3f_atmFx.gustMaxTime = 1000 * gmax;
	cg_q3f_atmFx.baseDrops = bdrop;
	cg_q3f_atmFx.gustDrops = gdrop;
	cg_q3f_atmFx.waterSplash = wsplash;
	cg_q3f_atmFx.landSplash = lsplash;

	cg_q3f_atmFx.numDrops = (cg_q3f_atmFx.baseDrops > cg_q3f_atmFx.gustDrops) ? cg_q3f_atmFx.baseDrops : cg_q3f_atmFx.gustDrops;
	if( cg_q3f_atmFx.numDrops > Q3F_MAX_ATMOSPHERIC_PARTICLES )
		cg_q3f_atmFx.numDrops = Q3F_MAX_ATMOSPHERIC_PARTICLES;

		// Load graphics

	// Rain
	if( type == "rain" ) {
		cg_q3f_atmFx.numEffectShaders = 1;
		cg_q3f_atmFx.effectshaders[0] = trap_R_RegisterShader( "gfx/misc/raindrop" );
		if( !(cg_q3f_atmFx.effectshaders[0]) )
			cg_q3f_atmFx.effectshaders[0] = -1;
		if( cg_q3f_atmFx.waterSplash )
			cg_q3f_atmFx.effectwatershader = trap_R_RegisterShader( "gfx/misc/raindropwater" );
		if( cg_q3f_atmFx.landSplash )
			cg_q3f_atmFx.effectlandshader = trap_R_RegisterShader( "gfx/misc/raindropsolid" );

	// Snow
	} else if( type == "snow" ) {
		for( cg_q3f_atmFx.numEffectShaders = 0; cg_q3f_atmFx.numEffectShaders < 6; cg_q3f_atmFx.numEffectShaders++ ) {
			if( !( cg_q3f_atmFx.effectshaders[cg_q3f_atmFx.numEffectShaders] = trap_R_RegisterShader( va("gfx/misc/snowflake0%i", cg_q3f_atmFx.numEffectShaders ) ) ) )
				cg_q3f_atmFx.effectshaders[cg_q3f_atmFx.numEffectShaders] = -1;	// we had some kind of a problem
		}
		cg_q3f_atmFx.waterSplash = 0;
		cg_q3f_atmFx.landSplash = 0;

	// This really should never happen
	} else
		cg_q3f_atmFx.numEffectShaders = 0;

		// Initialise atmospheric effect to prevent all particles falling at the start
	for( count = 0; count < cg_q3f_atmFx.numDrops; count++ )
		cg_q3f_atmFx.particles[count].nextDropTime = Q3F_ATMOSPHERIC_DROPDELAY + (rand() % Q3F_ATMOSPHERIC_DROPDELAY);

	CG_Q3F_EffectGust();
}

/*
** Main render loop
*/

void CG_Q3F_AddAtmosphericEffects()
{
	// Add atmospheric effects (e.g. rain, snow etc.) to view

	int curr, max, currnum;
	cg_q3f_atmosphericParticle_t *particle;
	vec3_t currvec;
	float currweight;

	if( cg_q3f_atmFx.numDrops <= 0 || cg_q3f_atmFx.numEffectShaders == 0 || !cg_atmosphericEffects.integer )
		return;

	CG_SetupFrustum();

	max = cg_lowEffects.integer ? (cg_q3f_atmFx.numDrops >> 1) : cg_q3f_atmFx.numDrops;
	if( CG_Q3F_EffectGustCurrent( currvec, &currweight, &currnum ) )
		CG_Q3F_EffectGust();			// Recalculate gust parameters

	cg_q3f_atmFx.dropsRendered = cg_q3f_atmFx.dropsCreated = cg_q3f_atmFx.dropsSkipped;

	VectorSet( cg_q3f_atmFx.viewDir, 0, cg.refdefViewAngles[YAW], 0 );
	AngleVectors( cg_q3f_atmFx.viewDir, cg_q3f_atmFx.viewDir, NULL, NULL );

	for( curr = 0; curr < max; curr++ )
	{
		particle = &cg_q3f_atmFx.particles[curr];
		if( !cg_q3f_atmFx.ParticleCheckVisible( particle ) )
		{
			// Effect has terminated / fallen from screen view

			if( !particle->nextDropTime )
			{
				// Stop rain being synchronized 
				particle->nextDropTime = cg.time + rand() % Q3F_ATMOSPHERIC_DROPDELAY;
			}
			if( currnum < curr || particle->nextDropTime > cg.time )
			{
				cg_q3f_atmFx.dropsRendered++;
				continue;
			}
			if( !cg_q3f_atmFx.ParticleGenerate( particle, currvec, currweight ) )
			{
				// Ensure it doesn't attempt to generate every frame, to prevent
				// 'clumping' when there's only a small sky area available.
				particle->nextDropTime = cg.time + Q3F_ATMOSPHERIC_DROPDELAY;
				continue;
			}
		}

		cg_q3f_atmFx.ParticleRender( particle );
	}

	cg_q3f_atmFx.lastRainTime = cg.time;
//	CG_Printf( "Generated: %d Rendered: %d Skipped: %d\n", cg_q3f_atmFx.dropsCreated, cg_q3f_atmFx.dropsRendered, cg_q3f_atmFx.dropsSkipped );
}


/*
**	G_Q3F_AtmosphericKludge
*/

static qboolean kludgeChecked, kludgeResult;
qboolean CG_Q3F_AtmosphericKludge()
{
	// Activate rain for specified kludge maps that don't
	// have it specified for them.

	if( kludgeChecked )
		return( kludgeResult );
	kludgeChecked = qtrue;
	kludgeResult = qfalse;

	if( !Q_stricmp( cgs.mapname, "maps/etf_2night3.bsp" ) )
	{
		CG_Q3F_EffectParse( "T=RAIN,D=800 800" );
		return( kludgeResult = qtrue );
	} else if( !Q_stricmp( cgs.mapname, "maps/etf_castles.bsp" ) ) {
		CG_Q3F_EffectParse( "T=SNOW,BV=30 30,GV=60 20,D=800 800" );
		return( kludgeResult = qtrue );
	}

	return( kludgeResult = qfalse );
}

#endif