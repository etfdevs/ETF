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

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include "cg_local.h"

#define	MAX_LOCAL_ENTITIES	512
localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
localEntity_t	cg_activeLocalEntities;		// double linked list
localEntity_t	*cg_freeLocalEntities;		// single linked list

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
	int		i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i+1];
	}
}


/*
==================
CG_FreeLocalEntity
==================
*/
void CG_FreeLocalEntity( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t dummyLocalEnt;
localEntity_t	*CG_AllocLocalEntity( int duration ) {
	localEntity_t	*le;

	// RR2DO2: don't draw impactmarks during initialization
	if( cgs.initPhase || cg.infoScreenText[0] != 0 ) {
		// They're not getting a _real_ localentity, they're invisible.

		memset( &dummyLocalEnt, 0, sizeof(dummyLocalEnt) );
		return( &dummyLocalEnt );
	}

	if( cg.drawFilter )
	{
		// They're not getting a _real_ localentity, they're invisible.

		memset( &dummyLocalEnt, 0, sizeof(dummyLocalEnt) );
		return( &dummyLocalEnt );
	}

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		CG_FreeLocalEntity( cg_activeLocalEntities.prev );
	}

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;

	/* Fill up some commonly used variables */
	le->startTime = cg.time;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0f / ( duration );
//	le->refEntity.shaderTime = 0.001 * cg.time;

	return le;
}

/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/

/*
================
CG_BloodTrail

Leave expanding blood puffs behind gibs
================
*/
static void CG_BloodTrail( localEntity_t *le ) {
	int		t;
	int		t2;
	int		step;
	vec3_t	newOrigin;
	localEntity_t	*blood;

	step = 150;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

		blood = CG_SmokePuff( newOrigin, vec3_origin, 
			2000, 200,			
			20, 30,
			colorWhite, cgs.media.bloodTrailShader );
		// use the optimized version
		blood->leType = LE_FALL_SCALE_FADE;
		// drop a total of 40 units over its lifetime
		blood->pos.trDelta[2] = 40;
	}
}


/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta );

	VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid || 
		( trace->plane.normal[2] > 0 && 
		( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {
		le->pos.trType = TR_STATIONARY;
	} else {

	}
}

/*
================
CG_AddFragment
================
*/
static void CG_AddFragment( localEntity_t *le ) {
	vec3_t	newOrigin;
	vec4_t	projection;
	trace_t	trace;

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity, NULL );
			le->refEntity.origin[2] = oldZ;
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity, NULL );
		}

		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE ) {
			vec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
		}

		trap_R_AddRefEntityToScene( &le->refEntity, NULL );

		// add a blood trail
		if (le->leFlags & LEF_MARK_BLOOD) {
			CG_BloodTrail( le );
		}
		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( CG_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	if (le->leFlags & LEF_MARK_BLOOD) {
		VectorSet4( projection, 0, 0, -1, 16 + (rand()&31) );
		trap_R_ProjectDecal( cgs.media.bloodMarkShader, 1, &trace.endpos, projection, colorWhite,
					20000, 20000 >> 4 );
	} else if (le->leFlags & LEF_MARK_BURN) {
		VectorSet4( projection, 0, 0, -1, 8 + (rand()&15) );
		trap_R_ProjectDecal( cgs.media.burnMarkShader, 1, &trace.endpos, projection, colorWhite,
					20000, 20000 >> 4 );
	}  
	
	// Make a bounce sound
	if (le->leFlags & LEF_SOUND_BLOOD) {
		// half the gibs will make splat sounds
		int r = rand();
		if (r & 8) {
			sfxHandle_t	s = (&cgs.media.gibBounce1Sound)[r % 3];
			trap_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
	} else if (le->leFlags & LEF_SOUND_BRASS) {
		int r = rand() % 3;
		sfxHandle_t	s = (&cgs.media.brassBounce1Sound)[r];
		trap_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
	}
	le->leFlags &= ~(LEF_SOUNDS|LEF_MARKS|LEF_TUMBLE);

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	trap_R_AddRefEntityToScene( &le->refEntity, NULL );
}

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
static void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	if (c>1) c = 0xff;
	else c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;

	trap_R_AddRefEntityToScene( re, NULL );
}

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddConstRGB( localEntity_t *le ) {
	refEntity_t *re;
	re = &le->refEntity;
	re->shaderRGBA[0] = le->color[0] * 255;
	re->shaderRGBA[1] = le->color[1] * 255;
	re->shaderRGBA[2] = le->color[2] * 255;
	re->shaderRGBA[3] = le->color[3] * 255;
	trap_R_AddRefEntityToScene( re, NULL );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		fade;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	fade  = ( le->endTime - cg.time ) * le->fadeRate;
	if (fade > 1) 
		fade = 1;

	re->shaderRGBA[3] = 0xff * fade * le->color[3];
	re->radius = le->radius + le->radiusrate * ( cg.time - le->startTime);
	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < re->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re, NULL );
}

