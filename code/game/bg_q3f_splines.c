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
**	bg_q3f_splines.c
**
**	Shared functions for bezier spline math
*/

#include "bg_q3f_splines.h"

// 3 Basis functions for a quadratic bezier spline
// t^2 + 2t(1-t) + (1-t)^2
#define Q1(t)	(t * t)
#define Q2(t)	(2 * t * (1 - t))
#define Q3(t)	((1 - t) * (1 - t))

// 4 Basis functions for a cubic bezier spline
// t^3 + 3t^2(1-t) + 3t(1-t)^2 + (1-t)^3
#define CB1(t)	(t * t * t)
#define CB2(t)	(3 * t * t * (1 - t))
#define CB3(t)	(3 * t * (1 - t) * (1 - t))
#define CB4(t)	((1 - t) * (1 - t) * (1 - t))

void MirrorVtxInVtx( vec3_t d, const vec3_t p, const vec3_t m ) {
	VectorSubtract (p, m, d);
	VectorNegate (d, d);
	VectorAdd(m, d, d);
}

////////////////////////////////////////////////////////
// Quadratic Splines

// Builds the array with the segment coordinates.
// MUST be called when you change the number of
// segments in the spline structure to prevent 
// index out of bound errors 
void BG_Q3F_QuadSpline_ComputeSegments ( Q3F_QuadSpline_t *spline ) {
	int		s;
	float	pcQ1, pcQ2, pcQ3;
	float	lodbias;

	VectorCopy( spline->ControlPoint[0], spline->SegmentVtx[0] );

	if ( spline->nSegments > Q3F_MAXSPLINESEGMENTS )
		spline->nSegments = Q3F_MAXSPLINESEGMENTS;

	lodbias = 1.f / spline->nSegments;

	for ( s = 1; s < spline->nSegments; s++ ) {
		pcQ1 = Q1( lodbias * s );
		pcQ2 = Q2( lodbias * s );
		pcQ3 = Q3( lodbias * s );

		spline->SegmentVtx[spline->nSegments - s][0] = spline->ControlPoint[0][0]*pcQ1 + spline->ControlPoint[1][0]*pcQ2 + spline->ControlPoint[2][0]*pcQ3;
		spline->SegmentVtx[spline->nSegments - s][1] = spline->ControlPoint[0][1]*pcQ1 + spline->ControlPoint[1][1]*pcQ2 + spline->ControlPoint[2][1]*pcQ3;
		spline->SegmentVtx[spline->nSegments - s][2] = spline->ControlPoint[0][2]*pcQ1 + spline->ControlPoint[1][2]*pcQ2 + spline->ControlPoint[2][2]*pcQ3;
	}

	VectorCopy( spline->ControlPoint[2], spline->SegmentVtx[spline->nSegments] );
}

// Returns the lenght of the spline, by walking over the segments
float BG_Q3F_QuadSpline_Length ( Q3F_QuadSpline_t *spline ) {
	int		s;
	float	length;
	
	if ( !spline->SegmentVtx )
		return -1;

	length = .0f;

	for ( s = 0; s < spline->nSegments; s++ )
		length += Distance( spline->SegmentVtx[s], spline->SegmentVtx[s+1] );

	return length;
}

// Calculates the two nearest vertexes on the spline, given a relative position on the spline
void BG_Q3F_QuadSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_QuadSpline_t *spline ) {
	if ( !spline->SegmentVtx )
		return;

	VectorCopy( spline->SegmentVtx[(int) (relpos)], src );
	VectorCopy( spline->SegmentVtx[(int) (relpos + 1.f)], dst );

	// Ugly, but works:
	if ( (int) (relpos) == spline->nSegments )
		VectorCopy( spline->SegmentVtx[(int) (relpos)], dst );
}

