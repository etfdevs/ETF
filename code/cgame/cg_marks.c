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

// cg_marks.c -- wall marks

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/

static markPoly_t	*cg_freeMarkPolys;			// single linked list
static markPoly_t	*cg_firstMarkPoly;	// double linked list
static markPoly_t	*cg_lastMarkPoly;	// double linked list
static markPoly_t	cg_markPolys[MAX_MARK_POLYS];

#define	MAX_MARK_FRAGMENTS		384
#define	MAX_MARK_POINTS			1024

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void	CG_InitMarkPolys( void ) {
	int		i;
	memset( cg_markPolys, 0, sizeof(cg_markPolys) );

	for ( i = 0 ; i < MAX_MARK_POLYS - 2 ; i++ ) {
		cg_markPolys[i].nextMark=&cg_markPolys[i+1];
	}
	cg_freeMarkPolys = cg_markPolys;
	cg_firstMarkPoly = 0;
	cg_lastMarkPoly = 0;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
static markPoly_t	*CG_AllocMark( int endtime ) {
	markPoly_t	*mp;
	markPoly_t	*scan;

	/* Find/clear a new poly */
	if ( !cg_freeMarkPolys ) {
		// remove the first mark, will have the lowest endtime
		mp = cg_firstMarkPoly;
		cg_firstMarkPoly = cg_firstMarkPoly->nextMark;
		cg_firstMarkPoly->prevMark = 0;
	} else {
		mp = cg_freeMarkPolys;
		cg_freeMarkPolys = cg_freeMarkPolys->nextMark;
	}
	memset( mp, 0, sizeof(markPoly_t) );
	mp->endtime=endtime;
	// No active polys yet, fill up the first
	if (!cg_firstMarkPoly) {
		cg_firstMarkPoly=mp;
		cg_lastMarkPoly=0;
		return mp;
	} 
	//Only 1 active poly, make 1 of the 2 last
	if (!cg_lastMarkPoly) {
		if (cg_firstMarkPoly->endtime<endtime) {
			cg_lastMarkPoly=mp;
			cg_firstMarkPoly->nextMark=mp;
			mp->prevMark=cg_firstMarkPoly;
		} else {
			cg_lastMarkPoly=cg_firstMarkPoly;
			cg_firstMarkPoly=mp;
			cg_firstMarkPoly->nextMark=cg_lastMarkPoly;
			cg_lastMarkPoly->prevMark=cg_firstMarkPoly;
		}
		return mp;
	}
	/* See if it fits at the start of the list */
	if (cg_firstMarkPoly->endtime>=endtime) {
		mp->nextMark=cg_firstMarkPoly;
		cg_firstMarkPoly->prevMark=mp;
		cg_firstMarkPoly=mp;
		return mp;
	}
	/* See if it fits at the end of the list */
	if (cg_lastMarkPoly->endtime<=endtime) {
		mp->prevMark=cg_lastMarkPoly;
		cg_lastMarkPoly->nextMark=mp;
		cg_lastMarkPoly=mp;
		return mp;
	}
	/* Go through list starting from pre-last and find a place for this new mark */
	scan=cg_lastMarkPoly->prevMark;
	while (scan && scan->endtime>endtime) scan=scan->prevMark;
	if (!scan) CG_Error("CG_AllocMark:Empty list/Couldn't find entry.");
	//Add our new poly in the list at correct time index
	mp->nextMark=scan->nextMark;
	mp->prevMark=scan;
	scan->nextMark->prevMark=mp;
	scan->nextMark=mp;
	return mp;
}

void CG_OldMark(qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
				    float orientation,float radius,vec4_t color, 
					int lifetime,leMarkFadeType_t fadetype)
{
	int				i,j;
	vec3_t			pushedOrigin;
	vec3_t			axis[3];
	vec4_t			projection;
	vec3_t			points[4];
	byte			bytecolor[4];
	float			texCoordScale;
	int				numFragments;
	markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec5_t			markPoints[MAX_MARK_POINTS];
	int				endtime,fadetime;


	if (!lifetime) return;

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	VectorSubtract( vec3_origin, axis[0], projection );
	projection[ 3 ] = radius;

	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	/* push the origin out a bit */
	VectorMA( origin, 0.5f, dir, pushedOrigin );

	/* create the full polygon */
	for( i = 0; i < 3; i++ )
	{
		points[ 0 ][ i ] = pushedOrigin[ i ] - radius * axis[ 1 ][ i ] - radius * axis[ 2 ][ i ];
		points[ 1 ][ i ] = pushedOrigin[ i ] - radius * axis[ 1 ][ i ] + radius * axis[ 2 ][ i ];
		points[ 2 ][ i ] = pushedOrigin[ i ] + radius * axis[ 1 ][ i ] + radius * axis[ 2 ][ i ];
		points[ 3 ][ i ] = pushedOrigin[ i ] + radius * axis[ 1 ][ i ] - radius * axis[ 2 ][ i ];
	}

	switch (fadetype) {
	case LEMFT_NORMAL:
		endtime=cg.time+lifetime;
		fadetime=endtime-(lifetime>>4);
		break;
	case LEMFT_ALPHA:
		endtime=cg.time+lifetime;
		fadetime=endtime-(9*lifetime/10);
		break;
	default:
		endtime=fadetime=0;
	}
	texCoordScale = 0.5 * 1.0 / radius;
	VectorScale( projection, projection[3], projection );
	numFragments = trap_CM_MarkFragments( 4, (void *)points,
					projection, MAX_MARK_POINTS, (float *)&markPoints[0],
					MAX_MARK_FRAGMENTS, markFragments );
	bytecolor[0]=color[0]*255;
	bytecolor[1]=color[1]*255;
	bytecolor[2]=color[2]*255;
	bytecolor[3]=color[3]*255;
	
	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		polyVert_t	*v;
		qboolean	hasST;
		polyVert_t	temp_verts[MAX_VERTS_ON_POLY];

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			mf->numPoints = MAX_VERTS_ON_POLY;
			hasST = qfalse;
		} else if (mf->numPoints < 0) {
			hasST = qtrue;
			mf->numPoints *= -1;
		} else {
			hasST = qfalse;
		}
		if (fadetype!=LEMFT_TEMP) {
			markPoly_t	*mark;
			mark = CG_AllocMark(endtime);
			mark->fadetime = fadetime;
			mark->fadeType = fadetype;
			mark->markShader = markShader;
			mark->poly.numVerts = mf->numPoints;
			*(int *)&mark->color = *(int *)&bytecolor;
			v = mark->verts;
		} else {
			v = temp_verts;
		}
		for ( j = 0; j < mf->numPoints ; j++, v++ ) {
			vec3_t		delta;
			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );
			if (!hasST) {
				VectorSubtract( v->xyz, origin, delta );
				v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
				v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			} else {
				v->st[0] = markPoints[mf->firstPoint + j][3];
				v->st[1] = markPoints[mf->firstPoint + j][4];
			}
			*(int *)&v->modulate = *(int *)&bytecolor;
         }
	     if (fadetype==LEMFT_TEMP) {
			trap_R_AddPolyToScene( markShader, mf->numPoints, temp_verts );
		 }
	}
}