/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t *re;
	float c;
	vec3_t delta;
	float len;

	re = &le->refEntity;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];
	//if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
	//	re->radius = le->radius * ( 1.0 - c ) + 8;
	//}

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re, NULL );
}

/*
=================
CG_AddShrapnel
=================
*/
static void CG_AddShrapnel( localEntity_t *le, qboolean scale ) {
	vec3_t		org, org2;
	vec4_t		rgba;
	trace_t		tr;
	float		c, radius;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	rgba[0] = 1;
	rgba[1] = 1;
	rgba[2] = 1;
	rgba[3] = c;

	radius = le->radius + le->radiusrate * (cg.time - le->startTime);

	BG_EvaluateTrajectory( &le->pos, cg.oldTime, org );
	BG_EvaluateTrajectory( &le->pos, cg.time, org2 );
	CG_Trace(&tr, org, vec3_origin, vec3_origin, org2, -1, MASK_SOLID);
	CG_Tracer(org, tr.endpos, qfalse, radius, le->refEntity.customShader, rgba); // radius is shader
	if(tr.fraction < 1.0)
	{		
		if(le->leFlags & LEF_MARK_BLOOD)
			CG_DecalMark(cgs.media.bloodMarkShader,tr.endpos,tr.plane.normal,Q_flrand(0.0f, 1.0f)*360,radius * 4,colorWhite,cg_markTime.integer,cg_markTime.integer >> 4);
		CG_FreeLocalEntity(le);
	}
}

/*
=================
CG_Q3F_AddRing
=================
*/
static void CG_Q3F_AddRing( localEntity_t *le ) {
	float		radius;
	polyVert_t	*v;
	polyVert_t	verts[4];
	vec3_t		originalPoints[4];
	byte		rgba[4];
	vec3_t		forward, right;
	float		texCoordScale;
	int i;
	float		fade;
		
	fade = ( le->endTime - cg.time ) * le->fadeRate;

	if( fade < 1 ) {
		rgba[0] = rgba[1] = rgba[2] = rgba[3] = fade * 255;;
	} else {
		rgba[0] = rgba[1] = rgba[2] = rgba[3] = 255;
	}

	radius = le->radius + le->radiusrate * (cg.time - le->startTime);

	// Add the poly
	AngleVectors( le->angles.trBase, forward, right, NULL );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = le->refEntity.origin[i] - radius * forward[i] - radius * right[i];
		originalPoints[1][i] = le->refEntity.origin[i] + radius * forward[i] - radius * right[i];
		originalPoints[2][i] = le->refEntity.origin[i] + radius * forward[i] + radius * right[i];
		originalPoints[3][i] = le->refEntity.origin[i] - radius * forward[i] + radius * right[i];
	}

	for ( i = 0, v = verts ; i < 4 ; i++, v++ ) {
		vec3_t		delta;

		VectorCopy( originalPoints[i], v->xyz );

		VectorSubtract( v->xyz, le->refEntity.origin, delta );
		v->st[0] = 0.5 + DotProduct( delta, forward ) * texCoordScale;
		v->st[1] = 0.5 + DotProduct( delta, right ) * texCoordScale;
		*(int*)&v->modulate = *(int*)&rgba;
	}

	trap_R_AddPolyToScene( le->refEntity.customShader, 4, verts );
}