// Calculates the angles of the two nearest vertexes on the spline, given a relative position on the spline
void BG_Q3F_QuadSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_QuadSpline_t *spline ) {
	int src_vtx_num, dst_vtx_num;

	if ( !spline->SegmentVtx )
		return;

	src_vtx_num = (int) (relpos);
	dst_vtx_num = (int) (relpos + 1.f);

	if ( src_vtx_num == 0 ) {
		VectorSubtract( spline->ControlPoint[1], spline->ControlPoint[0], src_angle );
		VectorSubtract( spline->SegmentVtx[dst_vtx_num + 1], spline->SegmentVtx[src_vtx_num], dst_angle );
	} else if ( src_vtx_num == spline->nSegments ) { // Ugly, but should work
		VectorSubtract( spline->ControlPoint[2], spline->ControlPoint[1], src_angle );
		VectorSubtract( spline->ControlPoint[2], spline->ControlPoint[1], dst_angle );
	} else if ( dst_vtx_num == spline->nSegments ) {
		VectorSubtract( spline->SegmentVtx[dst_vtx_num], spline->SegmentVtx[src_vtx_num - 1], src_angle );
		VectorSubtract( spline->ControlPoint[2], spline->ControlPoint[1], dst_angle );
	} else {
		VectorSubtract( spline->SegmentVtx[dst_vtx_num], spline->SegmentVtx[src_vtx_num - 1], src_angle );
		VectorSubtract( spline->SegmentVtx[dst_vtx_num + 1], spline->SegmentVtx[src_vtx_num], dst_angle );
	}
}

////////////////////////////////////////////////////////
// Cubic Splines

// Builds the array with the segment coordinates.
// MUST be called when you change the number of
// segments in the spline structure to prevent 
// index out of bound errors 
void BG_Q3F_CubicSpline_ComputeSegments ( Q3F_CubicSpline_t *spline ) {
	int		s;
	float	pcCB1, pcCB2, pcCB3, pcCB4;
	float	lodbias;

	VectorCopy( spline->ControlPoint[0], spline->SegmentVtx[0] );

	if ( spline->nSegments > Q3F_MAXSPLINESEGMENTS )
		spline->nSegments = Q3F_MAXSPLINESEGMENTS;

	lodbias = 1.f / spline->nSegments;

	for ( s = 1; s < spline->nSegments; s++ ) {
		pcCB1 = CB1( lodbias * s );
		pcCB2 = CB2( lodbias * s );
		pcCB3 = CB3( lodbias * s );
		pcCB4 = CB4( lodbias * s );

		spline->SegmentVtx[spline->nSegments - s][0] = spline->ControlPoint[0][0]*pcCB1 + spline->ControlPoint[1][0]*pcCB2 + spline->ControlPoint[2][0]*pcCB3 + spline->ControlPoint[3][0]*pcCB4;
		spline->SegmentVtx[spline->nSegments - s][1] = spline->ControlPoint[0][1]*pcCB1 + spline->ControlPoint[1][1]*pcCB2 + spline->ControlPoint[2][1]*pcCB3 + spline->ControlPoint[3][1]*pcCB4;
		spline->SegmentVtx[spline->nSegments - s][2] = spline->ControlPoint[0][2]*pcCB1 + spline->ControlPoint[1][2]*pcCB2 + spline->ControlPoint[2][2]*pcCB3 + spline->ControlPoint[3][2]*pcCB4;
	}

	VectorCopy( spline->ControlPoint[3], spline->SegmentVtx[spline->nSegments] );
}

// Returns the lenght of the spline, by walking over the segments
float BG_Q3F_CubicSpline_Length ( Q3F_CubicSpline_t *spline ) {
	int		s;
	float	length;
	
	if ( !spline->SegmentVtx )
		return -1;

	length = .0f;

	for ( s = 0; s < spline->nSegments; s++ )
		length += Distance( spline->SegmentVtx[s], spline->SegmentVtx[s+1] );

	return length;
}

// Calculates the two nearest vertexes on the spline, given a relative position on the spline
void BG_Q3F_CubicSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_CubicSpline_t *spline ) {
	if ( !spline->SegmentVtx )
		return;

	VectorCopy( spline->SegmentVtx[(int) (relpos)], src );
	VectorCopy( spline->SegmentVtx[(int) (relpos + 1.f)], dst );

	// Ugly, but works:
	if ( (int) (relpos) == spline->nSegments )
		VectorCopy( spline->SegmentVtx[(int) (relpos)], dst );
}