/*
===============
CG_AddMarks
===============
*/

void CG_AddMarks( void ) {
	int			j;
	markPoly_t	*mp,*next;
	float		fade;

	for ( mp=cg_firstMarkPoly ; mp ; mp=next ) {
		/* Check if it's time to remove this mark */
		next=mp->nextMark;
		if ( cg.time >= mp->endtime ) {
			cg_firstMarkPoly=next;
			//Add to the free poly list
			mp->nextMark=cg_freeMarkPolys;
			cg_freeMarkPolys=mp;
			//Also check if we have cleared the entire mark list
			if (!cg_firstMarkPoly) {
				cg_lastMarkPoly=0;
				return;
			} else cg_firstMarkPoly->prevMark=0;
			if (!cg_firstMarkPoly->nextMark) {
				cg_lastMarkPoly=0;
			}
			continue;
		}
		if ( cg.time >mp->fadetime) {
			fade=(float)(mp->endtime-cg.time)/(float)(mp->endtime-mp->fadetime);
			switch (mp->fadeType) {
			case LEMFT_NORMAL:
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[3] = mp->color[3] * fade;
				}
				break;
			case LEMFT_ALPHA:
				if (fade>0.8f) {
					fade=(fade-0.8f)*5;
					for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
						mp->verts[j].modulate[0] = mp->color[0] * fade;
						mp->verts[j].modulate[1] = mp->color[1] * fade;
						mp->verts[j].modulate[2] = mp->color[2] * fade;
					}
				} else if (fade<0.4) {
					fade*=(1/0.4);
					for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
						mp->verts[j].modulate[3] = mp->color[3] * fade;
					}
				}
				break;
			}
		} 
		trap_R_AddPolyToScene( mp->markShader, mp->poly.numVerts, mp->verts );
	}
}