/*
=================
CG_Q3F_AddBeam
=================
*/
static void CG_Q3F_AddBeam( localEntity_t *le ) {
	refEntity_t			*ent;
	vec3_t				origin, origin2;

	VectorCopy( le->pos.trBase, origin );
	VectorCopy( le->angles.trBase, origin2 );
	
	// Straight beam
	if ( le->leFlags & Q3F_BEAM_STRAIGHT || le->lifeRate < 1 ) { 
		ent = &le->refEntity;
		VectorCopy( origin, ent->origin );
		VectorCopy( origin2, ent->oldorigin );
		AxisClear( ent->axis );
		ent->reType = RT_LIGHTNING;

		// add to refresh list
		trap_R_AddRefEntityToScene(ent, NULL);

		return;
	} else {
		vec3_t		p1, p2, v1, v2, dir, prevpt1, prevpt2, /*nextpt, mid,*/ delta, up;
		int			i,ii,jj;
		polyVert_t	points[4];
		float		length;
		//int			segnum = 0;
		//int			beamnum = 0;
		float		alphafactor;
		int			picW;
		int			seed;

		// Let's use some decent names
		float		angleVar = le->bounceFactor;
		int			numSubdivisions = le->lifeRate;
		vec4_t		colour;
		float		scale = le->radius;
		int			flags = le->leFlags;
		float		speedscale = le->light;

		VectorCopy4( le->color, colour );
	
		picW = 64;
		seed = 100;

		// calcluate length of beam segment
		VectorSubtract( origin2, origin, delta );
		length = VectorLength( delta );
		length /= numSubdivisions;

		// get the dir of beam
		VectorCopy( delta, dir );
		VectorNormalizeFast( dir );

		// Calculate the first up vector
		VectorSubtract( origin, cg.refdef.vieworg, v1 );
		VectorSubtract( origin2, cg.refdef.vieworg, v2 );
		CrossProduct( v1, v2, up );
		VectorNormalizeFast( up );
		
		// Calculate the first points
		VectorMA( origin, scale, up, prevpt1 );
		VectorMA( origin, -scale, up, prevpt2 );
		VectorCopy( origin, p1 );

		// go through and calculate each point of the beam and offset it by the anglevar
		for ( i = 1; i <= numSubdivisions; i++ ) {
			// Calculate the next point along the beam
			VectorMA( origin, i * length, dir, p2 );

			// Random variance on the next point ( except if it's the last )
			if ( i != numSubdivisions ) {
				if ( flags & Q3F_BEAM_WAVE_EFFECT ) {
					float phase = p2[0] + p2[1];

					p2[2] += sin( (phase + (float)cg.time) * speedscale ) * angleVar;
				} else if ( flags & Q3F_BEAM_WAVE_EFFECT_3D ) {
					float phase1 = p2[0] + p2[1];
					float phase2 = p2[0] + p2[2];
					float phase3 = p2[1] + p2[2];

					p2[0] += sin( (phase3 + (float)cg.time) * speedscale ) * angleVar;
					p2[1] += sin( (phase2 + (float)cg.time) * speedscale ) * angleVar;
					p2[2] += sin( (phase1 + (float)cg.time) * speedscale ) * angleVar;
				/*} else if ( flags & BEAM_USE_NOISE ) {
					p2.x += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.y += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.z += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;*/
				} else {
					p2[0] += Q_crandom( &seed ) * angleVar;
					p2[1] += Q_crandom( &seed ) * angleVar;
					p2[2] += Q_crandom( &seed ) * angleVar;
				}
			}

			// Create the up vec for the beam which is parallel to the viewplane
			VectorSubtract( p1, cg.refdef.vieworg, v1 );
			VectorSubtract( p2, cg.refdef.vieworg, v2 );
			CrossProduct( v1, v2, up );
			VectorNormalizeFast( up );
      
			// Build the quad
			VectorMA( p2, scale, up, points[0].xyz );
			VectorCopy( prevpt1, points[1].xyz );
			VectorCopy( prevpt2, points[2].xyz );
			VectorMA( p2, -scale, up, points[3].xyz );

			 // Tile the shader across the beam
			if ( flags & Q3F_BEAM_TILESHADER ) {
				float startS = ( length * ( i-1 ) ) / (float)picW;
				float endS   = ( length * ( i ) )   / (float)picW;

				points[0].st[0] = startS; points[0].st[1] = 1;
				points[1].st[0] = endS;   points[1].st[1] = 1;
				points[2].st[0] = endS;   points[2].st[1] = 0;
				points[3].st[0] = startS; points[3].st[1] = 0;
			} else {
				points[0].st[0] = 1;   points[0].st[1] = 1;
				points[1].st[0] = 0;   points[1].st[1] = 1;
				points[2].st[0] = 0;   points[2].st[1] = 0;
				points[3].st[0] = 1;   points[3].st[1] = 0;
			}
				
			//if ( !alphastep )
				alphafactor = 1.0f;
			//else
			//	alphafactor = startalpha + (alphastep * i);

			// Set the color of the verts
			for ( ii=0; ii<4; ii++ ) {
				for ( jj=0; jj<4; jj++ ) {
					points[ii].modulate[jj] = colour[jj] * alphafactor;
				}
			}

			trap_R_AddPolyToScene( le->refEntity.customShader, 4, points );

			// Save off the last point to use as the first point on the next quad
			VectorMA( p2, scale, up, prevpt1 );
			VectorMA( p2, -scale, up, prevpt2 );
			VectorCopy( p2, p1 );
		}
	}
}


