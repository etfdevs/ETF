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
**	bg_q3f_splines.h
**
**	Common functions for bezier spline math
*/

#ifndef	__BG_Q3F_SPLINES_H
#define	__BG_Q3F_SPLINES_H

#include "q_shared.h"

void MirrorVtxInVtx( vec3_t d, const vec3_t p, const vec3_t m );

// Have to hardcode maxsegments (grr, no malloc)
#define Q3F_MAXSPLINESEGMENTS 32

typedef struct Q3F_QuadSpline_s {
	int		nSegments;
	vec3_t	ControlPoint[3];
	//vec3_t	*SegmentVtx;
	vec3_t	SegmentVtx[Q3F_MAXSPLINESEGMENTS + 1];
	//float	length;
} Q3F_QuadSpline_t;

typedef struct Q3F_CubicSpline_s {
	int		nSegments;
	vec3_t	ControlPoint[4];
	//vec3_t	*SegmentVtx;
	vec3_t	SegmentVtx[Q3F_MAXSPLINESEGMENTS + 1];
	//float	length;
} Q3F_CubicSpline_t;

void BG_Q3F_QuadSpline_ComputeSegments ( Q3F_QuadSpline_t *spline );
float BG_Q3F_QuadSpline_Length ( Q3F_QuadSpline_t *spline );
void BG_Q3F_QuadSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_QuadSpline_t *spline );
void BG_Q3F_QuadSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_QuadSpline_t *spline );

void BG_Q3F_CubicSpline_ComputeSegments ( Q3F_CubicSpline_t *spline );
float BG_Q3F_CubicSpline_Length ( Q3F_CubicSpline_t *spline );
void BG_Q3F_CubicSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_CubicSpline_t *spline );
void BG_Q3F_CubicSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_CubicSpline_t *spline );

// General Spline Calculation Functions
void BG_Q3F_EvaluateSplineTrajectory( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );
void BG_Q3F_EvaluateSplineTrajectoryAngle( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );
//void BG_Q3F_EvaluateSplineTrajectoryDelta( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );

#endif