/* This one shows the decals using the ET trap for it */
void CG_DecalMark(qhandle_t markShader, const vec3_t origin, vec4_t projection, 
				    float orientation,float radius,vec4_t color, 
					int lifetime,int fadetime
					)
{
	int			i;
	vec3_t		axis[ 3 ];
	vec3_t		points[ 4 ];

	if (cg.rendering2ndRefDef) 
		return;

	if (lifetime <= 0) 
		return;

	/* make rotated polygon axis */
	VectorCopy( projection, axis[ 0 ] );
	PerpendicularVector( axis[ 1 ], axis[ 0 ] );
	RotatePointAroundVector( axis[ 2 ], axis[ 0 ], axis[ 1 ], -orientation );
	CrossProduct( axis[ 0 ], axis[ 2 ], axis[ 1 ] );
	
	/* create the full polygon */
	for( i = 0; i < 3; i++ )
	{
		points[ 0 ][ i ] = origin[ i ] - radius * axis[ 1 ][ i ] - radius * axis[ 2 ][ i ];
		points[ 1 ][ i ] = origin[ i ] - radius * axis[ 1 ][ i ] + radius * axis[ 2 ][ i ];
		points[ 2 ][ i ] = origin[ i ] + radius * axis[ 1 ][ i ] + radius * axis[ 2 ][ i ];
		points[ 3 ][ i ] = origin[ i ] + radius * axis[ 1 ][ i ] - radius * axis[ 2 ][ i ];
	}
	
	/* add the decal */
	trap_R_ProjectDecal( markShader, 4, points, projection, color, lifetime, fadetime );
}

qboolean CG_ShadowMark(vec3_t origin, float radius, float height, float *shadowPlane )
{
	vec3_t		end,mins,maxs;
	vec4_t		color,projection;
	trace_t		trace;

	*shadowPlane = 0;

	if (!cg_shadows.integer) return qfalse;
	if (cg.rendering2ndRefDef) 	return qfalse;
	
	VectorSet( mins, -radius, -radius, 0);
	VectorSet( maxs, radius, radius, 2);

	// send a trace down from the origin downwards
	VectorCopy( origin, end );
	end[2] -= height;

	trap_CM_BoxTrace( &trace, origin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// Check if the trace actually hit something
	if ( trace.fraction == 1.0f ) {
		return qfalse;
	}
	*shadowPlane=trace.endpos[2]+=1;

	// fade the shadow out with height 
	color[0]=color[1]=color[2]= 1.0 - trace.fraction;
	color[3]=1.0f;
	VectorSet( projection, 0, 0, -1 );
	projection[ 3 ] = radius;
	trap_R_ProjectDecal(cgs.media.shadowMarkShader, 1, &trace.endpos, projection,color, 1, 0);
	return qtrue;
}

void CG_ExplosionMark(vec3_t origin,float radius,vec4_t color) 
{
	vec4_t projection;

	if (!cg_markTime.integer) return;
	VectorSet4( projection, 0, 0, -1, radius);
	trap_R_ProjectDecal(cgs.media.burnMarkShader, 1, (vec3_t*)origin, projection, color, cg_markTime.integer, (cg_markTime.integer >> 3) );
}

void CG_BulletMark(qhandle_t shader,vec3_t origin,vec3_t dir,float radius,vec4_t color) {
	vec4_t projection;
	vec3_t markOrigin;
	int totaltime,fadetime;

	if (!cg_markTime.integer) return;
	totaltime=cg_markTime.integer;
	fadetime=cg_markTime.integer >> 4;
	VectorSubtract( vec3_origin, dir, projection );
	projection[ 3 ] = radius;
	VectorMA( origin, -0.5f, projection, markOrigin );

	CG_DecalMark( shader, origin, projection, 
		Q_flrand(0.0f, 1.0f)*360, radius,color,totaltime,fadetime);
}