/*
=================
CG_AddFallScaleFade

This is just an optimized CG_AddMoveScaleFade
For blood mists that drift down, fade out, and are
removed if the view passes through them.
There are often 100+ of these, so it needs to be simple.
=================
*/
static void CG_AddFallScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c,fade;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade time
	fade = ( le->endTime - cg.time ) * le->fadeRate;
	if (fade > 1)
		fade = 1;
	c = ( le->endTime - cg.time ) * le->lifeRate;
	re->shaderRGBA[3] = 0xff * fade * le->color[3];
	re->origin[2] = le->pos.trBase[2] - ( 1.0 - c ) * le->pos.trDelta[2];
	re->radius = le->radius + le->radiusrate * (cg.time - le->startTime);
	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < re->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}
	trap_R_AddRefEntityToScene( re, NULL );
}

/*
================
CG_AddExplosion - RR2DO2/Hentai: Now volumetric explosions
================
*/

static void CG_AddExplosion( localEntity_t *le ) {
	refEntity_t	*re;
	int i;

	re = &le->refEntity;
	
	re->radius = le->radius + le->radiusrate * (cg.time - le->startTime);
	re->shaderRGBA[3] = le->color[3] * 255 * (le->endTime - cg.time) * le->lifeRate;

	/* Calculate the axis */
	if (re->reType == RT_MODEL && re->radius != 1) {
		for ( i = 0 ; i < 3 ; i++ ) {
			VectorNormalize( re->axis[i] );
			VectorScale( re->axis[i], re->radius, re->axis[i] );
		}
		re->nonNormalizedAxes = qtrue;
	} else if (re->reType == RT_SPRITE) {

	}

	trap_R_AddRefEntityToScene(re, NULL);

	// add the dlight
	if ( le->light ) {
		float		light;

		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = le->light * light;
		trap_R_AddLightToScene(re->origin, light, 1.f, le->lightColor[0], le->lightColor[1], le->lightColor[2], 0, 0 );
	}
}

/*
===================
CG_AddScorePlum
===================
*/
#define NUMBER_SIZE		8