// Calculates the angles of the two nearest vertexes on the spline, given a relative position on the spline
void BG_Q3F_CubicSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_CubicSpline_t *spline ) {
	int src_vtx_num, dst_vtx_num;

	if ( !spline->SegmentVtx )
		return;

	src_vtx_num = (int) (relpos);
	dst_vtx_num = (int) (relpos + 1.f);

	if ( src_vtx_num == 0 ) {
		VectorSubtract( spline->ControlPoint[1], spline->ControlPoint[0], src_angle );
		VectorSubtract( spline->SegmentVtx[dst_vtx_num + 1], spline->SegmentVtx[src_vtx_num], dst_angle );
	} else if ( src_vtx_num == spline->nSegments ) { // Ugly, but should work
		VectorSubtract( spline->ControlPoint[3], spline->ControlPoint[2], src_angle );
		VectorSubtract( spline->ControlPoint[3], spline->ControlPoint[2], dst_angle );
	} else if ( dst_vtx_num == spline->nSegments ) {
		VectorSubtract( spline->SegmentVtx[dst_vtx_num], spline->SegmentVtx[src_vtx_num - 1], src_angle );
		VectorSubtract( spline->ControlPoint[3], spline->ControlPoint[2], dst_angle );
	} else {
		VectorSubtract( spline->SegmentVtx[dst_vtx_num], spline->SegmentVtx[src_vtx_num - 1], src_angle );
		VectorSubtract( spline->SegmentVtx[dst_vtx_num + 1], spline->SegmentVtx[src_vtx_num], dst_angle );
	}
}

// General Spline Calculation Functions

/*
===========================
BG_Q3F_EvaluateSplineTrajectory

Done this at 2am in the Hampton Hotel at QuakeCon2K
===========================
*/
void BG_Q3F_EvaluateSplineTrajectory( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result ) {
	float		deltaTime;
	vec3_t		src, dst, delta;
	float		f;

	switch( tr->trType ) {
	case TR_QUAD_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		BG_Q3F_QuadSpline_Position( src, dst, ( deltaTime / tr->trDuration ) * qspline->nSegments , qspline );

		//if ( !qspline->length )
		//	qspline->length = BG_Q3F_QuadSpline_Length( qspline );

		// Calculate the time it takes to travel distance from segmentvtx[0] to segmentvtx[src]
		deltaTime = deltaTime - ( (int) ( ( deltaTime / tr->trDuration ) * qspline->nSegments ) ) * ( tr->trDuration / qspline->nSegments );

		// Calculate the direction of travelling from segmentvtx[src] towards segmentvtx[dst]
		VectorSubtract ( dst, src, delta );

		// Calculate current exact position
		f = 1000.0 / ( tr->trDuration / qspline->nSegments );
		VectorScale( delta, f, delta );
		deltaTime *= 0.001f;	// milliseconds to seconds
		VectorMA( src, deltaTime, delta, result );

		break;
	case TR_CUBIC_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		BG_Q3F_CubicSpline_Position( src, dst, ( deltaTime / tr->trDuration ) * cspline->nSegments , cspline );

		//if ( !cspline->length )
		//	cspline->length = BG_Q3F_CubicSpline_Length( cspline );

		// Calculate the time it takes to travel distance from segmentvtx[0] to segmentvtx[src]
		deltaTime = deltaTime - ( (int) ( ( deltaTime / tr->trDuration ) * cspline->nSegments ) ) * ( tr->trDuration / cspline->nSegments );

		// Calculate the direction of travelling from segmentvtx[src] towards segmentvtx[dst]
		VectorSubtract ( dst, src, delta );

		// Calculate current exact position
		f = 1000.0 / ( tr->trDuration / cspline->nSegments );
		VectorScale( delta, f, delta );
		deltaTime *= 0.001f;	// milliseconds to seconds
		VectorMA( src, deltaTime, delta, result );

		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateSplineTrajectory: unknown trType: %i", tr->trType );
		break;
	}
}