static void CG_AddScorePlum( localEntity_t *le ) {
	refEntity_t	*re;
	vec3_t		origin, delta, dir, vec, up = {0, 0, 1};
	float		c, len;
	int			i, score, digits[10], numdigits, negative;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;

	score = le->leFlags;
	if (score < 0) {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0x11;
		re->shaderRGBA[2] = 0x11;
	}
	else {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		if (score >= 50) {
			re->shaderRGBA[1] = 0;
		} else if (score >= 20) {
			re->shaderRGBA[0] = re->shaderRGBA[1] = 0;
		} else if (score >= 10) {
			re->shaderRGBA[2] = 0;
		} else if (score >= 2) {
			re->shaderRGBA[0] = re->shaderRGBA[2] = 0;
		}

	}
	if (c < 0.25)
		re->shaderRGBA[3] = 0xff * 4 * c;
	else
		re->shaderRGBA[3] = 0xff;

	re->radius = NUMBER_SIZE / 2;

	VectorCopy(le->pos.trBase, origin);
	origin[2] += 110 - c * 100;

	VectorSubtract(cg.refdef.vieworg, origin, dir);
	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	VectorMA(origin, -10 + 20 * sin(c * 2 * M_PI), vec, origin);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < 20 ) {
		CG_FreeLocalEntity( le );
		return;
	}

	negative = qfalse;
	if (score < 0) {
		negative = qtrue;
		score = -score;
	}

	for (numdigits = 0; !(numdigits && !score); numdigits++) {
		digits[numdigits] = score % 10;
		score = score / 10;
	}

	if (negative) {
		digits[numdigits] = 10;
		numdigits++;
	}

	for (i = 0; i < numdigits; i++) {
		VectorMA(origin, (float) (((float) numdigits / 2) - i) * NUMBER_SIZE, vec, re->origin);
		re->customShader = cgs.media.numberShaders[digits[numdigits-1-i]];
		trap_R_AddRefEntityToScene( re, NULL );
	}
}

/*
===================
CG_AddLocalLight
===================
*/
static void CG_AddLocalLight( localEntity_t *le )
{
//	vec3_t org;
	float intensity = le->color[3];
	float radius = le->radius + le->radiusrate * ( cg.time - le->startTime);
	float fade = ( le->endTime - cg.time ) * le->fadeRate;
	if (fade < 1) 
		intensity *= fade;
//	BG_EvaluateTrajectory( &le->pos, cg.time, org );
	trap_R_AddLightToScene(le->pos.trBase, radius, intensity, le->color[0], le->color[1], le->color[2], 0, 0 );
}

/*
===================
CG_AddBulletExplosion
===================
*/
static void CG_AddBulletExplosion( localEntity_t *le )
{
	refEntity_t	*re = &le->refEntity;	
	float c = ( le->endTime - cg.time ) * le->lifeRate;
	re->customShader = cgs.media.bulletExplosionShaders[(int)(c*2.999)];
	c *= 0xff;
	re->shaderRGBA[0] = c;
	re->shaderRGBA[1] = c;
	re->shaderRGBA[2] = c;
	re->shaderRGBA[3] = c;

	trap_R_AddRefEntityToScene( re, NULL );
}


/*
===================
CG_AddNapalmFlame
===================
*/
static void CG_AddNapalmFlame( localEntity_t *le )
{
	refEntity_t	*re;
	float		fade, len;
	vec3_t		delta, org;
	trace_t		tr;
	int			index;

	re = &le->refEntity;

	fade  = ( le->endTime - cg.time ) * le->fadeRate;
	if (fade > 1) 
		fade = 1;

	re->shaderRGBA[3] = 0xff * fade * le->color[3];
	re->radius = le->radius + le->radiusrate * ( cg.time - le->startTime);

	if (le->pos.trType != TR_STATIONARY) {
		BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );
		BG_EvaluateTrajectory( &le->pos, cg.oldTime, org );
		CG_Trace(&tr, org, NULL, NULL, re->origin, -1, MASK_SOLID);
		if (tr.fraction < 1.0 ) {
			VectorCopy(tr.endpos, re->origin );
			VectorCopy(tr.endpos, le->pos.trBase );
			le->pos.trType = TR_STATIONARY;
		}
	}
	/* Determine flame sprite to use */
	index = (NUM_FLAME_SPRITES*(cg.time - le->startTime)) / (le->endTime - le->startTime);
	re->customShader = cgs.media.flameShaders[index];

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < re->radius && ( rand() & 1 )) {
		return;
	}

	trap_R_AddRefEntityToScene( re, NULL );
}

static void CG_AddDebugBox(  localEntity_t *le )
{
	float	c;
	vec4_t color;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	if (c>1)
		c = 1;

	color[0] = c * le->color[0];
	color[1] = c * le->color[1];
	color[2] = c * le->color[2];
	color[3] = c * le->color[3];

	CG_DrawBoundingBox( le->pos.trBase, le->angles.trBase, le->angles.trDelta, color );
}


/*
===================
CG_AddLocalEntities

===================
*/

void CG_PositionSunflares2( void );
void CG_AddLocalEntities( void ) {
	localEntity_t	*le, *next;
	int i;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		if ( cg.time >= le->endTime ) {
			CG_FreeLocalEntity( le );
			continue;
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
		case LE_Q3F_PANELRADARBLIP:
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_SHRAPNEL:
			CG_AddShrapnel( le, qfalse );
			break;

		case LE_EXPAND_SHRAPNEL:
			CG_AddShrapnel( le, qtrue );
			break;

		case LE_EXPAND_FADE_RING:
			CG_Q3F_AddRing( le );
			break;

		case LE_BEAM:
			CG_Q3F_AddBeam( le );
			break;

		case LE_FRAGMENT:				// gibs and brass
			CG_AddFragment( le );
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			CG_AddMoveScaleFade( le );
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			CG_AddFadeRGB( le );
			break;
		case LE_CONST_RGB:
			CG_AddConstRGB( le );		// debug lines
			break;
		case LE_FALL_SCALE_FADE:		// gib blood trails
			CG_AddFallScaleFade( le );
			break;

		case LE_SCOREPLUM:
			CG_AddScorePlum( le );
			break;

		case LE_LIGHT:
			CG_AddLocalLight(le);
			break;

		case LE_BULLET_EXPLOSION:
			CG_AddBulletExplosion(le);
			break;

		case LE_NAPALM_FLAME:
			CG_AddNapalmFlame( le );
			break;

		case LE_DEBUG_BOX:
			CG_AddDebugBox( le );
			break;

		case LE_SCALE_FADE:     // rocket trails
			CG_AddScaleFade( le );
			break;
		}
	}

	// RR2DO2: add flares
	for( i = 0; i < cgs.numFlares; i++ ) {
		if( !trap_R_inPVS( cg.currentrefdef->vieworg, cgs.flares[i].pos ) )
			continue;

		if( !cg.rendering2ndRefDef )
			CG_SetFlareFader( &cgs.flares[i].flareFadeTime, &cgs.flares[i].flareFadeValue );
		CG_AddFullFlareToScene( cgs.flares[i].pos,
								cgs.flares[i].radius,
								(float)cgs.flares[i].rotation,
								cgs.flares[i].shader,
								cgs.flares[i].intensity,
								cgs.flares[i].color[0],
								cgs.flares[i].color[1],
								cgs.flares[i].color[2],
								( ( cgs.flares[i].type == FL_LENSFLARE || cgs.flares[i].type == FL_LENSFLAREBLIND ) ? qtrue : qfalse ),
								( ( cgs.flares[i].type == FL_LENSBLIND || cgs.flares[i].type == FL_LENSFLAREBLIND ) ? qtrue : qfalse ));
	}
	CG_SetFlareFader( NULL, NULL );

	// RR2DO2: sunflares
	if( cg.renderingSkyPortal ) {
		CG_PositionSunflares();
	}

	if( !cg.renderingSkyPortal ) {
		CG_PositionSunflares2();
	}

	/* Ensiform - sunflares without skyportals i guess :/ */
	//if( !cg.rendering2ndRefDef ) {
	//	CG_PositionSunflares();
	//}
}