/*
===========================
BG_Q3F_EvaluateSplineTrajectoryAngle

For determining velocity at a given time
===========================
*/
void BG_Q3F_EvaluateSplineTrajectoryAngle( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result ) {
	float		deltaTime;
	vec3_t		src_angle, dst_angle, delta;
	float		f;

	switch( tr->trType ) {
	case TR_QUAD_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		BG_Q3F_QuadSpline_PositionAngle( src_angle, dst_angle, ( deltaTime / tr->trDuration ) * qspline->nSegments , qspline );

		//if ( !qspline->length )
		//	qspline->length = BG_Q3F_QuadSpline_Length( qspline );

		// Calculate the time it takes to travel distance from segmentvtx[0] to segmentvtx[src]
		deltaTime = deltaTime - ( (int) ( ( deltaTime / tr->trDuration ) * qspline->nSegments ) ) * ( tr->trDuration / qspline->nSegments );

		// Calculate the angle of travelling from segmentvtx[src] towards segmentvtx[dst]
		VectorSubtract ( dst_angle, src_angle, delta );

		// Calculate current exact angle
		f = 1000.0 / ( tr->trDuration / qspline->nSegments );
		VectorScale( delta, f, delta );
		deltaTime *= 0.001f;	// milliseconds to seconds
		VectorMA( src_angle, deltaTime, delta, result );

		break;
	case TR_CUBIC_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		BG_Q3F_CubicSpline_PositionAngle( src_angle, dst_angle, ( deltaTime / tr->trDuration ) * cspline->nSegments , cspline );

		//if ( !cspline->length )
		//	cspline->length = BG_Q3F_CubicSpline_Length( cspline );

		// Calculate the time it takes to travel distance from segmentvtx[0] to segmentvtx[src]
		deltaTime = deltaTime - ( (int) ( ( deltaTime / tr->trDuration ) * cspline->nSegments ) ) * ( tr->trDuration / cspline->nSegments );

		// Calculate the angle of travelling from segmentvtx[src] towards segmentvtx[dst]
		VectorSubtract ( dst_angle, src_angle, delta );

		// Calculate current exact angle
		f = 1000.0 / ( tr->trDuration / cspline->nSegments );
		VectorScale( delta, f, delta );
		deltaTime *= 0.001f;	// milliseconds to seconds
		VectorMA( src_angle, deltaTime, delta, result );

		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateSplineTrajectory: unknown trType: %i", tr->trType );
		break;
	}
}

/*
================
BG_Q3F_EvaluateSplineTrajectoryDelta

For determining velocity at a given time

NOTE this is completely broken, and prolly not even needed.
================
*/
/*void BG_Q3F_EvaluateSplineTrajectoryDelta( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result ) {
	float	deltaTime;
	vec3_t	src, dst, delta;

	switch( tr->trType ) {
	case TR_QUAD_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		//BG_Q3F_QuadSpline_Position( src, dst, ( deltaTime / tr->trDuration ), qspline );
		BG_Q3F_QuadSpline_Position( src, dst, ( deltaTime / tr->trDuration ) * qspline->nSegments, qspline );
		deltaTime = ( (int) ( ( deltaTime / tr->trDuration ) - .5f ) - 1 ) * ( tr->trDuration / ( qspline->nSegments - 1 ) );
		VectorSubtract ( dst, src, delta );
		VectorScale( delta, 1 / ( tr->trDuration / ( qspline->nSegments - 1 ) ), delta );
		VectorCopy( delta, result );
		break;
	case TR_CUBIC_SPLINE_PATH:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime );
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		//BG_Q3F_CubicSpline_Position( src, dst, ( deltaTime / tr->trDuration ), cspline );
		BG_Q3F_CubicSpline_Position( src, dst, ( deltaTime / tr->trDuration ) * cspline->nSegments , cspline );
		deltaTime *= 0.001;
		//deltaTime = ( (int) ( ( deltaTime / tr->trDuration ) - .5f ) - 1 ) * ( tr->trDuration / ( cspline->nSegments - 1 ) );
		deltaTime = deltaTime - ( (int) ( ( ( deltaTime / tr->trDuration ) * cspline->nSegments ) - .5f) ) * ( tr->trDuration / cspline->nSegments );
		VectorSubtract ( dst, src, delta );
		//VectorScale( delta, 1 / ( tr->trDuration / ( cspline->nSegments - 1 ) ), delta );
		VectorNormalizeFast( delta );
		vectoangles( delta, delta );
		VectorCopy( delta, result );
		break;
	default:
		Com_Error( ERR_DROP, "BG_Q3F_EvaluateSplineTrajectoryDelta: unknown trType: %i", tr->trType );
		break;
	}
}*/
